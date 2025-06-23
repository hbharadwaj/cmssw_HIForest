#ifndef HELPERS_H
#define HELPERS_H

#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TSystem.h>
#include <TEnv.h>
#include <TStyle.h>
#include <TROOT.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TLatex.h>
#include <TPad.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TProfile.h>
#include <TObjArray.h>
#include <TObjString.h>
#include <TSystemDirectory.h>
#include <TSystemFile.h>
#include <TLeaf.h>
#include <TBranch.h>
#include <TList.h>
#include <THashList.h>
#include <TFriendElement.h>
#include <TLine.h>
#include <TMath.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <set>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <cstring>

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


// ============= Forward declarations =================
// =====================================================

// Global variables
int g_verbosity = 2; // Global verbosity level
extern struct PlottingConfiguration g_plotConfig;

// Structs
struct PlottingConfiguration;
struct HistogramConfig;

// Prints usage information for the program.
void printUsage();

// Sets up a ROOT TChain with input files from a directory or file.
bool setupInputChain(TChain* chain, const std::string& inputDir, bool testMode, int maxFiles = 1);

// Sets up a ROOT TChain with list of input files for batch submission
bool setupInputChain(TChain* chain, const std::vector<std::string>& files, bool testMode, int maxFiles = 1);

// Parse comma-separated file list
std::vector<std::string> parseFileList(const std::string& fileList);

// Calculates the absolute difference in phi between two angles, returned in [0, pi].
float getDeltaPhi(float phi1, float phi2);

// Calculates the ratio xj = jetPt / photonPt.
float getXj(float jetPt, float photonPt);

// Creates output directories if they do not exist.
bool createOutputDirectories(const std::string& outputDir);

// Prints a summary of the configuration parameters.
void printConfig(TEnv* config);

// Parses a comma- or space-separated list of floats from a config parameter.
std::vector<float> getFloatVector(TEnv* config, const std::string& param);

// Parses a comma-separated list of strings from a config parameter.
std::vector<std::string> getStringVector(TEnv* config, const std::string& param);

// Loads plotting configuration from a file into a PlottingConfiguration object.
bool loadPlottingConfig(const std::string& configPath, PlottingConfiguration& plotConfig, bool updateGlobal = true);

// Loads and merges histogram configuration from a TEnv into a PlottingConfiguration.
void loadHistogramConfigsFromEnv(TEnv* env, PlottingConfiguration& plotConfig);

// Creates a 1D histogram with configuration options.
TH1D* createHistogram1D(const HistogramConfig& config, const std::string& name = "", const std::string& title = "", int colorIndex = -1);

// Creates a 2D histogram with configuration options.
TH2D* createHistogram2D(const HistogramConfig& config, const std::string& name = "", const std::string& title = "", int colorIndex = -1);

// Creates a profile histogram with configuration options.
TProfile* createProfile(const HistogramConfig& config, const std::string& name = "", const std::string& title = "", int colorIndex = -1);

// Lists all branches and types in a TTree.
void ListBranchesAndTypes(TTree* tree, const TString& prefix = "");

// Lists all branches and types in a TChain and its friends.
void ListBranchesAndTypesWithFriends(TChain* base);

// Parses a comma-separated string into a vector of doubles.
std::vector<double> parseVector(const std::string& vecStr);

// Parses a comma-separated string into a pair representing a range.
std::pair<double, double> parseRange(const std::string& rangeStr);

// Applies style settings to a histogram.
void applyHistogramStyle(TH1* hist, const HistogramConfig& config, int colorIndex);

// Returns a color index from a color scheme.
int getAutoColor(const std::string& colorScheme, int index);

// Parses a delimited string into a vector of strings.
std::vector<std::string> parseStringVector(const std::string& str, char delimiter = ',');

// Parses a delimited string into a vector of doubles.
std::vector<double> parseDoubleVector(const std::string& str, char delimiter = ',');

// Recursively collects .root files from a directory.
std::vector<std::string> GetFiles(const std::string &dir, int limit = 99999);

// Creates a directory and all necessary parent directories.
bool createDirectory(const std::string& path);

// Parses a string for marker style into a int
int parseMarkerStyle(const std::string& style);

inline std::set<std::string> parseStringSet(const std::string& str);

// =====================================================
// ================ Beginning of the code ==============
// =====================================================

// Logging system
enum LogLevel { LOG_ERROR = 0, LOG_WARNING = 2, LOG_INFO = 1, LOG_DEBUG = 3, LOG_TRACE = 4 };

// Logging function with verbosity levels and colors
void log(LogLevel level, const std::string& message) {
    if (static_cast<int>(level) > g_verbosity) return;
    
    const char* colorCode = RESET_COLOR;
    const char* levelStr = "";
    
    switch (level) {
        case LOG_ERROR:   colorCode = RED_COLOR;     levelStr = "ERROR"; break;
        case LOG_WARNING: colorCode = YELLOW_COLOR;  levelStr = "WARNING"; break;
        case LOG_INFO:    colorCode = GREEN_COLOR;   levelStr = "INFO";  break;
        case LOG_DEBUG:   colorCode = MAGENTA_COLOR;  levelStr = "DEBUG"; break;
        case LOG_TRACE:   colorCode = CYAN_COLOR;    levelStr = "TRACE"; break;
    }
    
    std::cout << colorCode << "[" << levelStr << "] " << message << RESET_COLOR << std::endl;
}

float findNcoll(int hiBin) {
    const float Ncoll[200] = {1893.13, 1867.0, 1834.16, 1805.64, 1770.84, 1744.49, 1699.76, 1661.52, 1615.89, 1579.59, 1540.62, 1499.14, 1469.01, 1432.18, 1402.8, 1368.39, 1338.12, 1302.26, 1274.91, 1245.56, 1215.28, 1183.76, 1160.61, 1131.12, 1107.67, 1078.54, 1055.72, 1026.72, 1000.57, 980.728, 958.777, 936.515, 911.397, 889.182, 869.677, 853.33, 826.999, 808.145, 792.14, 769.639, 753.513, 732.883, 716.817, 697.168, 679.091, 668.056, 650.114, 631.024, 616.203, 597.835, 583.435, 571.454, 555.478, 543.589, 526.328, 511.657, 497.023, 489.255, 471.52, 461.133, 447.767, 436.993, 426.106, 412.626, 403.224, 389.71, 382.595, 371.48, 358.899, 349.179, 339.387, 330.523, 320.094, 313.254, 302.339, 292.421, 282.594, 274.834, 268.847, 259.463, 252.027, 244.561, 236.738, 229.574, 222.898, 215.138, 207.328, 200.879, 196.592, 190.921, 183.942, 176.685, 170.919, 166.96, 161.057, 154.421, 148.816, 144.84, 139.087, 134.448, 128.72, 124.905, 121.166, 116.648, 112.367, 109.012, 104.33, 100.736, 97.3484, 93.2283, 89.3299, 85.9068, 83.6446, 80.2019, 77.5299, 73.9647, 70.7606, 68.2284, 65.793, 63.4532, 60.4738, 58.2406, 55.063, 53.7287, 51.4638, 49.241, 47.0111, 45.5443, 43.1729, 41.5041, 39.5449, 37.9282, 36.8918, 34.9287, 33.1886, 31.9177, 30.756, 29.0803, 27.6721, 26.42, 25.2678, 24.2585, 23.1429, 22.0138, 21.0169, 19.8203, 19.1043, 18.1478, 17.1715, 16.3605, 15.4763, 14.7973, 14.1594, 13.3927, 12.795, 12.1059, 11.5921, 10.9751, 10.3213, 9.94434, 9.3518, 8.94274, 8.37618, 7.94437, 7.48868, 7.06923, 6.71137, 6.31856, 6.03184, 5.67048, 5.43369, 5.13727, 4.83292, 4.58846, 4.37208, 4.15225, 3.84385, 3.63752, 3.45214, 3.24892, 3.02845, 2.81715, 2.66395, 2.5053, 2.29512, 2.13703, 1.93591, 1.79771, 1.64165, 1.54375, 1.45878, 1.36718, 1.2942, 1.23934, 1.18423, 1.14467, 1.11826, 1.0863, 1.06149, 1.04497 };
    return Ncoll[hiBin];
 }

/**
 * Setup the input chain with files from the input directory
*/

/**
 * Centralized centrality bin handling for both PbPb and pp systems
 * Returns a struct containing all centrality-related information
 */
 struct CentralityInfo {
    bool isValid;           // Whether centrality binning applies to this event
    int binIndex;          // Index in centralityBins array (-1 for pp or invalid)
    std::string binName;   // String name like "cent0to60" or "inclusive"
    std::string displayName; // Display name for histograms
};

