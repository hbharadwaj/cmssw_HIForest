#ifndef UNFOLD_UTILS_H
#define UNFOLD_UTILS_H
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <cctype>

// Color codes for terminal output
#define RESET_COLOR   "\033[0m"
#define RED_COLOR     "\033[91m"
#define GREEN_COLOR   "\033[92m"
#define YELLOW_COLOR  "\033[93m"
#define BLUE_COLOR    "\033[94m"
#define MAGENTA_COLOR "\033[95m"
#define CYAN_COLOR    "\033[96m"
#define WHITE_COLOR   "\033[97m"
#define BOLD_COLOR    "\033[1m"

// Logging levels and log function
enum LogLevel { LOG_ERROR = 0, LOG_WARNING = 1, LOG_INFO = 2, LOG_DEBUG = 3, LOG_TRACE = 4 };
inline int gVerbosity = LOG_INFO;
inline void log(LogLevel level, const std::string& message) {
    if (level > gVerbosity) return;
    const char* colorCode = RESET_COLOR;
    const char* levelStr = "";
    switch (level) {
        case LOG_ERROR:   colorCode = RED_COLOR;     levelStr = "ERROR"; break;
        case LOG_WARNING: colorCode = YELLOW_COLOR;  levelStr = "WARNING"; break;
        case LOG_INFO:    colorCode = GREEN_COLOR;   levelStr = "INFO";  break;
        case LOG_DEBUG:   colorCode = MAGENTA_COLOR; levelStr = "DEBUG"; break;
        case LOG_TRACE:   colorCode = CYAN_COLOR;    levelStr = "TRACE"; break;
        default:          colorCode = RESET_COLOR;   levelStr = "LOG";   break;
    }
    std::cout << colorCode << "[" << levelStr << "] " << message << RESET_COLOR << std::endl;
}

// splitCSV: split comma-separated string into vector<string>
inline std::vector<std::string> splitCSV(const std::string& csv) {
    std::vector<std::string> result;
    std::istringstream ss(csv);
    std::string token;
    while (std::getline(ss, token, ',')) {
        token.erase(std::remove_if(token.begin(), token.end(), ::isspace), token.end());
        if (!token.empty()) result.push_back(token);
    }
    return result;
}

// parseBins: split comma-separated string into vector<double>
inline std::vector<double> parseBins(const std::string& binString) {
    std::vector<double> result;
    std::istringstream ss(binString);
    std::string token;
    while (std::getline(ss, token, ',')) {
        try {
            result.push_back(std::stod(token));
        } catch (...) {}
    }
    return result;
}


// Additional helper functions for N-dimensional unfolding

// Helper: flatten N-dimensional indices to 1D using ROW-MAJOR ordering (C++ standard)
// For 2D: flat = i*nY + j (Y increments fastest)
// For 3D: flat = i*nY*nZ + j*nZ + k (Z increments fastest)
inline int flattenIndices(const std::vector<int>& indices, const std::vector<int>& nBins) {
    if (indices.size() != nBins.size()) {
        log(LOG_ERROR, "flattenIndices: dimension mismatch - indices.size()=" + 
            std::to_string(indices.size()) + ", nBins.size()=" + std::to_string(nBins.size()));
        return -1;
    }
    
    int flat = 0;
    int stride = 1;
    
    // ROW-MAJOR: iterate from last dimension to first (rightmost increments fastest)
    for (int d = indices.size() - 1; d >= 0; --d) {
        if (indices[d] < 0 || indices[d] >= nBins[d]) {
            log(LOG_ERROR, "flattenIndices: index out of bounds - indices[" + 
                std::to_string(d) + "]=" + std::to_string(indices[d]) + 
                ", nBins[" + std::to_string(d) + "]=" + std::to_string(nBins[d]));
            return -1;
        }
        flat += indices[d] * stride;
        stride *= nBins[d];
    }
    
    log(LOG_TRACE, "flattenIndices: [" + 
        [&]() {
            std::string result;
            for (size_t i = 0; i < indices.size(); ++i) {
                if (i > 0) result += ",";
                result += std::to_string(indices[i]);
            }
            return result;
        }() + "] -> " + std::to_string(flat));
    
    return flat;
}

// Helper: unflatten 1D index back to N-dimensional indices using ROW-MAJOR ordering
inline std::vector<int> unflattenIndex(int flatIndex, const std::vector<int>& nBins) {
    std::vector<int> indices(nBins.size());
    int remainder = flatIndex;
    
    // ROW-MAJOR: extract indices from right to left
    for (int d = nBins.size() - 1; d >= 0; --d) {
        indices[d] = remainder % nBins[d];
        remainder /= nBins[d];
    }
    
    log(LOG_TRACE, "unflattenIndex: " + std::to_string(flatIndex) + " -> [" + 
        [&]() {
            std::string result;
            for (size_t i = 0; i < indices.size(); ++i) {
                if (i > 0) result += ",";
                result += std::to_string(indices[i]);
            }
            return result;
        }() + "]");
    
    return indices;
}

// Validation function to ensure flattening consistency
inline bool validateFlattening(const std::vector<int>& nBins) {
    log(LOG_DEBUG, "Validating flattening for dimensions: [" + 
        [&]() {
            std::string result;
            for (size_t i = 0; i < nBins.size(); ++i) {
                if (i > 0) result += ",";
                result += std::to_string(nBins[i]);
            }
            return result;
        }() + "]");
    
    // Test round-trip conversion for a few sample indices
    std::vector<std::vector<int>> testCases;
    if (nBins.size() == 2) {
        testCases = {{0,0}, {0,1}, {1,0}, {1,1}};
    } else if (nBins.size() == 3) {
        testCases = {{0,0,0}, {0,0,1}, {0,1,0}, {1,0,0}};
    }
    
    for (const auto& testIndex : testCases) {
        if (testIndex.size() != nBins.size()) continue;
        
        // Check bounds
        bool valid = true;
        for (size_t i = 0; i < testIndex.size(); ++i) {
            if (testIndex[i] >= nBins[i]) {
                valid = false;
                break;
            }
        }
        if (!valid) continue;
        
        int flat = flattenIndices(testIndex, nBins);
        auto recovered = unflattenIndex(flat, nBins);
        
        if (recovered != testIndex) {
            log(LOG_ERROR, "Flattening validation FAILED for test case");
            return false;
        }
    }
    
    log(LOG_DEBUG, "Flattening validation PASSED");
    return true;
}


#endif // UNFOLD_UTILS_H
