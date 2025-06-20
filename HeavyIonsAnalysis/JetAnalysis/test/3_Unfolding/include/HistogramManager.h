#ifndef HISTOGRAM_MANAGER_H
#define HISTOGRAM_MANAGER_H

#include "UnfoldHelpers.h"
#include "UnfoldConfig.h"
#include <memory>
#include <vector>
#include <TH1D.h>
#include <TH2D.h>
#include <TH3D.h>

// ============================================================================
// UNIFIED HISTOGRAM MANAGER
// ============================================================================

class HistogramManager {
private:
    std::unique_ptr<TH1> h_data, h_mc_meas, h_mc_truth;
    std::unique_ptr<TH2D> h_response;
    int ndim;
    
public:
    HistogramManager(const UnfoldConfig& config) : ndim(config.dimension) {
        createHistograms(config);
    }
    
    void createHistograms(const UnfoldConfig& config) {
        log(LOG_DEBUG, "Creating " + std::to_string(ndim) + "D histograms");
        if (ndim == 1) {
            h_data = std::make_unique<TH1D>("h_data", "Data", 
                                          config.measuredBins[0].size()-1, 
                                          config.measuredBins[0].data());
            h_data->SetDirectory(0);
            h_mc_meas = std::make_unique<TH1D>("h_mc_meas", "MC Measured", 
                                             config.measuredBins[0].size()-1, 
                                             config.measuredBins[0].data());
            h_mc_meas->SetDirectory(0);
            h_mc_truth = std::make_unique<TH1D>("h_mc_truth", "MC Truth", 
                                              config.truthBins[0].size()-1, 
                                              config.truthBins[0].data());
            h_mc_truth->SetDirectory(0);
            h_response = std::make_unique<TH2D>("h_response", "Response Matrix",
                                              config.truthBins[0].size()-1, config.truthBins[0].data(),
                                              config.measuredBins[0].size()-1, config.measuredBins[0].data());
            h_response->SetDirectory(0);
        }
        else if (ndim == 2) {
            h_data = std::make_unique<TH2D>("h_data", "Data", 
                                          config.measuredBins[0].size()-1, config.measuredBins[0].data(),
                                          config.measuredBins[1].size()-1, config.measuredBins[1].data());
            h_data->SetDirectory(0);
            h_mc_meas = std::make_unique<TH2D>("h_mc_meas", "MC Measured", 
                                             config.measuredBins[0].size()-1, config.measuredBins[0].data(),
                                             config.measuredBins[1].size()-1, config.measuredBins[1].data());
            h_mc_meas->SetDirectory(0);
            h_mc_truth = std::make_unique<TH2D>("h_mc_truth", "MC Truth", 
                                              config.truthBins[0].size()-1, config.truthBins[0].data(),
                                              config.truthBins[1].size()-1, config.truthBins[1].data());
            h_mc_truth->SetDirectory(0);
            int nTruthBins = (config.truthBins[0].size()-1) * (config.truthBins[1].size()-1);
            int nMeasBins = (config.measuredBins[0].size()-1) * (config.measuredBins[1].size()-1);
            h_response = std::make_unique<TH2D>("h_response", "Response Matrix",
                                              nTruthBins, 0, nTruthBins,
                                              nMeasBins, 0, nMeasBins);
            h_response->SetDirectory(0);
        }
        else if (ndim == 3) {
            h_data = std::make_unique<TH3D>("h_data", "Data", 
                                          config.measuredBins[0].size()-1, config.measuredBins[0].data(),
                                          config.measuredBins[1].size()-1, config.measuredBins[1].data(),
                                          config.measuredBins[2].size()-1, config.measuredBins[2].data());
            h_data->SetDirectory(0);
            h_mc_meas = std::make_unique<TH3D>("h_mc_meas", "MC Measured", 
                                             config.measuredBins[0].size()-1, config.measuredBins[0].data(),
                                             config.measuredBins[1].size()-1, config.measuredBins[1].data(),
                                             config.measuredBins[2].size()-1, config.measuredBins[2].data());
            h_mc_meas->SetDirectory(0);
            h_mc_truth = std::make_unique<TH3D>("h_mc_truth", "MC Truth", 
                                              config.truthBins[0].size()-1, config.truthBins[0].data(),
                                              config.truthBins[1].size()-1, config.truthBins[1].data(),
                                              config.truthBins[2].size()-1, config.truthBins[2].data());
            h_mc_truth->SetDirectory(0);
            int nTruthBins = (config.truthBins[0].size()-1) * (config.truthBins[1].size()-1) * (config.truthBins[2].size()-1);
            int nMeasBins = (config.measuredBins[0].size()-1) * (config.measuredBins[1].size()-1) * (config.measuredBins[2].size()-1);
            h_response = std::make_unique<TH2D>("h_response", "Response Matrix",
                                              nTruthBins, 0, nTruthBins,
                                              nMeasBins, 0, nMeasBins);
            h_response->SetDirectory(0);
        }
        log(LOG_DEBUG, "Histograms created successfully");
    }
    
