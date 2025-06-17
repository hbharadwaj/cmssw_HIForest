// TUnfold-based Jet Substructure Unfolding for CMS Heavy Ion Analysis
// Author: CMS Heavy Ion Team
// Date: June 2025
// 
// This macro performs 1D, 2D, and 3D unfolding of jet substructure observables
// using ROOT's TUnfold framework for CMS Heavy Ion gamma-jet analysis.
//
// Usage: 
// 1. Setup CMSSW environment: cmsenv (to get ROOT 6.26.11 with TUnfold)
// 2. root -l
// 3. .x TUnfoldJetSubstructure.C("../configs/UnfoldJetSub_xj_test.config")
//
// Requirements:
// - ROOT 6.26.11 or earlier (TUnfold removed in ROOT 6.30+)
// - CMSSW environment for compatible ROOT version

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TH3D.h>
#include <TEnv.h>
#include <TSystem.h>
#include <TStopwatch.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TUnfold.h>
#include <TUnfoldDensity.h>
#include <TUnfoldBinning.h>
#include <TMatrixD.h>

// Logging levels
enum LogLevel { LOG_ERROR = 0, LOG_WARNING = 1, LOG_INFO = 2, LOG_DEBUG = 3 };
int gVerbosity = LOG_INFO;

void log(LogLevel level, const std::string& message) {
    if (level <= gVerbosity) {
        const char* prefix = "";
        switch (level) {
            case LOG_ERROR:   prefix = "[ERROR]   "; break;
            case LOG_WARNING: prefix = "[WARNING] "; break;
            case LOG_INFO:    prefix = "[INFO]    "; break;
            case LOG_DEBUG:   prefix = "[DEBUG]   "; break;
        }
        std::cout << prefix << message << std::endl;
    }
}

// Structure to hold unfolding binning
struct UnfoldingBins {
    std::vector<double> jetPt = {40, 80, 120, 200};
    std::vector<double> jetGirth = {0.0, 0.02, 0.04, 0.06, 0.08, 0.1, 0.12};
    std::vector<double> photonEt = {80, 100, 150, 500};
    
    int nJetPt() const { return jetPt.size() - 1; }
    int nJetGirth() const { return jetGirth.size() - 1; }
    int nPhotonEt() const { return photonEt.size() - 1; }
    
    void print() const {
        log(LOG_INFO, "Binning configuration:");
        log(LOG_INFO, "  Jet pT bins: " + std::to_string(nJetPt()) + " bins");
        log(LOG_INFO, "  Jet girth bins: " + std::to_string(nJetGirth()) + " bins");
        log(LOG_INFO, "  Photon ET bins: " + std::to_string(nPhotonEt()) + " bins");
    }
};

// Helper function to get global bin index for 2D case
int getGlobalBin2D(int iBinX, int iBinY, int nBinsX) {
    return iBinY * nBinsX + iBinX;
}

// Helper function to get global bin index for 3D case
int getGlobalBin3D(int iBinX, int iBinY, int iBinZ, int nBinsX, int nBinsY) {
    return iBinZ * (nBinsX * nBinsY) + iBinY * nBinsX + iBinX;
}

// 1D Unfolding: Jet pT
class JetPtUnfolder {
private:
    UnfoldingBins bins;
    TH1D* h_measured;
    TH1D* h_truth;
    TH2D* h_response;
    TUnfold* unfold;
    
public:
    JetPtUnfolder(const UnfoldingBins& b) : bins(b), unfold(nullptr) {
        // Create histograms
        h_measured = new TH1D("h_jetPt_measured", "Measured Jet p_{T};Jet p_{T} [GeV];Events", 
                             bins.nJetPt(), &bins.jetPt[0]);
        h_truth = new TH1D("h_jetPt_truth", "Truth Jet p_{T};Jet p_{T} [GeV];Events", 
                          bins.nJetPt(), &bins.jetPt[0]);
        h_response = new TH2D("h_jetPt_response", "Response Matrix;Truth Jet p_{T} [GeV];Measured Jet p_{T} [GeV]", 
                             bins.nJetPt(), &bins.jetPt[0], bins.nJetPt(), &bins.jetPt[0]);
        
        log(LOG_INFO, "Created 1D unfolding histograms for jet pT");
    }
    
