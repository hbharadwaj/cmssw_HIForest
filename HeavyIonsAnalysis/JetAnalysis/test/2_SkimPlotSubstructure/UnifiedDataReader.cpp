/**
 * UnifiedDataReader.cpp - RVec-Compatible Implementation
 */

#include "include/UnifiedDataReader.h"
#include <iostream>
#include <stdexcept>

UnifiedDataReader::UnifiedDataReader(TChain* chain,
                                     const BranchMapper& mapper,
                                     const std::vector<std::string>& jetCollections)
    : chain_(chain), mapper_(mapper), jetCollections_(jetCollections),
      hiBin_(-999), vz_(-999), hiHF_(-999), rho_(-999),
      weight_(1.0), weightPthat_(1.0), nMC_(0),
      isScalarPhoton_(mapper.isScalarPhoton()),
      nPho_(0),
      pho_et_v_(nullptr), pho_eta_v_(nullptr), pho_phi_v_(nullptr), pho_hoe_v_(nullptr),
      pho_sieie_v_(nullptr), pho_see_v_(nullptr), pho_ecal_v_(nullptr), pho_hcal_v_(nullptr), pho_trk_v_(nullptr),
      pho_pfc_v_(nullptr), pho_pfn_v_(nullptr), pho_pfp_v_(nullptr), pho_r9_v_(nullptr),
      pho_genMatchedIndex_v_(nullptr), mcPID_v_(nullptr), mcMomPID_v_(nullptr),
      mcPt_v_(nullptr), mcEta_v_(nullptr), mcPhi_v_(nullptr), mcCalIsoDR04_v_(nullptr)
{
    log("INFO", "UnifiedDataReader created");
    
    // Allocate jet arrays for all observables
    for (const auto& coll : jetCollections_) {
        // Reco jets
        jet_pt_arr_[coll] = new Float_t[MAXJETS];
        jet_eta_arr_[coll] = new Float_t[MAXJETS];
        jet_phi_arr_[coll] = new Float_t[MAXJETS];
        jet_mass_arr_[coll] = new Float_t[MAXJETS];
        jet_area_arr_[coll] = new Float_t[MAXJETS];
        jet_split_arr_[coll] = new Int_t[MAXJETS];
        jet_z_arr_[coll] = new Float_t[MAXJETS];
        jet_kt_arr_[coll] = new Float_t[MAXJETS];
        jet_dr_arr_[coll] = new Float_t[MAXJETS];
        jet_girth_arr_[coll] = new Float_t[MAXJETS];
        jet_thrust_arr_[coll] = new Float_t[MAXJETS];
        jet_lha_arr_[coll] = new Float_t[MAXJETS];
        jet_ptd_arr_[coll] = new Float_t[MAXJETS];
        jet_tau_form_arr_[coll] = new Float_t[MAXJETS];
        jet_intjet_multi_arr_[coll] = new Int_t[MAXJETS];
        
        // Ref jets (MC truth)
        ref_pt_arr_[coll] = new Float_t[MAXJETS];
        ref_eta_arr_[coll] = new Float_t[MAXJETS];
        ref_phi_arr_[coll] = new Float_t[MAXJETS];
        ref_mass_arr_[coll] = new Float_t[MAXJETS];
        ref_area_arr_[coll] = new Float_t[MAXJETS];
        ref_split_arr_[coll] = new Int_t[MAXJETS];
        ref_z_arr_[coll] = new Float_t[MAXJETS];
        ref_kt_arr_[coll] = new Float_t[MAXJETS];
        ref_dr_arr_[coll] = new Float_t[MAXJETS];
        ref_girth_arr_[coll] = new Float_t[MAXJETS];
        ref_thrust_arr_[coll] = new Float_t[MAXJETS];
        ref_lha_arr_[coll] = new Float_t[MAXJETS];
        ref_ptd_arr_[coll] = new Float_t[MAXJETS];
        ref_tau_form_arr_[coll] = new Float_t[MAXJETS];
        ref_intjet_multi_arr_[coll] = new Int_t[MAXJETS];
    }
}

