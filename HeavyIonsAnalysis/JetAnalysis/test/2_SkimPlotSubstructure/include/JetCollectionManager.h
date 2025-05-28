/**
 * JetCollectionManager.h
 * 
 * Header-only simplified version of JetCollectionManager for photon-tagged jet analysis.
 * This class provides a unified interface to access jet data from multiple
 * jet collections defined in a configuration file.
 */

#ifndef JETCOLLECTIONMANAGER_H
#define JETCOLLECTIONMANAGER_H

#include <TTree.h>
#include <TEnv.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <regex>
#include <cstring> // For std::memset
#include <algorithm>
#include <memory> // For std::unique_ptr
#include "helpers.h" // Include for logging system

class JetCollectionManager {
public:
    // Enum for jet properties to replace individual getter functions
    enum JetProperty {
        PT, ETA, PHI, MASS, AREA, 
        DYN_SPLIT, DYN_KT, DYN_Z, 
        GIRTH, THRUST, LHA, PTD
    };

    /**
     * Constructor
     * @param config Pointer to configuration environment
     * @param tree Pointer to input TTree containing jet data
     */
    JetCollectionManager(TEnv* config, TTree* tree)
        : fConfig(config), fTree(tree), fVerbosity(0)
    {
        if (fConfig) {
            fVerbosity = fConfig->GetValue("Verbosity", 0);
            // Update global verbosity level for logging
            g_verbosity = fVerbosity;
        }
        log(LOG_DEBUG, "JetCollectionManager constructed with verbosity level: " + std::to_string(fVerbosity));
    }
    
    /**
     * Initialize jet collections from configuration
     * @return true if initialization successful, false otherwise
     */
    bool initialize()
    {
        if (!fConfig || !fTree) {
            log(LOG_ERROR, "Configuration or Tree not initialized.");
            return false;
        }
        
        // Get analysis cases from config
        std::string analysisCases = fConfig->GetValue("AnalysisCases", "");
        if (analysisCases.empty()) {
            log(LOG_ERROR, "No AnalysisCases defined in configuration.");
            return false;
        }
        
        log(LOG_DEBUG, "Initializing jet collections: " + analysisCases);
        
        // Split comma-separated list of jet collections
        std::stringstream ss(analysisCases);
        std::string collection;
        while (std::getline(ss, collection, ',')) {
            // Trim whitespace
            collection.erase(0, collection.find_first_not_of(" \t\n\r\f\v"));
            collection.erase(collection.find_last_not_of(" \t\n\r\f\v") + 1);
            
            if (collection.empty()) continue;
            
            // Parse collection name to validate format
            int radius, zCut;
            if (!parseCollection(collection, radius, zCut)) {
                log(LOG_ERROR, "Invalid collection format: " + collection + ". Skipping.");
                continue;
            }
            
            // Store collection and setup branch addresses directly
            fCollections.push_back(collection);
            if (!setupBranchAddresses(collection)) {
                log(LOG_ERROR, "Failed to setup branches for collection: " + collection);
            }
        }
        
        if (fCollections.empty()) {
            log(LOG_ERROR, "No valid jet collections initialized.");
            return false;
        }
        
        log(LOG_INFO, "Initialized " + std::to_string(fCollections.size()) + " jet collections:");
        for (const auto& coll : fCollections) {
            log(LOG_DEBUG, " - " + coll);
        }
        
        return true;
    }
    
    /**
     * Get number of jets in a collection for current event
     * @param collection Name of jet collection (e.g., AK2Z2)
     * @return Number of jets in the collection
     */
    int getNJets(const std::string& collection)
    {
        auto it = fJetBranches.find(collection);
        if (it == fJetBranches.end()) {
            return 0;
        }
        
        // Add bounds checking to prevent buffer overflow
        if (it->second.nJets > MAX_JETS) {
            log(LOG_ERROR, "Collection " + collection + " has " + std::to_string(it->second.nJets) + 
                " jets, exceeding MAX_JETS=" + std::to_string(MAX_JETS) + ". Clamping to MAX_JETS.");
            return MAX_JETS;
        }
        
        return it->second.nJets;
    }
    
