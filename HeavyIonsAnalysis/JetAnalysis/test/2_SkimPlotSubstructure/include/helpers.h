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
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <set>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <cstring>  // Add this for std::memcpy

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

// Logging system
enum LogLevel { LOG_ERROR = 0, LOG_INFO = 1, LOG_DEBUG = 2, LOG_TRACE = 3 };
extern int g_verbosity; // Global verbosity level

// Logging function with verbosity levels and colors
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

// Forward declaration for histogram styling and plotting
struct HistogramConfig;
struct PlottingConfiguration;
struct PlotOptions;

// Function forward declarations
void applyHistogramStyle(TH1* hist, const HistogramConfig& config, int colorIndex);
int getAutoColor(const std::string& colorScheme, int index);
std::vector<std::string> parseStringVector(const std::string& str, char delimiter = ',');
std::vector<double> parseDoubleVector(const std::string& str, char delimiter = ',');

// Plot function forward declarations
TCanvas* Plot_hist_CMS(const std::vector<TH1*>& histos, 
                      const std::vector<std::string>& labels,
                      const std::string& canvasName, 
                      const PlotOptions& opts);
TCanvas* Plot_hist2D_CMS(TH2* histo,
                        const std::string& canvasName,
                        const PlotOptions& opts);
TCanvas* Plot_overlay_CMS(const std::vector<TH1*>& histos,
                         const std::vector<std::string>& labels,
                         const std::string& canvasName,
                         const PlotOptions& opts);
TCanvas* Plot_ratio_CMS(TH1* data, TH1* mc, 
                       const std::string& dataLabel,
                       const std::string& mcLabel,
                       const std::string& canvasName,
                       const PlotOptions& opts);

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

// Helper functions for header generation
std::pair<std::string, std::string> generateStructureContent(const Config& cfg, const std::string& className, 
                                   const std::vector<ClassMember>& photonVecMembers,
                                   const std::vector<ClassMember>& photonIntMembers,
                                   const std::vector<ClassMember>& jetArrayMembers);
void updateExistingHeader(std::ofstream& headerFile, const std::string& existingContent,
                         const Config& cfg, const std::string& className, 
                         const std::string& typeAliases, const std::string& structContent);
void createNewHeader(std::ofstream& headerFile, const Config& cfg, const std::string& className,
                    const std::string& typeAliases, const std::string& structContent);

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
    void setupBranches(TTree* /* tree */) {
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
    // Check if header regeneration is enabled
    if (!cfg.regenerateHeader) {
        std::cout << "[LOG] Header regeneration disabled by config. Skipping header generation." << std::endl;
        return;
    }
    
    // Determine class name and header path based on config
    std::string className, headerPath;
    
    if (cfg.system.find("2023_PbPb") != std::string::npos) {
        if (cfg.dataType == "MC") {
            className = "GammaJet2023_PbPbMC";
        } else {
            className = "GammaJet2023_PbPbData";
        }
        
        // Try multiple possible paths for the header file
        std::vector<std::string> possiblePaths = {
            "./include/" + className + ".h",
            "../include/" + className + ".h",
            "include/" + className + ".h",
            "/afs/cern.ch/user/b/bharikri/private/HeavyIon/run3_gamma_jet/CMSSW_13_2_13/src/HeavyIonsAnalysis/JetAnalysis/test/2_SkimPlotSubstructure/include/" + className + ".h"
        };
        
        for (const auto& path : possiblePaths) {
            std::ifstream testFile(path);
            if (testFile.good()) {
                headerPath = path;
                break;
            }
        }
    } else if (cfg.system.find("2024_ppRef") != std::string::npos) {
        // Example of adding support for a new collision system
        if (cfg.dataType == "MC") {
            className = "GammaJet2024_ppRefMC";
        } else {
            className = "GammaJet2024_ppRefData";
        }
        
        // Try multiple possible paths for the header file
        std::vector<std::string> possiblePaths = {
            "./include/" + className + ".h",
            "../include/" + className + ".h",
            "include/" + className + ".h",
            "/afs/cern.ch/user/b/bharikri/private/HeavyIon/run3_gamma_jet/CMSSW_13_2_13/src/HeavyIonsAnalysis/JetAnalysis/test/2_SkimPlotSubstructure/include/" + className + ".h"
        };
        
        for (const auto& path : possiblePaths) {
            std::ifstream testFile(path);
            if (testFile.good()) {
                headerPath = path;
                break;
            }
        }
    } else {
        std::string errorMsg = "Unsupported system: " + cfg.system;
        std::cerr << errorMsg << std::endl;
        throw std::runtime_error(errorMsg);
    }
    
    if (headerPath.empty()) {
        std::string errorMsg = "Could not find header file for " + className;
        std::cerr << errorMsg << std::endl;
        throw std::runtime_error(errorMsg);
    }
    
    // Parse the header file
    std::vector<ClassMember> members = parseHeaderFile(headerPath, className);
    
    if (members.empty()) {
        std::string errorMsg = "No members found in header file: " + headerPath;
        std::cerr << errorMsg << std::endl;
        throw std::runtime_error(errorMsg);
    }
    
    // Categorize members by data type only, not by name
    std::vector<ClassMember> photonVecMembers, photonIntMembers, jetArrayMembers;
    std::set<std::string> jetCollections;
    
    for (const auto& member : members) {
        if (member.type.find("ROOT::VecOps::RVec<float>*") != std::string::npos) {
            photonVecMembers.push_back(member);
        } else if (member.type.find("ROOT::VecOps::RVec<int>*") != std::string::npos) {
            photonVecMembers.push_back(member);
        } else if ((member.type == "int" || member.type == "unsigned int" || 
                   member.type == "unsigned long long") && !member.isArray) {
            photonIntMembers.push_back(member);
        } else if (member.isArray) {
            // Extract jet collection prefix if it exists
            size_t underscorePos = member.name.find("_");
            if (underscorePos != std::string::npos) {
                std::string prefix = member.name.substr(0, underscorePos);
                jetCollections.insert(prefix);
            }
            jetArrayMembers.push_back(member);
        }
    }
    
    // Check if photonJet.h already exists and read existing content
    std::string outputHeaderPath = "../include/photonJet.h";
    std::string existingContent;
    bool headerExists = false;
    
    std::ifstream existingFile(outputHeaderPath);
    if (existingFile.is_open()) {
        headerExists = true;
        std::string line;
        while (std::getline(existingFile, line)) {
            existingContent += line + "\n";
        }
        existingFile.close();
    }
    
    // Generate new structure content
    // Create system-specific struct name by replacing non-alphanumeric chars with underscores
    std::string systemSuffix = cfg.system;
    std::replace(systemSuffix.begin(), systemSuffix.end(), '-', '_');
    std::replace(systemSuffix.begin(), systemSuffix.end(), ' ', '_');
    
    // Include both system and data type in the structure name
    std::string structName = "PhotonJetMemberMaps_" + systemSuffix + "_" + cfg.dataType;
    auto [typeAliases, structContent] = generateStructureContent(cfg, className, photonVecMembers, photonIntMembers, jetArrayMembers);
    
    // Write the header file
    std::ofstream headerFile(outputHeaderPath);
    if (!headerFile.is_open()) {
        std::string errorMsg = "Error: Could not open " + outputHeaderPath + " for writing";
        std::cerr << errorMsg << std::endl;
        throw std::runtime_error(errorMsg);
    }
    
    if (headerExists) {
        // Parse existing content and update/add the specific structure
        updateExistingHeader(headerFile, existingContent, cfg, className, typeAliases, structContent);
    } else {
        // Create new header from scratch
        createNewHeader(headerFile, cfg, className, typeAliases, structContent);
    }
    
    headerFile.close();
    
    std::cout << "Successfully updated photonJet.h for " << cfg.system << " " << cfg.dataType << std::endl;
    std::cout << "Found " << photonVecMembers.size() << " photon vector variables" << std::endl;
    std::cout << "Found " << photonIntMembers.size() << " photon integer variables" << std::endl;
    std::cout << "Found " << jetArrayMembers.size() << " jet array variables" << std::endl;
    std::cout << "Jet collections: ";
    for (const auto& collection : jetCollections) {
        std::cout << collection << " ";
    }
    std::cout << std::endl;
}

