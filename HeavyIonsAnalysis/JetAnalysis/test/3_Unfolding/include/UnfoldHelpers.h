#ifndef UNFOLD_HELPERS_H
#define UNFOLD_HELPERS_H

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <fstream>

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

#endif // UNFOLD_HELPERS_H
