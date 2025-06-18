// RooUnfold-based Jet Substructure Unfolding for CMS Heavy Ion Analysis
// Author: CMS Heavy Ion Team
// Date: June 2025
// 
// This macro performs 1D, 2D, and 3D unfolding of jet substructure observables
// using RooUnfold framework for CMS Heavy Ion gamma-jet analysis.
//
// Usage: 
// 1. Setup CMSSW environment: cmsenv (to get ROOT 6.26.11 with RooUnfold)
// 2. root -l
// 3. .x RooUnfoldJetSubstructure.C("../configs/UnfoldJetSub_xj_test.config")
//
// Requirements:
// - ROOT 6.26.11 or earlier
// - RooUnfold 2.0.0
// - CMSSW environment for compatible ROOT version

// Setup RooUnfold paths if compiling
#ifdef __ACLIC__
R__ADD_INCLUDE_PATH(./RooUnfold/src)
R__LOAD_LIBRARY(./RooUnfold/libRooUnfold.so)
#endif

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <sstream>
#include <algorithm>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TH3D.h>
#include <TEnv.h>
#include <TSystem.h>
#include <TStopwatch.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <THashList.h>
#include <RooUnfold.h>
#include <RooUnfoldResponse.h>
#include <RooUnfoldBayes.h>
#include <RooUnfoldSvd.h>
#include <RooUnfoldBinByBin.h>
#include <RooUnfoldInvert.h>
#include <TGaxis.h>
#include "./include/UnfoldHelpers.h"
#include <TBranch.h>
#include <THn.h>

// Use logging from UnfoldHelpers.h - no duplicate definitions needed

// Structure to hold unfolding binning
struct UnfoldingBins {
    std::vector<double> jetPt = {40, 80, 120, 200};
    std::vector<double> jetGirth = {0.0, 0.02, 0.04, 0.06, 0.08, 0.1, 0.12};
    std::vector<double> photonEt = {80, 100, 150, 500};
    
    int nJetPt() const { return jetPt.size() - 1; }
    int nJetGirth() const { return jetGirth.size() - 1; }
    int nPhotonEt() const { return photonEt.size() - 1; }
    
    void print() const {
        log(LOG_INFO, "Binning configuration:");
        log(LOG_INFO, "  Jet pT bins: " + std::to_string(nJetPt()) + " bins");
        log(LOG_INFO, "  Jet girth bins: " + std::to_string(nJetGirth()) + " bins");
        log(LOG_INFO, "  Photon ET bins: " + std::to_string(nPhotonEt()) + " bins");
    }
};

// Helper function to get global bin index for 2D case
int getGlobalBin2D(int iBinX, int iBinY, int nBinsX) {
    return iBinY * nBinsX + iBinX;
}

// Helper function to get global bin index for 3D case
int getGlobalBin3D(int iBinX, int iBinY, int iBinZ, int nBinsX, int nBinsY) {
    return iBinZ * (nBinsX * nBinsY) + iBinY * nBinsX + iBinX;
}

// Generic N-dimensional Unfolder (replaces all hardcoded classes)
// Edit 2: Finalize GenericUnfolderND class for full config-driven operation
class GenericUnfolderND {
private:
    std::vector<std::string> measuredVars;
    std::vector<std::string> truthVars;
    std::vector<std::vector<double>> measuredBins;
    std::vector<std::vector<double>> truthBins;
    int ndim;
    RooUnfoldResponse* response;
    RooUnfold* unfold;
    
    // Regular histograms instead of THnD
    TH1D* h_measured_data_1d;
    TH1D* h_measured_mc_1d;
    TH1D* h_truth_mc_1d;
    TH2D* h_response_2d;
    
    TH2D* h_measured_data_2d;
    TH2D* h_measured_mc_2d;
    TH2D* h_truth_mc_2d;
    