    void fillFromTree(TTree* tree, bool isMC = false) {
        Float_t jetPt, refJetPt = -999;
        tree->SetBranchAddress("jetPt_AK2Z2", &jetPt);
        if (isMC) tree->SetBranchAddress("refJetPt_AK2Z2", &refJetPt);
        
        Long64_t nEntries = tree->GetEntries();
        log(LOG_INFO, "Processing " + std::to_string(nEntries) + " entries for 1D jet pT unfolding");
        
        for (Long64_t i = 0; i < nEntries; i++) {
            tree->GetEntry(i);
            
            if (jetPt > bins.jetPt.front() && jetPt < bins.jetPt.back()) {
                h_measured->Fill(jetPt);
                
                if (isMC && refJetPt > bins.jetPt.front() && refJetPt < bins.jetPt.back()) {
                    h_truth->Fill(refJetPt);
                    h_response->Fill(refJetPt, jetPt);
                }
            }
        }
        
        log(LOG_INFO, "Filled 1D histograms: measured=" + std::to_string(h_measured->GetEntries()) + 
                     ", truth=" + std::to_string(h_truth->GetEntries()) + 
                     ", response=" + std::to_string(h_response->GetEntries()));
    }
    
    TH1D* performUnfolding(double tau = 0.001) {
        if (!h_response || h_response->GetEntries() == 0) {
            log(LOG_ERROR, "No response matrix for 1D unfolding!");
            return nullptr;
        }
        
        log(LOG_INFO, "Performing 1D TUnfold with tau=" + std::to_string(tau));
        
        unfold = new TUnfold(h_response, TUnfold::kHistMapOutputVert, TUnfold::kRegModeSize);
        unfold->SetInput(h_measured);
        
        Int_t result = unfold->DoUnfold(tau);
        if (result >= 10000) {
            log(LOG_WARNING, "TUnfold warning: " + std::to_string(result));
        }
        
        // Create output histogram and get results
        TH1D* h_unfolded = new TH1D("h_jetPt_unfolded", "Unfolded Jet p_{T};Jet p_{T} [GeV];Events", 
                                   bins.nJetPt(), &bins.jetPt[0]);
        unfold->GetOutput(h_unfolded);
        
        if (h_unfolded) {
            log(LOG_INFO, "1D unfolding successful! Unfolded integral: " + std::to_string(h_unfolded->Integral()));
        }
        
        return h_unfolded;
    }
    
    TH1D* getMeasured() { return h_measured; }
    TH1D* getTruth() { return h_truth; }
    TH2D* getResponse() { return h_response; }
    
    ~JetPtUnfolder() {
        delete unfold;
    }
};

// 2D Unfolding: Jet pT vs Girth
class JetPtGirthUnfolder {
private:
    UnfoldingBins bins;
    TH2D* h_measured;
    TH2D* h_truth;
    TH2D* h_response;
    TUnfold* unfold;
    
public:
    JetPtGirthUnfolder(const UnfoldingBins& b) : bins(b), unfold(nullptr) {
        // Create 2D histograms
        h_measured = new TH2D("h_jetPt_girth_measured", "Measured;Jet p_{T} [GeV];Jet Girth", 
                             bins.nJetPt(), &bins.jetPt[0], bins.nJetGirth(), &bins.jetGirth[0]);
        h_truth = new TH2D("h_jetPt_girth_truth", "Truth;Jet p_{T} [GeV];Jet Girth", 
                          bins.nJetPt(), &bins.jetPt[0], bins.nJetGirth(), &bins.jetGirth[0]);
        
        // Response matrix: truth bins vs measured bins (flattened)
        int nBins2D = bins.nJetPt() * bins.nJetGirth();
        h_response = new TH2D("h_jetPt_girth_response", "Response Matrix 2D;Truth bin;Measured bin", 
                             nBins2D, 0, nBins2D, nBins2D, 0, nBins2D);
        
        log(LOG_INFO, "Created 2D unfolding histograms for jet pT vs girth (" + 
                     std::to_string(nBins2D) + " bins total)");
    }
    
