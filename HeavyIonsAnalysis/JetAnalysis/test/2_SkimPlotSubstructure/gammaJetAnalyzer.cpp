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
 *   ./gammaJetAnalyzer -c ../configs/JetSub_2023_PbPb_MC.config -t 1000
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
    bool testMode = false;
    int maxEvents = 1000;
    
    // Parse command line arguments using getopt_long
    static struct option long_options[] = {
        {"config",     required_argument, 0, 'c'},
        {"test",       optional_argument, 0, 't'},
        {"production", no_argument,       0, 'p'},
        {"help",       no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };
    
    int option_index = 0;
    int c;
    
    while ((c = getopt_long(argc, argv, "c:t::ph", long_options, &option_index)) != -1) {
        switch (c) {
            case 'c':
                configFile = optarg;
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
            case 'p':
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
    log(LOG_DEBUG, "  Config file: " + configFile);
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
        
        // Load configuration
        TEnv* config = new TEnv();
        if (gSystem->AccessPathName(configFile.c_str())) {
            std::cerr << "Error: Configuration file not found: " << configFile << std::endl;
            delete config;
            return 1;
        }
        
        int readStatus = config->ReadFile(configFile.c_str(), kEnvLocal);
        if (readStatus != 0) {
            std::cerr << "Error: Failed to read configuration file: " << configFile << std::endl;
            delete config;
            return 1;
        }
        
        log(LOG_INFO, "Loaded configuration from " + configFile);
        
        // Print configuration summary
        printConfig(config);
        
        // Load plotting configuration
        std::string plottingConfigPath = config->GetValue("PlottingConfig", "../configs/PlotJetSub_2023_PbPb_MC.config");
        PlottingConfiguration plotConfig;
        
        log(LOG_INFO, "Loading plotting configuration from: " + plottingConfigPath);
        if (!loadPlottingConfig(plottingConfigPath, plotConfig)) {
            log(LOG_ERROR, "Failed to load plotting configuration, using defaults");
            // Initialize with default configuration if loading fails
            plotConfig = PlottingConfiguration(); // Uses default constructor
        } else {
            log(LOG_INFO, "Successfully loaded plotting configuration");
            log(LOG_DEBUG, "Loaded " + std::to_string(plotConfig.histogramConfigs.size()) + " histogram configurations");
        }
        
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
        
        // Clean up
        outFile->Close();
        delete outFile;
        delete chain;
        delete config;
        
        // Print timing information
        timer.Stop();
        double realTime = timer.RealTime();
        double cpuTime = timer.CpuTime();
        
        log(LOG_INFO, "==================================================");
        log(LOG_INFO, "Analysis complete. Output written to: " + outputFile);
        log(LOG_INFO, "Real time: " + std::to_string(realTime) + " seconds, CPU time: " + std::to_string(cpuTime) + " seconds");
        log(LOG_INFO, "==================================================");
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
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
        
        // Event selection
        if (std::abs(vz) > vzCut) continue;
        if (hiHF < hiHFCutMin || hiHF > hiHFCutMax) continue;
        
        // FIXED: Photon selection using two-stage approach
        // Stage 1: First apply only kinematic cuts and find leading photon
        std::vector<int> kinematicCandidates;
        
        for (int iPho = 0; iPho < nPhotons; ++iPho) {
            // Apply only basic kinematic cuts
            if (phoEt->at(iPho) < photonEtMin) continue;
            if (std::abs(phoEta->at(iPho)) > photonEtaMax) continue;
            
            // Store candidate index
            kinematicCandidates.push_back(iPho);
        }
        
        // Stage 2: Find the highest ET photon among kinematic candidates
        selectedPhotonIndex = -1;
        float maxPhotonEt = 0;
        
        for (int idx : kinematicCandidates) {
            if (phoEt->at(idx) > maxPhotonEt) {
                maxPhotonEt = phoEt->at(idx);
                selectedPhotonIndex = idx;
            }
        }
        
        // Stage 3: Apply photon ID criteria to the leading photon
        if (selectedPhotonIndex >= 0) {
            // Check shower shape, isolation and other ID criteria
            if (phoHoverE->at(selectedPhotonIndex) > photonHoverEMax ||
                phoSigmaIEtaIEta->at(selectedPhotonIndex) > photonSigmaIEtaIEtaMax ||
                phoIso->at(selectedPhotonIndex) > photonIsoMax ||
                phoR9->at(selectedPhotonIndex) < photonR9Min) {
                selectedPhotonIndex = -1; // Reset if ID criteria not satisfied
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
                                    break;
                                }
                            }
                            if (!validMomPID) selectedPhotonIndex = -1;
                        }
                    }
                }
            }
        }
        
        if (selectedPhotonIndex < 0) continue;
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
        }
        
        // Jet selection for each collection
        bool hasSelectedJet = false;
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
                
                // Calculate correlation variables
                selectedJetDeltaPhis[collection] = getDeltaPhi(selectedPhotonPhi, selectedJetPhis[collection]);
                selectedJetXjs[collection] = getXj(selectedJetPts[collection], selectedPhotonEt);
                
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
                        
                        TH1F* hDeltaPhi = (TH1F*)gDirectory->Get("hDeltaPhi");
                        if (hDeltaPhi) hDeltaPhi->Fill(selectedJetDeltaPhis[collection], eventWeight);
                        
                        TH1F* hXj = (TH1F*)gDirectory->Get("hXj");
                        if (hXj) hXj->Fill(selectedJetXjs[collection], eventWeight);
                        
                        // Jet substructure histograms
                        TH1F* hJetMass = (TH1F*)gDirectory->Get("hJetMass");
                        if (hJetMass) hJetMass->Fill(selectedJetMasses[collection], eventWeight);
                        
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
                        
                        // Profiles and 2D histograms with weights
                        TProfile* pDynSplitVsPt = (TProfile*)gDirectory->Get("pDynSplitVsPt");
                        if (pDynSplitVsPt) pDynSplitVsPt->Fill(selectedJetPts[collection], selectedJetDynSplits[collection], eventWeight);
                        
                        TH2F* h2DynSplitVsPt = (TH2F*)gDirectory->Get("h2DynSplitVsPt");
                        if (h2DynSplitVsPt) h2DynSplitVsPt->Fill(selectedJetPts[collection], selectedJetDynSplits[collection], eventWeight);
                    }
                }
            }
        }
        
        if (hasSelectedJet) {
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
            TH1F* hDeltaPhi = createHist1D("hDeltaPhi", "#Delta#phi;#Delta#phi;Entries", 50, 0, M_PI);
            TH1F* hXj = createHist1D("hXj", "x_{j} = p_{T}^{jet} / E_{T}^{#gamma};x_{j};Entries", 50, 0, 2.0);
            
            // Suppress unused variable warnings - histograms are created and stored in ROOT file automatically
            (void)hJetPt; (void)hJetEta; (void)hDeltaPhi; (void)hXj;
            
            // Jet substructure histograms
            TH1F* hJetMass = createHist1D("hJetMass", "Jet mass;m [GeV/c^{2}];Entries", 50, 0, 50);
            TH1F* hDynSplit = createHist1D("hDynSplit", "Dynamical groomed splitting scale;#sqrt{z#theta} [GeV/c];Entries", 50, 0, 50);
            TH1F* hDynKt = createHist1D("hDynKt", "Dynamical groomed k_{T};k_{T} [GeV/c];Entries", 50, 0, 50);
            TH1F* hDynZ = createHist1D("hDynZ", "Dynamical groomed z;z;Entries", 50, 0, 0.5);
            TH1F* hGirth = createHist1D("hGirth", "Jet girth;girth;Entries", 50, 0, 0.5);
            TH1F* hThrust = createHist1D("hThrust", "Jet thrust;thrust;Entries", 50, 0, 1.0);
            TH1F* hLHA = createHist1D("hLHA", "Jet LHA;LHA;Entries", 50, 0, 1.0);
            TH1F* hPtD = createHist1D("hPtD", "Jet p_{T}D;p_{T}D;Entries", 50, 0, 1.0);
            
            // Suppress unused variable warnings (histograms are retrieved by name later)
            (void)hJetMass; (void)hDynSplit; (void)hDynKt; (void)hDynZ;
            (void)hGirth; (void)hThrust; (void)hLHA; (void)hPtD;
            
            // Profiles and 2D histograms
            TProfile* pDynSplitVsPt = createHistProfile("pDynSplitVsPt", 
                                                       "Dynamical groomed splitting scale vs p_{T};p_{T} [GeV/c];<#sqrt{z#theta}> [GeV/c]", 
                                                       10, 40, 240, 0, 50);
            
            TH2F* h2DynSplitVsPt = createHist2D("h2DynSplitVsPt", 
                                               "Dynamical groomed splitting scale vs p_{T};p_{T} [GeV/c];#sqrt{z#theta} [GeV/c]", 
                                               10, 40, 240, 50, 0, 50);
            
            // Suppress unused variable warnings (histograms are retrieved by name later)
            (void)pDynSplitVsPt; (void)h2DynSplitVsPt;
            
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
            
            // Write histograms to the ROOT file
            for (auto hist : histograms) {
                hist->Write("", TObject::kWriteDelete);
            }
            
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
    (void)jetCollections;
    
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
    std::cout << "  --config, -c FILE      Config file path" << std::endl;
    std::cout << "  --test, -t [N]         Run in test mode with N events (default: 1000)" << std::endl;
    std::cout << "  --production, -p       Run in production mode (all events)" << std::endl;
    std::cout << "  --help, -h             Print this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  ./gammaJetAnalyzer -c config.config -t 10000" << std::endl;
    std::cout << "  ./gammaJetAnalyzer --config config.config --test 5000" << std::endl;
    std::cout << "  ./gammaJetAnalyzer -c config.config -p" << std::endl;
}
