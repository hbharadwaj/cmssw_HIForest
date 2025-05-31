/**
 * gammaJetAnalyzer.cpp
 *
 * Main C++ executable for photon-tagged jet substructure analysis.
 * This implements a hybrid approach combining traditional C-style event loop
 * with efficient branch handling for dynamic jet collections.
 *
 * Usage:
 *   ./gammaJetAnalyzer -c path/to/config.config [-t maxEvents] [-h]
 *
 * Example:
 *   ./gammaJetAnalyzer -c ../configs/JetSub_2023_PbPb_Data.config -t 1000
 */

// Include headers
#include "include/JetCollectionManager.h"
#include "include/helpers.h"
#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TProfile.h>
#include <TDirectory.h>
#include <TSystem.h>
#include <TStopwatch.h>
#include <TEnv.h>
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <getopt.h>
#include <unistd.h>
#include <cstdlib>

// Cut Flow Tracker Class
class CutFlowTracker {
public:
    struct CutInfo {
        std::string name;
        std::string description;
        int passedIndividual;   // Events passing this cut individually
        int passedSequential;   // Events passing all cuts up to this point
        bool isActive;          // Whether this cut is enabled in config
        
        CutInfo(const std::string& n, const std::string& desc, bool active = true) 
            : name(n), description(desc), passedIndividual(0), passedSequential(0), isActive(active) {}
    };
    
private:
    std::vector<CutInfo> cuts;
    int totalEvents;
    int currentSequentialPassed;
    bool isMC;
    
public:
    CutFlowTracker(TEnv* config) : totalEvents(0), currentSequentialPassed(0) {
        isMC = (std::string(config->GetValue("DataType", "Data")) == "MC");
        initializeCuts(config);
    }
    
    void initializeCuts(TEnv* config) {
        // Define all possible cuts based on config parameters
        cuts.clear();
        
        // Event-level cuts
        // RawEvents is always enabled
        cuts.emplace_back("RawEvents", "All input events", true);
        
        float vzCut = config->GetValue("VzCut", -1.0);
        if (vzCut > 0) {
            cuts.emplace_back("VertexCut", "Vertex |z| < " + std::to_string(vzCut) + " cm", true);
        }
        
        float hiHFMin = config->GetValue("HiHFCutMin", -1.0);
        float hiHFMax = config->GetValue("HiHFCutMax", -1.0);
        if (hiHFMin >= 0 || hiHFMax >= 0) {
            std::string desc = "Centrality: ";
            if (hiHFMin >= 0) desc += "HiHF > " + std::to_string(hiHFMin);
            if (hiHFMax >= 0) desc += (hiHFMin >= 0 ? " && " : "") + std::string("HiHF < ") + std::to_string(hiHFMax);
            cuts.emplace_back("CentralityCut", desc, true);
        }
        
        // Photon cuts
        float photonEtMin = config->GetValue("PhotonEtMin", -1.0);
        if (photonEtMin > 0) {
            cuts.emplace_back("PhotonKinematics", "Photon ET > " + std::to_string(photonEtMin) + " GeV", true);
        }
        
        float photonEtaMax = config->GetValue("PhotonEtaMax", -1.0);
        if (photonEtaMax > 0) {
            cuts.emplace_back("PhotonEta", "Photon |η| < " + std::to_string(photonEtaMax), true);
        }
        
        // Photon ID cuts
        float photonHoverEMax = config->GetValue("PhotonHoverEMax", -1.0);
        if (photonHoverEMax > 0) {
            cuts.emplace_back("PhotonHoverE", "Photon H/E < " + std::to_string(photonHoverEMax), true);
        }
        
        float photonSigmaMax = config->GetValue("PhotonSigmaIEtaIEtaMax", -1.0);
        if (photonSigmaMax > 0) {
            cuts.emplace_back("PhotonSigmaIEtaIEta", "Photon σ_iηiη < " + std::to_string(photonSigmaMax), true);
        }
        
        float photonIsoMax = config->GetValue("PhotonIsoMax", -1.0);
        if (photonIsoMax > 0) {
            cuts.emplace_back("PhotonIsolation", "Photon Iso < " + std::to_string(photonIsoMax), true);
        }
        
        float photonR9Min = config->GetValue("PhotonR9Min", -1.0);
        if (photonR9Min > 0) {
            cuts.emplace_back("PhotonR9", "Photon R9 > " + std::to_string(photonR9Min), true);
        }
        
        // MC-specific photon cuts
        if (isMC && config->GetValue("MCPhotonMatchRequired", 0)) {
            cuts.emplace_back("MCPhotonMatch", "MC truth photon matching", true);
        }
        
        // Jet cuts
        float jetPtMin = config->GetValue("JetPtMin", -1.0);
        if (jetPtMin > 0) {
            cuts.emplace_back("JetKinematics", "Jet pT > " + std::to_string(jetPtMin) + " GeV", true);
        }
        
        float jetEtaMax = config->GetValue("JetEtaMax", -1.0);
        if (jetEtaMax > 0) {
            cuts.emplace_back("JetEta", "Jet |η| < " + std::to_string(jetEtaMax), true);
        }
        
        // Angular correlation cuts
        float deltaPhiMin = config->GetValue("DeltaPhiMin", -1.0);
        if (deltaPhiMin > 0) {
            cuts.emplace_back("DeltaPhi", "Δφ(γ,jet) > " + std::to_string(deltaPhiMin) + " rad", true);
        }
        
        float xjMin = config->GetValue("XjMin", -1.0);
        if (xjMin > 0) {
            cuts.emplace_back("XjCut", "xj > " + std::to_string(xjMin), true);
        }
        
        cuts.emplace_back("FinalSelection", "All cuts passed", true);
        
        log(LOG_INFO, "CutFlowTracker initialized with " + std::to_string(cuts.size()) + " cuts");
        for (const auto& cut : cuts) {
            log(LOG_DEBUG, "  Cut: " + cut.name + " - " + cut.description);
        }
    }
    
    void startEvent() {
        totalEvents++;
        currentSequentialPassed = 0;
    }
    
    void applyCut(const std::string& cutName, bool passed) {
        auto it = std::find_if(cuts.begin(), cuts.end(), 
                              [&cutName](const CutInfo& cut) { return cut.name == cutName; });
        
        if (it != cuts.end() && it->isActive) {
            // Track individual count regardless of previous cuts
            if (passed) {
                it->passedIndividual++;
            }
            
            // Track sequential count only if in sequence
            size_t cutIndex = std::distance(cuts.begin(), it);
            
            // For the first cut or if this is the next cut after all previous cuts have passed
            if (cutIndex == 0) {
                // First cut - just track if it passed
                if (passed) {
                    it->passedSequential++;
                    currentSequentialPassed = 1;
                }
            } else if (cutIndex == currentSequentialPassed && passed) {
                // This is the next cut in sequence and it passed
                it->passedSequential++;
                currentSequentialPassed++;
            }
            
            // Debug log to help understand cut flow
            log(LOG_DEBUG, "Cut '" + cutName + "' evaluated: passed=" + std::to_string(passed) + 
                ", individual=" + std::to_string(it->passedIndividual) + 
                ", sequential=" + std::to_string(it->passedSequential) +
                ", currentSequentialPassed=" + std::to_string(currentSequentialPassed));
        }
    }
    
    void printCutFlow() const {
        log(LOG_INFO, "");
        log(LOG_INFO, "=== CUT FLOW SUMMARY ===");
        log(LOG_INFO, "Total events processed: " + std::to_string(totalEvents));
        log(LOG_INFO, "");
        log(LOG_INFO, std::string(100, '-'));
        log(LOG_INFO, "Cut Name              | Description                    | Individual        | Sequential        | Cut-to-Cut");
        log(LOG_INFO, "                      |                                | Count    (%)      | Count    (%)      | Efficiency (%)");
        log(LOG_INFO, std::string(100, '-'));
        
        for (size_t i = 0; i < cuts.size(); ++i) {
            const auto& cut = cuts[i];
            if (!cut.isActive) continue;
            
            double individualEff = totalEvents > 0 ? 100.0 * cut.passedIndividual / totalEvents : 0.0;
            double sequentialEff = 0.0;
            double sequentialPercentage = 0.0;
            
            if (i == 0) {
                sequentialEff = 100.0; // First cut (raw events) is always 100%
                sequentialPercentage = 100.0;
            } else if (cuts[i-1].passedSequential > 0) {
                sequentialEff = 100.0 * cut.passedSequential / cuts[i-1].passedSequential;
                sequentialPercentage = 100.0 * cut.passedSequential / totalEvents; // Percentage of total events
            }
            
            char buffer[250];
            snprintf(buffer, sizeof(buffer), "%-20s | %-30s | %7d (%5.1f%%) | %7d (%5.1f%%) | %6.2f%%",
                    cut.name.c_str(), 
                    cut.description.substr(0, 30).c_str(),
                    cut.passedIndividual, individualEff,
                    cut.passedSequential, sequentialPercentage,
                    sequentialEff);
            
            log(LOG_INFO, std::string(buffer));
        }
        
        log(LOG_INFO, std::string(100, '-'));
        
        if (cuts.size() > 1 && totalEvents > 0) {
            double overallEff = 100.0 * cuts.back().passedSequential / totalEvents;
            log(LOG_INFO, "Overall efficiency: " + std::to_string(cuts.back().passedSequential) + 
                         "/" + std::to_string(totalEvents) + " = " + 
                         std::to_string(overallEff) + "%");
        }
        log(LOG_INFO, "");
    }
    
    void saveCutFlowToFile(TFile* outFile) const {
        if (!outFile) return;
        
        outFile->cd();
        
        // Create histogram for cut flow
        TH1F* hCutFlow = new TH1F("hCutFlow", "Cut Flow;Cut Stage;Events", cuts.size(), 0, cuts.size());
        TH1F* hCutFlowEfficiency = new TH1F("hCutFlowEfficiency", "Cut Flow Efficiency;Cut Stage;Efficiency (%)", cuts.size(), 0, cuts.size());
        
        for (size_t i = 0; i < cuts.size(); ++i) {
            if (!cuts[i].isActive) continue;
            
            hCutFlow->SetBinContent(i + 1, cuts[i].passedSequential);
            hCutFlow->GetXaxis()->SetBinLabel(i + 1, cuts[i].name.c_str());
            
            double efficiency = (i == 0) ? 100.0 : 
                               (cuts[i-1].passedSequential > 0 ? 100.0 * cuts[i].passedSequential / cuts[i-1].passedSequential : 0.0);
            hCutFlowEfficiency->SetBinContent(i + 1, efficiency);
            hCutFlowEfficiency->GetXaxis()->SetBinLabel(i + 1, cuts[i].name.c_str());
        }
        
        hCutFlow->Write();
        hCutFlowEfficiency->Write();
        
        // Save cut flow table as TTree for easy access
        TTree* cutFlowTree = new TTree("cutFlowTree", "Cut Flow Information");
        
        std::string cutName, cutDescription;
        int passedIndividual, passedSequential, totalProcessed;
        double individualEff, sequentialEff;
        
        cutFlowTree->Branch("cutName", &cutName);
        cutFlowTree->Branch("cutDescription", &cutDescription); 
        cutFlowTree->Branch("passedIndividual", &passedIndividual);
        cutFlowTree->Branch("passedSequential", &passedSequential);
        cutFlowTree->Branch("totalProcessed", &totalProcessed);
        cutFlowTree->Branch("individualEfficiency", &individualEff);
        cutFlowTree->Branch("sequentialEfficiency", &sequentialEff);
        
        totalProcessed = totalEvents;
        
        for (size_t i = 0; i < cuts.size(); ++i) {
            const auto& cut = cuts[i];
            if (!cut.isActive) continue;
            
            cutName = cut.name;
            cutDescription = cut.description;
            passedIndividual = cut.passedIndividual;
            passedSequential = cut.passedSequential;
            individualEff = totalEvents > 0 ? 100.0 * cut.passedIndividual / totalEvents : 0.0;
            sequentialEff = (i == 0) ? 100.0 : 
                           (cuts[i-1].passedSequential > 0 ? 100.0 * cut.passedSequential / cuts[i-1].passedSequential : 0.0);
            
            cutFlowTree->Fill();
        }
        
        cutFlowTree->Write();
        
        log(LOG_INFO, "Cut flow information saved to output file");
    }
    
    int getFinalEventCount() const {
        return cuts.empty() ? 0 : cuts.back().passedSequential;
    }
};

// Forward declarations
void printUsage();
void processEvents(TChain* chain, TEnv* config, JetCollectionManager& jetManager, TFile* outFile, 
                  const PlottingConfiguration& plotConfig, Long64_t maxEvents = -1);
