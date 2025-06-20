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


#endif // UNFOLD_UTILS_H
