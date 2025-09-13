// -*- C++ -*-
//

#include <utility>
#include <unordered_map>

#include <string>
#include <iostream>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "Geometry/CommonDetUnit/interface/PixelGeomDetUnit.h"

#include "CondFormats/SiPhase2TrackerObjects/interface/TrackerDetToDTCELinkCablingMap.h"
#include "CondFormats/SiPhase2TrackerObjects/interface/DTCELinkId.h"
#include "CondFormats/DataRecord/interface/TrackerDetToDTCELinkCablingMapRcd.h"

#include "DataFormats/Phase2TrackerDigi/interface/Phase2ITChipBitStream.h"
#include "DataFormats/Common/interface/DetSetVector.h"
#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
#include "DataFormats/FEDRawData/interface/FEDRawData.h"
#include "DataFormats/FEDRawData/interface/FEDHeader.h"
#include "DataFormats/FEDRawData/interface/FEDTrailer.h"

class Phase2ITSLinkProducer : public edm::one::EDProducer<> {
public:
  explicit Phase2ITSLinkProducer(const edm::ParameterSet&);
  ~Phase2ITSLinkProducer() override;

private:
  void beginJob() override;
  void produce(edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

  const edm::ESGetToken<TrackerDetToDTCELinkCablingMap, TrackerDetToDTCELinkCablingMapRcd> cablingMapToken_;
  const edm::EDGetTokenT<edm::DetSetVector<Phase2ITChipBitStream>> ITChipBitStreamToken_;

  void printFEDRawDataCollection(const FEDRawDataCollection& fedRawDataCollection);
  uint16_t get16BitsFromBuffer(const unsigned char* buffer, size_t wordIndex);
  unsigned int IndexingDTC(unsigned int dtc_id);
  void AddHexToPtr(unsigned char *data_ptr, int globalIndex, int localIndex, const std::vector<bool>& moduleBitStream);

};

Phase2ITSLinkProducer::Phase2ITSLinkProducer(const edm::ParameterSet& iConfig)
  : cablingMapToken_(esConsumes()),
    ITChipBitStreamToken_(consumes<edm::DetSetVector<Phase2ITChipBitStream>>(iConfig.getParameter<edm::InputTag>("Phase2ITChipBitStream"))){
    produces<FEDRawDataCollection>();
}

Phase2ITSLinkProducer::~Phase2ITSLinkProducer() {}

void Phase2ITSLinkProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {

    using namespace edm;
    using namespace std;
    static const int MIN_DTC_ID = 0;
    static const int MAX_DTC_ID = 36;
    static const int MIN_SLINK_ID = 0;
    static const int MAX_SLINK_ID = 15;
    static const int SLINKS_PER_DTC = 16;

    unsigned int EventID = iEvent.id().event();
    const auto& cablingMap = iSetup.getData(cablingMapToken_);
    auto fedRawDataCollection = std::make_unique<FEDRawDataCollection>();

    unsigned int MODULES_ASSIGNED[36] = {0,};

    // loop over modules
    for (const auto& detSet : iEvent.get(ITChipBitStreamToken_)){
        // FIXME ignore header and trailer for now

        DetId detId = detSet.detId();
        // detId.rawId() : can do this but need to loop over again
        // Define a map file, xml or db to do the remapping
        auto equal_range=cablingMap.detIdToDTCELinkId(detId);
        unsigned int dtc_id = -1;
        for (auto it = equal_range.first; it != equal_range.second; ++it){
            dtc_id = it->second.dtc_id();
        }
        unsigned int dtc_id_index = IndexingDTC(dtc_id);
        unsigned int slink_in_dtc_index = MODULES_ASSIGNED[dtc_id_index]%16;
        unsigned int total_index = dtc_id_index*SLINKS_PER_DTC + slink_in_dtc_index;
        MODULES_ASSIGNED[dtc_id_index]++;
        //std::cout << "module_id " << detId << std::endl;
        //std::cout << "dtc_id " << dtc_id << std::endl;

        std::vector<bool> offset_bits; // collects offset bits
        std::vector<bool> data_bits; // collects data stream bits
        unsigned int bitstream_cumulative = 0; // accumulating data stream bits to fill in offset bits

        // loop over chips for a given module
        for (const auto& chip : detSet){
            std::vector<bool> bitstream = chip.get_bitstream();

            std::cout << "bitstream_cumulative " << bitstream_cumulative <<std::endl;
            // 16 bit configuration but offset is split into MSB and LSB
            unsigned int offset_chip = bitstream_cumulative;
            // unsigned int offset_chip = (bitstream_cumulative+31)/32;
            // std::cout << "offset_chip " << offset_chip <<std::endl;;

            // MSB LSB dummy value
            std::vector<bool> offset_MSB(16, false);
            std::vector<bool> offset_LSB(16, false);
            for (unsigned int i=0; i<16; ++i){
                offset_MSB[15-i] = (offset_chip >> (i+16)) & 1;
                offset_LSB[15-i] = (offset_chip >> i) & 1;
            }

            if (total_index == 0){
            for (unsigned int i=0; i<16; ++i){
                std::cout << offset_MSB[i];
            }
            for (unsigned int i=0; i<16; ++i){
                std::cout << offset_LSB[i];
            }
            std::cout <<std::endl;
            std::cout <<std::endl;
            std::cout <<std::endl;
            }

            offset_bits.insert(offset_bits.end(), offset_MSB.begin(), offset_MSB.end());
            offset_bits.insert(offset_bits.end(), offset_LSB.begin(), offset_LSB.end());

            std::vector<bool> data_header(16, true); // inject dummy
            std::vector<bool> data_chipsize(16, true); // inject dummy
            data_bits.insert(data_bits.end(), data_header.begin(), data_header.end());
            data_bits.insert(data_bits.end(), data_chipsize.begin(), data_chipsize.end());
            data_bits.insert(data_bits.end(), bitstream.begin(), bitstream.end());

            // padding to 16x8 bits for data block
            unsigned int padding_data_bits = (128 - (data_bits.size() % 128)) % 128;
            if (padding_data_bits > 0){
                data_bits.insert(data_bits.end(), padding_data_bits, false);      
            }                                                                     

            bitstream_cumulative += data_bits.size()/128; 
        }

        // padding to 16x8 bits for offset block
        unsigned int padding_offset_bits = (128 - (offset_bits.size() % 128)) % 128;
        if (padding_offset_bits > 0){
            offset_bits.insert(offset_bits.end(), padding_offset_bits, false);
        }             


        //std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"<<std::endl;
        unsigned int offset_bytes = 4 * ((offset_bits.size() + 15) / 16);
        unsigned int data_bytes = 4 * ((data_bits.size() + 15) / 16);

        FEDRawData combined_slink;
        combined_slink.resize(offset_bytes + data_bytes);
        unsigned char *ptr = combined_slink.data();

        unsigned int offset_chunks = offset_bits.size() / 16;
        unsigned int data_chunks = data_bits.size() / 16;

        for (unsigned int i = 0; i < offset_chunks; ++i) {
            AddHexToPtr(ptr, i, i, offset_bits);
        }
        for (unsigned int i = 0; i < data_chunks; ++i) {
            AddHexToPtr(ptr, i + offset_chunks, i, data_bits);
        }
        FEDRawData& current_slink = fedRawDataCollection->FEDData(total_index);
        unsigned int current_slink_size = current_slink.size();
        unsigned int new_slink_size = current_slink_size + combined_slink.size();
        current_slink.resize(new_slink_size);
        std::memcpy(current_slink.data() + current_slink_size, combined_slink.data(), combined_slink.size());
    }
    printFEDRawDataCollection(*fedRawDataCollection);

    //std::cout<<"modules assigned to"<<std::endl;
    for (int i=0; i<36; i++){
        std::cout<<i<<"\t"<<MODULES_ASSIGNED[i]<<std::endl;
    }
    iEvent.put(std::move(fedRawDataCollection));
}

void Phase2ITSLinkProducer::printFEDRawDataCollection(const FEDRawDataCollection& fedRawDataCollection) {
    std::cout << "DEBUGGER" << std::endl;
    // Loop over all FED IDs; here we focus on FED 0 for debugging.
    for (int fedId = 0; fedId < 576; ++fedId) {
        if (fedId != 0) continue;  // Only process FED 0 for debugging.
        const FEDRawData& fedData = fedRawDataCollection.FEDData(fedId);
        if (fedData.size() == 0) continue; // Skip empty FEDs

        std::cout << "FED ID: " << fedId << ", Size: " << fedData.size() << " bytes" << std::endl;
        const unsigned char* data = fedData.data();
        size_t totalChunks = fedData.size() / 4; // each chunk is 4 bytes = 1 packed 16-bit word
        
        // Reconstruct 16-bit words from the raw data.
        std::vector<uint16_t> words;
        words.reserve(totalChunks);
        for (size_t i = 0; i < totalChunks; ++i) {
            uint16_t word = get16BitsFromBuffer(data, i);
            words.push_back(word);
        }

        // Now, combine consecutive pairs into a 32-bit value and print one per line.
        if (words.size() % 2 != 0) {
            edm::LogWarning("Phase2ITSLinkProducer")
                << "FED 0 has an odd number of 16-bit words; the last word will be padded with 0.";
        }
        for (size_t i = 0; i < words.size(); i += 2) {
            uint32_t word32 = (words[i] << 16) | ((i+1 < words.size()) ? words[i+1] : 0);
            std::cout << std::setw(8) << std::setfill('0') << std::hex << word32 << std::endl;
        }
        // Reset formatting to default.
        std::cout << std::dec << std::setfill(' ') << std::endl;
    }
}

// Helper: reconstruct 16-bit word from 4 nibble bytes stored in the packer.
// The nibble ordering is: nib0 -> [15..12], nib1 -> [11..8], nib2 -> [7..4], nib3 -> [3..0].
uint16_t Phase2ITSLinkProducer::get16BitsFromBuffer(const unsigned char* buffer, size_t wordIndex) {
  size_t base = wordIndex * 4;
  uint16_t nib0 = buffer[base + 0] & 0xF;
  uint16_t nib1 = buffer[base + 1] & 0xF;
  uint16_t nib2 = buffer[base + 2] & 0xF;
  uint16_t nib3 = buffer[base + 3] & 0xF;
  uint16_t word = (nib0 << 12) | (nib1 << 8) | (nib2 << 4) | nib3;
  return word;
}

unsigned int Phase2ITSLinkProducer::IndexingDTC(unsigned int dtc_id){
    // Mapping 11-19, 21-29, 31-39, 41-49 to 0-35
    unsigned int first = dtc_id / 10;
    unsigned int second = dtc_id % 10;
    return (10 * (first - 1) + second - first);
}

void Phase2ITSLinkProducer::AddHexToPtr(unsigned char* ptr,
                 int globalIndex,
                 int localIndex,
                 const std::vector<bool>& bits)
{
    uint16_t hex_word = 0;

    // Read from chunk `localIndex` in the bits
    for (int i = 0; i < 16; ++i) {
        if (bits[ localIndex * 16 + i ]) {
            hex_word |= (1 << (15 - i));
        }
    }

    // Store into chunk `globalIndex` in the output buffer
    ptr[globalIndex * 4 + 0] = (hex_word >> 12) & 0xF;
    ptr[globalIndex * 4 + 1] = (hex_word >>  8) & 0xF;
    ptr[globalIndex * 4 + 2] = (hex_word >>  4) & 0xF;
    ptr[globalIndex * 4 + 3] = (hex_word >>  0) & 0xF;
}

void Phase2ITSLinkProducer::beginJob() {}

void Phase2ITSLinkProducer::endJob() {}

//define this as a plug-in
DEFINE_FWK_MODULE(Phase2ITSLinkProducer);



