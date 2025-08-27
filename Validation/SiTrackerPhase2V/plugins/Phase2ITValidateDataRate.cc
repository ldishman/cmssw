#include <memory>
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

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

class Phase2ITValidateDataRate : public DQMEDAnalyzer {
	public:
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

		void bookLayerHistos(DQMStore::IBooker& ibooker, uint32_t det_it, const std::string& subdir);
		std::map<std::string, DataRateMEs> layerMEs_;
		std::vector<std::pair<unsigned int, unsigned int>> knownDTCIdsWithIndex_;
		//std::unordered_map<unsigned int, std::vector<uint32_t>> dtcIdToDetIds_;

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
    : config_(iConfig), cablingMapToken_(
          esConsumes<TrackerDetToDTCELinkCablingMap, TrackerDetToDTCELinkCablingMapRcd, edm::Transition::BeginRun>()),
      ITChipBitStreamToken_(consumes<edm::DetSetVector<Phase2ITChipBitStream>>(
          iConfig.getParameter<edm::InputTag>("Phase2ITChipBitStream"))), geomToken_(esConsumes<TrackerGeometry, TrackerDigiGeometryRecord, edm::Transition::BeginRun>()), topoToken_(esConsumes<TrackerTopology, TrackerTopologyRcd, edm::Transition::BeginRun>()) {
	    edm::LogInfo("Phase2ITValidateDataRate") << ">>> Construct Phase2ITValidateDataRate ";
}

Phase2ITValidateDataRate::~Phase2ITValidateDataRate() {
	edm::LogInfo("Phase2ITValidateDataRate") << ">>> Destroy Phase2ITValidateDataRate ";
}

void Phase2ITValidateDataRate::dqmBeginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {
	tkGeom_ = &iSetup.getData(geomToken_);
	tTopo_ = &iSetup.getData(topoToken_);

	cablingMap_ = &iSetup.getData(cablingMapToken_);
	//knownDTCIdsWithIndex_ = cablingMap_->getKnownDTCIdsWithIndex();
}

void Phase2ITValidateDataRate::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
	edm::Handle<edm::DetSetVector<Phase2ITChipBitStream>> handle;
	iEvent.getByToken(ITChipBitStreamToken_, handle);

	if (!handle.isValid()) {
		edm::LogWarning("Phase2ITValidateDataRate") << "No Phase2ITChipBitStream collection found!";
		// The above line should be printed for now, given input file mismatch (solve later)
		return;
	}
}

void Phase2ITValidateDataRate::bookHistograms(DQMStore::IBooker& ibooker, edm::Run const& iRun, edm::EventSetup const& iSetup) {
	std::string top_folder = config_.getParameter<std::string>("TopFolderName");
	edm::LogInfo("Phase2ITValidateDataRate") << " Booking Histograms in: " << top_folder;
}

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
		//psd0.add<bool>("switch", true);     // What is this?
		psd0.add<double>("xmax", 1000.);     // Arbitrary nums right now
		psd0.add<double>("xmin", 0.);
		psd0.add<int>("NxBins", 200);
		desc.add<edm::ParameterSetDescription>("bitstreamSize", psd0);
	// May need some other desc.add statements here, not sure
	desc.add<edm::InputTag>("Phase2ITChipBitStream", edm::InputTag("PixelQCoreProducer"));
	desc.add<std::string>("TopFolderName", "TrackerPhase2ITDataRateV");
	descriptions.add("Phase2ITValidateDataRate", desc);
}

DEFINE_FWK_MODULE(Phase2ITValidateDataRate);
