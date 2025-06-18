#ifndef UNFOLD_HELPERS_H
#define UNFOLD_HELPERS_H

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iomanip> // For std::setprecision

// Logging levels
enum LogLevel { LOG_ERROR = 0, LOG_WARNING = 1, LOG_INFO = 2, LOG_DEBUG = 3 };
inline int gVerbosity = LOG_INFO;

inline void log(LogLevel level, const std::string& message) {
    if (level <= gVerbosity) {
        const char* prefix = "";
        switch (level) {
            case LOG_ERROR:   prefix = "[ERROR]   "; break;
            case LOG_WARNING: prefix = "[WARNING] "; break;
            case LOG_INFO:    prefix = "[INFO]    "; break;
            case LOG_DEBUG:   prefix = "[DEBUG]   "; break;
        }
        std::cout << prefix << message << std::endl;
    }
}

// Structure to hold binning for arbitrary variables
typedef std::vector<double> BinEdges;

struct UnfoldingBinning {
    std::map<std::string, BinEdges> binEdges; // variable name -> bin edges
    void print() const {
        log(LOG_INFO, "Binning configuration:");
        for (const auto& kv : binEdges) {
            log(LOG_INFO, "  " + kv.first + ": " + std::to_string(nBins(kv.first)) + " bins");
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

inline std::vector<std::string> splitCSV(const std::string& csv) {
    std::vector<std::string> result;
    std::stringstream ss(csv);
    std::string item;
    while (std::getline(ss, item, ',')) {
        item.erase(std::remove_if(item.begin(), item.end(), ::isspace), item.end());
        if (!item.empty()) result.push_back(item);
    }
    return result;
}

inline std::vector<double> parseBins(const std::string& binString) {
    std::vector<double> bins;
    std::vector<std::string> tokens = splitCSV(binString);
    for (const auto& t : tokens) {
        bins.push_back(std::stod(t));
    }
    return bins;
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

// Additional helper functions for N-dimensional unfolding

// Helper: flatten N-dimensional indices to 1D
inline int flattenIndices(const std::vector<int>& indices, const std::vector<int>& nBins) {
    int flat = 0;
    int stride = 1;
    for (size_t d = 0; d < indices.size(); ++d) {
        flat += indices[d] * stride;
        stride *= nBins[d];
    }
    return flat;
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

#endif // UNFOLD_HELPERS_H
