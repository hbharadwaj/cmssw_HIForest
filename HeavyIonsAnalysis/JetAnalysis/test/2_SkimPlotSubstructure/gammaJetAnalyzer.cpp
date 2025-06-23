/**
 * gammaJetAnalyzer.cpp
 *
 * Main C++ executable for photon-tagged jet substructure analysis.
 * This implements a hybrid approach combining traditional C-style event loop
 * with efficient branch handling for dynamic jet collections.
 *
 * SYSTEM COMPATIBILITY:
 * - Supports both PbPb (centrality-dependent) and pp (inclusive) collision systems
 * - Automatically detects system type from configuration "System" parameter
 * - Uses unified code paths with conditional logic for system-specific features
 *
 * CENTRALITY HANDLING:
 * - PbPb systems: Uses configured centrality bins for histogram organization and cut flow
 * - pp systems: Uses single "inclusive" bin, centrality variables treated as placeholders
 *
 * Usage:
 *   ./gammaJetAnalyzer -c path/to/config.config [-t maxEvents] [-h]
 *
 * Example:
 *   ./gammaJetAnalyzer -c ../configs/JetSub_2023_PbPb_Data.config -t 1000
 *   ./gammaJetAnalyzer -c ../configs/JetSub_2024_PP_Data.config -t 1000
 */

// Include headers
#include "include/JetCollectionManager.h"
#include "include/helpers.h"
#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TH1D.h>
#include <TH2D.h>
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

// Forward declarations
void processEvents(TChain* chain, TEnv* config, JetCollectionManager& jetManager, TFile* outFile, const PlottingConfiguration& plotConfig, Long64_t maxEvents = -1);
void createHistograms(TFile* outFile, const std::vector<std::string>& jetCollections, 
                     const std::vector<float>& centralityBins, const PlottingConfiguration& plotConfig, bool useCentrality);

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
        log(LOG_INFO, "==================================================");
        log(LOG_INFO, "=== PhotonJet Analysis: Jet Substructure v2.0 ===");
        log(LOG_INFO, "==================================================");
        log(LOG_INFO, "SUPPORTS: PbPb and pp collision systems");
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
 * 
 * SYSTEM COMPATIBILITY:
 * This function is designed to work with both PbPb and pp collision systems:
 * - System type detection: Automatically detects from config "System" parameter
 * - Centrality handling: Uses centrality bins for PbPb, inclusive binning for pp
 * - Cut flow tracking: Centrality-dependent for PbPb, system-agnostic for pp
 * - Histogram organization: Centrality-binned for PbPb, single inclusive bin for pp
 * 
 * LOGIC FLOW:
 * 1. System type detection and setup
 * 2. Event-level cuts and histogram filling  
 * 3. Centrality determination (PbPb) or inclusive binning (pp)
 * 4. Photon selection with MC matching (if applicable)
 * 5. Jet selection per collection
 * 6. Output tree filling
 * 
 * @param chain Input TChain with event data
 * @param config Configuration object with analysis parameters
 * @param jetManager Manager for multiple jet collections
 * @param outFile Output ROOT file for histograms and trees
 * @param plotConfig Plotting configuration for histogram creation
 * @param maxEvents Maximum events to process (-1 for all)