// Helper function implementations for header generation
// Split the structure content generation into two parts: type aliases and struct definition
std::pair<std::string, std::string> generateStructureContent(const Config& cfg, const std::string& className, 
                                   const std::vector<ClassMember>& photonVecMembers,
                                   const std::vector<ClassMember>& photonIntMembers,
                                   const std::vector<ClassMember>& jetArrayMembers) {
    std::ostringstream typeAliases;
    std::ostringstream structContent;
    
    // Create system-specific suffix by replacing non-alphanumeric chars with underscores
    std::string systemSuffix = cfg.system;
    std::replace(systemSuffix.begin(), systemSuffix.end(), '-', '_');
    std::replace(systemSuffix.begin(), systemSuffix.end(), ' ', '_');
    
    // Combined suffix for both system and data type
    std::string combinedSuffix = systemSuffix + "_" + cfg.dataType;
    
    // Generate type aliases with proper array handling
    typeAliases << "// Type aliases for " << cfg.system << " " << cfg.dataType << " pointer-to-member types" << std::endl;
    typeAliases << "using PhotonVecPtr_" << combinedSuffix << " = ROOT::VecOps::RVec<float>* " << className << "::*;" << std::endl;
    typeAliases << "using PhotonVecIntPtr_" << combinedSuffix << " = ROOT::VecOps::RVec<int>* " << className << "::*;" << std::endl;
    typeAliases << "// Using void* to safely handle all integer types (int, unsigned int, unsigned long long)" << std::endl;
    typeAliases << "using PhotonIntPtr_" << combinedSuffix << " = void* " << className << "::*;" << std::endl;
    
    // Use void* for arrays to handle different array sizes safely
    typeAliases << "// Using void* for jet arrays to handle different array sizes safely" << std::endl;
    typeAliases << "using JetArrPtr_" << combinedSuffix << " = void* " << className << "::*;" << std::endl;
    
    // Generate structure with corrected map types
    structContent << "struct PhotonJetMemberMaps_" << combinedSuffix << " {" << std::endl;
    structContent << "    std::map<std::string, PhotonVecPtr_" << combinedSuffix << "> photonVecMap;" << std::endl;
    structContent << "    std::map<std::string, PhotonVecIntPtr_" << combinedSuffix << "> photonVecIntMap;" << std::endl;
    structContent << "    std::map<std::string, void*> photonIntMap; // stores pointer-to-member as void*" << std::endl;
    structContent << "    std::map<std::string, void*> jetArrMap; // handles arrays of all sizes as void*" << std::endl;
    structContent << "    " << std::endl;
    
    // Add helper methods for type-safe array access using union technique
    structContent << "    // Helper union for safe pointer-to-member-array casting" << std::endl;
    structContent << "    union PtrToMemberCaster {" << std::endl;
    structContent << "        void* voidPtr;" << std::endl;
    structContent << "        // Template constructor for arrays" << std::endl;
    structContent << "        template<typename T, int N>" << std::endl;
    structContent << "        PtrToMemberCaster(T (" << className << "::*ptr)[N]) {" << std::endl;
    structContent << "            // Store the bit pattern as a void* - safe for storage only" << std::endl;
    structContent << "            static_assert(sizeof(ptr) <= sizeof(void*), \"Pointer-to-member too large\");" << std::endl;
    structContent << "            voidPtr = nullptr;" << std::endl;
    structContent << "            std::memcpy(&voidPtr, &ptr, sizeof(ptr));" << std::endl;
    structContent << "        }" << std::endl;
    structContent << "    };" << std::endl;
    structContent << "    " << std::endl;
    
    // Add helper methods for type-safe array access
    structContent << "    // Helper methods for type-safe array access" << std::endl;
    structContent << "    template<typename T, int N>" << std::endl;
    structContent << "    bool setArrayPtr(const std::string& name, T (" << className << "::*ptr)[N]) {" << std::endl;
    structContent << "        PtrToMemberCaster caster(ptr);" << std::endl;
    structContent << "        jetArrMap[name] = caster.voidPtr;" << std::endl;
    structContent << "        return true;" << std::endl;
    structContent << "    }" << std::endl;
    structContent << "    " << std::endl;
    
    structContent << "    PhotonJetMemberMaps_" << combinedSuffix << "() {" << std::endl;
    
    // Add photon vector variables
    structContent << "        // Initialize photon vector variables" << std::endl;
    for (const auto& member : photonVecMembers) {
        if (member.type.find("float") != std::string::npos) {
            structContent << "        photonVecMap[\"" << member.name << "\"] = &" << className << "::" << member.name << ";" << std::endl;
        } else if (member.type.find("int") != std::string::npos) {
            structContent << "        photonVecIntMap[\"" << member.name << "\"] = &" << className << "::" << member.name << ";" << std::endl;
        }
    }
    
    // Add photon integer variables
    structContent << "        " << std::endl;
    structContent << "        // Initialize photon integer variables" << std::endl;
    for (const auto& member : photonIntMembers) {
        structContent << "        photonIntMap[\"" << member.name << "\"] = reinterpret_cast<void*>(&" << className << "::" << member.name << ");" << std::endl;
    }
    
    // Add jet array variables using template helper method
    structContent << "        " << std::endl;
    structContent << "        // Initialize jet array variables using template helper" << std::endl;
    for (const auto& member : jetArrayMembers) {
        structContent << "        setArrayPtr(\"" << member.name << "\", &" << className << "::" << member.name << ");" << std::endl;
    }
    
    structContent << "    }" << std::endl;
    structContent << "};" << std::endl;
    
    return std::make_pair(typeAliases.str(), structContent.str());
}

void updateExistingHeader(std::ofstream& headerFile, const std::string& existingContent,
                         const Config& cfg, const std::string& className, 
                         const std::string& typeAliases, const std::string& structContent) {
    // Create system-specific struct name by replacing non-alphanumeric chars with underscores
    std::string systemSuffix = cfg.system;
    std::replace(systemSuffix.begin(), systemSuffix.end(), '-', '_');
    std::replace(systemSuffix.begin(), systemSuffix.end(), ' ', '_');
    
    // Include both system and data type in the structure name
    std::string structName = "PhotonJetMemberMaps_" + systemSuffix + "_" + cfg.dataType;
    
    // Create a combined suffix for type alias check
    std::string combinedSuffix = systemSuffix + "_" + cfg.dataType;
    
    // Check if type aliases already exist
    bool hasTypeAliases = existingContent.find("using PhotonVecPtr_" + combinedSuffix) != std::string::npos;
    
    // Check if all necessary includes are present, specifically both MC and Data headers
    std::string mcClassName, dataClassName;
    if (cfg.system.find("2023_PbPb") != std::string::npos) {
        mcClassName = "GammaJet2023_PbPbMC";
        dataClassName = "GammaJet2023_PbPbData";
    } else if (cfg.system.find("2024_ppRef") != std::string::npos) {
        mcClassName = "GammaJet2024_ppRefMC";
        dataClassName = "GammaJet2024_ppRefData";
    } else {
        // Use the provided className and derive the complementary one
        if (cfg.dataType == "MC") {
            mcClassName = className;
            dataClassName = className.substr(0, className.size() - 2) + "Data";
        } else {
            dataClassName = className;
            mcClassName = className.substr(0, className.size() - 4) + "MC";
        }
    }
    
    // Check if includes are properly present
    bool hasMcInclude = existingContent.find("#include \"" + mcClassName + ".h\"") != std::string::npos;
    bool hasDataInclude = existingContent.find("#include \"" + dataClassName + ".h\"") != std::string::npos;
    
    // Create a working copy of the content that we can modify
    std::string workingContent = existingContent;
    
    // First, let's update the includes if needed
    if (!hasMcInclude || !hasDataInclude) {
        // We need to update the includes section
        size_t pragmaEnd = workingContent.find("#pragma once");
        if (pragmaEnd != std::string::npos) {
            pragmaEnd = workingContent.find("\n", pragmaEnd) + 1; // Move to after the newline
            
            // Build the new includes section
            std::string newIncludes = "";
            if (!hasMcInclude) {
                newIncludes += "#include \"" + mcClassName + ".h\"\n";
            }
            if (!hasDataInclude) {
                newIncludes += "#include \"" + dataClassName + ".h\"\n";
            }
            
            // Insert the new includes after #pragma once
            workingContent = workingContent.substr(0, pragmaEnd) + 
                           newIncludes + 
                           workingContent.substr(pragmaEnd);
        }
    }
    
    // Check if the type aliases already exist
    if (hasTypeAliases) {
        // Find the position of the type alias to determine where to add the struct
        // (Currently not used but kept for potential future implementation)
    }
    
    // Check if the structure already exists
    size_t structStart = workingContent.find("struct " + structName);
    
    if (structStart != std::string::npos) {
        // Structure exists, replace it
        size_t structEnd = workingContent.find("};", structStart);
        if (structEnd != std::string::npos) {
            structEnd += 2; // Include the "};"
            
            // Write content before the old structure
            headerFile << workingContent.substr(0, structStart);
            
            // Write just the struct content since type aliases already exist
            headerFile << structContent << std::endl;
            
            // Write content after the old structure
            headerFile << workingContent.substr(structEnd + 1);
        } else {
            // Couldn't find structure end, append new structure
            headerFile << workingContent << std::endl;
            
            // Add type aliases only if needed
            if (!hasTypeAliases) {
                headerFile << typeAliases << std::endl;
            }
            
            headerFile << structContent << std::endl;
        }
    } else {
        // Structure doesn't exist, add it
        size_t insertPos = workingContent.find("// This file should be regenerated");
        if (insertPos != std::string::npos) {
            headerFile << workingContent.substr(0, insertPos);
            
            // Add type aliases only if needed
            if (!hasTypeAliases) {
                headerFile << typeAliases << std::endl;
            }
            
            headerFile << structContent << std::endl << std::endl;
            headerFile << workingContent.substr(insertPos);
        } else {
            // Just append at the end
            headerFile << workingContent << std::endl;
            
            // Add type aliases only if needed
            if (!hasTypeAliases) {
                headerFile << typeAliases << std::endl;
            }
            
            headerFile << structContent << std::endl;
        }
    }
}

