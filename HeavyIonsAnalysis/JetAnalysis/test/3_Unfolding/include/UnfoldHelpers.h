#ifndef UNFOLD_HELPERS_H
#define UNFOLD_HELPERS_H

#include "UnfoldUtils.h"
#include "UnfoldConfig.h"
#include "HistogramManager.h"

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <cctype>
#include <cmath>
#include <functional>
#include <Eigen/Dense>
#include <TObjString.h>
#include <TMatrixD.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TH3D.h>

// Structure to hold binning for arbitrary variables
typedef std::vector<double> BinEdges;

struct UnfoldingBinning {
    std::map<std::string, BinEdges> binEdges; // variable name -> bin edges
    void print() const {
        log(LOG_DEBUG, "Binning configuration:");
        for (const auto& kv : binEdges) {
            log(LOG_DEBUG, "  " + kv.first + ": " + std::to_string(nBins(kv.first)) + " bins");
        }
    }
    int nBins(const std::string& var) const {
        auto it = binEdges.find(var);
        if (it == binEdges.end()) return 0;
        return it->second.size() > 1 ? it->second.size() - 1 : 0;
    }
};

// Helper functions for global bin index (for N-dim) - ROW-MAJOR ordering
inline int getGlobalBin(const std::vector<int>& indices, const std::vector<int>& nBins) {
    // This function is deprecated - use flattenIndices() from UnfoldUtils.h instead
    log(LOG_WARNING, "getGlobalBin() is deprecated, use flattenIndices() from UnfoldUtils.h");
    return flattenIndices(indices, nBins);
}

// Config parsing helpers
inline std::map<std::string, std::string> parseConfig(const std::string& configFile) {
    std::map<std::string, std::string> config;
    std::ifstream infile(configFile);
    std::string line;
    while (std::getline(infile, line)) {
        if (line.empty() || line[0] == '#') continue;
        auto pos = line.find(' ');
        if (pos == std::string::npos) continue;
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        config[key] = value;
    }
    return config;
}

inline UnfoldingBinning parseBinningFromConfig(const std::map<std::string, std::string>& config, const std::vector<std::string>& variables) {
    UnfoldingBinning binning;
    for (const auto& var : variables) {
        std::string key = var + "Bins";
        auto it = config.find(key);
        if (it != config.end()) {
            binning.binEdges[var] = parseBins(it->second);
        }
    }
    return binning;
}

// Helper: generate axis labels for flattened axes with cycles and " " for overlaps
inline std::vector<std::string> generateAxisLabels(const std::vector<double>& binEdges, int cycles) {
    std::vector<std::string> labels;
    for (int c = 0; c < cycles; ++c) {
        for (size_t i = 0; i < binEdges.size(); ++i) {
            if (c > 0 && i == 0)
                labels.push_back(" ");
            else
                labels.push_back(std::to_string(binEdges[i]));
        }
    }
    return labels;
}

// Format axis value with automatic precision, no trailing zeros or exponents
inline std::string formatAxisValue(double value) {
    std::ostringstream oss;
    
    // Handle special cases
    if (value == 0.0) return "0";
    
    // Determine appropriate precision based on magnitude
    double absVal = std::abs(value);
    int precision;
    
    if (absVal >= 100) precision = 0;        // 150, 500
    else if (absVal >= 10) precision = 1;    // 40.0, 80.0  
    else if (absVal >= 1) precision = 2;     // 1.50, 2.75
    else if (absVal >= 0.1) precision = 2;   // 0.12, 0.08
    else precision = 3;                      // 0.020, 0.004
    
    oss << std::fixed << std::setprecision(precision) << value;
    std::string result = oss.str();
    
    // Remove trailing zeros after decimal point
    if (result.find('.') != std::string::npos) {
        while (result.back() == '0') result.pop_back();
        if (result.back() == '.') result.pop_back();
    }
    
    return result;
}

