import FWCore.ParameterSet.Config as cms
# Heavy Ion specific regression modifier for Run 3 
from RecoEgamma.EgammaTools.regressionModifier_cfi import *

# Adapted from regressionModifier106XUL

regressionModifierRun3HIN = cms.PSet(
    modifierName = cms.string('EGRegressionModifierV3'),       
    rhoTag = cms.InputTag('fixedGridRhoFastjetAllTmp'),
    useClosestToCentreSeedCrysDef = cms.bool(False),
    useBuggedHOverE = cms.bool(False),
    maxRawEnergyForLowPtEBSigma = cms.double(-1), 
    maxRawEnergyForLowPtEESigma = cms.double(1200.),
    eleRegs = cms.PSet(
        ecalOnlyMean = cms.PSet(
            rangeMinLowEt = cms.double(0.2),
            rangeMaxLowEt = cms.double(2.0),
            rangeMinHighEt = cms.double(-1.),
            rangeMaxHighEt = cms.double(3.0),
            forceHighEnergyTrainingIfSaturated = cms.bool(True),
            lowEtHighEtBoundary = cms.double(999999.),
            ebLowEtForestName = cms.ESInputTag("", "electron_eb_ecalOnly_10To500_0p2To2_mean"),
            ebHighEtForestName = cms.ESInputTag("", "electron_eb_ECALonly"),
            eeLowEtForestName = cms.ESInputTag("", "electron_ee_ecalOnly_10To500_0p2To2_mean"),
            eeHighEtForestName = cms.ESInputTag("", "electron_ee_ECALonly"),
            ),
        ecalOnlySigma = cms.PSet(
            rangeMinLowEt = cms.double(0.0002),
            rangeMaxLowEt = cms.double(0.5),
            rangeMinHighEt = cms.double(0.0002),
            rangeMaxHighEt = cms.double(0.5),
            forceHighEnergyTrainingIfSaturated = cms.bool(True),
            lowEtHighEtBoundary = cms.double(999999.),
            ebLowEtForestName = cms.ESInputTag("", "electron_eb_ecalOnly_10To500_0p0002To0p5_sigma"),
            ebHighEtForestName = cms.ESInputTag("", "electron_eb_ECALonly_var"),
            eeLowEtForestName = cms.ESInputTag("", "electron_ee_ecalOnly_10To500_0p0002To0p5_sigma"),
            eeHighEtForestName = cms.ESInputTag("", "electron_ee_ECALonly_var"),
            ),
        epComb = cms.PSet(
            ecalTrkRegressionConfig = cms.PSet(
                rangeMinLowEt = cms.double(0.2),
                rangeMaxLowEt = cms.double(2.0),
                rangeMinHighEt = cms.double(0.2),
                rangeMaxHighEt = cms.double(2.0),
                lowEtHighEtBoundary = cms.double(999999.),
                forceHighEnergyTrainingIfSaturated = cms.bool(False),
                ebLowEtForestName = cms.ESInputTag("", 'electron_eb_ecalTrk_10To500_0p2To2_mean'),
                ebHighEtForestName = cms.ESInputTag("", 'electron_eb_ecalTrk_10To500_0p2To2_mean'),
                eeLowEtForestName = cms.ESInputTag("", 'electron_ee_ecalTrk_10To500_0p2To2_mean'),
                eeHighEtForestName = cms.ESInputTag("", 'electron_ee_ecalTrk_10To500_0p2To2_mean'),
                ),
            ecalTrkRegressionUncertConfig = cms.PSet(
                rangeMinLowEt = cms.double(0.0002),
                rangeMaxLowEt = cms.double(0.5),
                rangeMinHighEt = cms.double(0.0002),
                rangeMaxHighEt = cms.double(0.5),
                lowEtHighEtBoundary = cms.double(999999.),  
                forceHighEnergyTrainingIfSaturated = cms.bool(False),
                ebLowEtForestName = cms.ESInputTag("", 'electron_eb_ecalTrk_10To500_0p0002To0p5_sigma'),
                ebHighEtForestName = cms.ESInputTag("", 'electron_eb_ecalTrk_10To500_0p0002To0p5_sigma'),
                eeLowEtForestName = cms.ESInputTag("", 'electron_ee_ecalTrk_10To500_0p0002To0p5_sigma'),
                eeHighEtForestName = cms.ESInputTag("", 'electron_ee_ecalTrk_10To500_0p0002To0p5_sigma'),
                ),
            maxEcalEnergyForComb=cms.double(200.),
            minEOverPForComb=cms.double(0.025),
            maxEPDiffInSigmaForComb=cms.double(15.),
            maxRelTrkMomErrForComb=cms.double(10.),                
            )
        ),
    phoRegs = cms.PSet(
        ecalOnlyMean = cms.PSet(
            rangeMinLowEt = cms.double(0.2),
            rangeMaxLowEt = cms.double(2.0),
            rangeMinHighEt = cms.double(-1.),
            rangeMaxHighEt = cms.double(3.0),
            forceHighEnergyTrainingIfSaturated = cms.bool(True),
            lowEtHighEtBoundary = cms.double(999999.),
            ebLowEtForestName = cms.ESInputTag("", "photon_eb_ecalOnly_10To500_0p2To2_mean"),
            ebHighEtForestName = cms.ESInputTag("", "photon_eb_ECALonly"),
            eeLowEtForestName = cms.ESInputTag("", "photon_ee_ecalOnly_10To500_0p2To2_mean"),
            eeHighEtForestName = cms.ESInputTag("", "photon_ee_ECALonly"),
            ),
        ecalOnlySigma = cms.PSet(
            rangeMinLowEt = cms.double(0.0002),
            rangeMaxLowEt = cms.double(0.5),
            rangeMinHighEt = cms.double(0.0002),
            rangeMaxHighEt = cms.double(0.5),
            forceHighEnergyTrainingIfSaturated = cms.bool(True),
            lowEtHighEtBoundary = cms.double(999999.),
            ebLowEtForestName = cms.ESInputTag("", "photon_eb_ecalOnly_10To500_0p0002To0p5_sigma"),
            ebHighEtForestName = cms.ESInputTag("", "photon_eb_ECALonly_var"),
            eeLowEtForestName = cms.ESInputTag("", "photon_ee_ecalOnly_10To500_0p0002To0p5_sigma"),
            eeHighEtForestName = cms.ESInputTag("", "photon_ee_ECALonly_var"),
        ),
    )
)

# Add HIN specific modifier for Run3 Heavy Ion collisions
from Configuration.Eras.Modifier_run3_egamma_cff import run3_egamma # using the era modifier for pp EGM as well for safety
run3_egamma.toReplaceWith(regressionModifier,regressionModifierRun3HIN)

from Configuration.Eras.Modifier_pp_on_PbPb_run3_2023_cff import pp_on_PbPb_run3_2023
pp_on_PbPb_run3_2023.toReplaceWith(regressionModifier,regressionModifierRun3HIN)

from Configuration.Eras.Modifier_pp_on_PbPb_run3_2024_cff import pp_on_PbPb_run3_2024
pp_on_PbPb_run3_2024.toReplaceWith(regressionModifier,regressionModifierRun3HIN)

from Configuration.Eras.Modifier_ppRef_2024_cff import ppRef_2024
ppRef_2024.toReplaceWith(regressionModifier,regressionModifierRun3HIN)
