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

// Logging system
enum LogLevel { LOG_ERROR = 0, LOG_INFO = 1, LOG_DEBUG = 2, LOG_TRACE = 3 };
extern int g_verbosity;

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
struct PlotOptions;

// Complete PlottingConfiguration struct definition
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
    
    // Plotting options
    bool createOverlayPlots = true;
    bool create2DPlots = true;
    bool createProfilePlots = true;
    bool createRatioPlots = false;
    
    // Configuration file reference for MC detection
    TEnv* configFile = nullptr;
    
    PlottingConfiguration() {}
};

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
    std::string system;
    std::string dataType;
    std::string inputDir;
    std::string outputDir;
    std::string outputPrefix;
    double vzCut;
    double hiHFCutMin;
    double hiHFCutMax;
    double photonEtMin;
    double photonEtaMax;
    double photonHoverEMax;
    double photonSigmaIEtaIEtaMax;
    double photonIsoMax;
    double photonR9Min;
    bool mcPhotonMatchRequired;
    int mcPhotonPID;
    std::vector<int> mcPhotonMomPIDs;
    double mcPhotonCalIsoDR04Max;
    double jetPtMin;
    double jetEtaMax;
    double deltaPhiMin;
    double xjMin;
    std::vector<double> centBins;
    std::vector<double> etaBins;
    std::vector<std::string> jetCollections;
    std::vector<std::string> AnalysisCases;
    std::string analysisCases;
    int verbosity;
    bool regenerateHeader;
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
    // Event-level variables
    int run;
    int lumi;
    long long evt;
    int hiBin;
    float vz;
    float hiHF;
    
    // Photon variables
    int nPho;
    std::vector<float> phoEt;
    std::vector<float> phoEta;
    std::vector<float> phoPhi;
    std::vector<float> phoHoverE;
    std::vector<float> phoSigmaIEtaIEta;
    std::vector<float> pho_ecalClusterIsoR4;
    std::vector<float> phoR9;
    std::vector<int> pho_genMatchedIndex;
    
    // MC photon variables (for MC only)
    int nMC;
    std::vector<float> mcEt;
    std::vector<float> mcEta;
    std::vector<float> mcPhi;
    std::vector<int> mcPID;
    std::vector<int> mcMomPID;
    std::vector<float> mcCalIsoDR04;
    
    // Jet variables (multiple collections possible)
    std::map<std::string, int> nJet;
    std::map<std::string, std::vector<float>> jetPt;
    std::map<std::string, std::vector<float>> jetEta;
    std::map<std::string, std::vector<float>> jetPhi;
};

