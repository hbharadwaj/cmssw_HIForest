// OPTIMIZED RooUnfold-based Jet Substructure Unfolding
// Refactored for maintainability and performance
// Author: CMS Heavy Ion Team - Optimized Version
// Date: June 2025

// RooUnfold version compatibility:
// This macro is compatible with RooUnfold v2.0.0 and v3.0.0 as of June 2025.
// If you use v3.0.0 and encounter API changes, define ROOUNFOLD_VERSION_3 and update code as needed.
// See include/OptimizedUnfolder.h for details on version-guarding code.

#include <memory>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <set>
#include <TFile.h>
#include <TTree.h>
#include <TEnv.h>
#include <TSystem.h>
#include <TStopwatch.h>
#include <RooUnfold.h>
#include <RooUnfoldResponse.h>
#include <RooUnfoldBayes.h>
#include <RooUnfoldSvd.h>
#include <RooUnfoldBinByBin.h>
#include <RooUnfoldInvert.h>
#include "./include/UnfoldHelpers.h"
#include "./include/UnfoldConfig.h"
#include "./include/HistogramManager.h"
#include "./include/ResponseMatrixPlotter.h"
#include "./include/OptimizedUnfolder.h"
#include <random>

// ============================================================================
// MAIN FUNCTION
// ============================================================================

