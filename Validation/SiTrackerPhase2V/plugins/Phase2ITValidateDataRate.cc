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
		// Declare other plugin member / helper functions
		void bookDTCHistos(DQMStore::IBooker& ibooker);
		void bookLayerHistos(DQMStore::IBooker& ibooker);
		void bookSectionHistos(DQMStore::IBooker& ibooker);
		void bookPaperSectionHistos(DQMStore::IBooker& ibooker);
		//void bookLayerHistos(DQMStore::IBooker& ibooker, uint32_t det_it, const std::string& subdir);

		// Declare some data organization maps / vectors
		std::vector<std::pair<unsigned int, unsigned int>> knownDTCIdsWithIndex_;
		std::unordered_map<unsigned int, std::vector<uint32_t>> dtcIdToDetIds_;
		std::unordered_map<uint32_t, unsigned int> detIdToDtcId_;
		std::unordered_map<uint32_t, unsigned int> detIdToLayerNum_;
		std::unordered_map<uint32_t, unsigned int> detIdToRingNum_;
		std::unordered_map<uint32_t, TrackerDetToDTCELinkCablingMap::Subdet> detIdToSubDet_;
		std::unordered_map<uint32_t, unsigned int> detIdToNElinks_;
		std::map<unsigned int, size_t> bitStreamSizesbyDTC_;
		std::map<std::pair<int, int>, std::pair<int, int>> slinkMap_;	// Declare slinkMap as <dtcId, slinkId> -> <num_times_filled, totalbitStream>

		// Declare 'simple' MonitorElement objects (DQM version of histos w/ some metadata)
		MonitorElement* me_bitStreamSizeChip_;
		MonitorElement* me_bitStreamSizeModule_;
		MonitorElement* me_occupancyELink_;
		MonitorElement* me_bitStreamSizeSLink_;
		MonitorElement* me_bitStreamSizeDTC_;

		// Declare 'complex' vector MonitorElement objects
		std::vector<MonitorElement*> mes_bitStreamSizePerDTC_;
		std::vector<MonitorElement*> mes_bitStreamSizeSLinkPerDTC_;
		std::vector<MonitorElement*> mes_bitStreamSizePerLayer_;
		std::vector<MonitorElement*> mes_bitStreamSizePerSection_;
		std::vector<MonitorElement*> mes_bitStreamSizePerPaperSection_;

		// Define toString helper for Subdet enum object in cabling map class
		std::string toString(TrackerDetToDTCELinkCablingMap::Subdet subDet) const {
			switch (subDet) {
				case TrackerDetToDTCELinkCablingMap::PXB:    return "PXB";
				case TrackerDetToDTCELinkCablingMap::FPIX_1: return "FPIX_1";
				case TrackerDetToDTCELinkCablingMap::FPIX_2: return "FPIX_2";
				default:                                     return "UNKNOWN";
			}
		}

		// Declare some geometry-related, globally used nums and arrays
		int nDTCs_ = 36;
		std::vector<int> dtcIds_ = {11,12,13,14,15,16,17,18,19,21,22,23,24,25,26,27,28,29,31,32,33,34,35,36,37,38,39,41,42,43,44,45,46,47,48,49};
		int nslinksPerDTC_ = 16;
		int nLayers_ = 8;
		std::vector<int> layerNums_ = {1,2,3,4,5,6,7,8};
		int nRings_ = 5;
		std::vector<int> ringNums_ = {1, 2, 3, 4, 5};
		int nSubDets_ = 3;
		std::vector<int> subDetIdxs_ = {0, 1, 2};
		int nSections_ = nSubDets_ * nRings_ * nLayers_;
		int nPaperTBPX_ = 4;		// layers
		int nPaperTFPX_ = 4;		// rings
		int nPaperTEPX_ = 5;		// rings
		int nPaperSections_ = nPaperTBPX_ + nPaperTFPX_ + nPaperTEPX_;     // just counting above defined sections from 'the paper'

		// Declare needed index maps for vectors of MEs / histos
		std::unordered_map<unsigned int, unsigned int> dtcIdToIndex_;
		std::unordered_map<unsigned int, unsigned int> layerNumToIndex_;
		std::map<std::tuple<int, int, int>, unsigned int> sectionToIndex_;
		std::map<std::tuple<int, int>, unsigned int> paperSectionToIndex_;

		// Declare THIST histogram pointers
		TH1F* thist_bitStreamSizeChip_ = nullptr;
		TH1F* thist_bitStreamSizeModule_ = nullptr;
		TH1F* thist_occupancyELink_ = nullptr;
		TH1F* thist_bitStreamSizeSLink_ = nullptr;
		TH1F* thist_bitStreamSizeDTC_ = nullptr;

		// Declare THIST TH1F object vectors
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
      	thist_bitStreamSizeChip_ = fs->make<TH1F>("bitStreamSizeChip_direct", "", 2000, 0., 20000.);
      	thist_bitStreamSizeModule_ = fs->make<TH1F>("bitStreamSizeModule_direct", "", 2000, 0., 20000.);
      	thist_occupancyELink_ = fs->make<TH1F>("occupancyOverELinks_direct", "", 60, 0., 3.3);
		thist_bitStreamSizeSLink_ = fs->make<TH1F>("bitStreamSizeSLinks_direct", "", 300, 0., 120000.);
		thist_bitStreamSizeDTC_ = fs->make<TH1F>("thist_bitStreamSizeDTC_direct", "", 100, 0., 800000.);
		
		// Write setup for DTC Histos
		int num_dtcs = 36;
		int dtcIds_[num_dtcs];
		int index = 0;

        // Create array for all valid DTC Ids (11-19, 21-29, 31-39, 41-49)
		for (int i = 0; i <= 3; i ++) {
			int start = 11 + i*10;
			for (int j = start; j < start + 9; j++) {
				dtcIds_[index++] = j;
			}
		}

		// Create TH1F for each DTC
        thist_bitStreamSizePerDTC_.resize(num_dtcs, nullptr);
		thist_bitStreamSizeSLinkPerDTC_.resize(num_dtcs, nullptr);
        for (int i = 0; i < num_dtcs; i++) {
        	thist_bitStreamSizePerDTC_[i] = fs->make<TH1F>(("thist_bitStreamSizePerDTC_" + std::to_string(dtcIds_[i])).c_str(), "", 500, 0., 8000.);
			thist_bitStreamSizeSLinkPerDTC_[i] = fs->make<TH1F>(("thist_bitStreamSizeSLinkPerDTC_" + std::to_string(dtcIds_[i])).c_str(), "", nslinksPerDTC_, -1, nslinksPerDTC_-1);
            //dtcIdToIndex_[dtcIds_[i]] = i;
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
			//layerNumToIndex_[layerNums[i]] = i;
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

	// Build several maps by using detIdToDtcId map for detIds, but getting individual nums from cabling map
	// detIdToLayerNum_, detIdToRingNum_, detIdToSubDet_, detIdToNElinks_
	detIdToLayerNum_.clear();
	detIdToRingNum_.clear();
	detIdToSubDet_.clear();
	detIdToNElinks_.clear();
	for (const auto& [detId, dtcId] : detIdToDtcId_) {
		detIdToLayerNum_[detId] = cablingMap_->detIdToLayerNum(detId);
		//std::cout << "cabling map detIdToLayerNum_[detId] = " << detIdToLayerNum_[detId] << " for detId " << detId << "\n";
		detIdToRingNum_[detId] = cablingMap_->detIdToRingNum(detId);
		//std::cout << "cabling map detIdToRingNum_[detId] = " << detIdToRingNum_[detId] << " for detId " << detId << "\n";
		detIdToSubDet_[detId] = cablingMap_->detIdToSubDet(detId);
		//std::cout << "cabling map detIdToSubDet_[detId] = " << detIdToSubDet_[detId] << " with toString val: " << toString(detIdToSubDet_[detId]) << " for detId " << detId << "\n";	// Note this prints 0, 1, or 2 for value, then PXB, FPIX_1, or FPIX_2 for toString(value)
		detIdToNElinks_[detId] = cablingMap_->detIdToNElinks(detId);
		//std::cout << "cabling map detIdToNElinks_[detId] = " << detIdToNElinks_[detId] << " for detId " << detId << "\n";
	}

	// Clear map for <dtc,slink> -> <num_times_filled,totalBitStream for each 36*16 = 576 slinks (36 DTCs, 16 slinks per DTC)
	slinkMap_.clear();

	// Build dtcIdToIndex map for histogram access / filling
	dtcIdToIndex_.clear();
	for (int i = 0; i < nDTCs_; i++) {
        dtcIdToIndex_[dtcIds_[i]] = i;
    }

	// Build layerNumToIndex_ map for histogram access / filling
	layerNumToIndex_.clear();
	for (int i = 0; i < nLayers_; i++) {
		layerNumToIndex_[layerNums_[i]] = i;
	}

	// Build sectionToIndex_ map for histogram access / filling
	sectionToIndex_.clear();
	int count = 0;
	for (int i = 0; i < nSubDets_; i++) {
		for (int j = 0; j < nLayers_; j++) {
			for (int k = 0; k < nRings_; k++) {
				sectionToIndex_[{subDetIdxs_[i], layerNums_[j], ringNums_[k]}] = count;
				count++;
			}
		}
	}

	// Build paperSectionToIndex_ for histogram access / filling (w/ 3 loops below)
	paperSectionToIndex_.clear();
	int counter = 0;

	for (int i = 0; i < nPaperTBPX_; i++) {		// For each paper section in TBPX (L1-L4)
		paperSectionToIndex_[{TrackerDetToDTCELinkCablingMap::PXB, layerNums_[i]}] = counter;
		counter++;
	}
	for (int i = 0; i < nPaperTFPX_; i++) {		// For each paper section in TFPX (R1-R4)
		paperSectionToIndex_[{TrackerDetToDTCELinkCablingMap::FPIX_1, ringNums_[i]}] = counter;
		counter++;
	}
	for (int i = 0; i < nPaperTEPX_; i++) {		// For each paper section in TEPX (R1-R5)
		paperSectionToIndex_[{TrackerDetToDTCELinkCablingMap::FPIX_2, ringNums_[i]}] = counter;
		counter++;
	}

}

