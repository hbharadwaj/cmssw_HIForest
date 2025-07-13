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
    std::unique_ptr<TMatrixD> covMatrix; // Store covariance matrix after unfolding
    std::unique_ptr<TMatrixD> probMatrix; // Store probability matrix from response
    std::unique_ptr<TH1> purityHist; // Store purity histogram after unfolding
    std::unique_ptr<TH1> efficiencyHist; // Store efficiency histogram after unfolding
    std::unique_ptr<TH1> purityCorrectedDataHist; // Store purity-corrected data histogram
    std::string unfoldingLog; // Store captured RooUnfold output
    
    // Numerator/denominator histograms for purity and efficiency (template logic)
    std::unique_ptr<TH1> purityNum, purityDen, effNum, effDen;
    std::unique_ptr<TH2> probabilityMatrixHist2D; // For saving the probability matrix as TH2D

public:
    OptimizedUnfolder(const UnfoldConfig& cfg) : config(cfg), histManager(cfg) {
        log(LOG_DEBUG, "Created unfolder for " + std::to_string(cfg.dimension) + "D case");
        initializePurityEfficiencyHistograms();
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
            unfoldAlgorithm = std::make_unique<RooUnfoldBayes>(response.get(), h_data_purity_corrected, config.iterations);
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
                        unfoldedHist = convertFlattenedToMultiDim(flattened, "h_unfolded_" + config.method);
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
                // Convert flattened back to multi-dimensional for storage
                purityCorrectedDataHist = convertFlattenedToMultiDim(h_data_purity_corrected, "h_data_purity_corrected");
                delete h_data_purity_corrected;
            } else {
                purityCorrectedDataHist.reset((TH1*)h_data_purity_corrected->Clone("h_data_purity_corrected"));
                purityCorrectedDataHist->SetDirectory(0);
                delete h_data_purity_corrected;
            }
        }
        
        // Get covariance matrix from unfolding algorithm
        if (unfoldAlgorithm) {
            const TMatrixD& cov = unfoldAlgorithm->Ereco();
            covMatrix = std::make_unique<TMatrixD>(cov);
        }
        
        log(LOG_INFO, "Unfolding with " + config.method + " method completed");
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
    
    std::unique_ptr<TH1> convertFlattenedToMultiDim(TH1D* flattened, const std::string& name) {
        if (config.dimension == 2) {
            // Create 2D histogram with truth binning
            auto hist2D = std::make_unique<TH2D>(name.c_str(), "Unfolded 2D Histogram",
                                               config.truthBins[0].size()-1, config.truthBins[0].data(),
                                               config.truthBins[1].size()-1, config.truthBins[1].data());
            hist2D->SetDirectory(0);
            
            std::vector<int> nBins = {hist2D->GetNbinsX(), hist2D->GetNbinsY()};
            log(LOG_DEBUG, "convertFlattenedToMultiDim: " + name + " - 1D -> 2D (" + 
                std::to_string(nBins[0]) + "x" + std::to_string(nBins[1]) + ")");
            
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
            // Create 3D histogram with truth binning
            auto hist3D = std::make_unique<TH3D>(name.c_str(), "Unfolded 3D Histogram",
                                               config.truthBins[0].size()-1, config.truthBins[0].data(),
                                               config.truthBins[1].size()-1, config.truthBins[1].data(),
                                               config.truthBins[2].size()-1, config.truthBins[2].data());
            hist3D->SetDirectory(0);
            
            std::vector<int> nBins = {hist3D->GetNbinsX(), hist3D->GetNbinsY(), hist3D->GetNbinsZ()};
            log(LOG_DEBUG, "convertFlattenedToMultiDim: " + name + " - 1D -> 3D (" + 
                std::to_string(nBins[0]) + "x" + std::to_string(nBins[1]) + "x" + std::to_string(nBins[2]) + ")");
            
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
            testHist = std::make_unique<TH2D>("test", "Test", 
                nBinsMeas[0], 0, nBinsMeas[0], nBinsMeas[1], 0, nBinsMeas[1]);
            TH2D* h2 = (TH2D*)testHist.get();
            
            // Fill with a pattern: bin content = flat index
            for (int i = 1; i <= nBinsMeas[0]; ++i) {
                for (int j = 1; j <= nBinsMeas[1]; ++j) {
                    std::vector<int> indices = {i-1, j-1};
                    int expectedValue = flattenIndices(indices, nBinsMeas);
                    h2->SetBinContent(i, j, expectedValue);
                }
            }
        }
        
        if (testHist) {
            // Test conversion: ND -> 1D -> ND
            auto flattened = std::unique_ptr<TH1D>(createFlattenedHistogram(testHist.get(), "test_flat"));
            auto recovered = convertFlattenedToMultiDim(flattened.get(), "test_recovered");
            
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
};

#endif // OPTIMIZED_UNFOLDER_H