*/
void processEvents(TChain* chain, TEnv* config, JetCollectionManager& jetManager, TFile* outFile, const PlottingConfiguration& plotConfig, Long64_t maxEvents) {
    if (!chain || !outFile) return;
    
    // Initialize logging verbosity from config
    g_verbosity = config->GetValue("Verbosity", LOG_INFO);
    log(LOG_INFO, "Starting event processing...");
    
    // Get parameters from config
    std::string dataType = config->GetValue("DataType", "Data");
    bool isMC = (dataType == "MC" || dataType == "mc");
    float vzCut = config->GetValue("VzCut", -999.0);
    float photonEtMin = config->GetValue("PhotonEtMin", -999.0);
    float photonEtaMax = config->GetValue("PhotonEtaMax", 100000.0);
    float photonHoverEMax = config->GetValue("PhotonHoverEMax", 100000.0);
    float photonSigmaIEtaIEtaMax = config->GetValue("PhotonSigmaIEtaIEtaMax", 100000.0);
    float photonIsoMax = config->GetValue("PhotonIsoMax", 10000); //! Update the defaults so it's always true if not given in the config
    float photonR9Min = config->GetValue("PhotonR9Min", -999.0);
    float jetPtMin = config->GetValue("JetPtMin", -999.0);
    float jetEtaMax = config->GetValue("JetEtaMax", 100000.0);
    float deltaPhiMin = config->GetValue("DeltaPhiMin", -999.0);
    std::vector<float> centralityBins = getFloatVector(config, "CentralityBins");
    std::vector<std::string> jetCollections = jetManager.getCollections();
    
    /**
     * System type detection and centrality usage determination
     * 
     * This logic automatically determines whether centrality should be used
     * based on both the system type and available centrality bins:
     * 
     * 1. PbPb detection: Any system string containing "PbPb"
     * 2. pp detection: Any system string containing "PP" or "pp" 
     * 3. Fallback: If string is empty or unknown, default to PbPb with warning
     * 
     * Centrality is only used if:
     * - The system is detected as PbPb AND
     * - Centrality bins are properly configured (non-empty)
     * 
     * This approach ensures compatibility with:
     * - "2023_PbPb", "2024_PbPb", "2025_PbPb", etc.
     * - "2024_PP", "2025_pp", etc.
     */
    std::string systemType = config->GetValue("System", "");
    bool isPbPb = false;
    bool useCentrality = false;
    
    // Detect system type - extensible for future systems
    if (systemType.find("PbPb") != std::string::npos) {
        isPbPb = true;
        useCentrality = !centralityBins.empty();
        log(LOG_INFO, "Detected PbPb collision system: " + systemType);
    } else if (systemType.find("PP") != std::string::npos || systemType.find("pp") != std::string::npos) {
        isPbPb = false;
        useCentrality = false; // Never use centrality for pp systems
        log(LOG_INFO, "Detected pp collision system: " + systemType);
    } else {
        // Default behavior for backwards compatibility or unrecognized systems
        isPbPb = true;
        useCentrality = !centralityBins.empty();
        if (systemType.empty()) {
            log(LOG_WARNING, "No System specified in config, defaulting to PbPb behavior");
        } else {
            log(LOG_WARNING, "Unknown system type '" + systemType + "', defaulting to PbPb behavior");
        }
    }
    
    log(LOG_DEBUG, "Configuration loaded:");
    log(LOG_DEBUG, "  System: " + systemType + " (" + (isPbPb ? "PbPb" : "pp") + ")");
    log(LOG_DEBUG, "  Centrality usage: " + std::string(useCentrality ? "ENABLED" : "DISABLED"));
    log(LOG_DEBUG, "  DataType: " + dataType);
    log(LOG_DEBUG, "  isMC: " + std::to_string(isMC));
    log(LOG_DEBUG, "  vzCut: " + std::to_string(vzCut));
    log(LOG_DEBUG, "  photonEtMin: " + std::to_string(photonEtMin));
    log(LOG_DEBUG, "  jetPtMin: " + std::to_string(jetPtMin));
    
    // Debug: Print centrality bins information
    if (useCentrality) {
        log(LOG_DEBUG, "centralityBins loaded from config:");
        log(LOG_DEBUG, "  centralityBins.size() = " + std::to_string(centralityBins.size()));
        std::string binContents = "  centralityBins contents: ";
        for (float bin : centralityBins) {
            binContents += std::to_string(bin) + " ";
        }
        log(LOG_DEBUG, binContents);
    } else {
        if(isPbPb){
            log(LOG_DEBUG, "Centrality bins disabled for PbPb system");
        }
        else{
            log(LOG_DEBUG, "Centrality bins disabled for pp system");
        }
    }
    
    // Setup output tree
    TTree* outTree = new TTree("gammaJetTree", "Gamma-Jet Analysis");
    
    // Create histograms with centrality usage flag
    createHistograms(outFile, jetCollections, centralityBins, plotConfig, useCentrality);
    
    // Variables for branch addresses
    int hiBin = -999;
    float vz = -999;
    float hiHF = -999;

    float rho = -999;
    
    // Photon variables - using vectors as original implementation for compatibility
    int nPhotons = -999;
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
    std::vector<float> *mcCalIsoDR04 = nullptr;
    
    // Event weight for MC
    float weight = 1.0;
    
    // Setup branch addresses for event variables
    chain->SetBranchAddress("hiBin", &hiBin);
    chain->SetBranchAddress("vz", &vz);
    chain->SetBranchAddress("hiHF", &hiHF);
    
    // Setup branch addresses for photon variables with correct ggHi_ prefix
    chain->SetBranchAddress("ggHi_rho", &rho);
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
        TBranch* mcCalIsoDR04Branch = chain->GetBranch("ggHi_mcCalIsoDR04");
        
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
        if (mcCalIsoDR04Branch) {
            chain->SetBranchAddress("ggHi_mcCalIsoDR04", &mcCalIsoDR04);
            log(LOG_DEBUG, "MC branch connected: ggHi_mcCalIsoDR04");
        } else {
            log(LOG_INFO, "MC branch not found: ggHi_mcCalIsoDR04");
        }

        // Try to set up weight branch - check if it exists
        TBranch* weightBranch = chain->GetBranch("weight");
        if (weightBranch) {
            chain->SetBranchAddress("weight", &weight);
            log(LOG_INFO, "Weight branch found and connected for MC events");
        } else {
            log(LOG_INFO, "No weight branch found, using weight = 1.0 for all events");
        }
    }
    
    // Output variables
    float selectedEventWeight = -999;
    int selectedHiBin = -999;
    float selectedVz = -999;
    float selectedHiHF = -999;
    float selectedRho = -999;
    int selectedPhotonIndex = -999;
    float selectedPhotonEt = -999;
    float selectedPhotonEta = -999;
    float selectedPhotonPhi = -999;
    float selectedPhotonHoverE = -999;
    float selectedPhotonSigmaIEtaIEta = -999;
    float selectedPhotonECALIso = -999;
    float selectedPhotonHCALIso = -999;
    float selectedPhotonTRKIso = -999;
    float selectedPhotonPFPIso = -999;
    float selectedPhotonPFCIso = -999;
    float selectedPhotonPFNIso = -999;
    float selectedPhotonIso = -999;
    float selectedPhotonR9 = -999;

    float selectedMCPhotonEt = -999;
    float selectedMCPhotonEta = -999;
    float selectedMCPhotonPhi = -999;
    float selectedMCPhotonPID = -999;
    float selectedMCPhotonIso = -999;
    
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
    std::map<std::string, float> selectedRefJetXjs;
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
    outTree->Branch("eventWeight", &selectedEventWeight);
    outTree->Branch("hiBin", &selectedHiBin);
    outTree->Branch("vz", &selectedVz);
    outTree->Branch("hiHF", &selectedHiHF);
    outTree->Branch("rho", &selectedRho);
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

    if(isMC){
        outTree->Branch("MCphotonEt", &selectedMCPhotonEt);
        outTree->Branch("MCphotonEta", &selectedMCPhotonEta);
        outTree->Branch("MCphotonPhi", &selectedMCPhotonPhi);
        outTree->Branch("MCPhotonPID", &selectedMCPhotonPID);
        outTree->Branch("MCphotonIso", &selectedMCPhotonIso);
    }
    
    // Initialize output variables for each jet collection
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
        if(isMC){
            outTree->Branch(("refxj_" + collection).c_str(), &selectedRefJetXjs[collection]);
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

    // Initialize multi-dimensional cut flow tracker with centrality usage flag
    MultiDimCutFlowTracker cutFlowTracker(config, centralityBins, jetCollections, useCentrality);

    for (Long64_t iEvent = 0; iEvent < nEvents; ++iEvent) {
        if (iEvent % 1000 == 0) {
            log(LOG_INFO, "Processing event " + std::to_string(iEvent) + "/" + 
            std::to_string(nEvents) + " (" + 
            std::to_string(static_cast<double>(iEvent) / nEvents * 100) + "%)");
        }
        chain->GetEntry(iEvent);
        nProcessed++;
        float eventWeight = 1.0;
        if (isMC && isPbPb){
            eventWeight = weight * findNcoll(hiBin);
        }
        else{
            eventWeight = 1.0;
        }
        log(LOG_TRACE, "Event " + std::to_string(iEvent) + " weight: " + std::to_string(eventWeight));
        
        // Start new event in cut flow tracker
        cutFlowTracker.startEvent();
        
        // Always apply RawEvents cut as true for every event processed
        cutFlowTracker.applyCut("RawEvents", true);
        
        // Event selection
        // if (std::abs(vz) > vzCut) {
        //     cutFlowTracker.applyCut("VertexCut", false);
        //     continue;
        // }
        // cutFlowTracker.applyCut("VertexCut", true);

        // === Fill event-level histograms after event-level cuts ===
        auto fillEvent1D = [&](const std::string& hname, double value, double weight=1.0) {
            auto it = hist1DMap.find("Event/" + hname);
            if (it != hist1DMap.end() && it->second) it->second->Fill(value, weight);
        };
        fillEvent1D("hVz", vz, eventWeight);
        fillEvent1D("hHiHF", hiHF, eventWeight);
        fillEvent1D("hCentrality", hiBin, eventWeight);
        fillEvent1D("hEventWeight", eventWeight, 1.0);
        
        // Determine centrality bin for this event using centralized helper
        CentralityInfo centInfo = getCentralityInfo(hiBin, centralityBins, useCentrality);
        if (!centInfo.isValid) {
            log(LOG_DEBUG, "Event centrality not in any configured bin: " + std::to_string(hiBin));
            continue;
        }
        
        // For cut flow tracking - use centrality bin name or "inclusive" for pp
        std::string centBin = centInfo.binName;
        if (useCentrality) {
            cutFlowTracker.startCentralityBin(centBin);
            cutFlowTracker.applyCut("CentralityCut", true, centBin);
        } else {
            // For pp system, we don't use centrality-based cut flow tracking
            // Just set centBin for consistency, but don't apply centrality cuts
            log(LOG_TRACE, "PP system: skipping centrality-based cut flow tracking");
        }
        
        std::string centName = centInfo.binName;
        log(LOG_TRACE, "Filling histograms for " + centName + "/General/ with weight: " + std::to_string(eventWeight));
        // General histograms
        auto fill1D = [&](const std::string& hname, double value, double weight=1.0) {
            outFile->cd();
            auto it = hist1DMap.find(centName + "/General/" + hname);
            if (it != hist1DMap.end() && it->second) it->second->Fill(value, weight);
            log(LOG_TRACE, "Filling 1D hist: " + centName + "/General/" + hname + " with value " + std::to_string(value));
            if (it != hist1DMap.end() && it->second) log(LOG_TRACE, "Filling 1D hist: " + centName + "/General/" + hname + " in directory " + (it->second->GetDirectory() ? it->second->GetDirectory()->GetName() : "nullptr"));
        };
        auto fill2D = [&](const std::string& hname, double x, double y, double weight=1.0) {
            outFile->cd();
            auto it = hist2DMap.find(centName + "/General/" + hname);
            if (it != hist2DMap.end() && it->second) it->second->Fill(x, y, weight);
        };
        // auto fillProfile = [&](const std::string& hname, double x, double y, double weight=1.0) {
        //     auto it = profileMap.find(centName + "/General/" + hname);
        //     if (it != profileMap.end() && it->second) it->second->Fill(x, y, weight);
        // };
        
        selectedRho = rho;
        outFile->cd();
        outFile->cd((centName + "/General/").c_str());
        fill1D("hRho", rho, eventWeight);

        // Photon selection (centrality-level cuts)
        std::vector<int> kinematicCandidates;
        for (int iPho = 0; iPho < nPhotons; ++iPho) {
            // Apply only basic kinematic cuts
            if (phoEt->at(iPho) < photonEtMin) continue;
            if (std::abs(phoEta->at(iPho)) > photonEtaMax) continue;
            
            // Store candidate index
            kinematicCandidates.push_back(iPho);
        }
        bool passPhotonKinematics = !kinematicCandidates.empty();
        cutFlowTracker.applyCut("PhotonKinematics", passPhotonKinematics, centBin);
        if (!passPhotonKinematics) continue;
        selectedPhotonIndex = -1;
        float maxPhotonEt = 0;
        for (int idx : kinematicCandidates) {
            if (phoEt->at(idx) > maxPhotonEt) {
                maxPhotonEt = phoEt->at(idx);
                selectedPhotonIndex = idx;
            }
        }
        
        if (selectedPhotonIndex >= 0 && isMC && config->GetValue("MCPhotonMatchRequired", 1)) {
            // Check if MC branches are available before using them
            if (!phoGenMatchedIndex) {
                log(LOG_INFO, "MC photon matching required but ggHi_pho_genMatchedIndex branch not available. Skipping MC checks.");
            } 
            else {
                int genMatchedIndex = phoGenMatchedIndex->at(selectedPhotonIndex);
                if (genMatchedIndex < 0) {
                    selectedPhotonIndex = -1;
                    cutFlowTracker.applyCut("MCPhotonMatch", false, centBin);
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
                    // Check particle isolation (only if mcCalIsoDR04 branch is available)
                    if (mcCalIsoDR04) {
                        float calIsoVal = config->GetValue("MCPhotonCalIsoDR04Max", 10000);
                                                    
                        // Check if MCIsolation is less than given value
                        bool validMCIso = false;
                        if (mcCalIsoDR04->at(genMatchedIndex) < calIsoVal) {
                            validMCIso = true;
                        }
                        if (!validMCIso) selectedPhotonIndex = -1;
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
                                cutFlowTracker.applyCut("MCPhotonMatch", true, centBin);
                                break;
                            }
                        }
                        if (!validMomPID) selectedPhotonIndex = -1;
                    }
                }
            }                
        }
        if(selectedPhotonIndex<0) continue; //Failed MCPhoton Match
        cutFlowTracker.applyCut("PhotonEta", true, centBin);
        bool passHoverE = (phoHoverE->at(selectedPhotonIndex) <= photonHoverEMax);
        cutFlowTracker.applyCut("PhotonHoverE", passHoverE, centBin);
        bool passSigmaIEtaIEta = (phoSigmaIEtaIEta->at(selectedPhotonIndex) <= photonSigmaIEtaIEtaMax);
        cutFlowTracker.applyCut("PhotonSigmaIEtaIEta", passSigmaIEtaIEta, centBin);
        // float phoIso = pho_ecalClusterIsoR3->at(selectedPhotonIndex) + pho_hcalRechitIsoR3->at(selectedPhotonIndex) + pho_trackIsoR3PtCut20->at(selectedPhotonIndex);
        float phoIso = pfpIso3subUEec->at(selectedPhotonIndex)+pfcIso3subUEec->at(selectedPhotonIndex)+pfnIso3subUEec->at(selectedPhotonIndex);
        bool passIso = (phoIso <= photonIsoMax);
        cutFlowTracker.applyCut("PhotonIsolation", passIso, centBin);
        bool passR9 = (phoR9->at(selectedPhotonIndex) >= photonR9Min);
        cutFlowTracker.applyCut("PhotonR9", passR9, centBin);
        
        // Photon histograms
        outFile->cd();
        outFile->cd((centName + "/General/").c_str());
        fill1D("hNPhotons", nPhotons, eventWeight);
        fill1D("hVz", vz, eventWeight);
        fill1D("hHiHF", hiHF, eventWeight);
        if (useCentrality) {
            fill1D("hCentrality", hiBin, eventWeight);
        } else {
            // For pp system, hiBin is meaningless but we can still fill it for consistency
            fill1D("hCentrality", -1, eventWeight);  // Use 0 as placeholder for pp
        }
        fill1D("hPhotonEt", phoEt->at(selectedPhotonIndex), eventWeight);
        fill1D("hPhotonEta", phoEta->at(selectedPhotonIndex), eventWeight);

        if(passSigmaIEtaIEta && passIso && passR9)
            fill1D("hPhotonHoverE", phoHoverE->at(selectedPhotonIndex), eventWeight);
        if(passHoverE && passIso && passR9)
            fill1D("hPhotonSigmaIEtaIEta", phoSigmaIEtaIEta->at(selectedPhotonIndex), eventWeight);
        if(passHoverE && passSigmaIEtaIEta && passR9)
            fill1D("hPhotonIso", phoIso, eventWeight);
        if(passHoverE && passSigmaIEtaIEta && passIso)
            fill1D("hPhotonR9", phoR9->at(selectedPhotonIndex), eventWeight);
        // --- MC photon histograms ---
        if (isMC && selectedPhotonIndex >= 0 && passHoverE && passSigmaIEtaIEta && passIso && passR9) {
            if (phoGenMatchedIndex) fill1D("hPhotonGenMatch", phoGenMatchedIndex->at(selectedPhotonIndex), eventWeight);
            if (phoGenMatchedIndex && phoGenMatchedIndex->at(selectedPhotonIndex) >= 0) {
                outFile->cd();
                outFile->cd((centName + "/General/").c_str());
                int genIndex = phoGenMatchedIndex->at(selectedPhotonIndex);
                if(genIndex>=0){
                    fill1D("hMCPhotonEt", mcPt->at(genIndex), eventWeight);
                    fill1D("hMCPhotonEta", mcEta->at(genIndex), eventWeight);
                    fill1D("hMCPhotonPhi", mcPhi->at(genIndex), eventWeight);
                    fill1D("hMCPhotonPID", mcPID->at(genIndex), eventWeight); 
                    fill1D("hMCPhotonMomPID", mcMomPID->at(genIndex), eventWeight);
                    fill2D("h2PhotonRecoEtVsGenEt", phoEt->at(selectedPhotonIndex), mcPt->at(genIndex), eventWeight);
                    // TODO: Add photon Et resolution plot
                }
            }
        }
        

        if (!passHoverE) { selectedPhotonIndex = -1; continue; }
        if (!passSigmaIEtaIEta) { selectedPhotonIndex = -1; continue; }
        if (!passIso) { selectedPhotonIndex = -1; continue; }
        if (!passR9) { selectedPhotonIndex = -1; continue; }
        if(selectedPhotonIndex<0) continue;
        log(LOG_TRACE, "Photon selection in event : "+std::to_string(iEvent));
        // Final photon selection check
        nWithPhoton++;
        
        // Store selected photon information
        selectedEventWeight = eventWeight;
        selectedHiBin = hiBin;
        selectedVz = vz;
        selectedHiHF = hiHF;
        selectedPhotonEt = phoEt->at(selectedPhotonIndex);
        selectedPhotonEta = phoEta->at(selectedPhotonIndex);
        selectedPhotonPhi = phoPhi->at(selectedPhotonIndex);
        selectedPhotonHoverE = phoHoverE->at(selectedPhotonIndex);
        selectedPhotonSigmaIEtaIEta = phoSigmaIEtaIEta->at(selectedPhotonIndex);
        // selectedPhotonECALIso = pho_ecalClusterIsoR3->at(selectedPhotonIndex);
        // selectedPhotonHCALIso = pho_hcalRechitIsoR3->at(selectedPhotonIndex);
        // selectedPhotonTRKIso = pho_trackIsoR3PtCut20->at(selectedPhotonIndex);
        selectedPhotonPFPIso = pfpIso3subUEec->at(selectedPhotonIndex);
        selectedPhotonPFCIso = pfcIso3subUEec->at(selectedPhotonIndex);
        selectedPhotonPFNIso = pfnIso3subUEec->at(selectedPhotonIndex);
        selectedPhotonIso = pfpIso3subUEec->at(selectedPhotonIndex)+pfcIso3subUEec->at(selectedPhotonIndex)+pfnIso3subUEec->at(selectedPhotonIndex);
        // selectedPhotonIso = pho_ecalClusterIsoR3->at(selectedPhotonIndex) + pho_hcalRechitIsoR3->at(selectedPhotonIndex) + pho_trackIsoR3PtCut20->at(selectedPhotonIndex);
        selectedPhotonR9 = phoR9->at(selectedPhotonIndex);
        if (isMC && selectedPhotonIndex >= 0) {
            int genIndex = phoGenMatchedIndex->at(selectedPhotonIndex);
            selectedMCPhotonEt = mcPt->at(genIndex);
            selectedMCPhotonEta = mcEta->at(genIndex);
            selectedMCPhotonPhi = mcPhi->at(genIndex);
            selectedMCPhotonPID = mcPID->at(genIndex);
            selectedMCPhotonIso = mcCalIsoDR04->at(genIndex);
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
            selectedRefJetXjs[collection] = -999;
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
        bool hasAnySelectedJet = false;
        for (const auto& collection : jetCollections) {
            cutFlowTracker.startJetCollection(centBin, collection);
            int nJets = jetManager.getNJets(collection);
            float maxJetPt = 0;
            int bestJetIndex = -1;
            bool passJetKinematics = false;
            for (int iJet = 0; iJet < nJets; ++iJet) {
                float jetPt = jetManager.getJetPt(collection, iJet);
                float jetEta = jetManager.getJetEta(collection, iJet);
                if (jetPt >= jetPtMin && std::abs(jetEta) <= jetEtaMax) {
                    passJetKinematics = true;
                    float jetPhi = jetManager.getJetPhi(collection, iJet);
                    float dPhi = getDeltaPhi(selectedPhotonPhi, jetPhi);
                    
                    // Select highest pT jet passing all cuts
                    if (jetPt > maxJetPt && dPhi >= deltaPhiMin) {
                        maxJetPt = jetPt;
                        bestJetIndex = iJet;
                    }
                }
            }
            cutFlowTracker.applyCut("JetKinematics", passJetKinematics, centBin, collection);
            if (!passJetKinematics) continue;
            if (bestJetIndex >= 0) {
                log(LOG_TRACE, "Jet Kinematics selection in event : "+std::to_string(iEvent));
                
                selectedJetDeltaPhis[collection] = getDeltaPhi(selectedPhotonPhi, jetManager.getJetPhi(collection, bestJetIndex));
                // Check and apply DeltaPhi cut if configured
                float deltaPhiMin = config->GetValue("DeltaPhiMin", -1.0);
                if (deltaPhiMin > 0) {
                    bool passDeltaPhi = selectedJetDeltaPhis[collection] >= deltaPhiMin;
                    cutFlowTracker.applyCut("DeltaPhi", passDeltaPhi, centBin, collection);
                    if (!passDeltaPhi) { selectedJetIndexes[collection] = -1; continue; }
                }
                
                selectedJetXjs[collection] = getXj(jetManager.getJetPt(collection, bestJetIndex), selectedPhotonEt);
                selectedRefJetXjs[collection] = getXj(jetManager.getRefJetPt(collection, bestJetIndex),selectedMCPhotonEt);

                // Check and apply XJ cut if configured
                float xjMin = config->GetValue("XjMin", -1.0);
                if (xjMin > 0) {
                    bool passXj = selectedJetXjs[collection] >= xjMin;
                    cutFlowTracker.applyCut("XjCut", passXj, centBin, collection);
                    if (!passXj) { selectedJetIndexes[collection] = -1; continue; }
                }
                selectedJetIndexes[collection] = bestJetIndex;
                selectedJetPts[collection] = jetManager.getJetPt(collection, bestJetIndex);
                selectedJetEtas[collection] = jetManager.getJetEta(collection, bestJetIndex);
                selectedJetPhis[collection] = jetManager.getJetPhi(collection, bestJetIndex);
                selectedJetMasses[collection] = jetManager.getJetMass(collection, bestJetIndex);
                selectedJetAreas[collection] = jetManager.getJetArea(collection, bestJetIndex);
                selectedJetDynSplits[collection] = jetManager.getJetDynSplit(collection, bestJetIndex);
                selectedJetDynKts[collection] = jetManager.getJetDynKt(collection, bestJetIndex);
                selectedJetDynZs[collection] = jetManager.getJetDynZ(collection, bestJetIndex);
                selectedJetGirths[collection] = jetManager.getJetGirth(collection, bestJetIndex);
                selectedJetThrusts[collection] = jetManager.getJetThrust(collection, bestJetIndex);
                selectedJetLHAs[collection] = jetManager.getJetLHA(collection, bestJetIndex);
                selectedJetPtDs[collection] = jetManager.getJetPtD(collection, bestJetIndex);
                selectedJetDynDeltaRs[collection] = jetManager.getJetDynDeltaR(collection, bestJetIndex);
                selectedJetIntJetMultis[collection] = jetManager.getJetIntJetMulti(collection, bestJetIndex);
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
                
                log(LOG_TRACE, "Filling histograms for " + collection + "/" + centName + 
                    " with weight: " + std::to_string(eventWeight));
                
                // Navigate to subdirectory and fill histograms with weights
                auto fill1Djet = [&](const std::string& hname, double value, double weight=1.0) {
                    outFile->cd();
                    auto it = hist1DMap.find(centName + "/" + collection + "/" + hname);
                    if (it != hist1DMap.end() && it->second) it->second->Fill(value, weight);
                };
                auto fill2Djet = [&](const std::string& hname, double x, double y, double weight=1.0) {
                    outFile->cd();
                    auto it = hist2DMap.find(centName + "/" + collection + "/" + hname);
                    if (it != hist2DMap.end() && it->second) it->second->Fill(x, y, weight);
                };
                // auto fillProfilejet = [&](const std::string& hname, double x, double y, double weight=1.0) {
                //     auto it = profileMap.find(centName + "/" + collection + "/" + hname);
                //     if (it != profileMap.end() && it->second) it->second->Fill(x, y, weight);
                // };
                outFile->cd();
                if (outFile->cd((centName + "/" + collection).c_str())) {
                    fill1Djet("hJetPt", selectedJetPts[collection], eventWeight);
                    fill1Djet("hJetEta", selectedJetEtas[collection], eventWeight);
                    fill1Djet("hJetPhi", selectedJetPhis[collection], eventWeight);
                    fill1Djet("hDeltaPhi", selectedJetDeltaPhis[collection], eventWeight);
                    fill1Djet("hJetXj", selectedJetXjs[collection], eventWeight);

                    // Jet observables histograms                        
                    fill1Djet("hJetMass", selectedJetMasses[collection], eventWeight);
                    fill1Djet("hJetArea", selectedJetAreas[collection], eventWeight);
                    fill1Djet("hJetDynDeltaR", selectedJetDynDeltaRs[collection], eventWeight);
                    fill1Djet("hIntJetMulti", selectedJetIntJetMultis[collection], eventWeight);
                    fill1Djet("hJetDynSplit", selectedJetDynSplits[collection], eventWeight);
                    fill1Djet("hJetDynKt", selectedJetDynKts[collection], eventWeight);
                    fill1Djet("hJetDynZ", selectedJetDynZs[collection], eventWeight);
                    fill1Djet("hJetGirth", selectedJetGirths[collection], eventWeight);
                    fill1Djet("hJetThrust", selectedJetThrusts[collection], eventWeight);
                    fill1Djet("hJetLHA", selectedJetLHAs[collection], eventWeight);
                    fill1Djet("hJetPtD", selectedJetPtDs[collection], eventWeight);
                    
                    fill1Djet("hNJets", jetManager.getNJets(collection), eventWeight);
                    fill2Djet("h2JetEtaVsJetPt", selectedJetEtas[collection], selectedJetPts[collection], eventWeight);
                    fill2Djet("h2JetPhiVsJetEta", selectedJetPhis[collection], selectedJetEtas[collection], eventWeight);

                    fill2Djet("h2JetPtVsJetMass", selectedJetPts[collection], selectedJetMasses[collection], eventWeight);                    
                    fill2Djet("h2JetPtVsJetGirth", selectedJetPts[collection], selectedJetGirths[collection], eventWeight);
                    fill2Djet("h2JetPtVsJetThrust", selectedJetPts[collection], selectedJetThrusts[collection], eventWeight);
                    fill2Djet("h2JetPtVsJetPtD", selectedJetPts[collection], selectedJetPtDs[collection], eventWeight);
                    // fillProfilejet("pGirthVsPt", selectedJetPts[collection], selectedJetGirths[collection], eventWeight);
                    // fillProfilejet("pThrustVsPt", selectedJetPts[collection], selectedJetThrusts[collection], eventWeight);
                    // fillProfilejet("pPtDVsPt", selectedJetPts[collection], selectedJetPtDs[collection], eventWeight);
                    
                    // Ref jet (MC-matched) histograms
                    if (isMC && selectedRefJetPts[collection] > -900) {
                        fill1Djet("hRefXj", selectedRefJetXjs[collection], eventWeight);
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
                        fill2Djet("h2RefJetEtaVsRefJetPt", selectedRefJetEtas[collection], selectedRefJetPts[collection], eventWeight);
                        fill2Djet("h2RefJetPtVsRefJetMass", selectedRefJetPts[collection], selectedRefJetMasses[collection], eventWeight);
                        fill2Djet("h2RefJetPtVsRefJetGirth", selectedRefJetPts[collection], selectedRefJetGirths[collection], eventWeight);
                        fill2Djet("h2RefJetPtVsRefJetThrust", selectedRefJetPts[collection], selectedRefJetThrusts[collection], eventWeight);
                        fill2Djet("h2RefJetPtVsRefJetPtD", selectedRefJetPts[collection], selectedRefJetPtDs[collection], eventWeight);
                        // Reco vs Ref correlation (new convention)
                        fill2Djet("h2JetPtVsRefPt", selectedJetPts[collection], selectedRefJetPts[collection], eventWeight);
                        fill2Djet("h2JetMassVsRefMass", selectedJetMasses[collection], selectedRefJetMasses[collection], eventWeight);
                        fill2Djet("h2JetGirthVsRefGirth", selectedJetGirths[collection], selectedRefJetGirths[collection], eventWeight);
                        fill2Djet("h2JetThrustVsRefThrust", selectedJetThrusts[collection], selectedRefJetThrusts[collection], eventWeight);
                        fill2Djet("h2JetLHAVsRefLHA", selectedJetLHAs[collection], selectedRefJetLHAs[collection], eventWeight);
                        fill2Djet("h2JetPtDVsRefPtD", selectedJetPtDs[collection], selectedRefJetPtDs[collection], eventWeight);
                        // Jet resolution histograms (Reco-Gen)/Gen
                        // if(selectedRefJetPts[collection]>=60 && selectedRefJetPts[collection]<120){
                            outFile->cd();
                            if (selectedRefJetPts[collection] != 0)
                                fill1Djet("hJetPtRes", (selectedJetPts[collection] - selectedRefJetPts[collection]) / selectedRefJetPts[collection], eventWeight);
                            if (selectedRefJetMasses[collection] != 0)
                                fill1Djet("hJetMassRes", (selectedJetMasses[collection] - selectedRefJetMasses[collection]) / selectedRefJetMasses[collection], eventWeight);
                            if (selectedRefJetGirths[collection] != 0)
                                fill1Djet("hJetGirthRes", (selectedJetGirths[collection] - selectedRefJetGirths[collection]) / selectedRefJetGirths[collection], eventWeight);
                            if (selectedRefJetThrusts[collection] != 0)
                                fill1Djet("hJetThrustRes", (selectedJetThrusts[collection] - selectedRefJetThrusts[collection]) / selectedRefJetThrusts[collection], eventWeight);
                            if (selectedRefJetLHAs[collection] != 0)
                                fill1Djet("hJetLHARes", (selectedJetLHAs[collection] - selectedRefJetLHAs[collection]) / selectedRefJetLHAs[collection], eventWeight);
                            if (selectedRefJetPtDs[collection] != 0)
                                fill1Djet("hJetPtDRes", (selectedJetPtDs[collection] - selectedRefJetPtDs[collection]) / selectedRefJetPtDs[collection], eventWeight);
                            if (selectedRefJetDynKts[collection] != 0)
                                fill1Djet("hJetDynKtRes", (selectedJetDynKts[collection] - selectedRefJetDynKts[collection]) / selectedRefJetDynKts[collection], eventWeight);
                            if (selectedRefJetDynDeltaRs[collection] != 0)
                                fill1Djet("hJetDynDeltaRRes", (selectedJetDynDeltaRs[collection] - selectedRefJetDynDeltaRs[collection]) / selectedRefJetDynDeltaRs[collection], eventWeight);
                            if (selectedRefJetDynZs[collection] != 0)
                                fill1Djet("hJetDynZRes", (selectedJetDynZs[collection] - selectedRefJetDynZs[collection]) / selectedRefJetDynZs[collection], eventWeight);
                        // }
                    }
                }
                bool passJetSelection = (selectedJetIndexes[collection] >= 0);
                cutFlowTracker.applyCut("JetSelection", passJetSelection, centBin, collection);
                if (passJetSelection) {
                    hasAnySelectedJet = true;
                    cutFlowTracker.applyCut("FinalSelection", true, centBin, collection);
                } else {
                    cutFlowTracker.applyCut("FinalSelection", false, centBin, collection);
                }
            } else {
                cutFlowTracker.applyCut("JetSelection", false, centBin, collection);
                cutFlowTracker.applyCut("FinalSelection", false, centBin, collection);
            }
        }
        if (hasAnySelectedJet) {
            nWithJet++;
            
            // Fill output tree
            outTree->Fill();
            nPassed++;
        }
    }
    
    // Write output tree
    outFile->cd();
    outTree->Write();
    log(LOG_INFO, "Writing histograms to file...");
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
 * 
 * SYSTEM COMPATIBILITY:
 * This function creates histogram directory structures appropriate for both systems:
 * - PbPb: Creates centrality-binned directories (e.g., "cent0to60/General/", "cent0to60/AK4Z1/")
 * - pp: Creates single inclusive directory (e.g., "inclusive/General/", "inclusive/AK4Z1/")
 * 
 * DIRECTORY STRUCTURE:
 * - Event-level histograms: "Event/" (system-agnostic)
 * - Analysis histograms: "{binName}/{collection}/" where:
 *   * binName = "cent{low}to{high}" for PbPb or "inclusive" for pp
 *   * collection = jet algorithm name (AK4Z1, etc.) or "General"
 * 
 * @param outFile Output ROOT file
 * @param jetCollections Vector of jet collection names
 * @param centralityBins Vector of centrality bin edges (used only if useCentrality=true)
 * @param plotConfig Plotting configuration with histogram definitions
 * @param useCentrality Flag to enable centrality binning (PbPb=true, pp=false)
