#ifndef UNFOLD_CONFIG_H
#define UNFOLD_CONFIG_H

#include "UnfoldUtils.h"
#include <string>
#include <vector>
#include <set>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <TEnv.h>

// Enum for test types
enum class UnfoldTestType { Nominal, Closure, Split, Bottomline };

// Struct for per-set test configuration
struct TestConfig {
    std::vector<UnfoldTestType> tests;
    double splitFraction = 0.5; // Default for Split test
};


// Utility: Parse test types from config string
inline std::vector<UnfoldTestType> parseTestTypes(const std::string& testStr, double& splitFraction) {
    std::vector<UnfoldTestType> result;
    splitFraction = 0.5;
    std::istringstream ss(testStr);
    std::string token;
    while (std::getline(ss, token, ',')) {
        // Remove whitespace
        token.erase(remove_if(token.begin(), token.end(), ::isspace), token.end());
        if (token.empty()) continue;
        if (token.find("Split:") == 0) {
            result.push_back(UnfoldTestType::Split);
            try {
                splitFraction = std::stod(token.substr(6));
            } catch (...) { splitFraction = 0.5; }
        } else if (token == "Nominal") {
            result.push_back(UnfoldTestType::Nominal);
        } else if (token == "Closure") {
            result.push_back(UnfoldTestType::Closure);
        } else if (token == "Bottomline") {
            result.push_back(UnfoldTestType::Bottomline);
        }
    }
    return result;
}

// ============================================================================
// UNIFIED CONFIGURATION MANAGEMENT
// ============================================================================

struct UnfoldConfig {
    std::string name;
    int dimension;
    std::vector<std::string> measuredVars;
    std::vector<std::string> truthVars;
    std::vector<std::vector<double>> measuredBins;
    std::vector<std::vector<double>> truthBins;
    std::string method = "MatrixInversion";
    int iterations = 4;
    std::string dataFile;
    std::string mcFile;
    std::string weightBranch = "eventWeight";
    
    static UnfoldConfig fromTEnv(TEnv* config, const std::string& setName) {
        UnfoldConfig cfg;
        cfg.name = setName;
        std::string prefix = setName + ".";
        
        cfg.dimension = config->GetValue((prefix+"UnfoldingDimension").c_str(), 1);
        cfg.measuredVars = splitCSV(config->GetValue((prefix+"UnfoldVariables").c_str(), ""));
        cfg.truthVars = splitCSV(config->GetValue((prefix+"TruthVariables").c_str(), ""));
        
        // Parse bins for each variable
        for (const auto& var : cfg.measuredVars) {
            std::string binStr = config->GetValue((prefix+var+"Bins").c_str(), "");
            cfg.measuredBins.push_back(parseBins(binStr));
        }
        for (const auto& var : cfg.truthVars) {
            std::string binStr = config->GetValue((prefix+var+"Bins").c_str(), "");
            cfg.truthBins.push_back(parseBins(binStr));
        }
        
        cfg.method = config->GetValue((prefix+"UnfoldingMethod").c_str(), "MatrixInversion");
        cfg.iterations = config->GetValue((prefix+"UnfoldingIterations").c_str(), 4);
        cfg.dataFile = config->GetValue((prefix+"DataInputFile").c_str(), 
                                      config->GetValue("default.DataInputFile", ""));
        cfg.mcFile = config->GetValue((prefix+"MCInputFile").c_str(),
                                    config->GetValue("default.MCInputFile", ""));
        cfg.weightBranch = config->GetValue((prefix+"eventWeightBranch").c_str(), "eventWeight");
        
        return cfg;
    }
    
    bool isValid() const {
        return dimension > 0 && !measuredVars.empty() && !truthVars.empty() && 
               measuredBins.size() == measuredVars.size() && 
               truthBins.size() == truthVars.size();
    }
    
    void print() const {
        log(LOG_DEBUG, "Configuration for " + name + ":");
        log(LOG_DEBUG, "  Dimension: " + std::to_string(dimension));
        log(LOG_DEBUG, "  Method: " + method + " (iterations: " + std::to_string(iterations) + ")");
        log(LOG_DEBUG, "  Measured variables: " + std::to_string(measuredVars.size()));
        log(LOG_DEBUG, "  Truth variables: " + std::to_string(truthVars.size()));
    }
};

// ============================================================================
// ENHANCED CONFIGURATION PARSER
// ============================================================================

class ConfigParser {
private:
    std::unique_ptr<TEnv> config;
    std::string configFilePath;
    
public:
    explicit ConfigParser(const std::string& configFile) 
        : configFilePath(configFile), config(std::make_unique<TEnv>(configFile.c_str())) {
        
        if (!config) {
            throw std::runtime_error("Failed to load config file: " + configFile);
        }
        
        log(LOG_INFO, "ConfigParser initialized with: " + configFile);
    }
    