CentralityInfo getCentralityInfo(int hiBin, const std::vector<float>& centralityBins, bool useCentrality) {
    CentralityInfo info;
    
    if (!useCentrality) {
        // For pp system - use inclusive binning
        info.isValid = true;
        info.binIndex = -1;
        info.binName = "inclusive";
        info.displayName = "Inclusive";
        return info;
    }
    
    // For PbPb system - find appropriate centrality bin
    info.binIndex = -1;
    for (size_t i = 0; i < centralityBins.size() - 1; ++i) {
        if (hiBin >= centralityBins[i] && hiBin < centralityBins[i+1]) {
            info.binIndex = static_cast<int>(i);
            break;
        }
    }
    
    if (info.binIndex >= 0) {
        info.isValid = true;
        info.binName = "cent" + std::to_string(static_cast<int>(centralityBins[info.binIndex])/2) + 
                       "to" + std::to_string(static_cast<int>(centralityBins[info.binIndex+1])/2);
        info.displayName = std::to_string(static_cast<int>(centralityBins[info.binIndex])/2) + 
                          "-" + std::to_string(static_cast<int>(centralityBins[info.binIndex+1])/2) + "%";
    } else {
        info.isValid = false;
        info.binName = "";
        info.displayName = "";
    }
    
    return info;
}

bool setupInputChain(TChain* chain, const std::string& inputDir, bool testMode, int maxFiles) {
    if (!chain) return false;
    
    if (inputDir.empty()) {
        std::cerr << "Error: Input directory not specified." << std::endl;
        return false;
    }
    
    // Check if inputDir is a single file (ends with .root)
    if (inputDir.size() > 5 && inputDir.substr(inputDir.size() - 5) == ".root") {
        log(LOG_INFO, "Adding single file: " + inputDir);
        if (chain->AddFile(inputDir.c_str()) <= 0) {
            std::cerr << "Error: Failed to add file to chain: " << inputDir << std::endl;
            return false;
        }
        return true;
    }
    
    // Find all ROOT files in the directory
    std::string cmd = "find " + inputDir + " -name \"*.root\" -type f";
    log(LOG_DEBUG, "Executing: " + cmd);
    
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        std::cerr << "Error: Failed to run command: " << cmd << std::endl;
        return false;
    }
    
    std::vector<std::string> files;
    char buffer[1024];
    while (!feof(pipe)) {
        if (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            std::string file = buffer;
            // Remove newline characters
            file.erase(std::remove(file.begin(), file.end(), '\n'), file.end());
            if (!file.empty()) {
                files.push_back(file);
            }
        }
    }
    pclose(pipe);
    
    // Limit number of files in test mode
    if (testMode && files.size() > static_cast<size_t>(maxFiles)) {
        log(LOG_INFO, "Test mode: limiting to " + std::to_string(maxFiles) + " files out of " + std::to_string(files.size()));
        files.resize(maxFiles);
    }
    
    // Add files to chain
    for (const auto& file : files) {
        log(LOG_DEBUG, "Adding file: " + file);
        if (chain->AddFile(file.c_str()) <= 0) {
            std::cerr << "Warning: Failed to add file to chain: " << file << std::endl;
            continue;
        }
    }
    
    if (chain->GetEntries() <= 0) {
        std::cerr << "Error: No entries in chain." << std::endl;
        return false;
    }
    
    log(LOG_INFO, "Added " + std::to_string(files.size()) + " files with " + std::to_string(chain->GetEntries()) + " entries.");
    return true;
}
/**
 * Parse comma-separated file list
 */
 std::vector<std::string> parseFileList(const std::string& fileList) {
    std::vector<std::string> files;
    if (fileList.empty()) return files;
    
    std::stringstream ss(fileList);
    std::string file;
    
    while (std::getline(ss, file, ',')) {
        // Trim whitespace
        file.erase(0, file.find_first_not_of(" \t"));
        file.erase(file.find_last_not_of(" \t") + 1);
        if (!file.empty()) {
            files.push_back(file);
        }
    }
    
    return files;
}

/**
 * Setup the input chain with explicit file list (for batch mode)
*/
bool setupInputChain(TChain* chain, const std::vector<std::string>& files, bool testMode, int maxFiles) {
    if (!chain) return false;
    
    if (files.empty()) {
        std::cerr << "Error: No files specified." << std::endl;
        return false;
    }
    
    std::vector<std::string> filesToAdd = files;
    
    // Limit number of files in test mode
    if (testMode && filesToAdd.size() > static_cast<size_t>(maxFiles)) {
        log(LOG_INFO, "Test mode: limiting to " + std::to_string(maxFiles) + " files out of " + std::to_string(filesToAdd.size()));
        filesToAdd.resize(maxFiles);
    }
    
    // Add files to chain
    for (const auto& file : filesToAdd) {
        log(LOG_DEBUG, "Adding file: " + file);
        if (chain->AddFile(file.c_str()) <= 0) {
            std::cerr << "Warning: Failed to add file to chain: " << file << std::endl;
            continue;
        }
    }
    
    if (chain->GetEntries() <= 0) {
        std::cerr << "Error: No entries in chain." << std::endl;
        return false;
    }
    
    log(LOG_INFO, "Added " + std::to_string(filesToAdd.size()) + " files with " + std::to_string(chain->GetEntries()) + " entries.");
    return true;
}

struct HistogramConfig {
    // Basic histogram properties
    std::string name;
    std::string title;
    std::string type;
    
    // Axis properties
    std::string xTitle;
    std::string yTitle;
    std::string zTitle;
    
    // Binning for X axis
    int nBinsX;
    double xMin;
    double xMax;
    std::vector<double> binEdgesX;
    
    // Binning for Y axis (for 2D histograms)
    int nBinsY;
    double yMin;
    double yMax;
    std::vector<double> binEdgesY;
    
    // Display properties
    int color;
    int marker;
    int markerStyle; // NEW: Marker style (ROOT enum or int)
    double markerSize; // NEW: Marker size
    int lineStyle;
    double lineWidth;
    std::string drawOption;
    
    HistogramConfig() : 
        name(""), title(""), type("TH1D"),
        xTitle(""), yTitle(""), zTitle(""),
        nBinsX(50), xMin(0), xMax(100),
        nBinsY(50), yMin(0), yMax(100),
        color(1), marker(20), markerStyle(20), markerSize(0.8), lineStyle(1), lineWidth(2), drawOption("") {}
};

/**
 * ============================================================================
 *  Histogram Configuration Convention (TEnv-based)
 * ============================================================================
 *  - Centralize all default histogram settings using keys like:
 *      Histogram.Default.Bins = 50
 *      Histogram.Default.XMin = 0
 *      Histogram.Default.XMax = 100
 *      Histogram.Default.LineWidth = 2
 *      ...
 *  - For each histogram, override only what is needed using:
 *      Histogram.<Name>.Bins = 100
 *      Histogram.<Name>.Title = Jet p_{T};p_{T} [GeV/c];Entries
 *      Histogram.<Name>.XMax = 500
 *      ...
 *  - The loader (see loadHistogramConfigsFromEnv) will:
 *      1. Read all Histogram.Default.* keys into histogramDefaults
 *      2. Read all Histogram.<Name>.* keys into histogramRawConfigs[<Name>]
 *      3. For each histogram, merge defaults with overrides to produce histogramConfigs[<Name>]
 *  - To add a new histogram:
 *      - Add only the keys you want to override for that histogram.
 *      - All other settings will be inherited from the defaults.
 *  - This enables maintainable, DRY, and consistent histogram configuration.
 * ============================================================================
*/
struct PlottingConfiguration {
    // General plotting settings
    bool enablePlotting = true;
    bool savePlots = true;
    std::string plotOutputDir = "plots";
    bool plotSubdirs = true;
    std::vector<std::string> plotFormats = {"pdf", "png"};
    
    // CMS Style settings
    bool useCMSStyle = true;
    std::string cmsLabel = "Preliminary";
    std::string cmsEnergyText = "5.36 TeV PbPb";
    std::string cmsLuminosity = "1.61 nb^{-1}";
    std::string cmsExtraText = "";
    
    // Color and style
    std::string colorScheme = "petroff6";
    bool useColorBlind = true;
    
    // Canvas settings
    int canvasWidth = 800;
    int canvasHeight = 600;
    int canvasDPI = 600;
    double marginLeft = 0.12;
    double marginRight = 0.05;
    double marginTop = 0.08;
    double marginBottom = 0.12;
    
    // Legend settings
    double legendX1 = 0.65;
    double legendY1 = 0.70;
    double legendX2 = 0.90;
    double legendY2 = 0.90;
    bool drawLegend = true;
    
