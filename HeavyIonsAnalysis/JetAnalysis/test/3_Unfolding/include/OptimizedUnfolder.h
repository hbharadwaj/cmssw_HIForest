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
    
public:
    OptimizedUnfolder(const UnfoldConfig& cfg) : config(cfg), histManager(cfg) {
        log(LOG_DEBUG, "Created unfolder for " + std::to_string(cfg.dimension) + "D case");
    }
    
    void fillFromTrees(TTree* dataTree, TTree* mcTree) {
        log(LOG_DEBUG, "Filling histograms from trees");
        fillTree(dataTree, true);   // isData = true
        fillTree(mcTree, false);    // isData = false
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
        log(LOG_INFO, "Unfolding with " + config.method + " method completed");
    }
    
    void saveResults(TDirectory* testDir, const std::string& testLabel, const std::string& details, TH1D* effHist, TMatrixD* covMatrix) {
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
        if (effHist) {
            auto* clone = (TH1*)effHist->Clone(("efficiency_" + testLabel).c_str());
            clone->SetDirectory(testDir);
            clone->Write();
            delete clone;
            log(LOG_DEBUG, "Wrote efficiency_" + testLabel + " to " + testDir->GetPath());
        }
        if (covMatrix) {
            covMatrix->Write(("covariance_" + testLabel).c_str());
            log(LOG_DEBUG, "Wrote covariance_" + testLabel + " to " + testDir->GetPath());
        }
        TObjString checkInfo(details.c_str());
        checkInfo.Write(("unfoldability_check_" + testLabel).c_str());
        log(LOG_DEBUG, "Wrote unfoldability_check_" + testLabel + " to " + testDir->GetPath());
    }
    
    HistogramManager& getHistograms() { return histManager; }
    const UnfoldConfig& getConfig() const { return config; }
    
private:
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
        log(LOG_DEBUG, "Processing " + std::to_string(nEntries) + " entries from " + 
            (isData ? "data" : "MC") + " tree");
        
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
                    histManager.fillMC(std::vector<double>(measValues.begin(), measValues.end()),
                                     std::vector<double>(truthValues.begin(), truthValues.end()), weight);
                }
                validEntries++;
            }
        }
        
        log(LOG_DEBUG, "Filled " + std::to_string(validEntries) + " valid entries");
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