// Create new header from scratch
void createNewHeader(std::ofstream& headerFile, const Config& cfg, const std::string& className,
                    const std::string& typeAliases, const std::string& structContent) {
    headerFile << "// This file is auto-generated based on the config and system." << std::endl;
    headerFile << "// It provides maps from variable names to pointer-to-member for dynamic access in photonJet.C" << std::endl;
    headerFile << "#pragma once" << std::endl;
    
    // Always include both MC and Data headers for the current system
    std::string mcClassName, dataClassName;
    if (cfg.system.find("2023_PbPb") != std::string::npos) {
        mcClassName = "GammaJet2023_PbPbMC";
        dataClassName = "GammaJet2023_PbPbData";
    } else if (cfg.system.find("2024_ppRef") != std::string::npos) {
        mcClassName = "GammaJet2024_ppRefMC";
        dataClassName = "GammaJet2024_ppRefData";
    } else {
        // Use the provided className and derive the complementary one
        if (cfg.dataType == "MC") {
            mcClassName = className;
            dataClassName = className.substr(0, className.size() - 2) + "Data";
        } else {
            dataClassName = className;
            mcClassName = className.substr(0, className.size() - 4) + "MC";
        }
    }
    
    headerFile << "#include \"" << mcClassName << ".h\"" << std::endl;
    headerFile << "#include \"" << dataClassName << ".h\"" << std::endl;
    headerFile << "#include <map>" << std::endl;
    headerFile << "#include <string>" << std::endl;
    headerFile << "#include <vector>" << std::endl;
    headerFile << std::endl;
    
    headerFile << typeAliases << std::endl << std::endl;
    headerFile << structContent << std::endl;
    headerFile << std::endl;
    headerFile << "// This file should be regenerated if the config or system changes." << std::endl;
}

// Global verbosity level definition
int g_verbosity = LOG_INFO; // Default verbosity level

// Logging function implementation with colored output
void log(LogLevel level, const std::string& message) {
    if (level <= g_verbosity) {
        std::string prefix, color;
        switch (level) {
            case LOG_ERROR: 
                color = RED_COLOR;
                prefix = "[ERROR] ";
                break;
            case LOG_INFO:  
                color = BLUE_COLOR;
                prefix = "[INFO]  ";
                break;
            case LOG_DEBUG: 
                color = CYAN_COLOR;
                prefix = "[DEBUG] ";
                break;
            case LOG_TRACE: 
                color = MAGENTA_COLOR;
                prefix = "[TRACE] ";
                break;
        }
        std::cout << color << prefix << RESET_COLOR << message << std::endl;
    }
}

// ================================================================================================
// CMS-STYLE PLOTTING FUNCTIONS WITH M. PETROFF COLOR SCHEMES
// Following CMS plotting guidelines from arXiv:2107.02270v2
// ================================================================================================

#include <TCanvas.h>
#include <TLegend.h>
#include <TLatex.h>
#include <TPad.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TGraphAsymmErrors.h>
#include <TMultiGraph.h>
#include <THStack.h>
#include <TColor.h>
#include <TGaxis.h>
#include <TAttMarker.h>
#include <TAttLine.h>
#include <TAttFill.h>

// M. Petroff Color Schemes (arXiv:2107.02270v2)
namespace CMSColors {
    // 6-color scheme for categorical data
    const std::vector<int> PetroffColors6 = {
        kBlack,
        TColor::GetColor("#5790fc"), // Blue 
        TColor::GetColor("#e42536"), // Red
        TColor::GetColor("#964a8b"), // Purple
        TColor::GetColor("#9c9ca1"), // Gray        
        TColor::GetColor("#f89c20"), // Orange 
        TColor::GetColor("#7a21dd")  // Violet
    };
    
    // 10-color scheme for more complex plots
    const std::vector<int> PetroffColors10 = {
        kBlack,
        TColor::GetColor("#3f90da"), // Blue
        TColor::GetColor("#bd1f01"), // Red
        TColor::GetColor("#832db6"), // Purple        
        TColor::GetColor("#ffa90e"), // Orange
        TColor::GetColor("#a96b59"), // Brown
        TColor::GetColor("#e76300"), // Dark Orange        
        TColor::GetColor("#94a4a2"), // Gray
        TColor::GetColor("#b9ac70"), // Olive
        TColor::GetColor("#717581"), // Dark Gray
        TColor::GetColor("#92dadd")  // Light Blue
    };
    
    // Traditional ROOT colors for compatibility
    const std::vector<int> TraditionalColors = {
        kBlack, kBlue, kRed, kMagenta, kGreen+2, kOrange, kCyan, kYellow+2, kGray+2
    };
    
    // Marker styles following CMS guidelines
    const std::vector<int> MarkerStyles = {
        kFullCircle, kFullSquare, kFullTriangleUp, kFullTriangleDown,
        kFullDiamond, kFullStar, kOpenCircle, kOpenSquare, kOpenTriangleUp
    };
    
    // Viridis color palette for sequential data/heatmaps
    void SetViridisColormap() {
        const int NRGBs = 5;
        const int NCont = 255;
        double stops[NRGBs] = {0.00, 0.34, 0.61, 0.84, 1.00};
        double red[NRGBs]   = {0.267004, 0.229739, 0.127568, 0.369214, 0.993248};
        double green[NRGBs] = {0.004874, 0.322361, 0.566949, 0.788888, 0.906157};
        double blue[NRGBs]  = {0.329415, 0.545706, 0.550556, 0.280197, 0.143936};
        TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
        gStyle->SetNumberContours(NCont);
    }
}

// CMS Style Configuration
namespace CMSStyle {
    void SetCMSStyle() {
        // Canvas and pad settings
        gStyle->SetCanvasBorderMode(0);
        gStyle->SetCanvasColor(kWhite);
        gStyle->SetCanvasDefH(600);
        gStyle->SetCanvasDefW(600);
        gStyle->SetCanvasDefX(0);
        gStyle->SetCanvasDefY(0);
        
        gStyle->SetPadBorderMode(0);
        gStyle->SetPadColor(kWhite);
        gStyle->SetPadGridX(false);
        gStyle->SetPadGridY(false);
        gStyle->SetGridColor(0);
        gStyle->SetGridStyle(3);
        gStyle->SetGridWidth(1);
        
        // Frame settings
        gStyle->SetFrameBorderMode(0);
        gStyle->SetFrameBorderSize(1);
        gStyle->SetFrameFillColor(0);
        gStyle->SetFrameFillStyle(0);
        gStyle->SetFrameLineColor(1);
        gStyle->SetFrameLineStyle(1);
        gStyle->SetFrameLineWidth(1);
        
        // Histogram settings
        gStyle->SetHistLineColor(1);
        gStyle->SetHistLineStyle(0);
        gStyle->SetHistLineWidth(1);
        gStyle->SetEndErrorSize(2);
        gStyle->SetErrorX(0.);
        gStyle->SetMarkerStyle(20);
        
        // Fit/function settings
        gStyle->SetOptFit(1);
        gStyle->SetFitFormat("5.4g");
        gStyle->SetFuncColor(2);
        gStyle->SetFuncStyle(1);
        gStyle->SetFuncWidth(1);
        
        // Date settings
        gStyle->SetOptDate(0);
        
        // Statistics box
        gStyle->SetOptFile(0);
        gStyle->SetOptStat(0); // No statistics box
        gStyle->SetStatColor(kWhite);
        gStyle->SetStatFont(42);
        gStyle->SetStatFontSize(0.025);
        gStyle->SetStatTextColor(1);
        gStyle->SetStatFormat("6.4g");
        gStyle->SetStatBorderSize(1);
        gStyle->SetStatH(0.1);
        gStyle->SetStatW(0.15);
        
        // Margins
        gStyle->SetPadTopMargin(0.05);
        gStyle->SetPadBottomMargin(0.13);
        gStyle->SetPadLeftMargin(0.16);
        gStyle->SetPadRightMargin(0.02);
        
        // Axis titles
        gStyle->SetTitleColor(1, "XYZ");
        gStyle->SetTitleFont(42, "XYZ");
        gStyle->SetTitleSize(0.06, "XYZ");
        gStyle->SetTitleXOffset(0.9);
        gStyle->SetTitleYOffset(1.25);
        
        // Axis labels
        gStyle->SetLabelColor(1, "XYZ");
        gStyle->SetLabelFont(42, "XYZ");
        gStyle->SetLabelOffset(0.007, "XYZ");
        gStyle->SetLabelSize(0.05, "XYZ");
        
        // Axis settings
        gStyle->SetAxisColor(1, "XYZ");
        gStyle->SetStripDecimals(kTRUE);
        gStyle->SetTickLength(0.03, "XYZ");
        gStyle->SetNdivisions(510, "XYZ");
        gStyle->SetPadTickX(1);  // To get tick marks on the opposite side of the frame
        gStyle->SetPadTickY(1);
        
        // Change for log plots
        gStyle->SetOptLogx(0);
        gStyle->SetOptLogy(0);
        gStyle->SetOptLogz(0);
        
        // Postscript options
        gStyle->SetPaperSize(20.,20.);
        
        gROOT->ForceStyle();
    }
}