// Generate axis labels with cycle handling for overlapping edges
inline std::vector<std::string> generateAxisLabelsWithCycles(
    const std::vector<double>& binEdges, 
    int nCycles) {
    
    std::vector<std::string> labels;
    
    for (int cycle = 0; cycle < nCycles; ++cycle) {
        for (size_t i = 0; i < binEdges.size(); ++i) {
            if (cycle > 0 && i == 0) {
                // Replace first label of subsequent cycles with space
                labels.push_back(" ");
            } else {
                labels.push_back(formatAxisValue(binEdges[i]));
            }
        }
    }
    
    return labels;
}

// Calculate cycle information for a dimension in flattened structure
struct DimensionCycleInfo {
    int nCycles;           // How many times this dimension repeats
    int binsPerCycle;      // Number of bins in each cycle for this dimension
    int stride;            // Stride for this dimension in flattened array
};

inline DimensionCycleInfo calculateCycleInfo(int dim, const std::vector<int>& nBins) {
    DimensionCycleInfo info;
    
    // Calculate stride: product of all dimensions before this one
    info.stride = 1;
    for (int i = 0; i < dim; ++i) {
        info.stride *= nBins[i];
    }
    
    // Bins per cycle for this dimension
    info.binsPerCycle = nBins[dim];
    
    // Number of cycles: total bins divided by (stride * bins per cycle)
    int totalBins = 1;
    for (int n : nBins) totalBins *= n;
    info.nCycles = totalBins / (info.stride * info.binsPerCycle);
    
    return info;
}

// Helper: Parse int vector from string
inline std::vector<int> parseIntVec(const std::string& s) {
    std::vector<int> v;
    std::stringstream ss(s);
    int x;
    while (ss >> x) v.push_back(x);
    return v;
}

// Helper: Convert global bin index to vector of physical bin indices (for N-dim)
inline std::vector<int> globalToIndices(int globalBin, const std::vector<int>& nBins) {
    std::vector<int> indices(nBins.size(), 0);
    int remainder = globalBin;
    for (int i = nBins.size() - 1; i >= 0; --i) {
        indices[i] = remainder % nBins[i];
        remainder /= nBins[i];
    }
    return indices;
}

// Helper: Get bin edges for a variable from binning structure
inline std::vector<double> getBinEdges(const std::vector<std::string>& varNames, const std::vector<std::vector<double>>& bins, const std::string& var) {
    for (size_t i = 0; i < varNames.size(); ++i) {
        if (varNames[i] == var) return bins[i];
    }
    return {};
}

// Helper: Get bin label for a given bin index and bin edges
inline std::string getBinLabel(const std::vector<double>& edges, int idx) {
    if (idx < 0 || idx >= (int)edges.size() - 1) return "";
    std::ostringstream oss;
    oss << edges[idx] << "-" << edges[idx+1];
    return oss.str();
}