void Phase2ITValidateDataRate::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
	edm::Handle<edm::DetSetVector<Phase2ITChipBitStream>> handle;	// handle ~= data
	iEvent.getByToken(ITChipBitStreamToken_, handle);	// retrieve bitstream data

	// Define some sanity check counters
	int num_modules = 0;		// (successfully matched) modules
	int num_elinks = 0;		// total number of elinks (as in, add one for each elink per module)

	// Loop over modules in handle
	for (const auto& detset : *handle) {
		size_t bitStreamSizeModule = 0.;
		uint32_t det_id = detset.id;

		// Check that the handle's det_id exists as a detId in the detIdToDtcId_ map
		if (detIdToDtcId_.find(det_id) == detIdToDtcId_.end()) {
			std::cout << "Missing DetId: " << det_id << "\n";
			continue;
		}

		// Find geometric / location quantities after ensuring det_id is valid
		// Each det_id has: dtcId, layerNum, ringNum, subDet, nElinks
		unsigned int dtcId = detIdToDtcId_.at(det_id);
		unsigned int layerNum = detIdToLayerNum_.at(det_id);
		unsigned int ringNum = detIdToRingNum_.at(det_id);
		TrackerDetToDTCELinkCablingMap::Subdet subDet = detIdToSubDet_.at(det_id);		// note: subDet != toString(subDet)
		unsigned int nElinks = detIdToNElinks_.at(det_id);

		// Find indices corresponding to geometric / location quantities
		unsigned int dtcIdx = dtcIdToIndex_.at(dtcId);
		unsigned int layerIdx = layerNumToIndex_.at(layerNum);
		auto sectionIdx = sectionToIndex_.at({subDet, layerNum, ringNum});
		auto paperSectionIdx = 0;

		// Find indices corresponding to subDet (tricky, used enum type in cabling map)
		if (toString(subDet)=="PXB" && layerNum<5) {
			paperSectionIdx = paperSectionToIndex_.at({subDet, layerNum});
		}
		else if ((toString(subDet)=="FPIX_1" && ringNum<5) || (toString(subDet)=="FPIX_2" && ringNum<6)) {
			paperSectionIdx = paperSectionToIndex_.at({subDet, ringNum});
		}
		
		// Loop over chips in current module
		for (const auto& bitStream : detset) {
			size_t bitStreamSize = bitStream.get_bitstream().size();	// bitStreamSize is always at chip level by default!

			// Fill TFileService Histos at each-chip level
			thist_bitStreamSizeChip_->Fill(bitStreamSize);
			thist_bitStreamSizePerDTC_[dtcIdx]->Fill(bitStreamSize);
			thist_bitStreamSizePerSection_[sectionIdx]->Fill(bitStreamSize);
			thist_bitStreamSizePerPaperSection_[paperSectionIdx]->Fill(bitStreamSize);
			thist_bitStreamSizePerLayer_[layerIdx]->Fill(bitStreamSize);

			// Fill DQM Histos at each-chip level
			me_bitStreamSizeChip_->Fill(bitStreamSize);
			mes_bitStreamSizePerDTC_[dtcIdx]->Fill(bitStreamSize);
			mes_bitStreamSizePerSection_[sectionIdx]->Fill(bitStreamSize);
			mes_bitStreamSizePerPaperSection_[paperSectionIdx]->Fill(bitStreamSize);
			mes_bitStreamSizePerLayer_[layerIdx]->Fill(bitStreamSize);

			bitStreamSizeModule += bitStreamSize;
		} // End loop over chips in current module

		// Fill histos at each-module level
		thist_bitStreamSizeModule_->Fill(bitStreamSizeModule);
		me_bitStreamSizeModule_->Fill(bitStreamSizeModule);

		// Define E-Link occupancy w/ proper constants and fill histos
		double trigger_rate = 750.0e3;	// Hz
		double bandwidth = 1.28e9;	// bits/s
		for (unsigned int i = 0; i < nElinks; i++) {
			double occupancy = (static_cast<double>(bitStreamSizeModule) * trigger_rate) / (static_cast<double>(nElinks) * bandwidth);
			thist_occupancyELink_->Fill(occupancy);
			me_occupancyELink_->Fill(occupancy);
			num_elinks += 1;		// Add to sanity-check counter
		}
		
		// NEXT: Fill slinkMap in order based on DTC (pseudo-slink-occupancy), and repeat when all slinks full for DTC
		// Recall: slinkMap looks like <dtcId, slinkId> -> <num_times_filled, totalbitStream>

		// First: Count how many modules have already been assigned to this DTC, hold info in small scope variable (assignedSoFar)
		int assignedSoFar = 0;
		for (int i = 0; i < nslinksPerDTC_; i++) {
			auto it = slinkMap_.find({dtcId, i});
			if (it != slinkMap_.end()) assignedSoFar += it->second.first;	// it->second.first ~= num_times_filled (by module)
			// std::cout << "Modules assignedSoFar for DTC " << dtcId << " : " << assignedSoFar << " \n";
		}

		// Second: Pick the slink in order (chooses slot filled least-recently)
		int slinkId = assignedSoFar % 16;
		// std::cout << "SLinkId for module " << det_id << " : " << slinkId << " \n";

		// Third: Update the (actual) map entry (via []) after choosing the right slot
		auto &entry = slinkMap_[{dtcId, slinkId}];
		entry.first += 1;	// add 1 to num_times_filled (by module)
		entry.second += bitStreamSizeModule;	// add this module's bitstream to the totalBitStream for this {dtcId, slinkId} pair

		// Fill map with totalBitStream per DTC for later histo
		bitStreamSizesbyDTC_[dtcId] += bitStreamSizeModule;

		num_modules += 1;		// Add to sanity-check counter

	} // End loop over modules in handle

	// Fill histo for bitStreamSize distributed across each DTC using map made above
	for (const auto& [dtcId, totalBitStream] : bitStreamSizesbyDTC_) {
		std::cout << "DtcId: " << dtcId << " and totalBitStream: " << totalBitStream << " \n";
		thist_bitStreamSizeDTC_->Fill(totalBitStream);
		me_bitStreamSizeDTC_->Fill(totalBitStream);
	}

	// Loop over each slink in each DTC
	for (int i = 0; i < nDTCs_; i++) {
		for (int j = 0; j < nslinksPerDTC_; j++) {
			int totalBitStream = slinkMap_[{dtcIds_[i], j}].second;		// this is totalBitStream per current slink

			// Fill 1 histo with total bitStreamSize for each slink
			thist_bitStreamSizeSLink_->Fill(totalBitStream);
			me_bitStreamSizeSLink_->Fill(totalBitStream);
			// std::cout << "DtcId: " << dtcIds_[i] << ", slink: " << j << ", totalBitStream: " << totalBitStream << " \n";

			// Fill 36 "histos" for slink distribution over EACH DTC
			thist_bitStreamSizeSLinkPerDTC_[i]->Fill(j, totalBitStream);
			mes_bitStreamSizeSLinkPerDTC_[i]->Fill(j, totalBitStream);
		}
	}

	// Check counters
	std::cout << "num_modules: " << num_modules << " \n";
	std::cout << "num_elinks: " << num_elinks << " \n";

}

