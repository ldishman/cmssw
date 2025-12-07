#include <memory>
#include <string>
#include <vector>
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "DQMServices/Core/interface/DQMEDAnalyzer.h"
#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/Common/interface/DetSetVector.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"
#include "DataFormats/Phase2TrackerDigi/interface/Phase2ITChipBitStream.h"

#include "CondFormats/SiPhase2TrackerObjects/interface/TrackerDetToDTCELinkCablingMap.h"
#include "CondFormats/DataRecord/interface/TrackerDetToDTCELinkCablingMapRcd.h"

#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"

#include "DQM/SiTrackerPhase2/interface/TrackerPhase2DQMUtil.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h" 
#include "TH1F.h"

class Phase2ITValidateDataRate : public DQMEDAnalyzer {
	public:
		// Declare explicit constructor, destructor, and member functions (or overrides) 
		explicit Phase2ITValidateDataRate(const edm::ParameterSet&);
		~Phase2ITValidateDataRate() override;
		void dqmBeginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) override;
		void analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) override;
		void bookHistograms(DQMStore::IBooker& ibooker, edm::Run const& iRun, edm::EventSetup const& iSetup) override;
		static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

	private:
		// Declare other plugin member functions and data organization variables
		void bookLayerHistos(DQMStore::IBooker& ibooker, uint32_t det_it, const std::string& subdir);
		std::vector<std::pair<unsigned int, unsigned int>> knownDTCIdsWithIndex_;
		std::unordered_map<unsigned int, std::vector<uint32_t>> dtcIdToDetIds_;
		std::unordered_map<uint32_t, unsigned int> detIdToDtcId_;
		std::unordered_map<uint32_t, unsigned int> detIdToLayerNum_;
		std::unordered_map<uint32_t, unsigned int> detIdToRingNum_;
		std::unordered_map<uint32_t, TrackerDetToDTCELinkCablingMap::Subdet> detIdToSubDet_;
		std::unordered_map<uint32_t, unsigned int> detIdToNElinks_;
		std::map<unsigned int, size_t> bitStreamSizesbyDTC;

		// Declare slinkMap as <dtcId, slinkId> -> <num_times_filled, totalbitStream>
		std::map<std::pair<int, int>, std::pair<int, int>> slinkMap_;

		// Declare MonitorElement objects (DQM version of histos w/ some metadata)
		MonitorElement* me_bitStreamSize;

		// Declare toString helper for Subdet enum object in cabling map class
		std::string toString(TrackerDetToDTCELinkCablingMap::Subdet subDet) const {
			switch (subDet) {
				case TrackerDetToDTCELinkCablingMap::PXB:    return "PXB";
				case TrackerDetToDTCELinkCablingMap::FPIX_1: return "FPIX_1";
				case TrackerDetToDTCELinkCablingMap::FPIX_2: return "FPIX_2";
				default:                                     return "UNKNOWN";
			}
		}

		// Declare some widely used nums
		int nDTCs = 36;
		int nslinksPerDTC = 16;
		
		// Declare histogram pointers
		TH1F* thist_bitStreamSize = nullptr;
		TH1F* thist_bitStreamSizeModule = nullptr;
		TH1F* thist_occupancyELink = nullptr;
		TH1F* thist_occupancySLink = nullptr;
		TH1F* thist_bitStreamSizeDTC = nullptr;

		// Declare needed index maps for vector histograms
		std::unordered_map<unsigned int, unsigned int> dtcIdToIndex_;
		std::unordered_map<unsigned int, unsigned int> layerNumToIndex_;
		std::map<std::tuple<int, int, int>, unsigned int> sectionToIndex_;
		std::map<std::tuple<int, int>, unsigned int> paperSectionToIndex_;

		// Declare TH1F object vectors
		std::vector<TH1F*> thist_bitStreamSizePerDTC_;
		std::vector<TH1F*> thist_bitStreamSizeSLinkPerDTC_;
		std::vector<TH1F*> thist_bitStreamSizePerLayer_;
		std::vector<TH1F*> thist_bitStreamSizePerSection_;
		std::vector<TH1F*> thist_bitStreamSizePerPaperSection_;

		// Declare other needed configs/inputs/tokens/pointers
		edm::ParameterSet config_;
		const edm::ESGetToken<TrackerDetToDTCELinkCablingMap, TrackerDetToDTCELinkCablingMapRcd> cablingMapToken_;
		const edm::EDGetTokenT<edm::DetSetVector<Phase2ITChipBitStream>> ITChipBitStreamToken_;
		const edm::ESGetToken<TrackerGeometry, TrackerDigiGeometryRecord> geomToken_;
		const edm::ESGetToken<TrackerTopology, TrackerTopologyRcd> topoToken_;
		const TrackerDetToDTCELinkCablingMap* cablingMap_ = nullptr;
		const TrackerGeometry* tkGeom_ = nullptr;
		const TrackerTopology* tTopo_ = nullptr;
};