    TH3D* h_measured_data_3d;
    TH3D* h_measured_mc_3d;
    TH3D* h_truth_mc_3d;
public:
    GenericUnfolderND(const std::vector<std::string>& mvars, const std::vector<std::vector<double>>& mbins,
                     const std::vector<std::string>& tvars, const std::vector<std::vector<double>>& tbins)
        : measuredVars(mvars), measuredBins(mbins), truthVars(tvars), truthBins(tbins), ndim(mvars.size()),
          response(nullptr), unfold(nullptr), 
          h_measured_data_1d(nullptr), h_measured_mc_1d(nullptr), h_truth_mc_1d(nullptr), h_response_2d(nullptr),
          h_measured_data_2d(nullptr), h_measured_mc_2d(nullptr), h_truth_mc_2d(nullptr),
          h_measured_data_3d(nullptr), h_measured_mc_3d(nullptr), h_truth_mc_3d(nullptr) {
        
        log(LOG_INFO, "Creating " + std::to_string(ndim) + "-dimensional histograms");
        
        // Log the bin edges for debugging
        for (int i = 0; i < ndim; ++i) {
            std::string binInfo = "  Measured bins for " + measuredVars[i] + ": ";
            for (size_t j = 0; j < measuredBins[i].size(); ++j) 
                binInfo += std::to_string(measuredBins[i][j]) + " ";
            log(LOG_INFO, binInfo);
            
            std::string truthBinInfo = "  Truth bins for " + truthVars[i] + ": ";
            for (size_t j = 0; j < truthBins[i].size(); ++j) 
                truthBinInfo += std::to_string(truthBins[i][j]) + " ";
            log(LOG_INFO, truthBinInfo);
        }
        
        // Create histograms based on dimension
        if (ndim == 1) {
            // 1D histograms
            // Create histogram titles with .c_str() to convert std::string to const char*
            std::string dataTitle = "Measured Data 1D;" + measuredVars[0] + ";Events";
            std::string mcTitle = "Measured MC 1D;" + measuredVars[0] + ";Events";
            std::string truthTitle = "Truth MC 1D;" + truthVars[0] + ";Events";
            
            h_measured_data_1d = new TH1D("h_measured_data_1d", dataTitle.c_str(), 
                                       measuredBins[0].size()-1, &(measuredBins[0][0]));
            h_measured_mc_1d = new TH1D("h_measured_mc_1d", mcTitle.c_str(), 
                                      measuredBins[0].size()-1, &(measuredBins[0][0]));
            h_truth_mc_1d = new TH1D("h_truth_mc_1d", truthTitle.c_str(), 
                                   truthBins[0].size()-1, &(truthBins[0][0]));
            
            // 2D response matrix (reco vs truth)
            std::string responseTitle = "Response Matrix;" + truthVars[0] + ";" + measuredVars[0];
            h_response_2d = new TH2D("h_response_2d", responseTitle.c_str(), 
                                    truthBins[0].size()-1, &(truthBins[0][0]),
                                    measuredBins[0].size()-1, &(measuredBins[0][0]));
            
            log(LOG_INFO, "Created 1D histograms with " + std::to_string(h_measured_data_1d->GetNbinsX()) + " bins");
        } 
        else if (ndim == 2) {
            // 2D histograms - convert std::string to const char*
            std::string data2dTitle = "Measured Data 2D;" + measuredVars[0] + ";" + measuredVars[1];
            std::string mc2dTitle = "Measured MC 2D;" + measuredVars[0] + ";" + measuredVars[1];
            std::string truth2dTitle = "Truth MC 2D;" + truthVars[0] + ";" + truthVars[1];
            
            h_measured_data_2d = new TH2D("h_measured_data_2d", data2dTitle.c_str(), 
                                       measuredBins[0].size()-1, &(measuredBins[0][0]),
                                       measuredBins[1].size()-1, &(measuredBins[1][0]));
            h_measured_mc_2d = new TH2D("h_measured_mc_2d", mc2dTitle.c_str(), 
                                      measuredBins[0].size()-1, &(measuredBins[0][0]),
                                      measuredBins[1].size()-1, &(measuredBins[1][0]));
            h_truth_mc_2d = new TH2D("h_truth_mc_2d", truth2dTitle.c_str(), 
                                   truthBins[0].size()-1, &(truthBins[0][0]),
                                   truthBins[1].size()-1, &(truthBins[1][0]));
            
            // 2D response matrix (flattened)
            int nBinsReco = (measuredBins[0].size()-1) * (measuredBins[1].size()-1);
            int nBinsTruth = (truthBins[0].size()-1) * (truthBins[1].size()-1);
            std::string response2dTitle = "Response Matrix;Truth Bin;Measured Bin";
            h_response_2d = new TH2D("h_response_2d", response2dTitle.c_str(), 
                                    nBinsTruth, 0, nBinsTruth, 
                                    nBinsReco, 0, nBinsReco);
            
            log(LOG_INFO, "Created 2D histograms with " + 
                std::to_string(h_measured_data_2d->GetNbinsX()) + "x" + 
                std::to_string(h_measured_data_2d->GetNbinsY()) + " bins");
        }
        else if (ndim == 3) {
            // 3D histograms - convert std::string to const char*
            std::string data3dTitle = "Measured Data 3D;" + measuredVars[0] + ";" + measuredVars[1] + ";" + measuredVars[2];
            std::string mc3dTitle = "Measured MC 3D;" + measuredVars[0] + ";" + measuredVars[1] + ";" + measuredVars[2];
            std::string truth3dTitle = "Truth MC 3D;" + truthVars[0] + ";" + truthVars[1] + ";" + truthVars[2];
            
            h_measured_data_3d = new TH3D("h_measured_data_3d", data3dTitle.c_str(), 
                                       measuredBins[0].size()-1, &(measuredBins[0][0]),
                                       measuredBins[1].size()-1, &(measuredBins[1][0]),
                                       measuredBins[2].size()-1, &(measuredBins[2][0]));
            h_measured_mc_3d = new TH3D("h_measured_mc_3d", mc3dTitle.c_str(), 
                                      measuredBins[0].size()-1, &(measuredBins[0][0]),
                                      measuredBins[1].size()-1, &(measuredBins[1][0]),
                                      measuredBins[2].size()-1, &(measuredBins[2][0]));
            h_truth_mc_3d = new TH3D("h_truth_mc_3d", truth3dTitle.c_str(), 
                                   truthBins[0].size()-1, &(truthBins[0][0]),
                                   truthBins[1].size()-1, &(truthBins[1][0]),
                                   truthBins[2].size()-1, &(truthBins[2][0]));
            // 2D response matrix (flattened)
            int nBinsReco = (measuredBins[0].size()-1) * (measuredBins[1].size()-1) * (measuredBins[2].size()-1);
            int nBinsTruth = (truthBins[0].size()-1) * (truthBins[1].size()-1) * (truthBins[2].size()-1);
            std::string response3dTitle = "Response Matrix;Truth Bin;Measured Bin";
            h_response_2d = new TH2D("h_response_2d", response3dTitle.c_str(), 
                                    nBinsTruth, 0, nBinsTruth, 
                                    nBinsReco, 0, nBinsReco);
            
            log(LOG_INFO, "Created 3D histograms with " + 
                std::to_string(h_measured_data_3d->GetNbinsX()) + "x" + 
                std::to_string(h_measured_data_3d->GetNbinsY()) + "x" + 
                std::to_string(h_measured_data_3d->GetNbinsZ()) + " bins");
        } else {
            log(LOG_ERROR, "Unsupported dimension: " + std::to_string(ndim));
        }
    }
    void fillFromTree(TTree* tree, bool isMC, bool isData, const std::string& weightBranch) {
        if (!tree) {
            log(LOG_ERROR, "Null tree pointer provided to fillFromTree");
            return;
        }
        
        // Print the actual branches in the tree
        log(LOG_INFO, "Tree branches available:");
        TObjArray* branches = tree->GetListOfBranches();
        for (int i = 0; i < std::min(20, branches->GetEntries()); ++i) {
            log(LOG_INFO, "  - " + std::string(branches->At(i)->GetName()));
        }
        if (branches->GetEntries() > 20) {
            log(LOG_INFO, "  ... and " + std::to_string(branches->GetEntries() - 20) + " more branches");
        }
        
        std::vector<float> mvars_f(ndim, 0), tvars_f(ndim, 0);
        
        log(LOG_INFO, "Setting branch addresses for measured variables:");
        for (int i = 0; i < ndim; ++i) {
            log(LOG_INFO, "  - " + measuredVars[i]);
            TBranch* branch = tree->GetBranch(measuredVars[i].c_str());
            if (!branch) {
                log(LOG_ERROR, "Branch not found: " + measuredVars[i]);
                return;
            }
            tree->SetBranchAddress(measuredVars[i].c_str(), &mvars_f[i]);
        }
        
        if (isMC) {
            log(LOG_INFO, "Setting branch addresses for truth variables:");
            for (int i = 0; i < ndim; ++i) {
                log(LOG_INFO, "  - " + truthVars[i]);
                TBranch* branch = tree->GetBranch(truthVars[i].c_str());
                if (!branch) {
                    log(LOG_ERROR, "Branch not found: " + truthVars[i]);
                    return;
                }
                tree->SetBranchAddress(truthVars[i].c_str(), &tvars_f[i]);
            }
        }
        
        float eventWeight = 1.0;
        TBranch* weightBr = tree->GetBranch(weightBranch.c_str());
        if (weightBr) {
            tree->SetBranchAddress(weightBranch.c_str(), &eventWeight);
            log(LOG_INFO, "Using weight branch: " + weightBranch);
        } else {
            log(LOG_WARNING, "Weight branch '" + weightBranch + "' not found. Using weight=1.0");
        }

        Long64_t nEntries = tree->GetEntries();
        log(LOG_INFO, "Processing " + std::to_string(nEntries) + " entries");
        
        int validEntries = 0;
        double sumWeights = 0.0;
        
        for (Long64_t i = 0; i < nEntries; ++i) {
            tree->GetEntry(i);
            
            // Log some values periodically to check data
            if (i < 5 || i % 1000 == 0) {
                std::string valueStr = "Entry " + std::to_string(i) + " values:";
                for (int j = 0; j < ndim; ++j) {
                    valueStr += " " + measuredVars[j] + "=" + std::to_string(mvars_f[j]);
                }
                valueStr += " weight=" + std::to_string(eventWeight);
                log(LOG_INFO, valueStr);
            }
            
            // Check for valid values (non-NaN, non-Inf)
            bool validMeas = true, validTruth = true;
            for (int j = 0; j < ndim; ++j) {
                if (std::isnan(mvars_f[j]) || std::isinf(mvars_f[j])) {
                    validMeas = false;
                    break;
                }
            }
            
            if (isMC) {
                for (int j = 0; j < ndim; ++j) {
                    if (std::isnan(tvars_f[j]) || std::isinf(tvars_f[j])) {
                        validTruth = false;
                        break;
                    }
                }
            }
            
            // Fill appropriate histograms based on dimension
            if (validMeas) {
                if (ndim == 1) {
                    if (isData) {
                        h_measured_data_1d->Fill(mvars_f[0], eventWeight);
                    } else {
                        h_measured_mc_1d->Fill(mvars_f[0], eventWeight);
                    }
                    validEntries++;
                    sumWeights += eventWeight;
                } else if (ndim == 2) {
                    if (isData) {
                        h_measured_data_2d->Fill(mvars_f[0], mvars_f[1], eventWeight);
                    } else {
                        h_measured_mc_2d->Fill(mvars_f[0], mvars_f[1], eventWeight);
                    }
                    validEntries++;
                    sumWeights += eventWeight;
                } else if (ndim == 3) {
                    if (isData) {
                        h_measured_data_3d->Fill(mvars_f[0], mvars_f[1], mvars_f[2], eventWeight);
                    } else {
                        h_measured_mc_3d->Fill(mvars_f[0], mvars_f[1], mvars_f[2], eventWeight);
                    }
                    validEntries++;
                    sumWeights += eventWeight;
                }
            }
            
            // Fill truth and response histograms for MC
            if (isMC && validMeas && validTruth) {
                if (ndim == 1) {
                    h_truth_mc_1d->Fill(tvars_f[0], eventWeight);
                    h_response_2d->Fill(tvars_f[0], mvars_f[0], eventWeight);
                } else if (ndim == 2) {
                    h_truth_mc_2d->Fill(tvars_f[0], tvars_f[1], eventWeight);
                    
                    // Calculate global bins for flattened 2D response
                    int truthBin = h_truth_mc_2d->FindBin(tvars_f[0], tvars_f[1]) - 1;
                    int measBin = h_measured_mc_2d->FindBin(mvars_f[0], mvars_f[1]) - 1;
                    h_response_2d->Fill(truthBin, measBin, eventWeight);
                } else if (ndim == 3) {
                    h_truth_mc_3d->Fill(tvars_f[0], tvars_f[1], tvars_f[2], eventWeight);
                    
                    // Calculate global bins for flattened 3D response
                    int truthBin = h_truth_mc_3d->FindBin(tvars_f[0], tvars_f[1], tvars_f[2]) - 1;
                    int measBin = h_measured_mc_3d->FindBin(mvars_f[0], mvars_f[1], mvars_f[2]) - 1;
                    h_response_2d->Fill(truthBin, measBin, eventWeight);
                }
            }
        }
        
        log(LOG_INFO, "Valid entries: " + std::to_string(validEntries) + " out of " + 
            std::to_string(nEntries) + " (sum of weights: " + std::to_string(sumWeights) + ")");
        
        // Log histogram stats depending on dimension
        if (ndim == 1) {
            if (isData) {
                log(LOG_INFO, "Data histogram entries: " + std::to_string(h_measured_data_1d->GetEntries()) + 
                    ", integral: " + std::to_string(h_measured_data_1d->Integral()));
            } else {
                log(LOG_INFO, "MC measured histogram entries: " + std::to_string(h_measured_mc_1d->GetEntries()) + 
                    ", integral: " + std::to_string(h_measured_mc_1d->Integral()));
            }
            
            if (isMC) {
                log(LOG_INFO, "Truth histogram entries: " + std::to_string(h_truth_mc_1d->GetEntries()) + 
                    ", integral: " + std::to_string(h_truth_mc_1d->Integral()));
                log(LOG_INFO, "Response histogram entries: " + std::to_string(h_response_2d->GetEntries()) + 
                    ", integral: " + std::to_string(h_response_2d->Integral()));
            }
        } else if (ndim == 2) {
            if (isData) {
                log(LOG_INFO, "Data histogram entries: " + std::to_string(h_measured_data_2d->GetEntries()) + 
                    ", integral: " + std::to_string(h_measured_data_2d->Integral()));
            } else {
                log(LOG_INFO, "MC measured histogram entries: " + std::to_string(h_measured_mc_2d->GetEntries()) + 
                    ", integral: " + std::to_string(h_measured_mc_2d->Integral()));
            }
            
            if (isMC) {
                log(LOG_INFO, "Truth histogram entries: " + std::to_string(h_truth_mc_2d->GetEntries()) + 
                    ", integral: " + std::to_string(h_truth_mc_2d->Integral()));
                log(LOG_INFO, "Response histogram entries: " + std::to_string(h_response_2d->GetEntries()) + 
                    ", integral: " + std::to_string(h_response_2d->Integral()));
            }
        } else if (ndim == 3) {
            if (isData) {
                log(LOG_INFO, "Data histogram entries: " + std::to_string(h_measured_data_3d->GetEntries()) + 
                    ", integral: " + std::to_string(h_measured_data_3d->Integral()));
            } else {
                log(LOG_INFO, "MC measured histogram entries: " + std::to_string(h_measured_mc_3d->GetEntries()) + 
                    ", integral: " + std::to_string(h_measured_mc_3d->Integral()));
            }
            
            if (isMC) {
                log(LOG_INFO, "Truth histogram entries: " + std::to_string(h_truth_mc_3d->GetEntries()) + 
                    ", integral: " + std::to_string(h_truth_mc_3d->Integral()));
                log(LOG_INFO, "Response histogram entries: " + std::to_string(h_response_2d->GetEntries()) + 
                    ", integral: " + std::to_string(h_response_2d->Integral()));
            }
        }
    }
    void performUnfolding(const std::string& method, int nIter) {
        if (ndim == 0) {
            log(LOG_ERROR, "Cannot unfold with dimension 0. No dimensions set.");
            return;
        }
        
        // Create RooUnfoldResponse object based on dimension
        response = new RooUnfoldResponse();
        
        if (ndim == 1) {
            log(LOG_INFO, "Setting up 1D response matrix");
            response->Setup(h_measured_mc_1d, h_truth_mc_1d, h_response_2d);
            
            // Choose unfolding method
            if (method == "Bayes") {
                unfold = new RooUnfoldBayes(response, h_measured_data_1d, nIter);
                log(LOG_INFO, "Using Bayesian unfolding with " + std::to_string(nIter) + " iterations");
            } else if (method == "SVD") {
                unfold = new RooUnfoldSvd(response, h_measured_data_1d, nIter);
                log(LOG_INFO, "Using SVD unfolding with kterm=" + std::to_string(nIter));
            } else if (method == "BinByBin") {
                unfold = new RooUnfoldBinByBin(response, h_measured_data_1d);
                log(LOG_INFO, "Using bin-by-bin unfolding");
            } else { // Invert or MatrixInversion
                unfold = new RooUnfoldInvert(response, h_measured_data_1d);
                log(LOG_INFO, "Using matrix inversion unfolding");
            }
        } else if (ndim == 2) {
            log(LOG_INFO, "Setting up 2D response matrix");
            // Convert to TH1D (flattened)
            TH1D* h_meas_data_flat = new TH1D("h_meas_data_flat", "Flattened Measured Data", 
                                           h_measured_data_2d->GetNcells(), 0, h_measured_data_2d->GetNcells());
            TH1D* h_meas_mc_flat = new TH1D("h_meas_mc_flat", "Flattened Measured MC", 
                                         h_measured_mc_2d->GetNcells(), 0, h_measured_mc_2d->GetNcells());
            TH1D* h_truth_mc_flat = new TH1D("h_truth_mc_flat", "Flattened Truth MC", 
                                          h_truth_mc_2d->GetNcells(), 0, h_truth_mc_2d->GetNcells());
            
            // Fill flattened histograms
            for (int i = 1; i <= h_measured_data_2d->GetNbinsX(); ++i) {
                for (int j = 1; j <= h_measured_data_2d->GetNbinsY(); ++j) {
                    int bin = (i-1) * h_measured_data_2d->GetNbinsY() + j;
                    h_meas_data_flat->SetBinContent(bin, h_measured_data_2d->GetBinContent(i, j));
                    h_meas_mc_flat->SetBinContent(bin, h_measured_mc_2d->GetBinContent(i, j));
                }
            }
            
            for (int i = 1; i <= h_truth_mc_2d->GetNbinsX(); ++i) {
                for (int j = 1; j <= h_truth_mc_2d->GetNbinsY(); ++j) {
                    int bin = (i-1) * h_truth_mc_2d->GetNbinsY() + j;
                    h_truth_mc_flat->SetBinContent(bin, h_truth_mc_2d->GetBinContent(i, j));
                }
            }
            
            response->Setup(h_meas_mc_flat, h_truth_mc_flat, h_response_2d);
            
            // Choose unfolding method
            if (method == "Bayes") {
                unfold = new RooUnfoldBayes(response, h_meas_data_flat, nIter);
                log(LOG_INFO, "Using Bayesian unfolding with " + std::to_string(nIter) + " iterations");
            } else if (method == "SVD") {
                unfold = new RooUnfoldSvd(response, h_meas_data_flat, nIter);
                log(LOG_INFO, "Using SVD unfolding with kterm=" + std::to_string(nIter));
            } else if (method == "BinByBin") {
                unfold = new RooUnfoldBinByBin(response, h_meas_data_flat);
                log(LOG_INFO, "Using bin-by-bin unfolding");
            } else { // Invert or MatrixInversion
                unfold = new RooUnfoldInvert(response, h_meas_data_flat);
                log(LOG_INFO, "Using matrix inversion unfolding");
            }
        } else if (ndim == 3) {
            log(LOG_INFO, "Setting up 3D response matrix");
            // Convert to TH1D (flattened)
            TH1D* h_meas_data_flat = new TH1D("h_meas_data_flat", "Flattened Measured Data", 
                                           h_measured_data_3d->GetNcells(), 0, h_measured_data_3d->GetNcells());
            TH1D* h_meas_mc_flat = new TH1D("h_meas_mc_flat", "Flattened Measured MC", 
                                         h_measured_mc_3d->GetNcells(), 0, h_measured_mc_3d->GetNcells());
            TH1D* h_truth_mc_flat = new TH1D("h_truth_mc_flat", "Flattened Truth MC", 
                                          h_truth_mc_3d->GetNcells(), 0, h_truth_mc_3d->GetNcells());
            
            // Fill flattened histograms (more complex for 3D)
            for (int i = 1; i <= h_measured_data_3d->GetNbinsX(); ++i) {
                for (int j = 1; j <= h_measured_data_3d->GetNbinsY(); ++j) {
                    for (int k = 1; k <= h_measured_data_3d->GetNbinsZ(); ++k) {
                        int bin = (i-1) * h_measured_data_3d->GetNbinsY() * h_measured_data_3d->GetNbinsZ() + 
                                 (j-1) * h_measured_data_3d->GetNbinsZ() + k;
                        h_meas_data_flat->SetBinContent(bin, h_measured_data_3d->GetBinContent(i, j, k));
                        h_meas_mc_flat->SetBinContent(bin, h_measured_mc_3d->GetBinContent(i, j, k));
                    }
                }
            }
            
            for (int i = 1; i <= h_truth_mc_3d->GetNbinsX(); ++i) {
                for (int j = 1; j <= h_truth_mc_3d->GetNbinsY(); ++j) {
                    for (int k = 1; k <= h_truth_mc_3d->GetNbinsZ(); ++k) {
                        int bin = (i-1) * h_truth_mc_3d->GetNbinsY() * h_truth_mc_3d->GetNbinsZ() + 
                                 (j-1) * h_truth_mc_3d->GetNbinsZ() + k;
                        h_truth_mc_flat->SetBinContent(bin, h_truth_mc_3d->GetBinContent(i, j, k));
                    }
                }
            }
            
            response->Setup(h_meas_mc_flat, h_truth_mc_flat, h_response_2d);
            
            // Choose unfolding method
            if (method == "Bayes") {
                unfold = new RooUnfoldBayes(response, h_meas_data_flat, nIter);
                log(LOG_INFO, "Using Bayesian unfolding with " + std::to_string(nIter) + " iterations");
            } else if (method == "SVD") {
                unfold = new RooUnfoldSvd(response, h_meas_data_flat, nIter);
                log(LOG_INFO, "Using SVD unfolding with kterm=" + std::to_string(nIter));
            } else if (method == "BinByBin") {
                unfold = new RooUnfoldBinByBin(response, h_meas_data_flat);
                log(LOG_INFO, "Using bin-by-bin unfolding");
            } else { // Invert or MatrixInversion
                unfold = new RooUnfoldInvert(response, h_meas_data_flat);
                log(LOG_INFO, "Using matrix inversion unfolding");
            }
        }
        
        // Set options and perform unfolding
        unfold->SetVerbose(1);
        TH1* h_unfolded = (TH1*)unfold->Hreco();  // Hreco() is the correct method name in RooUnfold
        if (!h_unfolded) {
            log(LOG_ERROR, "Unfolding failed, no histogram returned");
        } else {
            log(LOG_INFO, "Unfolding successful, got histogram with " + 
                std::to_string(h_unfolded->GetEntries()) + " entries and integral " + 
                std::to_string(h_unfolded->Integral()));
        }
    }
    