    // Display options
    bool showErrorBars = true;
    
    // Histogram configurations
    std::map<std::string, HistogramConfig> histogramConfigs;
    // Histogram category sets
    std::set<std::string> eventHistograms;
    std::set<std::string> generalHistograms;
    std::set<std::string> jetHistograms;

    // --- Histogram config merging for TEnv-based configs ---
    // Centralized histogram style/config management using TEnv keys.
    // See convention above for usage.
    std::map<std::string, std::string> histogramDefaults;
    // Stores per-histogram overrides from keys like Histogram.<Name>.*
    std::map<std::string, std::map<std::string, std::string>> histogramRawConfigs;
    // --------------------------------------------------------------

    // Usage:
    //  - histogramDefaults: stores all Histogram.Default.* key-value pairs
    //  - histogramRawConfigs: stores all Histogram.<Name>.* key-value pairs
    //  - histogramConfigs: after merging defaults with overrides for each histogram
    
    // Plotting options
    bool createOverlayPlots = true;
    bool create2DPlots = true;
    bool createProfilePlots = true;
    bool createRatioPlots = false;
    
    // Configuration file reference for MC detection
    TEnv* configFile = nullptr;
    
    PlottingConfiguration() {}
};

// Global plotting configuration definition
PlottingConfiguration g_plotConfig;

/*
 * Load and merge histogram configs from TEnv into PlottingConfiguration.
 * - Reads all Histogram.Default.* and Histogram.<Name>.* keys.
 * - Merges defaults with per-histogram overrides.
 * - Populates plotConfig.histogramConfigs for use in histogram creation.
 * - Usage: call after loading your TEnv config.
*/
inline void loadHistogramConfigsFromEnv(TEnv* env, PlottingConfiguration& plotConfig) {
    // plotConfig.histogramDefaults.clear();
    // plotConfig.histogramRawConfigs.clear();
    // plotConfig.histogramConfigs.clear();

    // Collect all keys from TEnv
    TList* keys = env->GetTable();
    if (!keys) return;

    TIter next(keys);
    TEnvRec* rec = nullptr;
    while ((rec = (TEnvRec*)next())) {
        std::string key = rec->GetName();
        std::string value = rec->GetValue();

        // Histogram.Default.*
        if (key.find("Histogram.Default.") == 0) {
            std::string subkey = key.substr(std::string("Histogram.Default.").size());
            plotConfig.histogramDefaults[subkey] = value;
        }
        // Histogram.<Name>.*
        else if (key.find("Histogram.") == 0) {
            size_t nextDot = key.find('.', std::string("Histogram.").size());
            if (nextDot != std::string::npos) {
                std::string histName = key.substr(std::string("Histogram.").size(), nextDot - std::string("Histogram.").size());
                std::string subkey = key.substr(nextDot + 1);
                plotConfig.histogramRawConfigs[histName][subkey] = value;
            }
        }
    }

    // For each histogram, merge defaults and overrides, then populate histogramConfigs
    for (const auto& kv : plotConfig.histogramRawConfigs) {
        const std::string& histName = kv.first;
        const auto& overrides = kv.second;
        std::map<std::string, std::string> merged = plotConfig.histogramDefaults;
        for (const auto& okv : overrides) {
            merged[okv.first] = okv.second;
        }
        HistogramConfig hcfg;
        hcfg.name = histName;
        hcfg.title = merged.count("Title") ? merged["Title"] : histName;
        hcfg.type = merged.count("PlotType") ? merged["PlotType"] : "TH1D";
        hcfg.xTitle = merged.count("XTitle") ? merged["XTitle"] : "";
        hcfg.yTitle = merged.count("YTitle") ? merged["YTitle"] : "";
        hcfg.zTitle = merged.count("ZTitle") ? merged["ZTitle"] : "";
        try { hcfg.nBinsX = merged.count("Bins") ? std::stoi(merged["Bins"]) : 50; }
        catch (const std::exception& e) { log(LOG_ERROR, "Failed to parse Bins for " + histName + ": '" + merged["Bins"] + "' (" + e.what() + "). Using default 50."); hcfg.nBinsX = 50; }
        try { hcfg.xMin = merged.count("XMin") ? std::stod(merged["XMin"]) : 0.0; }
        catch (const std::exception& e) { log(LOG_ERROR, "Failed to parse XMin for " + histName + ": '" + merged["XMin"] + "' (" + e.what() + "). Using default 0.0."); hcfg.xMin = 0.0; }
        try { hcfg.xMax = merged.count("XMax") ? std::stod(merged["XMax"]) : 100.0; }
        catch (const std::exception& e) { log(LOG_ERROR, "Failed to parse XMax for " + histName + ": '" + merged["XMax"] + "' (" + e.what() + "). Using default 100.0."); hcfg.xMax = 100.0; }
        if (merged.count("BinEdges") || merged.count("BinEdgesX")) {
            std::string binEdgesStr = merged.count("BinEdges") ? merged["BinEdges"] : merged["BinEdgesX"];
            hcfg.binEdgesX = parseDoubleVector(binEdgesStr);
            if (!hcfg.binEdgesX.empty()) {
                hcfg.nBinsX = hcfg.binEdgesX.size() - 1;
                hcfg.xMin = hcfg.binEdgesX.front();
                hcfg.xMax = hcfg.binEdgesX.back();
                log(LOG_DEBUG, "Parsed variable bin edges for " + histName + " (" + std::to_string(hcfg.nBinsX) + " bins)");
            }
        }        
        try { hcfg.nBinsY = merged.count("YBins") ? std::stoi(merged["YBins"]) : 50; }
        catch (const std::exception& e) { log(LOG_ERROR, "Failed to parse YBins for " + histName + ": '" + merged["YBins"] + "' (" + e.what() + "). Using default 50."); hcfg.nBinsY = 50; }
        try { hcfg.yMin = merged.count("YMin") ? std::stod(merged["YMin"]) : 0.0; }
        catch (const std::exception& e) { log(LOG_ERROR, "Failed to parse YMin for " + histName + ": '" + merged["YMin"] + "' (" + e.what() + "). Using default 0.0."); hcfg.yMin = 0.0; }
        try { hcfg.yMax = merged.count("YMax") ? std::stod(merged["YMax"]) : 100.0; }
        catch (const std::exception& e) { log(LOG_ERROR, "Failed to parse YMax for " + histName + ": '" + merged["YMax"] + "' (" + e.what() + "). Using default 100.0."); hcfg.yMax = 100.0; }
        if (merged.count("BinEdgesY")) {
            hcfg.binEdgesY = parseDoubleVector(merged["BinEdgesY"]);
            if (!hcfg.binEdgesY.empty()) {
                hcfg.nBinsY = hcfg.binEdgesY.size() - 1;
                hcfg.yMin = hcfg.binEdgesY.front();
                hcfg.yMax = hcfg.binEdgesY.back();
                log(LOG_DEBUG, "Parsed variable bin edges for " + histName + " (" + std::to_string(hcfg.nBinsY) + " bins)");
            }
        }
        try { hcfg.marker = merged.count("Marker") ? std::stoi(merged["Marker"]) : 20; }
        catch (const std::exception& e) { log(LOG_ERROR, "Failed to parse Marker for " + histName + ": '" + merged["Marker"] + "' (" + e.what() + "). Using default 20."); hcfg.marker = 20; }
        hcfg.markerStyle = merged.count("MarkerStyle") ? parseMarkerStyle(merged["MarkerStyle"]) : 20;
        try { hcfg.markerSize = merged.count("MarkerSize") ? std::stod(merged["MarkerSize"]) : 0.8; }
        catch (const std::exception& e) { log(LOG_ERROR, "Failed to parse MarkerSize for " + histName + ": '" + merged["MarkerSize"] + "' (" + e.what() + "). Using default 0.8."); hcfg.markerSize = 0.8; }
        try { hcfg.lineStyle = merged.count("LineStyle") ? std::stoi(merged["LineStyle"]) : 1; }
        catch (const std::exception& e) { log(LOG_ERROR, "Failed to parse LineStyle for " + histName + ": '" + merged["LineStyle"] + "' (" + e.what() + "). Using default 1."); hcfg.lineStyle = 1; }
        try { hcfg.lineWidth = merged.count("LineWidth") ? std::stod(merged["LineWidth"]) : 2.0; }
        catch (const std::exception& e) { log(LOG_ERROR, "Failed to parse LineWidth for " + histName + ": '" + merged["LineWidth"] + "' (" + e.what() + "). Using default 2.0."); hcfg.lineWidth = 2.0; }
        hcfg.drawOption = merged.count("DrawOption") ? merged["DrawOption"] : "E1][P0";
        // Color parsing with 'auto' support
        if (merged.count("Color")) {
            std::string colorStr = merged["Color"];
            if (colorStr == "auto" || colorStr == "AUTO" || colorStr == "Auto") {
                hcfg.color = -1; // Use auto-color logic later
            } else {
                try { hcfg.color = std::stoi(colorStr); }
                catch (const std::exception& e) {
                    log(LOG_ERROR, "Failed to parse Color for " + histName + ": '" + colorStr + "' (" + e.what() + "). Using default 1.");
                    hcfg.color = 1;
                }
            }
        } else {
            hcfg.color = 1;
        }

        log(LOG_TRACE, "Parsed config for " + histName);
        
        plotConfig.histogramConfigs[histName] = hcfg;
    }

    // Parse histogram category lists from config
    std::string eventHistStr = env->GetValue("EventHistograms", "");
    std::string generalHistStr = env->GetValue("GeneralHistograms", "");
    std::string jetHistStr = env->GetValue("JetHistograms", "");
    // Robust merging: add (do not overwrite) new entries from this config
    for (const auto& s : parseStringSet(eventHistStr)) plotConfig.eventHistograms.insert(s);
    for (const auto& s : parseStringSet(generalHistStr)) plotConfig.generalHistograms.insert(s);
    for (const auto& s : parseStringSet(jetHistStr)) plotConfig.jetHistograms.insert(s);
}

