#ifndef OPTIMIZED_UNFOLDER_H
#define OPTIMIZED_UNFOLDER_H

#include "UnfoldHelpers.h"
#include "UnfoldConfig.h"
#include <memory>
#include <vector>
#include <sstream>
#include <iostream>
#include <TTree.h>
#include <TDirectory.h>
#include <RooUnfold.h>
#include <RooUnfoldResponse.h>
#include <RooUnfoldBayes.h>
#include <RooUnfoldSvd.h>
#include <RooUnfoldBinByBin.h>
#include <RooUnfoldInvert.h>
#include <TMath.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TLegend.h>
#include <TLine.h>
#include <TLatex.h>
#include <TVectorD.h>
#include <TObjString.h>
#include <numeric>
#include <algorithm>
#include <iomanip>

// ============================================================================
// STREAMLINED UNFOLDER CLASS
// ============================================================================

class OptimizedUnfolder {
private:
    UnfoldConfig config;
    HistogramManager histManager;
    std::unique_ptr<RooUnfoldResponse> response;
    std::unique_ptr<RooUnfold> unfoldAlgorithm;
    std::unique_ptr<TH1> unfoldedHist;
    std::unique_ptr<TMatrixD> covMatrix; // Unfolded covariance matrix (Ereco)
    std::unique_ptr<TMatrixD> measuredCovMatrix; // Measured covariance matrix (GetMeasuredCov)
    std::unique_ptr<TMatrixD> probMatrix; // Store probability matrix from response
    std::unique_ptr<TH1> purityHist; // Store purity histogram after unfolding
    std::unique_ptr<TH1> efficiencyHist; // Store efficiency histogram after unfolding
    std::unique_ptr<TH1> purityCorrectedDataHist; // Store purity-corrected data histogram
    std::string unfoldingLog; // Store captured RooUnfold output
    
    // Storage for Bayesian iterations
    std::vector<std::unique_ptr<TH1>> bayesianIterations; // Store all iterations for Bayes method
    int maxIterations; // Maximum number of iterations to store
    
    // Numerator/denominator histograms for purity and efficiency (template logic)
    std::unique_ptr<TH1> purityNum, purityDen, effNum, effDen;
    std::unique_ptr<TH2> probabilityMatrixHist2D; // For saving the probability matrix as TH2D
    
    // Bottomline test results storage
    struct BottomlineResults {
        double chi2Smeared = -1.0;
        double pValueSmeared = -1.0;
        int ndfSmeared = 0;
        double chi2Unfolded = -1.0;
        double pValueUnfolded = -1.0;
        int ndfUnfolded = 0;
        int optimalIteration = -1;
        std::string summary = "";
        std::unique_ptr<TH1> forwardFoldedModel = nullptr;
        
        // Per-iteration results for Bayesian unfolding
        std::vector<double> chi2SmearedPerIteration;
        std::vector<double> pValueSmearedPerIteration;
        std::vector<double> chi2UnfoldedPerIteration;
        std::vector<double> pValueUnfoldedPerIteration;
    } bottomlineResults;

public:
    OptimizedUnfolder(const UnfoldConfig& cfg) : config(cfg), histManager(cfg), maxIterations(cfg.maxIterationsToStore) {
        log(LOG_DEBUG, "Created unfolder for " + std::to_string(cfg.dimension) + "D case");
        log(LOG_DEBUG, "Will store up to " + std::to_string(maxIterations) + " Bayesian iterations");
        initializePurityEfficiencyHistograms();
        bayesianIterations.clear(); // Initialize empty
    }
    
    void fillFromTrees(TTree* dataTree, TTree* mcTree) {
        log(LOG_DEBUG, "Filling histograms from trees");
        fillTree(dataTree, true);   // isData = true
        fillTree(mcTree, false);    // isData = false
        calculatePurityAndEfficiency();
    }
    
    void performUnfolding() {
        log(LOG_DEBUG, "Setting up unfolding with method: " + config.method);
        
        // Validate flattening consistency before proceeding
        if (config.dimension > 1) {
            validateFlatteningConsistency();
        }
        
        // Create flattened histograms for RooUnfold
        TH1D* h_data_flat = nullptr;
        TH1D* h_mc_meas_flat = nullptr;
        TH1D* h_mc_truth_flat = nullptr;
        
        if (config.dimension == 1) {
            h_data_flat = (TH1D*)histManager.getData();
            h_mc_meas_flat = (TH1D*)histManager.getMeasuredMC();
            h_mc_truth_flat = (TH1D*)histManager.getTruthMC();
        } else {
            // For multi-dimensional, we need to create flattened versions
            h_data_flat = createFlattenedHistogram(histManager.getData(), "h_data_flat");
            h_mc_meas_flat = createFlattenedHistogram(histManager.getMeasuredMC(), "h_mc_meas_flat");
            h_mc_truth_flat = createFlattenedHistogram(histManager.getTruthMC(), "h_mc_truth_flat");
        }
        
        // Apply purity correction to data histogram before unfolding
        TH1D* h_data_purity_corrected = nullptr;
        if (purityHist.get()) {
            if (config.dimension == 1) {
                h_data_purity_corrected = (TH1D*)h_data_flat->Clone("h_data_purity_corrected");
            } else {
                // For multi-dimensional, create flattened purity histogram
                TH1D* h_purity_flat = createFlattenedHistogram(purityHist.get(), "h_purity_flat");
                h_data_purity_corrected = (TH1D*)h_data_flat->Clone("h_data_purity_corrected");
                h_data_purity_corrected->Multiply(h_purity_flat);
                delete h_purity_flat;
            }
            if (config.dimension == 1) {
                h_data_purity_corrected->Multiply((TH1D*)purityHist.get());
            }
            h_data_purity_corrected->SetDirectory(0);
            log(LOG_DEBUG, "Applied purity correction to data histogram");
        } else {
            // If no purity correction available, use original data
            h_data_purity_corrected = (TH1D*)h_data_flat->Clone("h_data_no_purity_correction");
            h_data_purity_corrected->SetDirectory(0);
            log(LOG_WARNING, "No purity histogram available, proceeding without purity correction");
        }
        
        // Setup RooUnfold response
        response = std::make_unique<RooUnfoldResponse>(h_mc_meas_flat, h_mc_truth_flat, histManager.getResponse());
        
        // Get probability matrix from response
        const TMatrixD& prob = response->Mresponse();
        probMatrix = std::make_unique<TMatrixD>(prob);
        
        // Choose unfolding algorithm (use purity-corrected data)
        if (config.method == "Bayes") {
            // For Bayesian unfolding, we need to run multiple iterations and store each one
            bayesianIterations.clear();
            
            // Create RooUnfoldBayes with final iteration count
            unfoldAlgorithm = std::make_unique<RooUnfoldBayes>(response.get(), h_data_purity_corrected, config.iterations);
            
            // Store iterations from 1 to maxIterations or config.iterations, whichever is smaller
            int iterationsToStore = std::min(maxIterations, config.iterations);
            
            for (int iter = 1; iter <= iterationsToStore; ++iter) {
                // Create a temporary unfolder for this iteration
                RooUnfoldBayes tempBayes(response.get(), h_data_purity_corrected, iter);
                TH1* h_iter = tempBayes.Hreco();
                if (h_iter) {
                    if (config.dimension > 1) {
                        // Convert to multi-dimensional first
                        auto* flattened = dynamic_cast<TH1D*>(h_iter);
                        if (flattened) {
                            auto unfoldedMultiDim = convertFlattenedToMultiDim(flattened, ("h_unfolded_iter" + std::to_string(iter)).c_str(), false);
                            if (efficiencyHist.get()) {
                                unfoldedMultiDim->Divide(efficiencyHist.get());
                            }
                            bayesianIterations.push_back(std::move(unfoldedMultiDim));
                        } else {
                            log(LOG_ERROR, "Expected TH1D for multi-dimensional unfolded result at iteration " + std::to_string(iter));
                            bayesianIterations.emplace_back((TH1*)h_iter->Clone(("h_unfolded_iter" + std::to_string(iter)).c_str()));
                        }
                    } else {
                        // 1D case: apply efficiency correction directly
                        TH1* h_eff_corr = nullptr;
                        if (efficiencyHist.get()) {
                            h_eff_corr = (TH1*)h_iter->Clone(("h_unfolded_iter" + std::to_string(iter) + "_effcorr").c_str());
                            h_eff_corr->SetDirectory(0);
                            h_eff_corr->Divide(efficiencyHist.get());
                        } else {
                            h_eff_corr = (TH1*)h_iter->Clone(("h_unfolded_iter" + std::to_string(iter)).c_str());
                            h_eff_corr->SetDirectory(0);
                        }
                        bayesianIterations.emplace_back(h_eff_corr);
                    }
                    log(LOG_DEBUG, "Stored Bayesian iteration (efficiency corrected) " + std::to_string(iter));
                }
            }
        } else if (config.method == "SVD") {
            unfoldAlgorithm = std::make_unique<RooUnfoldSvd>(response.get(), h_data_purity_corrected, config.iterations);
        } else if (config.method == "BinByBin") {
            unfoldAlgorithm = std::make_unique<RooUnfoldBinByBin>(response.get(), h_data_purity_corrected);
        } else {
            unfoldAlgorithm = std::make_unique<RooUnfoldInvert>(response.get(), h_data_purity_corrected);
        }
        // Store the unfolded histogram (clone to avoid double-free)
        if (unfoldAlgorithm) {
            TH1* h = unfoldAlgorithm->Hreco();
            if (h) {
                auto* cloned = (TH1*)h->Clone(("h_unfolded_" + config.method).c_str());
                cloned->SetDirectory(0);
                
                // For multi-dimensional cases, convert flattened result back to original dimensions
                if (config.dimension > 1) {
                    // RooUnfold always returns TH1D for flattened results
                    auto* flattened = dynamic_cast<TH1D*>(cloned);
                    if (flattened) {
                        unfoldedHist = convertFlattenedToMultiDim(flattened, "h_unfolded_" + config.method, false);
                        delete cloned; // Clean up the flattened version
                    } else {
                        log(LOG_ERROR, "Expected TH1D for multi-dimensional unfolded result");
                        unfoldedHist.reset(cloned);
                    }
                } else {
                    unfoldedHist.reset(cloned);
                }
            } else {
                unfoldedHist.reset();
            }
        }
        
        // Store the purity-corrected data histogram for later saving
        if (h_data_purity_corrected) {
            if (config.dimension > 1) {
                // Convert flattened back to multi-dimensional for storage using measured binning
                purityCorrectedDataHist = convertFlattenedToMultiDim(h_data_purity_corrected, "h_data_purity_corrected", true);
                delete h_data_purity_corrected;
            } else {
                purityCorrectedDataHist.reset((TH1*)h_data_purity_corrected->Clone("h_data_purity_corrected"));
                purityCorrectedDataHist->SetDirectory(0);
                delete h_data_purity_corrected;
            }
        }
        
        // Get covariance matrices from unfolding algorithm
        if (unfoldAlgorithm) {
            // Unfolded covariance matrix (truth space)
            const TMatrixD& cov = unfoldAlgorithm->Ereco();
            covMatrix = std::make_unique<TMatrixD>(cov);
            // Measured covariance matrix (measured space)
            const TMatrixD& measuredCov = unfoldAlgorithm->GetMeasuredCov();
            measuredCovMatrix = std::make_unique<TMatrixD>(measuredCov);
        }
        
        log(LOG_INFO, "Unfolding with " + config.method + " method completed");
        
        // Perform bottomline test if enabled
        performBottomlineTest();
    }
    
