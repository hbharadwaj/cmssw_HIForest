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
#include <Eigen/Dense>
#include <TObjString.h>
#include <TMatrixD.h>
#include <TH1D.h>

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

// Helper functions for global bin index (for N-dim)
inline int getGlobalBin(const std::vector<int>& indices, const std::vector<int>& nBins) {
    int global = 0;
    int stride = 1;
    for (int i = indices.size() - 1; i >= 0; --i) {
        global += indices[i] * stride;
        stride *= nBins[i];
    }
    return global;
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

// Helper function to get global bin index for 2D case
inline int getGlobalBin2D(int iBinX, int iBinY, int nBinsX) {
    return iBinY * nBinsX + iBinX;
}

// Helper function to get global bin index for 3D case
inline int getGlobalBin3D(int iBinX, int iBinY, int iBinZ, int nBinsX, int nBinsY) {
    return iBinZ * (nBinsX * nBinsY) + iBinY * nBinsX + iBinX;
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
    // Purity histogram (fakes)
    TH1D* purityHist = new TH1D("purity", "Purity (fraction of true signal in measured bins)", nY, 0.5, nY+0.5);
    purityHist->SetMinimum(0.0);
    purityHist->SetMaximum(1.0);
    int nFakeBins = 0;
    double totalFakes = 0.0, totalMeas = 0.0;
    for (int j = 1; j <= nY; ++j) {
        double sumTruth = 0.0;
        for (int i = 1; i <= nX; ++i) sumTruth += hresp->GetBinContent(i, j);
        double purity = 0.0;
        if (sumTruth > 0) {
            purity = hresp->GetBinContent(j, j) / sumTruth;
        }
        purityHist->SetBinContent(j, purity);
        if (sumTruth == 0) nFakeBins++;
        totalMeas += sumTruth;
        // Fakes: measured bins with no matching truth
        if (sumTruth > 0 && hresp->GetBinContent(0, j) > 0) totalFakes += hresp->GetBinContent(0, j);
    }
    if (outDir) outDir->cd();
    purityHist->Write("purity_histogram");
    out << "Purity histogram: " << nFakeBins << " measured bins with no true signal\n";
    out << "Total fakes subtracted: " << totalFakes << " (" << (totalFakes/totalMeas*100.0) << "%)\n";
    out << "Empty truth bins: " << nEmptyTruth << "/" << nX << " (" << pctEmptyTruth << "%)\n";
    out << "Empty measured bins: " << nEmptyMeas << "/" << nY << " (" << pctEmptyMeas << "%)\n";
    // Efficiency (range 0 to 1.1, correct for 1D/2D/3D)
    effHist = (TH1D*)htruth->Clone("efficiency");
    effHist->SetTitle("Efficiency per truth bin");
    effHist->SetMinimum(0.0);
    effHist->SetMaximum(1.1);
    int nEffZero = 0, nEffFull = 0;
    for (int i = 1; i <= htruth->GetNbinsX(); ++i) {
        double denom = 0.0;
        for (int j = 1; j <= nY; ++j) denom += hresp->GetBinContent(i, j);
        double num = htruth->GetBinContent(i);
        double eff = (denom > 0) ? num / denom : 0.0;
        if (eff > 1.0) eff = 1.0;
        effHist->SetBinContent(i, eff);
        if (eff == 0.0) nEffZero++;
        if (eff == 1.0) nEffFull++;
    }
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