/**
 * Create output directories using ROOT's TSystem
*/
bool createOutputDirectories(const std::string& outputDir) {
    if (outputDir.empty()) {
        std::cerr << "Error: Empty output directory path" << std::endl;
        return false;
    }
    
    // Check if directory exists, create if it doesn't
    if (gSystem->AccessPathName(outputDir.c_str())) {
        if (gSystem->mkdir(outputDir.c_str(), kTRUE) != 0) {
            std::cerr << "Error: Failed to create output directory: " << outputDir << std::endl;
            return false;
        }
        log(LOG_INFO, "Created output directory: " + outputDir);
    } else {
        log(LOG_INFO, "Output directory exists: " + outputDir);
    }
    
    return true;
}

/**
 * Print configuration summary
 */
 void printConfig(TEnv* config) {
    if (!config) {
        std::cerr << "Error: Configuration is null" << std::endl;
        return;
    }
    
    // Get DataType and compute isMC status
    std::string dataType = config->GetValue("DataType", "Data");
    bool isMC = (dataType == "MC" || dataType == "mc");
    
    log(LOG_INFO, "=== Configuration Summary ===");
    log(LOG_INFO, "System: " + std::string(config->GetValue("System", "Unknown")));
    log(LOG_INFO, "DataType: " + dataType);
    log(LOG_INFO, "IsMC: " + std::to_string(isMC) + " (computed from DataType)");
    log(LOG_INFO, "InputDir: " + std::string(config->GetValue("InputDir", "")));
    log(LOG_INFO, "OutputDir: " + std::string(config->GetValue("OutputDir", "")));
    log(LOG_INFO, "OutputPrefix: " + std::string(config->GetValue("OutputPrefix", "")));
    log(LOG_INFO, "AnalysisCases: " + std::string(config->GetValue("AnalysisCases", "")));
    log(LOG_INFO, "PhotonEtMin: " + std::to_string(config->GetValue("PhotonEtMin", 60.0)));
    log(LOG_INFO, "JetPtMin: " + std::to_string(config->GetValue("JetPtMin", 40.0)));
    log(LOG_INFO, "VzCut: " + std::to_string(config->GetValue("VzCut", 15.0)));
    log(LOG_INFO, "===========================");
}

/**
 * Parse comma-separated float values from config
*/
std::vector<float> getFloatVector(TEnv* config, const std::string& param) {
    std::vector<float> result;
    std::string valueStr = config->GetValue(param.c_str(), "");
    
    if (valueStr.empty()) {
        // Return default centrality bins if not specified
        return {0, 10, 30, 50, 70, 100, 200};
    }
    
    // Handle both space and comma separated values
    std::stringstream ss(valueStr);
    std::string item;
    
    // First try comma-separated parsing
    if (valueStr.find(',') != std::string::npos) {
        while (std::getline(ss, item, ',')) {
            // Trim whitespace
            item.erase(0, item.find_first_not_of(" \t\n\r\f\v"));
            item.erase(item.find_last_not_of(" \t\n\r\f\v") + 1);
            if (!item.empty()) {
                try {
                    float value = std::stof(item);
                    result.push_back(value);
                } 
                catch (const std::exception& e) {
                    std::cerr << "Warning: Failed to parse float value '" << item << "' in parameter " << param << std::endl;
                }
            }
        }
    } 
    else {
        // Space-separated parsing
        while (ss >> item) {
            try {
                float value = std::stof(item);
                result.push_back(value);
            } 
            catch (const std::exception& e) {
                std::cerr << "Warning: Failed to parse float value '" << item << "' in parameter " << param << std::endl;
            }
        }
    }
    
    return result;
}

/**
 * Parse comma-separated string values from config
*/
std::vector<std::string> getStringVector(TEnv* config, const std::string& param) {
    std::vector<std::string> result;
    std::string valueStr = config->GetValue(param.c_str(), "");
    
    if (valueStr.empty()) {
        return result;
    }
    
    std::stringstream ss(valueStr);
    std::string item;
    while (std::getline(ss, item, ',')) {
        // Trim whitespace
        item.erase(0, item.find_first_not_of(" \t\n\r\f\v"));
        item.erase(item.find_last_not_of(" \t\n\r\f\v") + 1);
        
        if (!item.empty()) {
            result.push_back(item);
        }
    }
    
    return result;
}

/**
 * Calculate delta phi between two angles, returned in [0, pi]
*/
float getDeltaPhi(float phi1, float phi2) {
    float dPhi = std::abs(phi1 - phi2);
    while (dPhi > M_PI) dPhi = 2 * M_PI - dPhi;
    return dPhi;
}

/**
 * Calculate xj = jetPt / photonPt
*/
float getXj(float jetPt, float photonPt) {
    if (photonPt <= 0) return 0;
    return jetPt / photonPt;
}

/**
 * Print usage information
*/
void printUsage() {
    std::cout << "Usage: gammaJetAnalyzer [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --config, -c FILE          Analysis config file path" << std::endl;
    std::cout << "  --plot-config, -p FILE     Plotting config file path" << std::endl;
    std::cout << "  --files, -f LIST           Comma-separated list of input files (batch mode)" << std::endl;
    std::cout << "  --test, -t [N]             Run in test mode with N events (default: 1000)" << std::endl;
    std::cout << "  --batchid, -b [N]          Run in batch mode - batch ID" << std::endl;
    std::cout << "  --help, -h                 Print this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  ./gammaJetAnalyzer -c analysis.config -p plotting.config -t 10000" << std::endl;
    std::cout << "  ./gammaJetAnalyzer --config analysis.config --plot-config plotting.config --test 5000" << std::endl;
    std::cout << "  ./gammaJetAnalyzer -c analysis.config -p plotting.config --production" << std::endl;
    std::cout << "  ./gammaJetAnalyzer -c analysis.config -p plotting.config --files file1.root,file2.root,file3.root" << std::endl;
}