    void saveResults(TDirectory* testDir, const std::string& testLabel, const std::string& details, TH1D* effHist, TMatrixD* externalCovMatrix) {
        testDir->cd();
        // Helper lambda to clone and write a histogram into the subdirectory
        auto writeToDir = [testDir](TH1* h, const std::string& name) {
            if (!h) return;
            auto* clone = (TH1*)h->Clone(name.c_str());
            clone->SetDirectory(testDir);
            clone->Write();
            delete clone;
        };
        writeToDir(histManager.getData(), "h_measured_data_" + testLabel);
        log(LOG_DEBUG, "Wrote h_measured_data_" + testLabel + " to " + testDir->GetPath());
        
        // Write purity-corrected data histogram
        if (purityCorrectedDataHist) {
            writeToDir(purityCorrectedDataHist.get(), "h_data_purity_corrected_" + testLabel);
            log(LOG_DEBUG, "Wrote h_data_purity_corrected_" + testLabel + " to " + testDir->GetPath());
        }
        
        writeToDir(histManager.getMeasuredMC(), "h_measured_mc_" + testLabel);
        log(LOG_DEBUG, "Wrote h_measured_mc_" + testLabel + " to " + testDir->GetPath());
        writeToDir(histManager.getTruthMC(), "h_truth_mc_" + testLabel);
        log(LOG_DEBUG, "Wrote h_truth_mc_" + testLabel + " to " + testDir->GetPath());
        if (histManager.getResponse()) {
            auto* clone = (TH2D*)histManager.getResponse()->Clone(("h_response_" + testLabel).c_str());
            clone->SetDirectory(testDir);
            clone->Write();
            delete clone;
            log(LOG_DEBUG, "Wrote h_response_" + testLabel + " to " + testDir->GetPath());
        }
        if (unfoldedHist) {
            auto* clone = (TH1*)unfoldedHist->Clone(("h_unfolded_" + testLabel).c_str());
            clone->SetDirectory(testDir);
            clone->Write();
            delete clone;
            log(LOG_DEBUG, "Wrote h_unfolded_" + testLabel + " to " + testDir->GetPath());
            
            // Create efficiency-corrected unfolded histogram
            if (efficiencyHist.get()) {
                auto* corrected = (TH1*)unfoldedHist->Clone(("h_unfolded_corrected_" + testLabel).c_str());
                corrected->SetDirectory(testDir);
                corrected->SetTitle("Efficiency Corrected Unfolded Distribution");
                corrected->Divide(efficiencyHist.get());
                corrected->Write();
                delete corrected;
                log(LOG_DEBUG, "Wrote h_unfolded_corrected_" + testLabel + " to " + testDir->GetPath());
            }
        }
        if (purityHist.get() && efficiencyHist.get()) {
            // Write purity and efficiency histograms
            // purityHist.get()->SetDirectory(testDir);
            writeToDir(purityHist.get(), "h_purity_" + testLabel);
            log(LOG_DEBUG, "Wrote h_purity_" + testLabel + " to " + testDir->GetPath());
            writeToDir(efficiencyHist.get(), "h_efficiency_" + testLabel);
            log(LOG_DEBUG, "Wrote h_efficiency_" + testLabel + " to " + testDir->GetPath());
            
            // Write numerator/denominator histograms for debugging
            writeToDir(purityNum.get(), "h_purity_num_" + testLabel);
            writeToDir(purityDen.get(), "h_purity_den_" + testLabel);
            writeToDir(effNum.get(), "h_eff_num_" + testLabel);
            writeToDir(effDen.get(), "h_eff_den_" + testLabel);
        }
        
        
        // Write probability matrix from response
        if (probMatrix) {
            // Create TH2D histogram directly from the matrix
            TH2D probability_hist2D(*probMatrix);
            probability_hist2D.SetName(("probability_hist_" + testLabel).c_str());
            probability_hist2D.SetTitle("Probability Matrix;True Bin Number;Reco Bin Number");
            probability_hist2D.SetDirectory(testDir);
            probability_hist2D.Write();
            log(LOG_DEBUG, "Wrote probability matrix histogram for " + testLabel + " to " + testDir->GetPath());
            
            // Plot probability matrix using the new plotter
            ResponseMatrixPlotter::plotProbabilityMatrix(
                config.name + "_" + testLabel,
                &probability_hist2D,
                config.truthVars,
                config.truthBins,
                config.measuredVars,
                config.measuredBins,
                "Probability Matrix",
                "", // outputDir not needed since we're saving to ROOT file
                testDir
            );
        }
        
        // Write covariance matrix (prioritize internal one)
        if (covMatrix) {
            // Create TH2D histogram directly from the matrix
            TH2D mat_covariance_hist2D(*covMatrix);
            mat_covariance_hist2D.SetName(("covariance_hist_" + testLabel).c_str());
            mat_covariance_hist2D.SetTitle("Covariance Matrix;Bin Number;Bin Number");
            mat_covariance_hist2D.SetDirectory(testDir);
            mat_covariance_hist2D.Write();
            
            // Plot covariance matrix using the new plotter
            ResponseMatrixPlotter::plotCovarianceMatrix(
                config.name + "_" + testLabel,
                &mat_covariance_hist2D,
                config.truthVars,
                config.truthBins,
                "Covariance Matrix",
                "", // outputDir not needed since we're saving to ROOT file
                testDir
            );
            
            // Also write the matrix in its original format
            // covMatrix->Write(("covariance_matrix_" + testLabel).c_str());
            log(LOG_DEBUG, "Wrote internal covariance matrix histogram for " + testLabel + " to " + testDir->GetPath());
        } else if (externalCovMatrix) {
            // Create TH2D histogram directly from the external matrix
            TH2D mat_covariance_hist2D(*externalCovMatrix);
            mat_covariance_hist2D.SetName(("covariance_hist_" + testLabel).c_str());
            mat_covariance_hist2D.SetTitle("Covariance Matrix");
            mat_covariance_hist2D.SetDirectory(testDir);
            mat_covariance_hist2D.Write();
            
            // Plot external covariance matrix using the new plotter
            ResponseMatrixPlotter::plotCovarianceMatrix(
                config.name + "_" + testLabel,
                &mat_covariance_hist2D,
                config.truthVars,
                config.truthBins,
                "Covariance Matrix",
                "", // outputDir not needed since we're saving to ROOT file
                testDir
            );
            
            // Also write the matrix in its original format
            // externalCovMatrix->Write(("covariance_matrix_" + testLabel).c_str());
            log(LOG_DEBUG, "Wrote external covariance matrix and histogram for " + testLabel + " to " + testDir->GetPath());
        }
        
        // Plot response matrix if available
        if (histManager.getResponse()) {
            ResponseMatrixPlotter::plotGeneralizedResponseMatrix(
                config.name + "_" + testLabel,
                histManager.getResponse(),
                config.measuredVars,
                config.measuredBins,
                config.truthVars,
                config.truthBins,
                "Response Matrix",
                "", // outputDir not needed since we're saving to ROOT file
                testDir
            );
        }
        TObjString checkInfo(details.c_str());
        checkInfo.Write(("unfoldability_check_" + testLabel).c_str());
        log(LOG_DEBUG, "Wrote unfoldability_check_" + testLabel + " to " + testDir->GetPath());
        
        // Save Bayesian iterations if available (for all tests)
        if (!bayesianIterations.empty() && config.method == "Bayes") {
            saveBayesianIterations(testDir, testLabel);
        }
        
        // Save bottomline test results if available
        saveBottomlineResults(testDir, testLabel);
    }
    
