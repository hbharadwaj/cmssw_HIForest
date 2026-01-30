### HiForest Configuration
# Input: miniAOD
# Type: mc

import FWCore.ParameterSet.Config as cms
from Configuration.Eras.Era_Run3_pp_on_PbPb_2024_cff import Run3_pp_on_PbPb_2024
process = cms.Process('HiForest', Run3_pp_on_PbPb_2024)

###############################################################################

# HiForest info
process.load("HeavyIonsAnalysis.EventAnalysis.HiForestInfo_cfi")
process.HiForestInfo.info = cms.vstring("HiForest, miniAOD, 141X, mc")

###############################################################################

# input files
process.source = cms.Source("PoolSource",
    duplicateCheckMode = cms.untracked.string("noDuplicateCheck"),
    fileNames = cms.untracked.vstring(
        #'/store/group/phys_heavyions/jviinika/PythiaHydjetRun3_5p36TeV_dijet_ptHat15_100kEvents_miniAOD_2023_08_30/PythiaHydjetDijetRun3/PythiaHydjetRun3_dijet_ptHat15_5p36TeV_miniAOD/230830_165931/0000/pythiaHydjet_miniAOD_11.root'
        'root://cms-xrd-global.cern.ch///store/user/bharikri/Run3MC_PbPb/MINIAOD/2025_Jun_10_step4_MINIAODSIM_Pythia8_Embedded_QCDPhoton50_TuneCP5_14_1_7/Pythia8_Embedded_QCDPhoton_TuneCP5_2024/step4_MINIAODSIM_Pythia8_Embedded_QCDPhoton50_TuneCP5_14_1_7/250629_151438/0000/step4_1.root'
    ),
)

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
process.GlobalTag = GlobalTag(process.GlobalTag, '141X_mcRun3_2024_realistic_HI_v14', '')
process.HiForestInfo.GlobalTagLabel = process.GlobalTag.globaltag
process.GlobalTag.snapshotTime = cms.string("9999-12-31 23:59:59.000")
process.GlobalTag.toGet.extend([
    cms.PSet(record = cms.string("BTagTrackProbability3DRcd"),
             tag = cms.string("JPcalib_MC103X_2018PbPb_v4"),
             connect = cms.string("frontier://FrontierProd/CMS_CONDITIONS")
         )
])

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

#############################
# Gen Analyzer
#############################
process.load('HeavyIonsAnalysis.EventAnalysis.HiGenAnalyzer_cfi')

# event analysis
process.load('HeavyIonsAnalysis.EventAnalysis.hltanalysis_cfi')
process.load('HeavyIonsAnalysis.EventAnalysis.particleFlowAnalyser_cfi')
process.load('HeavyIonsAnalysis.EventAnalysis.hievtanalyzer_mc_cfi')
process.load('HeavyIonsAnalysis.EventAnalysis.skimanalysis_cfi')
process.load('HeavyIonsAnalysis.EventAnalysis.hltobject_cfi')
process.load('HeavyIonsAnalysis.EventAnalysis.l1object_cfi')

#from HeavyIonsAnalysis.EventAnalysis.hltobject_cfi import trigger_list_mc
#process.hltobject.triggerNames = trigger_list_mc

################################
# electrons, photons, muons
process.load('HeavyIonsAnalysis.EGMAnalysis.ggHiNtuplizer_cfi')
process.ggHiNtuplizer.doGenParticles = cms.bool(True)
process.ggHiNtuplizer.doMuons = cms.bool(False)
process.load("TrackingTools.TransientTrack.TransientTrackBuilder_cfi")
################################
# jet reco sequence
process.load('HeavyIonsAnalysis.JetAnalysis.akCs4PFJetSequence_pponPbPb_mc_cff')
################################
# tracks
process.load("HeavyIonsAnalysis.TrackAnalysis.TrackAnalyzers_cff")
#muons
process.load("HeavyIonsAnalysis.MuonAnalysis.unpackedMuons_cfi")
process.load("HeavyIonsAnalysis.MuonAnalysis.muonAnalyzer_cfi")
process.muonAnalyzer.doGen = cms.bool(True)

###############################################################################

#########################                                                                                                                                                 
# ZDC RecHit Producer && Analyzer                                                                                                                                         
#########################                                                                                                                                                 
# to prevent crash related to HcalSeverityLevelComputerRcd record                                                                                                         
process.load("RecoLocalCalo.HcalRecAlgos.hcalRecAlgoESProd_cfi")
process.load('HeavyIonsAnalysis.ZDCAnalysis.ZDCAnalyzersPbPb_cff')

###############################################################################
# main forest sequence
process.forest = cms.Path(
    process.HiForestInfo +
    process.centralityBin +
    process.hltanalysis +
#    process.hltobject +
#    process.l1object +
    process.trackSequencePbPb +
#    process.particleFlowAnalyser +
    process.hiEvtAnalyzer +
    process.HiGenParticleAna +
    process.ggHiNtuplizer +
    process.zdcSequencePbPb
#    process.unpackedMuons +
#    process.muonAnalyzer
    )

