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
int g_verbosity = 1; // Global verbosity level
extern struct PlottingConfiguration g_plotConfig;

// Structs
struct PlottingConfiguration;
struct HistogramConfig;

// Prints usage information for the program.
void printUsage();

// Sets up a ROOT TChain with input files from a directory or file.
bool setupInputChain(TChain* chain, const std::string& inputDir, bool testMode, int maxFiles = 1);

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
TH1F* createHistogram1D(const HistogramConfig& config, const std::string& name = "", const std::string& title = "", int colorIndex = -1);

// Creates a 2D histogram with configuration options.
TH2F* createHistogram2D(const HistogramConfig& config, const std::string& name = "", const std::string& title = "", int colorIndex = -1);

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

// =====================================================
// ================ Beginning of the code ==============
// =====================================================

// Logging system
enum LogLevel { LOG_ERROR = 0, LOG_INFO = 1, LOG_DEBUG = 2, LOG_TRACE = 3 };

// Logging function with verbosity levels and colors
void log(LogLevel level, const std::string& message) {
    if (static_cast<int>(level) > g_verbosity) return;
    
    const char* colorCode = RESET_COLOR;
    const char* levelStr = "";
    
    switch (level) {
        case LOG_ERROR:   colorCode = RED_COLOR;     levelStr = "ERROR"; break;
        case LOG_INFO:    colorCode = GREEN_COLOR;   levelStr = "INFO";  break;
        case LOG_DEBUG:   colorCode = YELLOW_COLOR;  levelStr = "DEBUG"; break;
        case LOG_TRACE:   colorCode = CYAN_COLOR;    levelStr = "TRACE"; break;
    }
    
    std::cout << colorCode << "[" << levelStr << "] " << message << RESET_COLOR << std::endl;
}

/**
 * Setup the input chain with files from the input directory
*/
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
    
    // Binning for Y axis (for 2D histograms)
    int nBinsY;
    double yMin;
    double yMax;
    
    // Display properties
    int color;
    int marker;
    int markerStyle; // NEW: Marker style (ROOT enum or int)
    double markerSize; // NEW: Marker size
    int lineStyle;
    double lineWidth;
    std::string drawOption;
    
    HistogramConfig() : 
        name(""), title(""), type("TH1F"),
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
        hcfg.type = merged.count("Type") ? merged["Type"] : "TH1F";
        hcfg.xTitle = merged.count("XTitle") ? merged["XTitle"] : "";
        hcfg.yTitle = merged.count("YTitle") ? merged["YTitle"] : "";
        hcfg.zTitle = merged.count("ZTitle") ? merged["ZTitle"] : "";
        try { hcfg.nBinsX = merged.count("Bins") ? std::stoi(merged["Bins"]) : 50; }
        catch (const std::exception& e) { log(LOG_ERROR, "Failed to parse Bins for " + histName + ": '" + merged["Bins"] + "' (" + e.what() + "). Using default 50."); hcfg.nBinsX = 50; }
        try { hcfg.xMin = merged.count("XMin") ? std::stod(merged["XMin"]) : 0.0; }
        catch (const std::exception& e) { log(LOG_ERROR, "Failed to parse XMin for " + histName + ": '" + merged["XMin"] + "' (" + e.what() + "). Using default 0.0."); hcfg.xMin = 0.0; }
        try { hcfg.xMax = merged.count("XMax") ? std::stod(merged["XMax"]) : 100.0; }
        catch (const std::exception& e) { log(LOG_ERROR, "Failed to parse XMax for " + histName + ": '" + merged["XMax"] + "' (" + e.what() + "). Using default 100.0."); hcfg.xMax = 100.0; }
        try { hcfg.nBinsY = merged.count("YBins") ? std::stoi(merged["YBins"]) : 50; }
        catch (const std::exception& e) { log(LOG_ERROR, "Failed to parse YBins for " + histName + ": '" + merged["YBins"] + "' (" + e.what() + "). Using default 50."); hcfg.nBinsY = 50; }
        try { hcfg.yMin = merged.count("YMin") ? std::stod(merged["YMin"]) : 0.0; }
        catch (const std::exception& e) { log(LOG_ERROR, "Failed to parse YMin for " + histName + ": '" + merged["YMin"] + "' (" + e.what() + "). Using default 0.0."); hcfg.yMin = 0.0; }
        try { hcfg.yMax = merged.count("YMax") ? std::stod(merged["YMax"]) : 100.0; }
        catch (const std::exception& e) { log(LOG_ERROR, "Failed to parse YMax for " + histName + ": '" + merged["YMax"] + "' (" + e.what() + "). Using default 100.0."); hcfg.yMax = 100.0; }
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
    std::cout << "  --test, -t [N]             Run in test mode with N events (default: 1000)" << std::endl;
    std::cout << "  --production               Run in production mode (all events)" << std::endl;
    std::cout << "  --help, -h                 Print this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  ./gammaJetAnalyzer -c analysis.config -p plotting.config -t 10000" << std::endl;
    std::cout << "  ./gammaJetAnalyzer --config analysis.config --plot-config plotting.config --test 5000" << std::endl;
    std::cout << "  ./gammaJetAnalyzer -c analysis.config -p plotting.config --production" << std::endl;
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
TH1F* createHistogram1D(const HistogramConfig& config, const std::string& name, 
                       const std::string& title, int colorIndex) {
    // Use name and title from config if not provided
    std::string histName = name.empty() ? config.name : name;
    std::string histTitle = title.empty() ? config.title : title;
    
    // Add axis titles if available
    if (!config.xTitle.empty() || !config.yTitle.empty()) {
        histTitle += ";" + config.xTitle + ";" + config.yTitle;
    }
    
    // Create histogram with the specified binning
    TH1F* hist = new TH1F(histName.c_str(), histTitle.c_str(), 
                          config.nBinsX, config.xMin, config.xMax);
    
    // Apply styling with the provided color index or default
    applyHistogramStyle(hist, config, colorIndex >= 0 ? colorIndex : 0);
    
    return hist;
}

/**
 * Enhanced 2D histogram creation with configuration
 */
TH2F* createHistogram2D(const HistogramConfig& config, const std::string& name, 
                       const std::string& title, int colorIndex) {
    std::string histName = name.empty() ? config.name : name;
    std::string histTitle = title.empty() ? config.title : title;
    
    // Add axis titles if available
    if (!config.xTitle.empty() || !config.yTitle.empty() || !config.zTitle.empty()) {
        histTitle += ";" + config.xTitle + ";" + config.yTitle + ";" + config.zTitle;
    }
    
    // Create histogram with the specified binning
    TH2F* hist = new TH2F(histName.c_str(), histTitle.c_str(), 
                         config.nBinsX, config.xMin, config.xMax,
                         config.nBinsY, config.yMin, config.yMax);
    
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

#endif
