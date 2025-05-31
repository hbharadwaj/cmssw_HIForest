from WMCore.Configuration import Configuration

config = Configuration()

# ── General ────────────────────────────────────────────────────────────────────
config.section_('General')
config.General.requestName    = '2025_May_31_HIRawPrime4_with_Trigger'
config.General.workArea       = 'crab_projects'
config.General.transferOutputs= True
config.General.transferLogs   = False

# ── JobType ────────────────────────────────────────────────────────────────────
config.section_('JobType')
config.JobType.pluginName     = 'Analysis'
config.JobType.psetName       = 'forest_miniAOD_run3_DATA_gammaJet.py'
config.JobType.numCores       = 1
# Keep memory per core at 2500 MB
config.JobType.maxMemoryMB    = 2500 * config.JobType.numCores
# Ask for up to 48 h runtime (in minutes)
config.JobType.maxJobRuntimeMin = 2750

# ── Data ───────────────────────────────────────────────────────────────────────
config.section_('Data')
config.Data.inputDataset      = "/HIPhysicsRawPrime4/HIRun2023A-PromptReco-v2/MINIAOD"
config.Data.inputDBS          = "global"

# Lumi‐based splitting: now *integer* 5 lumis per job
config.Data.splitting         = 'LumiBased'
config.Data.unitsPerJob       = 10        # was '10' (string); now 2 lumis ≃5 h/job
config.Data.totalUnits        = -1       # process all lumis

# Output location
config.Data.outLFNDirBase     = (
  '/store/group/phys_heavyions/bharikri/Forest/'
  'GammaJetSubstructure/2023_PbPb_Data/'
  + config.General.requestName + '/'
)

config.Data.publication       = False
config.Data.lumiMask          = 'Cert_Collisions2023HI_374288_375823_Golden.json'

# ── Site ───────────────────────────────────────────────────────────────────────
config.section_('Site')
config.Site.storageSite       = 'T2_CH_CERN'
# Optionally blacklist known short‐wall‐time sites:
config.Site.blacklist        = ['T3_UK_ScotGrid_GLA','T2_DE_DESY']

# ── Debug ──────────────────────────────────────────────────────────────────────
# (Uncomment if you need extra JDL flags)
# config.section_('Debug')
# config.Debug.extraJDL       = ['+CMS_ALLOW_OVERFLOW=False']