    void fillFromTree(TTree* tree, bool isMC = false) {
        Float_t jetPt, jetGirth, refJetPt = -999, refJetGirth = -999;
        tree->SetBranchAddress("jetPt_AK2Z2", &jetPt);
        tree->SetBranchAddress("jetGirth_AK2Z2", &jetGirth);
        if (isMC) {
            tree->SetBranchAddress("refJetPt_AK2Z2", &refJetPt);
            tree->SetBranchAddress("refJetGirth_AK2Z2", &refJetGirth);
        }
        
        Long64_t nEntries = tree->GetEntries();
        log(LOG_INFO, "Processing " + std::to_string(nEntries) + " entries for 2D jet pT vs girth unfolding");
        
        for (Long64_t i = 0; i < nEntries; i++) {
            tree->GetEntry(i);
            
            if (jetPt > bins.jetPt.front() && jetPt < bins.jetPt.back() &&
                jetGirth > bins.jetGirth.front() && jetGirth < bins.jetGirth.back()) {
                
                h_measured->Fill(jetPt, jetGirth);
                
                if (isMC && refJetPt > bins.jetPt.front() && refJetPt < bins.jetPt.back() &&
                    refJetGirth > bins.jetGirth.front() && refJetGirth < bins.jetGirth.back()) {
                    
                    h_truth->Fill(refJetPt, refJetGirth);
                    
                    // Fill response matrix with global bin indices
                    int measBinX = h_measured->GetXaxis()->FindBin(jetPt) - 1;
                    int measBinY = h_measured->GetYaxis()->FindBin(jetGirth) - 1;
                    int truthBinX = h_truth->GetXaxis()->FindBin(refJetPt) - 1;
                    int truthBinY = h_truth->GetYaxis()->FindBin(refJetGirth) - 1;
                    
                    if (measBinX >= 0 && measBinX < bins.nJetPt() && measBinY >= 0 && measBinY < bins.nJetGirth() &&
                        truthBinX >= 0 && truthBinX < bins.nJetPt() && truthBinY >= 0 && truthBinY < bins.nJetGirth()) {
                        
                        int globalTruthBin = getGlobalBin2D(truthBinX, truthBinY, bins.nJetPt());
                        int globalMeasBin = getGlobalBin2D(measBinX, measBinY, bins.nJetPt());
                        
                        h_response->Fill(globalTruthBin, globalMeasBin);
                    }
                }
            }
        }
        
        log(LOG_INFO, "Filled 2D histograms: measured=" + std::to_string(h_measured->GetEntries()) + 
                     ", truth=" + std::to_string(h_truth->GetEntries()) + 
                     ", response=" + std::to_string(h_response->GetEntries()));
    }
    