// Helper function to recursively collect .root files
std::vector<std::string> GetFiles(const std::string &dir, int limit = 99999) {
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

// Config loading functions
bool loadConfig(Config& cfg, const std::string& configPath) {
    TEnv config;
    if (config.ReadFile(configPath.c_str(), kEnvGlobal) < 0) {
        std::cerr << "Error: Could not read config file " << configPath << std::endl;
        return false;
    }
    
    // Load all configuration parameters
    cfg.system = config.GetValue("Analysis.System", "");
    cfg.dataType = config.GetValue("Analysis.DataType", "");
    cfg.inputDir = config.GetValue("Analysis.InputDir", "");
    cfg.outputDir = config.GetValue("Analysis.OutputDir", "output");
    cfg.outputPrefix = config.GetValue("Analysis.OutputPrefix", "analysis");
    
    cfg.vzCut = config.GetValue("Selection.VzCut", 15.0);
    cfg.hiHFCutMin = config.GetValue("Selection.HiHFCutMin", 0.0);
    cfg.hiHFCutMax = config.GetValue("Selection.HiHFCutMax", 5000.0);
    
    cfg.photonEtMin = config.GetValue("Selection.PhotonEtMin", 40.0);
    cfg.photonEtaMax = config.GetValue("Selection.PhotonEtaMax", 1.44);
    cfg.photonHoverEMax = config.GetValue("Selection.PhotonHoverEMax", 0.1);
    cfg.photonSigmaIEtaIEtaMax = config.GetValue("Selection.PhotonSigmaIEtaIEtaMax", 0.01);
    cfg.photonIsoMax = config.GetValue("Selection.PhotonIsoMax", 5.0);
    cfg.photonR9Min = config.GetValue("Selection.PhotonR9Min", 0.8);
    
    cfg.mcPhotonMatchRequired = config.GetValue("Selection.MCPhotonMatchRequired", true);
    cfg.mcPhotonPID = config.GetValue("Selection.MCPhotonPID", 22);
    cfg.mcPhotonCalIsoDR04Max = config.GetValue("Selection.MCPhotonCalIsoDR04Max", 5.0);
    
    cfg.jetPtMin = config.GetValue("Selection.JetPtMin", 30.0);
    cfg.jetEtaMax = config.GetValue("Selection.JetEtaMax", 1.6);
    cfg.deltaPhiMin = config.GetValue("Selection.DeltaPhiMin", 7.0 * TMath::Pi() / 8.0);
    cfg.xjMin = config.GetValue("Selection.XjMin", 0.0);
    
    cfg.verbosity = config.GetValue("Analysis.Verbosity", 1);
    cfg.regenerateHeader = config.GetValue("Analysis.RegenerateHeader", false);
    
    // Parse vectors
    std::string centBinsStr = config.GetValue("Analysis.CentralityBins", "");
    if (!centBinsStr.empty()) {
        cfg.centBins = parseVector(centBinsStr);
    }
    
    std::string etaBinsStr = config.GetValue("Analysis.EtaBins", "");
    if (!etaBinsStr.empty()) {
        cfg.etaBins = parseVector(etaBinsStr);
    }
    
    std::string jetCollectionsStr = config.GetValue("Analysis.JetCollections", "");
    if (!jetCollectionsStr.empty()) {
        cfg.jetCollections = parseStringVector(jetCollectionsStr);
    }
    
    return true;
}

bool loadHistConfig(std::map<std::string, HistConfig>& histConfigs, const std::string& histConfigPath) {
    TEnv config;
    if (config.ReadFile(histConfigPath.c_str(), kEnvGlobal) < 0) {
        std::cerr << "Error: Could not read histogram config file " << histConfigPath << std::endl;
        return false;
    }
    
    // Set global ROOT style
    gStyle->SetOptStat(0);
    if (config.GetValue("Global.CMS_Style", 1)) {
        gStyle->SetPadLeftMargin(0.16);
        gStyle->SetPadRightMargin(0.10);
        gStyle->SetPadTopMargin(0.08);
        gStyle->SetPadBottomMargin(0.12);
        gStyle->SetTitleOffset(1.2, "X");
        gStyle->SetTitleOffset(1.6, "Y");
    }
    
    // Load histogram configurations
    const char* histTypes[] = {"PhotonEt", "JetPt", "DeltaPhi"};
    for (const auto& hist : histTypes) {
        HistConfig hcfg;
        hcfg.title = config.GetValue(Form("%s.Title", hist), "");
        hcfg.bins = config.GetValue(Form("%s.Bins", hist), 50);
        hcfg.range_min = config.GetValue(Form("%s.RangeMin", hist), 0.0);
        hcfg.range_max = config.GetValue(Form("%s.RangeMax", hist), 100.0);
        hcfg.y_log = config.GetValue(Form("%s.YLog", hist), false);
        
        histConfigs[hist] = hcfg;
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
        std::cerr << "Unsupported system: " << cfg.system << std::endl;
        return;
    }
    
    if (headerPath.empty()) {
        std::cerr << "Could not find header file for class " << className << std::endl;
        return;
    }
    
    std::cout << "[LOG] Parsing header file: " << headerPath << " for class: " << className << std::endl;
    
    // Parse the header file to extract member variables
    std::vector<ClassMember> allMembers = parseHeaderFile(headerPath, className);
    
    if (allMembers.empty()) {
        std::cerr << "No members found in header file." << std::endl;
        return;
    }
    
    // Separate members into categories
    std::vector<ClassMember> photonVecMembers, photonIntMembers, jetArrayMembers;
    
    for (const auto& member : allMembers) {
        if (member.type.find("ROOT::VecOps::RVec") != std::string::npos) {
            photonVecMembers.push_back(member);
        } else if (member.isArray) {
            jetArrayMembers.push_back(member);
        } else {
            photonIntMembers.push_back(member);
        }
    }
    
    // Generate the structure content
    auto [typeAliases, structContent] = generateStructureContent(cfg, className, 
                                                                photonVecMembers, 
                                                                photonIntMembers, 
                                                                jetArrayMembers);
    
    // Write the photonJet.h file
    std::string outputPath = "./include/photonJet.h";
    
    // Check if file exists to determine update vs create
    std::ifstream existingFile(outputPath);
    std::string existingContent;
    if (existingFile.good()) {
        std::stringstream buffer;
        buffer << existingFile.rdbuf();
        existingContent = buffer.str();
        existingFile.close();
    }
    
    std::ofstream headerFile(outputPath);
    if (!headerFile.is_open()) {
        std::cerr << "Could not create photonJet.h file." << std::endl;
        return;
    }
    
    if (!existingContent.empty()) {
        updateExistingHeader(headerFile, existingContent, cfg, className, typeAliases, structContent);
    } else {
        createNewHeader(headerFile, cfg, className, typeAliases, structContent);
    }
    
    headerFile.close();
    std::cout << "[LOG] Generated photonJet.h successfully." << std::endl;
}

// Helper function implementations for header generation
// Split the structure content generation into two parts: type aliases and struct definition
std::pair<std::string, std::string> generateStructureContent(const Config& cfg, const std::string& className, 
                                   const std::vector<ClassMember>& photonVecMembers,
                                   const std::vector<ClassMember>& photonIntMembers,
                                   const std::vector<ClassMember>& jetArrayMembers) {
    
    std::ostringstream typeAliases;
    std::ostringstream structContent;
    
    // Generate type aliases
    typeAliases << "// Type aliases for ROOT vectors\n";
    typeAliases << "using VecF = ROOT::VecOps::RVec<float>;\n";
    typeAliases << "using VecI = ROOT::VecOps::RVec<int>;\n";
    typeAliases << "using VecD = ROOT::VecOps::RVec<double>;\n\n";
    
    // Start struct definition
    structContent << "struct PhotonJetEvent {\n";
    structContent << "    // Event-level variables\n";
    structContent << "    int run, lumi, hiBin;\n";
    structContent << "    long long evt;\n";
    structContent << "    float vz, hiHF;\n\n";
    
    // Add photon integer members
    if (!photonIntMembers.empty()) {
        structContent << "    // Photon integer variables\n";
        for (const auto& member : photonIntMembers) {
            structContent << "    " << member.type << " " << member.name << ";\n";
        }
        structContent << "\n";
    }
    
    // Add photon vector members
    if (!photonVecMembers.empty()) {
        structContent << "    // Photon vector variables\n";
        for (const auto& member : photonVecMembers) {
            if (member.type.find("float") != std::string::npos) {
                structContent << "    VecF* " << member.name << ";\n";
            } else if (member.type.find("int") != std::string::npos) {
                structContent << "    VecI* " << member.name << ";\n";
            }
        }
        structContent << "\n";
    }
    
    // Add jet collections
    for (const auto& jetCollection : cfg.jetCollections) {
        structContent << "    // " << jetCollection << " jet variables\n";
        structContent << "    int n" << jetCollection << ";\n";
        
        for (const auto& member : jetArrayMembers) {
            std::string jetVarName = jetCollection + "_" + member.name;
            if (member.arraySize > 0) {
                structContent << "    " << member.type << " " << jetVarName 
                             << "[" << member.arraySize << "];\n";
            } else {
                structContent << "    std::vector<" << member.type << "> " << jetVarName << ";\n";
            }
        }
        structContent << "\n";
    }
    
    // Add constructor
    structContent << "    // Constructor\n";
    structContent << "    PhotonJetEvent() {\n";
    structContent << "        // Initialize vectors\n";
    for (const auto& member : photonVecMembers) {
        structContent << "        " << member.name << " = nullptr;\n";
    }
    structContent << "    }\n\n";
    
    // Add destructor
    structContent << "    // Destructor\n";
    structContent << "    ~PhotonJetEvent() {\n";
    structContent << "        // Clean up vectors\n";
    for (const auto& member : photonVecMembers) {
        structContent << "        delete " << member.name << ";\n";
    }
    structContent << "    }\n";
    
    structContent << "};\n";
    
    return std::make_pair(typeAliases.str(), structContent.str());
}

void updateExistingHeader(std::ofstream& headerFile, const std::string& existingContent,
                         const Config& cfg, const std::string& className, 
                         const std::string& typeAliases, const std::string& structContent) {
    
    // Mark unused parameters to avoid compiler warnings
    (void)cfg;
    (void)className;
    
    // Find the struct definition in existing content
    size_t structStart = existingContent.find("struct PhotonJetEvent");
    
    if (structStart != std::string::npos) {
        // Write everything before the struct
        headerFile << existingContent.substr(0, structStart);
        
        // Write new type aliases and struct
        headerFile << typeAliases << structContent;
        
        // Find the end of the old struct and write everything after
        size_t structEnd = existingContent.find("};", structStart);
        if (structEnd != std::string::npos) {
            structEnd = existingContent.find("\n", structEnd) + 1;
            headerFile << existingContent.substr(structEnd);
        }
    } else {
        // No existing struct found, append to end
        headerFile << existingContent;
        if (!existingContent.empty() && existingContent.back() != '\n') {
            headerFile << "\n";
        }
        headerFile << typeAliases << structContent;
    }
}

void createNewHeader(std::ofstream& headerFile, const Config& cfg, const std::string& className,
                    const std::string& typeAliases, const std::string& structContent) {
    
    headerFile << "#ifndef PHOTONJET_H\n";
    headerFile << "#define PHOTONJET_H\n\n";
    headerFile << "#include <ROOT/RVec.hxx>\n";
    headerFile << "#include <vector>\n\n";
    headerFile << "// Auto-generated header for " << className << "\n";
    headerFile << "// Generated from config: " << cfg.system << " " << cfg.dataType << "\n\n";
    headerFile << typeAliases;
    headerFile << structContent;
    headerFile << "\n#endif // PHOTONJET_H\n";
}

// Plotting and histogram functions
struct PlotOptions {
    std::string title;
    std::string xtitle;
    std::string ytitle;
    double xmin, xmax;
    double ymin, ymax;
    bool logy;
    bool logx;
    std::string drawOption;
    std::vector<int> colors;
    std::vector<int> markers;
    std::vector<int> lineStyles;
    bool showLegend;
    std::string legendPosition;
    std::string cmsText;
    std::string lumiText;
    int canvasWidth;
    int canvasHeight;
    
    PlotOptions() : 
        xmin(0), xmax(0), ymin(0), ymax(0), 
        logy(false), logx(false), drawOption(""),
        showLegend(true), legendPosition("topright"),
        cmsText("CMS Preliminary"), lumiText(""),
        canvasWidth(800), canvasHeight(600) {}
};

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
    int lineStyle;
    double lineWidth;
    std::string drawOption;
    
    HistogramConfig() : 
        name(""), title(""), type("TH1F"),
        xTitle(""), yTitle(""), zTitle(""),
        nBinsX(100), xMin(0), xMax(100),
        nBinsY(100), yMin(0), yMax(100),
        color(1), marker(20), lineStyle(1), lineWidth(2), drawOption("") {}
};