Phase2ITValidateDataRate::Phase2ITValidateDataRate(const edm::ParameterSet& iConfig)
	: config_(iConfig), 
      	cablingMapToken_(esConsumes<TrackerDetToDTCELinkCablingMap, TrackerDetToDTCELinkCablingMapRcd, edm::Transition::BeginRun>()),
      	ITChipBitStreamToken_(consumes<edm::DetSetVector<Phase2ITChipBitStream>>(iConfig.getParameter<edm::InputTag>("Phase2ITChipBitStream"))),
      	geomToken_(esConsumes<TrackerGeometry, TrackerDigiGeometryRecord, edm::Transition::BeginRun>()), 
      	topoToken_(esConsumes<TrackerTopology, TrackerTopologyRcd, edm::Transition::BeginRun>()) 
      	{
      		edm::LogInfo("Phase2ITValidateDataRate") << ">>> Construct Phase2ITValidateDataRate ";
      	      	
		// Create simple histogram objects
      	      	edm::Service<TFileService> fs;
      	      	thist_bitStreamSize = fs->make<TH1F>("bitStreamSize_direct", "", 2000, 0., 20000.);
      	      	thist_bitStreamSizeModule = fs->make<TH1F>("bitStreamSizeModule_direct", "", 2000, 0., 20000.);
      	      	thist_occupancyELink = fs->make<TH1F>("occupancyOverELinks_direct", "", 60, 0., 3.3);
				thist_occupancySLink = fs->make<TH1F>("occupancyOverSLinks_direct", "", 300, 0., 120000.);
				thist_bitStreamSizeDTC = fs->make<TH1F>("thist_bitStreamSizeDTC_direct", "", 100, 0., 800000.);
		
		// Write setup for DTC Histos
		int num_dtcs = 36;
		int dtcIds[num_dtcs];
		int index = 0;

        // Create array for all valid DTC Ids (11-19, 21-29, 31-39, 41-49)
		for (int i = 0; i <= 3; i ++) {
			int start = 11 + i*10;
			for (int j = start; j < start + 9; j++) {
				dtcIds[index++] = j;
			}
		}

		// Define a value for use below
		double NSlinksPerDTC = 16.0;

		// Create TH1F for each DTC
                thist_bitStreamSizePerDTC_.resize(num_dtcs, nullptr);
				thist_bitStreamSizeSLinkPerDTC_.resize(num_dtcs, nullptr);
                for (int i = 0; i < num_dtcs; i++) {
                        thist_bitStreamSizePerDTC_[i] = fs->make<TH1F>(("thist_bitStreamSizePerDTC_" + std::to_string(dtcIds[i])).c_str(), "", 500, 0., 8000.);
						thist_bitStreamSizeSLinkPerDTC_[i] = fs->make<TH1F>(("thist_bitStreamSizeSLinkPerDTC_" + std::to_string(dtcIds[i])).c_str(), "", NSlinksPerDTC, -0.5, NSlinksPerDTC-0.5);
                        dtcIdToIndex_[dtcIds[i]] = i;
                }

		// Write setup for Layer Histos
		int num_layers = 8;
		int layerNums[num_layers];
		
		// Create array for all valid layer numbers (1-8)
		for (int i = 0; i <= num_layers-1; i++) {
			layerNums[i] = i+1;
		}

		// Create TH1F for each Layer
		thist_bitStreamSizePerLayer_.resize(num_layers, nullptr);
		for (int i = 0; i < num_layers; i++) {
			thist_bitStreamSizePerLayer_[i] = fs->make<TH1F>(("thist_bitStreamSizePerLayer_" + std::to_string(layerNums[i])).c_str(), "", 500, 0., 10000.);
			layerNumToIndex_[layerNums[i]] = i;
		}

		// Write remaining setup for Section Histos (overall needs subDet, layerNum, ringNum)
		int num_rings = 5;
		int ringNums[num_rings] = {1, 2, 3, 4, 5};
		int num_subDets = 3;
		int subDetIdxs[num_subDets] = {0, 1, 2};
		int num_sections = num_subDets * num_rings * num_layers;

		// Create TH1F for each section (subdet -> layer -> ring)
		thist_bitStreamSizePerSection_.resize(num_sections, nullptr);
		int count = 0;

		for (int i = 0; i < num_subDets; i++) {
			for (int j = 0; j < num_layers; j++) {
				for (int k = 0; k < num_rings; k++) {
					thist_bitStreamSizePerSection_[count] = fs->make<TH1F>(("thist_bitStreamSizePerSection_" + toString(static_cast<TrackerDetToDTCELinkCablingMap::Subdet>(subDetIdxs[i])) + "_" + std::to_string(layerNums[j]) + "_" + std::to_string(ringNums[k])).c_str(), "", 500, 0., 10000.);
					sectionToIndex_[{subDetIdxs[i], layerNums[j], ringNums[k]}] = count;
					count++;
				}
			}
		}

		// Write setup for Paper Section Histos (defined by TBPX:L1-L4 TFPX:R1-R4 TEPX:R1-R5)
		int num_paper_TBPX = 4;		// layers
		int num_paper_TFPX = 4;		// rings
		int num_paper_TEPX = 5;		// rings
		int num_paper_sections = num_paper_TBPX + num_paper_TFPX + num_paper_TEPX;     // just counting above defined paper sections
		
		// Resize TH1F paper sections vector for ALL
		thist_bitStreamSizePerPaperSection_.resize(num_paper_sections, nullptr);
		int t = 0;

		// Create TH1F for each paper section in TBPX (L1-L4)
		for (int i = 0; i < num_paper_TBPX; i++) {
			thist_bitStreamSizePerPaperSection_[t] = fs->make<TH1F>(("thist_bitStreamSizePerPaperSection_TBPX_L" + std::to_string(layerNums[i])).c_str(), "", 500, 0., 10000.);
			paperSectionToIndex_[{TrackerDetToDTCELinkCablingMap::PXB, layerNums[i]}] = t;
			t++;
		}
		
		// Create TH1F for each paper section in TFPX (R1-R4)
		for (int i = 0; i < num_paper_TFPX; i++) {
			thist_bitStreamSizePerPaperSection_[t] = fs->make<TH1F>(("thist_bitStreamSizePerPaperSection_TFPX_R" + std::to_string(ringNums[i])).c_str(), "", 500, 0., 10000.);
			paperSectionToIndex_[{TrackerDetToDTCELinkCablingMap::FPIX_1, ringNums[i]}] = t;
			t++;
		}
		
		// Create TH1F for each paper section in TEPX (R1-R5)
		for (int i = 0; i < num_paper_TEPX; i++) {
			thist_bitStreamSizePerPaperSection_[t] = fs->make<TH1F>(("thist_bitStreamSizePerPaperSection_TEPX_R" + std::to_string(ringNums[i])).c_str(), "", 500, 0., 10000.);
			paperSectionToIndex_[{TrackerDetToDTCELinkCablingMap::FPIX_2, ringNums[i]}] = t;
			t++;
		}

        }