void Phase2ITValidateDataRate::bookHistograms(DQMStore::IBooker& ibooker, edm::Run const& iRun, edm::EventSetup const& iSetup) {
	std::string top_folder = config_.getParameter<std::string>("TopFolderName");
	edm::LogInfo("Phase2ITValidateDataRate") << " Booking Histograms in: " << top_folder;

	ibooker.setCurrentFolder(top_folder);

	me_bitStreamSizeChip_ = ibooker.book1D("bitStreamSizeChip_direct", "Bit Stream Size Chip (direct)", 2000, 0., 20000.);
	me_bitStreamSizeModule_ = ibooker.book1D("bitStreamSizeModule_direct", "Bit Stream Size Module (direct)", 2000, 0., 20000.);
	me_occupancyELink_ = ibooker.book1D("occupancyELink_direct", "Bit Stream Size ELink (direct)", 60, 0., 3.3);
	me_bitStreamSizeSLink_ = ibooker.book1D("bitStreamSizeSLink_direct", "Bit Stream Size SLink (direct)", 300, 0., 120000.);
	me_bitStreamSizeDTC_ = ibooker.book1D("bitStreamSizeDTC_direct", "Bit Stream Size DTC (direct)", 100, 0., 800000.);

	bookDTCHistos(ibooker);
	bookLayerHistos(ibooker);
	bookSectionHistos(ibooker);
	bookPaperSectionHistos(ibooker);
}

