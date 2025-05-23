// ROOT includes
#include <TTree.h>
#include <TChain.h>
#include <TSystem.h>
#include <TEnv.h>
#include <TStyle.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TObjArray.h>
#include <TObjString.h>
#include "ROOT/RVec.hxx"
#include <TVector2.h>
#include <TSystemDirectory.h>
#include <TSystemFile.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <THStack.h>
#include <TLatex.h>
#include <TPaveText.h>
#include <TLine.h>
#include <iostream>
#include <fstream>
#include <cstdio>

// C++ includes
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>

// Local includes
#include "../include/helpers.h"
#include "../include/GammaJetAnalysis.h"
#include "../include/PhotonSelector.h"
#include "../include/JetSubstructure.h"
#define GammaJet2023_PbPbMC_cxx
#include "../include/GammaJet2023_PbPbMC.h"
#define GammaJet2023_PbPbData_cxx
#include "../include/GammaJet2023_PbPbData.h"

// Forward declaration of global configuration
extern Config cfg;
extern Long64_t g_maxEvents;

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

// Implementation of Loop method for GammaJet2023_PbPbMC
void GammaJet2023_PbPbMC::Loop() {
    // Access to global configuration
    const Config& configRef = cfg;
    
    if (!fChain) return;

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
        
        std::cout << "Will save output to: " << outputFileName << std::endl;
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
    std::cout << "Starting photon-only analysis with Et min = " << configRef.photonEtMin
              << " GeV and |eta| < " << configRef.photonEtaMax << std::endl;
    
    // Process each event
    Long64_t nentries = fChain->GetEntriesFast();
    // Limit number of events if maxEvents is set
    if (g_maxEvents > 0 && g_maxEvents < nentries) {
        nentries = g_maxEvents;
        cout << "Limiting to " << nentries << " events" << endl;
    }
    
    for (Long64_t jentry=0; jentry<nentries; jentry++) {
        if (jentry % 1000 == 0) cout << "Processing entry " << jentry << "/" << nentries << endl;
        
        Long64_t ientry = LoadTree(jentry);
        if (ientry < 0) break;
        fChain->GetEntry(jentry);

        // Apply event selection cuts
        if (fabs(vz) > configRef.vzCut) continue;
        if (hiHF < configRef.hiHFCutMin || hiHF > configRef.hiHFCutMax) continue;
        
        // Fill centrality histogram
        h_hiBin->Fill(hiBin);

        // Debug output for config parameters
        if (jentry < 5) {
            std::cout << "Photon selection criteria:" << std::endl;
            std::cout << "  ET min: " << configRef.photonEtMin << " GeV" << std::endl;
            std::cout << "  |eta| max: " << configRef.photonEtaMax << std::endl;
        }
        
        // Manual photon selection - simplified for debugging
        vector<int> selectedPhotons;
        for (int i = 0; i < ggHi_nPho; i++) {
            // Basic photon selection - only using ET and eta for now
            if ((*ggHi_phoEt)[i] < configRef.photonEtMin) continue;
            if (fabs((*ggHi_phoEta)[i]) > configRef.photonEtaMax) continue;
            
            // Add to selected photons
            selectedPhotons.push_back(i);
        }
        
        // Fill histograms for selected photons
        for (int idx : selectedPhotons) {
            h_photonEt->Fill((*ggHi_phoEt)[idx]);
            h_photonEta->Fill((*ggHi_phoEta)[idx]);
            h_photonPhi->Fill((*ggHi_phoPhi)[idx]);
            h_photonHoverE->Fill((*ggHi_phoHoverE)[idx]);
            h_photonSigma->Fill((*ggHi_phoSigmaIEtaIEta)[idx]);
            h_photonR9->Fill((*ggHi_phoR9)[idx]);
            
            // Debug output for first few events
            if (jentry < 5) {
                std::cout << "Event " << jentry << " photon " << idx 
                          << ": ET = " << (*ggHi_phoEt)[idx]
                          << ", eta = " << (*ggHi_phoEta)[idx] 
                          << ", phi = " << (*ggHi_phoPhi)[idx]
                          << ", H/E = " << (*ggHi_phoHoverE)[idx]
                          << ", sigma = " << (*ggHi_phoSigmaIEtaIEta)[idx]
                          << ", R9 = " << (*ggHi_phoR9)[idx] << std::endl;
            }
        }
        
        h_nPhotons->Fill(selectedPhotons.size());
        
        // Skip events with no selected photons
        if (selectedPhotons.empty()) continue;
        
        // Take highest ET photon as leading and fill output variables
        int leadingPhotonIdx = selectedPhotons[0];
        out_photonEt = (*ggHi_phoEt)[leadingPhotonIdx];
        out_photonEta = (*ggHi_phoEta)[leadingPhotonIdx];
        out_photonPhi = (*ggHi_phoPhi)[leadingPhotonIdx];
        out_photonHoverE = (*ggHi_phoHoverE)[leadingPhotonIdx];
        out_photonSigmaIEtaIEta = (*ggHi_phoSigmaIEtaIEta)[leadingPhotonIdx];
        out_photonR9 = (*ggHi_phoR9)[leadingPhotonIdx];
        
        // For simplicity, use ECAL isolation instead of summed isolation
        // This will be better defined in future iterations
        out_photonIso = (*ggHi_pho_ecalClusterIsoR3)[leadingPhotonIdx];
        
        // Fill the output tree if we have one
        if (outputTree) {
            outputTree->Fill();
        }
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
            delete outputTree;
        }
        
        // Close file
        outputFile->Close();
        delete outputFile;
        
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
    
    std::cout << "Finished processing " << nentries << " events" << std::endl;
}