UnifiedDataReader::~UnifiedDataReader() {
    // Delete reco jet arrays
    for (auto& pair : jet_pt_arr_) delete[] pair.second;
    for (auto& pair : jet_eta_arr_) delete[] pair.second;
    for (auto& pair : jet_phi_arr_) delete[] pair.second;
    for (auto& pair : jet_mass_arr_) delete[] pair.second;
    for (auto& pair : jet_area_arr_) delete[] pair.second;
    for (auto& pair : jet_split_arr_) delete[] pair.second;
    for (auto& pair : jet_z_arr_) delete[] pair.second;
    for (auto& pair : jet_kt_arr_) delete[] pair.second;
    for (auto& pair : jet_dr_arr_) delete[] pair.second;
    for (auto& pair : jet_girth_arr_) delete[] pair.second;
    for (auto& pair : jet_thrust_arr_) delete[] pair.second;
    for (auto& pair : jet_lha_arr_) delete[] pair.second;
    for (auto& pair : jet_ptd_arr_) delete[] pair.second;
    for (auto& pair : jet_tau_form_arr_) delete[] pair.second;
    for (auto& pair : jet_intjet_multi_arr_) delete[] pair.second;
    
    // Delete ref jet arrays
    for (auto& pair : ref_pt_arr_) delete[] pair.second;
    for (auto& pair : ref_eta_arr_) delete[] pair.second;
    for (auto& pair : ref_phi_arr_) delete[] pair.second;
    for (auto& pair : ref_mass_arr_) delete[] pair.second;
    for (auto& pair : ref_area_arr_) delete[] pair.second;
    for (auto& pair : ref_split_arr_) delete[] pair.second;
    for (auto& pair : ref_z_arr_) delete[] pair.second;
    for (auto& pair : ref_kt_arr_) delete[] pair.second;
    for (auto& pair : ref_dr_arr_) delete[] pair.second;
    for (auto& pair : ref_girth_arr_) delete[] pair.second;
    for (auto& pair : ref_thrust_arr_) delete[] pair.second;
    for (auto& pair : ref_lha_arr_) delete[] pair.second;
    for (auto& pair : ref_ptd_arr_) delete[] pair.second;
    for (auto& pair : ref_tau_form_arr_) delete[] pair.second;
    for (auto& pair : ref_intjet_multi_arr_) delete[] pair.second;
}

void UnifiedDataReader::log(const std::string& level, const std::string& msg) const {
    std::cout << "[" << level << "] UnifiedDataReader: " << msg << std::endl;
}

bool UnifiedDataReader::connectBranch(const std::string& name, void* addr) {
    if (mapper_.isMissing(name)) return false;
    
    TBranch* br = chain_->GetBranch(name.c_str());
    if (!br) {
        log("WARN", "Branch not found: " + name);
        return false;
    }
    
    chain_->SetBranchStatus(name.c_str(), 1);
    chain_->SetBranchAddress(name.c_str(), addr);
    log("INFO", "  Connected: " + name);
    return true;
}

