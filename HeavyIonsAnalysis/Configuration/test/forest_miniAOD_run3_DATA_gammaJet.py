### HiForest Configuration
# Input: miniAOD
# Type: data

import FWCore.ParameterSet.Config as cms
from Configuration.Eras.Era_Run3_pp_on_PbPb_2023_cff import Run3_pp_on_PbPb_2023
process = cms.Process('HiForest',Run3_pp_on_PbPb_2023)

###############################################################################

# HiForest info
process.load("HeavyIonsAnalysis.EventAnalysis.HiForestInfo_cfi")
process.HiForestInfo.info = cms.vstring("HiForest, miniAOD, 132X, data")

# import subprocess, os
# version = subprocess.check_output(
#     ['git', '-C', os.path.expandvars('$CMSSW_BASE/src'), 'describe', '--tags'])
# if version == '':
#     version = 'no git info'
# process.HiForestInfo.HiForestVersion = cms.string(version)

###############################################################################

# input files
process.source = cms.Source("PoolSource",
    duplicateCheckMode = cms.untracked.string("noDuplicateCheck"),
    fileNames = cms.untracked.vstring(
        '/store/hidata/HIRun2023A/HIPhysicsRawPrime0/MINIAOD/PromptReco-v2/000/375/790/00000/56ad580f-b228-4f3c-b8e3-17f9d95c7654.root'
    ), 
)

#only accessible from lxplus?
#import FWCore.PythonUtilities.LumiList as LumiList
#process.source.lumisToProcess = LumiList.LumiList(filename = '/eos/user/c/cmsdqm/www/CAF/certification/Collisions23HI/Cert_Collisions2023HI_374288_375823_Golden.json').getVLuminosityBlockRange()

# number of events to process, set to -1 to process all events
process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(100)
    )

###############################################################################

# load Global Tag, geometry, etc.
process.load('Configuration.Geometry.GeometryDB_cff')
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load('FWCore.MessageService.MessageLogger_cfi')


from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '132X_dataRun3_Prompt_v7', '')
process.HiForestInfo.GlobalTagLabel = process.GlobalTag.globaltag

###############################################################################

# Define centrality binning
process.load("RecoHI.HiCentralityAlgos.CentralityBin_cfi")
process.centralityBin.Centrality = cms.InputTag("hiCentrality")
process.centralityBin.centralityVariable = cms.string("HFtowers")

###############################################################################

# root output
process.TFileService = cms.Service("TFileService",
    fileName = cms.string("HiForestMiniAOD.root"))

# # edm output for debugging purposes
# process.output = cms.OutputModule(
#     "PoolOutputModule",
#     fileName = cms.untracked.string('HiForestEDM.root'),
#     outputCommands = cms.untracked.vstring(
#         'keep *',
#         )
#     )

# process.output_path = cms.EndPath(process.output)

###############################################################################

# event analysis
process.load('HeavyIonsAnalysis.EventAnalysis.hltanalysis_cfi')
process.load('HeavyIonsAnalysis.EventAnalysis.hievtanalyzer_data_cfi')
process.load('HeavyIonsAnalysis.EventAnalysis.hltanalysis_cfi')
process.load('HeavyIonsAnalysis.EventAnalysis.skimanalysis_cfi')
process.load('HeavyIonsAnalysis.EventAnalysis.hltobject_cfi')
process.load('HeavyIonsAnalysis.EventAnalysis.l1object_cfi')

#process.hiEvtAnalyzer.doCentrality = cms.bool(False)
#process.hiEvtAnalyzer.doHFfilters = cms.bool(False)

from HeavyIonsAnalysis.EventAnalysis.hltobject_cfi import trigger_list_data_2023_skimmed
process.hltobject.triggerNames = trigger_list_data_2023_skimmed

process.load('HeavyIonsAnalysis.EventAnalysis.particleFlowAnalyser_cfi')
################################
# electrons, photons, muons
process.load('HeavyIonsAnalysis.EGMAnalysis.ggHiNtuplizer_cfi')
process.ggHiNtuplizer.doMuons = cms.bool(False)
process.load("TrackingTools.TransientTrack.TransientTrackBuilder_cfi")
################################
# jet reco sequence
process.load('HeavyIonsAnalysis.JetAnalysis.akCs4PFJetSequence_pponPbPb_data_cff')
process.load('HeavyIonsAnalysis.JetAnalysis.akPu4CaloJetSequence_pponPbPb_data_cff')
process.akPu4CaloJetAnalyzer.doHiJetID = True
################################
# tracks
process.load("HeavyIonsAnalysis.TrackAnalysis.TrackAnalyzers_cff")
# muons (FTW)
process.load("HeavyIonsAnalysis.MuonAnalysis.unpackedMuons_cfi")
process.load("HeavyIonsAnalysis.MuonAnalysis.muonAnalyzer_cfi")
###############################################################################

# ZDC RecHit Producer
process.load('HeavyIonsAnalysis.ZDCAnalysis.QWZDC2018Producer_cfi')
process.load('HeavyIonsAnalysis.ZDCAnalysis.QWZDC2018RecHit_cfi')
process.load('HeavyIonsAnalysis.ZDCAnalysis.zdcanalyzer_cfi')