    TH2D* performUnfolding(double tau = 0.001) {
        if (!h_response || h_response->GetEntries() == 0) {
            log(LOG_ERROR, "No response matrix for 2D unfolding!");
            return nullptr;
        }
        
        log(LOG_INFO, "Performing 2D TUnfold with tau=" + std::to_string(tau));
        
        // Convert 2D measured histogram to 1D for TUnfold
        int nBins2D = bins.nJetPt() * bins.nJetGirth();
        TH1D* h_measured_1d = new TH1D("h_measured_2d_flat", "Measured 2D flattened", nBins2D, 0, nBins2D);
        
        for (int ix = 0; ix < bins.nJetPt(); ix++) {
            for (int iy = 0; iy < bins.nJetGirth(); iy++) {
                int globalBin = getGlobalBin2D(ix, iy, bins.nJetPt());
                double content = h_measured->GetBinContent(ix+1, iy+1);
                h_measured_1d->SetBinContent(globalBin+1, content);
            }
        }
        
        unfold = new TUnfold(h_response, TUnfold::kHistMapOutputVert, TUnfold::kRegModeSize);
        unfold->SetInput(h_measured_1d);
        
        Int_t result = unfold->DoUnfold(tau);
        if (result >= 10000) {
            log(LOG_WARNING, "TUnfold warning: " + std::to_string(result));
        }
        
        // Create output histogram and get results
        TH1D* h_unfolded_1d = new TH1D("h_unfolded_2d_flat", "Unfolded 2D flattened", nBins2D, 0, nBins2D);
        unfold->GetOutput(h_unfolded_1d);
        
        // Convert back to 2D
        TH2D* h_unfolded_2d = new TH2D("h_jetPt_girth_unfolded", "Unfolded;Jet p_{T} [GeV];Jet Girth", 
                                      bins.nJetPt(), &bins.jetPt[0], bins.nJetGirth(), &bins.jetGirth[0]);
        
        if (h_unfolded_1d) {
            for (int ix = 0; ix < bins.nJetPt(); ix++) {
                for (int iy = 0; iy < bins.nJetGirth(); iy++) {
                    int globalBin = getGlobalBin2D(ix, iy, bins.nJetPt());
                    double content = h_unfolded_1d->GetBinContent(globalBin+1);
                    h_unfolded_2d->SetBinContent(ix+1, iy+1, content);
                }
            }
            log(LOG_INFO, "2D unfolding successful! Unfolded integral: " + std::to_string(h_unfolded_2d->Integral()));
        }
        
        delete h_measured_1d;
        return h_unfolded_2d;
    }
    
    TH2D* getMeasured() { return h_measured; }
    TH2D* getTruth() { return h_truth; }
    TH2D* getResponse() { return h_response; }
    
    ~JetPtGirthUnfolder() {
        delete unfold;
    }
};

// 3D Unfolding: Photon ET vs Jet pT vs Girth
class PhotonJetPtGirthUnfolder {
private:
    UnfoldingBins bins;
    TH3D* h_measured;
    TH3D* h_truth;
    TH2D* h_response;
    TUnfold* unfold;
    
public:
    PhotonJetPtGirthUnfolder(const UnfoldingBins& b) : bins(b), unfold(nullptr) {
        // Create 3D histograms
        h_measured = new TH3D("h_photon_jetPt_girth_measured", "Measured;Photon E_{T} [GeV];Jet p_{T} [GeV];Jet Girth", 
                             bins.nPhotonEt(), &bins.photonEt[0], bins.nJetPt(), &bins.jetPt[0], bins.nJetGirth(), &bins.jetGirth[0]);
        h_truth = new TH3D("h_photon_jetPt_girth_truth", "Truth;Photon E_{T} [GeV];Jet p_{T} [GeV];Jet Girth", 
                          bins.nPhotonEt(), &bins.photonEt[0], bins.nJetPt(), &bins.jetPt[0], bins.nJetGirth(), &bins.jetGirth[0]);
        
        // Response matrix: truth bins vs measured bins (flattened)
        int nBins3D = bins.nPhotonEt() * bins.nJetPt() * bins.nJetGirth();
        h_response = new TH2D("h_photon_jetPt_girth_response", "Response Matrix 3D;Truth bin;Measured bin", 
                             nBins3D, 0, nBins3D, nBins3D, 0, nBins3D);
        
        log(LOG_INFO, "Created 3D unfolding histograms for photon ET vs jet pT vs girth (" + 
                     std::to_string(nBins3D) + " bins total)");
    }
    
