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

// Forward declarations
void printUsage();
void processEvents(TChain* chain, TEnv* config, JetCollectionManager& jetManager, TFile* outFile);
bool setupInputChain(TChain* chain, const std::string& inputDir, bool testMode, int maxFiles = 1);
void setupOutputTree(TTree* outTree, const std::vector<std::string>& jetCollections);
bool createOutputDirectories(const std::string& outputDir);
void printConfig(TEnv* config);
std::vector<float> getFloatVector(TEnv* config, const std::string& key, const std::vector<float>& defaultValues);
std::vector<std::string> getStringVector(TEnv* config, const std::string& key, const std::vector<std::string>& defaultValues);
void runAnalysis(const std::string& configFile, bool testMode = false, int maxEvents = -1);
void createHistograms(TFile* outFile, const std::vector<std::string>& jetCollections, const std::vector<float>& centralityBins);
float getDeltaPhi(float phi1, float phi2);
float getXj(float jetPt, float photonPt);

/**
 * Main function - entry point for standalone executable
 */
int main(int argc, char* argv[]) {
    std::string configFile = "";
    bool testMode = false;
    int maxEvents = -1;
    
    // Parse command line arguments
    int opt;
    static struct option long_options[] = {
        {"config", required_argument, 0, 'c'},
        {"test", optional_argument, 0, 't'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    while ((opt = getopt_long(argc, argv, "c:t::h", long_options, nullptr)) != -1) {
        switch (opt) {
            case 'c':
                configFile = optarg;
                break;
            case 't':
                testMode = true;
                if (optarg) {
                    maxEvents = std::atoi(optarg);
                } else {
                    maxEvents = 1000; // Default test events
                }
                break;
            case 'h':
                printUsage();
                return 0;
            default:
                printUsage();
                return 1;
        }
    }
    
    // Validate arguments
    if (configFile.empty()) {
        std::cerr << "Error: Configuration file must be specified with -c option" << std::endl;
        printUsage();
        return 1;
    }
    
    // Run the analysis
    try {
        runAnalysis(configFile, testMode, maxEvents);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

/**
 * Print usage information
 */
void printUsage() {
    std::cout << "Usage: gammaJetAnalyzer [OPTIONS]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -c, --config FILE     Configuration file (required)" << std::endl;
    std::cout << "  -t, --test [N]        Test mode (optional max events, default: 1000)" << std::endl;
    std::cout << "  -h, --help            Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  ./gammaJetAnalyzer -c ../configs/JetSub_2023_PbPb_MC.config" << std::endl;
    std::cout << "  ./gammaJetAnalyzer -c ../configs/JetSub_2023_PbPb_MC.config -t 1000" << std::endl;
    std::cout << "  ./gammaJetAnalyzer -c ../configs/JetSub_2023_PbPb_MC.config -t" << std::endl;
}

/**
 * Utility function to create output directories
 */
bool createOutputDirectories(const std::string& outputDir) {
    if (gSystem->mkdir(outputDir.c_str(), kTRUE) != 0) {
        // Check if directory already exists
        if (gSystem->AccessPathName(outputDir.c_str()) == 0) {
            std::cout << "Output directory already exists: " << outputDir << std::endl;
            return true;
        } else {
            std::cerr << "Failed to create output directory: " << outputDir << std::endl;
            return false;
        }
    }
    std::cout << "Created output directory: " << outputDir << std::endl;
    return true;
}

/**
 * Utility function to print configuration parameters
 */
void printConfig(TEnv* config) {
    if (!config) return;
    
    std::cout << std::endl;
    std::cout << "=== Configuration Parameters ===" << std::endl;
    std::cout << "Analysis Cases: " << config->GetValue("AnalysisCases", "") << std::endl;
    std::cout << "Input Directory: " << config->GetValue("InputDir", "") << std::endl;
    std::cout << "Output Directory: " << config->GetValue("OutputDir", "") << std::endl;
    std::cout << "Output Prefix: " << config->GetValue("OutputPrefix", "output") << std::endl;
    std::cout << "IsMC: " << (config->GetValue("IsMC", 0) ? "true" : "false") << std::endl;
    std::cout << "Verbosity: " << config->GetValue("Verbosity", 0) << std::endl;
    std::cout << "===============================" << std::endl;
    std::cout << std::endl;
}

/**
 * Utility function to parse float vectors from config
 */
std::vector<float> getFloatVector(TEnv* config, const std::string& key, const std::vector<float>& defaultValues) {
    std::string valueStr = config->GetValue(key.c_str(), "");
    if (valueStr.empty()) {
        return defaultValues;
    }
    
    std::vector<float> values;
    std::stringstream ss(valueStr);
    std::string item;
    
    while (std::getline(ss, item, ',')) {
        // Trim whitespace
        item.erase(0, item.find_first_not_of(" \t\n\r\f\v"));
        item.erase(item.find_last_not_of(" \t\n\r\f\v") + 1);
        
        if (!item.empty()) {
            values.push_back(std::stof(item));
        }
    }
    
    return values.empty() ? defaultValues : values;
}

/**
 * Utility function to parse string vectors from config
 */
std::vector<std::string> getStringVector(TEnv* config, const std::string& key, const std::vector<std::string>& defaultValues) {
    std::string valueStr = config->GetValue(key.c_str(), "");
    if (valueStr.empty()) {
        return defaultValues;
    }
    
    std::vector<std::string> values;
    std::stringstream ss(valueStr);
    std::string item;
    
    while (std::getline(ss, item, ',')) {
        // Trim whitespace
        item.erase(0, item.find_first_not_of(" \t\n\r\f\v"));
        item.erase(item.find_last_not_of(" \t\n\r\f\v") + 1);
        
        if (!item.empty()) {
            values.push_back(item);
        }
    }
    
    return values.empty() ? defaultValues : values;
}

/**
 * Main analysis function
 */
void runAnalysis(const std::string& configFile, bool testMode, int maxEvents) {
    // Start timer
    TStopwatch timer;
    timer.Start();
    
    // Print information
    std::cout << "==================================================" << std::endl;
    std::cout << "=== PhotonJet Analysis: Jet Substructure v1.0 ===" << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << "Config file: " << configFile << std::endl;
    if (testMode) {
        std::cout << "Running in TEST mode with " << maxEvents << " events" << std::endl;
    } else {
        std::cout << "Running in PRODUCTION mode (all events)" << std::endl;
    }
    
    // Load configuration
    TEnv* config = new TEnv();
    if (gSystem->AccessPathName(configFile.c_str())) {
        throw std::runtime_error("Configuration file not found: " + configFile);
    }
    
    int readStatus = config->ReadFile(configFile.c_str(), kEnvLocal);
    if (readStatus != 0) {
        delete config;
        throw std::runtime_error("Error reading configuration file: " + configFile);
    }
    
    std::cout << "Loaded configuration from " << configFile << std::endl;
    
    // Print configuration summary
    printConfig(config);
    
    // Get basic parameters
    std::string outputDir = config->GetValue("OutputDir", "");
    std::string outputPrefix = config->GetValue("OutputPrefix", "output");
    std::string inputDir = config->GetValue("InputDir", "");
    
    // Check essential parameters
    if (outputDir.empty()) {
        delete config;
        throw std::runtime_error("Output directory not specified in configuration");
    }
    
    if (inputDir.empty()) {
        delete config;
        throw std::runtime_error("Input directory not specified in configuration");
    }
    
    // Create output directories
    if (!createOutputDirectories(outputDir)) {
        delete config;
        throw std::runtime_error("Failed to create output directories");
    }
    
    // Setup input files
    TChain* chain = new TChain("jet_tree");
    if (!setupInputChain(chain, inputDir, testMode)) {
        delete chain;
        delete config;
        throw std::runtime_error("Failed to setup input chain");
    }
    
    // Configure max events if in test mode
    Long64_t nEvents = chain->GetEntries();
    if (testMode && maxEvents > 0 && maxEvents < nEvents) {
        nEvents = maxEvents;
        std::cout << "Test mode: processing " << nEvents << " events" << std::endl;
    } else {
        std::cout << "Production mode: processing all " << nEvents << " events" << std::endl;
    }
    
    // Setup output file
    std::string outputFile = outputDir + "/" + outputPrefix + "_output.root";
    
    TFile* outFile = new TFile(outputFile.c_str(), "RECREATE");
    if (!outFile || outFile->IsZombie()) {
        delete outFile;
        delete chain;
        delete config;
        throw std::runtime_error("Failed to create output file: " + outputFile);
    }
    
    // Initialize jet collection manager
    JetCollectionManager jetManager(config, chain);
    if (!jetManager.initialize()) {
        delete outFile;
        delete chain;
        delete config;
        throw std::runtime_error("Failed to initialize jet collections");
    }
    
    // Print jet collection information
    jetManager.printBranchMappings();
    
    // Process events
    processEvents(chain, config, jetManager, outFile);
    
    // Clean up
    outFile->Close();
    delete outFile;
    delete chain;
    delete config;
    
    // Print timing information
    timer.Stop();
    double realTime = timer.RealTime();
    double cpuTime = timer.CpuTime();
    
    std::cout << "==================================================" << std::endl;
    std::cout << "Analysis complete. Output written to: " << outputFile << std::endl;
    std::cout << "Processed " << nEvents << " events" << std::endl;
    std::cout << "Real time: " << realTime << " seconds, CPU time: " << cpuTime << " seconds" << std::endl;
    std::cout << "Events per second: " << nEvents / realTime << std::endl;
    std::cout << "==================================================" << std::endl;
}

/**
 * Process events in the chain
 */
void processEvents(TChain* chain, TEnv* config, JetCollectionManager& jetManager, TFile* outFile) {
    if (!chain || !outFile) return;
    
    // Get parameters from config
    bool isMC = config->GetValue("IsMC", 0);
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
    std::vector<float> centralityBins = getFloatVector(config, "CentralityBins", {0, 20, 60, 100, 200});
    std::vector<std::string> jetCollections = jetManager.getCollections();
    
    // Setup output tree
    TTree* outTree = new TTree("gammaJetTree", "Gamma-Jet Analysis");
    setupOutputTree(outTree, jetCollections);
    
    // Create histograms
    createHistograms(outFile, jetCollections, centralityBins);
    
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
        chain->SetBranchAddress("ggHi_genMatchedIndex", &phoGenMatchedIndex);
        chain->SetBranchAddress("ggHi_mcPID", &mcPID);
        chain->SetBranchAddress("ggHi_mcMomPID", &mcMomPID);
        chain->SetBranchAddress("ggHi_mcPt", &mcPt);
        chain->SetBranchAddress("ggHi_mcEta", &mcEta);
        chain->SetBranchAddress("ggHi_mcPhi", &mcPhi);
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
    Long64_t nEvents = chain->GetEntries();
    int nProcessed = 0;
    int nWithPhoton = 0;
    int nWithJet = 0;
    int nPassed = 0;
    
    for (Long64_t iEvent = 0; iEvent < nEvents; ++iEvent) {
        if (iEvent % 1000 == 0) {
            std::cout << "Processing event " << iEvent << "/" << nEvents << " (" 
                      << static_cast<double>(iEvent) / nEvents * 100 << "%)" << std::endl;
        }
        
        chain->GetEntry(iEvent);
        nProcessed++;
        
        // Event selection
        if (std::abs(vz) > vzCut) continue;
        if (hiHF < hiHFCutMin || hiHF > hiHFCutMax) continue;
        
        // Photon selection using two-stage approach
        std::vector<int> kinematicCandidates;
        
        for (int iPho = 0; iPho < nPhotons; ++iPho) {
            // Apply only basic kinematic cuts
            if (phoEt->at(iPho) < photonEtMin) continue;
            if (std::abs(phoEta->at(iPho)) > photonEtaMax) continue;
            
            // Store candidate index
            kinematicCandidates.push_back(iPho);
        }
        
        // Find the highest ET photon among kinematic candidates
        selectedPhotonIndex = -1;
        float maxPhotonEt = 0;
        
        for (int idx : kinematicCandidates) {
            if (phoEt->at(idx) > maxPhotonEt) {
                maxPhotonEt = phoEt->at(idx);
                selectedPhotonIndex = idx;
            }
        }
        
        // Apply photon ID criteria to the leading photon
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
                int genMatchedIndex = phoGenMatchedIndex->at(selectedPhotonIndex);
                if (genMatchedIndex < 0) {
                    selectedPhotonIndex = -1;
                } else {
                    // Check particle ID
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
                    
                    // Check mother particle ID if specified
                    if (selectedPhotonIndex >= 0) {
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
                    
                    std::string histDir = collection + "/" + centName;
                    if (outFile->cd(histDir.c_str())) {
                        TH1F* hJetPt = (TH1F*)gDirectory->Get("hJetPt");
                        if (hJetPt) hJetPt->Fill(selectedJetPts[collection]);
                        
                        TH1F* hJetEta = (TH1F*)gDirectory->Get("hJetEta");
                        if (hJetEta) hJetEta->Fill(selectedJetEtas[collection]);
                        
                        TH1F* hDeltaPhi = (TH1F*)gDirectory->Get("hDeltaPhi");
                        if (hDeltaPhi) hDeltaPhi->Fill(selectedJetDeltaPhis[collection]);
                        
                        TH1F* hXj = (TH1F*)gDirectory->Get("hXj");
                        if (hXj) hXj->Fill(selectedJetXjs[collection]);
                        
                        // Jet substructure histograms
                        TH1F* hJetMass = (TH1F*)gDirectory->Get("hJetMass");
                        if (hJetMass) hJetMass->Fill(selectedJetMasses[collection]);
                        
                        TH1F* hDynSplit = (TH1F*)gDirectory->Get("hDynSplit");
                        if (hDynSplit) hDynSplit->Fill(selectedJetDynSplits[collection]);
                        
                        TH1F* hDynKt = (TH1F*)gDirectory->Get("hDynKt");
                        if (hDynKt) hDynKt->Fill(selectedJetDynKts[collection]);
                        
                        TH1F* hDynZ = (TH1F*)gDirectory->Get("hDynZ");
                        if (hDynZ) hDynZ->Fill(selectedJetDynZs[collection]);
                        
                        TH1F* hGirth = (TH1F*)gDirectory->Get("hGirth");
                        if (hGirth) hGirth->Fill(selectedJetGirths[collection]);
                        
                        TH1F* hThrust = (TH1F*)gDirectory->Get("hThrust");
                        if (hThrust) hThrust->Fill(selectedJetThrusts[collection]);
                        
                        TH1F* hLHA = (TH1F*)gDirectory->Get("hLHA");
                        if (hLHA) hLHA->Fill(selectedJetLHAs[collection]);
                        
                        TH1F* hPtD = (TH1F*)gDirectory->Get("hPtD");
                        if (hPtD) hPtD->Fill(selectedJetPtDs[collection]);
                        
                        // Profiles and 2D histograms
                        TProfile* pDynSplitVsPt = (TProfile*)gDirectory->Get("pDynSplitVsPt");
                        if (pDynSplitVsPt) pDynSplitVsPt->Fill(selectedJetPts[collection], selectedJetDynSplits[collection]);
                        
                        TH2F* h2DynSplitVsPt = (TH2F*)gDirectory->Get("h2DynSplitVsPt");
                        if (h2DynSplitVsPt) h2DynSplitVsPt->Fill(selectedJetPts[collection], selectedJetDynSplits[collection]);
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
    
    // Print summary
    std::cout << "==================================================" << std::endl;
    std::cout << "Analysis Summary:" << std::endl;
    std::cout << "  Total events processed: " << nProcessed << std::endl;
    std::cout << "  Events with selected photon: " << nWithPhoton << std::endl;
    std::cout << "  Events with selected jet: " << nWithJet << std::endl;
    std::cout << "  Events passing all selections: " << nPassed << std::endl;
    std::cout << "==================================================" << std::endl;
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
        std::cout << "Adding single file: " << inputDir << std::endl;
        if (chain->AddFile(inputDir.c_str()) <= 0) {
            std::cerr << "Error: Failed to add file to chain: " << inputDir << std::endl;
            return false;
        }
        return true;
    }
    
    // Find all ROOT files in the directory
    std::string cmd = "find " + inputDir + " -name \"*.root\" -type f";
    std::cout << "Executing: " << cmd << std::endl;
    
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
        std::cout << "Test mode: limiting to " << maxFiles << " files out of " << files.size() << std::endl;
        files.resize(maxFiles);
    }
    
    // Add files to chain
    for (const auto& file : files) {
        std::cout << "Adding file: " << file << std::endl;
        if (chain->AddFile(file.c_str()) <= 0) {
            std::cerr << "Warning: Failed to add file to chain: " << file << std::endl;
            continue;
        }
    }
    
    if (chain->GetEntries() <= 0) {
        std::cerr << "Error: No entries in chain." << std::endl;
        return false;
    }
    
    std::cout << "Added " << files.size() << " files with " << chain->GetEntries() << " entries." << std::endl;
    return true;
}

/**
 * Create histograms for output
 */
void createHistograms(TFile* outFile, const std::vector<std::string>& jetCollections, 
                     const std::vector<float>& centralityBins) {
    if (!outFile) return;
    
    // Create histograms for each jet collection and centrality bin
    for (const auto& collection : jetCollections) {
        outFile->mkdir(collection.c_str());
        
        for (size_t i = 0; i < centralityBins.size() - 1; ++i) {
            std::string centName = "cent" + std::to_string(static_cast<int>(centralityBins[i])) + 
                                 "to" + std::to_string(static_cast<int>(centralityBins[i+1]));
            
            std::string histDir = collection + "/" + centName;
            outFile->mkdir(histDir.c_str());
            outFile->cd(histDir.c_str());
            
            // Basic histograms
            new TH1F("hJetPt", "Jet p_{T};p_{T} [GeV/c];Entries", 100, 0, 500);
            new TH1F("hJetEta", "Jet #eta;#eta;Entries", 50, -2.5, 2.5);
            new TH1F("hDeltaPhi", "#Delta#phi;#Delta#phi;Entries", 50, 0, M_PI);
            new TH1F("hXj", "x_{j} = p_{T}^{jet} / E_{T}^{#gamma};x_{j};Entries", 50, 0, 2.0);
            
            // Jet substructure histograms
            new TH1F("hJetMass", "Jet mass;m [GeV/c^{2}];Entries", 50, 0, 50);
            new TH1F("hDynSplit", "Dynamical groomed splitting scale;#sqrt{z#theta} [GeV/c];Entries", 50, 0, 50);
            new TH1F("hDynKt", "Dynamical groomed k_{T};k_{T} [GeV/c];Entries", 50, 0, 50);
            new TH1F("hDynZ", "Dynamical groomed z;z;Entries", 50, 0, 0.5);
            new TH1F("hGirth", "Jet girth;girth;Entries", 50, 0, 0.5);
            new TH1F("hThrust", "Jet thrust;thrust;Entries", 50, 0, 1.0);
            new TH1F("hLHA", "Jet LHA;LHA;Entries", 50, 0, 1.0);
            new TH1F("hPtD", "Jet p_{T}D;p_{T}D;Entries", 50, 0, 1.0);
            
            // Profiles and 2D histograms
            new TProfile("pDynSplitVsPt", "Dynamical groomed splitting scale vs p_{T};p_{T} [GeV/c];<#sqrt{z#theta}> [GeV/c]", 
                         10, 40, 240);
            new TH2F("h2DynSplitVsPt", "Dynamical groomed splitting scale vs p_{T};p_{T} [GeV/c];#sqrt{z#theta} [GeV/c]", 
                     10, 40, 240, 50, 0, 50);
        }
    }
    
    outFile->cd();
}

/**
 * Setup output tree branches
 */
void setupOutputTree(TTree* outTree, [[maybe_unused]] const std::vector<std::string>& jetCollections) {
    if (!outTree) return;
    
    // Basic tree structure is set up in processEvents
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