    /**
     * Generic getter for jet properties - replaces all individual getters
     * @param collection Name of jet collection
     * @param property Jet property to retrieve
     * @param jetIndex Index of the jet in the collection
     * @return Property value (float for most, int for DYN_SPLIT)
     */
    float getJetProperty(const std::string& collection, JetProperty property, int jetIndex)
    {
        auto it = fJetBranches.find(collection);
        if (it == fJetBranches.end() || jetIndex >= it->second.nJets || jetIndex < 0) {
            return (property == DYN_SPLIT) ? -999 : -999.0;
        }
        
        // Add additional bounds checking against MAX_JETS to prevent buffer overflow
        if (it->second.nJets > MAX_JETS) {
            log(LOG_ERROR, "Collection " + collection + " has " + std::to_string(it->second.nJets) + 
                " jets, exceeding MAX_JETS=" + std::to_string(MAX_JETS) + ". Data may be corrupted!");
            return (property == DYN_SPLIT) ? -999 : -999.0;
        }
        
        const auto& branches = it->second;
        
        switch (property) {
            case PT:        return branches.pt.get()[jetIndex];
            case ETA:       return branches.eta.get()[jetIndex];
            case PHI:       return branches.phi.get()[jetIndex];
            case MASS:      return branches.mass.get()[jetIndex];
            case AREA:      return branches.area.get()[jetIndex];
            case DYN_SPLIT: return static_cast<float>(branches.dyn_split.get()[jetIndex]);
            case DYN_KT:    return branches.dyn_kt.get()[jetIndex];
            case DYN_Z:     return branches.dyn_z.get()[jetIndex];
            case GIRTH:     return branches.girth.get()[jetIndex];
            case THRUST:    return branches.thrust.get()[jetIndex];
            case LHA:       return branches.lha.get()[jetIndex];
            case PTD:       return branches.ptd.get()[jetIndex];
            default:        return -999.0;
        }
    }
    
    /**
     * Convenience functions that call getJetProperty internally
     */
    float getJetPt(const std::string& collection, int jetIndex) { 
        return getJetProperty(collection, PT, jetIndex); 
    }
    float getJetEta(const std::string& collection, int jetIndex) { 
        return getJetProperty(collection, ETA, jetIndex); 
    }
    float getJetPhi(const std::string& collection, int jetIndex) { 
        return getJetProperty(collection, PHI, jetIndex); 
    }
    float getJetMass(const std::string& collection, int jetIndex) { 
        return getJetProperty(collection, MASS, jetIndex); 
    }
    float getJetArea(const std::string& collection, int jetIndex) { 
        return getJetProperty(collection, AREA, jetIndex); 
    }
    int getJetDynSplit(const std::string& collection, int jetIndex) { 
        return (int)getJetProperty(collection, DYN_SPLIT, jetIndex); 
    }
    float getJetDynKt(const std::string& collection, int jetIndex) { 
        return getJetProperty(collection, DYN_KT, jetIndex); 
    }
    float getJetDynZ(const std::string& collection, int jetIndex) { 
        return getJetProperty(collection, DYN_Z, jetIndex); 
    }
    float getJetGirth(const std::string& collection, int jetIndex) { 
        return getJetProperty(collection, GIRTH, jetIndex); 
    }
    float getJetThrust(const std::string& collection, int jetIndex) { 
        return getJetProperty(collection, THRUST, jetIndex); 
    }
    float getJetLHA(const std::string& collection, int jetIndex) { 
        return getJetProperty(collection, LHA, jetIndex); 
    }
    float getJetPtD(const std::string& collection, int jetIndex) { 
        return getJetProperty(collection, PTD, jetIndex); 
    }
    
    /**
     * Get list of all available jet collections
     * @return Vector of collection names
     */
    std::vector<std::string> getCollections() const
    {
        return fCollections;
    }
    
    /**
     * Print branch mappings for debugging
     * @param verbose If true, print detailed branch information
     */
    void printBranchMappings(bool verbose = false) const
    {
        log(LOG_INFO, "=== Jet Collection Branch Mappings ===");
        for (const auto& collection : fCollections) {
            log(LOG_INFO, "Collection: " + collection);
            
            if (verbose) {
                const auto& branches = fJetBranches.at(collection);
                log(LOG_DEBUG, "  Current nJets: " + std::to_string(branches.nJets));
                if (branches.nJets > 0) {
                    log(LOG_DEBUG, "  First jet pt: " + std::to_string(branches.pt[0]));
                    log(LOG_DEBUG, "  First jet eta: " + std::to_string(branches.eta[0]));
                    log(LOG_DEBUG, "  First jet phi: " + std::to_string(branches.phi[0]));
                }
            }
        }
        log(LOG_INFO, "=====================================");
    }

private:
    /**
     * Parse collection name into jet algorithm and parameters
     * @param collection Collection name (e.g., AK2Z2)
     * @param radius Output parameter for jet radius (x10)
     * @param zCut Output parameter for z cut threshold (x10)
     * @return true if parsing successful, false otherwise
     */
    bool parseCollection(const std::string& collection, int& radius, int& zCut)
    {
        // Regex pattern for AKnZm format
        std::regex pattern("AK([0-9]+)Z([0-9]+)");
        std::smatch matches;
        
        if (std::regex_match(collection, matches, pattern) && matches.size() == 3) {
            radius = std::stoi(matches[1].str());
            zCut = std::stoi(matches[2].str());
            return true;
        }
        
        return false;
    }
    
