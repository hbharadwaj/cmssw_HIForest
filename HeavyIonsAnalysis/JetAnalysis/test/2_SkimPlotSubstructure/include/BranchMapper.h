/**
 * BranchMapper.h
 * 
 * Loads and queries branch mapping configurations.
 * Maps logical/canonical branch names to physical branch names in ROOT files.
 * Handles format-specific differences (2018 vs 2023/2024, PbPb vs PP).
 * 
 * Usage:
 *   BranchMapper mapper("configs/BranchMap_2018_PbPb.config");
 *   std::string physical = mapper.getPhotonBranch("phoEt");  // returns "phoEtCorrected"
 *   std::string jetBranch = mapper.getJetBranch("AK2Z2", "jtpt");  // returns "jtpt" (2018) or "AK2Z2_jtpt" (2023)
 */

#ifndef BRANCHMAPPER_H
#define BRANCHMAPPER_H

#include <TEnv.h>
#include <string>
#include <iostream>
#include <stdexcept>

class BranchMapper {
public:
    /**
     * Load branch mapping from config file
     * @param configFile: Path to .config file (relative to configs/ or absolute)
     */
    BranchMapper(const std::string& configFile) {
        // Try loading config - check multiple possible paths
        std::string fullPath = configFile;
        
        // If already has configs/ or is absolute, use as-is
        if (configFile.find("configs/") != std::string::npos || configFile[0] == '/') {
            fullPath = configFile;
        } else {
            // Try ../configs/ first (when running from 2_SkimPlotSubstructure)
            fullPath = "../configs/" + configFile;
        }
        
        // Try loading
        int loadResult = config_.ReadFile(fullPath.c_str(), kEnvAll);
        
        // If failed and path didn't have configs/, try without ../
        if (loadResult != 0 && fullPath.find("../") != std::string::npos) {
            fullPath = configFile;
            if (configFile.find("configs/") == std::string::npos) {
                fullPath = "configs/" + configFile;
            }
            loadResult = config_.ReadFile(fullPath.c_str(), kEnvAll);
        }
        
        if (loadResult != 0) {
            throw std::runtime_error("BranchMapper: Failed to load config file: " + configFile + " (tried: " + fullPath + ")");
        }
        
        log("INFO", "Loaded branch mapping from: " + fullPath);
        
        // Parse format metadata
        year_ = config_.GetValue("Format.Year", 2023);
        
        std::string photonMode = config_.GetValue("Format.PhotonMode", "VECTOR");
        scalarPhotonMode_ = (photonMode == "SCALAR");
        
        std::string jetPrefixStr = config_.GetValue("Format.HasJetCollectionPrefix", "yes");
        hasJetPrefix_ = (jetPrefixStr == "yes" || jetPrefixStr == "true" || jetPrefixStr == "1");
        
        log("INFO", "  Year: " + std::to_string(year_));
        log("INFO", "  Photon mode: " + photonMode);
        log("INFO", "  Jet prefix string read: '" + jetPrefixStr + "'");
        log("INFO", "  Jet collection prefix: " + std::string(hasJetPrefix_ ? "yes" : "no"));
    }
    
    // Query methods for different branch categories
    std::string getEventLevelBranch(const std::string& logical) const {
        return resolveBranch("EventLevel", logical);
    }
    
    std::string getPhotonBranch(const std::string& logical) const {
        return resolveBranch("Photon", logical);
    }
    
    std::string getJetBranch(const std::string& collection, const std::string& logical) const {
        std::string physical = resolveBranch("Jet", logical);
        if (physical == "MISSING") return "MISSING";
        
        // If format has jet collection prefix and physical name starts with "_", prepend collection
        if (hasJetPrefix_ && physical.length() > 0 && physical[0] == '_') {
            return collection + physical;
        }
        return physical;
    }
    
    std::string getSubstructureBranch(const std::string& collection, const std::string& logical) const {
        std::string physical = resolveBranch("Substructure", logical);
        if (physical == "MISSING") return "MISSING";
        
        if (hasJetPrefix_ && physical.length() > 0 && physical[0] == '_') {
            return collection + physical;
        }
        return physical;
    }
    
    std::string getMCBranch(const std::string& collection, const std::string& logical) const {
        std::string physical = resolveBranch("MC", logical);
        if (physical == "MISSING") return "MISSING";
        
        // MC branches with collection prefix (e.g., AK2Z2_refpt)
        if (hasJetPrefix_ && physical.length() > 0 && physical[0] == '_') {
            return collection + physical;
        }
        return physical;
    }
    
    std::string getTriggerBranch(const std::string& logical) const {
        return resolveBranch("Trigger", logical);
    }
    
    // Status queries
    bool isMissing(const std::string& branchName) const {
        return branchName == "MISSING";
    }
    
    bool isScalarPhoton() const {
        return scalarPhotonMode_;
    }
    
    bool hasJetCollectionPrefix() const {
        return hasJetPrefix_;
    }
    
    int getYear() const {
        return year_;
    }
    
    // Get the configured number of photons (1 for scalar mode)
    int getPhotonCount() const {
        if (scalarPhotonMode_) {
            return config_.GetValue("Photon.nPho", 1);
        }
        return -1;  // Variable, determined at runtime
    }
    
private:
    TEnv config_;
    int year_;
    bool scalarPhotonMode_;
    bool hasJetPrefix_;
    
    std::string resolveBranch(const std::string& section, const std::string& key) const {
        std::string fullKey = section + "." + key;
        const char* value = config_.GetValue(fullKey.c_str(), "MISSING");
        return std::string(value);
    }
    
    void log(const std::string& level, const std::string& message) const {
        std::cout << "[" << level << "] BranchMapper: " << message << std::endl;
    }
};

#endif // BRANCHMAPPER_H
