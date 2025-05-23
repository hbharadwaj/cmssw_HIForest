#ifndef HELPERS_H
#define HELPERS_H

#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TSystem.h>
#include <TEnv.h>
#include <TStyle.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TObjArray.h>
#include <TObjString.h>
#include <TSystemDirectory.h>
#include <TSystemFile.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>

// Forward declarations of helper functions
std::vector<double> parseVector(const std::string& vecStr);
std::pair<double, double> parseRange(const std::string& rangeStr);

// Configuration structures
struct Config {
    // Existing members
    std::string system;
    std::string dataType;
    std::string inputDir;
    std::string outputDir;
    std::string outputPrefix;
    
    // Event selection
    double vzCut;
    double hiHFCutMin;
    double hiHFCutMax;
    
    // Photon selection
    double photonEtMin;
    double photonEtaMax;
    double photonHoverEMax;
    double photonSigmaIEtaIEtaMax;
    double photonIsoMax;
    double photonR9Min;
    
    // MC photon selection
    bool mcPhotonMatchRequired;
    int mcPhotonPID;
    std::vector<int> mcPhotonMomPIDs;
    double mcPhotonCalIsoDR04Max;
    
    // Jet selection
    double jetPtMin;
    double jetEtaMax;
    double deltaPhiMin;
    double xjMin;
    
    // Binning
    std::vector<double> centBins;
    std::vector<double> etaBins;
    
    // Jet collections
    std::vector<std::string> jetCollections;

    // Add missing members
    std::string analysisCases;  // List of jet collections to analyze
    
    // Ensure all config parameters are loaded
    bool loadFromEnv(TEnv& config) {
        system = config.GetValue("System", "2023_PbPb");
        dataType = config.GetValue("DataType", "Data");
        inputDir = config.GetValue("InputDir", "");
        outputDir = config.GetValue("OutputDir", "output");
        outputPrefix = config.GetValue("OutputPrefix", "outJetSubstructure");
        
        // Event selection
        vzCut = config.GetValue("VzCut", 15.0);
        hiHFCutMin = config.GetValue("HiHFCutMin", 0.0);
        hiHFCutMax = config.GetValue("HiHFCutMax", 7000.0);
        
        // Photon selection
        photonEtMin = config.GetValue("PhotonEtMin", 30.0);
        photonEtaMax = config.GetValue("PhotonEtaMax", 1.44);
        photonHoverEMax = config.GetValue("PhotonHoverEMax", 0.119947);
        photonSigmaIEtaIEtaMax = config.GetValue("PhotonSigmaIEtaIEtaMax", 0.010392);
        photonIsoMax = config.GetValue("PhotonIsoMax", 2.099277);
        photonR9Min = config.GetValue("PhotonR9Min", 0.8);
        
        // MC photon selection - only relevant for MC
        mcPhotonMatchRequired = config.GetValue("MCPhotonMatchRequired", false);
        mcPhotonPID = config.GetValue("MCPhotonPID", 22); // Default to photon (22)
        
        // Parse mother PIDs from comma-separated list
        std::string momPIDsStr = config.GetValue("MCPhotonMomPID", "22,-999");
        std::vector<double> momPIDsDouble = parseVector(momPIDsStr);
        mcPhotonMomPIDs.clear();
        for (const auto& pid : momPIDsDouble) {
            mcPhotonMomPIDs.push_back(static_cast<int>(pid));
        }
        
        mcPhotonCalIsoDR04Max = config.GetValue("MCPhotonCalIsoDR04Max", 5.0);
        
        // Jet selection - remove redundant parameters
        jetPtMin = config.GetValue("JetPtMin", 20.0);
        jetEtaMax = config.GetValue("JetEtaMax", 2.0);
        deltaPhiMin = config.GetValue("DeltaPhiMin", 2.094);
        xjMin = config.GetValue("XjMin", 0.4);
        analysisCases = config.GetValue("AnalysisCases", "");
        
        return true;
    }
};

struct HistConfig {
    std::string title;
    int bins;
    double range_min;
    double range_max;
    bool y_log;
    std::vector<double> centrality_bins;
    std::vector<double> eta_bins;
};

// Add EventBranches structure
struct EventBranches {
    void setupBranches(TTree* tree) {
        // Add your branch setup code here
        // This is just a placeholder
    }
};