bool UnifiedDataReader::initialize() {
    log("INFO", "Initializing UnifiedDataReader...");
    
    chain_->SetBranchStatus("*", 0);  // Disable all first
    
    // Event-level
    log("INFO", "Setting up event-level branches...");
    std::string hiBinBr = mapper_.getEventLevelBranch("hiBin");
    if (!mapper_.isMissing(hiBinBr)) connectBranch(hiBinBr, &hiBin_);
    
    std::string vzBr = mapper_.getEventLevelBranch("vz");
    if (!mapper_.isMissing(vzBr)) connectBranch(vzBr, &vz_);
    
    std::string hiHFBr = mapper_.getEventLevelBranch("hiHF");
    if (!mapper_.isMissing(hiHFBr)) connectBranch(hiHFBr, &hiHF_);
    
    std::string rhoBr = mapper_.getEventLevelBranch("rho");
    if (!mapper_.isMissing(rhoBr)) connectBranch(rhoBr, &rho_);
    
    // Photons
    log("INFO", "Setting up photon branches...");
    log("INFO", "  Photon mode: " + std::string(isScalarPhoton_ ? "SCALAR" : "VECTOR"));
    
    if (isScalarPhoton_) {
        // 2018: Scalar photons
        connectBranch(mapper_.getPhotonBranch("phoEt"), &pho_et_s_);
        connectBranch(mapper_.getPhotonBranch("phoEta"), &pho_eta_s_);
        connectBranch(mapper_.getPhotonBranch("phoPhi"), &pho_phi_s_);
        connectBranch(mapper_.getPhotonBranch("phoHoverE"), &pho_hoe_s_);
        connectBranch(mapper_.getPhotonBranch("phoSigmaIEtaIEta"), &pho_sieie_s_);
        connectBranch(mapper_.getPhotonBranch("phoSigmaEtaEta"), &pho_see_s_);
        connectBranch(mapper_.getPhotonBranch("phoECALIso"), &pho_ecal_s_);
        connectBranch(mapper_.getPhotonBranch("phoHCALIso"), &pho_hcal_s_);
        connectBranch(mapper_.getPhotonBranch("phoTrackIso"), &pho_trk_s_);
        connectBranch(mapper_.getPhotonBranch("pfcIso"), &pho_pfc_s_);
        connectBranch(mapper_.getPhotonBranch("pfnIso"), &pho_pfn_s_);
        connectBranch(mapper_.getPhotonBranch("pfpIso"), &pho_pfp_s_);
    } else {
        // 2023/2024: RVec photons
        connectBranch(mapper_.getPhotonBranch("nPho"), &nPho_);
        connectBranch(mapper_.getPhotonBranch("phoEt"), &pho_et_v_);
        connectBranch(mapper_.getPhotonBranch("phoEta"), &pho_eta_v_);
        connectBranch(mapper_.getPhotonBranch("phoPhi"), &pho_phi_v_);
        connectBranch(mapper_.getPhotonBranch("phoHoverE"), &pho_hoe_v_);
        connectBranch(mapper_.getPhotonBranch("phoSigmaIEtaIEta"), &pho_sieie_v_);
        connectBranch(mapper_.getPhotonBranch("phoSigmaEtaEta"), &pho_see_v_);
        connectBranch(mapper_.getPhotonBranch("phoECALIso"), &pho_ecal_v_);
        connectBranch(mapper_.getPhotonBranch("phoHCALIso"), &pho_hcal_v_);
        connectBranch(mapper_.getPhotonBranch("phoTrackIso"), &pho_trk_v_);
        connectBranch(mapper_.getPhotonBranch("pfcIso"), &pho_pfc_v_);
        connectBranch(mapper_.getPhotonBranch("pfnIso"), &pho_pfn_v_);
        connectBranch(mapper_.getPhotonBranch("pfpIso"), &pho_pfp_v_);
        
        // R9 (optional, may not exist in all files)
        std::string r9Br = mapper_.getPhotonBranch("phoR9");
        if (!mapper_.isMissing(r9Br)) connectBranch(r9Br, &pho_r9_v_);
        
        // MC truth particles (optional, only for MC)
        std::string genMatchBr = mapper_.getMCBranch("", "pho_genMatchedIndex");
        if (!mapper_.isMissing(genMatchBr)) {
            connectBranch(genMatchBr, &pho_genMatchedIndex_v_);
            log("INFO", "  Connected MC photon gen matching: " + genMatchBr);
        }
        
        std::string nMCBr = mapper_.getMCBranch("", "nMC");
        if (!mapper_.isMissing(nMCBr)) {
            connectBranch(nMCBr, &nMC_);
            log("INFO", "  Connected nMC: " + nMCBr);
        }
        
        std::string mcPIDBr = mapper_.getMCBranch("", "mcPID");
        if (!mapper_.isMissing(mcPIDBr)) {
            connectBranch(mcPIDBr, &mcPID_v_);
            log("INFO", "  Connected mcPID: " + mcPIDBr);
        }
        
        std::string mcMomPIDBr = mapper_.getMCBranch("", "mcMomPID");
        if (!mapper_.isMissing(mcMomPIDBr)) {
            connectBranch(mcMomPIDBr, &mcMomPID_v_);
            log("INFO", "  Connected mcMomPID: " + mcMomPIDBr);
        }
        
        std::string mcPtBr = mapper_.getMCBranch("", "mcPt");
        if (!mapper_.isMissing(mcPtBr)) {
            connectBranch(mcPtBr, &mcPt_v_);
            log("INFO", "  Connected mcPt: " + mcPtBr);
        }
        
        std::string mcEtaBr = mapper_.getMCBranch("", "mcEta");
        if (!mapper_.isMissing(mcEtaBr)) {
            connectBranch(mcEtaBr, &mcEta_v_);
            log("INFO", "  Connected mcEta: " + mcEtaBr);
        }
        
        std::string mcPhiBr = mapper_.getMCBranch("", "mcPhi");
        if (!mapper_.isMissing(mcPhiBr)) {
            connectBranch(mcPhiBr, &mcPhi_v_);
            log("INFO", "  Connected mcPhi: " + mcPhiBr);
        }
        
        std::string mcCalIsoBr = mapper_.getMCBranch("", "mcCalIsoDR04");
        if (!mapper_.isMissing(mcCalIsoBr)) {
            connectBranch(mcCalIsoBr, &mcCalIsoDR04_v_);
            log("INFO", "  Connected mcCalIsoDR04: " + mcCalIsoBr);
        }
        
        // Event-level MC weights
        std::string weightBr = mapper_.getMCBranch("", "weight");
        if (!mapper_.isMissing(weightBr)) {
            connectBranch(weightBr, &weight_);
            log("INFO", "  Connected weight: " + weightBr);
        }
        
        std::string weightPthatBr = mapper_.getMCBranch("", "weight_pthat");
        if (!mapper_.isMissing(weightPthatBr)) {
            connectBranch(weightPthatBr, &weightPthat_);
            log("INFO", "  Connected weight_pthat: " + weightPthatBr);
        }
    }
    
    // MC event weights (optional, only for MC)
    std::string weightBr = mapper_.getEventLevelBranch("weight");
    if (!mapper_.isMissing(weightBr)) {
        connectBranch(weightBr, &weight_);
        log("INFO", "  Connected MC weight branch");
    }
    
    std::string weightPthatBr = mapper_.getEventLevelBranch("weight_pthat");
    if (!mapper_.isMissing(weightPthatBr)) {
        connectBranch(weightPthatBr, &weightPthat_);
        log("INFO", "  Connected MC weight_pthat branch");
    }
    
    // Jets - Now handled by UnifiedDataReader (Phase 2 refactor)
    log("INFO", "Setting up jet branches for " + std::to_string(jetCollections_.size()) + " collections...");
    for (const auto& coll : jetCollections_) {
        log("INFO", "  Collection: " + coll);
        
        // Number of jets
        std::string nrefBr = mapper_.getJetBranch(coll, "nref");
        if (!mapper_.isMissing(nrefBr)) connectBranch(nrefBr, &jetNref_[coll]);
        
        // Reco jet kinematics
        std::string ptBr = mapper_.getJetBranch(coll, "jtpt");
        if (!mapper_.isMissing(ptBr)) connectBranch(ptBr, jet_pt_arr_[coll]);
        
        std::string etaBr = mapper_.getJetBranch(coll, "jteta");
        if (!mapper_.isMissing(etaBr)) connectBranch(etaBr, jet_eta_arr_[coll]);
        
        std::string phiBr = mapper_.getJetBranch(coll, "jtphi");
        if (!mapper_.isMissing(phiBr)) connectBranch(phiBr, jet_phi_arr_[coll]);
        
        std::string massBr = mapper_.getJetBranch(coll, "jtm");
        if (!mapper_.isMissing(massBr)) connectBranch(massBr, jet_mass_arr_[coll]);
        
        std::string areaBr = mapper_.getJetBranch(coll, "jtarea");
        if (!mapper_.isMissing(areaBr)) connectBranch(areaBr, jet_area_arr_[coll]);
        
        // Reco jet dynamical grooming
        std::string splitBr = mapper_.getSubstructureBranch(coll, "jtdyn_split");
        if (!mapper_.isMissing(splitBr)) connectBranch(splitBr, jet_split_arr_[coll]);
        
        std::string zBr = mapper_.getSubstructureBranch(coll, "jtdyn_z");
        if (!mapper_.isMissing(zBr)) connectBranch(zBr, jet_z_arr_[coll]);
        
        std::string ktBr = mapper_.getSubstructureBranch(coll, "jtdyn_kt");
        if (!mapper_.isMissing(ktBr)) connectBranch(ktBr, jet_kt_arr_[coll]);
        
        std::string drBr = mapper_.getSubstructureBranch(coll, "jtdyn_deltaR");
        if (!mapper_.isMissing(drBr)) connectBranch(drBr, jet_dr_arr_[coll]);
        
        // Reco jet soft substructure
        std::string girthBr = mapper_.getSubstructureBranch(coll, "jt_girth");
        if (!mapper_.isMissing(girthBr)) connectBranch(girthBr, jet_girth_arr_[coll]);
        
        std::string thrustBr = mapper_.getSubstructureBranch(coll, "jt_thrust");
        if (!mapper_.isMissing(thrustBr)) connectBranch(thrustBr, jet_thrust_arr_[coll]);
        
        std::string lhaBr = mapper_.getSubstructureBranch(coll, "jt_LHA");
        if (!mapper_.isMissing(lhaBr)) connectBranch(lhaBr, jet_lha_arr_[coll]);
        
        std::string ptdBr = mapper_.getSubstructureBranch(coll, "jt_pTD");
        if (!mapper_.isMissing(ptdBr)) connectBranch(ptdBr, jet_ptd_arr_[coll]);
        
        std::string tauFormBr = mapper_.getSubstructureBranch(coll, "jt_tau_form");
        if (!mapper_.isMissing(tauFormBr)) connectBranch(tauFormBr, jet_tau_form_arr_[coll]);
        
        std::string intjetMultiBr = mapper_.getSubstructureBranch(coll, "jt_intjet_multi");
        if (!mapper_.isMissing(intjetMultiBr)) connectBranch(intjetMultiBr, jet_intjet_multi_arr_[coll]);
        
        // Ref jet (MC truth) kinematics
        std::string refPtBr = mapper_.getJetBranch(coll, "refpt");
        if (!mapper_.isMissing(refPtBr)) connectBranch(refPtBr, ref_pt_arr_[coll]);
        
        std::string refEtaBr = mapper_.getJetBranch(coll, "refeta");
        if (!mapper_.isMissing(refEtaBr)) connectBranch(refEtaBr, ref_eta_arr_[coll]);
        
        std::string refPhiBr = mapper_.getJetBranch(coll, "refphi");
        if (!mapper_.isMissing(refPhiBr)) connectBranch(refPhiBr, ref_phi_arr_[coll]);
        
        std::string refMassBr = mapper_.getJetBranch(coll, "refm");
        if (!mapper_.isMissing(refMassBr)) connectBranch(refMassBr, ref_mass_arr_[coll]);
        
        std::string refAreaBr = mapper_.getJetBranch(coll, "refarea");
        if (!mapper_.isMissing(refAreaBr)) connectBranch(refAreaBr, ref_area_arr_[coll]);
        
        // Ref jet dynamical grooming
        std::string refSplitBr = mapper_.getSubstructureBranch(coll, "refdyn_split");
        if (!mapper_.isMissing(refSplitBr)) connectBranch(refSplitBr, ref_split_arr_[coll]);
        
        std::string refZBr = mapper_.getSubstructureBranch(coll, "refdyn_z");
        if (!mapper_.isMissing(refZBr)) connectBranch(refZBr, ref_z_arr_[coll]);
        
        std::string refKtBr = mapper_.getSubstructureBranch(coll, "refdyn_kt");
        if (!mapper_.isMissing(refKtBr)) connectBranch(refKtBr, ref_kt_arr_[coll]);
        
        std::string refDrBr = mapper_.getSubstructureBranch(coll, "refdyn_deltaR");
        if (!mapper_.isMissing(refDrBr)) connectBranch(refDrBr, ref_dr_arr_[coll]);
        
        // Ref jet soft substructure
        std::string refGirthBr = mapper_.getSubstructureBranch(coll, "ref_girth");
        if (!mapper_.isMissing(refGirthBr)) connectBranch(refGirthBr, ref_girth_arr_[coll]);
        
        std::string refThrustBr = mapper_.getSubstructureBranch(coll, "ref_thrust");
        if (!mapper_.isMissing(refThrustBr)) connectBranch(refThrustBr, ref_thrust_arr_[coll]);
        
        std::string refLhaBr = mapper_.getSubstructureBranch(coll, "ref_LHA");
        if (!mapper_.isMissing(refLhaBr)) connectBranch(refLhaBr, ref_lha_arr_[coll]);
        
        std::string refPtdBr = mapper_.getSubstructureBranch(coll, "ref_pTD");
        if (!mapper_.isMissing(refPtdBr)) connectBranch(refPtdBr, ref_ptd_arr_[coll]);
        
        std::string refTauFormBr = mapper_.getSubstructureBranch(coll, "ref_tau_form");
        if (!mapper_.isMissing(refTauFormBr)) connectBranch(refTauFormBr, ref_tau_form_arr_[coll]);
        
        std::string refIntjetMultiBr = mapper_.getSubstructureBranch(coll, "ref_intjet_multi");
        if (!mapper_.isMissing(refIntjetMultiBr)) connectBranch(refIntjetMultiBr, ref_intjet_multi_arr_[coll]);
    }
    
    log("INFO", "Initialization complete!");
    return true;
}