Phase2ITValidateDataRate::~Phase2ITValidateDataRate() {
	edm::LogInfo("Phase2ITValidateDataRate") << ">>> Destroy Phase2ITValidateDataRate ";
}

void Phase2ITValidateDataRate::dqmBeginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {
	tkGeom_ = &iSetup.getData(geomToken_);
	tTopo_ = &iSetup.getData(topoToken_);

	cablingMap_ = &iSetup.getData(cablingMapToken_);	// cablingMap from event setup
	knownDTCIdsWithIndex_ = cablingMap_->getKnownDTCIdsWithIndex();
	
	// Build dtcIdToDetIds map (directly from cabling map)
	dtcIdToDetIds_.clear();
	for (const auto& pair : knownDTCIdsWithIndex_) {
		unsigned int dtcId = pair.second;
		dtcIdToDetIds_[dtcId] = cablingMap_->getAllDetIdsForDTCId(dtcId);
		//std::cout << "dtcId: " << dtcId << " \n";
	}

	// Build detIdToDtcId map (by manually undoing dtcIdToDetIds map)
	detIdToDtcId_.clear();
	for (const auto& [dtcId, detIds] : dtcIdToDetIds_) {
		for (auto det_id : detIds) {
			detIdToDtcId_[det_id] = dtcId;
			//std::cout << "det_id: " << det_id << " gives detIdToDtcId_[det_id]: " << detIdToDtcId_[det_id] << " \n";
		}
	}

	// Build detIdToLayerNum map (using detIdToDtcId map for detIds, but getting layerNums from cabling map)
	detIdToLayerNum_.clear();
	for (const auto& [detId, dtcId] : detIdToDtcId_) {
		detIdToLayerNum_[detId] = cablingMap_->detIdToLayerNum(detId);
		//std::cout << "cabling map detIdToLayerNum_[detId] = " << detIdToLayerNum_[detId] << " for detId " << detId << "\n";
	}

	// Build detIdToRingNum map (using detIdToDtcId map for detIds, but getting ringNums from cabling map)
	detIdToRingNum_.clear();
	for (const auto& [detId, dtcId] : detIdToDtcId_) {
		detIdToRingNum_[detId] = cablingMap_->detIdToRingNum(detId);
		//std::cout << "cabling map detIdToRingNum_[detId] = " << detIdToRingNum_[detId] << " for detId " << detId << "\n";
	}

	// Build detIdToSubDet map (using detIdToDtcId map for detIds, but getting subDets from cabling map)
	detIdToSubDet_.clear();
	for (const auto& [detId, dtcId] : detIdToDtcId_) {
		detIdToSubDet_[detId] = cablingMap_->detIdToSubDet(detId);
		//std::cout << "cabling map detIdToSubDet_[detId] = " << detIdToSubDet_[detId] << " with toString val: " << toString(detIdToSubDet_[detId]) << " for detId " << detId << "\n";	// Note this prints 0, 1, or 2 for value, then PXB, FPIX_1, or FPIX_2 for toString(value)
	}

	// Build detIdToNElinks map (using detIdToDtcId map for detIds, but getting nElinks from cabling map)
	detIdToNElinks_.clear();
	for (const auto& [detId, dtcId] : detIdToDtcId_) {
		detIdToNElinks_[detId] = cablingMap_->detIdToNElinks(detId);
		//std::cout << "cabling map detIdToNElinks_[detId] = " << detIdToNElinks_[detId] << " for detId " << detId << "\n";
	}

	// Clear map for <dtc,slink> -> <num_times_filled,totalBitStream for each 36*16 = 576 slinks (36 DTCs, 16 slinks per DTC)
	slinkMap_.clear();

}