    // Generic getters based on dimension
    TObject* getMeasuredData() { 
        if (ndim == 1) return h_measured_data_1d;
        if (ndim == 2) return h_measured_data_2d;
        if (ndim == 3) return h_measured_data_3d;
        return nullptr;
    }
    
    TObject* getMeasuredMC() { 
        if (ndim == 1) return h_measured_mc_1d;
        if (ndim == 2) return h_measured_mc_2d;
        if (ndim == 3) return h_measured_mc_3d;
        return nullptr;
    }
    
    TObject* getTruthMC() { 
        if (ndim == 1) return h_truth_mc_1d;
        if (ndim == 2) return h_truth_mc_2d;
        if (ndim == 3) return h_truth_mc_3d;
        return nullptr;
    }
    
    TObject* getResponse() { return h_response_2d; }
    // Flatten N-dimensional response to TH2D for plotting (since we already use TH2D, this is simplified)
    TH2D* flattenResponse() {
        // We're already using TH2D for the response matrix, simply clone it
        if (h_response_2d) {
            return (TH2D*)h_response_2d->Clone("h_response_flat");
        }
        
        // Fallback if response matrix is null
        int nTruth = 1, nMeas = 1;
        for (int i = 0; i < ndim; ++i) nTruth *= truthBins[i].size()-1;
        for (int i = 0; i < ndim; ++i) nMeas *= measuredBins[i].size()-1;
        
        return new TH2D("h_response_flat", 
                       "Response Matrix (flattened);Global Truth Bin;Global Measured Bin", 
                       nTruth, 0, nTruth, nMeas, 0, nMeas);
    }
    ~GenericUnfolderND() { delete unfold; delete response; }
};