bool setupInputChain(TChain* chain, const std::string& inputDir, bool testMode, int maxFiles = 1);
void setupOutputTree(TTree* outTree, const std::vector<std::string>& jetCollections);
void createHistograms(TFile* outFile, const std::vector<std::string>& jetCollections, 
                     const std::vector<float>& centralityBins, const PlottingConfiguration& plotConfig);
float getDeltaPhi(float phi1, float phi2);
float getXj(float jetPt, float photonPt);

// Utility functions for config and directory management
bool createOutputDirectories(const std::string& outputDir);
void printConfig(TEnv* config);
std::vector<float> getFloatVector(TEnv* config, const std::string& param);
std::vector<std::string> getStringVector(TEnv* config, const std::string& param);

/**
 * Main function - entry point for standalone executable
 */
int main(int argc, char* argv[]) {
    log(LOG_DEBUG, "=== Command Line Debug ===");
    log(LOG_DEBUG, "argc: " + std::to_string(argc));
    for (int i = 0; i < argc; ++i) {
        log(LOG_DEBUG, "argv[" + std::to_string(i) + "]: " + std::string(argv[i]));
    }
    log(LOG_DEBUG, "=========================");
    
    std::string configFile = "../configs/JetSub_2023_PbPb_Data.config";
    std::string plotConfigFile = "../configs/PlotJetSub_2023_PbPb_Data.config";
    bool testMode = false;
    int maxEvents = 1000;
    
    // Parse command line arguments using getopt_long
    static struct option long_options[] = {
        {"config",       required_argument, 0, 'c'},
        {"plot-config",  required_argument, 0, 'p'},
        {"test",         optional_argument, 0, 't'},
        {"production",   no_argument,       0, 'P'},
        {"help",         no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };
    
    int option_index = 0;
    int c;
    
    while ((c = getopt_long(argc, argv, "c:p:t::Ph", long_options, &option_index)) != -1) {
        switch (c) {
            case 'c':
                configFile = optarg;
                break;
            case 'p':
                plotConfigFile = optarg;
                break;
            case 't':
                testMode = true;
                if (optarg) {
                    maxEvents = std::atoi(optarg);
                } else {
                    // Check if next argument is a number
                    if (optind < argc && argv[optind][0] != '-' && std::isdigit(argv[optind][0])) {
                        maxEvents = std::atoi(argv[optind]);
                        optind++;
                    }
                }
                break;
            case 'P':
                testMode = false;
                break;
            case 'h':
                printUsage();
                return 0;
            case '?':
                // Invalid option
                printUsage();
                return 1;
            default:
                printUsage();
                return 1;
        }
    }
    
    // Debug output
    log(LOG_DEBUG, "Command line parsing complete:");
    log(LOG_DEBUG, "  Analysis config file: " + configFile);
    log(LOG_DEBUG, "  Plotting config file: " + plotConfigFile);
    log(LOG_DEBUG, "  Test mode: " + std::string(testMode ? "true" : "false"));
    log(LOG_DEBUG, "  Max events: " + std::to_string(maxEvents));
    
    try {
        // Start timer
        TStopwatch timer;
        timer.Start();
        
        // Print information
        log(LOG_INFO, "==================================================");
        log(LOG_INFO, "=== PhotonJet Analysis: Jet Substructure v1.0 ===");
        log(LOG_INFO, "==================================================");
        log(LOG_INFO, "Config file: " + configFile);
        if (testMode) {
            log(LOG_INFO, "Running in TEST mode with " + std::to_string(maxEvents) + " events");
        } else {
            log(LOG_INFO, "Running in PRODUCTION mode (all events)");
        }
        
        // Load analysis configuration
        TEnv* config = new TEnv();
        if (gSystem->AccessPathName(configFile.c_str())) {
            std::cerr << "Error: Analysis configuration file not found: " << configFile << std::endl;
            delete config;
            return 1;
        }
        
        int readStatus = config->ReadFile(configFile.c_str(), kEnvLocal);
        if (readStatus != 0) {
            std::cerr << "Error: Failed to read analysis configuration file: " << configFile << std::endl;
            delete config;
            return 1;
        }
        
        log(LOG_INFO, "Loaded analysis configuration from " + configFile);
        
        // Print configuration summary
        printConfig(config);
        
        // Load plotting configuration from separate file
        PlottingConfiguration plotConfig;
        
        log(LOG_INFO, "Loading plotting configuration from: " + plotConfigFile);
        if (gSystem->AccessPathName(plotConfigFile.c_str())) {
            std::cerr << "Error: Plotting configuration file not found: " << plotConfigFile << std::endl;
            log(LOG_DEBUG, "Using default plotting configuration");
            // Initialize with default configuration if file not found
            plotConfig = PlottingConfiguration(); // Uses default constructor
        } else {
            if (!loadPlottingConfig(plotConfigFile, plotConfig)) {
                log(LOG_ERROR, "Failed to load plotting configuration, using defaults");
                // Initialize with default configuration if loading fails
                plotConfig = PlottingConfiguration(); // Uses default constructor
            } else {
                log(LOG_INFO, "Successfully loaded plotting configuration");
                log(LOG_DEBUG, "Loaded " + std::to_string(plotConfig.histogramConfigs.size()) + " histogram configurations");
            }
        }
        
        // Set the analysis config reference for MC detection
        plotConfig.configFile = config;
        
        // Get basic parameters
        std::string outputDir = config->GetValue("OutputDir", "");
        std::string outputPrefix = config->GetValue("OutputPrefix", "output");
        std::string inputDir = config->GetValue("InputDir", "");
        
        // Check essential parameters
        if (outputDir.empty()) {
            std::cerr << "Error: Output directory not specified in configuration" << std::endl;
            delete config;
            return 1;
        }
        
        if (inputDir.empty()) {
            std::cerr << "Error: Input directory not specified in configuration" << std::endl;
            delete config;
            return 1;
        }
        
        // Create output directories
        if (!createOutputDirectories(outputDir)) {
            std::cerr << "Error: Failed to create output directories" << std::endl;
            delete config;
            return 1;
        }
        
        // Setup input files
        TChain* chain = new TChain("jet_tree");
        if (!setupInputChain(chain, inputDir, testMode)) {
            std::cerr << "Error: Failed to setup input chain" << std::endl;
            delete chain;
            delete config;
            return 1;
        }
        
        // Setup output file
        std::string outputFile = outputDir + "/" + outputPrefix + "_output.root";
        
        TFile* outFile = new TFile(outputFile.c_str(), "RECREATE");
        if (!outFile || outFile->IsZombie()) {
            std::cerr << "Error: Failed to create output file: " << outputFile << std::endl;
            delete outFile;
            delete chain;
            delete config;
            return 1;
        }
        
        // Initialize jet collection manager
        JetCollectionManager jetManager(config, chain);
        if (!jetManager.initialize()) {
            std::cerr << "Error: Failed to initialize jet collections" << std::endl;
            delete outFile;
            delete chain;
            delete config;
            return 1;
        }
        
        // Print jet collection information
        jetManager.printBranchMappings();
        
        // Process events
        Long64_t nEvents = testMode && maxEvents > 0 ? maxEvents : -1;
        processEvents(chain, config, jetManager, outFile, plotConfig, nEvents);

        // Ensure all output is written before closing
        if (outFile && outFile->IsOpen()) {
            outFile->Write("", TObject::kOverwrite);
            outFile->Close();
            log(LOG_INFO, "Output file closed successfully.");
        }
        delete outFile;
        outFile = nullptr;
        delete chain;
        chain = nullptr;
        delete config;
        config = nullptr;
        
        // Print timing information
        timer.Stop();
        double realTime = timer.RealTime();
        double cpuTime = timer.CpuTime();
        
        log(LOG_INFO, "==================================================");
        log(LOG_INFO, "Analysis complete. Output written to: " + outputFile);
        log(LOG_INFO, "Real time: " + std::to_string(realTime) + " seconds, CPU time: " + std::to_string(cpuTime) + " seconds");
        log(LOG_INFO, "==================================================");
        
        log(LOG_INFO, "Batch job completed successfully. Exiting cleanly.");
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        // Attempt to close output file if open
        // if (outFile && outFile->IsOpen()) {
        //     outFile->Write("", TObject::kOverwrite);
        //     outFile->Close();
        //     log(LOG_INFO, "Output file closed after exception.");
        // }
        // delete outFile;
        // outFile = nullptr;
        log(LOG_ERROR, "Batch job failed with exception. Exiting with error code 1.");
        return 1;
    }
}

/**
 * Process events in the chain
 */
void processEvents(TChain* chain, TEnv* config, JetCollectionManager& jetManager, TFile* outFile, 
                  const PlottingConfiguration& plotConfig, Long64_t maxEvents) {
    if (!chain || !outFile) return;
    
    // Initialize logging verbosity from config
    g_verbosity = config->GetValue("Verbosity", LOG_INFO);
    log(LOG_INFO, "Starting event processing...");
    
    // Get parameters from config
    std::string dataType = config->GetValue("DataType", "Data");
    bool isMC = (dataType == "MC" || dataType == "mc");
    float vzCut = config->GetValue("VzCut", 15.0);
    float hiHFCutMin = config->GetValue("HiHFCutMin", 0.0);
    float hiHFCutMax = config->GetValue("HiHFCutMax", 7000.0);
    float photonEtMin = config->GetValue("PhotonEtMin", 60.0);
    float photonEtaMax = config->GetValue("PhotonEtaMax", 1.44);
    float photonHoverEMax = config->GetValue("PhotonHoverEMax", 0.12);
    float photonSigmaIEtaIEtaMax = config->GetValue("PhotonSigmaIEtaIEtaMax", 0.010392);
    float photonIsoMax = config->GetValue("PhotonIsoMax", 2.099277);
    float photonR9Min = config->GetValue("PhotonR9Min", 0.8);
    float jetPtMin = config->GetValue("JetPtMin", 40.0);
    float jetEtaMax = config->GetValue("JetEtaMax", 2.0);
    float deltaPhiMin = config->GetValue("DeltaPhiMin", 2.094);
    std::vector<float> centralityBins = getFloatVector(config, "CentralityBins");
    std::vector<std::string> jetCollections = jetManager.getCollections();
    
    log(LOG_DEBUG, "Configuration loaded:");
    log(LOG_DEBUG, "  DataType: " + dataType);
    log(LOG_DEBUG, "  isMC: " + std::to_string(isMC));
    log(LOG_DEBUG, "  vzCut: " + std::to_string(vzCut));
    log(LOG_DEBUG, "  photonEtMin: " + std::to_string(photonEtMin));
    log(LOG_DEBUG, "  jetPtMin: " + std::to_string(jetPtMin));
    
    // Debug: Print centrality bins right after loading
    log(LOG_DEBUG, "centralityBins loaded from config:");
    log(LOG_DEBUG, "  centralityBins.size() = " + std::to_string(centralityBins.size()));
    std::string binContents = "  centralityBins contents: ";
    for (float bin : centralityBins) {
        binContents += std::to_string(bin) + " ";
    }
    log(LOG_DEBUG, binContents);
    
    // Setup output tree
    TTree* outTree = new TTree("gammaJetTree", "Gamma-Jet Analysis");
    setupOutputTree(outTree, jetCollections);
    
    // Create histograms
    createHistograms(outFile, jetCollections, centralityBins, plotConfig);
    
    // Variables for branch addresses
    int hiBin = 0;
    float vz = 0;
    float hiHF = 0;
    
    // Photon variables - using vectors as original implementation for compatibility
    int nPhotons = 0;
    std::vector<float> *phoEt = nullptr;
    std::vector<float> *phoEta = nullptr;
    std::vector<float> *phoPhi = nullptr;
    std::vector<float> *phoHoverE = nullptr;
    std::vector<float> *phoSigmaIEtaIEta = nullptr;
    std::vector<float> *phoIso = nullptr;
    std::vector<float> *phoR9 = nullptr;
    
    // MC specific variables
    std::vector<int> *phoGenMatchedIndex = nullptr;
    std::vector<int> *mcPID = nullptr;
    std::vector<int> *mcMomPID = nullptr;
    std::vector<float> *mcPt = nullptr;
    std::vector<float> *mcEta = nullptr;
    std::vector<float> *mcPhi = nullptr;
    
    // Event weight for MC
    float eventWeight = 1.0;
    
    // Setup branch addresses for event variables
    chain->SetBranchAddress("hiBin", &hiBin);
    chain->SetBranchAddress("vz", &vz);
    chain->SetBranchAddress("hiHF", &hiHF);
    
    // Setup branch addresses for photon variables with correct ggHi_ prefix
    chain->SetBranchAddress("ggHi_nPho", &nPhotons);
    chain->SetBranchAddress("ggHi_phoEt", &phoEt);
    chain->SetBranchAddress("ggHi_phoEta", &phoEta);
    chain->SetBranchAddress("ggHi_phoPhi", &phoPhi);
    chain->SetBranchAddress("ggHi_phoHoverE", &phoHoverE);
    chain->SetBranchAddress("ggHi_phoSigmaIEtaIEta_2012", &phoSigmaIEtaIEta);
    chain->SetBranchAddress("ggHi_pho_ecalClusterIsoR3", &phoIso);
    chain->SetBranchAddress("ggHi_pho_swissCrx", &phoR9);
    
    // MC specific branch addresses
    if (isMC) {
        // Check and set up MC branches only if they exist
        TBranch* genMatchedBranch = chain->GetBranch("ggHi_pho_genMatchedIndex");
        TBranch* mcPIDBranch = chain->GetBranch("ggHi_mcPID");
        TBranch* mcMomPIDBranch = chain->GetBranch("ggHi_mcMomPID");
        TBranch* mcPtBranch = chain->GetBranch("ggHi_mcPt");
        TBranch* mcEtaBranch = chain->GetBranch("ggHi_mcEta");
        TBranch* mcPhiBranch = chain->GetBranch("ggHi_mcPhi");
        
        if (genMatchedBranch) {
            chain->SetBranchAddress("ggHi_pho_genMatchedIndex", &phoGenMatchedIndex);
            log(LOG_DEBUG, "MC branch connected: ggHi_pho_genMatchedIndex");
        } else {
            log(LOG_INFO, "MC branch not found: ggHi_pho_genMatchedIndex");
        }
        
        if (mcPIDBranch) {
            chain->SetBranchAddress("ggHi_mcPID", &mcPID);
            log(LOG_DEBUG, "MC branch connected: ggHi_mcPID");
        } else {
            log(LOG_INFO, "MC branch not found: ggHi_mcPID");
        }
        
        if (mcMomPIDBranch) {
            chain->SetBranchAddress("ggHi_mcMomPID", &mcMomPID);
            log(LOG_DEBUG, "MC branch connected: ggHi_mcMomPID");
        } else {
            log(LOG_INFO, "MC branch not found: ggHi_mcMomPID");
        }
        
        if (mcPtBranch) {
            chain->SetBranchAddress("ggHi_mcPt", &mcPt);
            log(LOG_DEBUG, "MC branch connected: ggHi_mcPt");
        } else {
            log(LOG_INFO, "MC branch not found: ggHi_mcPt");
        }
        
        if (mcEtaBranch) {
            chain->SetBranchAddress("ggHi_mcEta", &mcEta);
            log(LOG_DEBUG, "MC branch connected: ggHi_mcEta");
        } else {
            log(LOG_INFO, "MC branch not found: ggHi_mcEta");
        }
        
        if (mcPhiBranch) {
            chain->SetBranchAddress("ggHi_mcPhi", &mcPhi);
            log(LOG_DEBUG, "MC branch connected: ggHi_mcPhi");
        } else {
            log(LOG_INFO, "MC branch not found: ggHi_mcPhi");
        }
        
        // Try to set up weight branch - check if it exists
        TBranch* weightBranch = chain->GetBranch("weight");
        if (weightBranch) {
            chain->SetBranchAddress("weight", &eventWeight);
            log(LOG_INFO, "Weight branch found and connected for MC events");
        } else {
            log(LOG_INFO, "No weight branch found, using weight = 1.0 for all events");
        }
    }
    
    // Output variables
    int selectedHiBin = 0;
    float selectedVz = 0;
    float selectedHiHF = 0;
    int selectedPhotonIndex = -1;
    float selectedPhotonEt = 0;
    float selectedPhotonEta = 0;
    float selectedPhotonPhi = 0;
    float selectedPhotonHoverE = 0;
    float selectedPhotonSigmaIEtaIEta = 0;
    float selectedPhotonIso = 0;
    float selectedPhotonR9 = 0;
    
    // Jet output variables (one entry per jet collection)
    std::map<std::string, int> selectedJetIndexes;
    std::map<std::string, float> selectedJetPts;
    std::map<std::string, float> selectedJetEtas;
    std::map<std::string, float> selectedJetPhis;
    std::map<std::string, float> selectedJetMasses;
    std::map<std::string, float> selectedJetAreas;
    std::map<std::string, float> selectedJetDynSplits;
    std::map<std::string, float> selectedJetDynKts;
    std::map<std::string, float> selectedJetDynZs;
    std::map<std::string, float> selectedJetGirths;
    std::map<std::string, float> selectedJetThrusts;
    std::map<std::string, float> selectedJetLHAs;
    std::map<std::string, float> selectedJetPtDs;
    std::map<std::string, float> selectedJetDeltaPhis;
    std::map<std::string, float> selectedJetXjs;
    std::map<std::string, float> selectedJetDynDeltaRs;
    std::map<std::string, int> selectedJetIntJetMultis;
    // Ref jet (MC-matched) output variables
    std::map<std::string, float> selectedRefJetPts;
    std::map<std::string, float> selectedRefJetEtas;
    std::map<std::string, float> selectedRefJetPhis;
    std::map<std::string, float> selectedRefJetMasses;
    std::map<std::string, float> selectedRefJetAreas;
    std::map<std::string, float> selectedRefJetDynSplits;
    std::map<std::string, float> selectedRefJetDynKts;
    std::map<std::string, float> selectedRefJetDynZs;
    std::map<std::string, float> selectedRefJetGirths;
    std::map<std::string, float> selectedRefJetThrusts;
    std::map<std::string, float> selectedRefJetLHAs;
    std::map<std::string, float> selectedRefJetPtDs;
    std::map<std::string, float> selectedRefJetDynDeltaRs;
    std::map<std::string, int> selectedRefJetIntJetMultis;
    
    // Setup output tree branches
    outTree->Branch("hiBin", &selectedHiBin);
    outTree->Branch("vz", &selectedVz);
    outTree->Branch("hiHF", &selectedHiHF);
    outTree->Branch("photonIndex", &selectedPhotonIndex);
    outTree->Branch("photonEt", &selectedPhotonEt);
    outTree->Branch("photonEta", &selectedPhotonEta);
    outTree->Branch("photonPhi", &selectedPhotonPhi);
    outTree->Branch("photonHoverE", &selectedPhotonHoverE);
    outTree->Branch("photonSigmaIEtaIEta", &selectedPhotonSigmaIEtaIEta);
    outTree->Branch("photonIso", &selectedPhotonIso);
    outTree->Branch("photonR9", &selectedPhotonR9);
    
    // Initialize output variables for each jet collection
    for (const auto& collection : jetCollections) {
        selectedJetIndexes[collection] = -1;
        selectedJetPts[collection] = 0;
        selectedJetEtas[collection] = 0;
        selectedJetPhis[collection] = 0;
        selectedJetMasses[collection] = 0;
        selectedJetAreas[collection] = 0;
        selectedJetDynSplits[collection] = 0;
        selectedJetDynKts[collection] = 0;
        selectedJetDynZs[collection] = 0;
        selectedJetGirths[collection] = 0;
        selectedJetThrusts[collection] = 0;
        selectedJetLHAs[collection] = 0;
        selectedJetPtDs[collection] = 0;
        selectedJetDeltaPhis[collection] = 0;
        selectedJetXjs[collection] = 0;
        selectedJetDynDeltaRs[collection] = 0;
        selectedJetIntJetMultis[collection] = 0;
        
        // Create branches for each jet collection
        outTree->Branch(("jetIndex_" + collection).c_str(), &selectedJetIndexes[collection]);
        outTree->Branch(("jetPt_" + collection).c_str(), &selectedJetPts[collection]);
        outTree->Branch(("jetEta_" + collection).c_str(), &selectedJetEtas[collection]);
        outTree->Branch(("jetPhi_" + collection).c_str(), &selectedJetPhis[collection]);
        outTree->Branch(("jetMass_" + collection).c_str(), &selectedJetMasses[collection]);
        outTree->Branch(("jetArea_" + collection).c_str(), &selectedJetAreas[collection]);
        outTree->Branch(("jetDynSplit_" + collection).c_str(), &selectedJetDynSplits[collection]);
        outTree->Branch(("jetDynKt_" + collection).c_str(), &selectedJetDynKts[collection]);
        outTree->Branch(("jetDynZ_" + collection).c_str(), &selectedJetDynZs[collection]);
        outTree->Branch(("jetGirth_" + collection).c_str(), &selectedJetGirths[collection]);
        outTree->Branch(("jetThrust_" + collection).c_str(), &selectedJetThrusts[collection]);
        outTree->Branch(("jetLHA_" + collection).c_str(), &selectedJetLHAs[collection]);
        outTree->Branch(("jetPtD_" + collection).c_str(), &selectedJetPtDs[collection]);
        outTree->Branch(("deltaPhi_" + collection).c_str(), &selectedJetDeltaPhis[collection]);
        outTree->Branch(("xj_" + collection).c_str(), &selectedJetXjs[collection]);
        outTree->Branch(("jetDynDeltaR_" + collection).c_str(), &selectedJetDynDeltaRs[collection]);
        outTree->Branch(("jetIntJetMulti_" + collection).c_str(), &selectedJetIntJetMultis[collection]);
        
        // Ref jet branches (MC-matched jets)
        outTree->Branch(("refJetPt_" + collection).c_str(), &selectedRefJetPts[collection]);
        outTree->Branch(("refJetEta_" + collection).c_str(), &selectedRefJetEtas[collection]);
        outTree->Branch(("refJetPhi_" + collection).c_str(), &selectedRefJetPhis[collection]);
        outTree->Branch(("refJetMass_" + collection).c_str(), &selectedRefJetMasses[collection]);
        outTree->Branch(("refJetArea_" + collection).c_str(), &selectedRefJetAreas[collection]);
        outTree->Branch(("refJetDynSplit_" + collection).c_str(), &selectedRefJetDynSplits[collection]);
        outTree->Branch(("refJetDynKt_" + collection).c_str(), &selectedRefJetDynKts[collection]);
        outTree->Branch(("refJetDynZ_" + collection).c_str(), &selectedRefJetDynZs[collection]);
        outTree->Branch(("refJetGirth_" + collection).c_str(), &selectedRefJetGirths[collection]);
        outTree->Branch(("refJetThrust_" + collection).c_str(), &selectedRefJetThrusts[collection]);
        outTree->Branch(("refJetLHA_" + collection).c_str(), &selectedRefJetLHAs[collection]);
        outTree->Branch(("refJetPtD_" + collection).c_str(), &selectedRefJetPtDs[collection]);
        outTree->Branch(("refJetDynDeltaR_" + collection).c_str(), &selectedRefJetDynDeltaRs[collection]);
        outTree->Branch(("refJetIntJetMulti_" + collection).c_str(), &selectedRefJetIntJetMultis[collection]);
    }
    
    // Process events
    Long64_t totalEntries = chain->GetEntries();
    Long64_t nEvents = (maxEvents > 0 && maxEvents < totalEntries) ? maxEvents : totalEntries;
    
    log(LOG_INFO, "Total entries in chain: " + std::to_string(totalEntries));
    log(LOG_INFO, "Will process: " + std::to_string(nEvents) + " events");
    
    int nProcessed = 0;
    int nWithPhoton = 0;
    int nWithJet = 0;
    int nPassed = 0;
    
    // Initialize cut flow tracker
    CutFlowTracker cutFlowTracker(config);
    
    for (Long64_t iEvent = 0; iEvent < nEvents; ++iEvent) {
        if (iEvent % 1000 == 0) {
            log(LOG_INFO, "Processing event " + std::to_string(iEvent) + "/" + 
                std::to_string(nEvents) + " (" + 
                std::to_string(static_cast<double>(iEvent) / nEvents * 100) + "%)");
        }
        
        chain->GetEntry(iEvent);
        nProcessed++;
        
        // Reset event weight for each event (important for MC)
        if (!isMC) {
            eventWeight = 1.0;
        }
        
        log(LOG_TRACE, "Event " + std::to_string(iEvent) + " weight: " + std::to_string(eventWeight));
        
        // Start new event in cut flow tracker
        cutFlowTracker.startEvent();
        
        // Always apply RawEvents cut as true for every event processed
        cutFlowTracker.applyCut("RawEvents", true);
        
        // Event selection
        if (std::abs(vz) > vzCut) {
            cutFlowTracker.applyCut("VertexCut", false);
            continue;
        }
        cutFlowTracker.applyCut("VertexCut", true);
        
        if (hiHF < hiHFCutMin || hiHF > hiHFCutMax) {
            cutFlowTracker.applyCut("CentralityCut", false);
            continue;
        }
        cutFlowTracker.applyCut("CentralityCut", true);
        
        // FIXED: Photon selection using two-stage approach with individual cut tracking
        // Stage 1: First apply only kinematic cuts and find leading photon
        std::vector<int> kinematicCandidates;
        
        for (int iPho = 0; iPho < nPhotons; ++iPho) {
            // Apply only basic kinematic cuts
            if (phoEt->at(iPho) < photonEtMin) continue;
            if (std::abs(phoEta->at(iPho)) > photonEtaMax) continue;
            
            // Store candidate index
            kinematicCandidates.push_back(iPho);
        }
        
        // Track PhotonKinematics cut (ET + eta requirements)
        if (kinematicCandidates.empty()) {
            cutFlowTracker.applyCut("PhotonKinematics", false);
            continue;
        }
        cutFlowTracker.applyCut("PhotonKinematics", true);
        
        // Stage 2: Find the highest ET photon among kinematic candidates
        selectedPhotonIndex = -1;
        float maxPhotonEt = 0;
        
        for (int idx : kinematicCandidates) {
            if (phoEt->at(idx) > maxPhotonEt) {
                maxPhotonEt = phoEt->at(idx);
                selectedPhotonIndex = idx;
            }
        }
        
        // Stage 3: Apply photon ID criteria to the leading photon with individual cut tracking
        if (selectedPhotonIndex >= 0) {
            // Track PhotonEta cut (should pass since we selected from kinematic candidates)
            cutFlowTracker.applyCut("PhotonEta", true);
            
            // Check H/E cut
            bool passHoverE = (phoHoverE->at(selectedPhotonIndex) <= photonHoverEMax);
            cutFlowTracker.applyCut("PhotonHoverE", passHoverE);
            if (!passHoverE) {
                selectedPhotonIndex = -1;
                continue;
            }
            
            // Check sigma ieta ieta cut
            bool passSigmaIEtaIEta = (phoSigmaIEtaIEta->at(selectedPhotonIndex) <= photonSigmaIEtaIEtaMax);
            cutFlowTracker.applyCut("PhotonSigmaIEtaIEta", passSigmaIEtaIEta);
            if (!passSigmaIEtaIEta) {
                selectedPhotonIndex = -1;
                continue;
            }
            
            // Check isolation cut
            bool passIso = (phoIso->at(selectedPhotonIndex) <= photonIsoMax);
            cutFlowTracker.applyCut("PhotonIsolation", passIso);
            if (!passIso) {
                selectedPhotonIndex = -1;
                continue;
            }
            
            // Check R9 cut
            bool passR9 = (phoR9->at(selectedPhotonIndex) >= photonR9Min);
            cutFlowTracker.applyCut("PhotonR9", passR9);
            if (!passR9) {
                selectedPhotonIndex = -1;
                continue;
            }
            
            // MC-specific photon requirements (only if passed previous checks)
            if (selectedPhotonIndex >= 0 && isMC && config->GetValue("MCPhotonMatchRequired", 1)) {
                // Check if MC branches are available before using them
                if (!phoGenMatchedIndex) {
                    log(LOG_INFO, "MC photon matching required but ggHi_pho_genMatchedIndex branch not available. Skipping MC checks.");
                } else {
                    int genMatchedIndex = phoGenMatchedIndex->at(selectedPhotonIndex);
                    if (genMatchedIndex < 0) {
                        selectedPhotonIndex = -1;
                        cutFlowTracker.applyCut("MCPhotonMatch", false);
                    } else {
                        // Check particle ID (only if mcPID branch is available)
                        if (mcPID) {
                            std::string pidStr = config->GetValue("MCPhotonPID", "22");
                            std::vector<int> validPIDs;
                            std::stringstream ss(pidStr);
                            int pid;
                            while (ss >> pid) {
                                validPIDs.push_back(pid);
                                if (ss.peek() == ',') ss.ignore();
                            }
                            
                            // Check if mcPID matches any valid PID
                            bool validPID = false;
                            for (int pid : validPIDs) {
                                if (mcPID->at(genMatchedIndex) == pid) {
                                    validPID = true;
                                    break;
                                }
                            }
                            if (!validPID) selectedPhotonIndex = -1;
                        }
                        
                        // Check mother particle ID if specified (only if mcMomPID branch is available)
                        if (selectedPhotonIndex >= 0 && mcMomPID) {
                            std::string momPidStr = config->GetValue("MCPhotonMomPID", "22,-999");
                            std::vector<int> validMomPIDs;
                            std::stringstream momSS(momPidStr);
                            int momPid;
                            while (momSS >> momPid) {
                                validMomPIDs.push_back(momPid);
                                if (momSS.peek() == ',') momSS.ignore();
                            }
                            
                            // Check if mcMomPID matches any valid Mom PID
                            bool validMomPID = false;
                            for (int momPid : validMomPIDs) {
                                if (mcMomPID->at(genMatchedIndex) == momPid) {
                                    validMomPID = true;
                                    cutFlowTracker.applyCut("MCPhotonMatch", true);
                                    break;
                                }
                            }
                            if (!validMomPID) selectedPhotonIndex = -1;
                        }
                    }
                }
            }
        }
        
        // Final photon selection check
        if (selectedPhotonIndex < 0) {
            continue;
        }
        nWithPhoton++;
        
        // Store selected photon information
        selectedHiBin = hiBin;
        selectedVz = vz;
        selectedHiHF = hiHF;
        selectedPhotonEt = phoEt->at(selectedPhotonIndex);
        selectedPhotonEta = phoEta->at(selectedPhotonIndex);
        selectedPhotonPhi = phoPhi->at(selectedPhotonIndex);
        selectedPhotonHoverE = phoHoverE->at(selectedPhotonIndex);
        selectedPhotonSigmaIEtaIEta = phoSigmaIEtaIEta->at(selectedPhotonIndex);
        selectedPhotonIso = phoIso->at(selectedPhotonIndex);
        selectedPhotonR9 = phoR9->at(selectedPhotonIndex);
        
        // Reset jet selection for each collection
        for (const auto& collection : jetCollections) {
            selectedJetIndexes[collection] = -1;
            selectedJetPts[collection] = -999;
            selectedJetEtas[collection] = -999;
            selectedJetPhis[collection] = -999;
            selectedJetMasses[collection] = -999;
            selectedJetAreas[collection] = -999;
            selectedJetDynSplits[collection] = -999;
            selectedJetDynKts[collection] = -999;
            selectedJetDynZs[collection] = -999;
            selectedJetGirths[collection] = -999;
            selectedJetThrusts[collection] = -999;
            selectedJetLHAs[collection] = -999;
            selectedJetPtDs[collection] = -999;
            selectedJetDeltaPhis[collection] = -999;
            selectedJetXjs[collection] = -999;
            selectedJetDynDeltaRs[collection] = -999;
            selectedJetIntJetMultis[collection] = -999;
            
            // Reset ref jet variables
            selectedRefJetPts[collection] = -999;
            selectedRefJetEtas[collection] = -999;
            selectedRefJetPhis[collection] = -999;
            selectedRefJetMasses[collection] = -999;
            selectedRefJetAreas[collection] = -999;
            selectedRefJetDynSplits[collection] = -999;
            selectedRefJetDynKts[collection] = -999;
            selectedRefJetDynZs[collection] = -999;
            selectedRefJetGirths[collection] = -999;
            selectedRefJetThrusts[collection] = -999;
            selectedRefJetLHAs[collection] = -999;
            selectedRefJetPtDs[collection] = -999;
        }
        
        // Jet selection for each collection with individual cut tracking
        bool hasSelectedJet = false;
        std::vector<int> jetsPassingKinematics;
        
        // First pass: collect all jets passing kinematic cuts across all collections
        for (const auto& collection : jetCollections) {
            int nJets = jetManager.getNJets(collection);
            
            for (int iJet = 0; iJet < nJets; ++iJet) {
                float jetPt = jetManager.getJetPt(collection, iJet);
                float jetEta = jetManager.getJetEta(collection, iJet);
                
                // Apply jet kinematic selection
                if (jetPt >= jetPtMin && std::abs(jetEta) <= jetEtaMax) {
                    jetsPassingKinematics.push_back(1); // At least one jet passes
                    break; // We only need to know if any jet passes
                }
            }
            if (!jetsPassingKinematics.empty()) break; // Found at least one good jet
        }
        
        // Track JetKinematics cut (pT requirement only)
        bool passJetKinematics = !jetsPassingKinematics.empty();
        cutFlowTracker.applyCut("JetKinematics", passJetKinematics);
        
        // Immediately add JetEta cut - this passes together with kinematics since we checked eta in the loop above
        cutFlowTracker.applyCut("JetEta", passJetKinematics);
        
        if (!passJetKinematics) {
            continue; // Skip to next event if no jets pass
        } else {
            // Second pass: find the best jet among those passing cuts
            for (const auto& collection : jetCollections) {
                int nJets = jetManager.getNJets(collection);
                float maxJetPt = 0;
                int bestJetIndex = -1;
                
                for (int iJet = 0; iJet < nJets; ++iJet) {
                    float jetPt = jetManager.getJetPt(collection, iJet);
                    float jetEta = jetManager.getJetEta(collection, iJet);
                    float jetPhi = jetManager.getJetPhi(collection, iJet);
                    
                    // Apply jet selection
                    if (jetPt < jetPtMin) continue;
                    if (std::abs(jetEta) > jetEtaMax) continue;
                    
                    // Calculate delta phi between photon and jet
                    float dPhi = getDeltaPhi(selectedPhotonPhi, jetPhi);
                    
                    // Select highest pT jet passing all cuts
                    if (jetPt > maxJetPt && dPhi >= deltaPhiMin) {
                        maxJetPt = jetPt;
                        bestJetIndex = iJet;
                    }
                }
                
                // If jet found, store its information
                if (bestJetIndex >= 0) {
                    hasSelectedJet = true;
                    selectedJetIndexes[collection] = bestJetIndex;
                    
                    // Retrieve jet properties with bounds checking
                    selectedJetPts[collection] = jetManager.getJetPt(collection, bestJetIndex);
                    selectedJetEtas[collection] = jetManager.getJetEta(collection, bestJetIndex);
                    selectedJetPhis[collection] = jetManager.getJetPhi(collection, bestJetIndex);
                    selectedJetMasses[collection] = jetManager.getJetMass(collection, bestJetIndex);
                    selectedJetAreas[collection] = jetManager.getJetArea(collection, bestJetIndex);
                    
                    // Jet substructure variables with validation
                    selectedJetDynSplits[collection] = jetManager.getJetDynSplit(collection, bestJetIndex);
                    selectedJetDynKts[collection] = jetManager.getJetDynKt(collection, bestJetIndex);
                    selectedJetDynZs[collection] = jetManager.getJetDynZ(collection, bestJetIndex);
                    selectedJetGirths[collection] = jetManager.getJetGirth(collection, bestJetIndex);
                    selectedJetThrusts[collection] = jetManager.getJetThrust(collection, bestJetIndex);
                    selectedJetLHAs[collection] = jetManager.getJetLHA(collection, bestJetIndex);
                    selectedJetPtDs[collection] = jetManager.getJetPtD(collection, bestJetIndex);
                    selectedJetDynDeltaRs[collection] = jetManager.getJetDynDeltaR(collection, bestJetIndex);
                    selectedJetIntJetMultis[collection] = jetManager.getJetIntJetMulti(collection, bestJetIndex);
                    
                    // Retrieve ref jet properties (MC-matched jets)
                    selectedRefJetPts[collection] = jetManager.getRefJetPt(collection, bestJetIndex);
                    selectedRefJetEtas[collection] = jetManager.getRefJetEta(collection, bestJetIndex);
                    selectedRefJetPhis[collection] = jetManager.getRefJetPhi(collection, bestJetIndex);
                    selectedRefJetMasses[collection] = jetManager.getRefJetMass(collection, bestJetIndex);
                    selectedRefJetAreas[collection] = jetManager.getRefJetArea(collection, bestJetIndex);
                    selectedRefJetDynSplits[collection] = jetManager.getRefJetDynSplit(collection, bestJetIndex);
                    selectedRefJetDynKts[collection] = jetManager.getRefJetDynKt(collection, bestJetIndex);
                    selectedRefJetDynZs[collection] = jetManager.getRefJetDynZ(collection, bestJetIndex);
                    selectedRefJetGirths[collection] = jetManager.getRefJetGirth(collection, bestJetIndex);
                    selectedRefJetThrusts[collection] = jetManager.getRefJetThrust(collection, bestJetIndex);
                    selectedRefJetLHAs[collection] = jetManager.getRefJetLHA(collection, bestJetIndex);
                    selectedRefJetPtDs[collection] = jetManager.getRefJetPtD(collection, bestJetIndex);
                    selectedRefJetDynDeltaRs[collection] = jetManager.getRefJetDynDeltaR(collection, bestJetIndex);
                    selectedRefJetIntJetMultis[collection] = jetManager.getRefJetIntJetMulti(collection, bestJetIndex);
                } else {
                    // Reset values when no jet is found
                    selectedJetPts[collection] = -999;
                    selectedJetEtas[collection] = -999;
                    selectedJetPhis[collection] = -999;
                    selectedJetMasses[collection] = -999;
                    selectedJetAreas[collection] = -999;
                    selectedJetDynSplits[collection] = -999;
                    selectedJetDynKts[collection] = -999;
                    selectedJetDynZs[collection] = -999;
                    selectedJetGirths[collection] = -999;
                    selectedJetThrusts[collection] = -999;
                    selectedJetLHAs[collection] = -999;
                    selectedJetPtDs[collection] = -999;
                }
                
                // Calculate correlation variables
                selectedJetDeltaPhis[collection] = getDeltaPhi(selectedPhotonPhi, selectedJetPhis[collection]);
                selectedJetXjs[collection] = getXj(selectedJetPts[collection], selectedPhotonEt);
                
                // Check and apply DeltaPhi cut if configured
                float deltaPhiMin = config->GetValue("DeltaPhiMin", -1.0);
                if (deltaPhiMin > 0) {
                    bool passDeltaPhi = selectedJetDeltaPhis[collection] >= deltaPhiMin;
                    cutFlowTracker.applyCut("DeltaPhi", passDeltaPhi);
                    
                    // Skip jets not passing delta phi cut
                    if (!passDeltaPhi) {
                        selectedJetIndexes[collection] = -1;
                    }
                }
                
                // Check and apply XJ cut if configured
                float xjMin = config->GetValue("XjMin", -1.0);
                if (xjMin > 0) {
                    bool passXj = selectedJetXjs[collection] >= xjMin;
                    cutFlowTracker.applyCut("XjCut", passXj);
                    
                    // Skip jets not passing xj cut
                    if (!passXj) {
                        selectedJetIndexes[collection] = -1;
                    }
                }
                
                // Fill histograms for this collection
                int centBin = -1;
                for (size_t i = 0; i < centralityBins.size() - 1; ++i) {
                    if (hiBin >= centralityBins[i] && hiBin < centralityBins[i+1]) {
                        centBin = i;
                        break;
                    }
                }
                
                if (centBin >= 0) {
                    std::string centName = "cent" + std::to_string(static_cast<int>(centralityBins[centBin])) + 
                                         "to" + std::to_string(static_cast<int>(centralityBins[centBin+1]));
                    
                    log(LOG_TRACE, "Filling histograms for " + collection + "/" + centName + 
                        " with weight: " + std::to_string(eventWeight));
                    
                    // Navigate to subdirectory and fill histograms with weights
                    outFile->cd();
                    if (outFile->cd((collection + "/" + centName).c_str())) {
                        TH1F* hJetPt = (TH1F*)gDirectory->Get("hJetPt");
                        if (hJetPt) hJetPt->Fill(selectedJetPts[collection], eventWeight);
                        
                        TH1F* hJetEta = (TH1F*)gDirectory->Get("hJetEta");
                        if (hJetEta) hJetEta->Fill(selectedJetEtas[collection], eventWeight);
                        
                        TH1F* hJetPhi = (TH1F*)gDirectory->Get("hJetPhi");
                        if (hJetPhi) hJetPhi->Fill(selectedJetPhis[collection], eventWeight);
                        
                        TH1F* hDeltaPhi = (TH1F*)gDirectory->Get("hDeltaPhi");
                        if (hDeltaPhi) hDeltaPhi->Fill(selectedJetDeltaPhis[collection], eventWeight);
                        
                        TH1F* hXj = (TH1F*)gDirectory->Get("hXj");
                        if (hXj) hXj->Fill(selectedJetXjs[collection], eventWeight);
                        
                        // Photon histograms
                        TH1F* hPhotonEt = (TH1F*)gDirectory->Get("hPhotonEt");
                        if (hPhotonEt) hPhotonEt->Fill(selectedPhotonEt, eventWeight);
                        
                        TH1F* hPhotonEta = (TH1F*)gDirectory->Get("hPhotonEta");
                        if (hPhotonEta) hPhotonEta->Fill(selectedPhotonEta, eventWeight);
                        
                        // Photon identification histograms
                        TH1F* hPhotonHoverE = (TH1F*)gDirectory->Get("hPhotonHoverE");
                        if (hPhotonHoverE) hPhotonHoverE->Fill(selectedPhotonHoverE, eventWeight);
                        
                        TH1F* hPhotonSigmaIEtaIEta = (TH1F*)gDirectory->Get("hPhotonSigmaIEtaIEta");
                        if (hPhotonSigmaIEtaIEta) hPhotonSigmaIEtaIEta->Fill(selectedPhotonSigmaIEtaIEta, eventWeight);
                        
                        TH1F* hPhotonIso = (TH1F*)gDirectory->Get("hPhotonIso");
                        if (hPhotonIso) hPhotonIso->Fill(selectedPhotonIso, eventWeight);
                        
                        TH1F* hPhotonR9 = (TH1F*)gDirectory->Get("hPhotonR9");
                        if (hPhotonR9) hPhotonR9->Fill(selectedPhotonR9, eventWeight);
                        
                        // Jet substructure histograms
                        TH1F* hJetMass = (TH1F*)gDirectory->Get("hJetMass");
                        if (hJetMass) hJetMass->Fill(selectedJetMasses[collection], eventWeight);
                        
                        TH1F* hJetArea = (TH1F*)gDirectory->Get("hJetArea");
                        if (hJetArea) hJetArea->Fill(selectedJetAreas[collection], eventWeight);
                        
                        TH1F* hDynDeltaR = (TH1F*)gDirectory->Get("hDynDeltaR");
                        if (hDynDeltaR) hDynDeltaR->Fill(selectedJetDynDeltaRs[collection], eventWeight);
                        
                        TH1F* hIntJetMulti = (TH1F*)gDirectory->Get("hIntJetMulti");
                        if (hIntJetMulti) hIntJetMulti->Fill(selectedJetIntJetMultis[collection], eventWeight);
                        
                        TH1F* hDynSplit = (TH1F*)gDirectory->Get("hDynSplit");
                        if (hDynSplit) hDynSplit->Fill(selectedJetDynSplits[collection], eventWeight);
                        
                        TH1F* hDynKt = (TH1F*)gDirectory->Get("hDynKt");
                        if (hDynKt) hDynKt->Fill(selectedJetDynKts[collection], eventWeight);
                        
                        TH1F* hDynZ = (TH1F*)gDirectory->Get("hDynZ");
                        if (hDynZ) hDynZ->Fill(selectedJetDynZs[collection], eventWeight);
                        
                        TH1F* hGirth = (TH1F*)gDirectory->Get("hGirth");
                        if (hGirth) hGirth->Fill(selectedJetGirths[collection], eventWeight);
                        
                        TH1F* hThrust = (TH1F*)gDirectory->Get("hThrust");
                        if (hThrust) hThrust->Fill(selectedJetThrusts[collection], eventWeight);
                        
                        TH1F* hLHA = (TH1F*)gDirectory->Get("hLHA");
                        if (hLHA) hLHA->Fill(selectedJetLHAs[collection], eventWeight);
                        
                        TH1F* hPtD = (TH1F*)gDirectory->Get("hPtD");
                        if (hPtD) hPtD->Fill(selectedJetPtDs[collection], eventWeight);
                        
                        // Ref jet (MC-matched) histograms - only fill if MC data and valid ref jet values
                        if (isMC && selectedRefJetPts[collection] > -900) {
                            TH1F* hRefJetPt = (TH1F*)gDirectory->Get("hRefJetPt");
                            if (hRefJetPt) hRefJetPt->Fill(selectedRefJetPts[collection], eventWeight);
                            
                            TH1F* hRefJetEta = (TH1F*)gDirectory->Get("hRefJetEta");
                            if (hRefJetEta) hRefJetEta->Fill(selectedRefJetEtas[collection], eventWeight);
                            
                            TH1F* hRefJetPhi = (TH1F*)gDirectory->Get("hRefJetPhi");
                            if (hRefJetPhi) hRefJetPhi->Fill(selectedRefJetPhis[collection], eventWeight);
                            
                            TH1F* hRefJetMass = (TH1F*)gDirectory->Get("hRefJetMass");
                            if (hRefJetMass) hRefJetMass->Fill(selectedRefJetMasses[collection], eventWeight);
                            
                            TH1F* hRefJetArea = (TH1F*)gDirectory->Get("hRefJetArea");
                            if (hRefJetArea) hRefJetArea->Fill(selectedRefJetAreas[collection], eventWeight);
                            
                            TH1F* hRefDynDeltaR = (TH1F*)gDirectory->Get("hRefDynDeltaR");
                            if (hRefDynDeltaR) hRefDynDeltaR->Fill(selectedRefJetDynDeltaRs[collection], eventWeight);
                            
                            TH1F* hRefIntJetMulti = (TH1F*)gDirectory->Get("hRefIntJetMulti");
                            if (hRefIntJetMulti) hRefIntJetMulti->Fill(selectedRefJetIntJetMultis[collection], eventWeight);
                            
                            TH1F* hRefDynSplit = (TH1F*)gDirectory->Get("hRefDynSplit");
                            if (hRefDynSplit) hRefDynSplit->Fill(selectedRefJetDynSplits[collection], eventWeight);
                            
                            TH1F* hRefDynKt = (TH1F*)gDirectory->Get("hRefDynKt");
                            if (hRefDynKt) hRefDynKt->Fill(selectedRefJetDynKts[collection], eventWeight);
                            
                            TH1F* hRefDynZ = (TH1F*)gDirectory->Get("hRefDynZ");
                            if (hRefDynZ) hRefDynZ->Fill(selectedRefJetDynZs[collection], eventWeight);
                            
                            TH1F* hRefGirth = (TH1F*)gDirectory->Get("hRefGirth");
                            if (hRefGirth) hRefGirth->Fill(selectedRefJetGirths[collection], eventWeight);
                            
                            TH1F* hRefThrust = (TH1F*)gDirectory->Get("hRefThrust");
                            if (hRefThrust) hRefThrust->Fill(selectedRefJetThrusts[collection], eventWeight);
                            
                            TH1F* hRefLHA = (TH1F*)gDirectory->Get("hRefLHA");
                            if (hRefLHA) hRefLHA->Fill(selectedRefJetLHAs[collection], eventWeight);
                            
                            TH1F* hRefPtD = (TH1F*)gDirectory->Get("hRefPtD");
                            if (hRefPtD) hRefPtD->Fill(selectedRefJetPtDs[collection], eventWeight);
                        }
                        
                        // Event-level histograms
                        TH1F* hNPhotons = (TH1F*)gDirectory->Get("hNPhotons");
                        if (hNPhotons) hNPhotons->Fill(nPhotons, eventWeight);
                        
                        TH1F* hNJets = (TH1F*)gDirectory->Get("hNJets");
                        if (hNJets) hNJets->Fill(jetManager.getNJets(collection), eventWeight);
                        
                        TH1F* hEventWeight = (TH1F*)gDirectory->Get("hEventWeight");
                        if (hEventWeight) hEventWeight->Fill(eventWeight, eventWeight);
                        
                        TH1F* hVz = (TH1F*)gDirectory->Get("hVz");
                        if (hVz) hVz->Fill(vz, eventWeight);
                        
                        TH1F* hHiHF = (TH1F*)gDirectory->Get("hHiHF");
                        if (hHiHF) hHiHF->Fill(hiHF, eventWeight);
                        
                        TH1F* hCentrality = (TH1F*)gDirectory->Get("hCentrality");
                        if (hCentrality) hCentrality->Fill(hiBin, eventWeight);
                        
                        // 2D histograms
                        TH2F* h2JetPtVsEta = (TH2F*)gDirectory->Get("h2JetPtVsEta");
                        if (h2JetPtVsEta) h2JetPtVsEta->Fill(selectedJetEtas[collection], selectedJetPts[collection], eventWeight);
                        
                        TH2F* h2JetMassVsPt = (TH2F*)gDirectory->Get("h2JetMassVsPt");
                        if (h2JetMassVsPt) h2JetMassVsPt->Fill(selectedJetPts[collection], selectedJetMasses[collection], eventWeight);
                        
                        // Additional jet substructure 2D histograms
                        TH2F* h2GirthVsPt = (TH2F*)gDirectory->Get("h2GirthVsPt");
                        if (h2GirthVsPt) h2GirthVsPt->Fill(selectedJetPts[collection], selectedJetGirths[collection], eventWeight);
                        
                        TH2F* h2ThrustVsPt = (TH2F*)gDirectory->Get("h2ThrustVsPt");
                        if (h2ThrustVsPt) h2ThrustVsPt->Fill(selectedJetPts[collection], selectedJetThrusts[collection], eventWeight);
                        
                        TH2F* h2PtDVsPt = (TH2F*)gDirectory->Get("h2PtDVsPt");
                        if (h2PtDVsPt) h2PtDVsPt->Fill(selectedJetPts[collection], selectedJetPtDs[collection], eventWeight);
                        
                        // Profile histograms for jet substructure evolution
                        TProfile* pGirthVsPt = (TProfile*)gDirectory->Get("pGirthVsPt");
                        if (pGirthVsPt) pGirthVsPt->Fill(selectedJetPts[collection], selectedJetGirths[collection], eventWeight);
                                                
                        TProfile* pThrustVsPt = (TProfile*)gDirectory->Get("pThrustVsPt");
                        if (pThrustVsPt) pThrustVsPt->Fill(selectedJetPts[collection], selectedJetThrusts[collection], eventWeight);
                        
                        TProfile* pPtDVsPt = (TProfile*)gDirectory->Get("pPtDVsPt");
                        if (pPtDVsPt) pPtDVsPt->Fill(selectedJetPts[collection], selectedJetPtDs[collection], eventWeight);

                        
                        // Ref jet profile histograms - only fill if MC data and valid ref jet values
                        if (isMC && selectedRefJetPts[collection] > -900) {
                            // Ref jet 2D histograms
                            TH2F* h2RefJetPtVsEta = (TH2F*)gDirectory->Get("h2RefJetPtVsEta");
                            if (h2RefJetPtVsEta) h2RefJetPtVsEta->Fill(selectedRefJetEtas[collection], selectedRefJetPts[collection], eventWeight);
                            
                            TH2F* h2RefJetMassVsPt = (TH2F*)gDirectory->Get("h2RefJetMassVsPt");
                            if (h2RefJetMassVsPt) h2RefJetMassVsPt->Fill(selectedRefJetPts[collection], selectedRefJetMasses[collection], eventWeight);
                            
                            TH2F* h2RefGirthVsPt = (TH2F*)gDirectory->Get("h2RefGirthVsPt");
                            if (h2RefGirthVsPt) h2RefGirthVsPt->Fill(selectedRefJetPts[collection], selectedRefJetGirths[collection], eventWeight);
                            
                            TH2F* h2RefThrustVsPt = (TH2F*)gDirectory->Get("h2RefThrustVsPt");
                            if (h2RefThrustVsPt) h2RefThrustVsPt->Fill(selectedRefJetPts[collection], selectedRefJetThrusts[collection], eventWeight);
                            
                            TH2F* h2RefPtDVsPt = (TH2F*)gDirectory->Get("h2RefPtDVsPt");
                            if (h2RefPtDVsPt) h2RefPtDVsPt->Fill(selectedRefJetPts[collection], selectedRefJetPtDs[collection], eventWeight);

                            // Reco vs Ref correlation histograms
                            TH2F* h2JetVsRefPt = (TH2F*)gDirectory->Get("h2JetVsRefPt");
                            if (h2JetVsRefPt) h2JetVsRefPt->Fill(selectedRefJetPts[collection], selectedJetPts[collection], eventWeight);
                            
                            TH2F* h2JetVsRefMass = (TH2F*)gDirectory->Get("h2JetVsRefMass");
                            if (h2JetVsRefMass) h2JetVsRefMass->Fill(selectedRefJetMasses[collection], selectedJetMasses[collection], eventWeight);
                            
                            TH2F* h2JetVsRefGirth = (TH2F*)gDirectory->Get("h2JetVsRefGirth");
                            if (h2JetVsRefGirth) h2JetVsRefGirth->Fill(selectedRefJetGirths[collection], selectedJetGirths[collection], eventWeight);
                            
                            TH2F* h2JetVsRefThrust = (TH2F*)gDirectory->Get("h2JetVsRefThrust");
                            if (h2JetVsRefThrust) h2JetVsRefThrust->Fill(selectedRefJetThrusts[collection], selectedJetThrusts[collection], eventWeight);
                            
                            TH2F* h2JetVsRefPtD = (TH2F*)gDirectory->Get("h2JetVsRefPtD");
                            if (h2JetVsRefPtD) h2JetVsRefPtD->Fill(selectedRefJetPtDs[collection], selectedJetPtDs[collection], eventWeight);

                            
                            // Ref jet profile histograms
                            TProfile* pRefGirthVsPt = (TProfile*)gDirectory->Get("pRefGirthVsPt");
                            if (pRefGirthVsPt) pRefGirthVsPt->Fill(selectedRefJetPts[collection], selectedRefJetGirths[collection], eventWeight);
                            
                            TProfile* pRefThrustVsPt = (TProfile*)gDirectory->Get("pRefThrustVsPt");
                            if (pRefThrustVsPt) pRefThrustVsPt->Fill(selectedRefJetPts[collection], selectedRefJetThrusts[collection], eventWeight);
                            
                            TProfile* pRefPtDVsPt = (TProfile*)gDirectory->Get("pRefPtDVsPt");
                            if (pRefPtDVsPt) pRefPtDVsPt->Fill(selectedRefJetPts[collection], selectedRefJetPtDs[collection], eventWeight);
                        }
                        
                        // MC-specific histogram filling (only if MC data and MC branches available)
                        if (isMC && selectedPhotonIndex >= 0) {
                            // Fill photon gen match index
                            TH1F* hPhotonGenMatch = (TH1F*)gDirectory->Get("hPhotonGenMatch");
                            if (hPhotonGenMatch && phoGenMatchedIndex) {
                                hPhotonGenMatch->Fill(phoGenMatchedIndex->at(selectedPhotonIndex), eventWeight);
                            }
                            
                            // Fill MC photon information if we have a valid gen match
                            if (phoGenMatchedIndex && phoGenMatchedIndex->at(selectedPhotonIndex) >= 0) {
                                int genIndex = phoGenMatchedIndex->at(selectedPhotonIndex);
                                
                                // Check bounds for MC vectors before accessing
                                if (mcPt && genIndex < static_cast<int>(mcPt->size())) {
                                    TH1F* hMCPhotonPt = (TH1F*)gDirectory->Get("hMCPhotonPt");
                                    if (hMCPhotonPt) hMCPhotonPt->Fill(mcPt->at(genIndex), eventWeight);
                                }
                                
                                if (mcEta && genIndex < static_cast<int>(mcEta->size())) {
                                    TH1F* hMCPhotonEta = (TH1F*)gDirectory->Get("hMCPhotonEta");
                                    if (hMCPhotonEta) hMCPhotonEta->Fill(mcEta->at(genIndex), eventWeight);
                                }
                                
                                if (mcPhi && genIndex < static_cast<int>(mcPhi->size())) {
                                    TH1F* hMCPhotonPhi = (TH1F*)gDirectory->Get("hMCPhotonPhi");
                                    if (hMCPhotonPhi) hMCPhotonPhi->Fill(mcPhi->at(genIndex), eventWeight);
                                }
                                
                                if (mcPID && genIndex < static_cast<int>(mcPID->size())) {
                                    TH1F* hMCPhotonPID = (TH1F*)gDirectory->Get("hMCPhotonPID");
                                    if (hMCPhotonPID) hMCPhotonPID->Fill(mcPID->at(genIndex), eventWeight);
                                }
                                
                                if (mcMomPID && genIndex < static_cast<int>(mcMomPID->size())) {
                                    TH1F* hMCPhotonMomPID = (TH1F*)gDirectory->Get("hMCPhotonMomPID");
                                    if (hMCPhotonMomPID) hMCPhotonMomPID->Fill(mcMomPID->at(genIndex), eventWeight);
                                }
                                
                                // Fill gen vs reco comparison
                                if (mcPt && genIndex < static_cast<int>(mcPt->size())) {
                                    TH2F* h2PhotonGenVsReco = (TH2F*)gDirectory->Get("h2PhotonGenVsReco");
                                    if (h2PhotonGenVsReco) {
                                        h2PhotonGenVsReco->Fill(selectedPhotonEt, mcPt->at(genIndex), eventWeight);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
            // Track final JetSelection based on whether we found any jet
            cutFlowTracker.applyCut("JetSelection", hasSelectedJet);
        }
        
        // Final determination if event passes all cuts (photon and at least one good jet)
        bool passFinalSelection = hasSelectedJet;
        cutFlowTracker.applyCut("FinalSelection", passFinalSelection);
        
        if (passFinalSelection) {
            nWithJet++;
            
            // Fill output tree
            outTree->Fill();
            nPassed++;
        }
    }
    
    // Write output tree
    outFile->cd();
    outTree->Write();
    
    // Write all histograms explicitly
    log(LOG_INFO, "Writing histograms to file...");
    for (const auto& collection : jetCollections) {
        TDirectory* collectionDir = outFile->GetDirectory(collection.c_str());
        if (collectionDir) {
            log(LOG_DEBUG, "Writing histograms for collection: " + collection);
            collectionDir->Write("", TObject::kOverwrite);
        }
    }
    
    // Force write everything
    outFile->Write("", TObject::kOverwrite);
    
    log(LOG_INFO, "All histograms written successfully.");
    
    // Print summary
    log(LOG_INFO, "==================================================");
    log(LOG_INFO, "Analysis Summary:");
    log(LOG_INFO, "  Total events processed: " + std::to_string(nProcessed));
    log(LOG_INFO, "  Events with selected photon: " + std::to_string(nWithPhoton));
    log(LOG_INFO, "  Events with selected jet: " + std::to_string(nWithJet));
    log(LOG_INFO, "  Events passing all selections: " + std::to_string(nPassed));
    log(LOG_INFO, "==================================================");
    
    // Print the cut flow summary
    cutFlowTracker.printCutFlow();
    
    // Save the cut flow information to the output file
    outFile->cd(); // Make sure we're in the right directory
    cutFlowTracker.saveCutFlowToFile(outFile);
}

/**
 * Setup the input chain with files from the input directory
 */
bool setupInputChain(TChain* chain, const std::string& inputDir, bool testMode, int maxFiles) {
    if (!chain) return false;
    
    if (inputDir.empty()) {
        std::cerr << "Error: Input directory not specified." << std::endl;
        return false;
    }
    
    // Check if inputDir is a single file (ends with .root)
    if (inputDir.size() > 5 && inputDir.substr(inputDir.size() - 5) == ".root") {
        log(LOG_INFO, "Adding single file: " + inputDir);
        if (chain->AddFile(inputDir.c_str()) <= 0) {
            std::cerr << "Error: Failed to add file to chain: " << inputDir << std::endl;
            return false;
        }
        return true;
    }
    
    // Find all ROOT files in the directory
    std::string cmd = "find " + inputDir + " -name \"*.root\" -type f";
    log(LOG_DEBUG, "Executing: " + cmd);
    
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        std::cerr << "Error: Failed to run command: " << cmd << std::endl;
        return false;
    }
    
    std::vector<std::string> files;
    char buffer[1024];
    while (!feof(pipe)) {
        if (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            std::string file = buffer;
            // Remove newline characters
            file.erase(std::remove(file.begin(), file.end(), '\n'), file.end());
            if (!file.empty()) {
                files.push_back(file);
            }
        }
    }
    pclose(pipe);
    
    // Limit number of files in test mode
    if (testMode && files.size() > static_cast<size_t>(maxFiles)) {
        maxFiles = 1;
        log(LOG_INFO, "Test mode: limiting to " + std::to_string(maxFiles) + " files out of " + std::to_string(files.size()));
        files.resize(maxFiles);
    }
    
    // Add files to chain
    for (const auto& file : files) {
        log(LOG_DEBUG, "Adding file: " + file);
        if (chain->AddFile(file.c_str()) <= 0) {
            std::cerr << "Warning: Failed to add file to chain: " << file << std::endl;
            continue;
        }
    }
    
    if (chain->GetEntries() <= 0) {
        std::cerr << "Error: No entries in chain." << std::endl;
        return false;
    }
    
    log(LOG_INFO, "Added " + std::to_string(files.size()) + " files with " + std::to_string(chain->GetEntries()) + " entries.");
    return true;
}

/**
 * Create histograms for output
 */
void createHistograms(TFile* outFile, const std::vector<std::string>& jetCollections, 
                                 const std::vector<float>& centralityBins, const PlottingConfiguration& plotConfig) {
    if (!outFile) return;
    
    // Debug: Check input parameters
    log(LOG_DEBUG, "createHistograms called with:");
    log(LOG_DEBUG, "  jetCollections.size() = " + std::to_string(jetCollections.size()));
    log(LOG_DEBUG, "  centralityBins.size() = " + std::to_string(centralityBins.size()));
    log(LOG_DEBUG, "  plotConfig.histogramConfigs.size() = " + std::to_string(plotConfig.histogramConfigs.size()));
    
    if (jetCollections.empty()) {
        log(LOG_INFO, "jetCollections is empty!");
        return;
    }
    
    if (centralityBins.size() < 2) {
        log(LOG_INFO, "centralityBins has < 2 elements, cannot create bins!");
        std::string binContents = "centralityBins contents: ";
        for (float bin : centralityBins) {
            binContents += std::to_string(bin) + " ";
        }
        log(LOG_INFO, binContents);
        return;
    }
    
    // Determine if this is MC data by checking for DataType config
    bool isMC = false;
    if (plotConfig.configFile) {
        std::string dataType = plotConfig.configFile->GetValue("DataType", "Data");
        isMC = (dataType == "MC" || dataType == "mc");
        log(LOG_DEBUG, "  isMC: " + std::to_string(isMC) + " (from DataType: " + dataType + ")");
    }
    
    // Create histograms for each jet collection and centrality bin
    for (const auto& collection : jetCollections) {
        outFile->cd();
        TDirectory* collectionDir = outFile->mkdir(collection.c_str());
        if (!collectionDir) {
            std::cerr << "Error: Failed to create directory for collection: " << collection << std::endl;
            continue;
        }
        log(LOG_DEBUG, "Creating directory: " + collection);
        
        for (size_t i = 0; i < centralityBins.size() - 1; ++i) {
            std::string centName = "cent" + std::to_string(static_cast<int>(centralityBins[i])) + 
                                 "to" + std::to_string(static_cast<int>(centralityBins[i+1]));
            
            // Create subdirectory within the collection directory
            collectionDir->cd();
            TDirectory* centDir = collectionDir->mkdir(centName.c_str());
            if (!centDir) {
                std::cerr << "Error: Failed to create directory: " << centName << " in " << collection << std::endl;
                continue;
            }
            centDir->cd();
            
            log(LOG_DEBUG, "Creating histograms in directory: " + collection + "/" + centName);
            
            // Vector to store all histogram pointers for writing
            std::vector<TObject*> histograms;
            
            // Helper function to create a histogram using configuration if available,
            // or use defaults if not found in the config
            auto createHist1D = [&plotConfig, &centDir, &histograms](const std::string& name, const std::string& title, 
                                                                  int bins, double xmin, double xmax) {
                // Remove 'h' prefix if present for config lookup
                std::string configName = name;
                if (configName.size() > 1 && configName[0] == 'h') {
                    configName = configName.substr(1);
                }
                
                TH1F* hist = nullptr;
                
                // Check if this histogram has a config
                auto configIter = plotConfig.histogramConfigs.find(configName);
                if (configIter != plotConfig.histogramConfigs.end()) {
                    // Create with config
                    hist = createHistogram1D(configIter->second, name, "");
                } else {
                    // Create with provided defaults
                    hist = new TH1F(name.c_str(), title.c_str(), bins, xmin, xmax);
                }
                
                hist->SetDirectory(centDir);
                histograms.push_back(hist);
                return hist;
            };
            
            // Similar helper for 2D histograms
            auto createHist2D = [&plotConfig, &centDir, &histograms](const std::string& name, const std::string& title, 
                                                                  int xbins, double xmin, double xmax,
                                                                  int ybins, double ymin, double ymax) {
                // Remove 'h2' prefix if present for config lookup
                std::string configName = name;
                if (configName.size() > 2 && configName[0] == 'h' && configName[1] == '2') {
                    configName = configName.substr(2);
                }
                
                TH2F* hist = nullptr;
                
                // Check if this histogram has a config
                auto configIter = plotConfig.histogramConfigs.find(configName);
                if (configIter != plotConfig.histogramConfigs.end()) {
                    // Create with config
                    hist = createHistogram2D(configIter->second, name, "");
                } else {
                    // Create with provided defaults
                    hist = new TH2F(name.c_str(), title.c_str(), xbins, xmin, xmax, ybins, ymin, ymax);
                }
                
                hist->SetDirectory(centDir);
                histograms.push_back(hist);
                return hist;
            };
            
            // Helper for profile histograms
            auto createHistProfile = [&plotConfig, &centDir, &histograms](const std::string& name, const std::string& title, 
                                                                       int xbins, double xmin, double xmax,
                                                                       double ymin, double ymax) {
                // Remove 'p' prefix if present for config lookup
                std::string configName = name;
                if (configName.size() > 1 && configName[0] == 'p') {
                    configName = configName.substr(1);
                }
                
                TProfile* hist = nullptr;
                
                // Check if this histogram has a config
                auto configIter = plotConfig.histogramConfigs.find(configName);
                if (configIter != plotConfig.histogramConfigs.end()) {
                    // Create with config
                    hist = createProfile(configIter->second, name, "");
                } else {
                    // Create with provided defaults
                    hist = new TProfile(name.c_str(), title.c_str(), xbins, xmin, xmax, ymin, ymax);
                }
                
                hist->SetDirectory(centDir);
                histograms.push_back(hist);
                return hist;
            };
            
            // Create all histograms using our helper functions
            // Basic kinematic histograms
            TH1F* hJetPt = createHist1D("hJetPt", "Jet p_{T};p_{T} [GeV/c];Entries", 100, 0, 500);
            TH1F* hJetEta = createHist1D("hJetEta", "Jet #eta;#eta;Entries", 50, -2.5, 2.5);
            TH1F* hJetPhi = createHist1D("hJetPhi", "Jet #phi;#phi [rad];Entries", 50, -3.14159, 3.14159);
            TH1F* hDeltaPhi = createHist1D("hDeltaPhi", "#Delta#phi;#Delta#phi;Entries", 50, 0, M_PI);
            TH1F* hXj = createHist1D("hXj", "x_{j} = p_{T}^{jet} / E_{T}^{#gamma};x_{j};Entries", 50, 0, 2.0);
            
            // Photon histograms
            TH1F* hPhotonEt = createHist1D("hPhotonEt", "Photon E_{T};E_{T} [GeV];Entries", 100, 0, 400);
            TH1F* hPhotonEta = createHist1D("hPhotonEta", "Photon #eta;#eta;Entries", 50, -2.5, 2.5);
            
            // Photon identification histograms
            TH1F* hPhotonHoverE = createHist1D("hPhotonHoverE", "Photon H/E;H/E;Entries", 50, 0, 0.2);
            TH1F* hPhotonSigmaIEtaIEta = createHist1D("hPhotonSigmaIEtaIEta", "Photon #sigma_{i#eta i#eta};#sigma_{i#eta i#eta};Entries", 50, 0, 0.03);
            TH1F* hPhotonIso = createHist1D("hPhotonIso", "Photon Isolation;Isolation [GeV];Entries", 50, 0, 10.0);
            TH1F* hPhotonR9 = createHist1D("hPhotonR9", "Photon R9;R9;Entries", 50, 0.8, 1.0);
            
            // Jet substructure histograms
            TH1F* hJetMass = createHist1D("hJetMass", "Jet mass;m [GeV/c^{2}];Entries", 50, 0, 50);
            TH1F* hJetArea = createHist1D("hJetArea", "Jet area;Area;Entries", 50, 0, 1);
            TH1F* hDynSplit = createHist1D("hDynSplit", "Number of Splits;nSD;Entries", 50, 0, 50);
            TH1F* hDynKt = createHist1D("hDynKt", "Dynamical groomed k_{T};k_{T} [GeV/c];Entries", 50, 0, 50);
            TH1F* hDynZ = createHist1D("hDynZ", "Dynamical groomed z;z;Entries", 50, 0, 0.5);
            TH1F* hGirth = createHist1D("hGirth", "Jet girth;girth;Entries", 50, 0, 0.5);
            TH1F* hThrust = createHist1D("hThrust", "Jet thrust;thrust;Entries", 50, 0, 1.0);
            TH1F* hLHA = createHist1D("hLHA", "Jet LHA;LHA;Entries", 50, 0, 1.0);
            TH1F* hPtD = createHist1D("hPtD", "Jet p_{T}D;p_{T}D;Entries", 50, 0, 1.0);
            TH1F* hDynDeltaR = createHist1D("hDynDeltaR", "Jet R_{g};R_{g};Entries", 20, 0, 0.5);
            TH1F* hIntJetMulti = createHist1D("hIntJetMulti", "Intra Jet Multiplicity;Intra Jet Multiplicity;Entries", 15, 0, 15);
            
            // Ref jet (MC-matched) histograms
            TH1F* hRefJetPt = createHist1D("hRefJetPt", "Ref Jet p_{T};p_{T} [GeV/c];Entries", 100, 0, 500);
            TH1F* hRefJetEta = createHist1D("hRefJetEta", "Ref Jet #eta;#eta;Entries", 50, -2.5, 2.5);
            TH1F* hRefJetPhi = createHist1D("hRefJetPhi", "Ref Jet #phi;#phi [rad];Entries", 50, -3.14159, 3.14159);
            TH1F* hRefJetMass = createHist1D("hRefJetMass", "Ref Jet mass;m [GeV/c^{2}];Entries", 50, 0, 50);
            TH1F* hRefJetArea = createHist1D("hRefJetArea", "Ref Jet area;Area;Entries", 50, 0, 1);
            TH1F* hRefDynSplit = createHist1D("hRefDynSplit", "Ref Number of Splits;nSD;Entries", 50, 0, 50);
            TH1F* hRefDynKt = createHist1D("hRefDynKt", "Ref Dynamical groomed k_{T};k_{T} [GeV/c];Entries", 50, 0, 50);
            TH1F* hRefDynZ = createHist1D("hRefDynZ", "Ref Dynamical groomed z;z;Entries", 50, 0, 0.5);
            TH1F* hRefGirth = createHist1D("hRefGirth", "Ref Jet girth;girth;Entries", 50, 0, 0.5);
            TH1F* hRefThrust = createHist1D("hRefThrust", "Ref Jet thrust;thrust;Entries", 50, 0, 1.0);
            TH1F* hRefLHA = createHist1D("hRefLHA", "Ref Jet LHA;LHA;Entries", 50, 0, 1.0);
            TH1F* hRefPtD = createHist1D("hRefPtD", "Ref Jet p_{T}D;p_{T}D;Entries", 50, 0, 1.0);
            TH1F* hRefDynDeltaR = createHist1D("hRefDynDeltaR", "Jet R_{g};R_{g};Entries", 20, 0, 0.5);
            TH1F* hRefIntJetMulti = createHist1D("hRefIntJetMulti", "Intra Jet Multiplicity;Intra Jet Multiplicity;Entries", 15, 0, 15);
            
            // Event-level histograms
            TH1F* hNPhotons = createHist1D("hNPhotons", "Number of Photons;N_{#gamma};Entries", 20, 0, 20);
            TH1F* hNJets = createHist1D("hNJets", "Number of Jets;N_{jets};Entries", 50, 0, 50);
            TH1F* hEventWeight = createHist1D("hEventWeight", "Event Weight;Weight;Entries", 100, 0, 2);
            TH1F* hVz = createHist1D("hVz", "Primary Vertex z;v_{z} [cm];Entries", 100, -20, 20);
            TH1F* hHiHF = createHist1D("hHiHF", "HF Energy Sum;#Sigma E_{T}^{HF} [GeV];Entries", 100, 0, 7000);
            TH1F* hCentrality = createHist1D("hCentrality", "Centrality;Centrality [%];Entries", 100, 0, 100);
            
            // 2D histograms
            TH2F* h2JetPtVsEta = createHist2D("h2JetPtVsEta", "Jet p_{T} vs #eta;#eta;p_{T} [GeV/c]", 
                                             50, -2.5, 2.5, 100, 0, 500);
            TH2F* h2JetMassVsPt = createHist2D("h2JetMassVsPt", "Jet Mass vs p_{T};p_{T} [GeV/c];m [GeV/c^{2}]", 
                                              20, 40, 240, 50, 0, 50);
            
            // Additional jet substructure 2D histograms
            TH2F* h2GirthVsPt = createHist2D("h2GirthVsPt", "Jet Girth vs p_{T};p_{T} [GeV/c];Girth", 
                                           10, 40, 240, 50, 0, 0.5);
            TH2F* h2ThrustVsPt = createHist2D("h2ThrustVsPt", "Jet Thrust vs p_{T};p_{T} [GeV/c];Thrust", 
                                            10, 40, 240, 50, 0, 1.0);
            TH2F* h2PtDVsPt = createHist2D("h2PtDVsPt", "Jet p_{T}D vs p_{T};p_{T} [GeV/c];p_{T}D", 
                                         10, 40, 240, 50, 0, 1.0);
            
            // Ref jet 2D histograms
            TH2F* h2RefJetPtVsEta = createHist2D("h2RefJetPtVsEta", "Ref Jet p_{T} vs #eta;#eta;p_{T} [GeV/c]", 
                                               50, -2.5, 2.5, 100, 0, 500);
            TH2F* h2RefJetMassVsPt = createHist2D("h2RefJetMassVsPt", "Ref Jet Mass vs p_{T};p_{T} [GeV/c];m [GeV/c^{2}]", 
                                                20, 40, 240, 50, 0, 50);
            TH2F* h2RefGirthVsPt = createHist2D("h2RefGirthVsPt", "Ref Jet Girth vs p_{T};p_{T} [GeV/c];Girth", 
                                              10, 40, 240, 50, 0, 0.5);
            TH2F* h2RefThrustVsPt = createHist2D("h2RefThrustVsPt", "Ref Jet Thrust vs p_{T};p_{T} [GeV/c];Thrust", 
                                               10, 40, 240, 50, 0, 1.0);
            TH2F* h2RefPtDVsPt = createHist2D("h2RefPtDVsPt", "Ref Jet p_{T}D vs p_{T};p_{T} [GeV/c];p_{T}D", 
                                            10, 40, 240, 50, 0, 1.0);
            
            // Reco vs Ref correlation histograms
            TH2F* h2JetVsRefPt = createHist2D("h2JetVsRefPt", "Reco vs Ref Jet p_{T};Ref p_{T} [GeV/c];Reco p_{T} [GeV/c]", 
                                            100, 0, 500, 100, 0, 500);
            TH2F* h2JetVsRefMass = createHist2D("h2JetVsRefMass", "Reco vs Ref Jet Mass;Ref m [GeV/c^{2}];Reco m [GeV/c^{2}]", 
                                              50, 0, 50, 50, 0, 50);
            TH2F* h2JetVsRefGirth = createHist2D("h2JetVsRefGirth", "Reco vs Ref Jet Girth;Ref Girth;Reco Girth", 
                                               50, 0, 0.5, 50, 0, 0.5);
            TH2F* h2JetVsRefThrust = createHist2D("h2JetVsRefThrust", "Reco vs Ref Jet Thrust;Ref Thrust;Reco Thrust", 
                                                50, 0, 1.0, 50, 0, 1.0);
            TH2F* h2JetVsRefPtD = createHist2D("h2JetVsRefPtD", "Reco vs Ref Jet p_{T}D;Ref p_{T}D;Reco p_{T}D", 
                                             50, 0, 1.0, 50, 0, 1.0);
            
            // Profile histograms for jet substructure evolution with pT
            TProfile* pGirthVsPt = createHistProfile("pGirthVsPt", "Jet Girth vs p_{T};p_{T} [GeV/c];<Girth>", 
                                                   10, 40, 240, 0, 0.5);
            TProfile* pThrustVsPt = createHistProfile("pThrustVsPt", "Jet Thrust vs p_{T};p_{T} [GeV/c];<Thrust>", 
                                                    10, 40, 240, 0, 1.0);
            TProfile* pPtDVsPt = createHistProfile("pPtDVsPt", "Jet p_{T}D vs p_{T};p_{T} [GeV/c];<p_{T}D>", 
                                                 10, 40, 240, 0, 1.0);
            
            // Ref jet profile histograms
            TProfile* pRefGirthVsPt = createHistProfile("pRefGirthVsPt", "Ref Jet Girth vs p_{T};p_{T} [GeV/c];<Girth>", 
                                                      10, 40, 240, 0, 0.5);
            TProfile* pRefThrustVsPt = createHistProfile("pRefThrustVsPt", "Ref Jet Thrust vs p_{T};p_{T} [GeV/c];<Thrust>", 
                                                       10, 40, 240, 0, 1.0);
            TProfile* pRefPtDVsPt = createHistProfile("pRefPtDVsPt", "Ref Jet p_{T}D vs p_{T};p_{T} [GeV/c];<p_{T}D>", 
                                                    10, 40, 240, 0, 1.0);
            
            // MC-specific histograms (only create if this is MC data)
            TH1F* hMCPhotonPt = nullptr;
            TH1F* hMCPhotonEta = nullptr;
            TH1F* hMCPhotonPhi = nullptr;
            TH1F* hMCPhotonPID = nullptr;
            TH1F* hMCPhotonMomPID = nullptr;
            TH1F* hPhotonGenMatch = nullptr;
            TH2F* h2PhotonGenVsReco = nullptr;
            
            if (isMC) {
                hMCPhotonPt = createHist1D("hMCPhotonPt", "MC Photon p_{T};p_{T} [GeV/c];Entries", 100, 0, 400);
                hMCPhotonEta = createHist1D("hMCPhotonEta", "MC Photon #eta;#eta;Entries", 50, -2.5, 2.5);
                hMCPhotonPhi = createHist1D("hMCPhotonPhi", "MC Photon #phi;#phi [rad];Entries", 50, -3.14159, 3.14159);
                hMCPhotonPID = createHist1D("hMCPhotonPID", "MC Photon PID;PID;Entries", 50, -25, 25);
                hMCPhotonMomPID = createHist1D("hMCPhotonMomPID", "MC Photon Mother PID;Mother PID;Entries", 50, -25, 25);
                hPhotonGenMatch = createHist1D("hPhotonGenMatch", "Photon Gen Match Index;Match Index;Entries", 50, -5, 45);
                h2PhotonGenVsReco = createHist2D("h2PhotonGenVsReco", "Gen vs Reco Photon p_{T};Reco p_{T} [GeV/c];Gen p_{T} [GeV/c]", 
                                               50, 0, 400, 50, 0, 400);
                
                log(LOG_DEBUG, "Created MC-specific histograms in " + collection + "/" + centName);
            }

            
            // Create any additional histograms defined in the config but not explicitly included above
            for (const auto& histConfig : plotConfig.histogramConfigs) {
                // Skip if we've already created this histogram
                std::string histName = "h" + histConfig.first;
                if (centDir->FindObject(histName.c_str()) != nullptr) continue;
                
                // Create the histogram based on its type
                if (histConfig.second.type == "TH1F") {
                    createHist1D(histName, histConfig.second.title, 
                               histConfig.second.nBinsX, histConfig.second.xMin, histConfig.second.xMax);
                } else if (histConfig.second.type == "TH2F") {
                    createHist2D(histName, histConfig.second.title, 
                               histConfig.second.nBinsX, histConfig.second.xMin, histConfig.second.xMax,
                               histConfig.second.nBinsY, histConfig.second.yMin, histConfig.second.yMax);
                } else if (histConfig.second.type == "TProfile") {
                    createHistProfile(histName, histConfig.second.title, 
                                    histConfig.second.nBinsX, histConfig.second.xMin, histConfig.second.xMax,
                                    histConfig.second.yMin, histConfig.second.yMax);
                }
            }
            
            // Don't write histograms individually here - they will be written with the directory
            // Write directory metadata
            centDir->Write();
            outFile->cd();
        }
    }
    
    // Return to the main directory
    outFile->cd();
    
    log(LOG_DEBUG, "Histogram creation complete with " + 
        std::to_string(plotConfig.histogramConfigs.size()) + " histogram configurations.");
}

/**
 * Setup output tree branches
 */
void setupOutputTree(TTree* outTree, const std::vector<std::string>& jetCollections) {
    if (!outTree) return;
    
    // Suppress unused parameter warning
    // (void)jetCollections;
    
    // Basic tree structure is set up in processEvents
}

/**
 * Create output directories using ROOT's TSystem
 */
bool createOutputDirectories(const std::string& outputDir) {
    if (outputDir.empty()) {
        std::cerr << "Error: Empty output directory path" << std::endl;
        return false;
    }
    
    // Check if directory exists, create if it doesn't
    if (gSystem->AccessPathName(outputDir.c_str())) {
        if (gSystem->mkdir(outputDir.c_str(), kTRUE) != 0) {
            std::cerr << "Error: Failed to create output directory: " << outputDir << std::endl;
            return false;
        }
        log(LOG_INFO, "Created output directory: " + outputDir);
    } else {
        log(LOG_INFO, "Output directory exists: " + outputDir);
    }
    
    return true;
}

/**
 * Print configuration summary
 */
void printConfig(TEnv* config) {
    if (!config) {
        std::cerr << "Error: Configuration is null" << std::endl;
        return;
    }
    
    // Get DataType and compute isMC status
    std::string dataType = config->GetValue("DataType", "Data");
    bool isMC = (dataType == "MC" || dataType == "mc");
    
    log(LOG_INFO, "=== Configuration Summary ===");
    log(LOG_INFO, "System: " + std::string(config->GetValue("System", "Unknown")));
    log(LOG_INFO, "DataType: " + dataType);
    log(LOG_INFO, "IsMC: " + std::to_string(isMC) + " (computed from DataType)");
    log(LOG_INFO, "InputDir: " + std::string(config->GetValue("InputDir", "")));
    log(LOG_INFO, "OutputDir: " + std::string(config->GetValue("OutputDir", "")));
    log(LOG_INFO, "OutputPrefix: " + std::string(config->GetValue("OutputPrefix", "")));
    log(LOG_INFO, "AnalysisCases: " + std::string(config->GetValue("AnalysisCases", "")));
    log(LOG_INFO, "PhotonEtMin: " + std::to_string(config->GetValue("PhotonEtMin", 60.0)));
    log(LOG_INFO, "JetPtMin: " + std::to_string(config->GetValue("JetPtMin", 40.0)));
    log(LOG_INFO, "VzCut: " + std::to_string(config->GetValue("VzCut", 15.0)));
    log(LOG_INFO, "===========================");
}

/**
 * Parse comma-separated float values from config
 */
std::vector<float> getFloatVector(TEnv* config, const std::string& param) {
    std::vector<float> result;
    std::string valueStr = config->GetValue(param.c_str(), "");
    
    if (valueStr.empty()) {
        // Return default centrality bins if not specified
        return {0, 10, 30, 50, 70, 100, 200};
    }
    
    // Handle both space and comma separated values
    std::stringstream ss(valueStr);
    std::string item;
    
    // First try comma-separated parsing
    if (valueStr.find(',') != std::string::npos) {
        while (std::getline(ss, item, ',')) {
            // Trim whitespace
            item.erase(0, item.find_first_not_of(" \t\n\r\f\v"));
            item.erase(item.find_last_not_of(" \t\n\r\f\v") + 1);
            
            if (!item.empty()) {
                try {
                    float value = std::stof(item);
                    result.push_back(value);
                } catch (const std::exception& e) {
                    std::cerr << "Warning: Failed to parse float value '" << item 
                             << "' in parameter " << param << std::endl;
                }
            }
        }
    } else {
        // Space-separated parsing
        while (ss >> item) {
            try {
                float value = std::stof(item);
                result.push_back(value);
            } catch (const std::exception& e) {
                std::cerr << "Warning: Failed to parse float value '" << item 
                         << "' in parameter " << param << std::endl;
            }
        }
    }
    
    return result;
}

/**
 * Parse comma-separated string values from config
 */
std::vector<std::string> getStringVector(TEnv* config, const std::string& param) {
    std::vector<std::string> result;
    std::string valueStr = config->GetValue(param.c_str(), "");
    
    if (valueStr.empty()) {
        return result;
    }
    
    std::stringstream ss(valueStr);
    std::string item;
    while (std::getline(ss, item, ',')) {
        // Trim whitespace
        item.erase(0, item.find_first_not_of(" \t\n\r\f\v"));
        item.erase(item.find_last_not_of(" \t\n\r\f\v") + 1);
        
        if (!item.empty()) {
            result.push_back(item);
        }
    }
    
    return result;
}

/**
 * Calculate delta phi between two angles, returned in [0, pi]
 */
float getDeltaPhi(float phi1, float phi2) {
    float dPhi = std::abs(phi1 - phi2);
    while (dPhi > M_PI) dPhi = 2 * M_PI - dPhi;
    return dPhi;
}

/**
 * Calculate xj = jetPt / photonPt
 */
float getXj(float jetPt, float photonPt) {
    if (photonPt <= 0) return 0;
    return jetPt / photonPt;
}

/**
 * Print usage information
 */
void printUsage() {
    std::cout << "Usage: gammaJetAnalyzer [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --config, -c FILE          Analysis config file path" << std::endl;
    std::cout << "  --plot-config, -p FILE     Plotting config file path" << std::endl;
    std::cout << "  --test, -t [N]             Run in test mode with N events (default: 1000)" << std::endl;
    std::cout << "  --production               Run in production mode (all events)" << std::endl;
    std::cout << "  --help, -h                 Print this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  ./gammaJetAnalyzer -c analysis.config -p plotting.config -t 10000" << std::endl;
    std::cout << "  ./gammaJetAnalyzer --config analysis.config --plot-config plotting.config --test 5000" << std::endl;
    std::cout << "  ./gammaJetAnalyzer -c analysis.config -p plotting.config --production" << std::endl;
}