// Enhanced plotting options structure
struct PlotOptions {
    std::string title = "";
    std::string xtitle = "";
    std::string ytitle = "";
    std::string ztitle = "";
    
    // Range settings
    bool setXRange = false;
    double xmin = 0.0, xmax = 1.0;
    bool setYRange = false;
    double ymin = 0.0, ymax = 1.0;
    bool setZRange = false;
    double zmin = 0.0, zmax = 1.0;
    
    // Log scale
    bool logX = false;
    bool logY = false;
    bool logZ = false;
    
    // Canvas settings
    int canvasWidth = 800;
    int canvasHeight = 600;
    double marginLeft = 0.12;
    double marginRight = 0.05;
    double marginTop = 0.08;
    double marginBottom = 0.12;
    
    // Color scheme
    enum ColorScheme { PETROFF6, PETROFF10, TRADITIONAL, CUSTOM } colorScheme = PETROFF6;
    std::vector<int> customColors;
    std::vector<int> customMarkers;
    
    // Legend settings
    bool drawLegend = true;
    double legX1 = 0.7, legY1 = 0.7, legX2 = 0.9, legY2 = 0.9;
    std::string legHeader = "";
    
    // CMS label settings
    std::string cmsLabel = "Preliminary";
    std::string cmsEnergyText = "5.36 TeV PbPb";
    std::string cmsLuminosity = "1.61 nb^{-1}";
    
    // Additional settings
    bool showErrorBars = true;
    
    // CMS label settings
    bool drawCMSLabel = true;
    std::string cmsText = "CMS";
    std::string cmsExtra = "Preliminary";
    std::string lumiText = "";
    bool inFrame = false; // Whether to draw labels inside frame
    
    // Additional text
    std::vector<std::string> additionalText;
    std::vector<std::pair<double, double>> textPositions; // NDC coordinates
    
    // Drawing options
    std::string drawOption = "";
    bool normalize = false;
    
    // Grid
    bool gridX = false;
    bool gridY = false;
    
    PlotOptions() {}
};

// CMS Label Drawing Functions
namespace CMSLabels {
    void DrawCMSText(double x, double y, const std::string& text, double textSize = 0.04) {
        TLatex latex;
        latex.SetNDC();
        latex.SetTextSize(textSize);
        latex.SetTextFont(61); // Helvetica bold
        latex.DrawLatex(x, y, text.c_str());
    }
    
    void DrawExtraText(double x, double y, const std::string& text, double textSize = 0.035) {
        TLatex latex;
        latex.SetNDC();
        latex.SetTextSize(textSize);
        latex.SetTextFont(52); // Helvetica italic
        latex.DrawLatex(x, y, text.c_str());
    }
    
    void DrawLumiText(double x, double y, const std::string& text, double textSize = 0.035) {
        TLatex latex;
        latex.SetNDC();
        latex.SetTextSize(textSize);
        latex.SetTextFont(42); // Helvetica normal
        latex.SetTextAlign(31); // Right aligned
        latex.DrawLatex(x, y, text.c_str());
    }
    
    void DrawCMSLabels(const PlotOptions& opts) {
        if (!opts.drawCMSLabel) return;
        
        double cmsTextSize = 0.04;
        double extraTextSize = 0.035;
        double lumiTextSize = 0.035;
        
        if (opts.inFrame) {
            // In-frame positioning
            DrawCMSText(0.25, 0.85, opts.cmsText, cmsTextSize);
            if (!opts.cmsExtra.empty()) {
                DrawExtraText(0.25, 0.80, opts.cmsExtra, extraTextSize);
            }
            if (!opts.lumiText.empty()) {
                DrawLumiText(0.95, 0.92, opts.lumiText, lumiTextSize);
            }
        } else {
            // Out-of-frame positioning (default)
            DrawCMSText(0.18, 0.95, opts.cmsText, cmsTextSize);
            if (!opts.cmsExtra.empty()) {
                DrawExtraText(0.30, 0.95, opts.cmsExtra, extraTextSize);
            }
            if (!opts.lumiText.empty()) {
                DrawLumiText(0.95, 0.95, opts.lumiText, lumiTextSize);
            }
        }
    }
}

// Enhanced 1D Histogram Plotting Function
TCanvas* Plot_hist_CMS(const std::vector<TH1*>& histos, 
                       const std::vector<std::string>& labels, 
                       const std::string& canvasName,
                       const PlotOptions& opts = PlotOptions()) {
    
    if (histos.empty()) {
        log(LOG_ERROR, "Plot_hist_CMS: No histograms provided");
        return nullptr;
    }
    
    // Apply CMS style
    CMSStyle::SetCMSStyle();
    
    // Create canvas
    TCanvas* canvas = new TCanvas(canvasName.c_str(), opts.title.c_str(), 
                                 opts.canvasWidth, opts.canvasHeight);
    
    // Get color scheme
    std::vector<int> colors;
    std::vector<int> markers;
    
    switch (opts.colorScheme) {
        case PlotOptions::PETROFF6:
            colors = CMSColors::PetroffColors6;
            break;
        case PlotOptions::PETROFF10:
            colors = CMSColors::PetroffColors10;
            break;
        case PlotOptions::TRADITIONAL:
            colors = CMSColors::TraditionalColors;
            break;
        case PlotOptions::CUSTOM:
            colors = opts.customColors;
            markers = opts.customMarkers;
            break;
    }
    
    if (markers.empty()) {
        markers = CMSColors::MarkerStyles;
    }
    
    // Find maximum for scaling
    double maxVal = 0;
    for (auto* h : histos) {
        if (h && h->GetMaximum() > maxVal) {
            maxVal = h->GetMaximum();
        }
    }
    
    // Configure and draw histograms
    for (size_t i = 0; i < histos.size(); ++i) {
        if (!histos[i]) continue;
        
        auto* h = histos[i];
        
        // Normalize if requested
        if (opts.normalize && h->Integral() > 0) {
            h->Scale(1.0 / h->Integral());
        }
        
        // Set style
        int colorIndex = i % colors.size();
        int markerIndex = i % markers.size();
        
        h->SetLineColor(colors[colorIndex]);
        h->SetMarkerColor(colors[colorIndex]);
        h->SetMarkerStyle(markers[markerIndex]);
        h->SetMarkerSize(1.2);
        h->SetLineWidth(2);
        
        // Set titles
        h->SetTitle("");
        if (!opts.xtitle.empty()) h->GetXaxis()->SetTitle(opts.xtitle.c_str());
        if (!opts.ytitle.empty()) h->GetYaxis()->SetTitle(opts.ytitle.c_str());
        
        // Set ranges
        if (opts.setXRange) h->GetXaxis()->SetRangeUser(opts.xmin, opts.xmax);
        if (opts.setYRange) h->GetYaxis()->SetRangeUser(opts.ymin, opts.ymax);
        
        // Draw
        std::string drawOpt = (i == 0) ? opts.drawOption : opts.drawOption + " same";
        if (drawOpt.empty()) {
            drawOpt = (i == 0) ? "E1" : "E1 same";
        }
        h->Draw(drawOpt.c_str());
    }
    
    // Set log scales
    if (opts.logX) canvas->SetLogx();
    if (opts.logY) canvas->SetLogy();
    
    // Set grid
    if (opts.gridX || opts.gridY) {
        canvas->SetGrid(opts.gridX, opts.gridY);
    }
    
    // Draw legend
    if (opts.drawLegend && !labels.empty()) {
        TLegend* legend = new TLegend(opts.legX1, opts.legY1, opts.legX2, opts.legY2);
        legend->SetFillStyle(0);
        legend->SetBorderSize(0);
        legend->SetTextFont(42);
        legend->SetTextSize(0.035);
        
        if (!opts.legHeader.empty()) {
            legend->SetHeader(opts.legHeader.c_str());
        }
        
        for (size_t i = 0; i < std::min(histos.size(), labels.size()); ++i) {
            if (histos[i]) {
                legend->AddEntry(histos[i], labels[i].c_str(), "lep");
            }
        }
        legend->Draw();
    }
    
    // Draw CMS labels
    CMSLabels::DrawCMSLabels(opts);
    
    // Draw additional text
    for (size_t i = 0; i < opts.additionalText.size() && i < opts.textPositions.size(); ++i) {
        TLatex latex;
        latex.SetNDC();
        latex.SetTextFont(42);
        latex.SetTextSize(0.03);
        latex.DrawLatex(opts.textPositions[i].first, opts.textPositions[i].second, 
                       opts.additionalText[i].c_str());
    }
    
    canvas->Update();
    log(LOG_INFO, "Created CMS-style 1D histogram plot: " + canvasName);
    return canvas;
}

