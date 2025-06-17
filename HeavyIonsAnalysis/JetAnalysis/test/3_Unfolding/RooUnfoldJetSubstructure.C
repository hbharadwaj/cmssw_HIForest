// RooUnfold-based Jet Substructure Unfolding for CMS Heavy Ion Analysis
// Author: CMS Heavy Ion Team
// Date: June 2025
// 
// This macro performs 1D, 2D, and 3D unfolding of jet substructure observables
// using RooUnfold framework for CMS Heavy Ion gamma-jet analysis.
//
// Usage: 
// 1. Setup CMSSW environment: cmsenv (to get ROOT 6.26.11 with RooUnfold)
// 2. root -l
// 3. .x RooUnfoldJetSubstructure.C("../configs/UnfoldJetSub_xj_test.config")
//
// Requirements:
// - ROOT 6.26.11 or earlier
// - RooUnfold 2.0.0
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
#include <RooUnfold.h>
#include <RooUnfoldResponse.h>
#include <RooUnfoldBayes.h>
#include <RooUnfoldSvd.h>
#include <RooUnfoldBinByBin.h>
#include <RooUnfoldInvert.h>
#include <TGaxis.h>

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
    RooUnfoldResponse* response;
    RooUnfold* unfold;
    
public:
    JetPtUnfolder(const UnfoldingBins& b) : bins(b), unfold(nullptr) {
        // Create histograms
        h_measured = new TH1D("h_jetPt_measured", "Measured Jet p_{T};Jet p_{T} [GeV];Events", 
                             bins.nJetPt(), &bins.jetPt[0]);
        h_truth = new TH1D("h_jetPt_truth", "Truth Jet p_{T};Jet p_{T} [GeV];Events", 
                          bins.nJetPt(), &bins.jetPt[0]);
        
        // Create response matrix
        response = new RooUnfoldResponse(h_truth, h_measured);
        
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
                    response->Fill(refJetPt, jetPt);
                }
            }
        }
        
        log(LOG_INFO, "Filled 1D histograms: measured=" + std::to_string(h_measured->GetEntries()) + 
                     ", truth=" + std::to_string(h_truth->GetEntries()));
    }
    
    TH1D* performUnfolding(const std::string& method = "Invert", int nIter = 4) {
        if (!response) {
            log(LOG_ERROR, "No response matrix for 1D unfolding!");
            return nullptr;
        }
        
        log(LOG_INFO, "Performing 1D RooUnfold with method=" + method + (method=="Bayes" ? (", nIter="+std::to_string(nIter)) : ""));
        
        if (unfold) delete unfold;
        if (method == "Invert") {
            unfold = new RooUnfoldInvert(response, h_measured);
        } else if (method == "Bayes") {
            unfold = new RooUnfoldBayes(response, h_measured, nIter);
        } else {
            log(LOG_ERROR, "Unknown unfolding method: " + method);
            return nullptr;
        }
        
        TH1D* h_unfolded = (TH1D*)unfold->Hreco();
        if (h_unfolded) {
            h_unfolded->SetName("h_jetPt_unfolded");
            h_unfolded->SetTitle("Unfolded Jet p_{T};Jet p_{T} [GeV];Events");
            log(LOG_INFO, "1D unfolding successful! Unfolded integral: " + std::to_string(h_unfolded->Integral()));
        }
        
        return h_unfolded;
    }
    
    TH1D* getMeasured() { return h_measured; }
    TH1D* getTruth() { return h_truth; }
    RooUnfoldResponse* getResponse() { return response; }
    
    ~JetPtUnfolder() {
        delete unfold;
        delete response;
    }
};

// 2D Unfolding: Jet pT vs Girth
class JetPtGirthUnfolder {
private:
    UnfoldingBins bins;
    TH2D* h_measured;
    TH2D* h_truth;
    TH1D* h_measured_1d;
    TH1D* h_truth_1d;
    RooUnfoldResponse* response;
    RooUnfold* unfold;
public:
    JetPtGirthUnfolder(const UnfoldingBins& b) : bins(b), unfold(nullptr) {
        h_measured = new TH2D("h_jetPt_girth_measured", "Measured;Jet p_{T} [GeV];Jet Girth", 
            bins.nJetPt(), &bins.jetPt[0], bins.nJetGirth(), &bins.jetGirth[0]);
        h_truth = new TH2D("h_jetPt_girth_truth", "Truth;Jet p_{T} [GeV];Jet Girth", 
            bins.nJetPt(), &bins.jetPt[0], bins.nJetGirth(), &bins.jetGirth[0]);
        int nBins2D = bins.nJetPt() * bins.nJetGirth();
        h_measured_1d = new TH1D("h_jetPt_girth_measured_1d", "Measured 2D (flattened)", nBins2D, 0, nBins2D);
        h_truth_1d = new TH1D("h_jetPt_girth_truth_1d", "Truth 2D (flattened)", nBins2D, 0, nBins2D);
        response = new RooUnfoldResponse(h_measured_1d, h_truth_1d);
        log(LOG_INFO, "Created 2D unfolding histograms for jet pT vs girth");
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
        int nFilled = 0;
        for (Long64_t i = 0; i < nEntries; i++) {
            tree->GetEntry(i);
            if (jetPt > bins.jetPt.front() && jetPt < bins.jetPt.back() &&
                jetGirth > bins.jetGirth.front() && jetGirth < bins.jetGirth.back()) {
                h_measured->Fill(jetPt, jetGirth);
                int measBinX = h_measured->GetXaxis()->FindBin(jetPt) - 1;
                int measBinY = h_measured->GetYaxis()->FindBin(jetGirth) - 1;
                int globalMeasBin = getGlobalBin2D(measBinX, measBinY, bins.nJetPt());
                h_measured_1d->Fill(globalMeasBin);
                if (isMC && refJetPt > bins.jetPt.front() && refJetPt < bins.jetPt.back() &&
                    refJetGirth > bins.jetGirth.front() && refJetGirth < bins.jetGirth.back()) {
                    h_truth->Fill(refJetPt, refJetGirth);
                    int truthBinX = h_truth->GetXaxis()->FindBin(refJetPt) - 1;
                    int truthBinY = h_truth->GetYaxis()->FindBin(refJetGirth) - 1;
                    int globalTruthBin = getGlobalBin2D(truthBinX, truthBinY, bins.nJetPt());
                    h_truth_1d->Fill(globalTruthBin);
                    response->Fill(globalTruthBin, globalMeasBin);
                    if (nFilled < 10) {
                        log(LOG_DEBUG, "Filling response: truth (" + std::to_string(truthBinX) + "," + std::to_string(truthBinY) + ") => " + std::to_string(globalTruthBin) + ", measured (" + std::to_string(measBinX) + "," + std::to_string(measBinY) + ") => " + std::to_string(globalMeasBin));
                        nFilled++;
                    }
                }
            }
        }
        log(LOG_INFO, "Filled 2D histograms: measured=" + std::to_string(h_measured->GetEntries()) + ", truth=" + std::to_string(h_truth->GetEntries()));
        log(LOG_INFO, "Filled 2D response: measured_1d=" + std::to_string(h_measured_1d->GetEntries()) + ", truth_1d=" + std::to_string(h_truth_1d->GetEntries()));
        // Count nonzero bins in response matrix
        int nonzero = 0;
        TH2D* hresp = (TH2D*)response->Hresponse();
        for (int ix = 1; ix <= hresp->GetNbinsX(); ++ix) {
            for (int iy = 1; iy <= hresp->GetNbinsY(); ++iy) {
                if (hresp->GetBinContent(ix, iy) != 0) ++nonzero;
            }
        }
        log(LOG_INFO, "Response matrix nonzero bins: " + std::to_string(nonzero));
    }
    TH2D* performUnfolding(const std::string& method = "Invert", int nIter = 4) {
        if (!response) {
            log(LOG_ERROR, "No response matrix for 2D unfolding!");
            return nullptr;
        }
        log(LOG_INFO, "Performing 2D RooUnfold with method=Invert");
        if (unfold) delete unfold;
        unfold = new RooUnfoldInvert(response, h_measured);
        TH2D* h_unfolded = (TH2D*)unfold->Hreco();
        if (h_unfolded) {
            h_unfolded->SetName("h_jetPt_girth_unfolded");
            h_unfolded->SetTitle("Unfolded;Jet p_{T} [GeV];Jet Girth");
            h_unfolded->GetXaxis()->Set(bins.nJetPt(), &bins.jetPt[0]);
            h_unfolded->GetYaxis()->Set(bins.nJetGirth(), &bins.jetGirth[0]);
            log(LOG_INFO, "2D unfolding successful! Unfolded integral: " + std::to_string(h_unfolded->Integral()));
        }
        return h_unfolded;
    }
    TH2D* getMeasured() { return h_measured; }
    TH2D* getTruth() { return h_truth; }
    RooUnfoldResponse* getResponse() { return response; }
    ~JetPtGirthUnfolder() { delete unfold; delete response; }
};