    void saveBayesianIterations(TDirectory* testDir, const std::string& testLabel) {
        // Create subdirectory for Bayesian iterations
        TDirectory* iterDir = testDir->GetDirectory("BayesianIterations");
        if (!iterDir) iterDir = testDir->mkdir("BayesianIterations");
        iterDir->cd();
        
        log(LOG_INFO, "Saving " + std::to_string(bayesianIterations.size()) + " Bayesian iterations");
        
        for (size_t i = 0; i < bayesianIterations.size(); ++i) {
            if (bayesianIterations[i]) {
                int iterNumber = i + 1; // iterations start from 1
                std::string iterName = "h_unfolded_iter" + std::to_string(iterNumber);
                
                auto* clone = (TH1*)bayesianIterations[i]->Clone(iterName.c_str());
                clone->SetDirectory(iterDir);
                clone->SetTitle(("Unfolded Distribution - Iteration " + std::to_string(iterNumber)).c_str());
                clone->Write();
                delete clone;
                
                log(LOG_DEBUG, "Wrote " + iterName + " to BayesianIterations subdirectory");
            }
        }
        
        // Return to parent directory
        testDir->cd();
    }
    
    // Bottomline test methods
    void performBottomlineTest();
    void saveBottomlineResults(TDirectory* testDir, const std::string& testLabel);
    const BottomlineResults& getBottomlineResults() const { return bottomlineResults; }
    void enableBottomlineTest(bool enable = true, double pValueThreshold = 0.95) {
        config.enableBottomlineTest = enable;
        config.pValueThreshold = pValueThreshold;
        log(LOG_DEBUG, "Bottomline test " + std::string(enable ? "enabled" : "disabled") + 
            " with p-value threshold: " + std::to_string(pValueThreshold));
    }
    
    HistogramManager& getHistograms() { return histManager; }
    const UnfoldConfig& getConfig() const { return config; }
    
private:
    void initializePurityEfficiencyHistograms() {
        log(LOG_DEBUG, "Initializing purity and efficiency histograms for " + std::to_string(config.dimension) + "D case");
        
        if (config.dimension == 1) {
            // Purity histograms (detector level binning)
            purityNum = std::make_unique<TH1D>("purityNum", "Purity Numerator", 
                                              config.measuredBins[0].size()-1, config.measuredBins[0].data());
            purityDen = std::make_unique<TH1D>("purityDen", "Purity Denominator", 
                                              config.measuredBins[0].size()-1, config.measuredBins[0].data());
            // Efficiency histograms (truth level binning)
            effNum = std::make_unique<TH1D>("effNum", "Efficiency Numerator", 
                                           config.truthBins[0].size()-1, config.truthBins[0].data());
            effDen = std::make_unique<TH1D>("effDen", "Efficiency Denominator", 
                                           config.truthBins[0].size()-1, config.truthBins[0].data());
        } else if (config.dimension == 2) {
            // Purity histograms (detector level binning)
            purityNum = std::make_unique<TH2D>("purityNum", "Purity Numerator", 
                                              config.measuredBins[0].size()-1, config.measuredBins[0].data(),
                                              config.measuredBins[1].size()-1, config.measuredBins[1].data());
            purityDen = std::make_unique<TH2D>("purityDen", "Purity Denominator", 
                                              config.measuredBins[0].size()-1, config.measuredBins[0].data(),
                                              config.measuredBins[1].size()-1, config.measuredBins[1].data());
            // Efficiency histograms (truth level binning)
            effNum = std::make_unique<TH2D>("effNum", "Efficiency Numerator", 
                                           config.truthBins[0].size()-1, config.truthBins[0].data(),
                                           config.truthBins[1].size()-1, config.truthBins[1].data());
            effDen = std::make_unique<TH2D>("effDen", "Efficiency Denominator", 
                                           config.truthBins[0].size()-1, config.truthBins[0].data(),
                                           config.truthBins[1].size()-1, config.truthBins[1].data());
        } else if (config.dimension == 3) {
            // Purity histograms (detector level binning)
            purityNum = std::make_unique<TH3D>("purityNum", "Purity Numerator", 
                                              config.measuredBins[0].size()-1, config.measuredBins[0].data(),
                                              config.measuredBins[1].size()-1, config.measuredBins[1].data(),
                                              config.measuredBins[2].size()-1, config.measuredBins[2].data());
            purityDen = std::make_unique<TH3D>("purityDen", "Purity Denominator", 
                                              config.measuredBins[0].size()-1, config.measuredBins[0].data(),
                                              config.measuredBins[1].size()-1, config.measuredBins[1].data(),
                                              config.measuredBins[2].size()-1, config.measuredBins[2].data());
            // Efficiency histograms (truth level binning)
            effNum = std::make_unique<TH3D>("effNum", "Efficiency Numerator", 
                                           config.truthBins[0].size()-1, config.truthBins[0].data(),
                                           config.truthBins[1].size()-1, config.truthBins[1].data(),
                                           config.truthBins[2].size()-1, config.truthBins[2].data());
            effDen = std::make_unique<TH3D>("effDen", "Efficiency Denominator", 
                                           config.truthBins[0].size()-1, config.truthBins[0].data(),
                                           config.truthBins[1].size()-1, config.truthBins[1].data(),
                                           config.truthBins[2].size()-1, config.truthBins[2].data());
        }
        
        // Set directory to null to avoid ROOT memory management issues
        if (purityNum) purityNum->SetDirectory(0);
        if (purityDen) purityDen->SetDirectory(0);
        if (effNum) effNum->SetDirectory(0);
        if (effDen) effDen->SetDirectory(0);
        
        log(LOG_DEBUG, "Purity and efficiency histograms initialized successfully");
    }

    void fillTree(TTree* tree, bool isData) {
        if (!tree) {
            log(LOG_ERROR, "Null tree pointer");
            return;
        }
        
        // Set up branches dynamically
        std::vector<float> measValues(config.dimension), truthValues(config.dimension);
        float weight = 1.0;
        
        for (int i = 0; i < config.dimension; ++i) {
            tree->SetBranchAddress(config.measuredVars[i].c_str(), &measValues[i]);
            if (!isData) {
                tree->SetBranchAddress(config.truthVars[i].c_str(), &truthValues[i]);
            }
        }
        tree->SetBranchAddress(config.weightBranch.c_str(), &weight);
        Long64_t nEntries = tree->GetEntries();
        log(LOG_DEBUG, "Processing " + std::to_string(nEntries) + " entries from " + (isData ? "data" : "MC") + " tree");        
        int validEntries = 0;
        for (Long64_t i = 0; i < nEntries; ++i) {
            tree->GetEntry(i);
            
            // Check for valid values
            bool valid = true;
            for (int j = 0; j < config.dimension; ++j) {
                if (std::isnan(measValues[j]) || std::isinf(measValues[j]) || measValues[j] < -900) {
                    valid = false;
                    break;
                }
            }
            
            if (!isData) {
                for (int j = 0; j < config.dimension; ++j) {
                    if (std::isnan(truthValues[j]) || std::isinf(truthValues[j]) || truthValues[j] < -900) {
                        valid = false;
                        break;
                    }
                }
            }

            if (valid) {
                // Check detector cuts once for all cases
                bool passesDetectorCuts = true;
                for (int j = 0; j < config.dimension; ++j) {
                    if (measValues[j] < config.measuredBins[j][0] || 
                        measValues[j] >= config.measuredBins[j].back()) {
                        passesDetectorCuts = false;
                        break;
                    }
                }
                
                // Check truth cuts once (only for MC)
                bool passesTruthCuts = true;
                if (!isData) {
                    for (int j = 0; j < config.dimension; ++j) {
                        if (truthValues[j] < config.truthBins[j][0] || 
                            truthValues[j] >= config.truthBins[j].back()) {
                            passesTruthCuts = false;
                            break;
                        }
                    }
                }
                
                // Fill histograms based on cuts
                if (isData) {
                    // Fill data histogram only if passes detector cuts
                    if (passesDetectorCuts) {
                        histManager.fillData(std::vector<double>(measValues.begin(), measValues.end()), weight);
                    }
                } else {
                    // Fill MC histograms only if passes both detector and truth cuts
                    if (passesDetectorCuts && passesTruthCuts) {
                        histManager.fillMC(std::vector<double>(measValues.begin(), measValues.end()),
                                         std::vector<double>(truthValues.begin(), truthValues.end()), weight);
                    }
                    
                    // Fill purity histograms (detector level values)
                    if (passesDetectorCuts) {
                        fillHistogram(purityDen.get(), std::vector<double>(measValues.begin(), measValues.end()), weight);
                        if (passesTruthCuts) {
                            fillHistogram(purityNum.get(), std::vector<double>(measValues.begin(), measValues.end()), weight);
                        }
                    }
                    
                    // Fill efficiency histograms (truth level values)
                    if (passesTruthCuts) {
                        fillHistogram(effDen.get(), std::vector<double>(truthValues.begin(), truthValues.end()), weight);
                        if (passesDetectorCuts) {
                            fillHistogram(effNum.get(), std::vector<double>(truthValues.begin(), truthValues.end()), weight);
                        }
                    }
                }
                validEntries++;
            }
        }        
        log(LOG_DEBUG, "Filled " + std::to_string(validEntries) + " valid entries");
    }
    
    void fillHistogram(TH1* hist, const std::vector<double>& values, double weight) {
        if (!hist) return;
        if (config.dimension == 1) hist->Fill(values[0], weight);
        else if (config.dimension == 2) ((TH2*)hist)->Fill(values[0], values[1], weight);
        else if (config.dimension == 3) ((TH3*)hist)->Fill(values[0], values[1], values[2], weight);
    }
    