// Enhanced 2D Histogram Plotting Function
TCanvas* Plot_hist2D_CMS(TH2* histo, 
                         const std::string& canvasName,
                         const PlotOptions& opts = PlotOptions()) {
    
    if (!histo) {
        log(LOG_ERROR, "Plot_hist2D_CMS: No histogram provided");
        return nullptr;
    }
    
    // Apply CMS style
    CMSStyle::SetCMSStyle();
    
    // Set viridis colormap for 2D plots if not specified otherwise
    if (opts.drawOption.find("colz") != std::string::npos || 
        opts.drawOption.find("COLZ") != std::string::npos || 
        opts.drawOption.empty()) {
        CMSColors::SetViridisColormap();
    }
    
    // Create canvas
    TCanvas* canvas = new TCanvas(canvasName.c_str(), opts.title.c_str(), 
                                 opts.canvasWidth, opts.canvasHeight);
    
    // Adjust right margin for color palette
    canvas->SetRightMargin(0.15);
    
    // Configure histogram
    histo->SetTitle("");
    if (!opts.xtitle.empty()) histo->GetXaxis()->SetTitle(opts.xtitle.c_str());
    if (!opts.ytitle.empty()) histo->GetYaxis()->SetTitle(opts.ytitle.c_str());
    if (!opts.ztitle.empty()) histo->GetZaxis()->SetTitle(opts.ztitle.c_str());
    
    // Set ranges
    if (opts.setXRange) histo->GetXaxis()->SetRangeUser(opts.xmin, opts.xmax);
    if (opts.setYRange) histo->GetYaxis()->SetRangeUser(opts.ymin, opts.ymax);
    if (opts.setZRange) histo->GetZaxis()->SetRangeUser(opts.zmin, opts.zmax);
    
    // Draw histogram
    std::string drawOpt = opts.drawOption.empty() ? "COLZ" : opts.drawOption;
    histo->Draw(drawOpt.c_str());
    
    // Set log scales
    if (opts.logX) canvas->SetLogx();
    if (opts.logY) canvas->SetLogy();
    if (opts.logZ) canvas->SetLogz();
    
    // Set grid
    if (opts.gridX || opts.gridY) {
        canvas->SetGrid(opts.gridX, opts.gridY);
    }
    
    // Draw CMS labels
    CMSLabels::DrawCMSLabels(opts);
    
    // Draw additional text
    for (size_t i = 0; i < opts.additionalText.size() && i < opts.textPositions.size(); ++i) {
        TLatex latex;
        latex.SetNDC();
        latex.SetTextFont(42);
        latex.SetTextSize(0.03);
        latex.DrawLatex(opts.textPositions[i].first, opts.textPositions[i].second, 
                       opts.additionalText[i].c_str());
    }
    
    canvas->Update();
    log(LOG_INFO, "Created CMS-style 2D histogram plot: " + canvasName);
    return canvas;
}

// Enhanced Multi-Plot Overlay Function
TCanvas* Plot_overlay_CMS(const std::vector<TH1*>& histos,
                          const std::vector<std::string>& labels,
                          const std::string& canvasName,
                          const PlotOptions& opts = PlotOptions()) {
    
    if (histos.empty()) {
        log(LOG_ERROR, "Plot_overlay_CMS: No histograms provided");
        return nullptr;
    }
    
    // Use the enhanced 1D plotting function
    return Plot_hist_CMS(histos, labels, canvasName, opts);
}

// Ratio Plot Function (Data/MC comparison)
TCanvas* Plot_ratio_CMS(TH1* data, TH1* mc, 
                        const std::string& canvasName,
                        const PlotOptions& opts = PlotOptions()) {
    
    if (!data || !mc) {
        log(LOG_ERROR, "Plot_ratio_CMS: Data or MC histogram is null");
        return nullptr;
    }
    
    // Apply CMS style
    CMSStyle::SetCMSStyle();
    
    // Create canvas with split pads
    TCanvas* canvas = new TCanvas(canvasName.c_str(), opts.title.c_str(), 
                                 opts.canvasWidth, opts.canvasHeight);
    
    // Create pads
    TPad* pad1 = new TPad("pad1", "pad1", 0, 0.3, 1, 1.0);
    TPad* pad2 = new TPad("pad2", "pad2", 0, 0.0, 1, 0.3);
    
    pad1->SetBottomMargin(0.02);
    pad2->SetTopMargin(0.02);
    pad2->SetBottomMargin(0.4);
    
    pad1->Draw();
    pad2->Draw();
    
    // Main plot (top pad)
    pad1->cd();
    if (opts.logY) pad1->SetLogy();
    
    // Clone histograms to avoid modifying originals
    TH1* dataCopy = (TH1*)data->Clone();
    TH1* mcCopy = (TH1*)mc->Clone();
    
    // Style data
    dataCopy->SetMarkerColor(kBlack);
    dataCopy->SetMarkerStyle(kFullCircle);
    dataCopy->SetMarkerSize(1.0);
    dataCopy->SetLineColor(kBlack);
    dataCopy->SetLineWidth(2);
    
    // Style MC
    mcCopy->SetLineColor(kRed);
    mcCopy->SetLineWidth(2);
    mcCopy->SetFillColorAlpha(kRed, 0.3);
    
    // Set ranges and titles
    if (opts.setYRange) {
        dataCopy->GetYaxis()->SetRangeUser(opts.ymin, opts.ymax);
    }
    dataCopy->SetTitle("");
    dataCopy->GetYaxis()->SetTitle(opts.ytitle.c_str());
    dataCopy->GetYaxis()->SetTitleSize(0.05);
    dataCopy->GetYaxis()->SetLabelSize(0.04);
    
    // Draw
    mcCopy->Draw("HIST");
    dataCopy->Draw("E1 same");
    
    // Legend
    if (opts.drawLegend) {
        TLegend* legend = new TLegend(0.7, 0.7, 0.9, 0.9);
        legend->SetFillStyle(0);
        legend->SetBorderSize(0);
        legend->AddEntry(dataCopy, "Data", "lep");
        legend->AddEntry(mcCopy, "MC", "f");
        legend->Draw();
    }
    
    // Ratio plot (bottom pad)
    pad2->cd();
    TH1* ratio = (TH1*)dataCopy->Clone("ratio");
    ratio->Divide(mcCopy);
    
    ratio->SetTitle("");
    ratio->GetXaxis()->SetTitle(opts.xtitle.c_str());
    ratio->GetYaxis()->SetTitle("Data/MC");
    ratio->GetXaxis()->SetTitleSize(0.15);
    ratio->GetXaxis()->SetLabelSize(0.12);
    ratio->GetYaxis()->SetTitleSize(0.15);
    ratio->GetYaxis()->SetLabelSize(0.12);
    ratio->GetYaxis()->SetTitleOffset(0.4);
    ratio->GetYaxis()->SetRangeUser(0.5, 1.5);
    ratio->GetYaxis()->SetNdivisions(505);
    
    ratio->SetMarkerColor(kBlack);
    ratio->SetMarkerStyle(kFullCircle);
    ratio->SetMarkerSize(1.0);
    ratio->SetLineColor(kBlack);
    
    ratio->Draw("E1");
    
    // Draw unity line
    TLine* line = new TLine(ratio->GetXaxis()->GetXmin(), 1.0, 
                           ratio->GetXaxis()->GetXmax(), 1.0);
    line->SetLineColor(kRed);
    line->SetLineStyle(2);
    line->Draw();
    
    // Draw CMS labels on main pad
    pad1->cd();
    CMSLabels::DrawCMSLabels(opts);
    
    canvas->cd();
    canvas->Update();
    log(LOG_INFO, "Created CMS-style ratio plot: " + canvasName);
    return canvas;
}

// Convenience function to save canvas in multiple formats
void SaveCanvas_CMS(TCanvas* canvas, const std::string& outputDir, const std::string& name,
                    const std::vector<std::string>& formats = {"png", "pdf", "eps"}) {
    if (!canvas) {
        log(LOG_ERROR, "SaveCanvas_CMS: Canvas is null");
        return;
    }
    
    // Create output directory if it doesn't exist
    gSystem->mkdir(outputDir.c_str(), kTRUE);
    
    for (const auto& format : formats) {
        std::string filename = outputDir + "/" + name + "." + format;
        canvas->SaveAs(filename.c_str());
        log(LOG_DEBUG, "Saved plot: " + filename);
    }
}


// =============================================================================
// PLOTTING CONFIGURATION IMPLEMENTATION
// =============================================================================

// Configuration structures for histogram creation and plotting
struct HistogramConfig {
    // Basic histogram properties
    std::string name = "";
    std::string title = "";
    std::string type = "TH1F";  // TH1F, TH2F, TProfile
    
    // Binning
    int nBinsX = 100;
    double xMin = 0.0;
    double xMax = 1.0;
    int nBinsY = 100;
    double yMin = 0.0;
    double yMax = 1.0;
    
    // Axis labels
    std::string xTitle = "";
    std::string yTitle = "";
    std::string zTitle = "";
    
    // Styling
    int lineColor = 1;
    int lineStyle = 1;
    int lineWidth = 2;
    int markerColor = 1;
    int markerStyle = 20;
    double markerSize = 1.0;
    int fillColor = 0;
    int fillStyle = 0;
    
    // Plot-specific options
    bool normalize = false;
    bool logX = false;
    bool logY = false;
    bool logZ = false;
    std::string drawOption = "";
    
    // Directory structure
    std::vector<std::string> directories;
    