// 3D Unfolding: PhotonEt vs JetPt vs Girth
class PhotonJetPtGirthUnfolder {
private:
    UnfoldingBins bins;
    TH3D* h_measured;
    TH3D* h_truth;
    TH1D* h_measured_1d;
    TH1D* h_truth_1d;
    RooUnfoldResponse* response;
    RooUnfold* unfold;
public:
    PhotonJetPtGirthUnfolder(const UnfoldingBins& b) : bins(b), unfold(nullptr) {
        h_measured = new TH3D("h_photon_jetPt_girth_measured", "Measured;Photon E_{T} [GeV];Jet p_{T} [GeV];Jet Girth", 
            bins.nPhotonEt(), &bins.photonEt[0], bins.nJetPt(), &bins.jetPt[0], bins.nJetGirth(), &bins.jetGirth[0]);
        h_truth = new TH3D("h_photon_jetPt_girth_truth", "Truth;Photon E_{T} [GeV];Jet p_{T} [GeV];Jet Girth", 
            bins.nPhotonEt(), &bins.photonEt[0], bins.nJetPt(), &bins.jetPt[0], bins.nJetGirth(), &bins.jetGirth[0]);
        int nBins3D = bins.nPhotonEt() * bins.nJetPt() * bins.nJetGirth();
        h_measured_1d = new TH1D("h_photon_jetPt_girth_measured_1d", "Measured 3D (flattened)", nBins3D, 0, nBins3D);
        h_truth_1d = new TH1D("h_photon_jetPt_girth_truth_1d", "Truth 3D (flattened)", nBins3D, 0, nBins3D);
        response = new RooUnfoldResponse(h_measured_1d, h_truth_1d);
        log(LOG_INFO, "Created 3D unfolding histograms for photonEt vs jetPt vs girth");
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
        log(LOG_INFO, "Processing " + std::to_string(nEntries) + " entries for 3D photonEt vs jetPt vs girth unfolding");
        int nFilled = 0;
        for (Long64_t i = 0; i < nEntries; i++) {
            tree->GetEntry(i);
            if (photonEt > bins.photonEt.front() && photonEt < bins.photonEt.back() &&
                jetPt > bins.jetPt.front() && jetPt < bins.jetPt.back() &&
                jetGirth > bins.jetGirth.front() && jetGirth < bins.jetGirth.back()) {
                h_measured->Fill(photonEt, jetPt, jetGirth);
                int measBinX = h_measured->GetXaxis()->FindBin(photonEt) - 1;
                int measBinY = h_measured->GetYaxis()->FindBin(jetPt) - 1;
                int measBinZ = h_measured->GetZaxis()->FindBin(jetGirth) - 1;
                int globalMeasBin = getGlobalBin3D(measBinX, measBinY, measBinZ, bins.nPhotonEt(), bins.nJetPt());
                h_measured_1d->Fill(globalMeasBin);
                if (isMC && MCphotonEt > bins.photonEt.front() && MCphotonEt < bins.photonEt.back() &&
                    refJetPt > bins.jetPt.front() && refJetPt < bins.jetPt.back() &&
                    refJetGirth > bins.jetGirth.front() && refJetGirth < bins.jetGirth.back()) {
                    h_truth->Fill(MCphotonEt, refJetPt, refJetGirth);
                    int truthBinX = h_truth->GetXaxis()->FindBin(MCphotonEt) - 1;
                    int truthBinY = h_truth->GetYaxis()->FindBin(refJetPt) - 1;
                    int truthBinZ = h_truth->GetZaxis()->FindBin(refJetGirth) - 1;
                    int globalTruthBin = getGlobalBin3D(truthBinX, truthBinY, truthBinZ, bins.nPhotonEt(), bins.nJetPt());
                    h_truth_1d->Fill(globalTruthBin);
                    response->Fill(globalTruthBin, globalMeasBin);
                    if (nFilled < 10) {
                        log(LOG_DEBUG, "Filling response: truth (" + std::to_string(truthBinX) + "," + std::to_string(truthBinY) + "," + std::to_string(truthBinZ) + ") => " + std::to_string(globalTruthBin) + ", measured (" + std::to_string(measBinX) + "," + std::to_string(measBinY) + "," + std::to_string(measBinZ) + ") => " + std::to_string(globalMeasBin));
                        nFilled++;
                    }
                }
            }
        }
        log(LOG_INFO, "Filled 3D histograms: measured=" + std::to_string(h_measured->GetEntries()) + ", truth=" + std::to_string(h_truth->GetEntries()));
        log(LOG_INFO, "Filled 3D response: measured_1d=" + std::to_string(h_measured_1d->GetEntries()) + ", truth_1d=" + std::to_string(h_truth_1d->GetEntries()));
        // Count nonzero bins in response matrix
        int nonzero3 = 0;
        TH2D* hresp3 = (TH2D*)response->Hresponse();
        for (int ix = 1; ix <= hresp3->GetNbinsX(); ++ix) {
            for (int iy = 1; iy <= hresp3->GetNbinsY(); ++iy) {
                if (hresp3->GetBinContent(ix, iy) != 0) ++nonzero3;
            }
        }
        log(LOG_INFO, "Response matrix nonzero bins: " + std::to_string(nonzero3));
    }
    TH3D* performUnfolding(const std::string& method = "Invert", int nIter = 4) {
        if (!response) {
            log(LOG_ERROR, "No response matrix for 3D unfolding!");
            return nullptr;
        }
        log(LOG_INFO, "Performing 3D RooUnfold with method=Invert");
        if (unfold) delete unfold;
        unfold = new RooUnfoldInvert(response, h_measured);
        TH3D* h_unfolded = (TH3D*)unfold->Hreco();
        if (h_unfolded) {
            h_unfolded->SetName("h_photon_jetPt_girth_unfolded");
            h_unfolded->SetTitle("Unfolded;Photon E_{T} [GeV];Jet p_{T} [GeV];Jet Girth");
            h_unfolded->GetXaxis()->Set(bins.nPhotonEt(), &bins.photonEt[0]);
            h_unfolded->GetYaxis()->Set(bins.nJetPt(), &bins.jetPt[0]);
            h_unfolded->GetZaxis()->Set(bins.nJetGirth(), &bins.jetGirth[0]);
            log(LOG_INFO, "3D unfolding successful! Unfolded integral: " + std::to_string(h_unfolded->Integral()));
        }
        return h_unfolded;
    }
    TH3D* getMeasured() { return h_measured; }
    TH3D* getTruth() { return h_truth; }
    RooUnfoldResponse* getResponse() { return response; }
    ~PhotonJetPtGirthUnfolder() { delete unfold; delete response; }
};