    void calculatePurityAndEfficiency() {
        log(LOG_DEBUG, "Calculating purity and efficiency histograms");
        
        if (!purityNum || !purityDen || !effNum || !effDen) {
            log(LOG_WARNING, "Purity/efficiency histograms not initialized properly");
            return;
        }
        
        // Calculate purity histogram (detector level binning) and efficiency histogram (truth level binning)
        if (config.dimension == 1) {
            purityHist = std::make_unique<TH1D>(*((TH1D*)purityNum.get()));
            efficiencyHist = std::make_unique<TH1D>(*((TH1D*)effNum.get()));
        } else if (config.dimension == 2) {
            purityHist = std::make_unique<TH2D>(*((TH2D*)purityNum.get()));
            efficiencyHist = std::make_unique<TH2D>(*((TH2D*)effNum.get()));
        } else if (config.dimension == 3) {
            purityHist = std::make_unique<TH3D>(*((TH3D*)purityNum.get()));
            efficiencyHist = std::make_unique<TH3D>(*((TH3D*)effNum.get()));
        }
        
        // Set names
        purityHist->SetName("purity");
        purityHist->SetTitle("Purity");
        efficiencyHist->SetName("efficiency");
        efficiencyHist->SetTitle("Efficiency");
        
        // Divide to get purity and efficiency
        purityHist->Divide(purityNum.get(), purityDen.get(), 1.0, 1.0, "B");
        efficiencyHist->Divide(effNum.get(), effDen.get(), 1.0, 1.0, "B");
        
        // Set directory to null
        purityHist->SetDirectory(0);
        efficiencyHist->SetDirectory(0);
        
        log(LOG_DEBUG, "Purity and efficiency calculation completed");
    }
    
    TH1D* createFlattenedHistogram(TH1* source, const std::string& name) {
        int totalBins = 1;
        std::vector<int> nBins;
        
        if (config.dimension == 2) {
            TH2* h2 = (TH2*)source;
            nBins = {h2->GetNbinsX(), h2->GetNbinsY()};
            totalBins = nBins[0] * nBins[1];
        } else if (config.dimension == 3) {
            TH3* h3 = (TH3*)source;
            nBins = {h3->GetNbinsX(), h3->GetNbinsY(), h3->GetNbinsZ()};
            totalBins = nBins[0] * nBins[1] * nBins[2];
        }
        
        log(LOG_DEBUG, "createFlattenedHistogram: " + name + " - " + 
            std::to_string(config.dimension) + "D -> 1D (" + std::to_string(totalBins) + " bins)");
        
        // Validate flattening before proceeding
        if (config.dimension > 1 && !validateFlattening(nBins)) {
            log(LOG_ERROR, "Flattening validation failed for " + name);
            return nullptr;
        }
        
        TH1D* flattened = new TH1D(name.c_str(), "Flattened Histogram", totalBins, 0, totalBins);
        
        // Fill flattened histogram using ROW-MAJOR ordering (C++ standard)
        if (config.dimension == 2) {
            TH2* h2 = (TH2*)source;
            for (int i = 1; i <= h2->GetNbinsX(); ++i) {           // X (outer loop, slower increment)
                for (int j = 1; j <= h2->GetNbinsY(); ++j) {       // Y (inner loop, faster increment)
                    std::vector<int> indices = {i-1, j-1};         // Convert to 0-based
                    int flatBin = flattenIndices(indices, nBins) + 1; // Convert back to 1-based for ROOT
                    
                    flattened->SetBinContent(flatBin, h2->GetBinContent(i, j));
                    flattened->SetBinError(flatBin, h2->GetBinError(i, j));
                    
                    log(LOG_TRACE, "2D->1D: (" + std::to_string(i) + "," + std::to_string(j) + 
                        ") -> bin " + std::to_string(flatBin) + " = " + std::to_string(h2->GetBinContent(i, j)));
                }
            }
        } else if (config.dimension == 3) {
            TH3* h3 = (TH3*)source;
            for (int i = 1; i <= h3->GetNbinsX(); ++i) {           // X (outermost)
                for (int j = 1; j <= h3->GetNbinsY(); ++j) {       // Y (middle)
                    for (int k = 1; k <= h3->GetNbinsZ(); ++k) {   // Z (innermost, fastest increment)
                        std::vector<int> indices = {i-1, j-1, k-1}; // Convert to 0-based
                        int flatBin = flattenIndices(indices, nBins) + 1; // Convert back to 1-based for ROOT
                        
                        flattened->SetBinContent(flatBin, h3->GetBinContent(i, j, k));
                        flattened->SetBinError(flatBin, h3->GetBinError(i, j, k));
                        
                        log(LOG_TRACE, "3D->1D: (" + std::to_string(i) + "," + std::to_string(j) + "," + 
                            std::to_string(k) + ") -> bin " + std::to_string(flatBin) + " = " + 
                            std::to_string(h3->GetBinContent(i, j, k)));
                    }
                }
            }
        }
        
        log(LOG_DEBUG, "createFlattenedHistogram completed: " + name + 
            " (entries=" + std::to_string(flattened->GetEntries()) + 
            ", integral=" + std::to_string(flattened->Integral()) + ")");
        
        return flattened;
    }
    
    std::unique_ptr<TH1> convertFlattenedToMultiDim(TH1D* flattened, const std::string& name, bool useMeasuredBinning = false) {
        if (config.dimension == 2) {
            // Choose binning based on parameter
            const auto& binning0 = useMeasuredBinning ? config.measuredBins[0] : config.truthBins[0];
            const auto& binning1 = useMeasuredBinning ? config.measuredBins[1] : config.truthBins[1];
            
            // Create 2D histogram with selected binning
            std::string title = (useMeasuredBinning ? "Measured" : "Unfolded") + std::string(" 2D Histogram");
            auto hist2D = std::make_unique<TH2D>(name.c_str(), title.c_str(),
                                               binning0.size()-1, binning0.data(),
                                               binning1.size()-1, binning1.data());
            hist2D->SetDirectory(0);
            
            std::vector<int> nBins = {hist2D->GetNbinsX(), hist2D->GetNbinsY()};
            log(LOG_DEBUG, "convertFlattenedToMultiDim: " + name + " - 1D -> 2D (" + 
                std::to_string(nBins[0]) + "x" + std::to_string(nBins[1]) + ") using " + 
                (useMeasuredBinning ? "measured" : "truth") + " binning");
            
            // Validate flattening consistency
            if (!validateFlattening(nBins)) {
                log(LOG_ERROR, "Flattening validation failed during conversion for " + name);
                return nullptr;
            }
            
            // Fill 2D histogram from flattened data using ROW-MAJOR ordering
            for (int i = 1; i <= hist2D->GetNbinsX(); ++i) {       // X (outer loop)
                for (int j = 1; j <= hist2D->GetNbinsY(); ++j) {   // Y (inner loop)
                    std::vector<int> indices = {i-1, j-1};         // Convert to 0-based
                    int flatBin = flattenIndices(indices, nBins) + 1; // Convert to 1-based for ROOT
                    
                    hist2D->SetBinContent(i, j, flattened->GetBinContent(flatBin));
                    hist2D->SetBinError(i, j, flattened->GetBinError(flatBin));
                    
                    log(LOG_TRACE, "1D->2D: bin " + std::to_string(flatBin) + " -> (" + 
                        std::to_string(i) + "," + std::to_string(j) + ") = " + 
                        std::to_string(flattened->GetBinContent(flatBin)));
                }
            }
            
            log(LOG_DEBUG, "convertFlattenedToMultiDim completed: " + name + 
                " (entries=" + std::to_string(hist2D->GetEntries()) + 
                ", integral=" + std::to_string(hist2D->Integral()) + ")");
            
            return std::move(hist2D);
            
        } else if (config.dimension == 3) {
            // Choose binning based on parameter
            const auto& binning0 = useMeasuredBinning ? config.measuredBins[0] : config.truthBins[0];
            const auto& binning1 = useMeasuredBinning ? config.measuredBins[1] : config.truthBins[1];
            const auto& binning2 = useMeasuredBinning ? config.measuredBins[2] : config.truthBins[2];
            
            // Create 3D histogram with selected binning
            std::string title = (useMeasuredBinning ? "Measured" : "Unfolded") + std::string(" 3D Histogram");
            auto hist3D = std::make_unique<TH3D>(name.c_str(), title.c_str(),
                                               binning0.size()-1, binning0.data(),
                                               binning1.size()-1, binning1.data(),
                                               binning2.size()-1, binning2.data());
            hist3D->SetDirectory(0);
            
            std::vector<int> nBins = {hist3D->GetNbinsX(), hist3D->GetNbinsY(), hist3D->GetNbinsZ()};
            log(LOG_DEBUG, "convertFlattenedToMultiDim: " + name + " - 1D -> 3D (" + 
                std::to_string(nBins[0]) + "x" + std::to_string(nBins[1]) + "x" + std::to_string(nBins[2]) + ") using " + 
                (useMeasuredBinning ? "measured" : "truth") + " binning");
            
            // Validate flattening consistency
            if (!validateFlattening(nBins)) {
                log(LOG_ERROR, "Flattening validation failed during conversion for " + name);
                return nullptr;
            }
            
            // Fill 3D histogram from flattened data using ROW-MAJOR ordering
            for (int i = 1; i <= hist3D->GetNbinsX(); ++i) {       // X (outermost)
                for (int j = 1; j <= hist3D->GetNbinsY(); ++j) {   // Y (middle)
                    for (int k = 1; k <= hist3D->GetNbinsZ(); ++k) { // Z (innermost)
                        std::vector<int> indices = {i-1, j-1, k-1}; // Convert to 0-based
                        int flatBin = flattenIndices(indices, nBins) + 1; // Convert to 1-based for ROOT
                        
                        hist3D->SetBinContent(i, j, k, flattened->GetBinContent(flatBin));
                        hist3D->SetBinError(i, j, k, flattened->GetBinError(flatBin));
                        
                        log(LOG_TRACE, "1D->3D: bin " + std::to_string(flatBin) + " -> (" + 
                            std::to_string(i) + "," + std::to_string(j) + "," + std::to_string(k) + 
                            ") = " + std::to_string(flattened->GetBinContent(flatBin)));
                    }
                }
            }
            
            log(LOG_DEBUG, "convertFlattenedToMultiDim completed: " + name + 
                " (entries=" + std::to_string(hist3D->GetEntries()) + 
                ", integral=" + std::to_string(hist3D->Integral()) + ")");
            
            return std::move(hist3D);
        }
        
        // For 1D case, just return a copy
        auto hist1D = std::make_unique<TH1D>(*flattened);
        hist1D->SetDirectory(0);
        log(LOG_DEBUG, "convertFlattenedToMultiDim: " + name + " - 1D passthrough");
        return std::move(hist1D);
    }
    
