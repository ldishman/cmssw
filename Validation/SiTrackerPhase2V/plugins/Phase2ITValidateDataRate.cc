#include <memory>
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
		// Declare explicit constructor, destructor, member functions (or overrides) 
		explicit Phase2ITValidateDataRate(const edm::ParameterSet&);
		~Phase2ITValidateDataRate() override;
		void dqmBeginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) override;
		void analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) override;
		void bookHistograms(DQMStore::IBooker& ibooker, edm::Run const& iRun, edm::EventSetup const& iSetup) override;
		static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

	private:
		struct DataRateMEs {
			MonitorElement* bitstreamSize = nullptr;
			// MonitorElement* myHistoVar2 = nullptr;
		};

		// Test histogram pointers
		TH1F* thist_bitStreamSize = nullptr;
		TH1F* thist_bitStreamSizeModule = nullptr;
		TH1F* thist_bitStreamSizeDTC = nullptr;

		// Declare other plugin member functions/variables
		void bookLayerHistos(DQMStore::IBooker& ibooker, uint32_t det_it, const std::string& subdir);
		std::map<std::string, DataRateMEs> layerMEs_;
		std::vector<std::pair<unsigned int, unsigned int>> knownDTCIdsWithIndex_;
		std::unordered_map<unsigned int, std::vector<uint32_t>> dtcIdToDetIds_;
		std::unordered_map<unsigned int, unsigned int> detIdToDtcId_;
		std::map<unsigned int, size_t> bitStreamSizesbyDTC;

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
      	      	// Test histogram objects
      	      	edm::Service<TFileService> fs;
      	      	thist_bitStreamSize = fs->make<TH1F>("bitStreamSize_direct", "", 2000, 0., 20000.);
      	      	thist_bitStreamSizeModule = fs->make<TH1F>("bitStreamSizeModule_direct", "", 2000, 0., 20000.);
		thist_bitStreamSizeDTC = fs->make<TH1F>("thist_bitStreamSizeDTC_direct", "", 100, 0., 1000.);
      	}

Phase2ITValidateDataRate::~Phase2ITValidateDataRate() {
	edm::LogInfo("Phase2ITValidateDataRate") << ">>> Destroy Phase2ITValidateDataRate ";
}

void Phase2ITValidateDataRate::dqmBeginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {
	tkGeom_ = &iSetup.getData(geomToken_);
	tTopo_ = &iSetup.getData(topoToken_);

	cablingMap_ = &iSetup.getData(cablingMapToken_);	// cablingMap from event setup
	knownDTCIdsWithIndex_ = cablingMap_->getKnownDTCIdsWithIndex();

	dtcIdToDetIds_.clear();
	for (const auto& pair : knownDTCIdsWithIndex_) {
		unsigned int dtcId = pair.second;
		dtcIdToDetIds_[dtcId] = cablingMap_->getAllDetIdsForDTCId(dtcId);
		//std::cout << "dtcId: " << dtcId << " \n";
	}

	detIdToDtcId_.clear();
	for (const auto& [dtcId, detIds] : dtcIdToDetIds_) {
		for (auto det_id : detIds) {
			detIdToDtcId_[det_id] = dtcId;
			//std::cout << "det_id: " << det_id << "gives detIdToDtcId_[det_id]: " << detIdToDtcId_[det_id] << " \n";
		}
	}

}

