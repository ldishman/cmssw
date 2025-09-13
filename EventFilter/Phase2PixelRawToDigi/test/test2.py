import FWCore.ParameterSet.Config as cms

# Use the appropriate Phase-2 era
from Configuration.Eras.Era_Phase2C17I13M9_cff import Phase2C17I13M9
process = cms.Process('UNPACK', Phase2C17I13M9)

# Standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('Configuration.Geometry.GeometryExtended2026D91Reco_cff')  # Needed for cabling map
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

# Input file (output from Phase2ITSLinkProducer)
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        "file:output_file.root"  # Replace with your Packer's output file
    )
)

# GlobalTag - Must match what was used in the Packer's production
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:phase2_realistic_T30', '')

# Load cabling map from database (same as in your original config)
process.load("CondCore.CondDB.CondDB_cfi")
process.CondDB.connect = 'sqlite_file:OTandITDTCCablingMap.db'
process.PoolDBESSource = cms.ESSource("PoolDBESSource",
    process.CondDB,
    DumpStat = cms.untracked.bool(True),
    toGet = cms.VPSet(
        cms.PSet(
            record = cms.string('TrackerDetToDTCELinkCablingMapRcd'),
            tag = cms.string("DTCCablingMapProducerUserRun")
        )
    ),
)
process.es_prefer_local_cabling = cms.ESPrefer("PoolDBESSource", "")

# Configure the Unpacker
process.Unpacker = cms.EDProducer(
    'Phase2ITSLinkUnpacker',
    InputFED = cms.InputTag("Packer")  # Must match Packer's module label
)

# Output configuration: Save the unpacked Phase2ITChipBitStream
process.FEVTDEBUGoutput = cms.OutputModule("PoolOutputModule",
    fileName = cms.untracked.string('unpacker_output.root'),
    outputCommands = cms.untracked.vstring(
        'keep *_Phase2*_*_*',  # Keep unpacked data
        'keep *_Phase2*_*_*'   # Optionally keep original FED data
    )
)

# Path and schedule
process.unpack_step = cms.Path(process.Unpacker)
process.output_step = cms.EndPath(process.FEVTDEBUGoutput)
process.schedule = cms.Schedule(process.unpack_step, process.output_step)
