// ROOT includes
#include <TChain.h>
#include <TSystem.h>
#include <TEnv.h>
#include <TStyle.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TObjArray.h>
#include <TObjString.h>
#include "ROOT/RVec.hxx"
#include <TVector2.h>  // For TVector2::Phi_mpi_pi() used in delta phi calculations
#include <TSystemDirectory.h>
#include <TSystemFile.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <THStack.h>
#include <TLatex.h>
#include <TPaveText.h>
#include <TLine.h>

// C++ includes
#include <iostream>
#include <fstream>
#include <cstdio>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>  // For std::pow, std::isinf, std::isnan
#include <stdexcept> // For std::runtime_error
#include <typeinfo>  // For typeid operator

// Local includes
#include "../include/helpers.h"
#include "../include/GammaJetAnalysis.h"
#include "../include/JetSubstructure.h"
#define GammaJet2023_PbPbMC_cxx
#include "../include/GammaJet2023_PbPbMC.h"
#define GammaJet2023_PbPbData_cxx
#include "../include/GammaJet2023_PbPbData.h"

// Forward declaration of global configuration
extern Config cfg;
extern Long64_t g_maxEvents;

// Global configuration
Config cfg;
Long64_t g_maxEvents = -1;

// Helper function to check if an analyzer is MC type without requiring function declaration in header
bool isAnalyzerMC(const GammaJetAnalysis* analyzer) {
    return dynamic_cast<const GammaJet2023_PbPbMC*>(analyzer) != nullptr;
}