    void fillFromTree(TTree* tree, bool isMC = false) {
        Float_t photonEt, jetPt, jetGirth;
        Float_t MCphotonEt = -999, refJetPt = -999, refJetGirth = -999;
        
        tree->SetBranchAddress("photonEt", &photonEt);
        tree->SetBranchAddress("jetPt_AK2Z2", &jetPt);
        tree->SetBranchAddress("jetGirth_AK2Z2", &jetGirth);
        
        if (isMC) {
            tree->SetBranchAddress("MCphotonEt", &MCphotonEt);
            tree->SetBranchAddress("refJetPt_AK2Z2", &refJetPt);
            tree->SetBranchAddress("refJetGirth_AK2Z2", &refJetGirth);
        }
        
        Long64_t nEntries = tree->GetEntries();
        log(LOG_INFO, "Processing " + std::to_string(nEntries) + " entries for 3D photon ET vs jet pT vs girth unfolding");
        
        for (Long64_t i = 0; i < nEntries; i++) {
            tree->GetEntry(i);
            
            if (photonEt > bins.photonEt.front() && photonEt < bins.photonEt.back() &&
                jetPt > bins.jetPt.front() && jetPt < bins.jetPt.back() &&
                jetGirth > bins.jetGirth.front() && jetGirth < bins.jetGirth.back()) {
                
                h_measured->Fill(photonEt, jetPt, jetGirth);
                
                if (isMC && MCphotonEt > bins.photonEt.front() && MCphotonEt < bins.photonEt.back() &&
                    refJetPt > bins.jetPt.front() && refJetPt < bins.jetPt.back() &&
                    refJetGirth > bins.jetGirth.front() && refJetGirth < bins.jetGirth.back()) {
                    
                    h_truth->Fill(MCphotonEt, refJetPt, refJetGirth);
                    
                    // Fill response matrix with global bin indices
                    int measBinX = h_measured->GetXaxis()->FindBin(photonEt) - 1;
                    int measBinY = h_measured->GetYaxis()->FindBin(jetPt) - 1;
                    int measBinZ = h_measured->GetZaxis()->FindBin(jetGirth) - 1;
                    
                    int truthBinX = h_truth->GetXaxis()->FindBin(MCphotonEt) - 1;
                    int truthBinY = h_truth->GetYaxis()->FindBin(refJetPt) - 1;
                    int truthBinZ = h_truth->GetZaxis()->FindBin(refJetGirth) - 1;
                    
                    if (measBinX >= 0 && measBinX < bins.nPhotonEt() && measBinY >= 0 && measBinY < bins.nJetPt() && 
                        measBinZ >= 0 && measBinZ < bins.nJetGirth() &&
                        truthBinX >= 0 && truthBinX < bins.nPhotonEt() && truthBinY >= 0 && truthBinY < bins.nJetPt() && 
                        truthBinZ >= 0 && truthBinZ < bins.nJetGirth()) {
                        
                        int globalTruthBin = getGlobalBin3D(truthBinX, truthBinY, truthBinZ, bins.nPhotonEt(), bins.nJetPt());
                        int globalMeasBin = getGlobalBin3D(measBinX, measBinY, measBinZ, bins.nPhotonEt(), bins.nJetPt());
                        
                        h_response->Fill(globalTruthBin, globalMeasBin);
                    }
                }
            }
        }
        
        log(LOG_INFO, "Filled 3D histograms: measured=" + std::to_string(h_measured->GetEntries()) + 
                     ", truth=" + std::to_string(h_truth->GetEntries()) + 
                     ", response=" + std::to_string(h_response->GetEntries()));
    }
    