void Phase2ITValidateDataRate::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
	edm::Handle<edm::DetSetVector<Phase2ITChipBitStream>> handle;	// handle ~= data
	iEvent.getByToken(ITChipBitStreamToken_, handle);	// retrieve bitstream data

	// Define (successfully matched) module counter for sanity check
	int nModules = 0;

	// Count total number of elinks (as in, add one for each elink per module)
	int num_elinks = 0;

	// Loop over modules in handle
	for (const auto& detset : *handle) {
		size_t bitStreamSizeModule = 0.;
		uint32_t det_id = detset.id;
		//std::cout << "det_id = " << det_id << "\n";

		// Check that the handle's det_id exists as a detId in the detIdToDtcId_ map
		if (detIdToDtcId_.find(det_id) == detIdToDtcId_.end()) {
			std::cout << "Missing DetId: " << det_id << "\n";
			continue;
		}

		//std::cout << "det_id: " << det_id << " \n";
		unsigned int dtcId = detIdToDtcId_.at(det_id);
		//std::cout << "dtcId: " << dtcId << " \n";
		unsigned int idx = dtcIdToIndex_.at(dtcId);
		unsigned int layerNum = detIdToLayerNum_.at(det_id);
		//std::cout << "layerNum: " << layerNum << " \n";
		unsigned int ringNum = detIdToRingNum_.at(det_id);
		//std::cout << "ringNum: " << ringNum << " \n";
		TrackerDetToDTCELinkCablingMap::Subdet subDet = detIdToSubDet_.at(det_id);
		//std::cout << "subDet : " << subDet << "\n";
		//std::cout << "subDet label : " << toString(subDet) << "\n";
		unsigned int nElinks = detIdToNElinks_.at(det_id);
		//std::cout << "nElinks: " << nElinks << " \n";

		unsigned int layerIdx = layerNumToIndex_.at(layerNum);
		auto idx_section = sectionToIndex_.at({subDet, layerNum, ringNum});
		auto idx_paperSection = 0;
		

		// This is not robust, assumes you know the map's structure
		if (toString(subDet)=="PXB" && layerNum<5) {
			idx_paperSection = paperSectionToIndex_.at({subDet, layerNum});
		}
		else if ((toString(subDet)=="FPIX_1" && ringNum<5) || (toString(subDet)=="FPIX_2" && ringNum<6)) {
			idx_paperSection = paperSectionToIndex_.at({subDet, ringNum});
		}

		nModules += 1;
		
		// Loop over chips in current module, get bitStreamSize variable, and fill TH1Fs as needed
		for (const auto& bitStream : detset) {
			size_t bitStreamSize = bitStream.get_bitstream().size();	// This bitStreamSize is the only variable used to Fill
			thist_bitStreamSize->Fill(bitStreamSize);
			me_bitStreamSize->Fill(bitStreamSize);
			thist_bitStreamSizePerDTC_[idx]->Fill(bitStreamSize);
			thist_bitStreamSizePerSection_[idx_section]->Fill(bitStreamSize);
			thist_bitStreamSizePerPaperSection_[idx_paperSection]->Fill(bitStreamSize);
			thist_bitStreamSizePerLayer_[layerIdx]->Fill(bitStreamSize);

			bitStreamSizeModule += bitStreamSize;
		}

		thist_bitStreamSizeModule->Fill(bitStreamSizeModule);

		// Define some constants for E-Link occupancy
		double trigger_rate = 750.0e3;	// Hz
		double bandwidth = 1.28e9;	// bits/s

		for (unsigned int i = 0; i < nElinks; i++) {
			double occupancy = (static_cast<double>(bitStreamSizeModule) * trigger_rate) / (static_cast<double>(nElinks) * bandwidth);
			thist_occupancyELink->Fill(occupancy);
			num_elinks += 1;
		}
		
		// Fill slinkMap in order based on DTC (pseudo-slink-occupancy), and repeat when all slinks full for DTC
		// Count how many modules have already been assigned to this DTC
		int assignedSoFar = 0;
		for (int i = 0; i < nslinksPerDTC; i++) {
			auto it = slinkMap_.find({dtcId, i});
			if (it != slinkMap_.end()) assignedSoFar += it->second.first;
			// std::cout << "Modules assignedSoFar for DTC " << dtcId << " : " << assignedSoFar << " \n";
		}
		
		// Pick the slink in order
		int slinkId = assignedSoFar % 16;
		// std::cout << "SLinkId for module " << det_id << " : " << slinkId << " \n";

		// Update the (actual) map entry (via [])
		auto &entry = slinkMap_[{dtcId, slinkId}];
		entry.first += 1;
		entry.second += bitStreamSizeModule;
		
		bitStreamSizesbyDTC[dtcId] += bitStreamSizeModule;

	} // End module loop

	// Check (successfully matched) module counter
	std::cout << "nModules: " << nModules << " \n";
	
	// Check total number of elinks (adding 1 for every elink attached to each module)
	std::cout << "num elinks: " << num_elinks << " \n";

	// Fill TH1F for bitStreamSize across all DTCs
	for (const auto& [dtcId, totalBitStream] : bitStreamSizesbyDTC) {
		std::cout << "DtcId: " << dtcId << " and totalBitStream: " << totalBitStream << " \n";
		thist_bitStreamSizeDTC->Fill(totalBitStream);
	}

	// Remake array of all valid DTC Ids (11-19, 21-29, 31-39, 41-49)
	// Fix redundancy later
	int dtcIds[nDTCs];
	int index = 0;
	for (int i = 0; i <= 3; i ++) {
		int start = 11 + i*10;
		for (int j = start; j < start + 9; j++) {
			dtcIds[index++] = j;
		}
	}

	// Take filled slinkMap with bitStream info and fill 1 histo with totals
	for (int i = 0; i < nDTCs; i++) {
		for (int j = 0; j < nslinksPerDTC; j++) {
			int totalBitStream = slinkMap_[{dtcIds[i], j}].second;
			// Fill slink histogram with total bitStreamSize for each slink
			thist_occupancySLink->Fill(totalBitStream);
			// std::cout << "DtcId: " << dtcIds[i] << ", slink: " << j << ", totalBitStream: " << totalBitStream << " \n";
			// Fill "histo" for slink distribution over EACH DTC (36 histos)
			thist_bitStreamSizeSLinkPerDTC_[i]->Fill(j, totalBitStream);
			thist_bitStreamSizeSLinkPerDTC_[i]->GetXaxis()->SetTitle("SLink index");
			thist_bitStreamSizeSLinkPerDTC_[i]->GetYaxis()->SetTitle("Total BitStream");
		}
	}

}