    HistogramConfig() {}
};

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
    int canvasDPI = 300;
    double marginLeft = 0.12;   // Added margin settings
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
    bool showErrorBars = true;  // Added error bar settings
    
    // Histogram configurations
    std::map<std::string, HistogramConfig> histogramConfigs;
    
    // Plotting options
    bool createOverlayPlots = true;
    bool create2DPlots = true;
    bool createProfilePlots = true;
    bool createRatioPlots = false;
    
    PlottingConfiguration() {}
};

// Global plotting configuration definition
PlottingConfiguration g_plotConfig;

/**
 * Helper functions for parsing configuration
 */
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
    auto stringVec = parseStringVector(str, delimiter);
    
    for (const auto& s : stringVec) {
        try {
            result.push_back(std::stod(s));
        } catch (const std::exception& e) {
            log(LOG_ERROR, "Failed to parse double from: " + s);
        }
    }
    return result;
}

/**
 * Load plotting configuration from file path
 * 
 * @param configPath Path to the configuration file
 * @param plotConfig Reference to PlottingConfiguration object to populate
 * @param updateGlobal Whether to update the global g_plotConfig (default: true)
 * @return true if configuration was loaded successfully, false otherwise
 */
bool loadPlottingConfig(const std::string& configPath, PlottingConfiguration& plotConfig, bool updateGlobal = true) {
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
    
    // Parse histogram configurations - look for histogram entries in the format Histogram.Name.Property
    THashList* entries = (THashList*)config->GetTable();
    TIter next(entries);
    TEnvRec* record;
    std::set<std::string> histogramNames;
    
    // First pass - find all the histogram names
    while ((record = (TEnvRec*)next())) {
        std::string name = record->GetName();
        if (name.find("Histogram.") == 0) {
            // Extract the histogram name (e.g., "JetPt" from "Histogram.JetPt.Name")
            size_t firstDot = name.find('.');
            size_t secondDot = name.find('.', firstDot + 1);
            
            if (firstDot != std::string::npos && secondDot != std::string::npos) {
                std::string histName = name.substr(firstDot + 1, secondDot - firstDot - 1);
                histogramNames.insert(histName);
            }
        }
    }
    
    // Second pass - parse the histogram configurations
    log(LOG_DEBUG, "Found " + std::to_string(histogramNames.size()) + " histogram definitions in config");
    
    for (const auto& histName : histogramNames) {
        HistogramConfig histConfig;
        histConfig.name = config->GetValue(("Histogram." + histName + ".Name").c_str(), histName.c_str());
        histConfig.title = config->GetValue(("Histogram." + histName + ".Title").c_str(), "");
        
        // Set histogram type based on PlotType
        std::string plotType = config->GetValue(("Histogram." + histName + ".PlotType").c_str(), "1D");
        if (plotType == "1D") {
            histConfig.type = "TH1F";
        } else if (plotType == "2D") {
            histConfig.type = "TH2F";
        } else if (plotType == "Profile") {
            histConfig.type = "TProfile";
        }
        
        // Binning
        histConfig.nBinsX = config->GetValue(("Histogram." + histName + ".Bins").c_str(), 
                                          config->GetValue(("Histogram." + histName + ".XBins").c_str(), 100));
        histConfig.xMin = config->GetValue(("Histogram." + histName + ".XMin").c_str(), 0.0);
        histConfig.xMax = config->GetValue(("Histogram." + histName + ".XMax").c_str(), 1.0);
        
        // For 2D histograms
        histConfig.nBinsY = config->GetValue(("Histogram." + histName + ".YBins").c_str(), 100);
        histConfig.yMin = config->GetValue(("Histogram." + histName + ".YMin").c_str(), 0.0);
        histConfig.yMax = config->GetValue(("Histogram." + histName + ".YMax").c_str(), 1.0);
        
        // Get title parts for axis labels
        if (histConfig.title.find(';') != std::string::npos) {
            std::string fullTitle = histConfig.title;
            std::vector<std::string> parts;
            std::string part;
            std::istringstream titleStream(fullTitle);
            
            // Parse title;xtitle;ytitle;ztitle format
            std::getline(titleStream, part, ';');
            histConfig.title = part;
            
            if (std::getline(titleStream, part, ';')) {
                histConfig.xTitle = part;
            }
            
            if (std::getline(titleStream, part, ';')) {
                histConfig.yTitle = part;
            }
            
            if (std::getline(titleStream, part, ';')) {
                histConfig.zTitle = part;
            }
        }
        
        // Styling
        std::string colorStr = config->GetValue(("Histogram." + histName + ".Color").c_str(), "auto");
        if (colorStr != "auto") {
            if (colorStr.find('#') == 0) {
                // Handle hex color codes later
                histConfig.lineColor = 1; // Default black for now
            } else {
                // Try to parse as integer
                try {
                    histConfig.lineColor = std::stoi(colorStr);
                } catch (const std::exception&) {
                    histConfig.lineColor = 1;
                }
            }
        } else {
            histConfig.lineColor = 1;  // Default
        }
        
        histConfig.lineStyle = config->GetValue(("Histogram." + histName + ".LineStyle").c_str(), 1);
        histConfig.lineWidth = config->GetValue(("Histogram." + histName + ".LineWidth").c_str(), 2);
        histConfig.markerStyle = config->GetValue(("Histogram." + histName + ".MarkerStyle").c_str(), 20);
        histConfig.markerSize = config->GetValue(("Histogram." + histName + ".MarkerSize").c_str(), 1.0);
        
        // Options
        histConfig.logY = config->GetValue(("Histogram." + histName + ".LogY").c_str(), 0);
        histConfig.logX = config->GetValue(("Histogram." + histName + ".LogX").c_str(), 0);
        histConfig.logZ = config->GetValue(("Histogram." + histName + ".LogZ").c_str(), 0);
        histConfig.normalize = config->GetValue(("Histogram." + histName + ".Normalize").c_str(), 0);
        
        // Add to map
        plotConfig.histogramConfigs[histName] = histConfig;
    }
    
    // Also handle Profile.* entries
    std::set<std::string> profileNames;
    next.Reset();
    while ((record = (TEnvRec*)next())) {
        std::string name = record->GetName();
        if (name.find("Profile.") == 0) {
            size_t firstDot = name.find('.');
            size_t secondDot = name.find('.', firstDot + 1);
            
            if (firstDot != std::string::npos && secondDot != std::string::npos) {
                std::string profName = name.substr(firstDot + 1, secondDot - firstDot - 1);
                profileNames.insert(profName);
            }
        }
    }
    
    // Process profiles
    for (const auto& profName : profileNames) {
        HistogramConfig profConfig;
        profConfig.name = config->GetValue(("Profile." + profName + ".Name").c_str(), profName.c_str());
        profConfig.title = config->GetValue(("Profile." + profName + ".Title").c_str(), "");
        profConfig.type = "TProfile";
        
        // Binning
        profConfig.nBinsX = config->GetValue(("Profile." + profName + ".XBins").c_str(), 100);
        profConfig.xMin = config->GetValue(("Profile." + profName + ".XMin").c_str(), 0.0);
        profConfig.xMax = config->GetValue(("Profile." + profName + ".XMax").c_str(), 1.0);
        profConfig.yMin = config->GetValue(("Profile." + profName + ".YMin").c_str(), 0.0);
        profConfig.yMax = config->GetValue(("Profile." + profName + ".YMax").c_str(), 1.0);
        
        // Add to map
        plotConfig.histogramConfigs[profName] = profConfig;
    }
    
    // Update global configuration if requested
    if (updateGlobal) {
        g_plotConfig = plotConfig;
        log(LOG_DEBUG, "Updated global plotting configuration");
    }
    
    delete config;
    log(LOG_DEBUG, "Successfully loaded plotting configuration from: " + configPath);
    log(LOG_DEBUG, "Loaded " + std::to_string(plotConfig.histogramConfigs.size()) + " histogram configurations");
    return true;
}

/**
 * Parse individual histogram configuration
 */