// ---
// FLATTENING AND AXIS STRUCTURE FOR MULTI-DIMENSIONAL RESPONSE MATRICES
// For 2D: globalBin = iy * nJetPt + ix
//   - X axis: global bin number (0 ... nJetPt*nJetGirth-1)
//   - Additional axes: first for jetPt (repeats for each girth), second for jetGirth (cycles for each jetPt)
// For 3D: globalBin = iz * (nJetPt*nJetGirth) + iy * nJetPt + ix
//   - X axis: global bin number (0 ... nPhotonEt*nJetPt*nJetGirth-1)
//   - Additional axes: photonEt (repeats for each jetPt/girth), jetPt (repeats for each girth, cycles for photonEt), girth (cycles fastest)
// ---

// Helper: Parse all Unfold* sets from TEnv config
std::vector<std::string> getUnfoldSetNames(TEnv* config) {
    std::vector<std::string> sets;
    std::set<std::string> found;
    
    log(LOG_INFO, "Searching for unfolding sets in config...");
    
    // Get the config file path
    std::string configFile = config->GetValue("InputConfigFile", "../configs/UnfoldJetSub_xj_test.config");
    
    // Read the file line by line directly
    log(LOG_INFO, "Reading config file: " + configFile);
    std::ifstream infile(configFile.c_str());
    if (!infile.is_open()) {
        log(LOG_ERROR, "Failed to open config file: " + configFile);
        return sets;
    }
    
    std::string line;
    while (std::getline(infile, line)) {
        // Skip comments, empty lines
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // Look for lines with "UnfoldingDimension" - these define unfolding sets
        size_t dimPos = line.find(".UnfoldingDimension");
        if (dimPos != std::string::npos && line.substr(0, 6) == "Unfold") {
            std::string prefix = line.substr(0, dimPos);
            if (found.insert(prefix).second) {
                sets.push_back(prefix);
                log(LOG_INFO, "Found unfolding set: " + prefix);
            }
        }
    }
    
    log(LOG_INFO, "Found " + std::to_string(sets.size()) + " unfolding sets to process.");
    for (const auto& set : sets) {
        log(LOG_INFO, "  - " + set);
    }
    
    return sets;
}