    void fillData(const std::vector<double>& values, double weight) {
        fillHistogram(h_data.get(), values, weight);
    }
    
    void fillMC(const std::vector<double>& measValues, const std::vector<double>& truthValues, double weight) {
        fillHistogram(h_mc_meas.get(), measValues, weight);
        fillHistogram(h_mc_truth.get(), truthValues, weight);
        fillResponse(measValues, truthValues, weight);
    }
    
    // Accessors
    TH1* getData() { return h_data.get(); }
    TH1* getMeasuredMC() { return h_mc_meas.get(); }
    TH1* getTruthMC() const { return h_mc_truth.get(); }
    TH2D* getResponse() const { return h_response.get(); }
    
private:
    void fillHistogram(TH1* hist, const std::vector<double>& values, double weight) {
        if (ndim == 1) hist->Fill(values[0], weight);
        else if (ndim == 2) ((TH2*)hist)->Fill(values[0], values[1], weight);
        else if (ndim == 3) ((TH3*)hist)->Fill(values[0], values[1], values[2], weight);
    }
    
    void fillResponse(const std::vector<double>& measValues, const std::vector<double>& truthValues, double weight) {
        if (ndim == 1) {
            h_response->Fill(truthValues[0], measValues[0], weight);
        } else {
            // Use unified flattening for multi-dimensional
            auto truthFlat = calculateFlatIndex(truthValues, h_mc_truth.get());
            auto measFlat = calculateFlatIndex(measValues, h_mc_meas.get());
            h_response->Fill(truthFlat, measFlat, weight);
        }
    }
    
    int calculateFlatIndex(const std::vector<double>& values, TH1* hist) {
        // Unified bin index calculation
        std::vector<int> binIndices;
        std::vector<int> nBins;
        
        if (ndim == 2) {
            TH2* h2 = (TH2*)hist;
            binIndices.push_back(h2->GetXaxis()->FindBin(values[0]) - 1);
            binIndices.push_back(h2->GetYaxis()->FindBin(values[1]) - 1);
            nBins.push_back(h2->GetNbinsX());
            nBins.push_back(h2->GetNbinsY());
        } else if (ndim == 3) {
            TH3* h3 = (TH3*)hist;
            binIndices.push_back(h3->GetXaxis()->FindBin(values[0]) - 1);
            binIndices.push_back(h3->GetYaxis()->FindBin(values[1]) - 1);
            binIndices.push_back(h3->GetZaxis()->FindBin(values[2]) - 1);
            nBins.push_back(h3->GetNbinsX());
            nBins.push_back(h3->GetNbinsY());
            nBins.push_back(h3->GetNbinsZ());
        }
        
        return flattenIndices(binIndices, nBins);
    }
};

#endif // HISTOGRAM_MANAGER_H