    // Get list of all unfolding sets defined in the config file
    std::vector<std::string> getUnfoldingSets() {
        std::vector<std::string> sets;
        std::set<std::string> found;
        log(LOG_DEBUG, "Parsing unfolding sets from configuration...");
        
        // Read the file line by line to find unfolding set definitions
        std::ifstream infile(configFilePath);
        if (!infile.is_open()) {
            log(LOG_ERROR, "Failed to open config file for parsing: " + configFilePath);
            return sets;
        }
        
        std::string line;
        while (std::getline(infile, line)) {
            // Skip comments and empty lines
            if (line.empty() || line[0] == '#') continue;
            
            // Look for lines with "UnfoldingDimension" - these define unfolding sets
            size_t dimPos = line.find(".UnfoldingDimension");
            if (dimPos != std::string::npos && line.substr(0, 6) == "Unfold") {
                std::string prefix = line.substr(0, dimPos);
                if (found.insert(prefix).second) {
                    sets.push_back(prefix);
                    log(LOG_DEBUG, "Found unfolding set: " + prefix);
                }
            }
        }
        
        log(LOG_DEBUG, "Found " + std::to_string(sets.size()) + " unfolding sets");
        return sets;
    }
    
    // Parse configuration for a specific unfolding set
    UnfoldConfig parseUnfoldingSet(const std::string& setName) {
        log(LOG_DEBUG, "Parsing configuration for set: " + setName);
        
        UnfoldConfig cfg = UnfoldConfig::fromTEnv(config.get(), setName);
        
        if (!cfg.isValid()) {
            log(LOG_WARNING, "Invalid configuration parsed for set: " + setName);
        } else {
            log(LOG_DEBUG, "Successfully parsed " + std::to_string(cfg.dimension) + "D configuration for: " + setName);
        }
        
        return cfg;
    }
    
    // Get global configuration values
    std::string getOutputDir() const {
        return config->GetValue("default.OutputDir", "./");
    }
    
    std::string getOutputPrefix() const {
        return config->GetValue("default.OutputPrefix", "roounfold");
    }
    
    int getVerbosity() const {
        return config->GetValue("default.Verbosity", 2);
    }
    
    // Validate that the config file has required sections
    bool validateConfiguration() {
        auto sets = getUnfoldingSets();
        if (sets.empty()) {
            log(LOG_ERROR, "No unfolding sets found in configuration file");
            return false;
        }
        
        // Validate each unfolding set
        bool allValid = true;
        for (const auto& setName : sets) {
            UnfoldConfig cfg = parseUnfoldingSet(setName);
            if (!cfg.isValid()) {
                log(LOG_ERROR, "Invalid configuration for unfolding set: " + setName);
                allValid = false;
            }
        }
        
        if (allValid) {
            log(LOG_DEBUG, "Configuration validation successful");
        } else {
            log(LOG_ERROR, "Configuration validation failed");
        }
        
        return allValid;
    }
    
    // Get access to underlying TEnv if needed for advanced operations
    TEnv* getTEnv() const {
        return config.get();
    }
    
    // Legacy static method for backward compatibility
    static std::vector<std::string> getUnfoldSetNames(TEnv* config) {
        std::vector<std::string> sets;
        std::set<std::string> found;
        log(LOG_DEBUG, "Searching for unfolding sets in config...");
        
        // Get the config file path
        std::string configFile = config->GetValue("InputConfigFile", "../configs/UnfoldJetSub_xj_test.config");
        
        // Read the file line by line directly
        log(LOG_DEBUG, "Reading config file: " + configFile);
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
                    log(LOG_DEBUG, "Found unfolding set: " + prefix);
                }
            }
        }
        
        log(LOG_DEBUG, "Found " + std::to_string(sets.size()) + " unfolding sets to process.");
        for (const auto& set : sets) {
            log(LOG_DEBUG, "  - " + set);
        }
        
        return sets;
    }
    
    TestConfig getTestConfig(const std::string& setName) const {
        double split = 0.5;
        std::string key = setName + ".Tests";
        std::string testStr;
        if (config->Defined(key.c_str())) {
            testStr = config->GetValue(key.c_str(), "");
        } else {
            testStr = config->GetValue("default.Tests", "Nominal");
        }
        TestConfig tcfg;
        tcfg.tests = parseTestTypes(testStr, split);
        tcfg.splitFraction = split;
        return tcfg;
    }
    
    std::string getConfigValue(const std::string& key, const std::string& def = "") const {
        return config->GetValue(key.c_str(), def.c_str());
    }
};

#endif // UNFOLD_CONFIG_H
