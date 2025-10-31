/**
 * UnifiedDataReader.h - RVec-Compatible Version
 * 
 * Works with ALL HiForest formats using direct TTree::GetEntry()
 * - 2018: Scalar photons, leaf-list jets
 * - 2023/2024: RVec photons, leaf-list jets  
 */

#ifndef UNIFIEDDATAREADER_H
#define UNIFIEDDATAREADER_H

#include <TChain.h>
#include <ROOT/RVec.hxx>
#include "BranchMapper.h"
#include <vector>
#include <map>
#include <string>

class UnifiedDataReader {
public:
    UnifiedDataReader(TChain* chain, 
                      const BranchMapper& mapper,
                      const std::vector<std::string>& jetCollections);
    ~UnifiedDataReader();
    
    bool initialize();
    bool loadEntry(Long64_t entry);
    Long64_t getEntries() const;
    
    // Event-level
    int getHiBin() const { return hiBin_; }
    float getVz() const { return vz_; }
    float getHiHF() const { return hiHF_; }
    float getRho() const { return rho_; }
    
    // Photons (uniform vector interface with bounds checking)
    int getNPhotons() const { return photonEt_.size(); }
    float getPhotonEt(int i) const { return (i >= 0 && i < (int)photonEt_.size()) ? photonEt_[i] : -999.0f; }
    float getPhotonEta(int i) const { return (i >= 0 && i < (int)photonEta_.size()) ? photonEta_[i] : -999.0f; }
    float getPhotonPhi(int i) const { return (i >= 0 && i < (int)photonPhi_.size()) ? photonPhi_[i] : -999.0f; }
    float getPhotonHoverE(int i) const { return (i >= 0 && i < (int)photonHoverE_.size()) ? photonHoverE_[i] : -999.0f; }
    float getPhotonSigmaIEtaIEta(int i) const { return (i >= 0 && i < (int)photonSigmaIEtaIEta_.size()) ? photonSigmaIEtaIEta_[i] : -999.0f; }
    float getPhotonSigmaEtaEta(int i) const { return (i >= 0 && i < (int)photonSigmaEtaEta_.size()) ? photonSigmaEtaEta_[i] : -999.0f; }
    float getPhotonECALIso(int i) const { return (i >= 0 && i < (int)photonECALIso_.size()) ? photonECALIso_[i] : -999.0f; }
    float getPhotonHCALIso(int i) const { return (i >= 0 && i < (int)photonHCALIso_.size()) ? photonHCALIso_[i] : -999.0f; }
    float getPhotonTrackIso(int i) const { return (i >= 0 && i < (int)photonTrackIso_.size()) ? photonTrackIso_[i] : -999.0f; }
    float getPhotonPFCIso(int i) const { return (i >= 0 && i < (int)photonPFCIso_.size()) ? photonPFCIso_[i] : -999.0f; }
    float getPhotonPFNIso(int i) const { return (i >= 0 && i < (int)photonPFNIso_.size()) ? photonPFNIso_[i] : -999.0f; }
    float getPhotonPFPIso(int i) const { return (i >= 0 && i < (int)photonPFPIso_.size()) ? photonPFPIso_[i] : -999.0f; }
    float getPhotonR9(int i) const { return (i >= 0 && i < (int)photonR9_.size()) ? photonR9_[i] : -999.0f; }
    
    // MC truth particles (photon gen matching)
    int getPhotonGenMatchedIndex(int i) const { return (i >= 0 && i < (int)photonGenMatchedIndex_.size()) ? photonGenMatchedIndex_[i] : -999; }
    int getMCPID(int i) const { return (i >= 0 && i < (int)mcPID_.size()) ? mcPID_[i] : -999; }
    int getMCMomPID(int i) const { return (i >= 0 && i < (int)mcMomPID_.size()) ? mcMomPID_[i] : -999; }
    float getMCPt(int i) const { return (i >= 0 && i < (int)mcPt_.size()) ? mcPt_[i] : -999.0f; }
    float getMCEta(int i) const { return (i >= 0 && i < (int)mcEta_.size()) ? mcEta_[i] : -999.0f; }
    float getMCPhi(int i) const { return (i >= 0 && i < (int)mcPhi_.size()) ? mcPhi_[i] : -999.0f; }
    float getMCCalIsoDR04(int i) const { return (i >= 0 && i < (int)mcCalIsoDR04_.size()) ? mcCalIsoDR04_[i] : -999.0f; }
    int getNMCParticles() const { return mcPID_.size(); }
    
    // MC event weights
    float getWeight() const { return weight_; }
    float getWeightPthat() const { return weightPthat_; }
    
    // Jets - Basic kinematics
    int getNJets(const std::string& coll) const;
    float getJetPt(const std::string& coll, int i) const;
    float getJetEta(const std::string& coll, int i) const;
    float getJetPhi(const std::string& coll, int i) const;
    float getJetMass(const std::string& coll, int i) const;
    float getJetArea(const std::string& coll, int i) const;
    
    // Jets - Dynamical grooming
    int getJetDynSplit(const std::string& coll, int i) const;
    float getJetDynZ(const std::string& coll, int i) const;
    float getJetDynKt(const std::string& coll, int i) const;
    float getJetDynDeltaR(const std::string& coll, int i) const;
    
    // Jets - Soft substructure
    float getJetGirth(const std::string& coll, int i) const;
    float getJetThrust(const std::string& coll, int i) const;
    float getJetLHA(const std::string& coll, int i) const;
    float getJetPtD(const std::string& coll, int i) const;
    float getJetTauForm(const std::string& coll, int i) const;
    int getJetIntJetMulti(const std::string& coll, int i) const;
    
