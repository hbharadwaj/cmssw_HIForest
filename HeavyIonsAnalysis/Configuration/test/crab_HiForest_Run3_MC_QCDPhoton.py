from WMCore.Configuration import Configuration
#from CRABClient.Configuration import Configuration
config = Configuration()

config.section_('General')
config.General.requestName = '2025_May_26_QCDPhoton30_test_substructure'
config.General.workArea = 'crab_projects'
config.General.transferOutputs = True
config.General.transferLogs = False

config.section_('JobType')
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'forest_miniAOD_run3_MC_gammaJet.py'
config.JobType.numCores = 1
config.JobType.maxMemoryMB = 5000 * config.JobType.numCores

config.section_('Data')
config.Data.inputDataset = "/Pythia8_Embedded_QCDPhoton30_TuneCP5/bharikri-2024_Mar_07_step4_MINIAODSIM_Pythia8_Embedded_QCDPhoton30_TuneCP5_13_2_10-1bf2ca3122c0a5b11c34b8148c6e5d2d/USER"
config.Data.inputDBS = "phys03"
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 1
config.Data.totalUnits = -1
config.Data.outLFNDirBase = '/store/group/phys_heavyions/bharikri/Forest/GammaJetSubstructure/2023_QCDPhoton30/'+config.General.requestName+'/'
config.Data.publication = False
# config.Data.outputDatasetTag = 'step4_MINIAODSIM_Pythia8_Embedded_DoublePhoton_TuneCP5_13_2_13'

config.section_('Site')
# config.Data.ignoreLocality = True
config.Site.storageSite = 'T2_CH_CERN'
#config.Site.whitelist = ['T2_FR_GRIF_LLR','T2_US_MIT']# ['T2_FR_*', 'T2_CH_CERN']

#config.section_('Debug')
#config.Debug.extraJDL = ['+CMS_ALLOW_OVERFLOW=False']