bool UnifiedDataReader::loadEntry(Long64_t entry) {
    if (entry < 0 || entry >= chain_->GetEntries()) {
        log("ERROR", "Invalid entry: " + std::to_string(entry));
        return false;
    }
    
    chain_->GetEntry(entry);
    
    // Copy photon data to vectors
    photonEt_.clear();
    photonEta_.clear();
    photonPhi_.clear();
    photonHoverE_.clear();
    photonSigmaIEtaIEta_.clear();
    photonSigmaEtaEta_.clear();
    photonECALIso_.clear();
    photonHCALIso_.clear();
    photonTrackIso_.clear();
    photonPFCIso_.clear();
    photonPFNIso_.clear();
    photonPFPIso_.clear();
    photonR9_.clear();
    
    // Clear MC truth vectors
    photonGenMatchedIndex_.clear();
    mcPID_.clear();
    mcMomPID_.clear();
    mcPt_.clear();
    mcEta_.clear();
    mcPhi_.clear();
    mcCalIsoDR04_.clear();
    
    if (isScalarPhoton_) {
        // 2018: Single photon as scalars
        photonEt_.push_back(pho_et_s_);
        photonEta_.push_back(pho_eta_s_);
        photonPhi_.push_back(pho_phi_s_);
        photonHoverE_.push_back(pho_hoe_s_);
        photonSigmaIEtaIEta_.push_back(pho_sieie_s_);
        photonSigmaEtaEta_.push_back(pho_see_s_);
        photonECALIso_.push_back(pho_ecal_s_);
        photonHCALIso_.push_back(pho_hcal_s_);
        photonTrackIso_.push_back(pho_trk_s_);
        photonPFCIso_.push_back(pho_pfc_s_);
        photonPFNIso_.push_back(pho_pfn_s_);
        photonPFPIso_.push_back(pho_pfp_s_);
    } else {
        // 2023/2024: Multiple photons as RVec
        if (pho_et_v_) {
            for (size_t i = 0; i < pho_et_v_->size(); i++) {
                photonEt_.push_back((*pho_et_v_)[i]);
                if (pho_eta_v_) photonEta_.push_back((*pho_eta_v_)[i]);
                if (pho_phi_v_) photonPhi_.push_back((*pho_phi_v_)[i]);
                if (pho_hoe_v_) photonHoverE_.push_back((*pho_hoe_v_)[i]);
                if (pho_sieie_v_) photonSigmaIEtaIEta_.push_back((*pho_sieie_v_)[i]);
                if (pho_see_v_) photonSigmaEtaEta_.push_back((*pho_see_v_)[i]);
                if (pho_ecal_v_) photonECALIso_.push_back((*pho_ecal_v_)[i]);
                if (pho_hcal_v_) photonHCALIso_.push_back((*pho_hcal_v_)[i]);
                if (pho_trk_v_) photonTrackIso_.push_back((*pho_trk_v_)[i]);
                if (pho_pfc_v_) photonPFCIso_.push_back((*pho_pfc_v_)[i]);
                if (pho_pfn_v_) photonPFNIso_.push_back((*pho_pfn_v_)[i]);
                if (pho_pfp_v_) photonPFPIso_.push_back((*pho_pfp_v_)[i]);
                if (pho_r9_v_) photonR9_.push_back((*pho_r9_v_)[i]);
            }
        }
    }
    
    // Copy MC truth particles (if available, for RVec format)
    if (!isScalarPhoton_) {
        if (pho_genMatchedIndex_v_ && pho_genMatchedIndex_v_->size() > 0) {
            for (size_t i = 0; i < pho_genMatchedIndex_v_->size(); i++) {
                photonGenMatchedIndex_.push_back((*pho_genMatchedIndex_v_)[i]);
            }
        }
        
        if (mcPID_v_ && mcPID_v_->size() > 0) {
            for (size_t i = 0; i < mcPID_v_->size(); i++) {
                mcPID_.push_back((*mcPID_v_)[i]);
                if (mcMomPID_v_ && i < mcMomPID_v_->size()) mcMomPID_.push_back((*mcMomPID_v_)[i]);
                if (mcPt_v_ && i < mcPt_v_->size()) mcPt_.push_back((*mcPt_v_)[i]);
                if (mcEta_v_ && i < mcEta_v_->size()) mcEta_.push_back((*mcEta_v_)[i]);
                if (mcPhi_v_ && i < mcPhi_v_->size()) mcPhi_.push_back((*mcPhi_v_)[i]);
                if (mcCalIsoDR04_v_ && i < mcCalIsoDR04_v_->size()) mcCalIsoDR04_.push_back((*mcCalIsoDR04_v_)[i]);
            }
        }
    }
    
    // Copy jet data to vectors
    for (const auto& coll : jetCollections_) {
        int nJets = jetNref_[coll];
        
        // Clear all reco jet vectors
        jetPt_[coll].clear();
        jetEta_[coll].clear();
        jetPhi_[coll].clear();
        jetMass_[coll].clear();
        jetArea_[coll].clear();
        jetDynSplit_[coll].clear();
        jetDynZ_[coll].clear();
        jetDynKt_[coll].clear();
        jetDynDeltaR_[coll].clear();
        jetGirth_[coll].clear();
        jetThrust_[coll].clear();
        jetLHA_[coll].clear();
        jetPtD_[coll].clear();
        jetTauForm_[coll].clear();
        jetIntJetMulti_[coll].clear();
        
        // Clear all ref jet vectors
        refJetPt_[coll].clear();
        refJetEta_[coll].clear();
        refJetPhi_[coll].clear();
        refJetMass_[coll].clear();
        refJetArea_[coll].clear();
        refJetDynSplit_[coll].clear();
        refJetDynZ_[coll].clear();
        refJetDynKt_[coll].clear();
        refJetDynDeltaR_[coll].clear();
        refJetGirth_[coll].clear();
        refJetThrust_[coll].clear();
        refJetLHA_[coll].clear();
        refJetPtD_[coll].clear();
        refJetTauForm_[coll].clear();
        refJetIntJetMulti_[coll].clear();
        
        // Copy data from arrays to vectors
        for (int i = 0; i < nJets && i < MAXJETS; i++) {
            // Reco jets
            jetPt_[coll].push_back(jet_pt_arr_[coll][i]);
            jetEta_[coll].push_back(jet_eta_arr_[coll][i]);
            jetPhi_[coll].push_back(jet_phi_arr_[coll][i]);
            jetMass_[coll].push_back(jet_mass_arr_[coll][i]);
            jetArea_[coll].push_back(jet_area_arr_[coll][i]);
            jetDynSplit_[coll].push_back(jet_split_arr_[coll][i]);
            jetDynZ_[coll].push_back(jet_z_arr_[coll][i]);
            jetDynKt_[coll].push_back(jet_kt_arr_[coll][i]);
            jetDynDeltaR_[coll].push_back(jet_dr_arr_[coll][i]);
            jetGirth_[coll].push_back(jet_girth_arr_[coll][i]);
            jetThrust_[coll].push_back(jet_thrust_arr_[coll][i]);
            jetLHA_[coll].push_back(jet_lha_arr_[coll][i]);
            jetPtD_[coll].push_back(jet_ptd_arr_[coll][i]);
            jetTauForm_[coll].push_back(jet_tau_form_arr_[coll][i]);
            jetIntJetMulti_[coll].push_back(jet_intjet_multi_arr_[coll][i]);
            
            // Ref jets (MC truth)
            refJetPt_[coll].push_back(ref_pt_arr_[coll][i]);
            refJetEta_[coll].push_back(ref_eta_arr_[coll][i]);
            refJetPhi_[coll].push_back(ref_phi_arr_[coll][i]);
            refJetMass_[coll].push_back(ref_mass_arr_[coll][i]);
            refJetArea_[coll].push_back(ref_area_arr_[coll][i]);
            refJetDynSplit_[coll].push_back(ref_split_arr_[coll][i]);
            refJetDynZ_[coll].push_back(ref_z_arr_[coll][i]);
            refJetDynKt_[coll].push_back(ref_kt_arr_[coll][i]);
            refJetDynDeltaR_[coll].push_back(ref_dr_arr_[coll][i]);
            refJetGirth_[coll].push_back(ref_girth_arr_[coll][i]);
            refJetThrust_[coll].push_back(ref_thrust_arr_[coll][i]);
            refJetLHA_[coll].push_back(ref_lha_arr_[coll][i]);
            refJetPtD_[coll].push_back(ref_ptd_arr_[coll][i]);
            refJetTauForm_[coll].push_back(ref_tau_form_arr_[coll][i]);
            refJetIntJetMulti_[coll].push_back(ref_intjet_multi_arr_[coll][i]);
        }
    }
    
    return true;
}