    TH3D* performUnfolding(double tau = 0.001) {
        if (!h_response || h_response->GetEntries() == 0) {
            log(LOG_ERROR, "No response matrix for 3D unfolding!");
            return nullptr;
        }
        
        log(LOG_INFO, "Performing 3D TUnfold with tau=" + std::to_string(tau));
        
        // Convert 3D measured histogram to 1D for TUnfold
        int nBins3D = bins.nPhotonEt() * bins.nJetPt() * bins.nJetGirth();
        TH1D* h_measured_1d = new TH1D("h_measured_3d_flat", "Measured 3D flattened", nBins3D, 0, nBins3D);
        
        for (int ix = 0; ix < bins.nPhotonEt(); ix++) {
            for (int iy = 0; iy < bins.nJetPt(); iy++) {
                for (int iz = 0; iz < bins.nJetGirth(); iz++) {
                    int globalBin = getGlobalBin3D(ix, iy, iz, bins.nPhotonEt(), bins.nJetPt());
                    double content = h_measured->GetBinContent(ix+1, iy+1, iz+1);
                    h_measured_1d->SetBinContent(globalBin+1, content);
                }
            }
        }
        
        unfold = new TUnfold(h_response, TUnfold::kHistMapOutputVert, TUnfold::kRegModeSize);
        unfold->SetInput(h_measured_1d);
        
        Int_t result = unfold->DoUnfold(tau);
        if (result >= 10000) {
            log(LOG_WARNING, "TUnfold warning: " + std::to_string(result));
        }
        
        // Create output histogram and get results
        TH1D* h_unfolded_1d = new TH1D("h_unfolded_3d_flat", "Unfolded 3D flattened", nBins3D, 0, nBins3D);
        unfold->GetOutput(h_unfolded_1d);
        
        // Convert back to 3D
        TH3D* h_unfolded_3d = new TH3D("h_photon_jetPt_girth_unfolded", "Unfolded;Photon E_{T} [GeV];Jet p_{T} [GeV];Jet Girth", 
                                      bins.nPhotonEt(), &bins.photonEt[0], bins.nJetPt(), &bins.jetPt[0], bins.nJetGirth(), &bins.jetGirth[0]);
        
        if (h_unfolded_1d) {
            for (int ix = 0; ix < bins.nPhotonEt(); ix++) {
                for (int iy = 0; iy < bins.nJetPt(); iy++) {
                    for (int iz = 0; iz < bins.nJetGirth(); iz++) {
                        int globalBin = getGlobalBin3D(ix, iy, iz, bins.nPhotonEt(), bins.nJetPt());
                        double content = h_unfolded_1d->GetBinContent(globalBin+1);
                        h_unfolded_3d->SetBinContent(ix+1, iy+1, iz+1, content);
                    }
                }
            }
            log(LOG_INFO, "3D unfolding successful! Unfolded integral: " + std::to_string(h_unfolded_3d->Integral()));
        }
        
        delete h_measured_1d;
        return h_unfolded_3d;
    }
    
    TH3D* getMeasured() { return h_measured; }
    TH3D* getTruth() { return h_truth; }
    TH2D* getResponse() { return h_response; }
    
    ~PhotonJetPtGirthUnfolder() {
        delete unfold;
    }
};

