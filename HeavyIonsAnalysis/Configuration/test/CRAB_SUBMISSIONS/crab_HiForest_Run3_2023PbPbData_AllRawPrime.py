#!/usr/bin/env python

# from WMCore.Configuration import Configuration
from CRABClient.UserUtilities import Configuration
# import CRABClient

config = Configuration()

# ── General ────────────────────────────────────────────────────────────────────
config.section_('General')
config.General.requestName    = '2025_Jun_30_HIRawPrime'
config.General.workArea       = 'crab_projects'
config.General.transferOutputs= True
config.General.transferLogs   = False

# ── JobType ────────────────────────────────────────────────────────────────────
config.section_('JobType')
config.JobType.pluginName     = 'Analysis'
config.JobType.psetName       = '../forest_miniAOD_run3_DATA_gammaJet.py'
config.JobType.numCores       = 1
# Keep memory per core at 2500 MB
config.JobType.maxMemoryMB    = 2500 * config.JobType.numCores
# Ask for up to 48 h runtime (in minutes)
config.JobType.maxJobRuntimeMin = 2750

# ── Data ───────────────────────────────────────────────────────────────────────
config.section_('Data')
config.Data.inputDataset      = "/HIPhysicsRawPrime31/HIRun2023A-PromptReco-v2/MINIAOD"
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
config.Data.lumiMask          = '../Cert_Collisions2023HI_374288_375823_Golden.json'

# ── Site ───────────────────────────────────────────────────────────────────────
config.section_('Site')
config.Site.storageSite       = 'T2_CH_CERN'
# Optionally blacklist known short‐wall‐time sites:
config.Site.blacklist        = ['T3_UK_ScotGrid_GLA','T2_DE_DESY']

# ── Debug ──────────────────────────────────────────────────────────────────────
# (Uncomment if you need extra JDL flags)
# config.section_('Debug')
# config.Debug.extraJDL       = ['+CMS_ALLOW_OVERFLOW=False']

if __name__ == '__main__':

    from CRABAPI.RawCommand import crabCommand
    
    dataset_list_2023PbPb = [
        '/HIPhysicsRawPrime0/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime1/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime2/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime3/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime4/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime5/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime6/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime7/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime8/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime9/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime10/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime11/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime12/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime13/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime14/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime15/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime16/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime17/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime18/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime19/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime20/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime21/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime22/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime23/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime24/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime25/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime26/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime27/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime28/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime29/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime30/HIRun2023A-PromptReco-v2/MINIAOD',
        '/HIPhysicsRawPrime31/HIRun2023A-PromptReco-v2/MINIAOD'
    ]

    for dataset in dataset_list_2023PbPb:
        config.Data.inputDataset = dataset
        config.General.requestName ='2025_Jun_30_' + dataset.split('/')[1] + '_fixed_SD'
        crabCommand('submit', config = config)