Long64_t UnifiedDataReader::getEntries() const {
    return chain_->GetEntries();
}

int UnifiedDataReader::getNJets(const std::string& coll) const {
    auto it = jetPt_.find(coll);
    if (it == jetPt_.end()) return 0;
    return it->second.size();
}

float UnifiedDataReader::getJetPt(const std::string& coll, int i) const {
    auto it = jetPt_.find(coll);
    if (it == jetPt_.end() || i < 0 || i >= (int)it->second.size()) return -999;
    return it->second[i];
}

float UnifiedDataReader::getJetEta(const std::string& coll, int i) const {
    auto it = jetEta_.find(coll);
    if (it == jetEta_.end() || i < 0 || i >= (int)it->second.size()) return -999;
    return it->second[i];
}

float UnifiedDataReader::getJetPhi(const std::string& coll, int i) const {
    auto it = jetPhi_.find(coll);
    if (it == jetPhi_.end() || i < 0 || i >= (int)it->second.size()) return -999;
    return it->second[i];
}

int UnifiedDataReader::getJetDynSplit(const std::string& coll, int i) const {
    auto it = jetDynSplit_.find(coll);
    if (it == jetDynSplit_.end() || i < 0 || i >= (int)it->second.size()) return -999;
    return it->second[i];
}

