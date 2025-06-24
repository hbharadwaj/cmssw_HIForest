#ifndef OPTIMIZED_UNFOLDER_H
#define OPTIMIZED_UNFOLDER_H

#include "UnfoldHelpers.h"
#include "UnfoldConfig.h"
#include <memory>
#include <vector>
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
        
        // Setup RooUnfold response
        response = std::make_unique<RooUnfoldResponse>(h_mc_meas_flat, h_mc_truth_flat, histManager.getResponse());
        
        // Get probability matrix from response
        const TMatrixD& prob = response->Mresponse();
        probMatrix = std::make_unique<TMatrixD>(prob);
        
        // Choose unfolding algorithm
        if (config.method == "Bayes") {
            unfoldAlgorithm = std::make_unique<RooUnfoldBayes>(response.get(), h_data_flat, config.iterations);
        } else if (config.method == "SVD") {
            unfoldAlgorithm = std::make_unique<RooUnfoldSvd>(response.get(), h_data_flat, config.iterations);
        } else if (config.method == "BinByBin") {
            unfoldAlgorithm = std::make_unique<RooUnfoldBinByBin>(response.get(), h_data_flat);
        } else {
            unfoldAlgorithm = std::make_unique<RooUnfoldInvert>(response.get(), h_data_flat);
        }
        // Store the unfolded histogram (clone to avoid double-free)
        if (unfoldAlgorithm) {
            TH1* h = unfoldAlgorithm->Hreco();
            if (h) {
                auto* cloned = (TH1*)h->Clone(("h_unfolded_" + config.method).c_str());
                cloned->SetDirectory(0);
                unfoldedHist.reset(cloned);
            } else {
                unfoldedHist.reset();
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
        }
        
        // Write covariance matrix (prioritize internal one)
        if (covMatrix) {
            // Create TH2D histogram directly from the matrix
            TH2D mat_covariance_hist2D(*covMatrix);
            mat_covariance_hist2D.SetName(("covariance_hist_" + testLabel).c_str());
            mat_covariance_hist2D.SetTitle("Covariance Matrix;Bin Number;Bin Number");
            mat_covariance_hist2D.SetDirectory(testDir);
            mat_covariance_hist2D.Write();
            
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
            
            // Also write the matrix in its original format
            // externalCovMatrix->Write(("covariance_matrix_" + testLabel).c_str());
            log(LOG_DEBUG, "Wrote external covariance matrix and histogram for " + testLabel + " to " + testDir->GetPath());
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
                if (isData) {
                    histManager.fillData(std::vector<double>(measValues.begin(), measValues.end()), weight);
                } else {
                    // MC: fill measured and truth, and fill numerator/denominator for purity/efficiency
                    histManager.fillMC(std::vector<double>(measValues.begin(), measValues.end()),
                                     std::vector<double>(truthValues.begin(), truthValues.end()), weight);
                    
                    // Check if event passes detector cuts
                    bool passesDetectorCuts = true;
                    for (int j = 0; j < config.dimension; ++j) {
                        if (measValues[j] < config.measuredBins[j][0] || 
                            measValues[j] >= config.measuredBins[j].back()) {
                            passesDetectorCuts = false;
                            break;
                        }
                    }
                    
                    // Check if event passes truth cuts
                    bool passesTruthCuts = true;
                    for (int j = 0; j < config.dimension; ++j) {
                        if (truthValues[j] < config.truthBins[j][0] || 
                            truthValues[j] >= config.truthBins[j].back()) {
                            passesTruthCuts = false;
                            break;
                        }
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
        
        if (config.dimension == 2) {
            totalBins = ((TH2*)source)->GetNbinsX() * ((TH2*)source)->GetNbinsY();
        } else if (config.dimension == 3) {
            totalBins = ((TH3*)source)->GetNbinsX() * ((TH3*)source)->GetNbinsY() * ((TH3*)source)->GetNbinsZ();
        }
        
        TH1D* flattened = new TH1D(name.c_str(), "Flattened Histogram", totalBins, 0, totalBins);
        
        // Fill flattened histogram
        if (config.dimension == 2) {
            TH2* h2 = (TH2*)source;
            int bin = 1;
            for (int i = 1; i <= h2->GetNbinsX(); ++i) {
                for (int j = 1; j <= h2->GetNbinsY(); ++j) {
                    flattened->SetBinContent(bin, h2->GetBinContent(i, j));
                    flattened->SetBinError(bin, h2->GetBinError(i, j));
                    bin++;
                }
            }
        } else if (config.dimension == 3) {
            TH3* h3 = (TH3*)source;
            int bin = 1;
            for (int i = 1; i <= h3->GetNbinsX(); ++i) {
                for (int j = 1; j <= h3->GetNbinsY(); ++j) {
                    for (int k = 1; k <= h3->GetNbinsZ(); ++k) {
                        flattened->SetBinContent(bin, h3->GetBinContent(i, j, k));
                        flattened->SetBinError(bin, h3->GetBinError(i, j, k));
                        bin++;
                    }
                }
            }
        }
        
        return flattened;
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
};

#endif // OPTIMIZED_UNFOLDER_H