// Helper function to recursively collect .root files
std::vector<std::string> GetFiles(const std::string &dir, int limit = 99999) {
    std::vector<std::string> out;
    TSystemDirectory sd(dir.c_str(), dir.c_str());
    if (auto *lst = sd.GetListOfFiles()) {
        TIter next(lst);
        while (auto *f = (TSystemFile*)next()) {
            if ((int)out.size() >= limit) break;
            std::string name = f->GetName();
            if (f->IsDirectory() && name.find('.') == std::string::npos) {
                auto sub = GetFiles(dir + "/" + name, limit - out.size());
                out.insert(out.end(), sub.begin(), sub.end());
            } else if (name.rfind(".root") != std::string::npos) {
                out.push_back(dir + "/" + name);
            }
        }
    }
    return out;
}

// Add helper function for range parsing
std::pair<double, double> parseRange(const std::string& rangeStr) {
    size_t pos = rangeStr.find(",");
    if (pos != std::string::npos) {
        return {std::stod(rangeStr.substr(0, pos)), 
                std::stod(rangeStr.substr(pos + 1))};
    }
    return {0.0, 100.0}; // default range
}

// Add helper function for vector parsing
std::vector<double> parseVector(const std::string& vecStr) {
    std::vector<double> result;
    std::string temp = vecStr;
    size_t pos = 0;
    while ((pos = temp.find(",")) != std::string::npos) {
        result.push_back(std::stod(temp.substr(0, pos)));
        temp.erase(0, pos + 1);
    }
    if (!temp.empty()) {
        result.push_back(std::stod(temp));
    }
    return result;
}

// Config loading functions
bool loadConfig(Config& cfg, const std::string& configPath) {
    TEnv config;
    if (config.ReadFile(configPath.c_str(), kEnvGlobal) < 0) {
        std::cerr << "Error reading config file" << std::endl;
        return false;
    }
    return cfg.loadFromEnv(config);
}

bool loadHistConfig(std::map<std::string, HistConfig>& histConfigs, const std::string& histConfigPath) {
    TEnv config;
    if (config.ReadFile(histConfigPath.c_str(), kEnvGlobal) < 0) {
        std::cerr << "Error reading histogram config file" << std::endl;
        return false;
    }

    gStyle->SetOptStat(0);
    if (config.GetValue("Global.CMS_Style", 1)) {
        gStyle->SetPadTickX(1);
        gStyle->SetPadTickY(1);
    }

    const char* histTypes[] = {"PhotonEt", "JetPt", "DeltaPhi"};
    for (const auto& hist : histTypes) {
        std::string base = std::string("Hist.") + hist;
        HistConfig hc;
        hc.title = config.GetValue((base + ".title").c_str(), hist);
        hc.bins = config.GetValue((base + ".bins").c_str(), 50);
        
        std::string rangeStr = config.GetValue((base + ".range").c_str(), "0,100");
        auto [min, max] = parseRange(rangeStr);
        hc.range_min = min;
        hc.range_max = max;
        
        hc.y_log = config.GetValue((base + ".y_log").c_str(), false);
        
        // Parse centrality and eta bins
        std::string centStr = config.GetValue((base + ".centrality_bins").c_str(), "0,30,60,180");
        hc.centrality_bins = parseVector(centStr);
        
        std::string etaStr = config.GetValue((base + ".eta_bins").c_str(), "");
        if (!etaStr.empty()) {
            hc.eta_bins = parseVector(etaStr);
        }
        
        histConfigs[hist] = hc;
    }
    return true;
}

// Add directory creation helper
bool createDirectory(const std::string& path) {
    if (path.empty()) return false;
    
    std::string fullPath = path;
    if (fullPath.back() == '/') fullPath.pop_back();
    
    size_t pos = 0;
    std::string dir;
    int status = 0;

    while ((pos = fullPath.find('/', pos)) != std::string::npos) {
        dir = fullPath.substr(0, pos);
        if (!dir.empty()) {
            status = gSystem->Exec(("test -d " + dir).c_str());
            if (status != 0) {
                status = gSystem->Exec(("mkdir -p " + dir).c_str());
                if (status != 0) {
                    std::cerr << "Error creating directory: " << dir << std::endl;
                    return false;
                }
            }
        }
        pos++;
    }
    
    status = gSystem->Exec(("test -d " + fullPath).c_str());
    if (status != 0) {
        status = gSystem->Exec(("mkdir -p " + fullPath).c_str());
        if (status != 0) {
            std::cerr << "Error creating directory: " << fullPath << std::endl;
            return false;
        }
    }
    
    return true;
}


void ListBranchesAndTypes(TTree* tree, const TString& prefix = "") {
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

#endif
