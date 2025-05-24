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
#include <TLeaf.h>
#include <TBranch.h>
#include <TList.h>
#include <TFriendElement.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <set>
#include <sstream>
#include <algorithm>

// Logging system
enum LogLevel { LOG_ERROR = 0, LOG_INFO = 1, LOG_DEBUG = 2, LOG_TRACE = 3 };
extern int g_verbosity; // Global verbosity level

// Logging function with verbosity levels
void log(LogLevel level, const std::string& message);

// Structure to hold parsed class member information for header generation
struct ClassMember {
    std::string name;
    std::string type;
    bool isArray;
    int arraySize;
    
    ClassMember(const std::string& n, const std::string& t, bool arr = false, int size = 0) 
        : name(n), type(t), isArray(arr), arraySize(size) {}
};

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
    std::vector<std::string> AnalysisCases; // Add this for dynamic jet collection support

    // Add missing members
    std::string analysisCases;  // List of jet collections to analyze
    
    // Logging and compilation control
    int verbosity;              // Logging verbosity level (0-3)
    bool regenerateHeader;      // Whether to regenerate photonJet.h
    
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
        // Parse AnalysisCases string into vector
        AnalysisCases.clear();
        std::string ac = analysisCases;
        size_t pos = 0;
        while ((pos = ac.find(",")) != std::string::npos) {
            std::string token = ac.substr(0, pos);
            if (!token.empty()) AnalysisCases.push_back(token);
            ac.erase(0, pos + 1);
        }
        if (!ac.empty()) AnalysisCases.push_back(ac);
        
        // Logging and compilation control
        verbosity = config.GetValue("Verbosity", 1);
        regenerateHeader = config.GetValue("RegenerateHeader", false);
        
        return true;
    }
};

// Header generation function declarations (after Config is defined)
std::vector<ClassMember> parseHeaderFile(const std::string& headerPath, const std::string& className);
void generatePhotonJetHeader(const Config& cfg);

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

// Header generation functions for dynamic photonJet.h creation
std::vector<ClassMember> parseHeaderFile(const std::string& headerPath, const std::string& className) {
    std::vector<ClassMember> members;
    std::ifstream file(headerPath);
    
    if (!file.is_open()) {
        std::cerr << "Could not open header file: " << headerPath << std::endl;
        return members;
    }
    
    std::string line;
    bool inClass = false;
    bool inPublicSection = false;
    
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line.find("//") == 0) continue;
        
        // Check if we're entering the class
        if (line.find("class " + className) != std::string::npos) {
            inClass = true;
            continue;
        }
        
        if (!inClass) continue;
        
        // Check for public section
        if (line.find("public :") != std::string::npos || 
            line.find("public:") != std::string::npos) {
            inPublicSection = true;
            continue;
        }
        
        // Check for private/protected sections (stop parsing member variables)
        if (line.find("private") != std::string::npos || 
            line.find("protected") != std::string::npos) {
            inPublicSection = false;
            continue;
        }
        
        // End of class
        if (line.find("};") != std::string::npos && inClass) {
            break;
        }
        
        if (!inPublicSection) continue;
        
        // Parse member variables
        line = line.substr(line.find_first_not_of(" \t")); // trim leading whitespace
        
        // Skip function declarations, constructors, etc.
        if (line.find("(") != std::string::npos || 
            line.find("virtual") != std::string::npos ||
            line.find("~") != std::string::npos ||
            line.find("//") == 0) continue;
        
        // Parse ROOT::VecOps::RVec<type> *varName;
        if (line.find("ROOT::VecOps::RVec<") != std::string::npos) {
            size_t start = line.find("*") + 1;
            size_t end = line.find(";");
            if (start != std::string::npos && end != std::string::npos) {
                std::string varName = line.substr(start, end - start);
                varName.erase(0, varName.find_first_not_of(" \t"));
                varName.erase(varName.find_last_not_of(" \t") + 1);
                
                std::string vecType;
                if (line.find("RVec<float>") != std::string::npos) {
                    vecType = "ROOT::VecOps::RVec<float>*";
                } else if (line.find("RVec<int>") != std::string::npos) {
                    vecType = "ROOT::VecOps::RVec<int>*";
                }
                
                members.emplace_back(varName, vecType);
            }
        }
        // Parse simple types: Int_t, Float_t, etc.
        else if (line.find("Int_t") != std::string::npos || 
                 line.find("Float_t") != std::string::npos ||
                 line.find("UInt_t") != std::string::npos ||
                 line.find("ULong64_t") != std::string::npos) {
            
            size_t end = line.find(";");
            if (end == std::string::npos) continue;
            
            std::string varDecl = line.substr(0, end);
            
            // Check for arrays
            size_t arrayStart = varDecl.find("[");
            size_t arrayEnd = varDecl.find("]");
            
            if (arrayStart != std::string::npos && arrayEnd != std::string::npos) {
                // Array variable
                std::string arraySize = varDecl.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
                int size = 0;
                try {
                    size = std::stoi(arraySize);
                } catch (...) {
                    size = 0; // Use dynamic size if can't parse
                }
                
                size_t nameStart = varDecl.find_last_of(" \t", arrayStart) + 1;
                std::string varName = varDecl.substr(nameStart, arrayStart - nameStart);
                
                std::string baseType;
                if (line.find("Float_t") != std::string::npos) {
                    baseType = "float";
                } else if (line.find("Int_t") != std::string::npos) {
                    baseType = "int";
                }
                
                members.emplace_back(varName, baseType, true, size);
            } else {
                // Simple variable
                size_t nameStart = varDecl.find_last_of(" \t") + 1;
                std::string varName = varDecl.substr(nameStart);
                varName.erase(0, varName.find_first_not_of(" \t"));
                varName.erase(varName.find_last_not_of(" \t") + 1);
                
                std::string baseType;
                if (line.find("Int_t") != std::string::npos) {
                    baseType = "int";
                } else if (line.find("Float_t") != std::string::npos) {
                    baseType = "float";
                } else if (line.find("UInt_t") != std::string::npos) {
                    baseType = "unsigned int";
                } else if (line.find("ULong64_t") != std::string::npos) {
                    baseType = "unsigned long long";
                }
                
                members.emplace_back(varName, baseType);
            }
        }
    }
    
    file.close();
    return members;
}