    /**
     * Setup branch addresses for a specific collection using data-driven approach
     * @param collection Collection name
     * @return true if setup successful, false otherwise
     */
    bool setupBranchAddresses(const std::string& collection)
    {
        if (!fTree) return false;
        
        auto& branches = fJetBranches[collection];
        bool success = true;
        
        // Define branch suffixes and their corresponding member pointers
        struct BranchInfo {
            const char* suffix;
            void* ptr;
            bool required;
            const char* type;
        };
        
        BranchInfo branchDefs[] = {
            {"_nref", &branches.nJets, true, "int"},
            {"_jtpt", branches.pt.get(), true, "float[]"},
            {"_jteta", branches.eta.get(), true, "float[]"},
            {"_jtphi", branches.phi.get(), true, "float[]"},
            {"_jtm", branches.mass.get(), false, "float[]"},
            {"_jtarea", branches.area.get(), false, "float[]"},
            {"_jtdyn_split", branches.dyn_split.get(), false, "int[]"},
            {"_jtdyn_kt", branches.dyn_kt.get(), false, "float[]"},
            {"_jtdyn_z", branches.dyn_z.get(), false, "float[]"},
            {"_jt_girth", branches.girth.get(), false, "float[]"},
            {"_jt_thrust", branches.thrust.get(), false, "float[]"},
            {"_jt_LHA", branches.lha.get(), false, "float[]"},
            {"_jt_pTD", branches.ptd.get(), false, "float[]"}
        };
        
        // Setup all branches using loop
        for (const auto& branchDef : branchDefs) {
            std::string branchName = collection + branchDef.suffix;
            
            if (fTree->GetBranch(branchName.c_str())) {
                fTree->SetBranchAddress(branchName.c_str(), branchDef.ptr);
                log(LOG_TRACE, "  Connected branch: " + branchName + " (" + branchDef.type + ")");
            } else {
                if (branchDef.required) {
                    log(LOG_ERROR, "Required branch '" + branchName + "' not found.");
                    success = false;
                } else {
                    log(LOG_DEBUG, "Optional branch '" + branchName + "' not found.");
                }
            }
        }
        
        return success;
    }
    
    // Configuration and tree pointers
    TEnv* fConfig;
    TTree* fTree;
    
    // Verbosity level
    int fVerbosity;
    
    // Vector of configured jet collections
    std::vector<std::string> fCollections;
    
    // Branch addresses for each collection
    static const int MAX_JETS = 200; // Increased from 100 for safety
    struct JetBranches {
        int nJets;
        // Use heap-allocated arrays to avoid stack overflow
        std::unique_ptr<float[]> pt;
        std::unique_ptr<float[]> eta;
        std::unique_ptr<float[]> phi;
        std::unique_ptr<float[]> mass;
        std::unique_ptr<float[]> area;
        std::unique_ptr<int[]> dyn_split;
        std::unique_ptr<float[]> dyn_kt;
        std::unique_ptr<float[]> dyn_z;
        std::unique_ptr<float[]> girth;
        std::unique_ptr<float[]> thrust;
        std::unique_ptr<float[]> lha;
        std::unique_ptr<float[]> ptd;
        