/**
 * Load plotting configuration from file
*/
bool loadPlottingConfig(const std::string& configPath, PlottingConfiguration& plotConfig, bool updateGlobal) {
    TEnv* config = new TEnv();
    
    log(LOG_DEBUG, "Loading plotting configuration from: " + configPath);
    
    if (gSystem->AccessPathName(configPath.c_str())) {
        log(LOG_ERROR, "Plotting config file not found: " + configPath);
        delete config;
        return false;
    }
    
    int readStatus = config->ReadFile(configPath.c_str(), kEnvLocal);
    if (readStatus != 0) {
        log(LOG_ERROR, "Failed to read plotting config file: " + configPath);
        delete config;
        return false;
    }
    
    // General settings
    plotConfig.enablePlotting = config->GetValue("EnablePlotting", 1);
    plotConfig.savePlots = config->GetValue("SavePlots", 1);
    plotConfig.plotOutputDir = config->GetValue("PlotOutputDir", "plots");
    plotConfig.plotSubdirs = config->GetValue("PlotSubdirs", 1);
    
    // Parse plot formats
    std::string formatsStr = config->GetValue("PlotFormats", "pdf,png");
    plotConfig.plotFormats = parseStringVector(formatsStr);
    
    // CMS Style settings
    plotConfig.useCMSStyle = config->GetValue("UseCMSStyle", 1);
    plotConfig.cmsLabel = config->GetValue("CMSLabel", "Preliminary");
    plotConfig.cmsEnergyText = config->GetValue("CMSEnergyText", "5.36 TeV PbPb");
    plotConfig.cmsLuminosity = config->GetValue("CMSLuminosity", "1.61 nb^{-1}");
    plotConfig.cmsExtraText = config->GetValue("CMSExtraText", "");
    
    // Color scheme
    plotConfig.colorScheme = config->GetValue("ColorScheme", "petroff6");
    plotConfig.useColorBlind = config->GetValue("UseColorBlind", 1);
    
    // Canvas settings
    plotConfig.canvasWidth = config->GetValue("CanvasWidth", 800);
    plotConfig.canvasHeight = config->GetValue("CanvasHeight", 600);
    plotConfig.canvasDPI = config->GetValue("CanvasDPI", 300);
    
    // Canvas margins
    plotConfig.marginLeft = config->GetValue("CanvasMarginLeft", 0.15);
    plotConfig.marginRight = config->GetValue("CanvasMarginRight", 0.05);
    plotConfig.marginTop = config->GetValue("CanvasMarginTop", 0.08);
    plotConfig.marginBottom = config->GetValue("CanvasMarginBottom", 0.12);
    
    // Legend settings
    plotConfig.legendX1 = config->GetValue("LegendX1", 0.65);
    plotConfig.legendY1 = config->GetValue("LegendY1", 0.70);
    plotConfig.legendX2 = config->GetValue("LegendX2", 0.90);
    plotConfig.legendY2 = config->GetValue("LegendY2", 0.90);
    plotConfig.drawLegend = config->GetValue("DrawLegend", 1);
    
    // Plot options
    plotConfig.createOverlayPlots = config->GetValue("CreateOverlayPlots", 1);
    plotConfig.create2DPlots = config->GetValue("Create2DPlots", 1);
    plotConfig.createProfilePlots = config->GetValue("CreateProfilePlots", 1);
    plotConfig.createRatioPlots = config->GetValue("CreateRatioPlots", 0);
    
    if (updateGlobal) {
        g_plotConfig = plotConfig;
    }
    
    delete config;
    log(LOG_DEBUG, "Successfully loaded plotting configuration");
    return true;
}

/**
 * Enhanced 1D histogram creation with configuration
 */
TH1D* createHistogram1D(const HistogramConfig& config, const std::string& name, 
                       const std::string& title, int colorIndex) {
    // Use name and title from config if not provided
    std::string histName = name.empty() ? config.name : name;
    std::string histTitle = title.empty() ? config.title : title;
    
    // Add axis titles if available
    if (!config.xTitle.empty() || !config.yTitle.empty()) {
        histTitle += ";" + config.xTitle + ";" + config.yTitle;
    }
    
    TH1D* hist = nullptr;
    // Use variable binning if binEdgesX is set
    if (!config.binEdgesX.empty()) {
        hist = new TH1D(histName.c_str(), histTitle.c_str(), 
                        config.binEdgesX.size() - 1, config.binEdgesX.data());
    } else {
        hist = new TH1D(histName.c_str(), histTitle.c_str(), 
                        config.nBinsX, config.xMin, config.xMax);
    }
    
    // Apply styling with the provided color index or default
    applyHistogramStyle(hist, config, colorIndex >= 0 ? colorIndex : 0);
    
    return hist;
}

/**
 * Enhanced 2D histogram creation with configuration
 */
TH2D* createHistogram2D(const HistogramConfig& config, const std::string& name, 
                       const std::string& title, int colorIndex) {
    std::string histName = name.empty() ? config.name : name;
    std::string histTitle = title.empty() ? config.title : title;
    
    // Add axis titles if available
    if (!config.xTitle.empty() || !config.yTitle.empty() || !config.zTitle.empty()) {
        histTitle += ";" + config.xTitle + ";" + config.yTitle + ";" + config.zTitle;
    }
    
    // Create histogram with the specified binning
    TH2D* hist = nullptr;
    if (!config.binEdgesX.empty() && !config.binEdgesY.empty()) {
        hist = new TH2D(histName.c_str(), histTitle.c_str(),
                        config.binEdgesX.size() - 1, config.binEdgesX.data(),
                        config.binEdgesY.size() - 1, config.binEdgesY.data());
    } else if (!config.binEdgesX.empty()) {
        hist = new TH2D(histName.c_str(), histTitle.c_str(),
                        config.binEdgesX.size() - 1, config.binEdgesX.data(),
                        config.nBinsY, config.yMin, config.yMax);
    } else if (!config.binEdgesY.empty()) {
        hist = new TH2D(histName.c_str(), histTitle.c_str(),
                        config.nBinsX, config.xMin, config.xMax,
                        config.binEdgesY.size() - 1, config.binEdgesY.data());
    } else {
        hist = new TH2D(histName.c_str(), histTitle.c_str(),
                        config.nBinsX, config.xMin, config.xMax,
                        config.nBinsY, config.yMin, config.yMax);
    }
    
    // Apply styling
    applyHistogramStyle(hist, config, colorIndex >= 0 ? colorIndex : 0);
    
    // Additional 2D-specific settings
    hist->SetOption("COLZ"); // Default to color map with z scale
    
    return hist;
}

/**
 * Enhanced profile histogram creation with configuration
 */
TProfile* createProfile(const HistogramConfig& config, const std::string& name, 
                       const std::string& title, int colorIndex) {
    std::string histName = name.empty() ? config.name : name;
    std::string histTitle = title.empty() ? config.title : title;
    
    // Add axis titles if available
    if (!config.xTitle.empty() || !config.yTitle.empty()) {
        histTitle += ";" + config.xTitle + ";" + config.yTitle;
    }
    
    // Create profile with the specified binning
    TProfile* hist = new TProfile(histName.c_str(), histTitle.c_str(), 
                                 config.nBinsX, config.xMin, config.xMax,
                                 config.yMin, config.yMax);
    
    // Apply styling
    applyHistogramStyle(hist, config, colorIndex >= 0 ? colorIndex : 0);
    
    // Additional profile-specific settings
    hist->SetErrorOption(""); // Standard error calculation
    
    return hist;
}

std::vector<std::string> parseStringVector(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;
    
    while (std::getline(ss, item, delimiter)) {
        // Trim whitespace
        item.erase(0, item.find_first_not_of(" \t"));
        item.erase(item.find_last_not_of(" \t") + 1);
        
        if (!item.empty()) {
            result.push_back(item);
        }
    }
    
    return result;
}

std::vector<double> parseDoubleVector(const std::string& str, char delimiter) {
    std::vector<double> result;
    std::stringstream ss(str);
    std::string item;
    
    while (std::getline(ss, item, delimiter)) {
        // Trim whitespace
        item.erase(0, item.find_first_not_of(" \t"));
        item.erase(item.find_last_not_of(" \t") + 1);
        
        if (!item.empty()) {
            result.push_back(std::stod(item));
        }
    }
    
    return result;
}

// M. Petroff color schemes
std::vector<int> getPetroffColors(const std::string& scheme) {
    if (scheme == "qualitative") {
        return {1,862, 834, 838, 866, 870, 874, 878, 882};
    } else if (scheme == "sequential") {
        return {1,51, 52, 53, 54, 55, 56, 57, 58, 59};
    } else {
        // Default scheme
        return {1,634, 628, 807, 823, 419, 602, 921, 622};
    }
}


int getAutoColor(const std::string& colorScheme, int index) {
    std::vector<int> colors = getPetroffColors(colorScheme);
    return colors[index % colors.size()];
}

void applyHistogramStyle(TH1* hist, const HistogramConfig& config, int colorIndex) {
    if (!hist) return;
    
    hist->SetLineColor(config.color > 0 ? config.color : getAutoColor("default", colorIndex));
    hist->SetMarkerColor(config.color > 0 ? config.color : getAutoColor("default", colorIndex));
    hist->SetMarkerStyle(config.marker);
    hist->SetLineStyle(config.lineStyle);
    hist->SetLineWidth(config.lineWidth);
}

void ListBranchesAndTypes(TTree* tree, const TString& prefix) {
    if (!tree) return;

    std::cout << prefix << "Tree: " << tree->GetName() << "\n";
    TObjArray* branches = tree->GetListOfBranches();

    for (int i = 0; i < branches->GetEntries(); ++i) {
        TBranch* br = (TBranch*)branches->At(i);
        Bool_t isActive = tree->GetBranchStatus(br->GetName());
        TString statusStr = isActive ? "ENABLED" : "DISABLED";

        TObjArray* leaves = br->GetListOfLeaves();
        for (int j = 0; j < leaves->GetEntries(); ++j) {
            TLeaf* leaf = (TLeaf*)leaves->At(j);
            TString leafName = leaf->GetName();
            TString typeName = leaf->GetTypeName();
            TString countStr;

            if (leaf->GetLenStatic() > 1) {
                countStr = Form("[%d]", leaf->GetLenStatic());
            } else if (leaf->GetLeafCount()) {
                countStr = Form("[%s]", leaf->GetLeafCount()->GetName());
            }

            std::cout << "  " << leafName << " : " << typeName
                      << " " << countStr << " -- " << statusStr << "\n";
        }
    }
}

