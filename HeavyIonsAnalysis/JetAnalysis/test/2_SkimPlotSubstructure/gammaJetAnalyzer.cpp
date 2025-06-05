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
#include <map>

// Persistent histogram pointer maps
std::map<std::string, TH1*> hist1DMap;
std::map<std::string, TH2*> hist2DMap;
std::map<std::string, TProfile*> profileMap;

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
            } else if (cutIndex == (size_t)currentSequentialPassed && passed) {
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
void processEvents(TChain* chain, TEnv* config, JetCollectionManager& jetManager, TFile* outFile, const PlottingConfiguration& plotConfig, Long64_t maxEvents = -1);
void createHistograms(TFile* outFile, const std::vector<std::string>& jetCollections, 
                     const std::vector<float>& centralityBins, const PlottingConfiguration& plotConfig);



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
    std::string files = ""; // For explicit file specification (batch mode)
    bool testMode = false;
    int maxEvents = 1000;
    std::string batchid = "";
    
    // Parse command line arguments using getopt_long
    static struct option long_options[] = {
        {"config",       required_argument, 0, 'c'},
        {"plot-config",  required_argument, 0, 'p'},
        {"files",        required_argument, 0, 'f'},
        {"batchid",      required_argument, 0, 'b'},
        {"test",         optional_argument, 0, 't'},
        {"help",         no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };
    
    int option_index = 0;
    int c;
    
    while ((c = getopt_long(argc, argv, "c:p:f:b:t::h", long_options, &option_index)) != -1) {
        switch (c) {
            case 'c':
                configFile = optarg;
                break;
            case 'p':
                plotConfigFile = optarg;
                break;
            case 'f':
                files = optarg;
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
            case 'b':
                batchid = optarg;
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
    log(LOG_DEBUG, "  Files: " + (files.empty() ? "none (will use InputDir)" : files));
    log(LOG_DEBUG, "  Test mode: " + std::string(testMode ? "true" : "false"));
    log(LOG_DEBUG, "  Max events: " + std::to_string(maxEvents));
    
    try {
        // Start timer
        TStopwatch timer;
        timer.Start();
        // Print information
        // log(LOG_INFO, "==================================================");
        log(LOG_INFO, "=== PhotonJet Analysis: Jet Substructure v2.0 ===");
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
        
        // Check input specification - either files or inputDir must be provided
        if (files.empty() && inputDir.empty()) {
            std::cerr << "Error: Neither explicit files nor input directory specified" << std::endl;
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
        
        // Use explicit files if provided (batch mode), otherwise use directory
        bool setupSuccess = false;
        if (!files.empty()) {
            log(LOG_INFO, "Using explicit file list (batch mode)");
            std::vector<std::string> fileList = parseFileList(files);
            setupSuccess = setupInputChain(chain, fileList, testMode);
        } else {
            log(LOG_INFO, "Using input directory: " + inputDir);
            setupSuccess = setupInputChain(chain, inputDir, testMode);
        }
        
        if (!setupSuccess) {
            std::cerr << "Error: Failed to setup input chain" << std::endl;
            delete chain;
            delete config;
            return 1;
        }
        
        // Setup output file with unique naming for batch mode
        std::string outputFile = outputDir + "/" + outputPrefix + "_output";
        
        // Add unique identifier for batch mode to prevent file conflicts
        if (!files.empty()||!batchid.empty()) {
            // Batch mode - add process ID to avoid filename conflicts
            outputFile += "_batch_" + batchid;
            log(LOG_INFO, "Batch mode detected: adding process ID to output filename");
        }
        
        outputFile += ".root";
        log(LOG_INFO, "Output file: " + outputFile);
        
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

        TEnv plotEnv(plotConfigFile.c_str());
        loadHistogramConfigsFromEnv(&plotEnv, plotConfig); // <-- Config-specific histograms

        
        // Before processing events, check for HistogramConfigFile in TEnv and load it if present
        std::string histConfigFile = config->GetValue("HistogramConfigFile", "");
        if (!histConfigFile.empty()) {
            TEnv histEnv(histConfigFile.c_str());
            loadHistogramConfigsFromEnv(&histEnv, plotConfig);
        }
        
        // Now process events as usual
        Long64_t nEvents = testMode && maxEvents > 0 ? maxEvents : -1; // -1 means all events
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
void processEvents(TChain* chain, TEnv* config, JetCollectionManager& jetManager, TFile* outFile, const PlottingConfiguration& plotConfig, Long64_t maxEvents) {
    if (!chain || !outFile) return;
    
    // Initialize logging verbosity from config
    g_verbosity = config->GetValue("Verbosity", LOG_INFO);
    log(LOG_INFO, "Starting event processing...");
    
    // Get parameters from config
    std::string dataType = config->GetValue("DataType", "Data");
    bool isMC = (dataType == "MC" || dataType == "mc");
    float vzCut = config->GetValue("VzCut", 10000.0);
    float hiHFCutMin = config->GetValue("HiHFCutMin", -1.0);
    float hiHFCutMax = config->GetValue("HiHFCutMax", 700000.0);
    float photonEtMin = config->GetValue("PhotonEtMin", 0.0);
    float photonEtaMax = config->GetValue("PhotonEtaMax", 100000.0);
    float photonHoverEMax = config->GetValue("PhotonHoverEMax", 100000.0);
    float photonSigmaIEtaIEtaMax = config->GetValue("PhotonSigmaIEtaIEtaMax", 100000.0);
    float photonIsoMax = config->GetValue("PhotonIsoMax", 10000); //! Update the defaults so it's always true if not given in the config
    float photonR9Min = config->GetValue("PhotonR9Min", -1.0);
    float jetPtMin = config->GetValue("JetPtMin", -1.0);
    float jetEtaMax = config->GetValue("JetEtaMax", 1000.0);
    float deltaPhiMin = config->GetValue("DeltaPhiMin", -1);
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
    std::vector<float> *pho_ecalClusterIsoR3 = nullptr;
    std::vector<float> *pho_hcalRechitIsoR3 = nullptr;
    std::vector<float> *pho_trackIsoR3PtCut20 = nullptr;
    std::vector<float> *pfpIso3subUEec = nullptr;
    std::vector<float> *pfcIso3subUEec = nullptr;
    std::vector<float> *pfnIso3subUEec = nullptr;
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
    chain->SetBranchAddress("ggHi_pho_ecalClusterIsoR3", &pho_ecalClusterIsoR3);
    chain->SetBranchAddress("ggHi_pho_hcalRechitIsoR3", &pho_hcalRechitIsoR3);
    chain->SetBranchAddress("ggHi_pho_trackIsoR3PtCut20", &pho_trackIsoR3PtCut20);
    chain->SetBranchAddress("ggHi_pfpIso3subUEec", &pfpIso3subUEec);
    chain->SetBranchAddress("ggHi_pfcIso3subUEec", &pfcIso3subUEec);
    chain->SetBranchAddress("ggHi_pfnIso3subUEec", &pfnIso3subUEec);
    chain->SetBranchAddress("ggHi_phoR9_2012", &phoR9);
    
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
    float selectedPhotonECALIso = 0;
    float selectedPhotonHCALIso = 0;
    float selectedPhotonTRKIso = 0;
    float selectedPhotonPFPIso = 0;
    float selectedPhotonPFCIso = 0;
    float selectedPhotonPFNIso = 0;
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
    outTree->Branch("photonECALIso", &selectedPhotonECALIso);
    outTree->Branch("photonHCALIso", &selectedPhotonHCALIso);
    outTree->Branch("photonTRKIso", &selectedPhotonTRKIso);
    outTree->Branch("photonPFPIso", &selectedPhotonPFPIso);
    outTree->Branch("photonPFCIso", &selectedPhotonPFCIso);
    outTree->Branch("photonPFNIso", &selectedPhotonPFNIso);
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
            float phoIso = pho_ecalClusterIsoR3->at(selectedPhotonIndex) + pho_hcalRechitIsoR3->at(selectedPhotonIndex) + pho_trackIsoR3PtCut20->at(selectedPhotonIndex);
            bool passIso = (phoIso <= photonIsoMax);
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
                } 
                else {
                    int genMatchedIndex = phoGenMatchedIndex->at(selectedPhotonIndex);
                    if (genMatchedIndex < 0) {
                        selectedPhotonIndex = -1;
                        cutFlowTracker.applyCut("MCPhotonMatch", false);
                    } 
                    else {
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
        selectedPhotonECALIso = pho_ecalClusterIsoR3->at(selectedPhotonIndex);
        selectedPhotonHCALIso = pho_hcalRechitIsoR3->at(selectedPhotonIndex);
        selectedPhotonTRKIso = pho_trackIsoR3PtCut20->at(selectedPhotonIndex);
        selectedPhotonPFPIso = pfpIso3subUEec->at(selectedPhotonIndex);
        selectedPhotonPFCIso = pfcIso3subUEec->at(selectedPhotonIndex);
        selectedPhotonPFNIso = pfnIso3subUEec->at(selectedPhotonIndex);
        selectedPhotonIso = pho_ecalClusterIsoR3->at(selectedPhotonIndex) + pho_hcalRechitIsoR3->at(selectedPhotonIndex) + pho_trackIsoR3PtCut20->at(selectedPhotonIndex);
        selectedPhotonR9 = phoR9->at(selectedPhotonIndex);

        // Fill histograms for this collection
        int centBin = -1;
        for (size_t i = 0; i < centralityBins.size() - 1; ++i) {
            if (hiBin >= centralityBins[i] && hiBin < centralityBins[i+1]) {
                centBin = i;
                break;
            }
        }

        if (centBin >= 0) {
            std::string centName = "cent" + std::to_string(static_cast<int>(centralityBins[centBin])) + "to" + std::to_string(static_cast<int>(centralityBins[centBin+1]));
            log(LOG_TRACE, "Filling histograms for " + centName + "/General/ with weight: " + std::to_string(eventWeight));
            // General histograms
            auto fill1D = [&](const std::string& hname, double value, double weight=1.0) {
                auto it = hist1DMap.find(centName + "/General/" + hname);
                if (it != hist1DMap.end() && it->second) it->second->Fill(value, weight);
                log(LOG_TRACE, "Filling 1D hist: " + centName + "/General/" + hname + " with value " + std::to_string(value));
            };
            auto fill2D = [&](const std::string& hname, double x, double y, double weight=1.0) {
                auto it = hist2DMap.find(centName + "/General/" + hname);
                if (it != hist2DMap.end() && it->second) it->second->Fill(x, y, weight);
            };
            // auto fillProfile = [&](const std::string& hname, double x, double y, double weight=1.0) {
            //     auto it = profileMap.find(centName + "/General/" + hname);
            //     if (it != profileMap.end() && it->second) it->second->Fill(x, y, weight);
            // };
            // Photon histograms
            fill1D("hPhotonEt", selectedPhotonEt, eventWeight);
            fill1D("hPhotonEta", selectedPhotonEta, eventWeight);
            fill1D("hPhotonHoverE", selectedPhotonHoverE, eventWeight);
            fill1D("hPhotonSigmaIEtaIEta", selectedPhotonSigmaIEtaIEta, eventWeight);
            fill1D("hPhotonIso", selectedPhotonIso, eventWeight);
            fill1D("hPhotonR9", selectedPhotonR9, eventWeight);
            fill1D("hNPhotons", nPhotons, eventWeight);
            fill1D("hEventWeight", eventWeight, 1.0);
            fill1D("hVz", vz, eventWeight);
            fill1D("hHiHF", hiHF, eventWeight);
            fill1D("hCentrality", hiBin, eventWeight);
            // --- MC photon histograms ---
            if (isMC && selectedPhotonIndex >= 0) {
                if (phoGenMatchedIndex) fill1D("hPhotonGenMatch", phoGenMatchedIndex->at(selectedPhotonIndex), eventWeight);
                if (phoGenMatchedIndex && phoGenMatchedIndex->at(selectedPhotonIndex) >= 0) {
                    int genIndex = phoGenMatchedIndex->at(selectedPhotonIndex);
                    if (mcPt && genIndex < static_cast<int>(mcPt->size())) fill1D("hMCPhotonEt", mcPt->at(genIndex), eventWeight);
                    if (mcEta && genIndex < static_cast<int>(mcEta->size())) fill1D("hMCPhotonEta", mcEta->at(genIndex), eventWeight);
                    if (mcPhi && genIndex < static_cast<int>(mcPhi->size())) fill1D("hMCPhotonPhi", mcPhi->at(genIndex), eventWeight);
                    if (mcPID && genIndex < static_cast<int>(mcPID->size())) fill1D("hMCPhotonPID", mcPID->at(genIndex), eventWeight);
                    if (mcMomPID && genIndex < static_cast<int>(mcMomPID->size())) fill1D("hMCPhotonMomPID", mcMomPID->at(genIndex), eventWeight);
                    if (mcPt && genIndex < static_cast<int>(mcPt->size())) fill2D("h2PhotonGenVsReco", selectedPhotonEt, mcPt->at(genIndex), eventWeight);
                }
            }
        }
        
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
        } 
        else {
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
                
                if (centBin >= 0) {
                    std::string centName = "cent" + std::to_string(static_cast<int>(centralityBins[centBin])) + 
                                         "to" + std::to_string(static_cast<int>(centralityBins[centBin+1]));
                    
                    log(LOG_TRACE, "Filling histograms for " + collection + "/" + centName + 
                        " with weight: " + std::to_string(eventWeight));
                    
                    // Navigate to subdirectory and fill histograms with weights
                    auto fill1Djet = [&](const std::string& hname, double value, double weight=1.0) {
                        auto it = hist1DMap.find(centName + "/" + collection + "/" + hname);
                        if (it != hist1DMap.end() && it->second) it->second->Fill(value, weight);
                    };
                    auto fill2Djet = [&](const std::string& hname, double x, double y, double weight=1.0) {
                        auto it = hist2DMap.find(centName + "/" + collection + "/" + hname);
                        if (it != hist2DMap.end() && it->second) it->second->Fill(x, y, weight);
                    };
                    auto fillProfilejet = [&](const std::string& hname, double x, double y, double weight=1.0) {
                        auto it = profileMap.find(centName + "/" + collection + "/" + hname);
                        if (it != profileMap.end() && it->second) it->second->Fill(x, y, weight);
                    };
                    outFile->cd();
                    if (outFile->cd((centName + "/" + collection).c_str())) {
                        fill1Djet("hJetPt", selectedJetPts[collection], eventWeight);
                        fill1Djet("hJetEta", selectedJetEtas[collection], eventWeight);
                        fill1Djet("hJetPhi", selectedJetDeltaPhis[collection], eventWeight);
                        fill1Djet("hDeltaPhi", selectedJetDeltaPhis[collection], eventWeight);
                        fill1Djet("hXj", selectedJetXjs[collection], eventWeight);

                        // Jet observables histograms                        
                        fill1Djet("hJetMass", selectedJetMasses[collection], eventWeight);
                        fill1Djet("hJetArea", selectedJetAreas[collection], eventWeight);
                        fill1Djet("hDynDeltaR", selectedJetDynDeltaRs[collection], eventWeight);
                        fill1Djet("hIntJetMulti", selectedJetIntJetMultis[collection], eventWeight);
                        fill1Djet("hDynSplit", selectedJetDynSplits[collection], eventWeight);
                        fill1Djet("hDynKt", selectedJetDynKts[collection], eventWeight);
                        fill1Djet("hDynZ", selectedJetDynZs[collection], eventWeight);
                        fill1Djet("hGirth", selectedJetGirths[collection], eventWeight);
                        fill1Djet("hThrust", selectedJetThrusts[collection], eventWeight);
                        fill1Djet("hLHA", selectedJetLHAs[collection], eventWeight);
                        fill1Djet("hPtD", selectedJetPtDs[collection], eventWeight);
                        fill1Djet("hJetEta", selectedJetEtas[collection], eventWeight);
                        
                        //Event-level histograms
                        fill1Djet("hNJets", jetManager.getNJets(collection), eventWeight);
                        fill2Djet("h2JetPtVsEta", selectedJetEtas[collection], selectedJetPts[collection], eventWeight);
                        fill2Djet("h2JetMassVsPt", selectedJetPts[collection], selectedJetMasses[collection], eventWeight);
                        
                        // Ref jet (MC-matched) histograms
                        if (isMC && selectedRefJetPts[collection] > -900) {
                            fill1Djet("hRefPt", selectedRefJetPts[collection], eventWeight);
                            fill1Djet("hRefEta", selectedRefJetEtas[collection], eventWeight);
                            fill1Djet("hRefPhi", selectedRefJetPhis[collection], eventWeight);
                            fill1Djet("hRefMass", selectedRefJetMasses[collection], eventWeight);
                            fill1Djet("hRefArea", selectedRefJetAreas[collection], eventWeight);
                            fill1Djet("hRefDynDeltaR", selectedRefJetDynDeltaRs[collection], eventWeight);
                            fill1Djet("hRefIntJetMulti", selectedRefJetIntJetMultis[collection], eventWeight);
                            fill1Djet("hRefDynSplit", selectedRefJetDynSplits[collection], eventWeight);
                            fill1Djet("hRefDynKt", selectedRefJetDynKts[collection], eventWeight);
                            fill1Djet("hRefDynZ", selectedRefJetDynZs[collection], eventWeight);
                            fill1Djet("hRefGirth", selectedRefJetGirths[collection], eventWeight);
                            fill1Djet("hRefThrust", selectedRefJetThrusts[collection], eventWeight);
                            fill1Djet("hRefLHA", selectedRefJetLHAs[collection], eventWeight);
                            fill1Djet("hRefPtD", selectedRefJetPtDs[collection], eventWeight);

                            // 2D and profile ref jet histograms
                            fill2Djet("h2GirthVsPt", selectedJetPts[collection], selectedJetGirths[collection], eventWeight);
                            fill2Djet("h2ThrustVsPt", selectedJetPts[collection], selectedJetThrusts[collection], eventWeight);
                            fill2Djet("h2PtDVsPt", selectedJetEtas[collection], selectedJetPtDs[collection], eventWeight);
                            fillProfilejet("pGirthVsPt", selectedJetPts[collection], selectedJetGirths[collection], eventWeight);
                            fillProfilejet("pThrustVsPt", selectedJetPts[collection], selectedJetThrusts[collection], eventWeight);
                            fillProfilejet("pPtDVsPt", selectedJetPts[collection], selectedJetPtDs[collection], eventWeight);
                            fill2Djet("h2RefJetPtVsEta", selectedRefJetEtas[collection], selectedRefJetPts[collection], eventWeight);
                            fill2Djet("h2RefJetMassVsPt", selectedRefJetPts[collection], selectedRefJetMasses[collection], eventWeight);
                            fill2Djet("h2RefGirthVsPt", selectedRefJetPts[collection], selectedRefJetGirths[collection], eventWeight);
                            fill2Djet("h2RefThrustVsPt", selectedRefJetPts[collection], selectedRefJetThrusts[collection], eventWeight);
                            fill2Djet("h2RefPtDVsPt", selectedRefJetPts[collection], selectedRefJetPtDs[collection], eventWeight);
                            // Reco vs Ref correlation
                            fill2Djet("h2JetVsRefPt", selectedRefJetPts[collection], selectedJetPts[collection], eventWeight);
                            fill2Djet("h2JetVsRefMass", selectedRefJetMasses[collection], selectedJetMasses[collection], eventWeight);
                            fill2Djet("h2JetVsRefGirth", selectedRefJetGirths[collection], selectedJetGirths[collection], eventWeight);
                            fill2Djet("h2JetVsRefThrust", selectedRefJetThrusts[collection], selectedJetThrusts[collection], eventWeight);
                            fill2Djet("h2JetVsRefPtD", selectedRefJetPtDs[collection], selectedJetPtDs[collection], eventWeight);
                            // Ref jet profiles
                            fillProfilejet("pRefGirthVsPt", selectedRefJetPts[collection], selectedRefJetGirths[collection], eventWeight);
                            fillProfilejet("pRefThrustVsPt", selectedRefJetPts[collection], selectedRefJetThrusts[collection], eventWeight);
                            fillProfilejet("pRefPtDVsPt", selectedRefJetPts[collection], selectedRefJetPtDs[collection], eventWeight);
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
    // for (const auto& collection : jetCollections) {
    //     TDirectory* collectionDir = outFile->GetDirectory(collection.c_str());
    //     if (collectionDir) {
    //         log(LOG_DEBUG, "Writing histograms for collection: " + collection);
    //         collectionDir->Write("", TObject::kOverwrite);
    //     }
    // }
    
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

    // New structure: centralityDir/collectionDir
    for (size_t i = 0; i < centralityBins.size() - 1; ++i) {
        std::string centName = "cent" + std::to_string(static_cast<int>(centralityBins[i])) + "to" + std::to_string(static_cast<int>(centralityBins[i+1]));
        TDirectory* centDir = outFile->mkdir(centName.c_str());
        centDir->cd();
        // === FUTURE EXPANSION: To add eta binning, insert etaDir creation here ===
        // Example:
        // std::vector<std::pair<float, float>> etaBins = { {-2.5, -1.0}, {-1.0, 0.0}, {0.0, 1.0}, {1.0, 2.5} };
        // for (const auto& etaBin : etaBins) {
        //     std::string etaName = Form("eta%.1fto%.1f", etaBin.first, etaBin.second);
        //     TDirectory* etaDir = centDir->mkdir(etaName.c_str());
        //     if (!etaDir) continue;
        //     etaDir->cd();
        //     // Then create collectionDir inside etaDir as below
        //     for (const auto& collection : jetCollections) {
        //         TDirectory* collectionDir = etaDir->mkdir(collection.c_str());
        //         ...
        //     }
        // }
        // For now, we proceed without etaDir:
        
        // General histograms
        TDirectory* generalDir = centDir->mkdir("General");
        generalDir->cd();
        for (const auto& kv : plotConfig.histogramConfigs) {
            const std::string& hname = kv.first;
            const HistogramConfig& hcfg = kv.second;
            log(LOG_TRACE, "Creating hist: " + centName + "/General/" + hname);
            // Only create general histograms in General dir (not per-collection)
            if (hname.find("Jet") == std::string::npos && hname.find("Ref") == std::string::npos && hname.find("Dyn") == std::string::npos && hname.find("Girth") == std::string::npos && hname.find("Thrust") == std::string::npos && hname.find("LHA") == std::string::npos && hname.find("PtD") == std::string::npos && hname.find("Xj") == std::string::npos && hname.find("DeltaPhi") == std::string::npos) {
                if (hcfg.type == "TH1F") {
                    TH1F* h = createHistogram1D(hcfg,"h"+hname);
                    h->SetDirectory(generalDir);
                    hist1DMap[centName + "/General/h" + hname] = h;
                } else if (hcfg.type == "TH2F") {
                    TH2F* h = createHistogram2D(hcfg,"h2"+hname);
                    h->SetDirectory(generalDir);
                    hist2DMap[centName + "/General/h2" + hname] = h;
                } else if (hcfg.type == "TProfile") {
                    TProfile* p = createProfile(hcfg,"p"+hname);
                    p->SetDirectory(generalDir);
                    profileMap[centName + "/General/p" + hname] = p;
                }
            }
        }
        // Per-collection histograms
        for (const auto& collection : jetCollections) {
            TDirectory* collDir = centDir->mkdir(collection.c_str());
            collDir->cd();
            for (const auto& kv : plotConfig.histogramConfigs) {
                const std::string& hname = kv.first;
                const HistogramConfig& hcfg = kv.second;
                // Only create jet/ref-jet/substructure histograms in collection dir
                if (hname.find("Jet") != std::string::npos || hname.find("Ref") != std::string::npos || hname.find("Dyn") != std::string::npos || hname.find("Girth") != std::string::npos || hname.find("Thrust") != std::string::npos || hname.find("LHA") != std::string::npos || hname.find("PtD") != std::string::npos || hname.find("Xj") != std::string::npos || hname.find("DeltaPhi") != std::string::npos) {
                    if (hcfg.type == "TH1F") {
                        TH1F* h = createHistogram1D(hcfg,"h"+hname);
                        h->SetDirectory(collDir);
                        hist1DMap[centName + "/" + collection + "/h" + hname] = h;
                    } else if (hcfg.type == "TH2F") {
                        TH2F* h = createHistogram2D(hcfg,"h2"+hname);
                        h->SetDirectory(collDir);
                        hist2DMap[centName + "/" + collection + "/h2" + hname] = h;
                    } else if (hcfg.type == "TProfile") {
                        TProfile* p = createProfile(hcfg,"p"+hname);
                        p->SetDirectory(collDir);
                        profileMap[centName + "/" + collection + "/p" + hname] = p;
                    }
                }
            }
        }
        outFile->cd();
    }

    // --- Histogram validation: config vs created ---
    
    // Helper to extract base histogram name (removes directory and h/h2/p prefix)
    auto extractBaseName = [](const std::string& key) -> std::string {
        // Example key: "cent0to60/General/hPhotonEt" or "cent0to60/AK4PF/hJetPt"
        size_t lastSlash = key.find_last_of('/');
        std::string name = (lastSlash != std::string::npos) ? key.substr(lastSlash + 1) : key;
        // Remove h/h2/p prefix
        if (name.rfind("h2", 0) == 0) return name.substr(2);
        if (name.rfind("h", 0) == 0) return name.substr(1);
        if (name.rfind("p", 0) == 0) return name.substr(1);
        return name;
    };
    
    // 1. Check for histograms defined in config but not created
    for (const auto& kv : plotConfig.histogramConfigs) {
        const std::string& histName = kv.second.name;
        bool found = false;
        // Check each map separately to avoid type deduction issues
        for (const auto& mkv : hist1DMap) {
            if (extractBaseName(mkv.first) == histName) {
                found = true;
                break;
            }
        }
        if (!found) {
            for (const auto& mkv : hist2DMap) {
                if (extractBaseName(mkv.first) == histName) {
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            for (const auto& mkv : profileMap) {
                if (extractBaseName(mkv.first) == histName) {
                    found = true;
                    break;
                }
            }
        }
        if (!found) {
            log(LOG_ERROR, "Histogram defined in config but not created: " + histName);
        }
    }
    
    // 2. Check for histograms created in code but not defined in config
    auto checkMap = [&](const auto& m) {
        for (const auto& mkv : m) {
            std::string baseName = extractBaseName(mkv.first);
            if (plotConfig.histogramConfigs.find(baseName) == plotConfig.histogramConfigs.end()) {
                log(LOG_ERROR, "Histogram created in code but not defined in config: " + mkv.first + " (base: " + baseName + ")");
            }
        }
    };
    checkMap(hist1DMap);
    checkMap(hist2DMap);
    checkMap(profileMap);
    // --- End histogram validation ---

    log(LOG_DEBUG, "Histogram creation complete with persistent pointers.");
}