// Implementation of Loop method for GammaJet2023_PbPbData
void GammaJet2023_PbPbData::Loop() {
    // Access to global configuration
    const Config& configRef = cfg;
    
    if (!fChain) return;

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
        
        std::cout << "Will save output to: " << outputFileName << std::endl;
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
    }
    
    // Debug message
    std::cout << "Starting photon-only analysis with Et min = " << configRef.photonEtMin
              << " GeV and |eta| < " << configRef.photonEtaMax << std::endl;
    
    // Process each event
    Long64_t nentries = fChain->GetEntriesFast();
    // Limit number of events if maxEvents is set
    if (g_maxEvents > 0 && g_maxEvents < nentries) {
        nentries = g_maxEvents;
        cout << "Limiting to " << nentries << " events" << endl;
    }
    
    for (Long64_t jentry=0; jentry<nentries; jentry++) {
        if (jentry % 1000 == 0) cout << "Processing entry " << jentry << "/" << nentries << endl;
        
        Long64_t ientry = LoadTree(jentry);
        if (ientry < 0) break;
        fChain->GetEntry(jentry);

        // Apply event selection cuts
        if (fabs(vz) > configRef.vzCut) continue;
        if (hiHF < configRef.hiHFCutMin || hiHF > configRef.hiHFCutMax) continue;
        
        // Fill centrality histogram
        h_hiBin->Fill(hiBin);

        // Debug output for config parameters
        if (jentry < 5) {
            std::cout << "Photon selection criteria:" << std::endl;
            std::cout << "  ET min: " << configRef.photonEtMin << " GeV" << std::endl;
            std::cout << "  |eta| max: " << configRef.photonEtaMax << std::endl;
        }
        
        // Manual photon selection - simplified for debugging
        vector<int> selectedPhotons;
        for (int i = 0; i < ggHi_nPho; i++) {
            // Basic photon selection - only using ET and eta for now
            if ((*ggHi_phoEt)[i] < configRef.photonEtMin) continue;
            if (fabs((*ggHi_phoEta)[i]) > configRef.photonEtaMax) continue;
            
            // Add to selected photons
            selectedPhotons.push_back(i);
        }
        
        // Fill histograms for selected photons
        for (int idx : selectedPhotons) {
            h_photonEt->Fill((*ggHi_phoEt)[idx]);
            h_photonEta->Fill((*ggHi_phoEta)[idx]);
            h_photonPhi->Fill((*ggHi_phoPhi)[idx]);
            h_photonHoverE->Fill((*ggHi_phoHoverE)[idx]);
            h_photonSigma->Fill((*ggHi_phoSigmaIEtaIEta)[idx]);
            h_photonR9->Fill((*ggHi_phoR9)[idx]);
            
            // Debug output for first few events
            if (jentry < 5) {
                std::cout << "Event " << jentry << " photon " << idx 
                          << ": ET = " << (*ggHi_phoEt)[idx]
                          << ", eta = " << (*ggHi_phoEta)[idx] 
                          << ", phi = " << (*ggHi_phoPhi)[idx]
                          << ", H/E = " << (*ggHi_phoHoverE)[idx]
                          << ", sigma = " << (*ggHi_phoSigmaIEtaIEta)[idx]
                          << ", R9 = " << (*ggHi_phoR9)[idx] << std::endl;
            }
        }
        
        h_nPhotons->Fill(selectedPhotons.size());
        
        // Skip events with no selected photons
        if (selectedPhotons.empty()) continue;
        
        // Take highest ET photon as leading and fill output variables
        int leadingPhotonIdx = selectedPhotons[0];
        out_photonEt = (*ggHi_phoEt)[leadingPhotonIdx];
        out_photonEta = (*ggHi_phoEta)[leadingPhotonIdx];
        out_photonPhi = (*ggHi_phoPhi)[leadingPhotonIdx];
        out_photonHoverE = (*ggHi_phoHoverE)[leadingPhotonIdx];
        out_photonSigmaIEtaIEta = (*ggHi_phoSigmaIEtaIEta)[leadingPhotonIdx];
        out_photonR9 = (*ggHi_phoR9)[leadingPhotonIdx];
        
        // For simplicity, use ECAL isolation instead of summed isolation
        // This will be better defined in future iterations
        out_photonIso = (*ggHi_pho_ecalClusterIsoR3)[leadingPhotonIdx];
        
        // Fill the output tree if we have one
        if (outputTree) {
            outputTree->Fill();
        }
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
            delete outputTree;
        }
        
        // Close file
        outputFile->Close();
        delete outputFile;
        
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
    
    std::cout << "Finished processing " << nentries << " events" << std::endl;
}

