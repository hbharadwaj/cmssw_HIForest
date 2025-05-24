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
        }
    }
    
    /**
     * Initialize jet collections from configuration
     * @return true if initialization successful, false otherwise
     */
    bool initialize()
    {
        if (!fConfig || !fTree) {
            std::cerr << "Error: Configuration or Tree not initialized." << std::endl;
            return false;
        }
        
        // Get analysis cases from config
        std::string analysisCases = fConfig->GetValue("AnalysisCases", "");
        if (analysisCases.empty()) {
            std::cerr << "Error: No AnalysisCases defined in configuration." << std::endl;
            return false;
        }
        
        if (fVerbosity > 0) {
            std::cout << "Initializing jet collections: " << analysisCases << std::endl;
        }
        
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
                std::cerr << "Warning: Invalid collection format: " << collection << ". Skipping." << std::endl;
                continue;
            }
            
            // Store collection and setup branch addresses directly
            fCollections.push_back(collection);
            if (!setupBranchAddresses(collection)) {
                std::cerr << "Warning: Failed to setup branches for collection: " << collection << std::endl;
            }
        }
        
        if (fCollections.empty()) {
            std::cerr << "Error: No valid jet collections initialized." << std::endl;
            return false;
        }
        
        if (fVerbosity > 0) {
            std::cout << "Initialized " << fCollections.size() << " jet collections:" << std::endl;
            for (const auto& coll : fCollections) {
                std::cout << " - " << coll << std::endl;
            }
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
        
        const auto& branches = it->second;
        
        switch (property) {
            case PT:        return branches.pt[jetIndex];
            case ETA:       return branches.eta[jetIndex];
            case PHI:       return branches.phi[jetIndex];
            case MASS:      return branches.mass[jetIndex];
            case AREA:      return branches.area[jetIndex];
            case DYN_SPLIT: return static_cast<float>(branches.dyn_split[jetIndex]);
            case DYN_KT:    return branches.dyn_kt[jetIndex];
            case DYN_Z:     return branches.dyn_z[jetIndex];
            case GIRTH:     return branches.girth[jetIndex];
            case THRUST:    return branches.thrust[jetIndex];
            case LHA:       return branches.lha[jetIndex];
            case PTD:       return branches.ptd[jetIndex];
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
        std::cout << "=== Jet Collection Branch Mappings ===" << std::endl;
        for (const auto& collection : fCollections) {
            std::cout << "Collection: " << collection << std::endl;
            
            if (verbose) {
                const auto& branches = fJetBranches.at(collection);
                std::cout << "  Current nJets: " << branches.nJets << std::endl;
                if (branches.nJets > 0) {
                    std::cout << "  First jet pt: " << branches.pt[0] << std::endl;
                    std::cout << "  First jet eta: " << branches.eta[0] << std::endl;
                    std::cout << "  First jet phi: " << branches.phi[0] << std::endl;
                }
            }
        }
        std::cout << "=====================================" << std::endl;
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
            {"_jtpt", branches.pt, true, "float[]"},
            {"_jteta", branches.eta, true, "float[]"},
            {"_jtphi", branches.phi, true, "float[]"},
            {"_jtm", branches.mass, false, "float[]"},
            {"_jtarea", branches.area, false, "float[]"},
            {"_jtdyn_split", branches.dyn_split, false, "int[]"},
            {"_jtdyn_kt", branches.dyn_kt, false, "float[]"},
            {"_jtdyn_z", branches.dyn_z, false, "float[]"},
            {"_jt_girth", branches.girth, false, "float[]"},
            {"_jt_thrust", branches.thrust, false, "float[]"},
            {"_jt_LHA", branches.lha, false, "float[]"},
            {"_jt_pTD", branches.ptd, false, "float[]"}
        };
        
        // Setup all branches using loop
        for (const auto& branchDef : branchDefs) {
            std::string branchName = collection + branchDef.suffix;
            
            if (fTree->GetBranch(branchName.c_str())) {
                fTree->SetBranchAddress(branchName.c_str(), branchDef.ptr);
                if (fVerbosity > 1) {
                    std::cout << "  Connected branch: " << branchName << " (" << branchDef.type << ")" << std::endl;
                }
            } else {
                if (branchDef.required) {
                    std::cerr << "Warning: Required branch '" << branchName << "' not found." << std::endl;
                    success = false;
                } else if (fVerbosity > 0) {
                    std::cerr << "Info: Optional branch '" << branchName << "' not found." << std::endl;
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
    static const int MAX_JETS = 100;
    struct JetBranches {
        int nJets;
        float pt[MAX_JETS];
        float eta[MAX_JETS];
        float phi[MAX_JETS];
        float mass[MAX_JETS];
        float area[MAX_JETS];
        int dyn_split[MAX_JETS];
        float dyn_kt[MAX_JETS];
        float dyn_z[MAX_JETS];
        float girth[MAX_JETS];
        float thrust[MAX_JETS];
        float lha[MAX_JETS];
        float ptd[MAX_JETS];
        
        // Initialize arrays to zero
        JetBranches() : nJets(0) {
            std::memset(pt, 0, MAX_JETS * sizeof(float));
            std::memset(eta, 0, MAX_JETS * sizeof(float));
            std::memset(phi, 0, MAX_JETS * sizeof(float));
            std::memset(mass, 0, MAX_JETS * sizeof(float));
            std::memset(area, 0, MAX_JETS * sizeof(float));
            std::memset(dyn_split, 0, MAX_JETS * sizeof(int));
            std::memset(dyn_kt, 0, MAX_JETS * sizeof(float));
            std::memset(dyn_z, 0, MAX_JETS * sizeof(float));
            std::memset(girth, 0, MAX_JETS * sizeof(float));
            std::memset(thrust, 0, MAX_JETS * sizeof(float));
            std::memset(lha, 0, MAX_JETS * sizeof(float));
            std::memset(ptd, 0, MAX_JETS * sizeof(float));
        }
    };
    
    // Map from collection name to branch addresses
    std::map<std::string, JetBranches> fJetBranches;
};

#endif // JETCOLLECTIONMANAGER_H
