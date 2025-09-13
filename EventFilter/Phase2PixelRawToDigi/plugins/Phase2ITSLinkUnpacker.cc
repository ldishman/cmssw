// -*- C++ -*-
#include <cstdint>
#include <vector>
#include <memory>
#include <iostream>
#include <iomanip>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
#include "DataFormats/FEDRawData/interface/FEDRawData.h"
#include "DataFormats/FEDRawData/interface/FEDNumbering.h"

#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/Phase2TrackerDigi/interface/Phase2ITChipBitStream.h"
#include "DataFormats/Common/interface/DetSetVector.h"
#include "DataFormats/Common/interface/DetSetVectorNew.h"

/**
 * A minimal Unpacker that:
 *  1) Reads FEDRawDataCollection (from Phase2ITSLinkProducer).
 *  2) Reconstructs 16-bit words from the 4-nibble packing.
 *  3) Converts them into a vector<bool>.
 *  4) Stores them back into Phase2ITChipBitStream objects in an EDM collection.
 *
 * NOTE: This does not parse offset bits vs. data bits or multiple chips per FED.
 *       It simply lumps all nibble-based 16-bit words from each FED ID into one bitstream.
 */
class Phase2ITSLinkUnpacker : public edm::one::EDProducer<> {
public:
  explicit Phase2ITSLinkUnpacker(const edm::ParameterSet& iConfig);
  ~Phase2ITSLinkUnpacker() override = default;

private:
  void produce(edm::Event& iEvent, const edm::EventSetup& iSetup) override;

  // Helper: reconstruct 16 bits from 4 nibbles
  uint16_t get16BitsFromPtr(const unsigned char* buffer, size_t wordIndex) const;

  // Configuration / tokens
  edm::EDGetTokenT<FEDRawDataCollection> fedToken_;
};

// Constructor: read config, declare what we consume/produce
Phase2ITSLinkUnpacker::Phase2ITSLinkUnpacker(const edm::ParameterSet& iConfig)
{
  // Which FEDRawDataCollection to read (the output of Phase2ITSLinkProducer)
  fedToken_ = consumes<FEDRawDataCollection>(
      iConfig.getParameter<edm::InputTag>("InputFED"));

  // We produce a DetSetVector of Phase2ITChipBitStream
  produces<edm::DetSetVector<Phase2ITChipBitStream>>();
}

void Phase2ITSLinkUnpacker::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  edm::Handle<FEDRawDataCollection> fedRawDataHandle;
  iEvent.getByToken(fedToken_, fedRawDataHandle);
  if (!fedRawDataHandle.isValid()) {
    edm::LogError("Phase2ITSLinkUnpacker")
      << "Could not find FEDRawDataCollection product!";
    return;
  }

  auto output = std::make_unique<edm::DetSetVector<Phase2ITChipBitStream>>();
  std::cout << "UNPACKER starts here" << std::endl;
 
  static const int MAX_DTC_ID = 576;
  for (int fedId = 0; fedId < MAX_DTC_ID; ++fedId) {
    const FEDRawData& fedData = fedRawDataHandle->FEDData(fedId);
    if (fedData.size() == 0) {
        std::cout << "skipping fedId : " << fedId << std::endl;
        continue; // no data for this fedId
    }
    size_t totalBytes = fedData.size();
    size_t total16BitWords = totalBytes / 4;

    const unsigned char* ptr = fedData.data();
    std::vector<bool> allBits;
    allBits.reserve(total16BitWords * 16);

    // Optional: for FED 0, also store the raw words for printing
    std::vector<uint16_t> rawWords;
    rawWords.reserve(total16BitWords);

    for (size_t w = 0; w < total16BitWords; ++w) {
      uint16_t word = get16BitsFromPtr(ptr, w);
      rawWords.push_back(word);
      for (int b = 15; b >= 0; --b) {
        bool bitVal = (word >> b) & 0x1;
        allBits.push_back(bitVal);
      }
    }

    // For FED 0, print the raw 16-bit words, eight per line.
if (fedId == 0) {
  std::cout << "Printing raw 32-bit words for FED 0:" << std::endl;
  // Check that we have an even number of 16-bit words.
  if (rawWords.size() % 2 != 0) {
    edm::LogWarning("Phase2ITSLinkUnpacker")
      << "Odd number of 16-bit words; last word will be padded with 0.";
  }
  for (size_t i = 0; i < rawWords.size(); i += 2) {
    uint32_t word32 = (rawWords[i] << 16) | ( (i+1 < rawWords.size()) ? rawWords[i+1] : 0 );
    std::cout << std::setw(8) << std::setfill('0') << std::hex << word32 << std::endl;
  }
  // Reset formatting
  std::cout << std::setfill(' ') << std::dec;
}

    Phase2ITChipBitStream chip(0, allBits);

    edm::DetSet<Phase2ITChipBitStream> detSet(fedId);
    detSet.data.emplace_back(std::move(chip));
    output->insert(detSet);

    std::cout << "Phase2ITSLinkUnpacker"
              << "[Unpacker] FED " << fedId << " -> " 
              << total16BitWords << " words, " 
              << allBits.size() << " bits" << std::endl;
  }
  iEvent.put(std::move(output));
}

// Reconstruct 16-bit word from 4 nibble bytes. 
// Producer stored bits as nib0 -> [15..12], nib1-> [11..8], nib2-> [7..4], nib3-> [3..0].
uint16_t Phase2ITSLinkUnpacker::get16BitsFromPtr(const unsigned char* buffer, size_t wordIndex) const {
  size_t base = wordIndex * 4;
  // each nibble is in the lower 4 bits of the byte
  uint16_t nib0 = (buffer[base + 0] & 0xF); // bits [15..12]
  uint16_t nib1 = (buffer[base + 1] & 0xF); // bits [11..8]
  uint16_t nib2 = (buffer[base + 2] & 0xF); // bits [7..4]
  uint16_t nib3 = (buffer[base + 3] & 0xF); // bits [3..0]

  // Re-assemble the 16-bit value
  uint16_t word = (nib0 << 12) | (nib1 << 8) | (nib2 << 4) | (nib3);
  return word;
}

// Define as a plug-in
DEFINE_FWK_MODULE(Phase2ITSLinkUnpacker);