void ListBranchesAndTypesWithFriends(TChain* base) {
    if (!base) {
        std::cerr << "Error: Null TChain pointer.\n";
        return;
    }

    // Print base tree branches
    ListBranchesAndTypes(base, "Base ");

    // Print friends
    TList* friends = base->GetListOfFriends();
    if (friends && friends->GetSize() > 0) {
        TIter nextFriend(friends);
        TObject* obj;
        while ((obj = nextFriend())) {
            TFriendElement* fe = dynamic_cast<TFriendElement*>(obj);
            if (!fe) continue;

            TTree* friendTree = (TTree*)fe->GetTree();
            if (!friendTree) {
                std::cerr << "  Friend " << fe->GetName() << " has null TTree pointer.\n";
                continue;
            }

            ListBranchesAndTypes(friendTree, "Friend ");
        }
    }
}

// Helper function to recursively collect .root files
std::vector<std::string> GetFiles(const std::string &dir, int limit) {
    std::vector<std::string> files;
    TSystemDirectory directory(dir.c_str(), dir.c_str());
    TList *fileList = directory.GetListOfFiles();
    
    if (!fileList) {
        std::cerr << "Error: Could not access directory " << dir << std::endl;
        return files;
    }
    
    TSystemFile *file;
    TString fileName;
    TIter next(fileList);
    
    while ((file = (TSystemFile*)next()) && files.size() < static_cast<size_t>(limit)) {
        fileName = file->GetName();
        
        if (!file->IsDirectory() && fileName.EndsWith(".root")) {
            std::string fullPath = dir;
            if (fullPath.back() != '/') fullPath += "/";
            fullPath += fileName.Data();
            files.push_back(fullPath);
        } else if (file->IsDirectory() && fileName != "." && fileName != "..") {
            std::string subDir = dir;
            if (subDir.back() != '/') subDir += "/";
            subDir += fileName.Data();
            
            std::vector<std::string> subFiles = GetFiles(subDir, limit - files.size());
            files.insert(files.end(), subFiles.begin(), subFiles.end());
        }
    }
    
    delete fileList;
    return files;
}

// Add helper function for range parsing
std::pair<double, double> parseRange(const std::string& rangeStr) {
    size_t commaPos = rangeStr.find(',');
    if (commaPos == std::string::npos) {
        throw std::runtime_error("Invalid range format: " + rangeStr);
    }
    
    double min = std::stod(rangeStr.substr(0, commaPos));
    double max = std::stod(rangeStr.substr(commaPos + 1));
    
    return std::make_pair(min, max);
}

// Add helper function for vector parsing
std::vector<double> parseVector(const std::string& vecStr) {
    std::vector<double> result;
    std::stringstream ss(vecStr);
    std::string item;
    
    while (std::getline(ss, item, ',')) {
        // Trim whitespace
        item.erase(0, item.find_first_not_of(" \t"));
        item.erase(item.find_last_not_of(" \t") + 1);
        
        if (!item.empty()) {
            result.push_back(std::stod(item));
        }
    }
    
    return result;
}

// Helper to parse ROOT marker style names to int
inline int parseMarkerStyle(const std::string& style) {
    if (style == "kFullCircle") return 20;
    if (style == "kFullSquare") return 21;
    if (style == "kFullTriangleUp") return 22;
    if (style == "kFullTriangleDown") return 23;
    if (style == "kOpenCircle") return 24;
    if (style == "kOpenSquare") return 25;
    if (style == "kOpenTriangleUp") return 26;
    if (style == "kOpenDiamond") return 27;
    // Add more mappings as needed
    try { return std::stoi(style); } catch (...) { return 20; }
}

// ================= Multi-dimensional Cut Flow Tracker =====================
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <TEnv.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>

struct CutInfo {
    std::string name;
    std::string description;
    int passedIndividual;
    int passedSequential;
    bool isActive;
    CutInfo(const std::string& n, const std::string& desc, bool active = true)
        : name(n), description(desc), passedIndividual(0), passedSequential(0), isActive(active) {}
};

struct DimensionInfo {
    std::string name;
    std::vector<std::string> bins;
    DimensionInfo(const std::string& n, const std::vector<std::string>& b) : name(n), bins(b) {}
};

struct CutDimension {
    std::vector<CutInfo> cuts;
    int totalEvents;
    int currentSequentialPassed;
    CutDimension() : totalEvents(0), currentSequentialPassed(0) {}
};

class MultiDimCutFlowTracker {
public:
    MultiDimCutFlowTracker(TEnv* config, const std::vector<float>& centralityBins, const std::vector<std::string>& jetCollections, bool useCentrality) {
        isMC = (std::string(config->GetValue("DataType", "Data")) == "MC");
        
        // Set up centrality dimensions based on whether centrality bins are specified
        std::vector<std::string> centBins;
        if (useCentrality && centralityBins.size() >= 2) {
            // Create centrality bins - convert HiBin values (0-200) to percent (divide by 2)
            log(LOG_TRACE, "Creating centrality bins from HiBin values (converted to percent):");
            for (size_t i = 0; i < centralityBins.size() - 1; ++i) {
                int lowPercent = static_cast<int>(centralityBins[i] / 2.0);
                int highPercent = static_cast<int>(centralityBins[i+1] / 2.0);
                std::string binName = "cent" + std::to_string(lowPercent) + "to" + std::to_string(highPercent);
                centBins.push_back(binName);
                log(LOG_TRACE, "  Created bin: " + binName + " (from HiBin " + std::to_string(static_cast<int>(centralityBins[i])) + 
                    " to " + std::to_string(static_cast<int>(centralityBins[i+1])) + ")");
            }
        } else {
            // Create single inclusive bin when centrality bins are not specified
            centBins.push_back("inclusive");
            log(LOG_TRACE, "Created single inclusive bin (no centrality bins specified or useCentrality=false)");
        }
        
        dimensions.emplace_back("centrality", centBins);
        dimensions.emplace_back("jetCollection", jetCollections);
        log(LOG_TRACE, "MultiDimCutFlowTracker initialized with " + std::to_string(centBins.size()) + 
            " centrality bins and " + std::to_string(jetCollections.size()) + " jet collections");
        
        defineCutLevels();
        initializeCuts(config);
    }

    void initializeCuts(TEnv* config) {
        initializeCutList(config, globalCuts.cuts, "global");
        for (const auto& centBin : dimensions[0].bins) {
            for (const auto& collection : dimensions[1].bins) {
                CutDimension& cutDim = dimensionalCuts[centBin][collection];
                if (collection == dimensions[1].bins[0]) {
                    initializeCutList(config, dimensionalCuts[centBin]["centrality"].cuts, "centrality");
                }
                initializeCutList(config, cutDim.cuts, "collection");
            }
        }
    }