void Phase2ITValidateDataRate::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
	edm::Handle<edm::DetSetVector<Phase2ITChipBitStream>> handle;	// handle ~= data
	iEvent.getByToken(ITChipBitStreamToken_, handle);	// retrieve bitstream data

	//for (const auto& pair : knownDTCIdsWithIndex_) {
	//	unsigned int dtcIndex = pair.first;
	//	unsigned int dtcId = pair.second;
	//	auto& det_ids = dtcIdToDetIds_[dtcId];

	//	if (dtcIndex == 0) {
	//		std::cout << "knownDTCIdsWithIndex_ size is: " << knownDTCIdsWithIndex_.size() << "\n";
	//		std::cout << "dtcIdToDetIds_ size is: " << dtcIdToDetIds_.size() << "\n";
	//		std::cout << "det_ids size is: " << det_ids.size() << "\n";
	//	}

	//	size_t bitStreamSizeDTC = 0.;

	//	for (const auto& det_id: det_ids) {
	//		auto found_det_id = handle->find(det_id);
	//		const edm::DetSet<Phase2ITChipBitStream>& detSet = *found_det_id;

	//		//if (found_det_id == handle->end()) {
	//		//	throw cms::Exception("BitstreamToRawProducer") << "Could not find detId from the inputs";
	//		//}
	//		
	//		size_t bitStreamSizeModule = 0.;

	//		if (found_det_id != handle->end()) {
	//			for(const auto& chip: detSet) {
	//				std::vector<bool> chipBitstream = chip.get_bitstream();
	//				unsigned int bitStreamSize = chipBitstream.size();
	//				bitStreamSizeModule += bitStreamSize;
	//			}
	//		}

	//		bitStreamSizeDTC += bitStreamSizeModule;
	//	}
	//	
	//	thist_bitStreamSizeDTC->Fill(bitStreamSizeDTC);
	//	
	//}

	for (const auto& detset : *handle) {
		size_t bitStreamSizeModule = 0.;
		unsigned int det_id = detset.id;

		if (detIdToDtcId_.find(det_id) == detIdToDtcId_.end()) continue;

		auto dtcLinkPair = cablingMap_->detIdToDTCELinkId(det_id);
		auto it = dtcLinkPair.first;
		//if (it == dtcLinkPair.second) continue;  // double-check empty range
		unsigned int dtcId = it->first;

		//unsigned int dtcId = detIdToDtcId_.at(det_id);
		//unsigned int dtcId = cablingMap_->detIdToDTCELinkId(det_id).first;
		//std::cout << "dtcId: " << dtcId << " \n";
		//std::cout << "det_id: " << det_id << " \n";
		
		for (const auto& bitStream : detset) {
			size_t bitStreamSize = bitStream.get_bitstream().size();
			thist_bitStreamSize->Fill(bitStreamSize);
			bitStreamSizeModule += bitStreamSize;
		}

		thist_bitStreamSizeModule->Fill(bitStreamSizeModule);
		bitStreamSizesbyDTC[dtcId] += bitStreamSizeModule;

	}

	for (const auto& [dtcId, totalBitStream] : bitStreamSizesbyDTC) {
		std::cout << "DtcId: " << dtcId << " and totalBitStream: " << totalBitStream << " \n";
		thist_bitStreamSizeDTC->Fill(totalBitStream);
	}

	//for (const auto& detset : *handle) {
	//	for (const auto& chip : detset) {
	//		bitstreamSize_->Fill(bitstreamSize);
	//	}
	//}
}

void Phase2ITValidateDataRate::bookHistograms(DQMStore::IBooker& ibooker, edm::Run const& iRun, edm::EventSetup const& iSetup) {
	std::string top_folder = config_.getParameter<std::string>("TopFolderName");
	edm::LogInfo("Phase2ITValidateDataRate") << " Booking Histograms in: " << top_folder;

	ibooker.setCurrentFolder(top_folder);
	//ibooker.setCurrentFolder("TrackerPhase2ITDataRateV");
	
	//bitstreamSize_ = ibooker.book1D("bitstreamSize", "Bitstream size;Size [bits];Entries", 200, 0., 1000.);
	//for (auto const& det_u : tkGeom_->detUnits()) {
	//	if (!(det_u->subDetector() == GeomDetEnumerators::SubDetector::P2PXB ||
	//		det_u->subDetector() == GeomDetEnumerators::SubDetector::P2PXEC))
	//		continue; // continue if not Pixel
	//	uint32_t detId_raw = det_u->geographicalId().rawId();
	//	bookLayerHistos(ibooker, detId_raw, top_folder);
	//}
}

void Phase2ITValidateDataRate::bookLayerHistos(DQMStore::IBooker& ibooker, uint32_t det_id, const std::string& subdir) {
	std::string folderName = phase2tkutil::getITHistoId(det_id, tTopo_);

	if (folderName.empty()) {
		edm::LogWarning("Phase2ITValidateDataRate") << ">>>> Invalid histo_id ";
		return;
	}

	//if (layerMEs_.find(folderName) == layerMEs_.end()) {
	//	ibooker.cd();
	//	ibooker.setCurrentFolder(subdir + '/' + folderName);
	//	edm::LogInfo("Phase2ITValidateDataRate") << " Booking Histograms in: " << subdir + '/' + folderName;

	//	DataRateMEs local_mes;
	//	local_mes.bitstreamSize = phase2tkutil::book1DFromPSet(config_.getParameter<edm::ParameterSet>("bitstreamSize"), ibooker);
	//	layerMEs_.emplace(folderName, local_mes);
	//}
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
	// May need some other desc.add statements here, not sure
	desc.add<edm::InputTag>("Phase2ITChipBitStream", edm::InputTag("Phase2ITQCoreProducer"));
	desc.add<std::string>("TopFolderName", "TrackerPhase2ITDataRateV");
	descriptions.add("Phase2ITValidateDataRate", desc);
}

DEFINE_FWK_MODULE(Phase2ITValidateDataRate);