// Helper function to get event weight for MC analyzers
float getMCEventWeight(const GammaJet2023_PbPbMC* analyzer) {
    float weight = 1.0;
    
    try {
        // Initialize weights from MC variables
        float ptHatWeight = 1.0;
        float centralityWeight = 1.0;
        float pileupWeight = 1.0;
        
        // Example ptHat weighting if available in the tree
        // if (analyzer->AK2Z1_pthat > 0) {
        //     // Typical ptHat weighting decreases contribution from high-ptHat samples
        //     // Using a common weighting scheme for QCD photon samples
        //     ptHatWeight = std::pow(analyzer->AK2Z1_pthat, -0.5); // Weight ~ 1/sqrt(ptHat)
        // }
        
        // Centrality weighting (optional)
        // if (analyzer->hiBin >= 0 && analyzer->hiBin < 200) {
        //     // Using a simple centrality weighting scheme
        //     if (analyzer->hiBin < 20) centralityWeight = 1.2;      // Boost central events
        //     else if (analyzer->hiBin >= 120) centralityWeight = 0.8; // Reduce peripheral events
        // }
        
        // Final weight is the product of all component weights
        weight = ptHatWeight * centralityWeight * pileupWeight;
        
        // For safety, check for unreasonable weights
        if (weight <= 0.0 || std::isinf(weight) || std::isnan(weight)) {
            weight = 1.0; // Fallback to no weight if problems detected
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception in getMCEventWeight(): " << e.what() << std::endl;
        weight = 1.0; // Return safe default
    }
    
    return weight;
}

// Helper function to get event weight for Data analyzers
float getDataEventWeight(const GammaJet2023_PbPbData* analyzer) {
    try {
        // For data, we typically don't apply weights
        // However, there might be cases where we need to apply corrections:
        // - Trigger efficiency corrections
        // - Luminosity section weights
        // - Prescale factor corrections
        
        // Add any data-specific weighting logic here if needed
        float triggerEfficiencyCorrection = 1.0;
        float luminosityWeight = 1.0;
        float prescaleCorrection = 1.0;
        
        float weight = triggerEfficiencyCorrection * luminosityWeight * prescaleCorrection;
        
        // Safety check
        if (weight <= 0.0 || std::isinf(weight) || std::isnan(weight)) {
            return 1.0;
        }
        
        return weight;
    } catch (const std::exception& e) {
        std::cerr << "Exception in getDataEventWeight(): " << e.what() << std::endl;
        return 1.0; // Return safe default
    }
}

// Generic function to get event weight for any analyzer type
float getEventWeight(const GammaJetAnalysis* analyzer) {
    // Check if analyzer is MC type
    const GammaJet2023_PbPbMC* mcAnalyzer = dynamic_cast<const GammaJet2023_PbPbMC*>(analyzer);
    if (mcAnalyzer) {
        return getMCEventWeight(mcAnalyzer);
    }
    
    // Check if analyzer is Data type
    const GammaJet2023_PbPbData* dataAnalyzer = dynamic_cast<const GammaJet2023_PbPbData*>(analyzer);
    if (dataAnalyzer) {
        return getDataEventWeight(dataAnalyzer);
    }
    
    // Default implementation for unknown analyzer types
    return 1.0;
}

// Helper function to plot and save a TH1D with CMS style
void plotAndSave(TH1D* h, const std::string& outdir, const std::string& tag = "", 
                 const Config& config = cfg, bool setLogY = false) {
    if (!h) return;
    
    // Use CMS style
    gStyle->SetOptStat(0);
    gStyle->SetOptTitle(0);
    gStyle->SetPadTickX(1);
    gStyle->SetPadTickY(1);
    
    TCanvas c("c", "", 800, 800);
    c.SetLeftMargin(0.15);
    c.SetRightMargin(0.08);
    c.SetTopMargin(0.08);
    c.SetBottomMargin(0.12);
    
    if (setLogY) c.SetLogy();
    
    // Format histogram
    h->SetLineWidth(2);
    h->SetLineColor(kBlue+1);
    h->SetMarkerStyle(20);
    h->SetMarkerColor(kBlue+1);
    h->SetMarkerSize(1.0);
    h->GetXaxis()->SetLabelFont(42);
    h->GetXaxis()->SetTitleFont(42);
    h->GetXaxis()->SetTitleOffset(1.2);
    h->GetYaxis()->SetLabelFont(42);
    h->GetYaxis()->SetTitleFont(42);
    h->GetYaxis()->SetTitleOffset(1.5);
    h->GetXaxis()->SetLabelSize(0.04);
    h->GetYaxis()->SetLabelSize(0.04);
    h->GetXaxis()->SetTitleSize(0.05);
    h->GetYaxis()->SetTitleSize(0.05);
    
    // Draw histogram
    h->Draw("hist e");
    
    // Add CMS text
    TLatex* cms_text = new TLatex();
    cms_text->SetNDC();
    cms_text->SetTextFont(42);
    cms_text->SetTextSize(0.05);
    cms_text->SetTextAlign(11);
    cms_text->DrawLatex(0.17, 0.96, "#bf{CMS} #it{Preliminary}");
    
    // Add energy and collision system
    TString systemLabel = TString::Format("%s 5.36 TeV", config.system.c_str());
    TLatex* energy_text = new TLatex();
    energy_text->SetNDC();
    energy_text->SetTextFont(42);
    energy_text->SetTextSize(0.04);
    energy_text->SetTextAlign(31);
    energy_text->DrawLatex(0.92, 0.96, systemLabel.Data());
    
    // Add selection info
    TLatex* selection_text = new TLatex();
    selection_text->SetNDC();
    selection_text->SetTextFont(42);
    selection_text->SetTextSize(0.04);
    selection_text->SetTextAlign(11);
    selection_text->DrawLatex(0.20, 0.88, TString::Format("Photon E_{T} > %.1f GeV", config.photonEtMin));
    selection_text->DrawLatex(0.20, 0.84, TString::Format("|#eta| < %.1f", config.photonEtaMax));
    
    // Save as PNG and PDF
    std::string baseName = outdir + "/" + h->GetName();
    if (!tag.empty()) baseName += "_" + tag;
    
    c.SaveAs((baseName + ".png").c_str());
    c.SaveAs((baseName + ".pdf").c_str());
}

// Common analysis function to be used by both MC and Data analyzers
// Each analyzer class handles its own weight calculations through getEventWeight()
// This allows us to use the same analysis code for both data and MC
// while properly handling the different weighting schemes
template<typename AnalyzerType>
void runPhotonAnalysis(AnalyzerType* analyzer) {
    // Access to global configuration
    const Config& configRef = cfg;
    
    if (!analyzer || !analyzer->fChain) return;

    // Create histograms - photon only
    TH1D* h_photonEt = new TH1D("h_photonEt", "Photon E_{T};E_{T} [GeV]", 100, 0, 500);
    TH1D* h_photonEta = new TH1D("h_photonEta", "Photon #eta;#eta", 100, -3, 3);
    TH1D* h_photonPhi = new TH1D("h_photonPhi", "Photon #phi;#phi", 100, -M_PI, M_PI);
    TH1D* h_photonHoverE = new TH1D("h_photonHoverE", "Photon H/E;H/E", 100, 0, 0.5);
    TH1D* h_photonSigma = new TH1D("h_photonSigma", "Photon #sigma_{i#etai#eta};#sigma_{i#etai#eta}", 100, 0, 0.05);
    TH1D* h_photonR9 = new TH1D("h_photonR9", "Photon R9;R9", 100, 0, 1.5);
    TH1D* h_nPhotons = new TH1D("h_nPhotons", "Number of photons;N_{#gamma}", 50, 0, 50);
    TH1D* h_hiBin = new TH1D("h_hiBin", "Centrality bin;Centrality bin", 200, 0, 200);
    
    // Output variables - photon only
    Float_t out_photonEt = 0.0;
    Float_t out_photonEta = 0.0;
    Float_t out_photonPhi = 0.0;
    Float_t out_photonHoverE = 0.0;
    Float_t out_photonSigmaIEtaIEta = 0.0;
    Float_t out_photonR9 = 0.0;
    Float_t out_photonIso = 0.0;
    Bool_t out_photonIsGenMatched = false;
    Float_t out_photonGenPt = 0.0;
    Float_t out_photonGenEta = 0.0;
    Float_t out_photonGenPhi = 0.0;
    
    // Initialize output file
    TFile* outputFile = nullptr;
    TTree* outputTree = nullptr;
    std::string outputFileName = "";
    if (!configRef.outputDir.empty()) {
        if (!configRef.outputPrefix.empty()) {
            outputFileName = configRef.outputDir + "/" + configRef.outputPrefix + "_histograms.root";
        } else {
            outputFileName = configRef.outputDir + "/photonJet_" + configRef.system + "_" + configRef.dataType + "_histograms.root";
        }
        
        log(LOG_INFO, "Will save output to: " + outputFileName);
        gSystem->mkdir(configRef.outputDir.c_str(), true);
        outputFile = new TFile(outputFileName.c_str(), "RECREATE");
        
        // Create output tree
        outputTree = new TTree("selectedEvents", "Selected Photon-Jet Events");
        
        // Set up branches - photon only
        outputTree->Branch("photonEt", &out_photonEt, "photonEt/F");
        outputTree->Branch("photonEta", &out_photonEta, "photonEta/F");
        outputTree->Branch("photonPhi", &out_photonPhi, "photonPhi/F");
        outputTree->Branch("photonHoverE", &out_photonHoverE, "photonHoverE/F");
        outputTree->Branch("photonSigmaIEtaIEta", &out_photonSigmaIEtaIEta, "photonSigmaIEtaIEta/F");
        outputTree->Branch("photonR9", &out_photonR9, "photonR9/F");
        outputTree->Branch("photonIso", &out_photonIso, "photonIso/F");
        
        if (configRef.dataType == "MC") {
            outputTree->Branch("photonIsGenMatched", &out_photonIsGenMatched, "photonIsGenMatched/O");
            outputTree->Branch("photonGenPt", &out_photonGenPt, "photonGenPt/F");
            outputTree->Branch("photonGenEta", &out_photonGenEta, "photonGenEta/F");
            outputTree->Branch("photonGenPhi", &out_photonGenPhi, "photonGenPhi/F");
        }
    }
    
    // Debug message
    log(LOG_INFO, "Starting photon analysis with Et min = " + std::to_string(configRef.photonEtMin) + 
          " GeV and |eta| < " + std::to_string(configRef.photonEtaMax));
    
    // Process each event
    Long64_t nentries = analyzer->fChain->GetEntriesFast();
    // Limit number of events if maxEvents is set
    if (g_maxEvents > 0 && g_maxEvents < nentries) {
        nentries = g_maxEvents;
        log(LOG_INFO, "Limiting to " + std::to_string(nentries) + " events");
    }
    
    for (Long64_t jentry=0; jentry<nentries; jentry++) {
        if (jentry % 1000 == 0) log(LOG_DEBUG, "Processing entry " + std::to_string(jentry) + "/" + std::to_string(nentries));
        
        Long64_t ientry = analyzer->LoadTree(jentry);
        if (ientry < 0) break;
        analyzer->fChain->GetEntry(jentry);

        // Apply event selection cuts
        if (fabs(analyzer->vz) > configRef.vzCut) continue;
        if (analyzer->hiHF < configRef.hiHFCutMin || analyzer->hiHF > configRef.hiHFCutMax) continue;
        
        // Fill centrality histogram - weight handled by analyzer class
        float weight = getEventWeight(analyzer);
        h_hiBin->Fill(analyzer->hiBin, weight);

        // Debug output for config parameters in first few events
        if (jentry < 5) {
            log(LOG_DEBUG, "Photon selection criteria:");
            log(LOG_DEBUG, "  ET min: " + std::to_string(configRef.photonEtMin) + " GeV");
            log(LOG_DEBUG, "  |eta| max: " + std::to_string(configRef.photonEtaMax));
            log(LOG_DEBUG, "  H/E max: " + std::to_string(configRef.photonHoverEMax));
            log(LOG_DEBUG, "  SigmaIEtaIEta max: " + std::to_string(configRef.photonSigmaIEtaIEtaMax));
            log(LOG_DEBUG, "  Isolation max: " + std::to_string(configRef.photonIsoMax));
            log(LOG_DEBUG, "  R9 min: " + std::to_string(configRef.photonR9Min));
        }
        
        // Step 1: Find the leading photon (highest Et)
        int leadingPhotonIdx = -1;
        float maxPhotonEt = -1.0;
        for (int i = 0; i < analyzer->ggHi_nPho; i++) {
            float et = (*(analyzer->ggHi_phoEt))[i];
            if (et > maxPhotonEt) {
                maxPhotonEt = et;
                leadingPhotonIdx = i;
            }
        }
        // If no photons found, skip event
        if (leadingPhotonIdx < 0) continue;

        // Step 2: In MC, require MC-matching and mother PID/isolation for leading photon
        if (configRef.dataType == "MC" && configRef.mcPhotonMatchRequired) {
            if (!isAnalyzerMC(analyzer)) {
                std::cerr << "Error: Trying to access MC branches with a non-MC analyzer!" << std::endl;
                continue;
            }
            GammaJet2023_PbPbMC* mcAnalyzer = dynamic_cast<GammaJet2023_PbPbMC*>(analyzer);
            try {
                int genMatchedIndex = (*(mcAnalyzer->ggHi_pho_genMatchedIndex))[leadingPhotonIdx];
                if (genMatchedIndex < 0) continue;
                int mcPID = (*(mcAnalyzer->ggHi_mcPID))[genMatchedIndex];
                if (abs(mcPID) != configRef.mcPhotonPID) continue;
                int mcMomPID = (*(mcAnalyzer->ggHi_mcMomPID))[genMatchedIndex];
                bool validMother = false;
                if (configRef.mcPhotonMomPIDs.empty()) {
                    validMother = true;
                } else {
                    for (int allowedPID : configRef.mcPhotonMomPIDs) {
                        if (mcMomPID == allowedPID || abs(mcMomPID) == abs(allowedPID)) {
                            validMother = true;
                            break;
                        }
                    }
                    if (!validMother) {
                        for (int allowedPID : configRef.mcPhotonMomPIDs) {
                            if (allowedPID == 22 && abs(mcMomPID) <= 22) {
                                validMother = true;
                                break;
                            }
                        }
                    }
                }
                if (!validMother) continue;
                float mcCalIsoDR04 = (*(mcAnalyzer->ggHi_mcCalIsoDR04))[genMatchedIndex];
                if (!(mcCalIsoDR04 < configRef.mcPhotonCalIsoDR04Max)) continue;
            } catch (const std::exception& e) {
                std::cerr << "Exception while accessing MC branches: " << e.what() << std::endl;
                continue;
            }
        }
        // Step 3: Apply all other photon selection criteria to the leading photon
        if ((*(analyzer->ggHi_phoEt))[leadingPhotonIdx] <= configRef.photonEtMin) continue;
        if (fabs((*(analyzer->ggHi_phoEta))[leadingPhotonIdx]) >= configRef.photonEtaMax) continue;
        if ((*(analyzer->ggHi_phoHoverE))[leadingPhotonIdx] >= configRef.photonHoverEMax) continue;
        if ((*(analyzer->ggHi_phoSigmaIEtaIEta))[leadingPhotonIdx] >= configRef.photonSigmaIEtaIEtaMax) continue;
        if ((*(analyzer->ggHi_pho_ecalClusterIsoR3))[leadingPhotonIdx] >= configRef.photonIsoMax) continue;
        if ((*(analyzer->ggHi_phoR9))[leadingPhotonIdx] <= configRef.photonR9Min) continue;

        // All criteria passed, fill histograms and output variables for the leading photon
        // Avoid variable redefinition: do not redeclare 'weight' here
        weight = getEventWeight(analyzer);
        h_photonEt->Fill((*(analyzer->ggHi_phoEt))[leadingPhotonIdx], weight);
        h_photonEta->Fill((*(analyzer->ggHi_phoEta))[leadingPhotonIdx], weight);
        h_photonPhi->Fill((*(analyzer->ggHi_phoPhi))[leadingPhotonIdx], weight);
        h_photonHoverE->Fill((*(analyzer->ggHi_phoHoverE))[leadingPhotonIdx], weight);
        h_photonSigma->Fill((*(analyzer->ggHi_phoSigmaIEtaIEta))[leadingPhotonIdx], weight);
        h_photonR9->Fill((*(analyzer->ggHi_phoR9))[leadingPhotonIdx], weight);
        h_nPhotons->Fill(1, weight);
        out_photonEt = (*(analyzer->ggHi_phoEt))[leadingPhotonIdx];
        out_photonEta = (*(analyzer->ggHi_phoEta))[leadingPhotonIdx];
        out_photonPhi = (*(analyzer->ggHi_phoPhi))[leadingPhotonIdx];
        out_photonHoverE = (*(analyzer->ggHi_phoHoverE))[leadingPhotonIdx];
        out_photonSigmaIEtaIEta = (*(analyzer->ggHi_phoSigmaIEtaIEta))[leadingPhotonIdx];
        out_photonR9 = (*(analyzer->ggHi_phoR9))[leadingPhotonIdx];
        out_photonIso = (*(analyzer->ggHi_pho_ecalClusterIsoR3))[leadingPhotonIdx];
        // MC gen-matching output
        out_photonIsGenMatched = false;
        out_photonGenPt = 0.0;
        out_photonGenEta = 0.0;
        out_photonGenPhi = 0.0;
        if (configRef.dataType == "MC" && isAnalyzerMC(analyzer)) {
            GammaJet2023_PbPbMC* mcAnalyzer = dynamic_cast<GammaJet2023_PbPbMC*>(analyzer);
            try {
                int genMatchedIndex = (*(mcAnalyzer->ggHi_pho_genMatchedIndex))[leadingPhotonIdx];
                if (genMatchedIndex >= 0) {
                    out_photonIsGenMatched = true;
                    out_photonGenPt = (*(mcAnalyzer->ggHi_mcPt))[genMatchedIndex];
                    out_photonGenEta = (*(mcAnalyzer->ggHi_mcEta))[genMatchedIndex];
                    out_photonGenPhi = (*(mcAnalyzer->ggHi_mcPhi))[genMatchedIndex];
                }
            } catch (const std::exception& e) {
                std::cerr << "Exception while filling MC photon information: " << e.what() << std::endl;
            }
        }
        if (outputTree) outputTree->Fill();
    }
    
    // Write histograms and clean up
    if (outputFile) {
        outputFile->cd();
        
        // Write basic histograms
        h_photonEt->Write();
        h_photonEta->Write();
        h_photonPhi->Write();
        h_photonHoverE->Write();
        h_photonSigma->Write();
        h_photonR9->Write();
        h_nPhotons->Write();
        h_hiBin->Write();
        
        // Write tree
        if (outputTree) {
            outputTree->Write();
        }
        
        // --- Plot and save PNGs for each histogram
        std::string plotdir = configRef.outputDir + "/plots";
        gSystem->mkdir(plotdir.c_str(), true);
        
        // Generate plots with different Y-scale options
        plotAndSave(h_photonEt, plotdir, "", configRef, true);  // log scale for Et
        plotAndSave(h_photonEta, plotdir, "", configRef);
        plotAndSave(h_photonPhi, plotdir, "", configRef);
        plotAndSave(h_photonHoverE, plotdir, "", configRef);
        plotAndSave(h_photonSigma, plotdir, "", configRef);
        plotAndSave(h_photonR9, plotdir, "", configRef);
        plotAndSave(h_nPhotons, plotdir, "", configRef);
        plotAndSave(h_hiBin, plotdir, "", configRef);
        
        // Close file
        outputFile->Close();
    }
    
    // Clean up histograms
    delete h_photonEt;
    delete h_photonEta;
    delete h_photonPhi;
    delete h_photonHoverE;
    delete h_photonSigma;
    delete h_photonR9;
    delete h_nPhotons;
    delete h_hiBin;
    
    if (outputFile) {
        delete outputFile;
    }
    
    log(LOG_INFO, "Finished processing " + std::to_string(nentries) + " events");
}

// Implementation of Loop method for GammaJet2023_PbPbMC

// Implementation of Loop method for GammaJet2023_PbPbMC
void GammaJet2023_PbPbMC::Loop() {
    // Access to global configuration
    const Config& configRef = cfg;
    
    if (!fChain) return;
    
    runPhotonAnalysis(this);
}

// Implementation moved to helper functions above

// Implementation of Loop method for GammaJet2023_PbPbData
void GammaJet2023_PbPbData::Loop() {
    // Access to global configuration
    const Config& configRef = cfg;
    
    if (!fChain) return;
    
    runPhotonAnalysis(this);
}

// Function to create appropriate GammaJet analyzer based on config
GammaJetAnalysis* createAnalyzer(const Config& cfg, TChain* chain) {
    if (!chain) {
        std::cerr << "Error: Null chain pointer passed to createAnalyzer" << std::endl;
        return nullptr;
    }
    
    /* Note on weighting:
     * Each analyzer class must implement the getEventWeight() method that returns the
     * appropriate event weight for that specific dataset:
     * 
     * - For GammaJet2023_PbPbMC: Should apply MC-specific weights which may include centrality 
     *   reweighting, pileup reweighting, etc. specific to 2023 PbPb MC samples
     * 
     * - For GammaJet2023_PbPbData: Should return 1.0 (no weights) or implement data-specific
     *   corrections if needed
     * 
     * This design allows different collision systems (2023 PbPb, 2024 PbPb, etc.) to have
     * completely independent weighting schemes while sharing common analysis code.
     */
     
    GammaJetAnalysis* analyzer = nullptr;
    
    try {
        // Create the appropriate analyzer based on system and dataType
        if (cfg.system == "2023_PbPb") {
            if (cfg.dataType == "MC") {
                log(LOG_INFO, "Creating 2023 PbPb MC analyzer");
                analyzer = new GammaJet2023_PbPbMC(chain);
            } 
            else if (cfg.dataType == "Data") {
                log(LOG_INFO, "Creating 2023 PbPb Data analyzer");
                analyzer = new GammaJet2023_PbPbData(chain);
            }
            else {
                throw std::runtime_error("Unknown data type: " + cfg.dataType);
            }
        }
        else {
            throw std::runtime_error("Unsupported collision system: " + cfg.system);
        }
        
        // Verify the analyzer was created successfully
        if (!analyzer) {
            throw std::runtime_error("Failed to create analyzer object");
        }
        
        // Initialize the analyzer
        analyzer->Init(chain);
        
    } catch (const std::exception& e) {
        std::cerr << "Error creating analyzer: " << e.what() << std::endl;
        delete analyzer;  // Clean up if needed
        return nullptr;
    }
    
    return analyzer;
}

// ClassMember struct and parseHeaderFile function are now implemented in helpers.h
// generatePhotonJetHeader function is also implemented in helpers.h

// Main analysis function
void photonJet(const char* configPath = "../../configs/photon_only.config",
               const char* histConfigPath = "../../configs/histParams.config",
               bool testMode = false, Long64_t maxTestEvents = 10000)
{
    // Set global max events
    g_maxEvents = testMode ? maxTestEvents : -1;
    
    // Load configuration
    if (!loadConfig(cfg, configPath)) {
        std::cerr << "Failed to load config from " << configPath << std::endl;
        return;
    }
    
    // Set global verbosity level from config
    g_verbosity = cfg.verbosity;
    log(LOG_INFO, "Configuration loaded successfully from " + std::string(configPath));
    log(LOG_DEBUG, "System: " + cfg.system + ", DataType: " + cfg.dataType);
    log(LOG_DEBUG, "Verbosity level set to: " + std::to_string(g_verbosity));
    
    // Generate photonJet.h dynamically based on configuration
    if (cfg.regenerateHeader) {
        log(LOG_INFO, "Regenerating photonJet.h header file...");
        generatePhotonJetHeader(cfg);
    } else {
        log(LOG_DEBUG, "Using existing photonJet.h header file");
    }
    
    // Parse centrality bins if provided as string
    std::string centBins = "CentralityBins";
    if (cfg.centBins.empty()) {
        const char* centBinsStr = gSystem->Getenv(centBins.c_str());
        if (centBinsStr) {
            std::string centBinsValue(centBinsStr);
            cfg.centBins = parseVector(centBinsValue);
            log(LOG_INFO, "Using centrality bins from environment");
        } else {
            // Default centrality bins
            cfg.centBins = {0, 30, 60, 180};
            log(LOG_INFO, "Using default centrality bins");
        }
        
        std::string binList = "";
        for (size_t i = 0; i < cfg.centBins.size(); i++) {
            binList += std::to_string(cfg.centBins[i]);
            if (i < cfg.centBins.size() - 1) binList += ", ";
        }
        log(LOG_INFO, "Centrality bins: " + binList);
    }
    
    // Add test mode indicator to output prefix if needed
    if (testMode && !cfg.outputPrefix.empty()) {
        cfg.outputPrefix = cfg.outputPrefix + "_test";
    }

    // Setup chain
    TChain* chain = new TChain("jet_tree");
    auto files = GetFiles(cfg.inputDir);
    
    // In test mode, only use the first file
    if (testMode && !files.empty()) {
        log(LOG_INFO, "TEST MODE: Using only the first input file");
        chain->Add(files[0].c_str());
    } else {
        for (const auto& file : files) {
            chain->Add(file.c_str());
        }
    }

    if (chain->GetEntries() == 0) {
        std::cerr << "No events found in input files" << std::endl;
        delete chain;
        return;
    }

    log(LOG_INFO, "Number of events in chain: " + std::to_string(chain->GetEntries()));
    log(LOG_INFO, "Running photon analysis with config: " + std::string(configPath));
    log(LOG_INFO, "System: " + cfg.system + ", DataType: " + cfg.dataType);
    log(LOG_INFO, "Input directory: " + cfg.inputDir);
    log(LOG_INFO, "Output directory: " + cfg.outputDir);
    
    // Enable auto loading of libraries
    gSystem->Load("libTree");
    
    // Required for proper branch handling
    chain->SetMakeClass(1); 
    
    // Create analyzer - the chain is already set up with SetMakeClass
    auto analyzer = createAnalyzer(cfg, chain);
    if (!analyzer) {
        std::cerr << "Failed to create analyzer. Exiting." << std::endl;
        delete chain;
        return;
    }
    
    // Verify initialization was successful
    if (!analyzer->Notify()) {
        std::cerr << "Failed to initialize analyzer" << std::endl;
        delete analyzer;
        delete chain;
        return;
    }
    
    // Generate dynamic header file based on config
    generatePhotonJetHeader(cfg);
    
    try {
        // Run analysis
        log(LOG_INFO, "Starting analysis loop...");
        analyzer->Loop();
        log(LOG_INFO, "Analysis completed successfully");
    } catch (const std::exception& e) {
        std::cerr << "Exception during analysis: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception during analysis" << std::endl;
    }
    
    // Cleanup
    delete analyzer;
    delete chain;
    
    log(LOG_INFO, "Analysis finished.");
}

int main(int argc, char* argv[]){
    bool testMode = true;
    Long64_t maxTestEvents = 10000; // Process only 10,000 events for testing by default
    
    // Default paths to config files
    const char* configPath = "../../configs/photon_only.config";
    const char* histConfigPath = "../../configs/histParams.config";
    
    // Parse command line options
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--production" || arg == "-p") {
            testMode = false;
            log(LOG_INFO, "Running in production mode (all events)");
        } 
        else if (arg == "--test" || arg == "-t") {
            testMode = true;
            // Check if next argument is a number for max events
            if (i + 1 < argc) {
                try {
                    maxTestEvents = std::stoll(argv[i + 1]);
                    i++; // Skip the next argument since we used it
                } catch (const std::exception&) {
                    // If conversion fails, use default and don't skip next arg
                    maxTestEvents = 10000;
                }
            }
            log(LOG_INFO, "Running in test mode with " + std::to_string(maxTestEvents) + " events");
        }
        else if (arg == "--config" || arg == "-c") {
            if (i + 1 < argc) {
                configPath = argv[i + 1];
                i++; // Skip the next argument since we used it
                log(LOG_INFO, "Using config file: " + std::string(configPath));
            } else {
                log(LOG_ERROR, "--config requires a file path argument");
                return 1;
            }
        }
        else if (arg == "--hist" || arg == "-H") {
            if (i + 1 < argc) {
                histConfigPath = argv[i + 1];
                i++; // Skip the next argument since we used it
                log(LOG_INFO, "Using histogram config file: " + std::string(histConfigPath));
            } else {
                log(LOG_ERROR, "--hist requires a file path argument");
                return 1;
            }
        }
        else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --production, -p     Run in production mode (all events)" << std::endl;
            std::cout << "  --test, -t [n]       Run in test mode with n events (default: 10,000)" << std::endl;
            std::cout << "  --config, -c FILE    Specify config file (default: ../../configs/photon_only.config)" << std::endl;
            std::cout << "  --hist, -H FILE      Specify histogram config file (default: ../../configs/histParams.config)" << std::endl;
            return 0;
        }
        else {
            log(LOG_ERROR, "Unknown option: " + arg);
            std::cout << "Use --help for usage information" << std::endl;
            return 1;
        }
    }
    
    // Run the analysis
    photonJet(configPath, histConfigPath, testMode, maxTestEvents);
    return 0;
}