float UnifiedDataReader::getJetDynZ(const std::string& coll, int i) const {
    auto it = jetDynZ_.find(coll);
    if (it == jetDynZ_.end() || i < 0 || i >= (int)it->second.size()) return -999;
    return it->second[i];
}

float UnifiedDataReader::getJetDynKt(const std::string& coll, int i) const {
    auto it = jetDynKt_.find(coll);
    if (it == jetDynKt_.end() || i < 0 || i >= (int)it->second.size()) return -999;
    return it->second[i];
}

float UnifiedDataReader::getJetDynDeltaR(const std::string& coll, int i) const {
    auto it = jetDynDeltaR_.find(coll);
    if (it == jetDynDeltaR_.end() || i < 0 || i >= (int)it->second.size()) return -999;
    return it->second[i];
}

// Additional reco jet methods
float UnifiedDataReader::getJetMass(const std::string& coll, int i) const {
    auto it = jetMass_.find(coll);
    if (it == jetMass_.end() || i < 0 || i >= (int)it->second.size()) return -999;
    return it->second[i];
}

float UnifiedDataReader::getJetArea(const std::string& coll, int i) const {
    auto it = jetArea_.find(coll);
    if (it == jetArea_.end() || i < 0 || i >= (int)it->second.size()) return -999;
    return it->second[i];
}

