#ifndef GammaJetAnalysis_h
#define GammaJetAnalysis_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include "ROOT/RVec.hxx"

class GammaJetAnalysis {
protected:
    TTree* fChain = nullptr;   //!pointer to the analyzed TTree or TChain
    Int_t fCurrent = -1;       //!current Tree number in a TChain

    // Common branches for all systems
    Int_t ggHi_nPho;
    UInt_t run;
    ULong64_t evt;
    UInt_t lumi;

public:
    GammaJetAnalysis(TTree* tree = nullptr) : fChain(nullptr) {}
    virtual ~GammaJetAnalysis() {
        if (fChain) delete fChain->GetCurrentFile();
    }

    // Virtual interface
    virtual void Init(TTree* tree) = 0;
    virtual Bool_t Notify() = 0;
    virtual void Loop() = 0;
    virtual Int_t GetEntry(Long64_t entry) = 0;
    virtual Long64_t LoadTree(Long64_t entry) = 0;
    
    // Method to get event weight (for histogramming)
    // Default implementation returns 1.0 (no weighting)
    // Override in derived classes for MC-specific weighting
    virtual float getEventWeight() const { return 1.0; }
};

#endif