        // Initialize arrays on heap
        JetBranches() : nJets(0) {
            pt = std::make_unique<float[]>(MAX_JETS);
            eta = std::make_unique<float[]>(MAX_JETS);
            phi = std::make_unique<float[]>(MAX_JETS);
            mass = std::make_unique<float[]>(MAX_JETS);
            area = std::make_unique<float[]>(MAX_JETS);
            dyn_split = std::make_unique<int[]>(MAX_JETS);
            dyn_kt = std::make_unique<float[]>(MAX_JETS);
            dyn_z = std::make_unique<float[]>(MAX_JETS);
            girth = std::make_unique<float[]>(MAX_JETS);
            thrust = std::make_unique<float[]>(MAX_JETS);
            lha = std::make_unique<float[]>(MAX_JETS);
            ptd = std::make_unique<float[]>(MAX_JETS);
            
            // Initialize to zero
            std::memset(pt.get(), 0, MAX_JETS * sizeof(float));
            std::memset(eta.get(), 0, MAX_JETS * sizeof(float));
            std::memset(phi.get(), 0, MAX_JETS * sizeof(float));
            std::memset(mass.get(), 0, MAX_JETS * sizeof(float));
            std::memset(area.get(), 0, MAX_JETS * sizeof(float));
            std::memset(dyn_split.get(), 0, MAX_JETS * sizeof(int));
            std::memset(dyn_kt.get(), 0, MAX_JETS * sizeof(float));
            std::memset(dyn_z.get(), 0, MAX_JETS * sizeof(float));
            std::memset(girth.get(), 0, MAX_JETS * sizeof(float));
            std::memset(thrust.get(), 0, MAX_JETS * sizeof(float));
            std::memset(lha.get(), 0, MAX_JETS * sizeof(float));
            std::memset(ptd.get(), 0, MAX_JETS * sizeof(float));
        }
        
        // Copy constructor
        JetBranches(const JetBranches& other) : nJets(other.nJets) {
            pt = std::make_unique<float[]>(MAX_JETS);
            eta = std::make_unique<float[]>(MAX_JETS);
            phi = std::make_unique<float[]>(MAX_JETS);
            mass = std::make_unique<float[]>(MAX_JETS);
            area = std::make_unique<float[]>(MAX_JETS);
            dyn_split = std::make_unique<int[]>(MAX_JETS);
            dyn_kt = std::make_unique<float[]>(MAX_JETS);
            dyn_z = std::make_unique<float[]>(MAX_JETS);
            girth = std::make_unique<float[]>(MAX_JETS);
            thrust = std::make_unique<float[]>(MAX_JETS);
            lha = std::make_unique<float[]>(MAX_JETS);
            ptd = std::make_unique<float[]>(MAX_JETS);
            
            std::memcpy(pt.get(), other.pt.get(), MAX_JETS * sizeof(float));
            std::memcpy(eta.get(), other.eta.get(), MAX_JETS * sizeof(float));
            std::memcpy(phi.get(), other.phi.get(), MAX_JETS * sizeof(float));
            std::memcpy(mass.get(), other.mass.get(), MAX_JETS * sizeof(float));
            std::memcpy(area.get(), other.area.get(), MAX_JETS * sizeof(float));
            std::memcpy(dyn_split.get(), other.dyn_split.get(), MAX_JETS * sizeof(int));
            std::memcpy(dyn_kt.get(), other.dyn_kt.get(), MAX_JETS * sizeof(float));
            std::memcpy(dyn_z.get(), other.dyn_z.get(), MAX_JETS * sizeof(float));
            std::memcpy(girth.get(), other.girth.get(), MAX_JETS * sizeof(float));
            std::memcpy(thrust.get(), other.thrust.get(), MAX_JETS * sizeof(float));
            std::memcpy(lha.get(), other.lha.get(), MAX_JETS * sizeof(float));
            std::memcpy(ptd.get(), other.ptd.get(), MAX_JETS * sizeof(float));
        }
        
        // Assignment operator
        JetBranches& operator=(const JetBranches& other) {
            if (this != &other) {
                nJets = other.nJets;
                std::memcpy(pt.get(), other.pt.get(), MAX_JETS * sizeof(float));
                std::memcpy(eta.get(), other.eta.get(), MAX_JETS * sizeof(float));
                std::memcpy(phi.get(), other.phi.get(), MAX_JETS * sizeof(float));
                std::memcpy(mass.get(), other.mass.get(), MAX_JETS * sizeof(float));
                std::memcpy(area.get(), other.area.get(), MAX_JETS * sizeof(float));
                std::memcpy(dyn_split.get(), other.dyn_split.get(), MAX_JETS * sizeof(int));
                std::memcpy(dyn_kt.get(), other.dyn_kt.get(), MAX_JETS * sizeof(float));
                std::memcpy(dyn_z.get(), other.dyn_z.get(), MAX_JETS * sizeof(float));
                std::memcpy(girth.get(), other.girth.get(), MAX_JETS * sizeof(float));
                std::memcpy(thrust.get(), other.thrust.get(), MAX_JETS * sizeof(float));
                std::memcpy(lha.get(), other.lha.get(), MAX_JETS * sizeof(float));
                std::memcpy(ptd.get(), other.ptd.get(), MAX_JETS * sizeof(float));
            }
            return *this;
        }
    };
    
    // Map from collection name to branch addresses
    std::map<std::string, JetBranches> fJetBranches;
};

#endif // JETCOLLECTIONMANAGER_H