    void logStatistics() {
        log(LOG_INFO, "Final statistics:");
        log(LOG_INFO, "  Data entries: " + std::to_string(histManager.getData()->GetEntries()));
        log(LOG_INFO, "  Data integral: " + std::to_string(histManager.getData()->Integral()));
        log(LOG_INFO, "  MC measured entries: " + std::to_string(histManager.getMeasuredMC()->GetEntries()));
        log(LOG_INFO, "  MC measured integral: " + std::to_string(histManager.getMeasuredMC()->Integral()));
        log(LOG_INFO, "  MC truth entries: " + std::to_string(histManager.getTruthMC()->GetEntries()));
        log(LOG_INFO, "  MC truth integral: " + std::to_string(histManager.getTruthMC()->Integral()));
        log(LOG_INFO, "  Response entries: " + std::to_string(histManager.getResponse()->GetEntries()));
        log(LOG_INFO, "  Response integral: " + std::to_string(histManager.getResponse()->Integral()));
    }
    
    void validateFlatteningConsistency() {
        log(LOG_INFO, "=== VALIDATING FLATTENING CONSISTENCY ===");
        
        // Get dimensions for validation
        std::vector<int> nBinsMeas, nBinsTruth;
        
        if (config.dimension == 2) {
            TH2* h_meas = (TH2*)histManager.getMeasuredMC();
            TH2* h_truth = (TH2*)histManager.getTruthMC();
            nBinsMeas = {h_meas->GetNbinsX(), h_meas->GetNbinsY()};
            nBinsTruth = {h_truth->GetNbinsX(), h_truth->GetNbinsY()};
        } else if (config.dimension == 3) {
            TH3* h_meas = (TH3*)histManager.getMeasuredMC();
            TH3* h_truth = (TH3*)histManager.getTruthMC();
            nBinsMeas = {h_meas->GetNbinsX(), h_meas->GetNbinsY(), h_meas->GetNbinsZ()};
            nBinsTruth = {h_truth->GetNbinsX(), h_truth->GetNbinsY(), h_truth->GetNbinsZ()};
        }
        
        // Validate basic flattening
        bool measValid = validateFlattening(nBinsMeas);
        bool truthValid = validateFlattening(nBinsTruth);
        
        if (!measValid || !truthValid) {
            log(LOG_ERROR, "Basic flattening validation failed!");
            return;
        }
        
        // Test round-trip consistency for a few sample bins
        log(LOG_DEBUG, "Testing round-trip flattening consistency...");
        
        // Test a few corner cases and middle bins
        std::vector<std::vector<int>> testIndices;
        if (config.dimension == 2) {
            testIndices = {
                {0, 0}, {0, nBinsMeas[1]-1}, 
                {nBinsMeas[0]-1, 0}, {nBinsMeas[0]-1, nBinsMeas[1]-1},
                {nBinsMeas[0]/2, nBinsMeas[1]/2}
            };
        } else if (config.dimension == 3) {
            testIndices = {
                {0, 0, 0}, {0, 0, nBinsMeas[2]-1},
                {0, nBinsMeas[1]-1, 0}, {nBinsMeas[0]-1, 0, 0},
                {nBinsMeas[0]/2, nBinsMeas[1]/2, nBinsMeas[2]/2}
            };
        }
        
        bool allTestsPassed = true;
        for (const auto& indices : testIndices) {
            // Check bounds
            bool validIndices = true;
            for (size_t i = 0; i < indices.size(); ++i) {
                if (indices[i] >= nBinsMeas[i]) {
                    validIndices = false;
                    break;
                }
            }
            if (!validIndices) continue;
            
            // Test flatten -> unflatten
            int flat = flattenIndices(indices, nBinsMeas);
            auto recovered = unflattenIndex(flat, nBinsMeas);
            
            if (recovered != indices) {
                log(LOG_ERROR, "Round-trip test failed for indices " + 
                    [&]() {
                        std::string s = "[";
                        for (size_t i = 0; i < indices.size(); ++i) {
                            if (i > 0) s += ",";
                            s += std::to_string(indices[i]);
                        }
                        s += "]";
                        return s;
                    }());
                allTestsPassed = false;
            }
        }
        
        if (allTestsPassed) {
            log(LOG_INFO, "✅ All flattening consistency tests PASSED");
        } else {
            log(LOG_ERROR, "❌ Flattening consistency tests FAILED");
        }
        
        // Validate that createFlattenedHistogram and convertFlattenedToMultiDim are consistent
        log(LOG_DEBUG, "Testing histogram conversion consistency...");
        
        // Create a test histogram with known pattern
        std::unique_ptr<TH1> testHist;
        if (config.dimension == 2) {
            // Create test histogram with EXACT same binning as measured data
            testHist = std::make_unique<TH2D>("test", "Test", 
                config.measuredBins[0].size()-1, config.measuredBins[0].data(),
                config.measuredBins[1].size()-1, config.measuredBins[1].data());
            TH2D* h2 = (TH2D*)testHist.get();
            
            // Fill with pattern where bin content = flattenIndices result
            // This tests if flattenIndices returns the correct flat index for each (i,j) position
            for (int i = 1; i <= nBinsMeas[0]; ++i) {
                for (int j = 1; j <= nBinsMeas[1]; ++j) {
                    std::vector<int> indices = {i-1, j-1}; // Convert to 0-based for flattenIndices
                    int expectedValue = flattenIndices(indices, nBinsMeas);
                    h2->SetBinContent(i, j, expectedValue);
                }
            }
        }
        
        if (testHist) {
            // Test conversion: ND -> 1D -> ND
            auto flattened = std::unique_ptr<TH1D>(createFlattenedHistogram(testHist.get(), "test_flat"));
            // CRITICAL FIX: Use measured binning to match the test histogram binning
            auto recovered = convertFlattenedToMultiDim(flattened.get(), "test_recovered", true);
            
            // Debug: Print dimensions and first few values
            if (config.dimension == 2) {
                TH2D* orig = (TH2D*)testHist.get();
                TH2D* rec = (TH2D*)recovered.get();
                
                log(LOG_DEBUG, "Original dimensions: " + std::to_string(orig->GetNbinsX()) + "x" + std::to_string(orig->GetNbinsY()));
                log(LOG_DEBUG, "Recovered dimensions: " + std::to_string(rec->GetNbinsX()) + "x" + std::to_string(rec->GetNbinsY()));
                log(LOG_DEBUG, "Flattened bins: " + std::to_string(flattened->GetNbinsX()));
                log(LOG_DEBUG, "nBinsMeas: [" + std::to_string(nBinsMeas[0]) + "," + std::to_string(nBinsMeas[1]) + "]");
                
                // Debug first few bins
                for (int i = 1; i <= std::min(3, nBinsMeas[0]); ++i) {
                    for (int j = 1; j <= std::min(2, nBinsMeas[1]); ++j) {
                        std::vector<int> indices = {i-1, j-1};
                        int expectedFlat = flattenIndices(indices, nBinsMeas);
                        double origValue = orig->GetBinContent(i, j);
                        double flatValue = flattened->GetBinContent(expectedFlat + 1); // +1 for ROOT indexing
                        double recValue = rec->GetBinContent(i, j);
                        
                        log(LOG_DEBUG, "Bin (" + std::to_string(i) + "," + std::to_string(j) + 
                            "): indices=" + std::to_string(indices[0]) + "," + std::to_string(indices[1]) +
                            ", expectedFlat=" + std::to_string(expectedFlat) + 
                            ", orig=" + std::to_string(origValue) + 
                            ", flat[" + std::to_string(expectedFlat+1) + "]=" + std::to_string(flatValue) +
                            ", recovered=" + std::to_string(recValue));
                    }
                }
            }
            
            // Compare original and recovered
            bool conversionValid = true;
            if (config.dimension == 2) {
                TH2D* orig = (TH2D*)testHist.get();
                TH2D* rec = (TH2D*)recovered.get();
                
                for (int i = 1; i <= nBinsMeas[0]; ++i) {
                    for (int j = 1; j <= nBinsMeas[1]; ++j) {
                        if (std::abs(orig->GetBinContent(i, j) - rec->GetBinContent(i, j)) > 1e-10) {
                            log(LOG_ERROR, "Conversion test failed at bin (" + 
                                std::to_string(i) + "," + std::to_string(j) + "): " +
                                std::to_string(orig->GetBinContent(i, j)) + " != " +
                                std::to_string(rec->GetBinContent(i, j)));
                            conversionValid = false;
                        }
                    }
                }
            }
            
            if (conversionValid) {
                log(LOG_INFO, "✅ Histogram conversion consistency test PASSED");
            } else {
                log(LOG_ERROR, "❌ Histogram conversion consistency test FAILED");
            }
        }
        
        log(LOG_INFO, "=== FLATTENING VALIDATION COMPLETE ===");
    }
    
    // Bottomline test helper methods
    void scanBayesianIterations();
    void saveBottomlineIterationPlots(TDirectory* testDir, const std::string& testLabel);
};

// ============================================================================
// BOTTOMLINE TEST INLINE IMPLEMENTATIONS
// ============================================================================