void generatePhotonJetHeader(const Config& cfg) {
    // Determine which header file to parse based on config
    std::string className, headerPath;
    
    if (cfg.system.find("2023_PbPb") != std::string::npos) {
        if (cfg.dataType == "MC") {
            className = "GammaJet2023_PbPbMC";
            // Try multiple possible paths for the header file
            std::vector<std::string> possiblePaths = {
                "./include/GammaJet2023_PbPbMC.h",
                "../include/GammaJet2023_PbPbMC.h",
                "include/GammaJet2023_PbPbMC.h",
                "/afs/cern.ch/user/b/bharikri/private/HeavyIon/run3_gamma_jet/CMSSW_13_2_13/src/HeavyIonsAnalysis/JetAnalysis/test/2_SkimPlotSubstructure/include/GammaJet2023_PbPbMC.h"
            };
            
            for (const auto& path : possiblePaths) {
                std::ifstream testFile(path);
                if (testFile.good()) {
                    headerPath = path;
                    break;
                }
            }
        } else {
            className = "GammaJet2023_PbPbData";
            // Try multiple possible paths for the header file
            std::vector<std::string> possiblePaths = {
                "./include/GammaJet2023_PbPbData.h",
                "../include/GammaJet2023_PbPbData.h", 
                "include/GammaJet2023_PbPbData.h",
                "/afs/cern.ch/user/b/bharikri/private/HeavyIon/run3_gamma_jet/CMSSW_13_2_13/src/HeavyIonsAnalysis/JetAnalysis/test/2_SkimPlotSubstructure/include/GammaJet2023_PbPbData.h"
            };
            
            for (const auto& path : possiblePaths) {
                std::ifstream testFile(path);
                if (testFile.good()) {
                    headerPath = path;
                    break;
                }
            }
        }
    } else {
        std::cerr << "Unsupported system: " << cfg.system << std::endl;
        return;
    }
    
    if (headerPath.empty()) {
        std::cerr << "Could not find header file for " << className << std::endl;
        return;
    }
    
    // Parse the header file
    std::vector<ClassMember> members = parseHeaderFile(headerPath, className);
    
    if (members.empty()) {
        std::cerr << "No members found in header file: " << headerPath << std::endl;
        return;
    }
    
    // Categorize members
    std::vector<ClassMember> photonVecMembers, photonIntMembers, jetArrayMembers;
    std::set<std::string> jetCollections;
    
    for (const auto& member : members) {
        if (member.name.find("ggHi_pho") != std::string::npos) {
            if (member.type.find("ROOT::VecOps::RVec<float>*") != std::string::npos) {
                photonVecMembers.push_back(member);
            } else if (member.type.find("ROOT::VecOps::RVec<int>*") != std::string::npos) {
                photonVecMembers.push_back(member);
            }
        } else if (member.name == "ggHi_nPho") {
            photonIntMembers.push_back(member);
        } else if (member.isArray && member.name.find("_jt") != std::string::npos) {
            // Extract jet collection prefix (e.g., AK2Z1, AK3Z2, etc.)
            size_t underscorePos = member.name.find("_");
            if (underscorePos != std::string::npos) {
                std::string prefix = member.name.substr(0, underscorePos);
                jetCollections.insert(prefix);
                jetArrayMembers.push_back(member);
            }
        }
    }
    
    // Generate the header file
    std::string outputHeaderPath = "../include/photonJet.h";
    std::ofstream headerFile(outputHeaderPath);
    
    if (!headerFile.is_open()) {
        std::cerr << "Error: Could not open " << outputHeaderPath << " for writing" << std::endl;
        return;
    }
    
    headerFile << "// This file is auto-generated based on the config and system." << std::endl;
    headerFile << "// It provides maps from variable names to pointer-to-member for dynamic access in photonJet.C" << std::endl;
    headerFile << "#pragma once" << std::endl;
    headerFile << "#include \"" << className << ".h\"" << std::endl;
    
    if (cfg.dataType == "MC") {
        headerFile << "#include \"GammaJet2023_PbPbData.h\"" << std::endl;
    } else {
        headerFile << "#include \"GammaJet2023_PbPbMC.h\"" << std::endl;
    }
    
    headerFile << "#include <map>" << std::endl;
    headerFile << "#include <string>" << std::endl;
    headerFile << "#include <vector>" << std::endl;
    headerFile << std::endl;
    
    // Generate type aliases
    if (cfg.dataType == "MC") {
        headerFile << "// Type aliases for pointer-to-member types" << std::endl;
        headerFile << "using PhotonVecPtrMC = ROOT::VecOps::RVec<float>* " << className << "::*;" << std::endl;
        headerFile << "using PhotonVecIntPtrMC = ROOT::VecOps::RVec<int>* " << className << "::*;" << std::endl;
        headerFile << "using PhotonIntPtrMC = int " << className << "::*;" << std::endl;
        
        // Determine max array size for jet arrays
        int maxArraySize = 0;
        for (const auto& member : jetArrayMembers) {
            if (member.arraySize > maxArraySize) {
                maxArraySize = member.arraySize;
            }
        }
        headerFile << "using JetArrPtrMC = float (" << className << "::*)[" << maxArraySize << "];" << std::endl;
        headerFile << "using JetIntArrPtrMC = int (" << className << "::*)[" << maxArraySize << "];" << std::endl;
        headerFile << std::endl;
        
        // Generate MC struct
        headerFile << "// For MC analyzer" << std::endl;
        headerFile << "struct PhotonJetMemberMapsMC {" << std::endl;
        headerFile << "    std::map<std::string, PhotonVecPtrMC> photonVecMap;" << std::endl;
        headerFile << "    std::map<std::string, PhotonVecIntPtrMC> photonVecIntMap;" << std::endl;
        headerFile << "    std::map<std::string, PhotonIntPtrMC> photonIntMap;" << std::endl;
        headerFile << "    std::map<std::string, JetArrPtrMC> jetArrMapMC;" << std::endl;
        headerFile << "    std::map<std::string, JetIntArrPtrMC> jetIntArrMapMC;" << std::endl;
        headerFile << "    " << std::endl;
        headerFile << "    PhotonJetMemberMapsMC() {" << std::endl;
        
        // Add photon vector variables
        headerFile << "        // Initialize photon vector variables" << std::endl;
        for (const auto& member : photonVecMembers) {
            if (member.type.find("float") != std::string::npos) {
                headerFile << "        photonVecMap[\"" << member.name << "\"] = &" << className << "::" << member.name << ";" << std::endl;
            } else if (member.type.find("int") != std::string::npos) {
                headerFile << "        photonVecIntMap[\"" << member.name << "\"] = &" << className << "::" << member.name << ";" << std::endl;
            }
        }
        
        // Add photon integer variables
        headerFile << "        " << std::endl;
        headerFile << "        // Initialize photon integer variables" << std::endl;
        for (const auto& member : photonIntMembers) {
            headerFile << "        photonIntMap[\"" << member.name << "\"] = &" << className << "::" << member.name << ";" << std::endl;
        }
        
        // Add jet array variables
        headerFile << "        " << std::endl;
        headerFile << "        // Initialize jet array variables" << std::endl;
        for (const auto& member : jetArrayMembers) {
            if (member.type == "float") {
                headerFile << "        jetArrMapMC[\"" << member.name << "\"] = &" << className << "::" << member.name << ";" << std::endl;
            } else if (member.type == "int") {
                headerFile << "        jetIntArrMapMC[\"" << member.name << "\"] = &" << className << "::" << member.name << ";" << std::endl;
            }
        }
        
        headerFile << "    }" << std::endl;
        headerFile << "};" << std::endl;
        
    } else {
        // Generate similar for Data
        headerFile << "// Type aliases for pointer-to-member types" << std::endl;
        headerFile << "using PhotonVecPtrData = ROOT::VecOps::RVec<float>* " << className << "::*;" << std::endl;
        headerFile << "using PhotonVecIntPtrData = ROOT::VecOps::RVec<int>* " << className << "::*;" << std::endl;
        headerFile << "using PhotonIntPtrData = int " << className << "::*;" << std::endl;
        
        // Determine max array size for jet arrays
        int maxArraySize = 0;
        for (const auto& member : jetArrayMembers) {
            if (member.arraySize > maxArraySize) {
                maxArraySize = member.arraySize;
            }
        }
        headerFile << "using JetArrPtrData = float (" << className << "::*)[" << maxArraySize << "];" << std::endl;
        headerFile << "using JetIntArrPtrData = int (" << className << "::*)[" << maxArraySize << "];" << std::endl;
        headerFile << std::endl;
        
        // Generate Data struct
        headerFile << "// For Data analyzer" << std::endl;
        headerFile << "struct PhotonJetMemberMapsData {" << std::endl;
        headerFile << "    std::map<std::string, PhotonVecPtrData> photonVecMap;" << std::endl;
        headerFile << "    std::map<std::string, PhotonVecIntPtrData> photonVecIntMap;" << std::endl;
        headerFile << "    std::map<std::string, PhotonIntPtrData> photonIntMap;" << std::endl;
        headerFile << "    std::map<std::string, JetArrPtrData> jetArrMapData;" << std::endl;
        headerFile << "    std::map<std::string, JetIntArrPtrData> jetIntArrMapData;" << std::endl;
        headerFile << "    " << std::endl;
        headerFile << "    PhotonJetMemberMapsData() {" << std::endl;
        
        // Add photon vector variables
        headerFile << "        // Initialize photon vector variables" << std::endl;
        for (const auto& member : photonVecMembers) {
            if (member.type.find("float") != std::string::npos) {
                headerFile << "        photonVecMap[\"" << member.name << "\"] = &" << className << "::" << member.name << ";" << std::endl;
            } else if (member.type.find("int") != std::string::npos) {
                headerFile << "        photonVecIntMap[\"" << member.name << "\"] = &" << className << "::" << member.name << ";" << std::endl;
            }
        }
        
        // Add photon integer variables
        headerFile << "        " << std::endl;
        headerFile << "        // Initialize photon integer variables" << std::endl;
        for (const auto& member : photonIntMembers) {
            headerFile << "        photonIntMap[\"" << member.name << "\"] = &" << className << "::" << member.name << ";" << std::endl;
        }
        
        // Add jet array variables
        headerFile << "        " << std::endl;
        headerFile << "        // Initialize jet array variables" << std::endl;
        for (const auto& member : jetArrayMembers) {
            if (member.type == "float") {
                headerFile << "        jetArrMapData[\"" << member.name << "\"] = &" << className << "::" << member.name << ";" << std::endl;
            } else if (member.type == "int") {
                headerFile << "        jetIntArrMapData[\"" << member.name << "\"] = &" << className << "::" << member.name << ";" << std::endl;
            }
        }
        
        headerFile << "    }" << std::endl;
        headerFile << "};" << std::endl;
    }
    
    headerFile << std::endl;
    headerFile << "// This file should be regenerated if the config or system changes." << std::endl;
    
    headerFile.close();
    
    std::cout << "Successfully generated photonJet.h for " << cfg.system << " " << cfg.dataType << std::endl;
    std::cout << "Found " << photonVecMembers.size() << " photon vector variables" << std::endl;
    std::cout << "Found " << photonIntMembers.size() << " photon integer variables" << std::endl;
    std::cout << "Found " << jetArrayMembers.size() << " jet array variables" << std::endl;
    std::cout << "Jet collections: ";
    for (const auto& collection : jetCollections) {
        std::cout << collection << " ";
    }
    std::cout << std::endl;
}

// Global verbosity level definition
int g_verbosity = LOG_INFO; // Default verbosity level

// Logging function implementation
void log(LogLevel level, const std::string& message) {
    if (level <= g_verbosity) {
        std::string prefix;
        switch (level) {
            case LOG_ERROR: prefix = "[ERROR] "; break;
            case LOG_INFO:  prefix = "[INFO]  "; break;
            case LOG_DEBUG: prefix = "[DEBUG] "; break;
            case LOG_TRACE: prefix = "[TRACE] "; break;
        }
        std::cout << prefix << message << std::endl;
    }
}

#endif