    void initializeCutList(TEnv* config, std::vector<CutInfo>& cuts, const std::string& level) {
        cuts.clear();
        if (level == "global") {
            cuts.emplace_back("RawEvents", "All input events", true);
            float vzCut = config->GetValue("VzCut", -999.0);
            if (vzCut > -900.0) cuts.emplace_back("VertexCut", "Vertex |z| < " + std::to_string(vzCut) + " cm", true);
        } else if (level == "centrality") {
            cuts.emplace_back("CentralityCut", "Events in this centrality bin", true);
            float photonEtMin = config->GetValue("PhotonEtMin", -999.0);
            if (photonEtMin > -900.0) cuts.emplace_back("PhotonKinematics", "Photon ET > " + std::to_string(photonEtMin) + " GeV", true);
            if (isMC && config->GetValue("MCPhotonMatchRequired", 0)) cuts.emplace_back("MCPhotonMatch", "MC truth photon matching", true);
            float photonEtaMax = config->GetValue("PhotonEtaMax", -999.0);
            if (photonEtaMax > -900.0) cuts.emplace_back("PhotonEta", "Photon |η| < " + std::to_string(photonEtaMax), true);
            float photonHoverEMax = config->GetValue("PhotonHoverEMax", -999.0);
            if (photonHoverEMax > -900.0) cuts.emplace_back("PhotonHoverE", "Photon H/E < " + std::to_string(photonHoverEMax), true);
            float photonSigmaMax = config->GetValue("PhotonSigmaIEtaIEtaMax", -999.0);
            if (photonSigmaMax > -900.0) cuts.emplace_back("PhotonSigmaIEtaIEta", "Photon σ_iηiη < " + std::to_string(photonSigmaMax), true);
            float photonIsoMax = config->GetValue("PhotonIsoMax", -999.0);
            if (photonIsoMax > -900.0) cuts.emplace_back("PhotonIsolation", "Photon Iso < " + std::to_string(photonIsoMax), true);
            float photonR9Min = config->GetValue("PhotonR9Min", -999.0);
            if (photonR9Min > -900.0) cuts.emplace_back("PhotonR9", "Photon R9 > " + std::to_string(photonR9Min), true);            
        } else if (level == "collection") {
            float jetPtMin = config->GetValue("JetPtMin", -999.0);
            if (jetPtMin > -900.0) cuts.emplace_back("JetKinematics", "Jet pT > " + std::to_string(jetPtMin) + " GeV", true);
            float deltaPhiMin = config->GetValue("DeltaPhiMin", -999.0);
            if (deltaPhiMin > -900.0) cuts.emplace_back("DeltaPhi", "Δφ(γ,jet) > " + std::to_string(deltaPhiMin) + " rad", true);
            float xjMin = config->GetValue("XjMin", -999.0);
            if (xjMin > -900.0) cuts.emplace_back("XjCut", "xj > " + std::to_string(xjMin), true);
            cuts.emplace_back("JetSelection", "Found jet passing all cuts", true);
            cuts.emplace_back("FinalSelection", "All cuts passed", true);
        }
    }

    void startEvent() {
        globalCuts.totalEvents++;
        globalCuts.currentSequentialPassed = 0;
    }

    void startCentralityBin(const std::string& centBin) {
        std::string resolvedBin = resolveCentralityBin(centBin);
        log(LOG_TRACE, "startCentralityBin: '" + centBin + "' resolved to '" + resolvedBin + "'");
        updateCutDimension(resolvedBin, "centrality");
    }

    void startJetCollection(const std::string& centBin, const std::string& collection) {
        std::string resolvedBin = resolveCentralityBin(centBin);
        log(LOG_TRACE, "startJetCollection: '" + centBin + "' resolved to '" + resolvedBin + "' for collection '" + collection + "'");
        updateCutDimension(resolvedBin, collection);
    }

    void applyCut(const std::string& cutName, bool passed, const std::string& centBin = "", const std::string& collection = "") {
        auto levelIt = cutToLevel.find(cutName);
        if (levelIt == cutToLevel.end()) {
            log(LOG_WARNING, "Unknown cut: " + cutName);
            return;
        }
        const std::string& level = levelIt->second;
        
        if (level == "global") {
            log(LOG_TRACE, "Applying global cut '" + cutName + "' (passed=" + std::to_string(passed) + ")");
            applyGlobalCut(cutName, passed, globalCuts);
        } else if (level == "centrality") {
            std::string resolvedBin = resolveCentralityBin(centBin);
            if (resolvedBin.empty()) {
                log(LOG_DEBUG, "Centrality bin not specified for centrality-level cut: " + cutName);
                return;
            }
            log(LOG_TRACE, "Applying centrality cut '" + cutName + "' to bin '" + resolvedBin + "' (passed=" + std::to_string(passed) + ")");
            applyGlobalCut(cutName, passed, dimensionalCuts[resolvedBin]["centrality"]);
        } else if (level == "collection") {
            std::string resolvedBin = resolveCentralityBin(centBin);
            if (resolvedBin.empty() || collection.empty()) {
                log(LOG_DEBUG, "Centrality bin or collection not specified for collection-level cut: " + cutName);
                return;
            }
            log(LOG_TRACE, "Applying collection cut '" + cutName + "' to bin '" + resolvedBin + "' collection '" + collection + "' (passed=" + std::to_string(passed) + ")");
            applyGlobalCut(cutName, passed, dimensionalCuts[resolvedBin][collection]);
        }
    }

    void printCutFlow() const {
        printCutFlowForDimension("Global Cuts", globalCuts);
        for (const auto& centBin : dimensions[0].bins) {
            printCutFlowForDimension("Centrality Bin: " + centBin, dimensionalCuts.at(centBin).at("centrality"));
            for (const auto& collection : dimensions[1].bins) {
                if (collection != "centrality") {
                    printCutFlowForDimension("Centrality: " + centBin + ", Collection: " + collection, dimensionalCuts.at(centBin).at(collection));
                }
            }
        }
    }

    void printCutFlowForDimension(const std::string& title, const CutDimension& cutDim) const {
        log(LOG_DEBUG, "");
        log(LOG_DEBUG, "=== " + title + " ===");
        log(LOG_DEBUG, "Total events processed: " + std::to_string(cutDim.totalEvents));
        log(LOG_DEBUG, "");
        log(LOG_DEBUG, std::string(100, '-'));
        log(LOG_DEBUG, "Cut Name              | Description                    | Individual        | Sequential        | Cut-to-Cut");
        log(LOG_DEBUG, "                      |                                | Count    (%)      | Count    (%)      | Efficiency (%)");
        log(LOG_DEBUG, std::string(100, '-'));
        for (size_t i = 0; i < cutDim.cuts.size(); ++i) {
            const auto& cut = cutDim.cuts[i];
            if (!cut.isActive) continue;
            double individualEff = cutDim.totalEvents > 0 ? 100.0 * cut.passedIndividual / cutDim.totalEvents : 0.0;
            double sequentialEff = 0.0;
            double sequentialPercentage = 0.0;
            if (i == 0) {
                sequentialEff = 100.0;
                sequentialPercentage = cutDim.totalEvents > 0 ? 100.0 * cut.passedSequential / cutDim.totalEvents : 0.0;
            } else if (cutDim.cuts[i-1].passedSequential > 0) {
                sequentialEff = 100.0 * cut.passedSequential / cutDim.cuts[i-1].passedSequential;
                sequentialPercentage = cutDim.totalEvents > 0 ? 100.0 * cut.passedSequential / cutDim.totalEvents : 0.0;
            }
            char buffer[250];
            snprintf(buffer, sizeof(buffer), "%-20s | %-30s | %7d (%5.1f%%) | %7d (%5.1f%%) | %6.2f%%",
                    cut.name.c_str(), 
                    cut.description.substr(0, 30).c_str(),
                    cut.passedIndividual, individualEff,
                    cut.passedSequential, sequentialPercentage,
                    sequentialEff);
            log(LOG_DEBUG, std::string(buffer));
        }
        log(LOG_DEBUG, std::string(100, '-'));
        if (cutDim.cuts.size() > 1 && cutDim.totalEvents > 0) {
            double overallEff = 100.0 * cutDim.cuts.back().passedSequential / cutDim.totalEvents;
            log(LOG_DEBUG, "Overall efficiency: " + std::to_string(cutDim.cuts.back().passedSequential) + 
                         "/" + std::to_string(cutDim.totalEvents) + " = " + 
                         std::to_string(overallEff) + "%");
        }
        log(LOG_DEBUG, "");
    }

    void saveCutFlowToFile(TFile* outFile) const {
        if (!outFile) return;
        outFile->cd();
        outFile->mkdir("cutFlow");
        outFile->cd("cutFlow");
        saveDimensionToFile(outFile, "global", globalCuts);
        for (const auto& centBin : dimensions[0].bins) {
            saveDimensionToFile(outFile, "centrality_" + centBin, dimensionalCuts.at(centBin).at("centrality"));
            for (const auto& collection : dimensions[1].bins) {
                if (collection != "centrality") {
                    saveDimensionToFile(outFile, "collection_" + centBin + "_" + collection, 
                                     dimensionalCuts.at(centBin).at(collection));
                }
            }
        }
        TTree* dimensionTree = new TTree("dimensionTree", "Dimension Information");
        std::string dimName, binName;
        int dimIndex, binIndex;
        dimensionTree->Branch("dimensionIndex", &dimIndex);
        dimensionTree->Branch("dimensionName", &dimName);
        dimensionTree->Branch("binIndex", &binIndex);
        dimensionTree->Branch("binName", &binName);
        for (size_t d = 0; d < dimensions.size(); ++d) {
            dimName = dimensions[d].name;
            dimIndex = d;
            for (size_t b = 0; b < dimensions[d].bins.size(); ++b) {
                binName = dimensions[d].bins[b];
                binIndex = b;
                dimensionTree->Fill();
            }
        }
        dimensionTree->Write();
        log(LOG_INFO, "Multi-dimensional cut flow information saved to output file");
        outFile->cd();
    }