void OptimizedUnfolder::performBottomlineTest() {
    if (!config.enableBottomlineTest) return;
    if (!histManager.getData() || !histManager.getTruthMC() || !response) return;
    
    log(LOG_INFO, "=== PERFORMING BOTTOMLINE TEST ===");
    bottomlineResults = BottomlineResults();
    
    // Get and flatten histograms if needed
    TH1* dataHist = histManager.getData();
    TH1* truthHist = histManager.getTruthMC();
    std::unique_ptr<TH1D> dataFlat, truthFlat;
    if (config.dimension > 1) {
        dataFlat.reset(createFlattenedHistogram(dataHist, "data_flat_bottomline"));
        truthFlat.reset(createFlattenedHistogram(truthHist, "truth_flat_bottomline"));
        dataHist = dataFlat.get();
        truthHist = truthFlat.get();
    }
    
    // Normalize histograms using Scale("width")
    std::unique_ptr<TH1> normData(static_cast<TH1*>(dataHist->Clone("dataNorm")));
    std::unique_ptr<TH1> normTruth(static_cast<TH1*>(truthHist->Clone("truthNorm")));
    normData->SetDirectory(0);
    normTruth->SetDirectory(0);
    
    double dataIntegral = normData->Integral();
    double truthIntegral = normTruth->Integral();
    if (dataIntegral > 0) normData->Scale(1.0 / dataIntegral, "width");
    if (truthIntegral > 0) normTruth->Scale(1.0 / truthIntegral, "width");
    
    // Forward fold normalized truth
    std::unique_ptr<TH1> forwardFolded(response->ApplyToTruth(normTruth.get()));
    if (!forwardFolded) {
        log(LOG_ERROR, "Forward folding failed");
        return;
    }
    forwardFolded->SetDirectory(0);
    double ffIntegral = forwardFolded->Integral();
    if (ffIntegral > 0) forwardFolded->Scale(1.0 / ffIntegral, "width");
    
    // Measured space chi2 - use measuredCovMatrix only
    double chi2Smeared = 0.0;
    int ndfSmeared = 0;
    if (!measuredCovMatrix || measuredCovMatrix->GetNrows() != normData->GetNbinsX()) {
        log(LOG_ERROR, "Measured covariance matrix missing or wrong size for measured space chi2 calculation. Aborting chi2 calculation.");
        return;
    }
    log(LOG_DEBUG, "Using measured covariance matrix for measured space chi2 calculation");
    // Create residual vector
    std::vector<double> residuals(normData->GetNbinsX());
    for (int i = 1; i <= normData->GetNbinsX(); ++i) {
        residuals[i-1] = normData->GetBinContent(i) - forwardFolded->GetBinContent(i);
    }
    // Calculate chi2 using measured covariance matrix: chi2 = r^T * C^-1 * r
    TMatrixD covInverse(*measuredCovMatrix);
    if (covInverse.Determinant() == 0) {
        log(LOG_ERROR, "Measured covariance matrix is singular for measured space chi2 calculation. Aborting chi2 calculation.");
        return;
    }
    covInverse.Invert();
    for (int i = 0; i < normData->GetNbinsX(); ++i) {
        for (int j = 0; j < normData->GetNbinsX(); ++j) {
            chi2Smeared += residuals[i] * covInverse[i][j] * residuals[j];
        }
    }
    ndfSmeared = normData->GetNbinsX() - 1;
    
    // Store raw chi2 and reduced chi2
    double rawChi2Smeared = chi2Smeared;
    double reducedChi2Smeared = chi2Smeared / ndfSmeared;
    
    bottomlineResults.chi2Smeared = rawChi2Smeared;  // Store raw chi2 for consistency with reference
    bottomlineResults.ndfSmeared = ndfSmeared;
    bottomlineResults.pValueSmeared = TMath::Prob(rawChi2Smeared, ndfSmeared);
    
    log(LOG_INFO, "Measured space: χ² = " + std::to_string(rawChi2Smeared) + 
        ", χ²/NDF = " + std::to_string(reducedChi2Smeared) +
        ", NDF = " + std::to_string(ndfSmeared) + 
        ", p-value = " + std::to_string(bottomlineResults.pValueSmeared));
    
    // Unfolded space chi2
    if (unfoldedHist) {
        TH1* unfoldedForComparison = unfoldedHist.get();
        std::unique_ptr<TH1D> unfoldedFlat;
        if (config.dimension > 1) {
            unfoldedFlat.reset(createFlattenedHistogram(unfoldedForComparison, "unfolded_flat_bottomline"));
            unfoldedForComparison = unfoldedFlat.get();
        }
        
        std::unique_ptr<TH1> normUnfolded(static_cast<TH1*>(unfoldedForComparison->Clone("unfoldedNorm")));
        normUnfolded->SetDirectory(0);
        double unfoldedIntegral = normUnfolded->Integral();
        if (unfoldedIntegral > 0) normUnfolded->Scale(1.0 / unfoldedIntegral, "width");
        
        // Ensure binning matches
        int nBinsTruth = normTruth->GetNbinsX();
        int nBinsUnfolded = normUnfolded->GetNbinsX();
        if (nBinsTruth == nBinsUnfolded) {
            double chi2Unfolded = 0.0;
            int ndfUnfolded = 0;
            
            // Try to use full covariance matrix for unfolded space
            if (covMatrix && covMatrix->GetNrows() == nBinsTruth) {
                log(LOG_DEBUG, "Using full covariance matrix for unfolded space chi2 calculation");
                
                // Create residual vector for truth vs unfolded
                std::vector<double> residuals(nBinsTruth);
                for (int i = 1; i <= nBinsTruth; ++i) {
                    residuals[i-1] = normTruth->GetBinContent(i) - normUnfolded->GetBinContent(i);
                }
                
                // Calculate chi2 using full covariance matrix: chi2 = r^T * C^-1 * r
                TMatrixD covInverse(*covMatrix);
                if (covInverse.Determinant() != 0) {
                    covInverse.Invert();
                    
                    for (int i = 0; i < nBinsTruth; ++i) {
                        for (int j = 0; j < nBinsTruth; ++j) {
                            chi2Unfolded += residuals[i] * covInverse[i][j] * residuals[j];
                        }
                    }
                    ndfUnfolded = nBinsTruth - 1;
                    
                    log(LOG_DEBUG, "Full covariance matrix chi2 calculation completed");
                } else {
                    log(LOG_WARNING, "Covariance matrix is singular, falling back to diagonal approximation");
                    // Fall back to diagonal method
                    for (int i = 1; i <= nBinsTruth; ++i) {
                        double truthVal = normTruth->GetBinContent(i);
                        double unfoldedVal = normUnfolded->GetBinContent(i);
                        double truthError = normTruth->GetBinError(i);
                        double unfoldedError = normUnfolded->GetBinError(i);
                        
                        // CRITICAL: Use combined error from both distributions
                        double combinedError = std::sqrt(truthError * truthError + unfoldedError * unfoldedError);
                        
                        // Check if combined error is too small and use a minimum error
                        if (combinedError < 1e-10) {
                            combinedError = std::max(1e-10, std::max(truthVal, unfoldedVal) * 0.01);
                        }
                        
                        if (truthVal > 0 && combinedError > 0) {
                            double residual = truthVal - unfoldedVal;
                            chi2Unfolded += (residual * residual) / (combinedError * combinedError);
                            ndfUnfolded++;
                        }
                    }
                    ndfUnfolded = std::max(1, ndfUnfolded - 1);
                }
            } else {
                log(LOG_DEBUG, "Using diagonal approximation for unfolded space chi2 calculation");
                // Standard diagonal method with combined errors
                for (int i = 1; i <= nBinsTruth; ++i) {
                    double truthVal = normTruth->GetBinContent(i);
                    double unfoldedVal = normUnfolded->GetBinContent(i);
                    double truthError = normTruth->GetBinError(i);
                    double unfoldedError = normUnfolded->GetBinError(i);
                    
                    // CRITICAL: Use combined error from both distributions
                    double combinedError = std::sqrt(truthError * truthError + unfoldedError * unfoldedError);
                    
                    // Check if combined error is too small and use a minimum error
                    if (combinedError < 1e-10) {
                        combinedError = std::max(1e-10, std::max(truthVal, unfoldedVal) * 0.01);
                    }
                    
                    if (truthVal > 0 && combinedError > 0) {
                        double residual = truthVal - unfoldedVal;
                        chi2Unfolded += (residual * residual) / (combinedError * combinedError);
                        ndfUnfolded++;
                    }
                }
                ndfUnfolded = std::max(1, ndfUnfolded - 1);
            }
            
            // Store raw chi2 and reduced chi2
            double rawChi2Unfolded = chi2Unfolded;
            double reducedChi2Unfolded = chi2Unfolded / ndfUnfolded;
            
            bottomlineResults.chi2Unfolded = rawChi2Unfolded;  // Store raw chi2 for consistency with reference
            bottomlineResults.ndfUnfolded = ndfUnfolded;
            bottomlineResults.pValueUnfolded = TMath::Prob(rawChi2Unfolded, ndfUnfolded);
            
            log(LOG_INFO, "Unfolded space: χ² = " + std::to_string(rawChi2Unfolded) + 
                ", χ²/NDF = " + std::to_string(reducedChi2Unfolded) +
                ", NDF = " + std::to_string(ndfUnfolded) + 
                ", p-value = " + std::to_string(bottomlineResults.pValueUnfolded));
        }
    }
    
    // Bayesian iteration scanning for optimal regularization
    if (config.method == "Bayes" && !bayesianIterations.empty()) {
        log(LOG_DEBUG, "Found " + std::to_string(bayesianIterations.size()) + " Bayesian iterations to scan");
        scanBayesianIterations();
    } else {
        log(LOG_WARNING, "Bayesian iteration scanning skipped - method: " + config.method + 
            ", iterations available: " + std::to_string(bayesianIterations.size()));
    }
    
    // Format results
    std::ostringstream oss;
    oss << "=== BOTTOMLINE TEST RESULTS ===\n";
    oss << "Measured Space (constant): Chi^2 = " << std::fixed << std::setprecision(5) << bottomlineResults.chi2Smeared;
    oss << ", Chi^2/NDF = " << std::setprecision(5) << (bottomlineResults.chi2Smeared / bottomlineResults.ndfSmeared);
    oss << ", NDF = " << bottomlineResults.ndfSmeared;
    oss << ", p-value = " << std::setprecision(5) << bottomlineResults.pValueSmeared << "\n";

    // Unfolded space for optimal iteration
    if (config.method == "Bayes" && bottomlineResults.optimalIteration > 0 &&
        bottomlineResults.optimalIteration <= (int)bottomlineResults.chi2UnfoldedPerIteration.size()) {
        int opt = bottomlineResults.optimalIteration - 1;
        oss << "Unfolded Space (optimal iteration): Chi^2 = " << std::setprecision(5) << bottomlineResults.chi2UnfoldedPerIteration[opt];
        oss << ", Chi^2/NDF = " << std::setprecision(5) << (bottomlineResults.chi2UnfoldedPerIteration[opt] / (bottomlineResults.ndfUnfolded > 0 ? bottomlineResults.ndfUnfolded : 1));
        oss << ", NDF = " << bottomlineResults.ndfUnfolded;
        oss << ", p-value = " << std::setprecision(5) << bottomlineResults.pValueUnfoldedPerIteration[opt] << "\n";
    } else if (bottomlineResults.chi2Unfolded >= 0) {
        oss << "Unfolded Space: Chi^2 = " << std::setprecision(5) << bottomlineResults.chi2Unfolded;
        oss << ", Chi^2/NDF = " << std::setprecision(5) << (bottomlineResults.chi2Unfolded / bottomlineResults.ndfUnfolded);
        oss << ", NDF = " << bottomlineResults.ndfUnfolded;
        oss << ", p-value = " << std::setprecision(5) << bottomlineResults.pValueUnfolded << "\n";
    }

    if (config.method == "Bayes" && bottomlineResults.optimalIteration > 0) {
        oss << "Bayesian Optimization:\n";
        oss << "  Optimal iteration: " << bottomlineResults.optimalIteration;
        oss << " (p-value threshold: " << std::fixed << std::setprecision(2) << config.pValueThreshold << ")\n";
        oss << "  Scanned " << bottomlineResults.pValueSmearedPerIteration.size() << " iterations\n";
    }

    bottomlineResults.summary = oss.str();
    log(LOG_INFO, "=== BOTTOMLINE TEST COMPLETED ===");
    log(LOG_INFO, bottomlineResults.summary);
}