float UnifiedDataReader::getJetGirth(const std::string& coll, int i) const {
    auto it = jetGirth_.find(coll);
    if (it == jetGirth_.end() || i < 0 || i >= (int)it->second.size()) return -999;
    return it->second[i];
}

float UnifiedDataReader::getJetThrust(const std::string& coll, int i) const {
    auto it = jetThrust_.find(coll);
    if (it == jetThrust_.end() || i < 0 || i >= (int)it->second.size()) return -999;
    return it->second[i];
}

float UnifiedDataReader::getJetLHA(const std::string& coll, int i) const {
    auto it = jetLHA_.find(coll);
    if (it == jetLHA_.end() || i < 0 || i >= (int)it->second.size()) return -999;
    return it->second[i];
}

float UnifiedDataReader::getJetPtD(const std::string& coll, int i) const {
    auto it = jetPtD_.find(coll);
    if (it == jetPtD_.end() || i < 0 || i >= (int)it->second.size()) return -999;
    return it->second[i];
}

float UnifiedDataReader::getJetTauForm(const std::string& coll, int i) const {
    auto it = jetTauForm_.find(coll);
    if (it == jetTauForm_.end() || i < 0 || i >= (int)it->second.size()) return -999;
    return it->second[i];
}

int UnifiedDataReader::getJetIntJetMulti(const std::string& coll, int i) const {
    auto it = jetIntJetMulti_.find(coll);
    if (it == jetIntJetMulti_.end() || i < 0 || i >= (int)it->second.size()) return -999;
    return it->second[i];
}

