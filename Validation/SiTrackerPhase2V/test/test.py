import FWCore.ParameterSet.Config as cms
from DQMServices.Core.DQMEDAnalyzer import DQMEDAnalyzer

from Configuration.Eras.Era_Phase2_cff import Phase2
process = cms.Process('bitstreamDQM',Phase2)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(-1)
)

# Load essential services
process.load('Configuration.StandardSequences.Services_cff')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('Configuration.Geometry.GeometryExtended2026D91Reco_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')

# GlobalTag
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:phase2_realistic_T30', '')

# Input files
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        'file:output_file.root'  # Your input file
    )
)

# Specify correct Cabling Map file
process.load("CondCore.CondDB.CondDB_cfi")
process.CondDB.connect = 'sqlite_file:OTandITDTCCablingMap.db'
process.PoolDBESSource = cms.ESSource("PoolDBESSource",
    process.CondDB,
    DumpStat = cms.untracked.bool(True),
    toGet = cms.VPSet(cms.PSet(
        record = cms.string('TrackerDetToDTCELinkCablingMapRcd'),
        tag = cms.string("DTCCablingMapProducerUserRun")
    ))
)
process.es_prefer_local_cabling = cms.ESPrefer("PoolDBESSource", "")

# Configuration metadata
process.configurationMetadata = cms.untracked.PSet(
    version = cms.untracked.string('$Revision: 1.0 $'),
    annotation = cms.untracked.string('Phase2IT DataRate DQM'),
    name = cms.untracked.string('Phase2ITDataRateDQM')
)

process.Phase2ITValidateDataRate = DQMEDAnalyzer("Phase2ITValidateDataRate",
    # Input collection
    Phase2ITChipBitStream = cms.InputTag("Phase2ITQCoreProducer"),
    
    # DQM folder name
    TopFolderName = cms.string("TrackerPhase2ITDataRateV"),
    
    # Histogram configuration
    #bitStreamSize = cms.PSet(
    #    name = cms.string("bitStreamSize"),
    #    title = cms.string("Bitstream Size per Chip;Bitstream Size [bits];Number of Chips"),
    #    xMax = cms.double(20000.0),
    #    xMin = cms.double(0.0),
    #    nBins = cms.int32(2000)
    #)
)

process.load('DQMServices.Components.DQMEventInfo_cfi')
process.dqmEnv.subSystemFolder = cms.untracked.string('Ph2TkDataRate')

# DQM sequences
process.bitstream_seq = cms.Sequence(process.Phase2ITValidateDataRate)
process.dqm_comm = cms.Sequence(process.dqmEnv)

# Output definition - DQM output
process.DQMoutput = cms.OutputModule("DQMRootOutputModule",
    fileName = cms.untracked.string('/eos/user/l/ldishman/DQMtests/dqm_output.root')
)

process.TFileService = cms.Service("TFileService",
    fileName = cms.string("/eos/user/l/ldishman/DQMtests/tfile_output.root")
)

# Path and EndPath definitions
process.endjob_step = cms.EndPath(process.endOfProcess)
process.DQMoutput_step = cms.EndPath(process.DQMoutput)

# Main processing path
process.p = cms.Path(process.bitstream_seq * process.dqm_comm)

# Schedule
process.schedule = cms.Schedule(process.p, process.endjob_step, process.DQMoutput_step)