void OptimizedUnfolder::scanBayesianIterations() {
    log(LOG_INFO, "Scanning " + std::to_string(bayesianIterations.size()) + " Bayesian iterations");
    
    bottomlineResults.chi2SmearedPerIteration.clear();
    bottomlineResults.pValueSmearedPerIteration.clear();
    bottomlineResults.chi2UnfoldedPerIteration.clear();
    bottomlineResults.pValueUnfoldedPerIteration.clear();
    
    TH1* dataHist = histManager.getData();
    TH1* truthHist = histManager.getTruthMC();
    
    // Create flattened versions if needed
    std::unique_ptr<TH1D> dataFlat, truthFlat;
    if (config.dimension > 1) {
        dataFlat.reset(createFlattenedHistogram(dataHist, "data_flat_iter_scan"));
        truthFlat.reset(createFlattenedHistogram(truthHist, "truth_flat_iter_scan"));
        dataHist = dataFlat.get();
        truthHist = truthFlat.get();
    }
    
    // Normalize truth histogram using Scale("width")
    std::unique_ptr<TH1> normTruth(static_cast<TH1*>(truthHist->Clone("truthNormIter")));
    normTruth->SetDirectory(0);
    double truthIntegral = normTruth->Integral();
    if (truthIntegral > 0) normTruth->Scale(1.0 / truthIntegral, "width");
    
    // Measured space chi2/p-value per iteration: always use the main result (constant)
    double chi2SmearedConstant = bottomlineResults.chi2Smeared;
    double pValueSmearedConstant = bottomlineResults.pValueSmeared;
    
    // Reset optimal iteration
    bottomlineResults.optimalIteration = -1;
    
    for (size_t iter = 0; iter < bayesianIterations.size(); ++iter) {
        int iterNumber = iter + 1;
        log(LOG_DEBUG, "Processing iteration " + std::to_string(iterNumber) + " of " + std::to_string(bayesianIterations.size()));
        
        TH1* unfoldedIter = bayesianIterations[iter].get();
        if (!unfoldedIter) {
            log(LOG_WARNING, "Null histogram for iteration " + std::to_string(iterNumber));
            continue;
        }
        
        // Get unfolded histogram for this iteration (flatten if needed)
        TH1* unfoldedForComparison = unfoldedIter;
        std::unique_ptr<TH1D> unfoldedFlat;
        if (config.dimension > 1) {
            unfoldedFlat.reset(createFlattenedHistogram(unfoldedForComparison, "unfolded_flat_iter" + std::to_string(iterNumber)));
            unfoldedForComparison = unfoldedFlat.get();
        }
        
        // Normalize unfolded histogram
        std::unique_ptr<TH1> normUnfolded(static_cast<TH1*>(unfoldedForComparison->Clone("unfoldedNormIter")));
        normUnfolded->SetDirectory(0);
        double unfoldedIntegral = normUnfolded->Integral();
        if (unfoldedIntegral > 0) normUnfolded->Scale(1.0 / unfoldedIntegral, "width");
        
        // Measured space is constant (from measuredCovMatrix)
        bottomlineResults.chi2SmearedPerIteration.push_back(chi2SmearedConstant);
        bottomlineResults.pValueSmearedPerIteration.push_back(pValueSmearedConstant);
        
        // Calculate chi2 and p-value in unfolded space (varies per iteration)
        int nBinsTruth = normTruth->GetNbinsX();
        int nBinsUnfolded = normUnfolded->GetNbinsX();
        
        if (nBinsTruth == nBinsUnfolded) {
            double chi2Unfolded = 0.0;
            int ndfUnfolded = 0;
            
            // Debug: Print first few bins for diagnosis
            if (iterNumber <= 3) {  // Only for first few iterations to avoid spam
                log(LOG_DEBUG, "Unfolded space diagnosis for iteration " + std::to_string(iterNumber) + ":");
                log(LOG_DEBUG, "  Truth integral: " + std::to_string(normTruth->Integral()));
                log(LOG_DEBUG, "  Unfolded integral: " + std::to_string(normUnfolded->Integral()));
                
                for (int i = 1; i <= std::min(5, nBinsTruth); ++i) {
                    double truthVal = normTruth->GetBinContent(i);
                    double unfoldedVal = normUnfolded->GetBinContent(i);
                    double truthError = normTruth->GetBinError(i);
                    double unfoldedError = normUnfolded->GetBinError(i);
                    double combinedError = std::sqrt(truthError * truthError + unfoldedError * unfoldedError);
                    
                    log(LOG_DEBUG, "  Bin " + std::to_string(i) + ": truth=" + std::to_string(truthVal) + 
                        ", unfolded=" + std::to_string(unfoldedVal) + 
                        ", truthErr=" + std::to_string(truthError) + 
                        ", unfoldedErr=" + std::to_string(unfoldedError) + 
                        ", combinedErr=" + std::to_string(combinedError) + 
                        ", ratio=" + std::to_string(truthVal/std::max(unfoldedVal, 1e-10)));
                    
                    if (truthVal > 0 && combinedError > 0) {
                        double residual = truthVal - unfoldedVal;
                        double chi2_contrib = (residual * residual) / (combinedError * combinedError);
                        
                        log(LOG_DEBUG, "    residual=" + std::to_string(residual) + 
                            ", chi2_contrib=" + std::to_string(chi2_contrib));
                    }
                }
            }
            
            // Calculate chi2 for all bins
            // Try to use full covariance matrix for unfolded space (consistent with performBottomlineTest)
            if (covMatrix && covMatrix->GetNrows() == nBinsTruth) {
                log(LOG_DEBUG, "Using full covariance matrix for unfolded space chi2 calculation (iteration " + std::to_string(iterNumber) + ")");
                
                // Create residual vector for truth vs unfolded
                std::vector<double> residuals(nBinsTruth);
                for (int i = 1; i <= nBinsTruth; ++i) {
                    residuals[i-1] = normTruth->GetBinContent(i) - normUnfolded->GetBinContent(i);
                }
                
                // Calculate chi2 using full covariance matrix: chi2 = r^T * C^-1 * r
                TMatrixD covInverse(*covMatrix);
                if (covInverse.Determinant() != 0) {
                    covInverse.Invert();
                    
                    for (int i = 0; i < nBinsTruth; ++i) {
                        for (int j = 0; j < nBinsTruth; ++j) {
                            chi2Unfolded += residuals[i] * covInverse[i][j] * residuals[j];
                        }
                    }
                    ndfUnfolded = nBinsTruth - 1;
                    
                    log(LOG_DEBUG, "Full covariance matrix chi2 calculation completed for iteration " + std::to_string(iterNumber));
                } else {
                    log(LOG_WARNING, "Covariance matrix is singular, falling back to diagonal approximation for iteration " + std::to_string(iterNumber));
                    // Fall back to diagonal method
                    for (int i = 1; i <= nBinsTruth; ++i) {
                        double truthVal = normTruth->GetBinContent(i);
                        double unfoldedVal = normUnfolded->GetBinContent(i);
                        double truthError = normTruth->GetBinError(i);
                        double unfoldedError = normUnfolded->GetBinError(i);
                        
                        // CRITICAL: Use combined error from both distributions
                        double combinedError = std::sqrt(truthError * truthError + unfoldedError * unfoldedError);
                        
                        // Check if combined error is too small and use a minimum error
                        if (combinedError < 1e-10) {
                            combinedError = std::max(1e-10, std::max(truthVal, unfoldedVal) * 0.01);
                        }
                        
                        if (truthVal > 0 && combinedError > 0) {
                            double residual = truthVal - unfoldedVal;
                            chi2Unfolded += (residual * residual) / (combinedError * combinedError);
                            ndfUnfolded++;
                        }
                    }
                    ndfUnfolded = std::max(1, ndfUnfolded - 1);
                }
            } else {
                log(LOG_DEBUG, "Using diagonal approximation for unfolded space chi2 calculation (iteration " + std::to_string(iterNumber) + ")");
                // Standard diagonal method with combined errors
                for (int i = 1; i <= nBinsTruth; ++i) {
                    double truthVal = normTruth->GetBinContent(i);
                    double unfoldedVal = normUnfolded->GetBinContent(i);
                    double truthError = normTruth->GetBinError(i);
                    double unfoldedError = normUnfolded->GetBinError(i);
                    
                    // CRITICAL: Use combined error from both distributions
                    double combinedError = std::sqrt(truthError * truthError + unfoldedError * unfoldedError);
                    
                    // Check if combined error is too small and use a minimum error
                    if (combinedError < 1e-10) {
                        combinedError = std::max(1e-10, std::max(truthVal, unfoldedVal) * 0.01);  // Use 1% of larger value as minimum error
                    }
                    
                    if (truthVal > 0 && combinedError > 0) {
                        double residual = truthVal - unfoldedVal;
                        chi2Unfolded += (residual * residual) / (combinedError * combinedError);
                        ndfUnfolded++;
                    }
                }
                ndfUnfolded = std::max(1, ndfUnfolded - 1);
            }
            
            // Store raw chi2 and reduced chi2
            double rawChi2Unfolded = chi2Unfolded;
            double reducedChi2Unfolded = chi2Unfolded / ndfUnfolded;
            double pValueUnfolded = TMath::Prob(rawChi2Unfolded, ndfUnfolded);
            
            bottomlineResults.chi2UnfoldedPerIteration.push_back(rawChi2Unfolded);  // Store raw chi2
            bottomlineResults.pValueUnfoldedPerIteration.push_back(pValueUnfolded);
            
            log(LOG_INFO, "Iteration " + std::to_string(iterNumber) + 
                ": χ² = " + std::to_string(rawChi2Unfolded) + 
                ", χ²/NDF = " + std::to_string(reducedChi2Unfolded) +
                ", p-value(smeared) = " + std::to_string(pValueSmearedConstant) + " [constant]" +
                ", p-value(unfolded) = " + std::to_string(pValueUnfolded));
            
            // Check for optimal iteration based on unfolded space p-value
            if (bottomlineResults.optimalIteration == -1 && pValueUnfolded >= config.pValueThreshold) {
                bottomlineResults.optimalIteration = iterNumber;
                log(LOG_INFO, "✅ Found optimal iteration: " + std::to_string(iterNumber) + 
                    " (unfolded p-value = " + std::to_string(pValueUnfolded) + " >= " + std::to_string(config.pValueThreshold) + ")");
            }
        } else {
            log(LOG_ERROR, "Binning mismatch for iteration " + std::to_string(iterNumber) + 
                ": truth=" + std::to_string(nBinsTruth) + ", unfolded=" + std::to_string(nBinsUnfolded));
        }
    }
    
    // If no iteration meets the threshold, use the last one
    if (bottomlineResults.optimalIteration == -1 && !bottomlineResults.pValueUnfoldedPerIteration.empty()) {
        bottomlineResults.optimalIteration = bottomlineResults.pValueUnfoldedPerIteration.size();
        log(LOG_WARNING, "No iteration met p-value threshold " + std::to_string(config.pValueThreshold) + 
            ", using final iteration " + std::to_string(bottomlineResults.optimalIteration));
    }
    
    log(LOG_INFO, "Bayesian iteration scan completed. Processed " + std::to_string(bottomlineResults.pValueUnfoldedPerIteration.size()) + " iterations.");
}