process.zdcdigi.SOI = cms.untracked.int32(2)
process.zdcanalyzer.doZDCRecHit = False
process.zdcanalyzer.doZDCDigi = True
process.zdcanalyzer.zdcRecHitSrc = cms.InputTag("QWzdcreco")
process.zdcanalyzer.zdcDigiSrc = cms.InputTag("hcalDigis", "ZDC")
process.zdcanalyzer.calZDCDigi = False
process.zdcanalyzer.verbose = False
process.zdcanalyzer.nZdcTs = cms.int32(6)


###############################################################################
# main forest sequence
process.forest = cms.Path(
    process.HiForestInfo +
    process.centralityBin +
    process.hiEvtAnalyzer +
    process.hltanalysis +
    # process.hltobject +  # HLT objects can be bulky; enable if needed for specific studies
    # process.l1object +   # L1 objects can be bulky; enable if needed for specific studies
    # process.trackSequencePbPb + # Track sequence removed to align with MC gammaJet focus
    #process.particleFlowAnalyser +
    process.ggHiNtuplizer #+
    #process.zdcdigi +
    #process.QWzdcreco +
    # process.zdcanalyzer + # ZDC analyzer removed to align with MC gammaJet focus
    # process.unpackedMuons + # Muon sequence removed to align with MC gammaJet focus
    # process.muonAnalyzer # Muon sequence removed to align with MC gammaJet focus
    # process.akPu4CaloJetAnalyzer # Calo jets removed to focus on PF jets with substructure
    )

#customisation
process.particleFlowAnalyser.ptMin = 0.0
process.ggHiNtuplizer.muonPtMin = 0.0

# Select the types of jets filled
# Parameters adapted from MC gammaJet configuration
jetPtMin = 15
jetAbsEtaMax = 2.5

# Substructure settings from MC gammaJet
doSubstructure = True
sdZList = ["1", "2", "3", "4", "5"]

# Choose which additional information is added to jet trees
doHIJetID = False            # Fill jet ID and composition information branches (set to False like MC gammaJet)
doWTARecluster = False       # Add jet phi and eta for WTA axis (set to False like MC gammaJet)
doBtagging  =  False         # Note that setting to True increases computing time a lot
matchJets = False            # No MC truth matching for data

# 0 means use original mini-AOD jets, otherwise use R value, e.g., 3,4,8
# Add all the values you want to process to the list
# These will create collections of CS subtracted jets (only eta dependent background)
jetLabelsCS = ["2","3","4","5","6","8"] # Wider range of jets from MC gammaJet

# For this list, give the R-values for flow subtracted CS jets (eta and phi dependent background)
jetLabelsFlowCS = ["4"]

# Combine the two lists such that all selected jets can be easily looped over
# Also add "Flow" tag for the flow jets to distinguish them from non-flow jets
allJetLabels = jetLabelsCS + [flowR + "Flow" for flowR in jetLabelsFlowCS]

# If adding substructure observables, keeping only 1 default jet analyzer for non-flow, non-substructure case
baseAdded = False

# add candidate tagging, copy/paste to add other jet radii
from HeavyIonsAnalysis.JetAnalysis.setupJets_PbPb_cff import candidateBtaggingMiniAOD