// Utility: Unfoldability check (Nominal only)
// Add TDirectory* outDir argument
inline bool checkUnfoldability(const HistogramManager& histManager, const UnfoldConfig& config, std::string& details, TH1D*& effHist, TMatrixD*& covMatrix, TDirectory* outDir = nullptr) {
    std::ostringstream out;
    bool fatal = false;
    // Matrix shape
    auto* hresp = histManager.getResponse();
    int nX = hresp->GetNbinsX();
    int nY = hresp->GetNbinsY();
    out << "Matrix shape: " << nX << " x " << nY << (nX == nY ? " (square)" : " (not square)") << "\n";
    // Matrix rank and condition number
    Eigen::MatrixXd mat(nX, nY);
    for (int i = 0; i < nX; ++i)
        for (int j = 0; j < nY; ++j)
            mat(i, j) = hresp->GetBinContent(i+1, j+1);
    Eigen::JacobiSVD<Eigen::MatrixXd> svd(mat);
    double cond = svd.singularValues()(0) / svd.singularValues().tail(1)(0);
    out << "Matrix rank: " << svd.rank() << "/" << std::min(nX, nY) << "\n";
    out << "Condition number: " << cond << "\n";
    // Sufficient statistics
    auto* htruth = histManager.getTruthMC();
    double nEntries = htruth->GetEntries();
    out << "MC truth entries: " << nEntries << "\n";
    if (nEntries < 100) {
        out << "Warning: Insufficient MC statistics!\n";
    }
    // Bin population
    std::vector<int> emptyTruth, emptyMeas;
    for (int i = 1; i <= nX; ++i) {
        bool empty = true;
        for (int j = 1; j <= nY; ++j) if (hresp->GetBinContent(i, j) > 0) { empty = false; break; }
        if (empty) emptyTruth.push_back(i);
    }
    for (int j = 1; j <= nY; ++j) {
        bool empty = true;
        for (int i = 1; i <= nX; ++i) if (hresp->GetBinContent(i, j) > 0) { empty = false; break; }
        if (empty) emptyMeas.push_back(j);
    }
    int nEmptyTruth = emptyTruth.size();
    int nEmptyMeas = emptyMeas.size();
    double pctEmptyTruth = 100.0 * nEmptyTruth / nX;
    double pctEmptyMeas = 100.0 * nEmptyMeas / nY;
    // Purity histogram calculation using existing flattening helpers
    auto* hmeas = histManager.getMeasuredMC();
    
    // Get number of bins per dimension for measured histogram  
    std::vector<int> measNBins;
    if (config.dimension == 1) {
        measNBins.push_back(hmeas->GetNbinsX());
    } else if (config.dimension == 2) {
        TH2* h2meas = (TH2*)hmeas;
        measNBins.push_back(h2meas->GetNbinsX());
        measNBins.push_back(h2meas->GetNbinsY());
    } else if (config.dimension == 3) {
        TH3* h3meas = (TH3*)hmeas;
        measNBins.push_back(h3meas->GetNbinsX());
        measNBins.push_back(h3meas->GetNbinsY());
        measNBins.push_back(h3meas->GetNbinsZ());
    }
    
    // Calculate total measured bins using existing helper logic
    int totalMeasBins = 1;
    for (int n : measNBins) totalMeasBins *= n;
    
    TH1D* purityHist = new TH1D("fake", "Fakes measured with response matrix (fraction of true signal in measured bins)", totalMeasBins, 0.5, totalMeasBins + 0.5);
    purityHist->SetMinimum(0.0);
    purityHist->SetMaximum(1.0);
    int nFakeBins = 0;
    double totalFakes = 0.0, totalMeas = 0.0;
    
    // Use direct flattened indexing for purity calculation
    for (int j = 1; j <= totalMeasBins; ++j) {
        double sumTruth = 0.0;
        for (int i = 1; i <= nX; ++i) sumTruth += hresp->GetBinContent(i, j);
        double purity = 0.0;
        if (sumTruth > 0) {
            // For purity, we use diagonal elements of response matrix
            purity = hresp->GetBinContent(j, j) / sumTruth;
        }
        purityHist->SetBinContent(j, purity);
        if (sumTruth == 0) nFakeBins++;
        totalMeas += sumTruth;
        // Fakes: measured bins with no matching truth
        if (sumTruth > 0 && hresp->GetBinContent(0, j) > 0) totalFakes += hresp->GetBinContent(0, j);
    }
    if (outDir) outDir->cd();
    purityHist->Write("Fakes_histogram_flattened");
    // Efficiency calculation for all dimensions (using existing flattening helpers)
    auto* htruthEff = histManager.getTruthMC();
    
    // Get number of bins per dimension for truth histogram
    std::vector<int> truthNBins;
    if (config.dimension == 1) {
        truthNBins.push_back(htruthEff->GetNbinsX());
    } else if (config.dimension == 2) {
        TH2* h2truth = (TH2*)htruthEff;
        truthNBins.push_back(h2truth->GetNbinsX());
        truthNBins.push_back(h2truth->GetNbinsY());
    } else if (config.dimension == 3) {
        TH3* h3truth = (TH3*)htruthEff;
        truthNBins.push_back(h3truth->GetNbinsX());
        truthNBins.push_back(h3truth->GetNbinsY());
        truthNBins.push_back(h3truth->GetNbinsZ());
    }
    
    // Calculate total truth bins using existing helper
    int totalTruthBins = 1;
    for (int n : truthNBins) totalTruthBins *= n;
    
    effHist = new TH1D("efficiency", "Efficiency per truth bin", totalTruthBins, 0.5, totalTruthBins + 0.5);
    effHist->SetMinimum(0.0);
    effHist->SetMaximum(1.1);
    
    int nEffZero = 0, nEffFull = 0;
    
    // Fill efficiency histogram using existing flattening logic
    if (config.dimension == 1) {
        // 1D case - direct bin access
        for (int i = 1; i <= truthNBins[0]; ++i) {
            double denom = 0.0;
            for (int j = 1; j <= nY; ++j) denom += hresp->GetBinContent(i, j);
            double num = htruthEff->GetBinContent(i);
            double eff = (denom > 0) ? num / denom : 0.0;
            if (eff > 1.0) eff = 1.0;
            effHist->SetBinContent(i, eff);
            if (eff == 0.0) nEffZero++;
            if (eff == 1.0) nEffFull++;
        }
    } else {
        // Multi-dimensional case using existing helper functions
        std::vector<int> indices(config.dimension);
        
        // Generate all possible bin combinations
        std::function<void(int)> fillBins = [&](int dim) {
            if (dim == config.dimension) {
                // Calculate flattened index using existing helper
                int flatBin = flattenIndices(indices, truthNBins) + 1; // +1 for ROOT indexing
                
                // Get histogram content based on dimension
                double num = 0.0;
                if (config.dimension == 2) {
                    num = ((TH2*)htruthEff)->GetBinContent(indices[0] + 1, indices[1] + 1);
                } else if (config.dimension == 3) {
                    num = ((TH3*)htruthEff)->GetBinContent(indices[0] + 1, indices[1] + 1, indices[2] + 1);
                }
                
                // Calculate efficiency
                double denom = 0.0;
                for (int j = 1; j <= nY; ++j) denom += hresp->GetBinContent(flatBin, j);
                double eff = (denom > 0) ? num / denom : 0.0;
                if (eff > 1.0) eff = 1.0;
                effHist->SetBinContent(flatBin, eff);
                if (eff == 0.0) nEffZero++;
                if (eff == 1.0) nEffFull++;
                return;
            }
            
            for (int i = 0; i < truthNBins[dim]; ++i) {
                indices[dim] = i;
                fillBins(dim + 1);
            }
        };
        
        fillBins(0);
    }
    out << "Total fakes subtracted: " << totalFakes << " (" << (totalFakes/totalMeas*100.0) << "%)\n";
    out << "Empty truth bins: " << nEmptyTruth << "/" << nX << " (" << pctEmptyTruth << "%)\n";
    out << "Empty measured bins: " << nEmptyMeas << "/" << nY << " (" << pctEmptyMeas << "%)\n";
    out << "Purity histogram (flattened): " << nFakeBins << " measured bins with no true signal\n";
    out << "Efficiency histogram: " << nEffZero << " bins with 0, " << nEffFull << " bins with 1\n";
    // Covariance matrix (dummy example: identity)
    covMatrix = new TMatrixD(nX, nY);
    for (int i = 0; i < nX; ++i)
        for (int j = 0; j < nY; ++j)
            (*covMatrix)(i, j) = (i == j ? 1.0 : 0.0);
    // Output details
    details = out.str();
    return true;
}

#endif // UNFOLD_HELPERS_H