void OptimizedUnfolder::saveBottomlineResults(TDirectory* testDir, const std::string& testLabel) {
    if (!config.enableBottomlineTest || bottomlineResults.summary.empty()) {
        return;
    }
    
    testDir->cd();
    
    // Append numerical results to summary and save as a single string
    std::ostringstream results;
    results << bottomlineResults.summary;
    results << "chi2_smeared " << bottomlineResults.chi2Smeared << "\n";
    results << "pvalue_smeared " << bottomlineResults.pValueSmeared << "\n";
    results << "ndf_smeared " << bottomlineResults.ndfSmeared << "\n";
    results << "chi2_unfolded " << bottomlineResults.chi2Unfolded << "\n";
    results << "pvalue_unfolded " << bottomlineResults.pValueUnfolded << "\n";
    results << "ndf_unfolded " << bottomlineResults.ndfUnfolded << "\n";
    TObjString summary(results.str().c_str());
    summary.Write(("bottomline_summary_" + testLabel).c_str());
    // Save iteration scan results if available
    if (!bottomlineResults.pValueSmearedPerIteration.empty()) {
        saveBottomlineIterationPlots(testDir, testLabel);
    }
}

void OptimizedUnfolder::saveBottomlineIterationPlots(TDirectory* testDir, const std::string& testLabel) {
    testDir->cd();
    
    int nIterations = bottomlineResults.pValueSmearedPerIteration.size();
    if (nIterations == 0) return;
    
    log(LOG_DEBUG, "Saving bottomline iteration plots for " + testLabel);
    
    // Create p-value vs iteration plot
    auto c1 = std::make_unique<TCanvas>("c_pvalue_unfolded_smeared_vs_iterations", 
        "Bottomline Test: p-value vs Iteration", 800, 600);
    c1->SetGridx();
    c1->SetGridy();
    
    // Create graphs
    std::vector<double> iterations(nIterations);
    std::iota(iterations.begin(), iterations.end(), 1.0);
    
    TGraph grSmearedPValue(nIterations, iterations.data(), bottomlineResults.pValueSmearedPerIteration.data());
    grSmearedPValue.SetName(("gr_pvalue_smeared_" + testLabel).c_str());
    grSmearedPValue.SetTitle("p-value vs Iteration;Iteration;p-value");
    grSmearedPValue.SetMarkerStyle(20);
    grSmearedPValue.SetMarkerColor(kBlue);
    grSmearedPValue.SetLineColor(kBlue);
    grSmearedPValue.SetLineWidth(2);
    
    TGraph grUnfoldedPValue(nIterations, iterations.data(), bottomlineResults.pValueUnfoldedPerIteration.data());
    grUnfoldedPValue.SetName(("gr_pvalue_unfolded_" + testLabel).c_str());
    grUnfoldedPValue.SetMarkerStyle(24);
    grUnfoldedPValue.SetMarkerColor(kRed);
    grUnfoldedPValue.SetLineColor(kRed);
    grUnfoldedPValue.SetLineWidth(2);
    
    // Set axis ranges
    double maxPValue = std::max(*std::max_element(bottomlineResults.pValueSmearedPerIteration.begin(), 
                                                  bottomlineResults.pValueSmearedPerIteration.end()),
                               *std::max_element(bottomlineResults.pValueUnfoldedPerIteration.begin(),
                                                 bottomlineResults.pValueUnfoldedPerIteration.end()));
    grSmearedPValue.SetMaximum(std::min(1.0, maxPValue * 1.1));
    grSmearedPValue.SetMinimum(0.0);
    
    grSmearedPValue.Draw("ALP");
    grUnfoldedPValue.Draw("LP SAME");
    
    // Add threshold line
    TLine thresholdLine(1, config.pValueThreshold, nIterations, config.pValueThreshold);
    thresholdLine.SetLineColor(kGreen);
    thresholdLine.SetLineStyle(2);
    thresholdLine.SetLineWidth(2);
    thresholdLine.Draw("SAME");
    
    // Add optimal iteration line
    TLine* optimalLine = nullptr;
    if (bottomlineResults.optimalIteration > 0) {
        optimalLine = new TLine(bottomlineResults.optimalIteration, 0, bottomlineResults.optimalIteration, maxPValue * 1.1);
        optimalLine->SetLineColor(kMagenta);
        optimalLine->SetLineStyle(3);
        optimalLine->SetLineWidth(2);
        optimalLine->Draw("SAME");
    }
    
    // Add legend
    auto legend = std::make_unique<TLegend>(0.65, 0.15, 0.9, 0.4);
    legend->AddEntry(&grSmearedPValue, "Measured space", "lp");
    legend->AddEntry(&grUnfoldedPValue, "Unfolded space", "lp");
    legend->AddEntry(&thresholdLine, ("Threshold (" + std::to_string(config.pValueThreshold) + ")").c_str(), "l");
    if (optimalLine) {
        legend->AddEntry(optimalLine, ("Optimal (iter " + std::to_string(bottomlineResults.optimalIteration) + ")").c_str(), "l");
    }
    legend->Draw();
    
    c1->Write();
    
    // Do not save individual graphs
    
    // Clean up
    if (optimalLine) delete optimalLine;
    
    log(LOG_DEBUG, "Saved bottomline iteration plots for " + testLabel);
}

#endif // OPTIMIZED_UNFOLDER_H