// Ref jet (MC truth) accessor methods
float UnifiedDataReader::getRefJetPt(const std::string& coll, int i) const {
    auto it = refJetPt_.find(coll);
    if (it == refJetPt_.end() || i < 0 || i >= (int)it->second.size()) return -999;
    return it->second[i];
}

float UnifiedDataReader::getRefJetEta(const std::string& coll, int i) const {
    auto it = refJetEta_.find(coll);
    if (it == refJetEta_.end() || i < 0 || i >= (int)it->second.size()) return -999;
    return it->second[i];
}

float UnifiedDataReader::getRefJetPhi(const std::string& coll, int i) const {
    auto it = refJetPhi_.find(coll);
    if (it == refJetPhi_.end() || i < 0 || i >= (int)it->second.size()) return -999;
    return it->second[i];
}

float UnifiedDataReader::getRefJetMass(const std::string& coll, int i) const {
    auto it = refJetMass_.find(coll);
    if (it == refJetMass_.end() || i < 0 || i >= (int)it->second.size()) return -999;
    return it->second[i];
}

float UnifiedDataReader::getRefJetArea(const std::string& coll, int i) const {
    auto it = refJetArea_.find(coll);
    if (it == refJetArea_.end() || i < 0 || i >= (int)it->second.size()) return -999;
    return it->second[i];
}

int UnifiedDataReader::getRefJetDynSplit(const std::string& coll, int i) const {
    auto it = refJetDynSplit_.find(coll);
    if (it == refJetDynSplit_.end() || i < 0 || i >= (int)it->second.size()) return -999;
    return it->second[i];
}

float UnifiedDataReader::getRefJetDynZ(const std::string& coll, int i) const {
    auto it = refJetDynZ_.find(coll);
    if (it == refJetDynZ_.end() || i < 0 || i >= (int)it->second.size()) return -999;
    return it->second[i];
}

float UnifiedDataReader::getRefJetDynKt(const std::string& coll, int i) const {
    auto it = refJetDynKt_.find(coll);
    if (it == refJetDynKt_.end() || i < 0 || i >= (int)it->second.size()) return -999;
    return it->second[i];
}

float UnifiedDataReader::getRefJetDynDeltaR(const std::string& coll, int i) const {
    auto it = refJetDynDeltaR_.find(coll);
    if (it == refJetDynDeltaR_.end() || i < 0 || i >= (int)it->second.size()) return -999;
    return it->second[i];
}

float UnifiedDataReader::getRefJetGirth(const std::string& coll, int i) const {
    auto it = refJetGirth_.find(coll);
    if (it == refJetGirth_.end() || i < 0 || i >= (int)it->second.size()) return -999;
    return it->second[i];
}

float UnifiedDataReader::getRefJetThrust(const std::string& coll, int i) const {
    auto it = refJetThrust_.find(coll);
    if (it == refJetThrust_.end() || i < 0 || i >= (int)it->second.size()) return -999;
    return it->second[i];
}

float UnifiedDataReader::getRefJetLHA(const std::string& coll, int i) const {
    auto it = refJetLHA_.find(coll);
    if (it == refJetLHA_.end() || i < 0 || i >= (int)it->second.size()) return -999;
    return it->second[i];
}

float UnifiedDataReader::getRefJetPtD(const std::string& coll, int i) const {
    auto it = refJetPtD_.find(coll);
    if (it == refJetPtD_.end() || i < 0 || i >= (int)it->second.size()) return -999;
    return it->second[i];
}

float UnifiedDataReader::getRefJetTauForm(const std::string& coll, int i) const {
    auto it = refJetTauForm_.find(coll);
    if (it == refJetTauForm_.end() || i < 0 || i >= (int)it->second.size()) return -999;
    return it->second[i];
}

int UnifiedDataReader::getRefJetIntJetMulti(const std::string& coll, int i) const {
    auto it = refJetIntJetMulti_.find(coll);
    if (it == refJetIntJetMulti_.end() || i < 0 || i >= (int)it->second.size()) return -999;
    return it->second[i];
}
