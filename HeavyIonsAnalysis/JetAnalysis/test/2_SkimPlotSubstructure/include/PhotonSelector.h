#ifndef PHOTON_SELECTOR_H
#define PHOTON_SELECTOR_H

#include <string>
#include <vector>

class PhotonSelector {
public:
    // Constructor with config parameters
    PhotonSelector(float etMin, float etaMax, float hoverEMax, float sigmaIEtaIEtaMax, 
                  float isoMax, float r9Min, bool requireGenMatch = false, float genMatchDRMax = 0.1)
        : etMin_(etMin), etaMax_(etaMax), hoverEMax_(hoverEMax), 
          sigmaIEtaIEtaMax_(sigmaIEtaIEtaMax), isoMax_(isoMax), r9Min_(r9Min),
          requireGenMatch_(requireGenMatch), genMatchDRMax_(genMatchDRMax) {}
    
    // Selection method for photons without MC truth
    template <typename PhotonContainer>
    std::vector<int> select(const PhotonContainer& phoEt, const PhotonContainer& phoEta,
                           const PhotonContainer& phoPhi, const PhotonContainer& phoHoverE,
                           const PhotonContainer& phoSigmaIEtaIEta, 
                           const PhotonContainer& phoIso, const PhotonContainer& phoR9,
                           int nPho) {
        std::vector<int> selectedIdx;
        for (int i = 0; i < nPho; ++i) {
            if (passesSelection(phoEt[i], phoEta[i], phoHoverE[i], 
                              phoSigmaIEtaIEta[i], phoIso[i], phoR9[i])) {
                selectedIdx.push_back(i);
            }
        }
        return selectedIdx;
    }
    
    // Selection method with MC truth matching
    template <typename PhotonContainer, typename GenContainer>
    std::vector<int> selectWithGenMatch(const PhotonContainer& phoEt, const PhotonContainer& phoEta,
                                      const PhotonContainer& phoPhi, const PhotonContainer& phoHoverE,
                                      const PhotonContainer& phoSigmaIEtaIEta, 
                                      const PhotonContainer& phoIso, const PhotonContainer& phoR9,
                                      int nPho,
                                      const GenContainer& genPt, const GenContainer& genEta,
                                      const GenContainer& genPhi, int nGen) {
        std::vector<int> selectedIdx;
        for (int i = 0; i < nPho; ++i) {
            if (!passesSelection(phoEt[i], phoEta[i], phoHoverE[i], 
                               phoSigmaIEtaIEta[i], phoIso[i], phoR9[i])) {
                continue;
            }
            
            if (requireGenMatch_) {
                bool matched = false;
                for (int j = 0; j < nGen; ++j) {
                    float dEta = phoEta[i] - genEta[j];
                    float dPhi = deltaPhi(phoPhi[i], genPhi[j]);
                    float dR = std::sqrt(dEta*dEta + dPhi*dPhi);
                    
                    if (dR < genMatchDRMax_) {
                        matched = true;
                        break;
                    }
                }
                
                if (!matched) continue;
            }
            
            selectedIdx.push_back(i);
        }
        return selectedIdx;
    }

public:
    // Check if gen matching is required
    bool requiresGenMatch() const {
        return requireGenMatch_;
    }

private:
    // Selection criteria
    float etMin_;
    float etaMax_;
    float hoverEMax_;
    float sigmaIEtaIEtaMax_;
    float isoMax_;
    float r9Min_;
    bool requireGenMatch_;
    float genMatchDRMax_;
    
    // Helper function for deltaR calculation
    float deltaPhi(float phi1, float phi2) {
        float dphi = phi1 - phi2;
        while (dphi > M_PI) dphi -= 2*M_PI;
        while (dphi <= -M_PI) dphi += 2*M_PI;
        return dphi;
    }
    
    // Core selection logic
    bool passesSelection(float et, float eta, float hoverE, 
                        float sigmaIEtaIEta, float iso, float r9) {
        if (et <= etMin_) return false;
        if (std::abs(eta) >= etaMax_) return false;
        if (hoverE >= hoverEMax_) return false;
        if (sigmaIEtaIEta >= sigmaIEtaIEtaMax_) return false;
        if (iso >= isoMax_) return false;
        if (r9 <= r9Min_) return false;
        return true;
    }
};

#endif // PHOTON_SELECTOR_H