// Main unfolding function
void TUnfoldJetSubstructure(const char* configFile = "../configs/UnfoldJetSub_xj_test.config") {
    
    TStopwatch timer;
    timer.Start();
    
    log(LOG_INFO, "=== TUnfold Jet Substructure Unfolding ===");
    log(LOG_INFO, "Loading configuration from: " + std::string(configFile));
    
    // Load configuration
    TEnv* config = new TEnv(configFile);
    gVerbosity = config->GetValue("Verbosity", 2);
    
    // Get input files
    const char* dataInputFile = config->GetValue("DataInputFile", "");
    const char* mcInputFile = config->GetValue("MCInputFile", "");
    const char* outputDir = config->GetValue("OutputDir", "./");
    const char* outputPrefix = config->GetValue("OutputPrefix", "tunfold");
    
    // Create output directory
    gSystem->mkdir(outputDir, kTRUE);
    log(LOG_INFO, "Output directory: " + std::string(outputDir));
    
    // Setup binning
    UnfoldingBins bins;
    bins.print();
    
    // Open files
    TFile* dataFile = TFile::Open(dataInputFile);
    TFile* mcFile = TFile::Open(mcInputFile);
    
    if (!dataFile || dataFile->IsZombie()) {
        log(LOG_ERROR, "Could not open data file: " + std::string(dataInputFile));
        return;
    }
    
    if (!mcFile || mcFile->IsZombie()) {
        log(LOG_ERROR, "Could not open MC file: " + std::string(mcInputFile));
        return;
    }
    
    // Get trees
    TTree* dataTree = (TTree*)dataFile->Get("gammaJetTree");
    TTree* mcTree = (TTree*)mcFile->Get("gammaJetTree");
    
    if (!dataTree || !mcTree) {
        log(LOG_ERROR, "Could not find gammaJetTree in input files");
        return;
    }
    
    log(LOG_INFO, "Data entries: " + std::to_string(dataTree->GetEntries()));
    log(LOG_INFO, "MC entries: " + std::to_string(mcTree->GetEntries()));
    
    // === 1D Unfolding: Jet pT ===
    log(LOG_INFO, "\n=== 1D Unfolding: Jet pT ===");
    JetPtUnfolder unfolder1D(bins);
    unfolder1D.fillFromTree(dataTree, false);  // Data
    unfolder1D.fillFromTree(mcTree, true);     // MC for response
    TH1D* unfolded1D = unfolder1D.performUnfolding(0.001);
    
    // === 2D Unfolding: Jet pT vs Girth ===
    log(LOG_INFO, "\n=== 2D Unfolding: Jet pT vs Girth ===");
    JetPtGirthUnfolder unfolder2D(bins);
    unfolder2D.fillFromTree(dataTree, false);  // Data
    unfolder2D.fillFromTree(mcTree, true);     // MC for response
    TH2D* unfolded2D = unfolder2D.performUnfolding(0.001);
    
    // === 3D Unfolding: Photon ET vs Jet pT vs Girth ===
    log(LOG_INFO, "\n=== 3D Unfolding: Photon ET vs Jet pT vs Girth ===");
    PhotonJetPtGirthUnfolder unfolder3D(bins);
    unfolder3D.fillFromTree(dataTree, false);  // Data
    unfolder3D.fillFromTree(mcTree, true);     // MC for response
    TH3D* unfolded3D = unfolder3D.performUnfolding(0.001);
    
    // === Save Results ===
    std::string outputPath = std::string(outputDir) + "/" + std::string(outputPrefix) + "_tunfold.root";
    TFile* outFile = TFile::Open(outputPath.c_str(), "RECREATE");
    
    if (outFile && !outFile->IsZombie()) {
        // 1D results
        TDirectory* dir1D = outFile->mkdir("Unfolding1D");
        dir1D->cd();
        unfolder1D.getMeasured()->Write();
        unfolder1D.getTruth()->Write();
        unfolder1D.getResponse()->Write();
        if (unfolded1D) unfolded1D->Write();
        
        // 2D results
        TDirectory* dir2D = outFile->mkdir("Unfolding2D");
        dir2D->cd();
        unfolder2D.getMeasured()->Write();
        unfolder2D.getTruth()->Write();
        unfolder2D.getResponse()->Write();
        if (unfolded2D) unfolded2D->Write();
        
        // 3D results
        TDirectory* dir3D = outFile->mkdir("Unfolding3D");
        dir3D->cd();
        unfolder3D.getMeasured()->Write();
        unfolder3D.getTruth()->Write();
        unfolder3D.getResponse()->Write();
        if (unfolded3D) unfolded3D->Write();
        
        outFile->Close();
        log(LOG_INFO, "Results saved to: " + outputPath);
    } else {
        log(LOG_ERROR, "Could not create output file: " + outputPath);
    }
    
    // Cleanup
    dataFile->Close();
    mcFile->Close();
    delete config;
    
    timer.Stop();
    log(LOG_INFO, "\n=== TUnfold completed in " + std::to_string(timer.RealTime()) + " seconds ===");
}