void parseHistogramConfig(TEnv* config, const std::string& histName, HistogramConfig& histConfig) {
    std::string prefix = histName + ".";
    
    histConfig.name = config->GetValue((prefix + "Name").c_str(), histName.c_str());
    histConfig.title = config->GetValue((prefix + "Title").c_str(), "");
    histConfig.type = config->GetValue((prefix + "Type").c_str(), "TH1F");
    
    // Binning
    histConfig.nBinsX = config->GetValue((prefix + "NBinsX").c_str(), 100);
    histConfig.xMin = config->GetValue((prefix + "XMin").c_str(), 0.0);
    histConfig.xMax = config->GetValue((prefix + "XMax").c_str(), 1.0);
    histConfig.nBinsY = config->GetValue((prefix + "NBinsY").c_str(), 100);
    histConfig.yMin = config->GetValue((prefix + "YMin").c_str(), 0.0);
    histConfig.yMax = config->GetValue((prefix + "YMax").c_str(), 1.0);
    
    // Axis labels
    histConfig.xTitle = config->GetValue((prefix + "XTitle").c_str(), "");
    histConfig.yTitle = config->GetValue((prefix + "YTitle").c_str(), "");
    histConfig.zTitle = config->GetValue((prefix + "ZTitle").c_str(), "");
    
    // Styling
    histConfig.lineColor = config->GetValue((prefix + "LineColor").c_str(), 1);
    histConfig.lineStyle = config->GetValue((prefix + "LineStyle").c_str(), 1);
    histConfig.lineWidth = config->GetValue((prefix + "LineWidth").c_str(), 2);
    histConfig.markerColor = config->GetValue((prefix + "MarkerColor").c_str(), 1);
    histConfig.markerStyle = config->GetValue((prefix + "MarkerStyle").c_str(), 20);
    histConfig.markerSize = config->GetValue((prefix + "MarkerSize").c_str(), 1.0);
    histConfig.fillColor = config->GetValue((prefix + "FillColor").c_str(), 0);
    histConfig.fillStyle = config->GetValue((prefix + "FillStyle").c_str(), 0);
    
    // Options
    histConfig.normalize = config->GetValue((prefix + "Normalize").c_str(), 0);
    histConfig.logX = config->GetValue((prefix + "LogX").c_str(), 0);
    histConfig.logY = config->GetValue((prefix + "LogY").c_str(), 0);
    histConfig.logZ = config->GetValue((prefix + "LogZ").c_str(), 0);
    histConfig.drawOption = config->GetValue((prefix + "DrawOption").c_str(), "");
    
    // Directories
    std::string directoriesStr = config->GetValue((prefix + "Directories").c_str(), "");
    if (!directoriesStr.empty()) {
        histConfig.directories = parseStringVector(directoriesStr);
    }
}

/**
 * Parse histogram configuration from config
 */
HistogramConfig parseHistogramConfig(TEnv* config, const std::string& histName, const std::string& prefix) {
    HistogramConfig histConfig;
    
    if (!config) return histConfig;
    
    std::string baseKey = prefix + "." + histName + ".";
    
    histConfig.name = config->GetValue((baseKey + "Name").c_str(), ("h" + histName).c_str());
    histConfig.title = config->GetValue((baseKey + "Title").c_str(), histName.c_str());
    histConfig.nBinsX = config->GetValue((baseKey + "Bins").c_str(), 50);
    histConfig.xMin = config->GetValue((baseKey + "XMin").c_str(), 0.0);
    histConfig.xMax = config->GetValue((baseKey + "XMax").c_str(), 1.0);
    histConfig.nBinsX = config->GetValue((baseKey + "XBins").c_str(), 50);
    histConfig.nBinsY = config->GetValue((baseKey + "YBins").c_str(), 50);
    histConfig.yMin = config->GetValue((baseKey + "YMin").c_str(), 0.0);
    histConfig.yMax = config->GetValue((baseKey + "YMax").c_str(), 1.0);
    histConfig.logY = config->GetValue((baseKey + "LogY").c_str(), 0);
    histConfig.logZ = config->GetValue((baseKey + "LogZ").c_str(), 0);
    
    // Set type based on PlotType
    std::string plotType = config->GetValue((baseKey + "PlotType").c_str(), "1D");
    if (plotType == "1D") {
        histConfig.type = "TH1F";
    } else if (plotType == "2D") {
        histConfig.type = "TH2F";
    } else if (plotType == "Profile") {
        histConfig.type = "TProfile";
    }
    
    // Handle color
    std::string colorStr = config->GetValue((baseKey + "Color").c_str(), "auto");
    if (colorStr != "auto") {
        try {
            histConfig.lineColor = std::stoi(colorStr);
            histConfig.markerColor = std::stoi(colorStr);
        } catch (const std::exception&) {
            histConfig.lineColor = 1;
            histConfig.markerColor = 1;
        }
    }
    
    histConfig.lineWidth = config->GetValue((baseKey + "LineWidth").c_str(), 2);
    histConfig.lineStyle = config->GetValue((baseKey + "LineStyle").c_str(), 1);
    histConfig.markerStyle = config->GetValue((baseKey + "MarkerStyle").c_str(), 20);
    histConfig.markerSize = config->GetValue((baseKey + "MarkerSize").c_str(), 1.0);
    
    return histConfig;
}

/**
 * Get centrality bin name from bin indices
 */
std::string getCentralityBinName(float minBin, float maxBin) {
    return "cent" + std::to_string(static_cast<int>(minBin)) + "to" + std::to_string(static_cast<int>(maxBin));
}

/**
 * Get automatic color for histogram based on color scheme
 */
int getAutoColor(const std::string& colorScheme, int index) {
    // Define color schemes with visually distinct colors
    if (colorScheme == "petroff6") {
        // A set of 6 colors optimized for data visualization (Petroff palette)
        std::vector<int> colors = {kBlue, kOrange-3, kRed+1, kMagenta+1, kGray+1, kViolet};
        return colors[index % colors.size()];
    } else if (colorScheme == "petroff10") {
        // An extended set of 10 colors based on the Petroff palette
        std::vector<int> colors = {kBlue, kOrange-3, kRed+1, kMagenta+1, kGray+1, kViolet, 
                                  kGreen+2, kCyan+1, kYellow+1, kPink+1};
        return colors[index % colors.size()];
    } else if (colorScheme == "colorblind") {
        // Color-blind friendly palette with 8 colors - more accessible
        std::vector<int> colors = {TColor::GetColor("#0072B2"), // Blue
                                 TColor::GetColor("#E69F00"), // Orange
                                 TColor::GetColor("#009E73"), // Green
                                 TColor::GetColor("#CC79A7"), // Pink
                                 TColor::GetColor("#56B4E9"), // Light blue
                                 TColor::GetColor("#D55E00"), // Vermillion 
                                 TColor::GetColor("#F0E442"), // Yellow
                                 TColor::GetColor("#999999")}; // Gray
        return colors[index % colors.size()];
    } else if (colorScheme == "viridis") {
        // Sequential colors inspired by matplotlib's viridis (approximated in ROOT)
        std::vector<int> colors = {kBlue-7, kBlue-5, kCyan+1, kGreen+2, 
                                 kYellow-3, kYellow+1, kOrange+7};
        return colors[index % colors.size()];
    } else if (colorScheme == "pastel") {
        // Pastel colors for softer visualizations
        std::vector<int> colors = {kAzure-8, kRed-9, kGreen-8, kOrange-9, 
                                 kMagenta-9, kCyan-8, kYellow-7, kBlue-9};
        return colors[index % colors.size()];
    } else {
        // Default ROOT colors
        std::vector<int> colors = {kBlue, kRed, kGreen+1, kMagenta, kCyan, kOrange};
        return colors[index % colors.size()];
    }
}

/**
 * Apply CMS style settings from configuration
 */
void applyCMSStyleFromConfig(const PlottingConfiguration& config) {
    if (config.useCMSStyle) {
        CMSStyle::SetCMSStyle();
        log(LOG_DEBUG, "Applied CMS style from configuration");
    }
}

/**
 * Create output directory structure for plots
 * This function creates the directory structure for storing plots based on the configuration
 * 
 * @param config PlottingConfiguration object with plot settings
 * @param additionalSubdirs Optional vector of additional subdirectories to create
 * @param createJetCollectionDirs Whether to create directories for jet collections (default: true)
 * @param jetCollections Optional vector of jet collection names for creating subdirectories
 * @param centralityBins Optional vector of centrality bins for creating subdirectories
 * @return true if directories were created successfully, false otherwise
 */
bool createPlotDirectories(const PlottingConfiguration& config, 
                          const std::vector<std::string>& additionalSubdirs = {},
                          bool createJetCollectionDirs = true,
                          const std::vector<std::string>& jetCollections = {},
                          const std::vector<float>& centralityBins = {}) {
    if (!config.savePlots) return true;
    
    log(LOG_DEBUG, "Creating plot directories in: " + config.plotOutputDir);
    
    // Create main plot directory
    if (gSystem->AccessPathName(config.plotOutputDir.c_str())) {
        if (gSystem->mkdir(config.plotOutputDir.c_str(), kTRUE) != 0) {
            log(LOG_ERROR, "Failed to create plot output directory: " + config.plotOutputDir);
            return false;
        }
    }
    
    if (config.plotSubdirs) {
        // Default set of subdirectories for different plot types
        std::vector<std::string> subdirs = {"1D", "2D", "profiles", "overlays", "ratios", "event_level", "kinematics"};
        
        // Add user-specified subdirectories
        for (const auto& dir : additionalSubdirs) {
            if (std::find(subdirs.begin(), subdirs.end(), dir) == subdirs.end()) {
                subdirs.push_back(dir);
            }
        }
        
        // Create each subdirectory
        for (const auto& subdir : subdirs) {
            std::string fullPath = config.plotOutputDir + "/" + subdir;
            if (gSystem->AccessPathName(fullPath.c_str())) {
                if (gSystem->mkdir(fullPath.c_str(), kTRUE) != 0) {
                    log(LOG_INFO, "Warning: Failed to create subdirectory: " + fullPath);
                }
            }
            
            // Create jet collection and centrality directories if requested
            if (createJetCollectionDirs && !jetCollections.empty()) {
                for (const auto& jetColl : jetCollections) {
                    std::string jetPath = fullPath + "/" + jetColl;
                    if (gSystem->AccessPathName(jetPath.c_str())) {
                        if (gSystem->mkdir(jetPath.c_str(), kTRUE) != 0) {
                            log(LOG_INFO, "Warning: Failed to create jet collection directory: " + jetPath);
                            continue;
                        }
                    }
                    
                    // Create centrality bin directories if centrality bins are provided
                    if (centralityBins.size() >= 2) {
                        for (size_t i = 0; i < centralityBins.size() - 1; ++i) {
                            std::string centName = "cent" + std::to_string(static_cast<int>(centralityBins[i])) + 
                                                 "to" + std::to_string(static_cast<int>(centralityBins[i+1]));
                            std::string centPath = jetPath + "/" + centName;
                            
                            if (gSystem->AccessPathName(centPath.c_str())) {
                                if (gSystem->mkdir(centPath.c_str(), kTRUE) != 0) {
                                    log(LOG_INFO, "Warning: Failed to create centrality directory: " + centPath);
                                }
                            }
                        }
                    }
                }
            }
            
            // Check if we need to create histogram-specific subdirectories
            for (const auto& histConfig : config.histogramConfigs) {
                if (!histConfig.second.directories.empty()) {
                    for (const auto& histDir : histConfig.second.directories) {
                        std::string histPath = config.plotOutputDir + "/" + subdir + "/" + histDir;
                        if (gSystem->AccessPathName(histPath.c_str())) {
                            if (gSystem->mkdir(histPath.c_str(), kTRUE) != 0) {
                                log(LOG_INFO, "Warning: Failed to create histogram subdirectory: " + histPath);
                            }
                        }
                    }
                }
            }
        }
    }
    
    log(LOG_INFO, "Created plot directory structure in: " + config.plotOutputDir);
    return true;
}