void Phase2ITValidateDataRate::bookDTCHistos(DQMStore::IBooker& ibooker) {
	mes_bitStreamSizePerDTC_.resize(nDTCs_, nullptr);
	mes_bitStreamSizeSLinkPerDTC_.resize(nDTCs_, nullptr);

	// Create DQM Histo(s) for each DTC
    for (int i = 0; i < nDTCs_; i++) {
        mes_bitStreamSizePerDTC_[i] = ibooker.book1D(("bitStreamSizePerDTC_" + std::to_string(dtcIds_[i])).c_str(), "", 500, 0., 8000.);
		mes_bitStreamSizeSLinkPerDTC_[i] = ibooker.book1D(("bitStreamSizeSLinkPerDTC_" + std::to_string(dtcIds_[i])).c_str(), "", nslinksPerDTC_, -1, nslinksPerDTC_-1);
		mes_bitStreamSizeSLinkPerDTC_[i]->getTH1()->GetXaxis()->SetTitle("SLink index");
		mes_bitStreamSizeSLinkPerDTC_[i]->getTH1()->GetYaxis()->SetTitle("Total BitStream");
    }
}

void Phase2ITValidateDataRate::bookLayerHistos(DQMStore::IBooker& ibooker) {
	mes_bitStreamSizePerLayer_.resize(nLayers_, nullptr);

	// Create DQM Histo(s) for each Layer
	for (int i = 0; i < nLayers_; i++) {
		mes_bitStreamSizePerLayer_[i] = ibooker.book1D(("bitStreamSizePerLayer_" + std::to_string(layerNums_[i])).c_str(), "", 500, 0., 10000.);
	}
}