for jetLabel in allJetLabels:
    # B-tagging setup
    candidateBtaggingMiniAOD(
        process,
        isMC=False, # Data specific
        jetPtMin=jetPtMin,
        jetCorrLevels=['L2Relative', 'L2L3Residual'], # Data specific
        doBtagging=doBtagging,
        labelR=jetLabel
    )

    # Base jet analyzer
    baseLabel = "akCs{0}PFJetAnalyzer".format(jetLabel)
    setattr(process, baseLabel, process.akCs4PFJetAnalyzer.clone())
    ana = getattr(process, baseLabel)
    ana.jetTag                   = "selectedUpdatedPatJetsAK{0}PFBtag".format(jetLabel)
    ana.jetName                  = 'akCs{0}PF'.format(jetLabel)
    ana.matchJets                = matchJets # False for data
    ana.doHiJetID                = doHIJetID
    ana.doWTARecluster           = doWTARecluster
    ana.jetPtMin                 = jetPtMin
    ana.jetAbsEtaMax             = cms.untracked.double(jetAbsEtaMax)
    ana.rParam                   = 0.4 if jetLabel == "0" else float(jetLabel.replace("Flow","")) * 0.1

    if not doSubstructure or "Flow" in jetLabel:
        # no substructure OR flow jets -> keep every base analyzer
        process.forest += ana
    else:
        # substructure AND not a flow jet -> keep only the *first* base analyzer
        if not baseAdded:
            process.forest += ana
            baseAdded = True
    if doBtagging:
        ana.useNewBtaggers = True
        ana.pfJetProbabilityBJetTag        = cms.untracked.string(
            "pfJetProbabilityBJetTagsAK{0}PFBtag".format(jetLabel)
        )
        ana.pfUnifiedParticleTransformerAK4JetTags = cms.untracked.string(
            "pfUnifiedParticleTransformerAK4JetTagsAK{0}PFBtag".format(jetLabel)
        )

    # If substructure studies are enabled, add one analyzer per sdZcut for non-Flow jets
    if doSubstructure and "Flow" not in jetLabel:
        for sdZ in sdZList:
            sdLabel = "akCs{0}PFJetAnalyzerSDZcut{1}".format(jetLabel, sdZ)
            setattr(process, sdLabel, process.akCs4PFJetAnalyzer.clone()) # Clone the base one for DATA
            ana_sd = getattr(process, sdLabel)
            # Inherit all settings from the base analyzer...
            ana_sd.doSubstructure = True
            ana_sd.sdZcut         = cms.untracked.double(float(sdZ) * 0.1)
            # And point all other needed inputs identical to the base:
            ana_sd.jetTag                   = ana.jetTag
            ana_sd.jetName                  = ana.jetName
            ana_sd.matchJets                = ana.matchJets # False
            ana_sd.doHiJetID                = ana.doHiJetID
            ana_sd.doWTARecluster           = ana.doWTARecluster
            ana_sd.jetPtMin                 = ana.jetPtMin
            ana_sd.jetAbsEtaMax             = ana.jetAbsEtaMax
            ana_sd.rParam                   = ana.rParam
            if doBtagging:
                ana_sd.useNewBtaggers                      = ana.useNewBtaggers
                ana_sd.pfJetProbabilityBJetTag             = ana.pfJetProbabilityBJetTag
                ana_sd.pfUnifiedParticleTransformerAK4JetTags = ana.pfUnifiedParticleTransformerAK4JetTags

            # Finally, add each SDZ analyzer to the forest
            process.forest += getattr(process, sdLabel)

#########################
# Event Selection -> add the needed filters here
#########################

process.load('HeavyIonsAnalysis.EventAnalysis.collisionEventSelection_cff')
process.pclusterCompatibilityFilter = cms.Path(process.clusterCompatibilityFilter)
process.pprimaryVertexFilter = cms.Path(process.primaryVertexFilter)
process.load('HeavyIonsAnalysis.EventAnalysis.hffilter_cfi')
process.pphfCoincFilter4Th2 = cms.Path(process.phfCoincFilter4Th2)
process.pphfCoincFilter1Th3 = cms.Path(process.phfCoincFilter1Th3)
process.pphfCoincFilter2Th3 = cms.Path(process.phfCoincFilter2Th3)
process.pphfCoincFilter3Th3 = cms.Path(process.phfCoincFilter3Th3)
process.pphfCoincFilter4Th3 = cms.Path(process.phfCoincFilter4Th3)
process.pphfCoincFilter5Th3 = cms.Path(process.phfCoincFilter5Th3)
process.pphfCoincFilter1Th4 = cms.Path(process.phfCoincFilter1Th4)
process.pphfCoincFilter2Th4 = cms.Path(process.phfCoincFilter2Th4)
process.pphfCoincFilter3Th4 = cms.Path(process.phfCoincFilter3Th4)
process.pphfCoincFilter4Th4 = cms.Path(process.phfCoincFilter4Th4)
process.pphfCoincFilter5Th4 = cms.Path(process.phfCoincFilter5Th4)
process.pphfCoincFilter1Th5 = cms.Path(process.phfCoincFilter1Th5)
process.pphfCoincFilter2Th5 = cms.Path(process.phfCoincFilter2Th5)
process.pphfCoincFilter3Th5 = cms.Path(process.phfCoincFilter3Th5)
process.pphfCoincFilter4Th5 = cms.Path(process.phfCoincFilter4Th5)
process.pphfCoincFilter5Th5 = cms.Path(process.phfCoincFilter5Th5)
process.pphfCoincFilter1Th6 = cms.Path(process.phfCoincFilter1Th6)
process.pphfCoincFilter2Th6 = cms.Path(process.phfCoincFilter2Th6)
process.pphfCoincFilter3Th6 = cms.Path(process.phfCoincFilter3Th6)
process.pphfCoincFilter4Th6 = cms.Path(process.phfCoincFilter4Th6)
process.pphfCoincFilter5Th6 = cms.Path(process.phfCoincFilter5Th6)
process.pAna = cms.EndPath(process.skimanalysis)

#from HLTrigger.HLTfilters.hltHighLevel_cfi import hltHighLevel
#process.hltfilter = hltHighLevel.clone(
#    HLTPaths = [
#        #"HLT_HIZeroBias_v4",                                                     
#        "HLT_HIMinimumBias_v2",
#    ]
#)
#process.filterSequence = cms.Sequence(
#    process.hltfilter
#)
#
#process.superFilterPath = cms.Path(process.filterSequence)
#process.skimanalysis.superFilters = cms.vstring("superFilterPath")
#
#for path in process.paths:
#    getattr(process, path)._seq = process.filterSequence * getattr(process,path)._seq