    void saveDimensionToFile(TFile* outFile, const std::string& prefix, const CutDimension& cutDim) const {
        if (!outFile) return;
        outFile->cd();
        outFile->cd("cutFlow");
        TH1D* hCutFlow = new TH1D(("hCutFlow_" + prefix).c_str(), 
                                 ("Cut Flow for " + prefix + ";Cut Stage;Events").c_str(), 
                                 cutDim.cuts.size(), 0, cutDim.cuts.size());
        TH1D* hCutFlowEfficiency = new TH1D(("hCutFlowEfficiency_" + prefix).c_str(), 
                                          ("Cut Flow Efficiency for " + prefix + ";Cut Stage;Efficiency (%)").c_str(), 
                                          cutDim.cuts.size(), 0, cutDim.cuts.size());
        for (size_t i = 0; i < cutDim.cuts.size(); ++i) {
            if (!cutDim.cuts[i].isActive) continue;
            hCutFlow->SetBinContent(i + 1, cutDim.cuts[i].passedSequential);
            hCutFlow->GetXaxis()->SetBinLabel(i + 1, cutDim.cuts[i].name.c_str());
            double efficiency = (i == 0) ? 100.0 : 
                               (cutDim.cuts[i-1].passedSequential > 0 ? 
                                100.0 * cutDim.cuts[i].passedSequential / cutDim.cuts[i-1].passedSequential : 0.0);
            hCutFlowEfficiency->SetBinContent(i + 1, efficiency);
            hCutFlowEfficiency->GetXaxis()->SetBinLabel(i + 1, cutDim.cuts[i].name.c_str());
        }
        hCutFlow->Write();
        hCutFlowEfficiency->Write();
        TTree* cutFlowTree = new TTree(("cutFlowTree_" + prefix).c_str(), ("Cut Flow Information for " + prefix).c_str());
        std::string cutName, cutDescription;
        int passedIndividual, passedSequential, totalProcessed;
        double individualEff, sequentialEff;
        cutFlowTree->Branch("cutName", &cutName);
        cutFlowTree->Branch("cutDescription", &cutDescription); 
        cutFlowTree->Branch("passedIndividual", &passedIndividual);
        cutFlowTree->Branch("passedSequential", &passedSequential);
        cutFlowTree->Branch("totalProcessed", &totalProcessed);
        cutFlowTree->Branch("individualEfficiency", &individualEff);
        cutFlowTree->Branch("sequentialEfficiency", &sequentialEff);
        totalProcessed = cutDim.totalEvents;
        for (size_t i = 0; i < cutDim.cuts.size(); ++i) {
            const auto& cut = cutDim.cuts[i];
            if (!cut.isActive) continue;
            cutName = cut.name;
            cutDescription = cut.description;
            passedIndividual = cut.passedIndividual;
            passedSequential = cut.passedSequential;
            individualEff = cutDim.totalEvents > 0 ? 100.0 * cut.passedIndividual / cutDim.totalEvents : 0.0;
            sequentialEff = (i == 0) ? 100.0 : 
                           (cutDim.cuts[i-1].passedSequential > 0 ? 
                            100.0 * cut.passedSequential / cutDim.cuts[i-1].passedSequential : 0.0);
            cutFlowTree->Fill();
        }
        cutFlowTree->Write();
    }

    // std::string getCentralityBin(int hiBin, const std::vector<float>& centralityBinsValues) const {
    //     for (size_t i = 0; i < centralityBinsValues.size() - 1; ++i) {
    //         if (hiBin >= centralityBinsValues[i] && hiBin < centralityBinsValues[i+1]) {
    //             return "cent" + std::to_string(int(centralityBinsValues[i])) + 
    //                    "to" + std::to_string(int(centralityBinsValues[i+1]));
    //         }
    //     }
    //     return "";
    // }

private:
    // Helper to resolve centrality bin name (handles pp and PbPb)
    std::string resolveCentralityBin(const std::string& centBin) const {
        // If the bin exists in our dimensions, return as is
        if (std::find(dimensions[0].bins.begin(), dimensions[0].bins.end(), centBin) != dimensions[0].bins.end()) {
            log(LOG_TRACE, "resolveCentralityBin: '" + centBin + "' found in dimensions");
            return centBin;
        }
        
        // If we only have one bin and it's "inclusive", use that (for pp or when no centrality bins specified)
        if (dimensions[0].bins.size() == 1 && dimensions[0].bins[0] == "inclusive") {
            log(LOG_TRACE, "resolveCentralityBin: '" + centBin + "' resolved to 'inclusive' (single bin system)");
            return "inclusive";
        }
        
        // If empty string provided, return first available bin
        if (centBin.empty() && !dimensions[0].bins.empty()) {
            log(LOG_TRACE, "resolveCentralityBin: empty bin resolved to first available '" + dimensions[0].bins[0] + "'");
            return dimensions[0].bins[0];
        }
        
        // Otherwise, warn and return first bin as fallback
        if (!dimensions[0].bins.empty()) {
            log(LOG_WARNING, "resolveCentralityBin: Unknown centrality bin '" + centBin + "', defaulting to '" + dimensions[0].bins[0] + "'");
            return dimensions[0].bins[0];
        }
        
        log(LOG_ERROR, "resolveCentralityBin: No centrality bins available!");
        return "";
    }

    // Helper to update CutDimension for a given bin/collection
    void updateCutDimension(const std::string& centBin, const std::string& collection, bool resetSequential = true) {
        if (collection == "centrality") {
            // Centrality-level update
            for (auto& kv : dimensionalCuts[centBin]) {
                if (kv.first == "centrality") {
                    kv.second.totalEvents++;
                    if (resetSequential) kv.second.currentSequentialPassed = 0;
                    log(LOG_TRACE, "updateCutDimension: Updated centrality bin '" + centBin + "' (total events: " + std::to_string(kv.second.totalEvents) + ")");
                    break;
                }
            }
        } else {
            // Collection-level update
            dimensionalCuts[centBin][collection].totalEvents++;
            if (resetSequential) dimensionalCuts[centBin][collection].currentSequentialPassed = 0;
            log(LOG_TRACE, "updateCutDimension: Updated collection '" + collection + "' in bin '" + centBin + "' (total events: " + std::to_string(dimensionalCuts[centBin][collection].totalEvents) + ")");
        }
    }

    void defineCutLevels() {
        cutToLevel["RawEvents"] = "global";
        cutToLevel["VertexCut"] = "global";
        cutToLevel["CentralityCut"] = "centrality";
        cutToLevel["PhotonKinematics"] = "centrality";
        cutToLevel["PhotonEta"] = "centrality";
        cutToLevel["PhotonHoverE"] = "centrality";
        cutToLevel["PhotonSigmaIEtaIEta"] = "centrality";
        cutToLevel["PhotonIsolation"] = "centrality";
        cutToLevel["PhotonR9"] = "centrality";
        cutToLevel["MCPhotonMatch"] = "centrality";
        cutToLevel["JetKinematics"] = "collection";
        cutToLevel["DeltaPhi"] = "collection";
        cutToLevel["XjCut"] = "collection";
        cutToLevel["JetSelection"] = "collection";
        cutToLevel["FinalSelection"] = "collection";
    }
    void applyGlobalCut(const std::string& cutName, bool passed, CutDimension& cutDim) {
        auto it = std::find_if(cutDim.cuts.begin(), cutDim.cuts.end(), 
                              [&cutName](const CutInfo& cut) { return cut.name == cutName; });
        if (it != cutDim.cuts.end() && it->isActive) {
            if (passed) it->passedIndividual++;
            size_t cutIndex = std::distance(cutDim.cuts.begin(), it);
            if (cutIndex == 0) {
                if (passed) {
                    it->passedSequential++;
                    cutDim.currentSequentialPassed = 1;
                }
            } else if (cutIndex == (size_t)cutDim.currentSequentialPassed && passed) {
                it->passedSequential++;
                cutDim.currentSequentialPassed++;
            }
        }
    }
    std::map<std::string, std::map<std::string, CutDimension>> dimensionalCuts;
    CutDimension globalCuts;
    std::vector<DimensionInfo> dimensions;
    std::map<std::string, std::string> cutToLevel;
    bool isMC;
};

// Utility: Parse comma-separated string into set<string>
inline std::set<std::string> parseStringSet(const std::string& str) {
    std::set<std::string> result;
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, ',')) {
        // Trim whitespace
        size_t start = item.find_first_not_of(" \t\n\r");
        size_t end = item.find_last_not_of(" \t\n\r");
        if (start != std::string::npos && end != std::string::npos)
            result.insert(item.substr(start, end - start + 1));
        else if (start != std::string::npos)
            result.insert(item.substr(start));
    }
    return result;
}

#endif