void Phase2ITValidateDataRate::bookHistograms(DQMStore::IBooker& ibooker, edm::Run const& iRun, edm::EventSetup const& iSetup) {
	std::string top_folder = config_.getParameter<std::string>("TopFolderName");
	edm::LogInfo("Phase2ITValidateDataRate") << " Booking Histograms in: " << top_folder;

	ibooker.setCurrentFolder(top_folder);

	me_bitStreamSize = ibooker.book1D("bitStreamSize_direct", "Bit Stream Size (direct)", 2000, 0., 20000.);
}

// Function unused for now
void Phase2ITValidateDataRate::bookLayerHistos(DQMStore::IBooker& ibooker, uint32_t det_id, const std::string& subdir) {
	std::string folderName = phase2tkutil::getITHistoId(det_id, tTopo_);

	if (folderName.empty()) {
		edm::LogWarning("Phase2ITValidateDataRate") << ">>>> Invalid histo_id ";
		return;
	}
}

void Phase2ITValidateDataRate::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
	edm::ParameterSetDescription desc;
		edm::ParameterSetDescription psd0;
		psd0.add<std::string>("name", "bitstreamSize");
		psd0.add<std::string>("title", "Bitstream Size per Chip;Bitstream Size [bits];Number of Chips");
		psd0.add<bool>("switch", true);     // What is this? cmsRun yelled at me without it
		psd0.add<double>("xmax", 20000.);
		psd0.add<double>("xmin", 0.);
		psd0.add<int>("NxBins", 2000);
		desc.add<edm::ParameterSetDescription>("bitstreamSize", psd0);
	desc.add<edm::InputTag>("Phase2ITChipBitStream", edm::InputTag("Phase2ITQCoreProducer"));
	desc.add<std::string>("TopFolderName", "TrackerPhase2ITDataRateV");
	descriptions.add("Phase2ITValidateDataRate", desc);
}

DEFINE_FWK_MODULE(Phase2ITValidateDataRate);