// Global config declaration
Config cfg;
// Global variable for max events
Long64_t g_maxEvents = -1;

// Function to create appropriate GammaJet analyzer based on config
GammaJetAnalysis* createAnalyzer(const Config& cfg, TTree* tree) {
    if (!tree) {
        std::cerr << "Error: Null tree pointer passed to createAnalyzer" << std::endl;
        return nullptr;
    }
    
    // For now we only have 2023 PbPb MC implementation
    if (cfg.system == "2023_PbPb" && cfg.dataType == "MC") {
        // Create analyzer with tree directly to avoid initialization issues
        auto analyzer = new GammaJet2023_PbPbMC(tree);
        return analyzer;
    }
    
    // For PbPb data, use the data-specific analyzer
    if (cfg.system == "2023_PbPb" && cfg.dataType == "Data") {
        auto analyzer = new GammaJet2023_PbPbData(tree);
        return analyzer;
    }
    
    std::cerr << "Unsupported system/datatype combination: " 
              << cfg.system << "/" << cfg.dataType << std::endl;
    return nullptr;
}

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
    
    // Parse centrality bins if provided as string
    std::string centBins = "CentralityBins";
    if (cfg.centBins.empty()) {
        const char* centBinsStr = gSystem->Getenv(centBins.c_str());
        if (centBinsStr) {
            std::string centBinsValue(centBinsStr);
            cfg.centBins = parseVector(centBinsValue);
            std::cout << "Using centrality bins from environment: ";
        } else {
            // Default centrality bins
            cfg.centBins = {0, 30, 60, 180};
            std::cout << "Using default centrality bins: ";
        }
        
        for (size_t i = 0; i < cfg.centBins.size(); i++) {
            std::cout << cfg.centBins[i];
            if (i < cfg.centBins.size() - 1) std::cout << ", ";
        }
        std::cout << std::endl;
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
        std::cout << "TEST MODE: Using only the first input file" << std::endl;
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

    std::cout << "Number of events in chain: " << chain->GetEntries() << std::endl;
    std::cout << "Running photon-only analysis with config: " << configPath << std::endl;
    std::cout << "System: " << cfg.system << ", DataType: " << cfg.dataType << std::endl;
    std::cout << "Input directory: " << cfg.inputDir << std::endl;
    std::cout << "Output directory: " << cfg.outputDir << std::endl;
    
    // Enable auto loading of libraries
    gSystem->Load("libTree");
    
    // Required for proper branch handling
    chain->SetMakeClass(1); 
    
    // Create analyzer - the chain is already set up with SetMakeClass
    auto analyzer = createAnalyzer(cfg, chain);
    if (!analyzer) {
        delete chain;
        return;
    }
    
    // The analyzer is already initialized with the chain in the constructor
    
    // Verify initialization was successful
    if (!analyzer->Notify()) {
        std::cerr << "Failed to initialize analyzer" << std::endl;
        delete analyzer;
        delete chain;
        return;
    }
    
    // Run analysis
    analyzer->Loop();
    
    // Cleanup
    delete analyzer;
    delete chain;
}

int main(int argc, char* argv[]){
    bool testMode = true;
    Long64_t maxTestEvents = 10000; // Process only 10,000 events for testing by default
    
    // Path to config files
    const char* configPath = "../../configs/photon_only.config";
    const char* histConfigPath = "../../configs/histParams.config";
    
    // Parse command line options
    if (argc > 1) {
        std::string arg1 = argv[1];
        if (arg1 == "--production" || arg1 == "-p") {
            // Production mode - process all events
            testMode = false;
            std::cout << "Running in production mode (all events)" << std::endl;
        } else if (arg1 == "--test" || arg1 == "-t") {
            // Explicit test mode
            testMode = true;
            // If a number is provided, use it as max events
            if (argc > 2) {
                maxTestEvents = std::stoll(argv[2]);
            }
            std::cout << "Running in test mode with " << maxTestEvents << " events" << std::endl;
        } else if (arg1 == "--help" || arg1 == "-h") {
            std::cout << "Usage: root -l 'photonJet.C([options])'" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --production, -p     Run in production mode (all events)" << std::endl;
            std::cout << "  --test, -t [n]       Run in test mode with n events (default: 10,000)" << std::endl;
            std::cout << "  --config, -c FILE    Specify config file (default: ../../configs/jetSubstructure.config)" << std::endl;
            std::cout << "  --hist, -h FILE      Specify histogram config file (default: ../../configs/histParams.config)" << std::endl;
            return 0;
        } else if (arg1 == "--config" || arg1 == "-c") {
            if (argc > 2) configPath = argv[2];
        } else if (arg1 == "--hist" || arg1 == "-H") {
            if (argc > 2) histConfigPath = argv[2];
        }
    }
    
    // Run the analysis
    photonJet(configPath, histConfigPath, testMode, maxTestEvents);
    return 0;
}

// Using Config struct defined in helpers.h