    // Ref jets (MC truth) - Basic kinematics
    float getRefJetPt(const std::string& coll, int i) const;
    float getRefJetEta(const std::string& coll, int i) const;
    float getRefJetPhi(const std::string& coll, int i) const;
    float getRefJetMass(const std::string& coll, int i) const;
    float getRefJetArea(const std::string& coll, int i) const;
    
    // Ref jets - Dynamical grooming
    int getRefJetDynSplit(const std::string& coll, int i) const;
    float getRefJetDynZ(const std::string& coll, int i) const;
    float getRefJetDynKt(const std::string& coll, int i) const;
    float getRefJetDynDeltaR(const std::string& coll, int i) const;
    
    // Ref jets - Soft substructure
    float getRefJetGirth(const std::string& coll, int i) const;
    float getRefJetThrust(const std::string& coll, int i) const;
    float getRefJetLHA(const std::string& coll, int i) const;
    float getRefJetPtD(const std::string& coll, int i) const;
    float getRefJetTauForm(const std::string& coll, int i) const;
    int getRefJetIntJetMulti(const std::string& coll, int i) const;
    
private:
    TChain* chain_;
    const BranchMapper& mapper_;
    std::vector<std::string> jetCollections_;
    
    // Cached data
    Int_t hiBin_;
    Float_t vz_, hiHF_, rho_;
    std::vector<float> photonEt_, photonEta_, photonPhi_;
    std::vector<float> photonHoverE_, photonSigmaIEtaIEta_, photonSigmaEtaEta_;
    std::vector<float> photonECALIso_, photonHCALIso_, photonTrackIso_;
    std::vector<float> photonPFCIso_, photonPFNIso_, photonPFPIso_, photonR9_;
    
    // MC truth particles
    Int_t nMC_;
    std::vector<int> photonGenMatchedIndex_;
    std::vector<int> mcPID_, mcMomPID_;
    std::vector<float> mcPt_, mcEta_, mcPhi_, mcCalIsoDR04_;
    
    // MC event weights
    Float_t weight_, weightPthat_;
    
    std::map<std::string, Int_t> jetNref_;
    std::map<std::string, std::vector<float>> jetPt_, jetEta_, jetPhi_, jetMass_, jetArea_;
    std::map<std::string, std::vector<int>> jetDynSplit_, jetIntJetMulti_;
    std::map<std::string, std::vector<float>> jetDynZ_, jetDynKt_, jetDynDeltaR_;
    std::map<std::string, std::vector<float>> jetGirth_, jetThrust_, jetLHA_, jetPtD_, jetTauForm_;
    
    // Ref jets (MC truth)
    std::map<std::string, std::vector<float>> refJetPt_, refJetEta_, refJetPhi_, refJetMass_, refJetArea_;
    std::map<std::string, std::vector<int>> refJetDynSplit_, refJetIntJetMulti_;
    std::map<std::string, std::vector<float>> refJetDynZ_, refJetDynKt_, refJetDynDeltaR_;
    std::map<std::string, std::vector<float>> refJetGirth_, refJetThrust_, refJetLHA_, refJetPtD_, refJetTauForm_;
    
    // Branch objects
    bool isScalarPhoton_;
    
    // Scalar photon (2018)
    Float_t pho_et_s_, pho_eta_s_, pho_phi_s_, pho_hoe_s_;
    Float_t pho_sieie_s_, pho_see_s_, pho_ecal_s_, pho_hcal_s_, pho_trk_s_;
    Float_t pho_pfc_s_, pho_pfn_s_, pho_pfp_s_, pho_r9_s_;
    
    // RVec photon (2023/2024)
    Int_t nPho_;
    ROOT::VecOps::RVec<float> *pho_et_v_, *pho_eta_v_, *pho_phi_v_, *pho_hoe_v_;
    ROOT::VecOps::RVec<float> *pho_sieie_v_, *pho_see_v_, *pho_ecal_v_, *pho_hcal_v_, *pho_trk_v_;
    ROOT::VecOps::RVec<float> *pho_pfc_v_, *pho_pfn_v_, *pho_pfp_v_, *pho_r9_v_;
    
    // MC truth particles (RVec)
    ROOT::VecOps::RVec<int> *pho_genMatchedIndex_v_, *mcPID_v_, *mcMomPID_v_;
    ROOT::VecOps::RVec<float> *mcPt_v_, *mcEta_v_, *mcPhi_v_, *mcCalIsoDR04_v_;
    
    // Jets (fixed arrays)
    static const int MAXJETS = 5000;
    std::map<std::string, Float_t*> jet_pt_arr_, jet_eta_arr_, jet_phi_arr_, jet_mass_arr_, jet_area_arr_;
    std::map<std::string, Int_t*> jet_split_arr_, jet_intjet_multi_arr_;
    std::map<std::string, Float_t*> jet_z_arr_, jet_kt_arr_, jet_dr_arr_;
    std::map<std::string, Float_t*> jet_girth_arr_, jet_thrust_arr_, jet_lha_arr_, jet_ptd_arr_, jet_tau_form_arr_;
    
    // Ref jets (fixed arrays)
    std::map<std::string, Float_t*> ref_pt_arr_, ref_eta_arr_, ref_phi_arr_, ref_mass_arr_, ref_area_arr_;
    std::map<std::string, Int_t*> ref_split_arr_, ref_intjet_multi_arr_;
    std::map<std::string, Float_t*> ref_z_arr_, ref_kt_arr_, ref_dr_arr_;
    std::map<std::string, Float_t*> ref_girth_arr_, ref_thrust_arr_, ref_lha_arr_, ref_ptd_arr_, ref_tau_form_arr_;
    
    void log(const std::string& level, const std::string& msg) const;
    bool connectBranch(const std::string& name, void* addr);
};

#endif