// Helper: Parse int vector from string
std::vector<int> parseIntVec(const std::string& s) {
    std::vector<int> v;
    std::stringstream ss(s);
    int x;
    while (ss >> x) v.push_back(x);
    return v;
}

// Forward declaration and simple implementation of plotResponseMatrix
void plotResponseMatrix(const std::string& set, GenericUnfolderND& unfolder, const std::vector<std::vector<double>>& measuredBins, const std::vector<std::string>& measuredVars) {
    log(LOG_INFO, "Creating response matrix plot for set: " + set);
    TCanvas* c_resp = new TCanvas((set+"_response_matrix").c_str(), (set+" Response Matrix").c_str(), 900, 800);
    c_resp->cd();
    
    // Get the response matrix
    TH2D* h_response = (TH2D*)unfolder.getResponse();
    if (h_response) {
        h_response->Draw("COLZ");
        h_response->SetStats(0);
        h_response->SetTitle(("Response Matrix for " + set).c_str());
        c_resp->SetLogz();
    } else {
        log(LOG_WARNING, "No response matrix available for plotting");
        // Create a simple placeholder
        TH2D* h_simple = new TH2D((set+"_response_simple").c_str(), "Response Matrix (Placeholder)", 10, 0, 10, 10, 0, 10);
        h_simple->Draw("COLZ");
    }
    
    c_resp->Write();
}