#customisation
process.particleFlowAnalyser.ptMin = 0.0
# process.ggHiNtuplizer.muonPtMin = 0.0

# Select the types of jets filled
doCaloJets = False             # Fill jet ID and composition information branches
matchJets = False             # Enables q/g and heavy flavor jet identification in MC
jetPtMin = 10.0
jetAbsEtaMax = 5.2

doSubstructure = True
sdZList = ["1", "2", "3"]

# Choose which additional information is added to jet trees
doHIJetID = True             # Fill jet ID and composition information branches
doWTARecluster = False        # Add jet phi and eta for WTA axis
doBtagging = False           # Note that setting to True increases computing time a lot

# 0 means use original mini-AOD jets, otherwise use R value, e.g., 3,4,8
# Generator level jets in original miniAOD jets contain neutrinos
# You will need to do reclustering with R-value to get generator level jets without neutrinos
# Add all the values you want to process to the list
# These will create collections of CS subtracted jets (only eta dependent background)
jetLabelsCS = ["2","3","4","5","6","8"]

# For this list, give the R-values for flow subtracted CS jets (eta and phi dependent background)
jetLabelsFlowCS = ["4"]

# Combine the two lists such that all selected jets can be easily looped over
# Also add "Flow" tag for the flow jets to distinguish them from non-flow jets
allJetLabels = jetLabelsCS + [flowR + "Flow" for flowR in jetLabelsFlowCS]

# If adding substructure observables, keeping only 1 default jet analyzer
baseAdded = False

# add candidate tagging, copy/paste to add other jet radii
from HeavyIonsAnalysis.JetAnalysis.setupJets_PbPb_cff import candidateBtaggingMiniAOD

for jetLabel in allJetLabels:
    # B-tagging setup
    candidateBtaggingMiniAOD(
        process,
        isMC=True,
        jetPtMin=jetPtMin,
        jetCorrLevels=['L2Relative', 'L3Absolute'],
        doBtagging=doBtagging,
        labelR=jetLabel
    )

    # Base jet analyzer
    baseLabel = "akCs{0}PFJetAnalyzer".format(jetLabel)
    setattr(process, baseLabel, process.akCs4PFJetAnalyzer.clone())
    ana = getattr(process, baseLabel)
    ana.jetTag                   = "selectedUpdatedPatJetsAK{0}PFBtag".format(jetLabel)
    ana.jetName                  = 'akCs{0}PF'.format(jetLabel)
    ana.matchJets                = matchJets
    ana.matchTag                 = 'patJetsAK{0}PFUnsubJets'.format(jetLabel)
    ana.doHiJetID                = doHIJetID
    ana.doCaloJets               = doCaloJets
    ana.doWTARecluster          = doWTARecluster
    ana.jetPtMin                 = jetPtMin
    ana.jetAbsEtaMax             = cms.untracked.double(jetAbsEtaMax)
    ana.rParam                   = 0.4 if jetLabel == "0" else float(jetLabel.replace("Flow","")) * 0.1
    ana.jetFlavourInfos          = "ak{0}PFUnsubJetFlavourInfos".format(jetLabel)
    if jetLabel != "0":
        ana.genjetTag = "ak{0}GenJetsReclusterNoNu".format(jetLabel)
            
    if not doSubstructure or "Flow" in jetLabel:
        # no substructure → keep every base analyzer
        process.forest += ana
    else:
        # substructure → keep only the *first* base analyzer
        if "Flow" in jetLabel:
            process.forest += ana
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

    # If substructure studies are enabled, add one analyzer per sdZcut
    if doSubstructure and "Flow" not in jetLabel:
        for sdZ in sdZList:
            sdLabel = "akCs{0}PFJetAnalyzerSDZcut{1}".format(jetLabel, sdZ)
            setattr(process, sdLabel, process.akCs4PFJetAnalyzer.clone())
            ana_sd = getattr(process, sdLabel)
            # Inherit all settings from the base analyzer...
            ana_sd.doSubstructure = True
            ana_sd.sdZcut         = cms.untracked.double(float(sdZ) * 0.1)
            # And point all other needed inputs identical to the base:
            ana_sd.jetTag                   = ana.jetTag
            ana_sd.jetName                  = ana.jetName
            ana_sd.matchJets                = ana.matchJets
            ana_sd.matchTag                 = ana.matchTag
            ana_sd.doHiJetID                = ana.doHiJetID
            ana_sd.doWTARecluster          = ana.doWTARecluster
            ana_sd.jetPtMin                 = ana.jetPtMin
            ana_sd.jetAbsEtaMax             = ana.jetAbsEtaMax
            ana_sd.rParam                   = ana.rParam
            ana_sd.jetFlavourInfos          = ana.jetFlavourInfos
            ana_sd.doChargedConstOnly       = True
            if jetLabel != "0":
                ana_sd.genjetTag = "ak{0}GenJetsReclusterNoNu".format(jetLabel)
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
process.load('HeavyIonsAnalysis.EventAnalysis.hffilterPF_cfi')
process.pphfCoincFilter2Th4 = cms.Path(process.phfCoincFilter2Th4)
process.pAna = cms.EndPath(process.skimanalysis)
