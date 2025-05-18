from WMCore.Configuration import Configuration
#from CRABClient.Configuration import Configuration
config = Configuration()

config.section_('General')
config.General.requestName = '2025_May_18_HIRawPrime0_test_substructure'
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs = False

config.section_('JobType')
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'forest_miniAOD_run3_DATA_gammaJet.py'
config.JobType.numCores = 1
config.JobType.maxMemoryMB = 2000 * config.JobType.numCores

config.section_('Data')
config.Data.inputDataset = "/HIPhysicsRawPrime0/HIRun2023A-PromptReco-v2/MINIAOD"
config.Data.inputDBS = "global"
config.Data.splitting = 'Automatic'
# config.Data.unitsPerJob = 1
config.Data.totalUnits = -1
config.Data.outLFNDirBase = '/store/group/phys_heavyions/bharikri/Forest/GammaJetSubstructure/2023_PbPb_Data/'+config.General.requestName+'/'
config.Data.publication = False
# config.Data.outputDatasetTag = 'step4_MINIAODSIM_Pythia8_Embedded_DoublePhoton_TuneCP5_13_2_13'

config.section_('Site')
# config.Data.ignoreLocality = True
config.Site.storageSite = 'T2_CH_CERN'
#config.Site.whitelist = ['T2_FR_GRIF_LLR','T2_US_MIT']# ['T2_FR_*', 'T2_CH_CERN']

#config.section_('Debug')
#config.Debug.extraJDL = ['+CMS_ALLOW_OVERFLOW=False']