// Edit 4: Refactor main function to use only generic logic and config-driven workflow
void RooUnfoldJetSubstructure(const char* configFile = "../configs/UnfoldJetSub_xj_test.config") {
    TStopwatch timer;
    timer.Start();
    log(LOG_INFO, "=== RooUnfold Jet Substructure Unfolding ===");
    log(LOG_INFO, "Loading configuration from: " + std::string(configFile));
    
    // Explicitly set verbosity high for debugging
    gVerbosity = LOG_DEBUG;
    
    TEnv* config = new TEnv(configFile);
    // Store the config file path in the TEnv for later use
    config->SetValue("InputConfigFile", configFile);
    
    gVerbosity = config->GetValue("default.Verbosity", 2);
    
    // Ensure file exists
    if (gSystem->AccessPathName(configFile)) {
        log(LOG_ERROR, "Config file not found: " + std::string(configFile));
        return;
    }
    
    std::vector<std::string> unfoldSets = getUnfoldSetNames(config);
    if (unfoldSets.empty()) {
        log(LOG_WARNING, "No unfolding sets found in config file. Check your configuration.");
    }
    
    const char* outputDir = config->GetValue("default.OutputDir", "./");
    const char* outputPrefix = config->GetValue("default.OutputPrefix", "roounfold");
    gSystem->mkdir(outputDir, kTRUE);
    std::string outputPath = std::string(outputDir) + "/" + std::string(outputPrefix) + "_roounfold.root";
    TFile* outFile = TFile::Open(outputPath.c_str(), "RECREATE");
    if (!outFile || outFile->IsZombie()) {
        log(LOG_ERROR, "Could not create output file: " + outputPath);
        delete config;
        return;
    }
    // Ensure all main directories exist
    outFile->mkdir("Unfolding1D");
    outFile->mkdir("Unfolding2D");
    outFile->mkdir("Unfolding3D");
    for (const auto& set : unfoldSets) {
        log(LOG_INFO, "Processing unfolding set: " + set);
        std::string prefix = set + ".";
        
        // Debug the retrieved config values
        TString test = config->GetValue((prefix+"UnfoldingDimension").c_str(), "NOT_FOUND");
        log(LOG_INFO, "Config value for " + prefix + "UnfoldingDimension: " + test.Data());
        
        int ndim = config->GetValue((prefix+"UnfoldingDimension").c_str(), 1);
        log(LOG_INFO, "  Dimension: " + std::to_string(ndim));
        
        std::string varsStr = config->GetValue((prefix+"UnfoldVariables").c_str(), "");
        log(LOG_INFO, "  UnfoldVariables string: " + varsStr);
        std::vector<std::string> vars = splitCSV(varsStr);
        
        std::string tvarsStr = config->GetValue((prefix+"TruthVariables").c_str(), "");
        log(LOG_INFO, "  TruthVariables string: " + tvarsStr);
        std::vector<std::string> tvars = splitCSV(tvarsStr);
        
        // Debug the vars list
        std::string varsList = "  Measured variables:";
        for (const auto& var : vars) varsList += " " + var;
        log(LOG_INFO, varsList);
        
        std::string tvarsList = "  Truth variables:";
        for (const auto& var : tvars) tvarsList += " " + var;
        log(LOG_INFO, tvarsList);
        
        if (vars.size() != tvars.size() || vars.size() != ndim) {
            log(LOG_ERROR, "Inconsistent dimensions. Measured vars: " + std::to_string(vars.size()) + 
                ", truth vars: " + std::to_string(tvars.size()) + ", declared dimension: " + 
                std::to_string(ndim));
            continue;
        }
        
        std::vector<std::vector<double>> bins, truthBins;
        for (size_t i = 0; i < vars.size(); ++i) {
            std::string binKey = prefix + vars[i] + "Bins";
            std::string defaultBinKey = "default." + vars[i] + "Bins";
            std::string binStr = config->GetValue(binKey.c_str(), config->GetValue(defaultBinKey.c_str(), ""));
            log(LOG_INFO, "  " + binKey + ": " + binStr);
            
            std::vector<double> varBins = parseBins(binStr);
            if (varBins.empty()) {
                log(LOG_ERROR, "Empty bins for " + vars[i]);
                continue;
            }
            bins.push_back(varBins);
            
            std::string truthBinKey = prefix + tvars[i] + "Bins";
            std::string defaultTruthBinKey = "default." + tvars[i] + "Bins";
            std::string truthBinStr = config->GetValue(truthBinKey.c_str(), config->GetValue(defaultTruthBinKey.c_str(), ""));
            log(LOG_INFO, "  " + truthBinKey + ": " + truthBinStr);
            
            std::vector<double> varTruthBins = parseBins(truthBinStr);
            if (varTruthBins.empty()) {
                log(LOG_ERROR, "Empty bins for " + tvars[i]);
                continue;
            }
            truthBins.push_back(varTruthBins);
        }
        
        std::string weightBranch = config->GetValue((prefix+"eventWeightBranch").c_str(), 
                                    config->GetValue("default.eventWeightBranch", "eventWeight"));
        log(LOG_INFO, "  Weight branch: " + weightBranch);
        
        std::string dataFile = config->GetValue((prefix+"DataInputFile").c_str(), 
                                config->GetValue("default.DataInputFile", ""));
        std::string mcFile = config->GetValue((prefix+"MCInputFile").c_str(), 
                             config->GetValue("default.MCInputFile", ""));
        
        log(LOG_INFO, "  Data file: " + dataFile);
        log(LOG_INFO, "  MC file: " + mcFile);
        
        TFile* dataF = TFile::Open(dataFile.c_str());
        TFile* mcF = TFile::Open(mcFile.c_str());
        
        if (!dataF || dataF->IsZombie()) {
            log(LOG_ERROR, "Could not open data file: " + dataFile);
            continue;
        }
        
        if (!mcF || mcF->IsZombie()) {
            log(LOG_ERROR, "Could not open MC file: " + mcFile);
            continue;
        }
        
        TTree* dataTree = (TTree*)dataF->Get("gammaJetTree");
        TTree* mcTree = (TTree*)mcF->Get("gammaJetTree");
        
        if (!dataTree) {
            log(LOG_ERROR, "Could not find gammaJetTree in data file");
            continue;
        }
        
        if (!mcTree) {
            log(LOG_ERROR, "Could not find gammaJetTree in MC file");
            continue;
        }
        
        log(LOG_INFO, "Creating unfolder with " + std::to_string(ndim) + " dimensions");
        GenericUnfolderND unfolder(vars, bins, tvars, truthBins);
        
        log(LOG_INFO, "Filling from data tree");
        unfolder.fillFromTree(dataTree, false, true, weightBranch);
        
        log(LOG_INFO, "Filling from MC tree");
        unfolder.fillFromTree(mcTree, true, false, weightBranch);
        
        std::string method = config->GetValue((prefix+"UnfoldingMethod").c_str(), 
                             config->GetValue("default.UnfoldingMethod", "Invert"));
        int nIter = config->GetValue((prefix+"UnfoldingIterations").c_str(), 
                    config->GetValue("default.UnfoldingIterations", 4));
        
        log(LOG_INFO, "Performing unfolding with method: " + method + ", iterations: " + std::to_string(nIter));
        unfolder.performUnfolding(method, nIter);
        
        outFile->cd();
        // Create a directory for each dimension (Unfolding1D, Unfolding2D, Unfolding3D) if not already present
        std::string dimDir = "Unfolding" + std::to_string(ndim) + "D";
        TDirectory* mainDir = (TDirectory*)outFile->Get(dimDir.c_str());
        if (!mainDir) mainDir = outFile->mkdir(dimDir.c_str());
        mainDir->cd();
        
        // Check if directory already exists and delete it if it does
        TDirectory* existingDir = (TDirectory*)mainDir->Get(set.c_str());
        if (existingDir) {
            log(LOG_WARNING, "Directory " + set + " already exists, replacing it");
            mainDir->Delete((set + ";*").c_str());
        }
        
        TDirectory* dir = mainDir->mkdir(set.c_str());
        dir->cd();
        
        // Get histograms from unfolder based on dimension
        TObject* h_measured_data = unfolder.getMeasuredData();
        TObject* h_measured_mc = unfolder.getMeasuredMC();
        TObject* h_truth_mc = unfolder.getTruthMC();
        TH2D* h_response = (TH2D*)unfolder.getResponse();
        
        log(LOG_INFO, "Writing histograms to output file");
        
        if (ndim == 1) {
            TH1D* h_data_1d = (TH1D*)h_measured_data;
            TH1D* h_mc_1d = (TH1D*)h_measured_mc;
            TH1D* h_truth_1d = (TH1D*)h_truth_mc;
            
            log(LOG_INFO, "  Data entries: " + std::to_string(h_data_1d ? h_data_1d->GetEntries() : 0));
            log(LOG_INFO, "  MC measured entries: " + std::to_string(h_mc_1d ? h_mc_1d->GetEntries() : 0));
            log(LOG_INFO, "  MC truth entries: " + std::to_string(h_truth_1d ? h_truth_1d->GetEntries() : 0));
            log(LOG_INFO, "  Response entries: " + std::to_string(h_response ? h_response->GetEntries() : 0));
            
            log(LOG_INFO, "  Data integral: " + std::to_string(h_data_1d ? h_data_1d->Integral() : 0));
            log(LOG_INFO, "  MC measured integral: " + std::to_string(h_mc_1d ? h_mc_1d->Integral() : 0));
            log(LOG_INFO, "  MC truth integral: " + std::to_string(h_truth_1d ? h_truth_1d->Integral() : 0));
            log(LOG_INFO, "  Response integral: " + std::to_string(h_response ? h_response->Integral() : 0));
            
            if (h_data_1d) h_data_1d->Write("h_measured_data");
            if (h_mc_1d) h_mc_1d->Write("h_measured_mc");
            if (h_truth_1d) h_truth_1d->Write("h_truth_mc");
            if (h_response) h_response->Write("h_response");
            
        } else if (ndim == 2) {
            TH2D* h_data_2d = (TH2D*)h_measured_data;
            TH2D* h_mc_2d = (TH2D*)h_measured_mc;
            TH2D* h_truth_2d = (TH2D*)h_truth_mc;
            
            log(LOG_INFO, "  Data entries: " + std::to_string(h_data_2d ? h_data_2d->GetEntries() : 0));
            log(LOG_INFO, "  MC measured entries: " + std::to_string(h_mc_2d ? h_mc_2d->GetEntries() : 0));
            log(LOG_INFO, "  MC truth entries: " + std::to_string(h_truth_2d ? h_truth_2d->GetEntries() : 0));
            log(LOG_INFO, "  Response entries: " + std::to_string(h_response ? h_response->GetEntries() : 0));
            
            log(LOG_INFO, "  Data integral: " + std::to_string(h_data_2d ? h_data_2d->Integral() : 0));
            log(LOG_INFO, "  MC measured integral: " + std::to_string(h_mc_2d ? h_mc_2d->Integral() : 0));
            log(LOG_INFO, "  MC truth integral: " + std::to_string(h_truth_2d ? h_truth_2d->Integral() : 0));
            log(LOG_INFO, "  Response integral: " + std::to_string(h_response ? h_response->Integral() : 0));
            
            if (h_data_2d) h_data_2d->Write("h_measured_data");
            if (h_mc_2d) h_mc_2d->Write("h_measured_mc");
            if (h_truth_2d) h_truth_2d->Write("h_truth_mc");
            if (h_response) h_response->Write("h_response");
            
        } else if (ndim == 3) {
            TH3D* h_data_3d = (TH3D*)h_measured_data;
            TH3D* h_mc_3d = (TH3D*)h_measured_mc;
            TH3D* h_truth_3d = (TH3D*)h_truth_mc;
            
            log(LOG_INFO, "  Data entries: " + std::to_string(h_data_3d ? h_data_3d->GetEntries() : 0));
            log(LOG_INFO, "  MC measured entries: " + std::to_string(h_mc_3d ? h_mc_3d->GetEntries() : 0));
            log(LOG_INFO, "  MC truth entries: " + std::to_string(h_truth_3d ? h_truth_3d->GetEntries() : 0));
            log(LOG_INFO, "  Response entries: " + std::to_string(h_response ? h_response->GetEntries() : 0));
            
            log(LOG_INFO, "  Data integral: " + std::to_string(h_data_3d ? h_data_3d->Integral() : 0));
            log(LOG_INFO, "  MC measured integral: " + std::to_string(h_mc_3d ? h_mc_3d->Integral() : 0));
            log(LOG_INFO, "  MC truth integral: " + std::to_string(h_truth_3d ? h_truth_3d->Integral() : 0));
            log(LOG_INFO, "  Response integral: " + std::to_string(h_response ? h_response->Integral() : 0));
            
            if (h_data_3d) h_data_3d->Write("h_measured_data");
            if (h_mc_3d) h_mc_3d->Write("h_measured_mc");
            if (h_truth_3d) h_truth_3d->Write("h_truth_mc");
            if (h_response) h_response->Write("h_response");
        }
        
        plotResponseMatrix(set, unfolder, bins, vars);
        log(LOG_INFO, "Unfolding set '" + set + "' completed and saved.");
        
        dataF->Close();
        mcF->Close();
    }
    outFile->Close();
    log(LOG_INFO, "Results saved to: " + outputPath);
    delete config;
    timer.Stop();
    log(LOG_INFO, "\n=== RooUnfold completed in " + std::to_string(timer.RealTime()) + " seconds ===");
}