/**
 * Enhanced histogram creation with configuration
 * Creates a 1D histogram using the provided configuration
 * 
 * @param config The histogram configuration to use
 * @param name Optional custom name (overrides config name if provided)
 * @param title Optional custom title (overrides config title if provided)
 * @param colorIndex Optional color index for auto color assignment
 * @return Pointer to the newly created histogram
 */
TH1F* createHistogram1D(const HistogramConfig& config, const std::string& name = "", 
                       const std::string& title = "", int colorIndex = -1) {
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
 * Creates a 2D histogram using the provided configuration
 * 
 * @param config The histogram configuration to use
 * @param name Optional custom name (overrides config name if provided)
 * @param title Optional custom title (overrides config title if provided)
 * @param colorIndex Optional color index for auto color assignment
 * @return Pointer to the newly created histogram
 */
TH2F* createHistogram2D(const HistogramConfig& config, const std::string& name = "", 
                       const std::string& title = "", int colorIndex = -1) {
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
 * Creates a profile histogram using the provided configuration
 * 
 * @param config The histogram configuration to use
 * @param name Optional custom name (overrides config name if provided)
 * @param title Optional custom title (overrides config title if provided)
 * @param colorIndex Optional color index for auto color assignment
 * @return Pointer to the newly created profile histogram
 */
TProfile* createProfile(const HistogramConfig& config, const std::string& name = "", 
                       const std::string& title = "", int colorIndex = -1) {
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

/**
 * Apply histogram styling from configuration
 */
void applyHistogramStyle(TH1* hist, const HistogramConfig& config, int colorIndex = 0) {
    if (!hist) return;
    
    // Set colors based on configuration or auto-assigned from scheme
    int color = config.lineColor;
    if (color <= 0) {
        // Auto-assign color from scheme if no specific color is set
        // Use the global color scheme from configuration if available
        extern PlottingConfiguration g_plotConfig;
        std::string scheme = g_plotConfig.colorScheme.empty() ? "petroff6" : g_plotConfig.colorScheme;
        color = getAutoColor(scheme, colorIndex);
    }
    
    // Apply colors to different parts of the histogram
    hist->SetLineColor(color);
    hist->SetMarkerColor(config.markerColor > 0 ? config.markerColor : color);
    hist->SetFillColor(config.fillColor);
    hist->SetFillStyle(config.fillStyle);
    
    // Set line and marker properties
    hist->SetLineWidth(config.lineWidth);
    hist->SetLineStyle(config.lineStyle);
    hist->SetMarkerStyle(config.markerStyle);
    hist->SetMarkerSize(config.markerSize);
    
    // Apply axis styling with better defaults
    hist->GetXaxis()->SetTitleSize(0.05);
    hist->GetYaxis()->SetTitleSize(0.05);
    hist->GetXaxis()->SetLabelSize(0.04);
    hist->GetYaxis()->SetLabelSize(0.04);
    hist->GetXaxis()->SetTitleOffset(1.0);
    hist->GetYaxis()->SetTitleOffset(1.2);
    
    // Set draw options if specified
    if (!config.drawOption.empty()) {
        hist->SetDrawOption(config.drawOption.c_str());
    }
    
    // Apply normalization if requested
    if (config.normalize && hist->GetEntries() > 0) {
        hist->Scale(1.0 / hist->Integral());
    }
}

/**
 * Create and save plot with CMS styling
 */
void createAndSavePlot(TH1* hist, const PlottingConfiguration& config, const HistogramConfig& histConfig,
                      const std::string& outputName, const std::string& subdirectory) {
    if (!hist || !config.savePlots) return;
    
    // Set up plot options
    PlotOptions options;
    options.canvasWidth = config.canvasWidth;
    options.canvasHeight = config.canvasHeight;
    options.logY = histConfig.logY;
    
    // CMS settings
    options.cmsText = config.cmsLabel;
    options.cmsExtra = config.cmsExtraText;
    options.lumiText = config.cmsEnergyText + " (" + config.cmsLuminosity + ")";
    
    // Create output path
    std::string outputPath = config.plotOutputDir;
    if (!subdirectory.empty() && config.plotSubdirs) {
        outputPath += "/" + subdirectory;
    }
    outputPath += "/" + outputName;
    
    // Use the Plot_hist_CMS function
    std::vector<TH1*> histVec = {hist};
    std::vector<std::string> labels = {hist->GetTitle()};
    Plot_hist_CMS(histVec, labels, outputName, options);
    
    // Save in different formats if specified
    if (!config.plotFormats.empty()) {
        TCanvas* canvas = gPad->GetCanvas();
        if (canvas) {
            for (const auto& format : config.plotFormats) {
                canvas->SaveAs((outputPath + "." + format).c_str());
            }
        }
    }
    
    log(LOG_DEBUG, "Created and saved plot: " + outputPath);
}

/**
 * Create overlay plot with multiple histograms
 */
void createOverlayPlot(const std::vector<TH1*>& histograms, const std::vector<std::string>& labels,
                      const PlottingConfiguration& config, const HistogramConfig& histConfig,
                      const std::string& outputName, const std::string& subdirectory) {
    if (histograms.empty() || !config.savePlots) return;
    
    // Set up plot options
    PlotOptions options;
    options.canvasWidth = config.canvasWidth;
    options.canvasHeight = config.canvasHeight;
    options.marginLeft = config.marginLeft;
    options.marginRight = config.marginRight;
    options.marginTop = config.marginTop;
    options.marginBottom = config.marginBottom;
    options.logY = histConfig.logY;
    options.drawLegend = config.drawLegend;
    options.showErrorBars = config.showErrorBars;
    
    // Set legend options
    options.legX1 = config.legendX1;
    options.legY1 = config.legendY1;
    options.legX2 = config.legendX2;
    options.legY2 = config.legendY2;
    
    // Create output path
    std::string outputPath = config.plotOutputDir;
    if (!subdirectory.empty() && config.plotSubdirs) {
        outputPath += "/" + subdirectory;
    }
    outputPath += "/" + outputName;
    
    // Use the Plot_overlay_CMS function
    Plot_overlay_CMS(histograms, labels, outputName, options);
    
    log(LOG_DEBUG, "Created and saved overlay plot: " + outputPath);
}

/**
 * Create 2D plot with proper styling and colormaps
 */
void create2DPlot(TH2* hist, const PlottingConfiguration& config, const HistogramConfig& histConfig,
                 const std::string& outputName, const std::string& subdirectory) {
    if (!hist || !config.savePlots) return;
    
    // Set up plot options
    PlotOptions options;
    options.canvasWidth = config.canvasWidth;
    options.canvasHeight = config.canvasHeight;
    options.logZ = histConfig.logZ;
    
    // CMS settings
    options.cmsText = config.cmsLabel;
    options.cmsExtra = config.cmsExtraText;
    options.lumiText = config.cmsEnergyText + " (" + config.cmsLuminosity + ")";
    
    // Create output path
    std::string outputPath = config.plotOutputDir;
    if (!subdirectory.empty() && config.plotSubdirs) {
        outputPath += "/" + subdirectory;
    }
    outputPath += "/" + outputName;
    
    // Use the Plot_hist2D_CMS function
    Plot_hist2D_CMS(hist, outputName, options);
    
    log(LOG_DEBUG, "Created and saved 2D plot: " + outputPath);
}

// These functions are already defined above, removing duplicate definitions

// =============================================================================
// END OF PLOTTING CONFIGURATION IMPLEMENTATION
// =============================================================================

#endif