*/
void createHistograms(TFile* outFile, const std::vector<std::string>& jetCollections, 
                                 const std::vector<float>& centralityBins, const PlottingConfiguration& plotConfig, bool useCentrality) {
    if (!outFile) return;

    // Debug: Check input parameters
    log(LOG_DEBUG, "createHistograms called with:");
    log(LOG_DEBUG, "  jetCollections.size() = " + std::to_string(jetCollections.size()));
    log(LOG_DEBUG, "  centralityBins.size() = " + std::to_string(centralityBins.size()));
    log(LOG_DEBUG, "  plotConfig.histogramConfigs.size() = " + std::to_string(plotConfig.histogramConfigs.size()));
    log(LOG_DEBUG, "  useCentrality: " + std::string(useCentrality ? "true" : "false"));

    if (jetCollections.empty()) {
        log(LOG_INFO, "jetCollections is empty!");
        return;
    }

    // Determine if this is MC data by checking for DataType config
    bool isMC = false;
    if (plotConfig.configFile) {
        std::string dataType = plotConfig.configFile->GetValue("DataType", "Data");
        isMC = (dataType == "MC" || dataType == "mc");
        log(LOG_DEBUG, "  isMC: " + std::to_string(isMC) + " (from DataType: " + dataType + ")");
    }

    // Create bin directories based on system type
    std::vector<std::string> binNames;
    if (useCentrality) {
        if (centralityBins.size() < 2) {
            log(LOG_WARNING, "centralityBins has < 2 elements, cannot create centrality bins!");
            std::string binContents = "centralityBins contents: ";
            for (float bin : centralityBins) {
                binContents += std::to_string(bin) + " ";
            }
            log(LOG_WARNING, binContents);
            return;
        }
        // Create centrality bins for PbPb
        for (size_t i = 0; i < centralityBins.size() - 1; ++i) {
            std::string centName = "cent" + std::to_string(static_cast<int>(centralityBins[i])/2) + 
                                   "to" + std::to_string(static_cast<int>(centralityBins[i+1])/2);
            binNames.push_back(centName);
        }
    } else {
        // Create single inclusive bin for pp
        binNames.push_back("inclusive");
    }

    // Create histograms for each bin (centrality or inclusive)
    for (const std::string& binName : binNames) {
        TDirectory* binDir = outFile->mkdir(binName.c_str());
        binDir->cd();
        
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
        TDirectory* generalDir = binDir->mkdir("General");
        generalDir->cd();
        for (const auto& kv : plotConfig.histogramConfigs) {
            const std::string& hname = kv.first;
            const HistogramConfig& hcfg = kv.second;
            log(LOG_TRACE, "Creating hist: " + binName + "/General/" + hname);
            // Use config-driven classification
            if (plotConfig.generalHistograms.count(hname)) {
                if (hcfg.type == "TH1D") {
                    TH1D* h = createHistogram1D(hcfg,"h"+hname);
                    h->SetDirectory(generalDir);
                    hist1DMap[binName + "/General/h" + hname] = h;
                } else if (hcfg.type == "TH2D") {
                    TH2D* h = createHistogram2D(hcfg,"h2"+hname);
                    h->SetDirectory(generalDir);
                    hist2DMap[binName + "/General/h2" + hname] = h;
                } else if (hcfg.type == "TProfile") {
                    TProfile* p = createProfile(hcfg,"p"+hname);
                    p->SetDirectory(generalDir);
                    profileMap[binName + "/General/p" + hname] = p;
                }
            }
        }
        
        // Per-collection histograms
        for (const auto& collection : jetCollections) {
            TDirectory* collDir = binDir->mkdir(collection.c_str());
            collDir->cd();
            for (const auto& kv : plotConfig.histogramConfigs) {
                const std::string& hname = kv.first;
                const HistogramConfig& hcfg = kv.second;
                if (plotConfig.jetHistograms.count(hname)) {
                    if (hcfg.type == "TH1D") {
                        TH1D* h = createHistogram1D(hcfg,"h"+hname);
                        h->SetDirectory(collDir);
                        hist1DMap[binName + "/" + collection + "/h" + hname] = h;
                    } else if (hcfg.type == "TH2D") {
                        TH2D* h = createHistogram2D(hcfg,"h2"+hname);
                        h->SetDirectory(collDir);
                        hist2DMap[binName + "/" + collection + "/h2" + hname] = h;
                    } else if (hcfg.type == "TProfile") {
                        TProfile* p = createProfile(hcfg,"p"+hname);
                        p->SetDirectory(collDir);
                        profileMap[binName + "/" + collection + "/p" + hname] = p;
                    }
                }
            }
        }
        outFile->cd();
    }

    // --- Event-level histograms (not binned by centrality) ---
    outFile->cd();
    TDirectory* eventDir = outFile->mkdir("Event");
    eventDir->cd();
    for (const auto& kv : plotConfig.histogramConfigs) {
        const std::string& hname = kv.first;
        const HistogramConfig& hcfg = kv.second;
        if (plotConfig.eventHistograms.count(hname)) {
            log(LOG_TRACE, "Creating event-level hist: Event/" + hname);
            if (hcfg.type == "TH1D") {
                TH1D* h = createHistogram1D(hcfg, "h" + hname);
                h->SetDirectory(eventDir);
                hist1DMap["Event/h" + hname] = h;
            } else if (hcfg.type == "TH2D") {
                TH2D* h = createHistogram2D(hcfg, "h2" + hname);
                h->SetDirectory(eventDir);
                hist2DMap["Event/h2" + hname] = h;
            } else if (hcfg.type == "TProfile") {
                TProfile* p = createProfile(hcfg, "p" + hname);
                p->SetDirectory(eventDir);
                profileMap["Event/p" + hname] = p;
            }
        }
    }
    outFile->cd();

    // --- Histogram validation: config vs created ---
    //! TO BE UPDATED TO CHECK ACTUALLY CREATED HISTOGRAMS
    
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