void Phase2ITValidateDataRate::bookSectionHistos(DQMStore::IBooker& ibooker) {
	mes_bitStreamSizePerSection_.resize(nSections_, nullptr);
	
	int count = 0;
	// Create DQM Histo(s) for each section (subdet -> layer -> ring)
	for (int i = 0; i < nSubDets_; i++) {
		for (int j = 0; j < nLayers_; j++) {
			for (int k = 0; k < nRings_; k++) {
				mes_bitStreamSizePerSection_[count] = ibooker.book1D(("bitStreamSizePerSection_" + toString(static_cast<TrackerDetToDTCELinkCablingMap::Subdet>(subDetIdxs_[i])) + "_" + std::to_string(layerNums_[j]) + "_" + std::to_string(ringNums_[k])).c_str(), "", 500, 0., 10000.);
				count++;
			}
		}
	}
}

void Phase2ITValidateDataRate::bookPaperSectionHistos(DQMStore::IBooker& ibooker) {
	// Resize DQM Histo(s) paper sections vector for ALL of below loops
	mes_bitStreamSizePerPaperSection_.resize(nPaperSections_, nullptr);

	int count = 0;

	// Create DQM Histo(s) for each paper section in TBPX (L1-L4)
	for (int i = 0; i < nPaperTBPX_; i++) {
		mes_bitStreamSizePerPaperSection_[count] = ibooker.book1D(("bitStreamSizePerPaperSection_TBPX_L" + std::to_string(layerNums_[i])).c_str(), "", 500, 0., 10000.);
		count++;
	}
	
	// Create DQM Histo(s) for each paper section in TFPX (R1-R4)
	for (int i = 0; i < nPaperTFPX_; i++) {
		mes_bitStreamSizePerPaperSection_[count] = ibooker.book1D(("bitStreamSizePerPaperSection_TFPX_R" + std::to_string(ringNums_[i])).c_str(), "", 500, 0., 10000.);
		count++;
	}
	
	// Create DQM Histo(s) for each paper section in TEPX (R1-R5)
	for (int i = 0; i < nPaperTEPX_; i++) {
		mes_bitStreamSizePerPaperSection_[count] = ibooker.book1D(("bitStreamSizePerPaperSection_TEPX_R" + std::to_string(ringNums_[i])).c_str(), "", 500, 0., 10000.);
		count++;
	}
}

// Function unused for now
//void Phase2ITValidateDataRate::bookLayerHistos(DQMStore::IBooker& ibooker, uint32_t det_id, const std::string& subdir) {
//	std::string folderName = phase2tkutil::getITHistoId(det_id, tTopo_);
//
//	if (folderName.empty()) {
//		edm::LogWarning("Phase2ITValidateDataRate") << ">>>> Invalid histo_id ";
//		return;
//	}
//}

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