void RooUnfoldOptimized(const char* configFile = "../configs/UnfoldJetSub_xj_test.config") {
    TStopwatch timer;
    timer.Start();
    
    // Print RooUnfold version info for clarity
    #ifdef ROOUNFOLD_VERSION
        log(LOG_INFO, std::string("RooUnfold library version (compile-time): ") + ROOUNFOLD_VERSION);
    #else
        log(LOG_INFO, "RooUnfold library version: (not defined at compile time)");
    #endif
    
    log(LOG_INFO, "=== Optimized RooUnfold Jet Substructure Unfolding ===");
    try {
        // Initialize ConfigParser
        ConfigParser parser(configFile);
        
        // Validate configuration
        if (!parser.validateConfiguration()) {
            log(LOG_ERROR, "Configuration validation failed. Exiting.");
            return;
        }
        
        // Set global verbosity from config
        gVerbosity = parser.getVerbosity();;
        log(LOG_INFO, "Setting global verbosity to: " + std::to_string(gVerbosity));
        
        // Get unfolding sets to process
        auto unfoldSets = parser.getUnfoldingSets();
        if (unfoldSets.empty()) {
            log(LOG_WARNING, "No unfolding sets found in configuration. Exiting.");
            return;
        }
        
        // Create output directory and file - NO fallback to current directory
        std::string outputDir = parser.getOutputDir();
        std::string outputPrefix = parser.getOutputPrefix();
        
        // Ensure output directory exists (create if possible)
        if (gSystem->mkdir(outputDir.c_str(), kTRUE) != 0) {
            // Try to create parent directories step by step
            log(LOG_WARNING, "Initial directory creation failed, trying step-by-step creation");
            std::string parentDir = outputDir.substr(0, outputDir.find_last_of('/'));
            if (!parentDir.empty() && gSystem->mkdir(parentDir.c_str(), kTRUE) == 0) {
                gSystem->mkdir(outputDir.c_str(), kTRUE);
            }
        }
        
        // Verify directory is accessible
        if (gSystem->AccessPathName(outputDir.c_str())) {
            log(LOG_ERROR, "Cannot access output directory: " + outputDir);
            log(LOG_ERROR, "Please ensure the directory exists and is writable");
            return;
        }
        
        log(LOG_INFO, "Using output directory: " + outputDir);
        
        std::string outputPath = outputDir;
        if (outputPath.back() != '/') outputPath += "/";
        outputPath += outputPrefix + "_optimized_results.root";
        auto outFile = std::unique_ptr<TFile>(TFile::Open(outputPath.c_str(), "RECREATE"));
        if (!outFile || outFile->IsZombie()) {
            log(LOG_ERROR, "Failed to create output file: " + outputPath);
            log(LOG_ERROR, "Check directory permissions and disk space");
            return;
        }
        
        log(LOG_INFO, "Output file created: " + outputPath);
        
        log(LOG_INFO, "Processing " + std::to_string(unfoldSets.size()) + " unfolding sets...");
        
        // Map to track if input histograms have been saved for each set
        std::set<std::string> savedInputs;
        int n1D = 0, n2D = 0, n3D = 0, nTotal = 0;
        // Process each unfolding set
        for (const auto& setName : unfoldSets) {
            log(LOG_INFO, "Processing unfolding set: " + setName);
            // Parse configuration for this set
            auto cfg = parser.parseUnfoldingSet(setName);
            if (!cfg.isValid()) {
                log(LOG_WARNING, "Invalid configuration for " + setName + ". Skipping.");
                continue;
            }
            if (cfg.dimension == 1) n1D++;
            else if (cfg.dimension == 2) n2D++;
            else if (cfg.dimension == 3) n3D++;
            nTotal++;
            TestConfig tcfg = parser.getTestConfig(setName);
            for (auto test : tcfg.tests) {
                std::string testLabel;
                if (test == UnfoldTestType::Nominal) testLabel = "Nominal";
                else if (test == UnfoldTestType::Closure) testLabel = "Closure";
                else if (test == UnfoldTestType::Split) testLabel = "Split";
                else if (test == UnfoldTestType::Bottomline) testLabel = "Bottomline";
                log(LOG_INFO, "Running test: " + testLabel + " for set: " + setName);
                // --- FIX: Ensure correct nested directory structure ---
                TDirectory* setDir = outFile->GetDirectory(setName.c_str());
                if (!setDir) setDir = outFile->mkdir(setName.c_str());
                setDir->cd();
                TDirectory* testDir = setDir->GetDirectory(testLabel.c_str());
                if (!testDir) testDir = setDir->mkdir(testLabel.c_str());
                testDir->cd();
                log(LOG_DEBUG, "Created/entered directory: " + std::string(testDir->GetPath()));
                OptimizedUnfolder unfolder(cfg);
                log(LOG_DEBUG, "Instantiated OptimizedUnfolder for test: " + testLabel);
                if (test == UnfoldTestType::Nominal) {
                    log(LOG_DEBUG, "Opening data file: " + cfg.dataFile);
                    TFile dataFile(cfg.dataFile.c_str());
                    log(LOG_DEBUG, "Opening MC file: " + cfg.mcFile);
                    TFile mcFile(cfg.mcFile.c_str());
                    std::string treeName = parser.getConfigValue("default.TreeName", "gammaJetTree");
                    log(LOG_DEBUG, "Getting data tree: " + treeName);
                    TTree* dataTree = (TTree*)dataFile.Get(treeName.c_str());
                    log(LOG_DEBUG, "Getting MC tree: " + treeName);
                    TTree* mcTree = (TTree*)mcFile.Get(treeName.c_str());
                    if (!dataTree || !mcTree) {
                        log(LOG_ERROR, "Could not find TTree '" + treeName + "' in data or MC file for set " + setName);
                        continue;
                    }
                    log(LOG_DEBUG, "Data tree entries: " + std::to_string(dataTree->GetEntries()));
                    log(LOG_DEBUG, "MC tree entries: " + std::to_string(mcTree->GetEntries()));
                    log(LOG_DEBUG, "Calling fillFromTrees");
                    unfolder.fillFromTrees(dataTree, mcTree);
                    std::string details;
                    TH1D* effHist = nullptr;
                    TMatrixD* covMatrix = nullptr;
                    log(LOG_TRACE, "Calling checkUnfoldability");
                    checkUnfoldability(unfolder.getHistograms(), cfg, details, effHist, covMatrix, testDir);
                    log(LOG_TRACE, "Calling performUnfolding");
                    unfolder.performUnfolding();
                    log(LOG_TRACE, "Calling saveResults");
                    unfolder.saveResults(testDir, testLabel, details, effHist, covMatrix);
                    // Draw and store the response matrix canvas in the output ROOT file
                    ResponseMatrixPlotter::plotGeneralizedResponseMatrix(
                        setName,
                        unfolder.getHistograms().getResponse(),
                        cfg.measuredVars,
                        cfg.measuredBins,
                        cfg.truthVars,
                        cfg.truthBins,
                        "Response Matrix",
                        outputDir, // for PNG output
                        testDir    // for ROOT file output
                    );
                    log(LOG_DEBUG, "Finished saveResults");
                    // Prevent use-after-free: null pointers after use
                    dataTree = nullptr;
                    mcTree = nullptr;
                    // dataFile and mcFile will go out of scope and close here
                } else if (test == UnfoldTestType::Closure) {
                    log(LOG_DEBUG, "Opening MC file: " + cfg.mcFile);
                    TFile mcFile(cfg.mcFile.c_str());
                    std::string treeName = parser.getConfigValue("default.TreeName", "gammaJetTree");
                    log(LOG_DEBUG, "Getting MC tree: " + treeName);
                    TTree* mcTree = (TTree*)mcFile.Get(treeName.c_str());
                    if (!mcTree) {
                        log(LOG_ERROR, "Could not find TTree '" + treeName + "' in MC file for set " + setName);
                        continue;
                    }
                    log(LOG_DEBUG, "MC tree entries: " + std::to_string(mcTree->GetEntries()));
                    log(LOG_DEBUG, "Calling fillFromTrees");
                    unfolder.fillFromTrees(mcTree, mcTree);
                    log(LOG_TRACE, "Calling performUnfolding");
                    unfolder.performUnfolding();
                    std::string details;
                    TH1D* effHist = nullptr;
                    TMatrixD* covMatrix = nullptr;
                    log(LOG_TRACE, "Calling checkUnfoldability");
                    checkUnfoldability(unfolder.getHistograms(), cfg, details, effHist, covMatrix, testDir);
                    log(LOG_TRACE, "Calling saveResults");
                    unfolder.saveResults(testDir, testLabel, details, effHist, covMatrix);
                    log(LOG_DEBUG, "Finished saveResults");
                    mcTree = nullptr;
                    // mcFile will go out of scope and close here
                } else if (test == UnfoldTestType::Split) {
                    log(LOG_DEBUG, "Opening MC file: " + cfg.mcFile);
                    TFile mcFile(cfg.mcFile.c_str());
                    std::string treeName = parser.getConfigValue("default.TreeName", "gammaJetTree");
                    log(LOG_DEBUG, "Getting MC tree: " + treeName);
                    TTree* mcTree = (TTree*)mcFile.Get(treeName.c_str());
                    if (!mcTree) {
                        log(LOG_ERROR, "Could not find TTree '" + treeName + "' in MC file for set " + setName);
                        continue;
                    }
                    log(LOG_DEBUG, "MC tree entries: " + std::to_string(mcTree->GetEntries()));
                    Long64_t nEntries = mcTree->GetEntries();
                    log(LOG_TRACE, "Cloning trees for split test");
                    auto* pseudoDataTree = mcTree->CloneTree(0);
                    auto* responseTree = mcTree->CloneTree(0);
                    std::mt19937 rng(42);
                    std::uniform_real_distribution<> dist(0.0, 1.0);
                    for (Long64_t i = 0; i < nEntries; ++i) {
                        mcTree->GetEntry(i);
                        if (dist(rng) < tcfg.splitFraction) pseudoDataTree->Fill();
                        else responseTree->Fill();
                    }
                    log(LOG_DEBUG, "Calling fillFromTrees");
                    unfolder.fillFromTrees(pseudoDataTree, responseTree);
                    log(LOG_DEBUG, "Calling performUnfolding");
                    unfolder.performUnfolding();
                    std::string details;
                    TH1D* effHist = nullptr;
                    TMatrixD* covMatrix = nullptr;
                    log(LOG_TRACE, "Calling checkUnfoldability");
                    checkUnfoldability(unfolder.getHistograms(), cfg, details, effHist, covMatrix, testDir);
                    log(LOG_TRACE, "Calling saveResults");
                    unfolder.saveResults(testDir, testLabel, details, effHist, covMatrix);
                    log(LOG_DEBUG, "Finished saveResults");
                    mcTree = nullptr;
                    pseudoDataTree = nullptr;
                    responseTree = nullptr;
                    // mcFile will go out of scope and close here
                } else if (test == UnfoldTestType::Bottomline) {
                    log(LOG_DEBUG, "Opening data file: " + cfg.dataFile);
                    TFile dataFile(cfg.dataFile.c_str());
                    log(LOG_DEBUG, "Opening MC file: " + cfg.mcFile);
                    TFile mcFile(cfg.mcFile.c_str());
                    std::string treeName = parser.getConfigValue("default.TreeName", "gammaJetTree");
                    log(LOG_DEBUG, "Getting data tree: " + treeName);
                    TTree* dataTree = (TTree*)dataFile.Get(treeName.c_str());
                    log(LOG_DEBUG, "Getting MC tree: " + treeName);
                    TTree* mcTree = (TTree*)mcFile.Get(treeName.c_str());
                    if (!dataTree || !mcTree) {
                        log(LOG_ERROR, "Could not find TTree '" + treeName + "' in data or MC file for set " + setName);
                        continue;
                    }
                    log(LOG_DEBUG, "Data tree entries: " + std::to_string(dataTree->GetEntries()));
                    log(LOG_DEBUG, "MC tree entries: " + std::to_string(mcTree->GetEntries()));
                    log(LOG_DEBUG, "Calling fillFromTrees");
                    unfolder.fillFromTrees(dataTree, mcTree);
                    log(LOG_DEBUG, "Calling performUnfolding");
                    unfolder.performUnfolding();
                    std::string details;
                    TH1D* effHist = nullptr;
                    TMatrixD* covMatrix = nullptr;
                    log(LOG_TRACE, "Calling checkUnfoldability");
                    checkUnfoldability(unfolder.getHistograms(), cfg, details, effHist, covMatrix, testDir);
                    log(LOG_TRACE, "Calling saveResults");
                    unfolder.saveResults(testDir, testLabel, details, effHist, covMatrix);
                    log(LOG_DEBUG, "Finished saveResults");
                    dataTree = nullptr;
                    mcTree = nullptr;
                    // dataFile and mcFile will go out of scope and close here
                } else {
                    log(LOG_WARNING, "Unknown test type encountered for " + setName);
                }
                
                // Save input histograms only once per set (handled in saveResults)
            } 
            
        }
        outFile->Close();
        log(LOG_INFO, "Results saved to: " + outputPath);
        log(LOG_INFO, "Summary: Processed " + std::to_string(nTotal) + " sets: " +
            std::to_string(n1D) + " x 1D, " + std::to_string(n2D) + " x 2D, " + std::to_string(n3D) + " x 3D.");
    } catch (const std::exception& e) {
        log(LOG_ERROR, "Exception caught: " + std::string(e.what()));
        return;
    }
    timer.Stop();
    log(LOG_INFO, "=== Completed in " + std::to_string(timer.RealTime()) + " seconds ===");
}