// M. Petroff color schemes
std::vector<int> getPetroffColors(const std::string& scheme = "default") {
    if (scheme == "qualitative") {
        return {862, 834, 838, 866, 870, 874, 878, 882};
    } else if (scheme == "sequential") {
        return {51, 52, 53, 54, 55, 56, 57, 58, 59};
    } else {
        // Default scheme
        return {634, 628, 807, 823, 419, 602, 921, 622};
    }
}

int getAutoColor(const std::string& colorScheme, int index) {
    std::vector<int> colors = getPetroffColors(colorScheme);
    return colors[index % colors.size()];
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

void applyHistogramStyle(TH1* hist, const HistogramConfig& config, int colorIndex) {
    if (!hist) return;
    
    hist->SetLineColor(config.color > 0 ? config.color : getAutoColor("default", colorIndex));
    hist->SetMarkerColor(config.color > 0 ? config.color : getAutoColor("default", colorIndex));
    hist->SetMarkerStyle(config.marker);
    hist->SetLineStyle(config.lineStyle);
    hist->SetLineWidth(config.lineWidth);
}

// CMS plotting functions
TCanvas* Plot_hist_CMS(const std::vector<TH1*>& histos, 
                      const std::vector<std::string>& labels,
                      const std::string& canvasName, 
                      const PlotOptions& opts) {
    
    TCanvas* canvas = new TCanvas(canvasName.c_str(), opts.title.c_str(), 
                                 opts.canvasWidth, opts.canvasHeight);
    canvas->SetLeftMargin(0.16);
    canvas->SetRightMargin(0.04);
    canvas->SetTopMargin(0.08);
    canvas->SetBottomMargin(0.12);
    
    if (opts.logy) canvas->SetLogy();
    if (opts.logx) canvas->SetLogx();
    
    // Find y-range if not specified
    double ymin = opts.ymin, ymax = opts.ymax;
    if (ymin == ymax) {
        ymin = 1e10; ymax = -1e10;
        for (auto hist : histos) {
            if (!hist) continue;
            double hmin = hist->GetMinimum();
            double hmax = hist->GetMaximum();
            if (hmin < ymin) ymin = hmin;
            if (hmax > ymax) ymax = hmax;
        }
        if (opts.logy && ymin <= 0) ymin = 0.001;
        ymax *= opts.logy ? 10 : 1.2;
        ymin *= opts.logy ? 0.5 : 0.9;
    }
    
    bool first = true;
    for (size_t i = 0; i < histos.size(); ++i) {
        if (!histos[i]) continue;
        
        HistogramConfig hcfg;
        hcfg.color = getAutoColor("default", i);
        applyHistogramStyle(histos[i], hcfg, i);
        
        histos[i]->SetTitle("");
        histos[i]->GetXaxis()->SetTitle(opts.xtitle.c_str());
        histos[i]->GetYaxis()->SetTitle(opts.ytitle.c_str());
        histos[i]->GetYaxis()->SetRangeUser(ymin, ymax);
        
        std::string drawOpt = opts.drawOption;
        if (!first) drawOpt += " SAME";
        histos[i]->Draw(drawOpt.c_str());
        first = false;
    }
    
    // Add legend if requested
    if (opts.showLegend && !labels.empty()) {
        TLegend* leg = nullptr;
        if (opts.legendPosition == "topright") {
            leg = new TLegend(0.65, 0.70, 0.93, 0.88);
        } else if (opts.legendPosition == "topleft") {
            leg = new TLegend(0.20, 0.70, 0.48, 0.88);
        } else {
            leg = new TLegend(0.65, 0.70, 0.93, 0.88);
        }
        
        leg->SetBorderSize(0);
        leg->SetFillStyle(0);
        
        for (size_t i = 0; i < std::min(histos.size(), labels.size()); ++i) {
            if (histos[i]) {
                leg->AddEntry(histos[i], labels[i].c_str(), "LP");
            }
        }
        leg->Draw();
    }
    
    // Add CMS text
    if (!opts.cmsText.empty()) {
        TLatex* cmsText = new TLatex(0.18, 0.92, opts.cmsText.c_str());
        cmsText->SetNDC();
        cmsText->SetTextFont(62);
        cmsText->SetTextSize(0.045);
        cmsText->Draw();
    }
    
    if (!opts.lumiText.empty()) {
        TLatex* lumiText = new TLatex(0.95, 0.92, opts.lumiText.c_str());
        lumiText->SetNDC();
        lumiText->SetTextAlign(31);
        lumiText->SetTextFont(42);
        lumiText->SetTextSize(0.04);
        lumiText->Draw();
    }
    
    canvas->Update();
    return canvas;
}

TCanvas* Plot_hist2D_CMS(TH2* histo,
                        const std::string& canvasName,
                        const PlotOptions& opts) {
    
    if (!histo) return nullptr;
    
    TCanvas* canvas = new TCanvas(canvasName.c_str(), opts.title.c_str(), 
                                 opts.canvasWidth, opts.canvasHeight);
    canvas->SetLeftMargin(0.16);
    canvas->SetRightMargin(0.16);
    canvas->SetTopMargin(0.08);
    canvas->SetBottomMargin(0.12);
    
    if (opts.logx) canvas->SetLogx();
    if (opts.logy) canvas->SetLogy();
    
    histo->SetTitle("");
    histo->GetXaxis()->SetTitle(opts.xtitle.c_str());
    histo->GetYaxis()->SetTitle(opts.ytitle.c_str());
    
    std::string drawOpt = opts.drawOption.empty() ? "COLZ" : opts.drawOption;
    histo->Draw(drawOpt.c_str());
    
    // Add CMS text
    if (!opts.cmsText.empty()) {
        TLatex* cmsText = new TLatex(0.18, 0.92, opts.cmsText.c_str());
        cmsText->SetNDC();
        cmsText->SetTextFont(62);
        cmsText->SetTextSize(0.045);
        cmsText->Draw();
    }
    
    if (!opts.lumiText.empty()) {
        TLatex* lumiText = new TLatex(0.95, 0.92, opts.lumiText.c_str());
        lumiText->SetNDC();
        lumiText->SetTextAlign(31);
        lumiText->SetTextFont(42);
        lumiText->SetTextSize(0.04);
        lumiText->Draw();
    }
    
    canvas->Update();
    return canvas;
}

TCanvas* Plot_overlay_CMS(const std::vector<TH1*>& histos,
                         const std::vector<std::string>& labels,
                         const std::string& canvasName,
                         const PlotOptions& opts) {
    // This is essentially the same as Plot_hist_CMS but with different styling
    return Plot_hist_CMS(histos, labels, canvasName, opts);
}

TCanvas* Plot_ratio_CMS(TH1* data, TH1* mc, 
                       const std::string& dataLabel,
                       const std::string& mcLabel,
                       const std::string& canvasName,
                       const PlotOptions& opts) {
    
    if (!data || !mc) return nullptr;
    
    TCanvas* canvas = new TCanvas(canvasName.c_str(), opts.title.c_str(), 
                                 opts.canvasWidth, opts.canvasHeight);
    
    // Create two pads
    TPad* pad1 = new TPad("pad1", "pad1", 0, 0.3, 1, 1.0);
    pad1->SetBottomMargin(0);
    pad1->SetLeftMargin(0.16);
    pad1->SetRightMargin(0.04);
    pad1->Draw();
    
    TPad* pad2 = new TPad("pad2", "pad2", 0, 0.05, 1, 0.3);
    pad2->SetTopMargin(0);
    pad2->SetBottomMargin(0.4);
    pad2->SetLeftMargin(0.16);
    pad2->SetRightMargin(0.04);
    pad2->Draw();
    
    // Upper pad - data and MC
    pad1->cd();
    if (opts.logy) pad1->SetLogy();
    
    TH1* dataClone = (TH1*)data->Clone("dataClone");
    TH1* mcClone = (TH1*)mc->Clone("mcClone");
    
    dataClone->SetMarkerStyle(20);
    dataClone->SetMarkerColor(kBlack);
    dataClone->SetLineColor(kBlack);
    
    mcClone->SetLineColor(kRed);
    mcClone->SetLineWidth(2);
    
    // Find max for y-range
    double maxData = dataClone->GetMaximum();
    double maxMC = mcClone->GetMaximum();
    double ymax = std::max(maxData, maxMC) * (opts.logy ? 10 : 1.3);
    
    mcClone->GetYaxis()->SetRangeUser(opts.logy ? 0.001 : 0, ymax);
    mcClone->SetTitle("");
    mcClone->GetXaxis()->SetLabelSize(0);
    mcClone->GetYaxis()->SetTitle(opts.ytitle.c_str());
    mcClone->Draw("HIST");
    dataClone->Draw("E SAME");
    
    // Legend
    TLegend* leg = new TLegend(0.65, 0.70, 0.93, 0.88);
    leg->SetBorderSize(0);
    leg->SetFillStyle(0);
    leg->AddEntry(dataClone, dataLabel.c_str(), "LP");
    leg->AddEntry(mcClone, mcLabel.c_str(), "L");
    leg->Draw();
    
    // CMS text
    if (!opts.cmsText.empty()) {
        TLatex* cmsText = new TLatex(0.18, 0.85, opts.cmsText.c_str());
        cmsText->SetNDC();
        cmsText->SetTextFont(62);
        cmsText->SetTextSize(0.055);
        cmsText->Draw();
    }
    
    // Lower pad - ratio
    pad2->cd();
    TH1* ratio = (TH1*)dataClone->Clone("ratio");
    ratio->Divide(mcClone);
    ratio->SetMarkerStyle(20);
    ratio->SetMarkerColor(kBlack);
    ratio->SetLineColor(kBlack);
    ratio->SetTitle("");
    ratio->GetXaxis()->SetTitle(opts.xtitle.c_str());
    ratio->GetYaxis()->SetTitle("Data/MC");
    ratio->GetYaxis()->SetRangeUser(0.5, 1.5);
    ratio->GetXaxis()->SetLabelSize(0.1);
    ratio->GetXaxis()->SetTitleSize(0.12);
    ratio->GetYaxis()->SetLabelSize(0.1);
    ratio->GetYaxis()->SetTitleSize(0.1);
    ratio->GetYaxis()->SetTitleOffset(0.5);
    ratio->Draw("E");
    
    // Reference line at 1
    TLine* line = new TLine(ratio->GetXaxis()->GetXmin(), 1, 
                           ratio->GetXaxis()->GetXmax(), 1);
    line->SetLineStyle(2);
    line->SetLineColor(kRed);
    line->Draw();
    
    canvas->Update();
    return canvas;
}

// Global plotting configuration definition
PlottingConfiguration g_plotConfig;

/**
 * Load plotting configuration from file
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

// Global verbosity variable
int g_verbosity = 1;

#endif