// ---
// FLATTENING AND AXIS STRUCTURE FOR MULTI-DIMENSIONAL RESPONSE MATRICES
// For 2D: globalBin = iy * nJetPt + ix
//   - X axis: global bin number (0 ... nJetPt*nJetGirth-1)
//   - Additional axes: first for jetPt (repeats for each girth), second for jetGirth (cycles for each jetPt)
// For 3D: globalBin = iz * (nJetPt*nJetGirth) + iy * nJetPt + ix
//   - X axis: global bin number (0 ... nPhotonEt*nJetPt*nJetGirth-1)
//   - Additional axes: photonEt (repeats for each jetPt/girth), jetPt (repeats for each girth, cycles for photonEt), girth (cycles fastest)
// ---

// Main unfolding function
void RooUnfoldJetSubstructure(const char* configFile = "../configs/UnfoldJetSub_xj_test.config") {
    // gSystem->Load("./RooUnfold/libRooUnfold.so");
    // gSystem->AddIncludePath("-I./RooUnfold/src");
    
    TStopwatch timer;
    timer.Start();
    
    log(LOG_INFO, "=== RooUnfold Jet Substructure Unfolding ===");
    log(LOG_INFO, "Loading configuration from: " + std::string(configFile));
    
    // Load configuration
    TEnv* config = new TEnv(configFile);
    gVerbosity = config->GetValue("Verbosity", 2);
    
    // Get input files
    const char* dataInputFile = config->GetValue("DataInputFile", "");
    const char* mcInputFile = config->GetValue("MCInputFile", "");
    const char* outputDir = config->GetValue("OutputDir", "./");
    const char* outputPrefix = config->GetValue("OutputPrefix", "roounfold");
    
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
    TH1D* unfolded1D = unfolder1D.performUnfolding("Invert");  // Matrix inversion for first pass
    
    // === 2D Unfolding: Jet pT vs Girth ===
    log(LOG_INFO, "\n=== 2D Unfolding: Jet pT vs Girth ===");
    JetPtGirthUnfolder unfolder2D(bins);
    unfolder2D.fillFromTree(dataTree, false);
    unfolder2D.fillFromTree(mcTree, true);
    TH2D* unfolded2D = unfolder2D.performUnfolding("Invert");
    // === 3D Unfolding: PhotonEt vs JetPt vs Girth ===
    log(LOG_INFO, "\n=== 3D Unfolding: PhotonEt vs JetPt vs Girth ===");
    PhotonJetPtGirthUnfolder unfolder3D(bins);
    unfolder3D.fillFromTree(dataTree, false);
    unfolder3D.fillFromTree(mcTree, true);
    TH3D* unfolded3D = unfolder3D.performUnfolding("Invert");
    // === Save Results ===
    std::string outputPath = std::string(outputDir) + "/" + std::string(outputPrefix) + "_roounfold.root";
    TFile* outFile = TFile::Open(outputPath.c_str(), "RECREATE");
    if (outFile && !outFile->IsZombie()) {
        // 1D results
        TDirectory* dir1D = outFile->mkdir("Unfolding1D");
        dir1D->cd();
        unfolder1D.getMeasured()->Write();
        unfolder1D.getTruth()->Write();
        if (unfolded1D) unfolded1D->Write();
        if (unfolder1D.getResponse()) unfolder1D.getResponse()->Write();
        if (unfolder1D.getResponse()) {
            TH2D* h_resp = (TH2D*)unfolder1D.getResponse()->Hresponse();
            if (h_resp) {
                TH2D* h_phys = (TH2D*)h_resp->Clone("h_jetPt_response");
                h_phys->SetTitle("Response Matrix;Truth Jet p_{T} [GeV];Measured Jet p_{T} [GeV]");
                h_phys->GetXaxis()->Set(bins.nJetPt(), &bins.jetPt[0]);
                h_phys->GetYaxis()->Set(bins.nJetPt(), &bins.jetPt[0]);
                // Remove default axis titles for consistency
                h_phys->GetXaxis()->SetTitle("");
                h_phys->GetYaxis()->SetTitle("");
                h_phys->Write();
                // Draw and overlay physical axis for 1D
                TCanvas* c1 = new TCanvas("c_jetPt_response", "1D Response Matrix with Physical Axes", 800, 700);
                c1->SetBottomMargin(0.20); // Margin for additional axis
                c1->SetLeftMargin(0.20);
                h_phys->Draw("COLZ");
                double x1 = h_phys->GetXaxis()->GetXmin();
                double x2 = h_phys->GetXaxis()->GetXmax();
                double y1 = h_phys->GetYaxis()->GetXmin();
                double y2 = h_phys->GetYaxis()->GetXmax();
                
                // X: Draw jetPt axis below default X
                std::vector<double> jetPtEdges = bins.jetPt;
                int nPt = bins.nJetPt();
                double axisY = y1 - 0.15*(y2-y1);
                TGaxis* ptXAxis = new TGaxis(x1, axisY, x2, axisY, jetPtEdges.front(), jetPtEdges.back(), nPt, "S-");
                ptXAxis->SetLabelColor(kBlack);
                ptXAxis->SetLineColor(kBlack);
                ptXAxis->SetLabelSize(0.020);
                ptXAxis->SetTitleSize(0.025);
                ptXAxis->SetTickSize(0.015);
                ptXAxis->SetTitle("Truth Jet p_{T} [GeV]");
                for (int i = 0; i <= nPt; ++i) {
                  if (i < (int)jetPtEdges.size()) {
                    ptXAxis->ChangeLabel(i+1, -1, -1, -1, -1, -1, std::to_string((int)jetPtEdges[i]).c_str());
                  }
                }
                ptXAxis->Draw();
                
                // Y: Draw jetPt axis left of default Y
                double axisX = x1 - 0.15*(x2-x1);
                TGaxis* ptYAxis = new TGaxis(axisX, y1, axisX, y2, jetPtEdges.front(), jetPtEdges.back(), nPt, "S-");
                ptYAxis->SetLabelColor(kBlack);
                ptYAxis->SetLineColor(kBlack);
                ptYAxis->SetLabelSize(0.020);
                ptYAxis->SetTitleSize(0.025);
                ptYAxis->SetTickSize(0.015);
                ptYAxis->SetTitle("Measured Jet p_{T} [GeV]");
                for (int i = 0; i <= nPt; ++i) {
                  if (i < (int)jetPtEdges.size()) {
                    ptYAxis->ChangeLabel(i+1, -1, -1, -1, -1, -1, std::to_string((int)jetPtEdges[i]).c_str());
                  }
                }
                ptYAxis->Draw();
                
                c1->Write();
                c1->SaveAs((std::string("c_jetPt_response.png")).c_str());
            }
        }
        // 2D results
        TDirectory* dir2D = outFile->mkdir("Unfolding2D");
        dir2D->cd();
        unfolder2D.getMeasured()->Write();
        unfolder2D.getTruth()->Write();
        if (unfolded2D) unfolded2D->Write();
        if (unfolder2D.getResponse()) unfolder2D.getResponse()->Write();
        if (unfolder2D.getResponse()) {
            TH2D* h_resp2 = (TH2D*)unfolder2D.getResponse()->Hresponse();
            if (h_resp2) {
                TH2D* h_phys2 = (TH2D*)h_resp2->Clone("h_jetPt_girth_response");
                h_phys2->SetTitle("Response Matrix;Truth [Jet p_{T}, Girth];Measured [Jet p_{T}, Girth]");
                h_phys2->GetXaxis()->Set(bins.nJetPt()*bins.nJetGirth(), 0, bins.nJetPt()*bins.nJetGirth());
                h_phys2->GetYaxis()->Set(bins.nJetPt()*bins.nJetGirth(), 0, bins.nJetPt()*bins.nJetGirth());
                // Remove default axis titles
                h_phys2->GetXaxis()->SetTitle("");
                h_phys2->GetYaxis()->SetTitle("");
                h_phys2->Write();
                // Draw and overlay segmented cycling physical axes for 2D
                TCanvas* c2 = new TCanvas("c_jetPt_girth_response", "2D Response Matrix with Segmented Physical Axes", 900, 800);
                c2->SetBottomMargin(0.25); // Large margin for multiple axes
                c2->SetLeftMargin(0.25);
                h_phys2->Draw("COLZ");
                double x1 = h_phys2->GetXaxis()->GetXmin();
                double x2 = h_phys2->GetXaxis()->GetXmax();
                double y1 = h_phys2->GetYaxis()->GetXmin();
                double y2 = h_phys2->GetYaxis()->GetXmax();
                // --- Segmented cycling axes for 2D flattening ---
                std::vector<double> jetPtEdges = bins.jetPt;
                std::vector<double> jetGirthEdges = bins.jetGirth;
                int nPt = bins.nJetPt();
                int nGirth = bins.nJetGirth();
                int nBins2D = nPt * nGirth;
                
                // X: for each girth bin, draw a jetPt axis segment
                double axisY = y1 - 0.18*(y2-y1);  // Slightly increased X axis offset
                for (int iGirth = 0; iGirth < nGirth; ++iGirth) {
                  int startBin = iGirth * nPt;
                  int endBin = (iGirth+1) * nPt;
                  double segX1 = x1 + (x2-x1) * (double)startBin / nBins2D;
                  double segX2 = x1 + (x2-x1) * (double)endBin / nBins2D;
                  // Each segment covers the full jetPt range but positioned for this girth bin  
                  TGaxis* ptSeg = new TGaxis(segX1, axisY, segX2, axisY, jetPtEdges.front(), jetPtEdges.back(), nPt, "S-");
                  ptSeg->SetLabelColor(kBlack);
                  ptSeg->SetLineColor(kBlack);
                  ptSeg->SetLabelSize(0.015);
                  ptSeg->SetTitleSize(0.018);
                  ptSeg->SetTickSize(0.01);
                  if (iGirth == nGirth-1) ptSeg->SetTitle("Jet p_{T} [GeV]");  // Title on last segment
                  else ptSeg->SetTitle("");
                  
                  // Control which labels to show: first bin edge blank if not first segment
                  if (iGirth == 0) {
                    // First segment: show all labels
                    std::cout << "DEBUG 2D X jetPt segment " << iGirth << " labels: ";
                    for (int i = 0; i <= nPt; ++i) {
                      if (i < (int)jetPtEdges.size()) {
                        std::string label = std::to_string((int)jetPtEdges[i]);
                        ptSeg->ChangeLabel(i+1, -1, -1, -1, -1, -1, label.c_str());
                        std::cout << "'" << label << "'";
                        if (i < nPt) std::cout << ", ";
                      }
                    }
                    std::cout << " (all values shown)" << std::endl;
                  } else {
                    // Subsequent segments: space for first label, show others
                    std::cout << "DEBUG 2D X jetPt segment " << iGirth << " labels: ";
                    for (int i = 0; i <= nPt; ++i) {
                      if (i < (int)jetPtEdges.size()) {
                        std::string label;
                        if (i == 0) {
                          label = " ";  // Space to avoid overlap
                          ptSeg->ChangeLabel(i+1, -1, -1, -1, -1, -1, label.c_str());
                        } else {
                          label = std::to_string((int)jetPtEdges[i]);
                          ptSeg->ChangeLabel(i+1, -1, -1, -1, -1, -1, label.c_str());
                        }
                        std::cout << "'" << label << "'";
                        if (i < nPt) std::cout << ", ";
                      }
                    }
                    std::cout << " (first=space, others=values)" << std::endl;
                  }
                  ptSeg->Draw();
                  // Draw separator line between segments (except last)
                  if (iGirth < nGirth-1) {
                    double sepX = segX2;
                    TLine* vline = new TLine(sepX, axisY-0.015*(y2-y1), sepX, axisY+0.015*(y2-y1));
                    vline->SetLineColor(kGray+2);
                    vline->SetLineStyle(2);
                    vline->Draw();
                  }
                }
                
                // X: Draw jetGirth axis (single segment covering all)
                double axisY2 = y1 - 0.25*(y2-y1);
                TGaxis* girthXSeg = new TGaxis(x1, axisY2, x2, axisY2, jetGirthEdges.front(), jetGirthEdges.back(), nGirth, "S-");
                girthXSeg->SetLabelColor(kBlack);
                girthXSeg->SetLineColor(kBlack);
                girthXSeg->SetLabelSize(0.015);
                girthXSeg->SetTitleSize(0.018);
                girthXSeg->SetTickSize(0.01);
                girthXSeg->SetTitle("Jet Girth");
                for (int i = 0; i <= nGirth; ++i) {
                  if (i < (int)jetGirthEdges.size()) {
                    char label[10];
                    snprintf(label, sizeof(label), "%.2g", jetGirthEdges[i]);
                    girthXSeg->ChangeLabel(i+1, -1, -1, -1, -1, -1, label);
                  }
                }
                girthXSeg->Draw();
                
                // Y: for each girth bin, draw a jetPt axis segment (same as X)
                double axisX = x1 - 0.12*(x2-x1);  // Reduced Y axis offset
                for (int iGirth = 0; iGirth < nGirth; ++iGirth) {
                  int startBin = iGirth * nPt;
                  int endBin = (iGirth+1) * nPt;
                  double segY1 = y1 + (y2-y1) * (double)startBin / nBins2D;
                  double segY2 = y1 + (y2-y1) * (double)endBin / nBins2D;
                  TGaxis* ptYSeg = new TGaxis(axisX, segY1, axisX, segY2, jetPtEdges.front(), jetPtEdges.back(), nPt, "S-");
                  ptYSeg->SetLabelColor(kBlack);
                  ptYSeg->SetLineColor(kBlack);
                  ptYSeg->SetLabelSize(0.015);
                  ptYSeg->SetTitleSize(0.018);
                  ptYSeg->SetTickSize(0.01);
                  if (iGirth == nGirth-1) ptYSeg->SetTitle("Jet p_{T} [GeV]");  // Title on last segment
                  else ptYSeg->SetTitle("");
                  
                  // Control which labels to show: first bin edge blank if not first segment
                  if (iGirth == 0) {
                    // First segment: show all labels
                    std::cout << "DEBUG 2D Y jetPt segment " << iGirth << " labels: ";
                    for (int i = 0; i <= nPt; ++i) {
                      if (i < (int)jetPtEdges.size()) {
                        std::string label = std::to_string((int)jetPtEdges[i]);
                        ptYSeg->ChangeLabel(i+1, -1, -1, -1, -1, -1, label.c_str());
                        std::cout << "'" << label << "'";
                        if (i < nPt) std::cout << ", ";
                      }
                    }
                    std::cout << " (all values shown)" << std::endl;
                  } else {
                    // Subsequent segments: space for first label, show others
                    std::cout << "DEBUG 2D Y jetPt segment " << iGirth << " labels: ";
                    for (int i = 0; i <= nPt; ++i) {
                      if (i < (int)jetPtEdges.size()) {
                        std::string label;
                        if (i == 0) {
                          label = " ";  // Space to avoid overlap
                          ptYSeg->ChangeLabel(i+1, -1, -1, -1, -1, -1, label.c_str());
                        } else {
                          label = std::to_string((int)jetPtEdges[i]);
                          ptYSeg->ChangeLabel(i+1, -1, -1, -1, -1, -1, label.c_str());
                        }
                        std::cout << "'" << label << "'";
                        if (i < nPt) std::cout << ", ";
                      }
                    }
                    std::cout << " (first=space, others=values)" << std::endl;
                  }
                  ptYSeg->Draw();
                  // Draw separator line between segments (except last)
                  if (iGirth < nGirth-1) {
                    double sepY = segY2;
                    TLine* hline = new TLine(axisX-0.015*(x2-x1), sepY, axisX+0.015*(x2-x1), sepY);
                    hline->SetLineColor(kGray+2);
                    hline->SetLineStyle(2);
                    hline->Draw();
                  }
                }
                
                // Y: Draw jetGirth axis (single segment covering all)
                double axisX2 = x1 - 0.20*(x2-x1);  // Reduced Y axis second level offset
                TGaxis* girthYSeg = new TGaxis(axisX2, y1, axisX2, y2, jetGirthEdges.front(), jetGirthEdges.back(), nGirth, "S-");
                girthYSeg->SetLabelColor(kBlack);
                girthYSeg->SetLineColor(kBlack);
                girthYSeg->SetLabelSize(0.015);
                girthYSeg->SetTitleSize(0.018);
                girthYSeg->SetTickSize(0.01);
                girthYSeg->SetTitle("Jet Girth");
                for (int i = 0; i <= nGirth; ++i) {
                  if (i < (int)jetGirthEdges.size()) {
                    char label[10];
                    snprintf(label, sizeof(label), "%.2g", jetGirthEdges[i]);
                    girthYSeg->ChangeLabel(i+1, -1, -1, -1, -1, -1, label);
                  }
                }
                girthYSeg->Draw();
                // --- End segmented cycling axes for 2D ---
                c2->Write();
                // c2->SaveAs((std::string("c_jetPt_girth_response.png")).c_str());
            }
        }
        // 3D results
        TDirectory* dir3D = outFile->mkdir("Unfolding3D");
        dir3D->cd();
        unfolder3D.getMeasured()->Write();
        unfolder3D.getTruth()->Write();
        if (unfolded3D) unfolded3D->Write();
        if (unfolder3D.getResponse()) unfolder3D.getResponse()->Write();
        if (unfolder3D.getResponse()) {
            TH2D* h_resp3 = (TH2D*)unfolder3D.getResponse()->Hresponse();
            if (h_resp3) {
                TH2D* h_phys3 = (TH2D*)h_resp3->Clone("h_photon_jetPt_girth_response");
                h_phys3->SetTitle("Response Matrix;Truth [PhotonEt, JetPt, Girth];Measured [PhotonEt, JetPt, Girth]");
                int nBins3D = bins.nPhotonEt()*bins.nJetPt()*bins.nJetGirth();
                h_phys3->GetXaxis()->Set(nBins3D, 0, nBins3D);
                h_phys3->GetYaxis()->Set(nBins3D, 0, nBins3D);
                // Remove default axis titles
                h_phys3->GetXaxis()->SetTitle("");
                h_phys3->GetYaxis()->SetTitle("");
                h_phys3->Write();
                // Draw and overlay segmented cycling physical axes for 3D
                TCanvas* c3 = new TCanvas("c_photon_jetPt_girth_response", "3D Response Matrix with Segmented Physical Axes", 1000, 900);
                c3->SetBottomMargin(0.45); // Extra large margin for 3D axes
                c3->SetLeftMargin(0.45);
                h_phys3->Draw("COLZ");
                double x1 = h_phys3->GetXaxis()->GetXmin();
                double x2 = h_phys3->GetXaxis()->GetXmax();
                double y1 = h_phys3->GetYaxis()->GetXmin();
                double y2 = h_phys3->GetYaxis()->GetXmax();
                // --- Segmented cycling axes for 3D flattening ---
                std::vector<double> photonEtEdges = bins.photonEt;
                std::vector<double> jetPtEdges = bins.jetPt;
                std::vector<double> jetGirthEdges = bins.jetGirth;
                int nPhotonEt = bins.nPhotonEt();
                int nPt = bins.nJetPt();
                int nGirth = bins.nJetGirth();
                
                // X: for each photonEt, then each girth, draw jetPt segments (jetPt cycles fastest)
                double axisY1 = y1 - 0.14*(y2-y1); // jetPt axis - slightly increased offset
                double axisY2 = y1 - 0.24*(y2-y1); // girth axis - slightly increased offset
                double axisY3 = y1 - 0.34*(y2-y1); // photonEt axis - slightly increased offset
                
                // Draw jetPt segments for each (photonEt, girth) combination
                for (int iPhotonEt = 0; iPhotonEt < nPhotonEt; ++iPhotonEt) {
                  for (int iGirth = 0; iGirth < nGirth; ++iGirth) {
                    int startBin = iPhotonEt * (nPt * nGirth) + iGirth * nPt;
                    int endBin = startBin + nPt;
                    double segX1 = x1 + (x2-x1) * (double)startBin / nBins3D;
                    double segX2 = x1 + (x2-x1) * (double)endBin / nBins3D;
                    TGaxis* ptSeg = new TGaxis(segX1, axisY1, segX2, axisY1, jetPtEdges.front(), jetPtEdges.back(), nPt, "S-");
                    ptSeg->SetLabelColor(kBlack);
                    ptSeg->SetLineColor(kBlack);
                    ptSeg->SetLabelSize(0.012);
                    ptSeg->SetTitleSize(0.015);
                    ptSeg->SetTickSize(0.008);
                    // Title on last segment for right alignment
                    if (iPhotonEt == nPhotonEt-1 && iGirth == nGirth-1) ptSeg->SetTitle("Jet p_{T} [GeV]");
                    else ptSeg->SetTitle("");
                    
                    // Control which labels to show: first bin edge blank if not first segment
                    if (iPhotonEt == 0 && iGirth == 0) {
                      // First segment: show all labels
                      std::cout << "DEBUG 3D X jetPt segment (" << iPhotonEt << "," << iGirth << ") labels: ";
                      for (int i = 0; i <= nPt; ++i) {
                        if (i < (int)jetPtEdges.size()) {
                          std::string label = std::to_string((int)jetPtEdges[i]);
                          ptSeg->ChangeLabel(i+1, -1, -1, -1, -1, -1, label.c_str());
                          std::cout << "'" << label << "'";
                          if (i < nPt) std::cout << ", ";
                        }
                      }
                      std::cout << " (all values shown)" << std::endl;
                    } else {
                      // Subsequent segments: space for first label, show others
                      std::cout << "DEBUG 3D X jetPt segment (" << iPhotonEt << "," << iGirth << ") labels: ";
                      for (int i = 0; i <= nPt; ++i) {
                        if (i < (int)jetPtEdges.size()) {
                          std::string label;
                          if (i == 0) {
                            label = " ";  // Space to avoid overlap
                            ptSeg->ChangeLabel(i+1, -1, -1, -1, -1, -1, label.c_str());
                          } else {
                            label = std::to_string((int)jetPtEdges[i]);
                            ptSeg->ChangeLabel(i+1, -1, -1, -1, -1, -1, label.c_str());
                          }
                          std::cout << "'" << label << "'";
                          if (i < nPt) std::cout << ", ";
                        }
                      }
                      std::cout << " (first=space, others=values)" << std::endl;
                    }
                    ptSeg->Draw();
                    // Minor separator between girth cycles
                    if (iGirth < nGirth-1) {
                      TLine* minorSep = new TLine(segX2, axisY1-0.008*(y2-y1), segX2, axisY1+0.008*(y2-y1));
                      minorSep->SetLineColor(kGray+1);
                      minorSep->SetLineStyle(3);
                      minorSep->Draw();
                    }
                  }
                  // Major separator between photonEt cycles
                  if (iPhotonEt < nPhotonEt-1) {
                    double majorSepX = x1 + (x2-x1) * (double)((iPhotonEt+1) * nPt * nGirth) / nBins3D;
                    TLine* majorSep = new TLine(majorSepX, axisY1-0.015*(y2-y1), majorSepX, axisY1+0.015*(y2-y1));
                    majorSep->SetLineColor(kBlack);
                    majorSep->SetLineStyle(2);
                    majorSep->SetLineWidth(2);
                    majorSep->Draw();
                  }
                }
                
                // X: Draw girth segments for each photonEt cycle
                for (int iPhotonEt = 0; iPhotonEt < nPhotonEt; ++iPhotonEt) {
                  int startBin = iPhotonEt * (nPt * nGirth);
                  int endBin = (iPhotonEt+1) * (nPt * nGirth);
                  double segX1 = x1 + (x2-x1) * (double)startBin / nBins3D;
                  double segX2 = x1 + (x2-x1) * (double)endBin / nBins3D;
                  TGaxis* girthXSeg = new TGaxis(segX1, axisY2, segX2, axisY2, jetGirthEdges.front(), jetGirthEdges.back(), nGirth, "S-");
                  girthXSeg->SetLabelColor(kBlack);
                  girthXSeg->SetLineColor(kBlack);
                  girthXSeg->SetLabelSize(0.012);
                  girthXSeg->SetTitleSize(0.015);
                  girthXSeg->SetTickSize(0.008);
                  if (iPhotonEt == nPhotonEt-1) girthXSeg->SetTitle("Jet Girth");  // Title on last segment
                  else girthXSeg->SetTitle("");
                  for (int i = 0; i <= nGirth; ++i) {
                    if (i < (int)jetGirthEdges.size()) {
                      // Use blank label for first bin if not the first segment to avoid overlap
                      if (i == 0 && iPhotonEt > 0) {
                        char label[10];
                        snprintf(label, sizeof(label), "%.2g", jetGirthEdges[i]);
                        girthXSeg->ChangeLabel(i+1, -1, -1, -1, -1, -1, "");
                      } else {
                        char label[10];
                        snprintf(label, sizeof(label), "%.2g", jetGirthEdges[i]);
                        girthXSeg->ChangeLabel(i+1, -1, -1, -1, -1, -1, label);
                      }
                    }
                  }
                  girthXSeg->Draw();
                  // Major separator between photonEt cycles
                  if (iPhotonEt < nPhotonEt-1) {
                    double majorSepX = segX2;
                    TLine* majorSep = new TLine(majorSepX, axisY2-0.015*(y2-y1), majorSepX, axisY2+0.015*(y2-y1));
                    majorSep->SetLineColor(kBlack);
                    majorSep->SetLineStyle(2);
                    majorSep->SetLineWidth(2);
                    majorSep->Draw();
                  }
                }
                
                // X: Draw photonEt axis (one segment covering all)
                TGaxis* photonXSeg = new TGaxis(x1, axisY3, x2, axisY3, photonEtEdges.front(), photonEtEdges.back(), nPhotonEt, "S-");
                photonXSeg->SetLabelColor(kBlack);
                photonXSeg->SetLineColor(kBlack);
                photonXSeg->SetLabelSize(0.012);
                photonXSeg->SetTitleSize(0.015);
                photonXSeg->SetTickSize(0.008);
                photonXSeg->SetTitle("Photon E_{T} [GeV]");
                for (int i = 0; i <= nPhotonEt; ++i) {
                  if (i < (int)photonEtEdges.size()) {
                    photonXSeg->ChangeLabel(i+1, -1, -1, -1, -1, -1, std::to_string((int)photonEtEdges[i]).c_str());
                  }
                }
                photonXSeg->Draw();
                
                // Y: for each photonEt, then each jetPt, draw girth segments
                double axisX1 = x1 - 0.10*(x2-x1); // girth axis - reduced offset
                double axisX2 = x1 - 0.22*(x2-x1); // jetPt axis - reduced offset  
                double axisX3 = x1 - 0.32*(x2-x1); // photonEt axis - reduced offset
                
                // Draw girth segments for each (photonEt, jetPt) combination
                for (int iPhotonEt = 0; iPhotonEt < nPhotonEt; ++iPhotonEt) {
                  for (int iPt = 0; iPt < nPt; ++iPt) {
                    for (int iGirth = 0; iGirth < nGirth; ++iGirth) {
                      int globalBin = iPhotonEt * (nPt * nGirth) + iGirth * nPt + iPt;
                      double segY1 = y1 + (y2-y1) * (double)globalBin / nBins3D;
                      double segY2 = y1 + (y2-y1) * (double)(globalBin+1) / nBins3D;
                      TGaxis* girthSeg = new TGaxis(axisX1, segY1, axisX1, segY2, jetGirthEdges[iGirth], jetGirthEdges[iGirth+1], 2, "S-");
                      girthSeg->SetLabelColor(kBlack);
                      girthSeg->SetLineColor(kBlack);
                      girthSeg->SetLabelSize(0.012);
                      girthSeg->SetTitleSize(0.015);
                      girthSeg->SetTickSize(0.008);
                      // Title on last segment for right alignment
                      if (iPhotonEt == nPhotonEt-1 && iPt == nPt-1 && iGirth == nGirth-1) girthSeg->SetTitle("Jet Girth");
                      else girthSeg->SetTitle("");
                      char label1[10], label2[10];
                      snprintf(label1, sizeof(label1), "%.2g", jetGirthEdges[iGirth]);
                      snprintf(label2, sizeof(label2), "%.2g", jetGirthEdges[iGirth+1]);
                      // Use blank label for first bin if not the first bin to avoid overlap with previous cycle
                      if (iGirth == 0 && (iPhotonEt > 0 || iPt > 0)) {
                        girthSeg->ChangeLabel(1, -1, -1, -1, -1, -1, "");
                      } else {
                        girthSeg->ChangeLabel(1, -1, -1, -1, -1, -1, label1);
                      }
                      girthSeg->ChangeLabel(2, -1, -1, -1, -1, -1, label2);
                      girthSeg->Draw();
                    }
                    // Minor separator between jetPt cycles within photonEt
                    if (iPt < nPt-1) {
                      double minorSepY = y1 + (y2-y1) * (double)(iPhotonEt * (nPt * nGirth) + (iPt+1) * nGirth) / nBins3D;
                      TLine* minorSep = new TLine(axisX1-0.008*(x2-x1), minorSepY, axisX1+0.008*(x2-x1), minorSepY);
                      minorSep->SetLineColor(kGray+1);
                      minorSep->SetLineStyle(3);
                      minorSep->Draw();
                    }
                  }
                  // Major separator between photonEt cycles
                  if (iPhotonEt < nPhotonEt-1) {
                    double majorSepY = y1 + (y2-y1) * (double)((iPhotonEt+1) * nPt * nGirth) / nBins3D;
                    TLine* majorSep = new TLine(axisX1-0.015*(x2-x1), majorSepY, axisX1+0.015*(x2-x1), majorSepY);
                    majorSep->SetLineColor(kBlack);
                    majorSep->SetLineStyle(2);
                    majorSep->SetLineWidth(2);
                    majorSep->Draw();
                  }
                }
                
                // Y: Draw jetPt segments for each photonEt cycle
                for (int iPhotonEt = 0; iPhotonEt < nPhotonEt; ++iPhotonEt) {
                  int startBin = iPhotonEt * (nPt * nGirth);
                  int endBin = (iPhotonEt+1) * (nPt * nGirth);
                  double segY1 = y1 + (y2-y1) * (double)startBin / nBins3D;
                  double segY2 = y1 + (y2-y1) * (double)endBin / nBins3D;
                  TGaxis* ptYSeg = new TGaxis(axisX2, segY1, axisX2, segY2, jetPtEdges.front(), jetPtEdges.back(), nPt, "S-");
                  ptYSeg->SetLabelColor(kBlack);
                  ptYSeg->SetLineColor(kBlack);
                  ptYSeg->SetLabelSize(0.012);
                  ptYSeg->SetTitleSize(0.015);
                  ptYSeg->SetTickSize(0.008);
                  if (iPhotonEt == nPhotonEt-1) ptYSeg->SetTitle("Jet p_{T} [GeV]");  // Title on last segment
                  else ptYSeg->SetTitle("");
                  
                  // Control which labels to show: first bin edge blank if not first segment
                  if (iPhotonEt == 0) {
                    // First segment: show all labels
                    std::cout << "DEBUG 3D Y jetPt segment " << iPhotonEt << " labels: ";
                    for (int i = 0; i <= nPt; ++i) {
                      if (i < (int)jetPtEdges.size()) {
                        std::string label = std::to_string((int)jetPtEdges[i]);
                        ptYSeg->ChangeLabel(i+1, -1, -1, -1, -1, -1, label.c_str());
                        std::cout << "'" << label << "'";
                        if (i < nPt) std::cout << ", ";
                      }
                    }
                    std::cout << " (all values shown)" << std::endl;
                  } else {
                    // Subsequent segments: space for first label, show others
                    std::cout << "DEBUG 3D Y jetPt segment " << iPhotonEt << " labels: ";
                    for (int i = 0; i <= nPt; ++i) {
                      if (i < (int)jetPtEdges.size()) {
                        std::string label;
                        if (i == 0) {
                          label = " ";  // Space to avoid overlap
                          ptYSeg->ChangeLabel(i+1, -1, -1, -1, -1, -1, label.c_str());
                        } else {
                          label = std::to_string((int)jetPtEdges[i]);
                          ptYSeg->ChangeLabel(i+1, -1, -1, -1, -1, -1, label.c_str());
                        }
                        std::cout << "'" << label << "'";
                        if (i < nPt) std::cout << ", ";
                      }
                    }
                    std::cout << " (first=space, others=values)" << std::endl;
                  }
                  ptYSeg->Draw();
                  // Major separator between photonEt cycles
                  if (iPhotonEt < nPhotonEt-1) {
                    double majorSepY = segY2;
                    TLine* majorSep = new TLine(axisX2-0.015*(x2-x1), majorSepY, axisX2+0.015*(x2-x1), majorSepY);
                    majorSep->SetLineColor(kBlack);
                    majorSep->SetLineStyle(2);
                    majorSep->SetLineWidth(2);
                    majorSep->Draw();
                  }
                }
                
                // Y: Draw photonEt axis (one segment covering all)
                TGaxis* photonYSeg = new TGaxis(axisX3, y1, axisX3, y2, photonEtEdges.front(), photonEtEdges.back(), nPhotonEt, "S-");
                photonYSeg->SetLabelColor(kBlack);
                photonYSeg->SetLineColor(kBlack);
                photonYSeg->SetLabelSize(0.012);
                photonYSeg->SetTitleSize(0.015);
                photonYSeg->SetTickSize(0.008);
                photonYSeg->SetTitle("Photon E_{T} [GeV]");
                for (int i = 0; i <= nPhotonEt; ++i) {
                  if (i < (int)photonEtEdges.size()) {
                    photonYSeg->ChangeLabel(i+1, -1, -1, -1, -1, -1, std::to_string((int)photonEtEdges[i]).c_str());
                  }
                }
                photonYSeg->Draw();
                photonYSeg->SetTitleSize(0.015);
                photonYSeg->SetTickSize(0.008);
                photonYSeg->SetTitle("");
                for (int i = 0; i <= nPhotonEt; ++i) {
                  if (i < (int)photonEtEdges.size()) {
                    photonYSeg->ChangeLabel(i+1, -1, -1, -1, -1, -1, std::to_string((int)photonEtEdges[i]).c_str());
                  }
                }
                photonYSeg->Draw();
                // --- End segmented cycling axes for 3D ---
                c3->Write();
                // c3->SaveAs((std::string("c_photon_jetPt_girth_response.png")).c_str());
            }
        }
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
    log(LOG_INFO, "\n=== RooUnfold completed in " + std::to_string(timer.RealTime()) + " seconds ===");
}