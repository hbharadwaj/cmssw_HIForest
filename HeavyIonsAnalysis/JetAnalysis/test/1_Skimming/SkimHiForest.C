// ROOT includes
#include <TChain.h>
#include <TSystemDirectory.h>
#include <TSystemFile.h>
#include <TEnv.h>
#include <THashList.h>
#include <TObjString.h>
#include <TROOT.h>
#include <TSystem.h>
#include <ROOT/RDataFrame.hxx>

// C++ Standard Library includes
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <string>

// System includes
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

using namespace ROOT;

// Helper function to check if directory exists
bool dirExists(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        return false; // Cannot access
    }
    return (info.st_mode & S_IFDIR); // Is a directory
}

// Helper function to create directories recursively
bool createDirectories(const std::string& path) {
    std::string current_path = "";
    std::string remaining = path;
    
    // Handle absolute paths
    if (path.size() > 0 && path[0] == '/') {
        current_path = "/";
        remaining = remaining.substr(1);
    }
    
    size_t pos = 0;
    while ((pos = remaining.find('/')) != std::string::npos) {
        std::string dir = remaining.substr(0, pos);
        if (!dir.empty()) {
            current_path += dir;
            if (!dirExists(current_path)) {
                int result = mkdir(current_path.c_str(), 0775);
                if (result != 0 && errno != EEXIST) {
                    std::cerr << "[ERROR] Failed to create directory: " << current_path 
                              << " (errno: " << errno << " - " << strerror(errno) << ")" << std::endl;
                    return false;
                }
            }
            current_path += "/";
        }
        remaining = remaining.substr(pos + 1);
    }
    
    // Handle final part
    if (!remaining.empty()) {
        current_path += remaining;
        if (!dirExists(current_path)) {
            int result = mkdir(current_path.c_str(), 0775);
            if (result != 0 && errno != EEXIST) {
                std::cerr << "[ERROR] Failed to create directory: " << current_path 
                          << " (errno: " << errno << " - " << strerror(errno) << ")" << std::endl;
                return false;
            }
        }
    }
    
    return true;
}

// Add verbose as static to make it accessible in ParseBranchSelection
static int verbose = 1;

// Helper function to recursively collect .root files
std::vector<std::string> GetFiles(const std::string &dir, int limit) {
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

// Structure to hold batches of files
struct FileBatches {
    std::vector<std::vector<std::string>> batches;
    size_t totalFiles = 0;
    size_t totalBatches = 0;
    
    // Create batches from a flat list of files
    void createBatches(const std::vector<std::string>& files, size_t filesPerBatch) {
        batches.clear();
        totalFiles = files.size();
        
        if (filesPerBatch <= 0) filesPerBatch = 1;
        totalBatches = (totalFiles + filesPerBatch - 1) / filesPerBatch; // Ceiling division
        
        batches.resize(totalBatches);
        
        for (size_t i = 0; i < totalFiles; i++) {
            size_t batchIndex = i / filesPerBatch;
            if (batchIndex >= totalBatches) break; // Safety check
            batches[batchIndex].push_back(files[i]);
        }
    }
    
    // Get a specific batch by index (0-based)
    std::vector<std::string> getBatch(size_t batchIndex) const {
        if (batchIndex >= totalBatches) {
            std::cerr << "[ERROR] Batch index " << batchIndex << " out of range (max: " 
                      << (totalBatches > 0 ? totalBatches - 1 : 0) << ")" << std::endl;
            return {};
        }
        return batches[batchIndex];
    }
};

// Progress monitoring
void displayProgress(long current, long max) {
    if (max < 100) return;
    if (current % (max / 100) != 0 && current < max - 1) return;

    float progress = (float)current / max;
    int barWidth = 70;
    std::cout << "[";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << "%\r";
    std::cout.flush();
}

struct BranchConfig {
    std::string treePath;    // Original tree path
    std::string treeAlias;   // Tree alias
    std::vector<std::string> branches; // Selected branches
    std::map<std::string, std::string> branchAliases; // branch -> alias mapping
};

std::vector<BranchConfig> ParseBranchSelection(TEnv& env, const std::map<std::string, std::string>& treeMap) {
    std::vector<BranchConfig> configs;
    std::map<std::string, BranchConfig> treeConfigs;
    
    // First initialize treeConfigs from the treeMap
    for (const auto& [path, alias] : treeMap) {
        BranchConfig cfg;
        cfg.treePath = path;
        cfg.treeAlias = alias;
        treeConfigs[path] = cfg;
    }

    // Get all keys from config and parse branch selections
    THashList* keys = (THashList*)env.GetTable();
    for (auto key : *keys) {
        std::string keyName = ((TObjString*)key)->GetString().Data();
        if (keyName.find("Branches_") == 0) {
            std::string treePath = keyName.substr(9); // Remove "Branches_"
            if (treeConfigs.find(treePath) != treeConfigs.end()) {
                std::string branchList = env.GetValue(keyName.c_str(), "");
                std::istringstream ss(branchList);
                std::string branch;
                while (std::getline(ss, branch, ',')) {
                    branch.erase(std::remove_if(branch.begin(), branch.end(), ::isspace), branch.end());
                    if (!branch.empty()) {
                        treeConfigs[treePath].branches.push_back(branch);
                        std::string alias = treeConfigs[treePath].treeAlias + "." + branch;
                        treeConfigs[treePath].branchAliases[branch] = alias;
                        if (verbose > 1) {
                            std::cout << "[DEBUG] Added branch: " << branch << " -> " << alias << std::endl;
                        }
                    }
                }
            }
        }
    }

    // Convert map to vector
    for (const auto& pair : treeConfigs) {
        if (verbose > 0) {
            std::cout << "[INFO] Tree: " << pair.first << " has " << pair.second.branches.size() << " branches" << std::endl;
        }
        configs.push_back(pair.second);
    }

    return configs;
}

// Function to process a single batch of files
void ProcessBatch(const TEnv& env, const std::vector<std::string>& batchFiles, 
                 const std::string& outputDir, const std::string& outName, 
                 int batchIndex, int totalBatches) {
    
    if (batchFiles.empty()) {
        std::cerr << "[ERROR] Batch " << batchIndex << " is empty" << std::endl;
        return;
    }
    
    auto startTime = std::chrono::steady_clock::now();
    
    if (verbose > 0) {
        std::cout << "[INFO] Batch " << batchIndex << "/" << totalBatches-1 
                 << ": Processing " << batchFiles.size() << " files" << std::endl;
        std::cout << "[INFO] Output: " << outputDir << "/" << outName << ".root" << std::endl;
    }
    
    // --- 3) Build chains and tree mapping ---
    std::map<std::string, std::string> treeMap;  // Store mapping for branch selection
    std::map<std::string, std::string> treeAliases;
    std::string treesStr = env.GetValue("Trees", "");
    std::istringstream treeStream(treesStr);
    std::string tree;
    while (std::getline(treeStream, tree, ' ')) {
        if (tree.empty()) continue;
        size_t colonPos = tree.find(':');
        if (colonPos != std::string::npos) {
            std::string path = tree.substr(0, colonPos);
            std::string alias = tree.substr(colonPos + 1);
            treeAliases[path] = alias;
            treeMap[path] = alias;  // Store mapping for later use
        }
    }

    // Base chain - make sure we use HiTree as base
    std::string hiEvtPath = "hiEvtAnalyzer/HiTree";
    TChain *base = new TChain(hiEvtPath.c_str());
    for (const auto &file : batchFiles) {
        base->Add(file.c_str());
    }

    // Add friends with proper aliases
    std::vector<TChain*> friends;
    for (const auto& [path, alias] : treeAliases) {
        if (path == hiEvtPath) continue; // Skip base tree
        
        TChain *friend_chain = new TChain(path.c_str());
        for (const auto &file : batchFiles) {
            friend_chain->Add(file.c_str());
        }
        // Debug friend chain content
        if (verbose > 1) {
            std::cout << "[DEBUG] Adding friend tree " << path << " as " << alias 
                     << " with " << friend_chain->GetEntries() << " entries" << std::endl;
        }
        base->AddFriend(friend_chain, alias.c_str());
        friends.push_back(friend_chain);
    }

    // --- 4) Create RDataFrame ---
    RDataFrame df(*base);

    // Debug available columns
    if (verbose > 1) {
        auto cols = df.GetColumnNames();
        std::cout << "\n[DEBUG] Available RDF columns after friend addition:" << std::endl;
        for (const auto& col : cols) {
            std::cout << " - " << col << std::endl;
        }
    }

    // --- 5) Configure output ---
    if (!dirExists(outputDir)) {
        createDirectories(outputDir);
    }
    std::string outFile = outputDir + "/" + outName + ".root";
    
    // --- 6) Process and save ---
    // Get available columns from RDataFrame
    auto colNames = df.GetColumnNames();
    std::set<std::string> colSet(colNames.begin(), colNames.end());

    // Debug output of available columns
    if (verbose > 1) {
        std::cout << "\n[DEBUG] Available columns in RDataFrame:" << std::endl;
        for (const auto& col : colNames) {
            std::cout << col << std::endl;
        }
    }

    // Debug: Print tree configurations
    if (verbose > 1) {
        std::cout << "\n[DEBUG] Tree configurations from config:" << std::endl;
        for (const auto& [path, alias] : treeMap) {
            std::cout << "Tree path: " << path << " -> Alias: " << alias << std::endl;
        }
    }

    // Parse branch configurations and prepare output columns
    auto branchConfigs = ParseBranchSelection(const_cast<TEnv&>(env), treeMap);
    if (verbose > 1) {
        std::cout << "\n[DEBUG] Branch configurations:" << std::endl;
        for (const auto& cfg : branchConfigs) {
            std::cout << "Tree: " << cfg.treePath << " -> " << cfg.treeAlias << std::endl;
            std::cout << "Requested branches:" << std::endl;
            for (const auto& branch : cfg.branches) {
                std::cout << "  " << branch << " -> " << cfg.branchAliases.at(branch) << std::endl;
            }
        }
    }

    // Debug: Print branch search attempts
    std::vector<std::string> outCols;
    std::vector<std::string> missingCols;

    if (verbose > 1) {
        std::cout << "\n[DEBUG] Branch search attempts:" << std::endl;
    }

    // Process each branch configuration
    for (const auto& cfg : branchConfigs) {
        for (const auto& branch : cfg.branches) {
            std::string fullName = cfg.treeAlias + "." + branch;
            std::string aliasName = cfg.branchAliases.at(branch);
            
            // Try branch names based on tree type
            bool found = false;
            if (cfg.treePath == "hiEvtAnalyzer/HiTree") {
                // Base tree branches have no prefix
                if (colSet.count(branch)) {
                    outCols.push_back(branch);
                    found = true;
                    if (verbose > 1) std::cout << "Found base branch: " << branch << std::endl;
                }
            } else {
                // Friend trees need full prefixed name
                if (colSet.count(fullName)) {
                    outCols.push_back(fullName);
                    found = true;
                    if (verbose > 1) std::cout << "Found friend branch: " << fullName << std::endl;
                }
            }

            if (!found) {
                missingCols.push_back(aliasName);
                if (verbose > 1) std::cout << "Missing: " << aliasName << std::endl;
            }
        }
    }

    // Calculate total branches from all trees
    size_t totalBranches = 0;
    for (const auto& cfg : branchConfigs) {
        totalBranches += cfg.branches.size();
    }

    // Verbose output
    if (verbose > 0) {
        std::cout << "[INFO] " << totalBranches << " branches in config file\n";
        std::cout << "[INFO] " << outCols.size() << " branches selected to be copied\n";
        std::cout << "[INFO] " << colNames.size() << " total columns in RDF\n";
    }

    // Report missing columns
    if (!missingCols.empty()) {
        std::cerr << "[ERROR] The following requested columns are missing:\n";
        for (const auto& name : missingCols) {
            std::cerr << "  - " << name << "\n";
        }
    }

    if (outCols.empty()) {
        std::cerr << "[ERROR] No valid branches selected" << std::endl;
        return;
    }

    // Create output with selected branches
    ROOT::RDF::RSnapshotOptions options;
    options.fMode = "RECREATE";
    
    // Show progress
    std::cout << "[INFO] Batch " << batchIndex << "/" << (totalBatches-1) << ": Writing output..." << std::endl;
    
    df.Snapshot("jet_tree", outFile, outCols, options);

    // Calculate elapsed time
    auto endTime = std::chrono::steady_clock::now();
    auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();
    
    if (verbose) {
        std::cout << "[DONE] Batch " << batchIndex << "/" << (totalBatches-1) 
                 << ": Output written to " << outFile 
                 << " (completed in " << elapsedSeconds << " seconds)" << std::endl;
    }

    // Cleanup
    delete base;
    for (auto *f : friends) delete f;
}

void SkimHiForest(const std::string &cfgPath = "../configs/2023_PbPb_Data_HirawPrime0_part.config", int specificBatchID = -1) {
    // Start timing the entire process
    auto startTimeTotal = std::chrono::steady_clock::now();
    
    // --- 1) Read config ---
    TEnv env;
    if (env.ReadFile(cfgPath.c_str(), kEnvGlobal) < 0) {
        std::cerr << "[ERROR] Cannot read config: " << cfgPath << std::endl;
        return;
    }

    verbose = env.GetValue("Verbose", 1);
    std::string inputDir = env.GetValue("InputDir", ".");
    std::string outputDir = env.GetValue("OutputDir", "output");
    std::string outName = env.GetValue("OutName", "jet_tree");
    int fileLimit = env.GetValue("FileLimit", 99999);
    int filesPerBatch = env.GetValue("FilesPerOutput", 1); // Default to 1 file per batch if not specified
    bool batchMode = env.GetValue("BatchMode", 0) != 0; // Enable batch mode if specified in config

    if (verbose) {
        std::cout << "Processing config: " << cfgPath << std::endl;
        std::cout << "Input directory: " << inputDir << std::endl;
        std::cout << "Output directory: " << outputDir << std::endl;
        std::cout << "Batch mode: " << (batchMode ? "enabled" : "disabled") << std::endl;
        if (batchMode) {
            std::cout << "Files per batch: " << filesPerBatch << std::endl;
            if (specificBatchID >= 0) {
                std::cout << "Processing only batch " << specificBatchID << std::endl;
            } else {
                std::cout << "Processing all batches sequentially" << std::endl;
            }
        }
    }

    // --- 2) Gather input files ---
    auto allFiles = GetFiles(inputDir, fileLimit);
    if (allFiles.empty()) {
        std::cerr << "[ERROR] No .root files found in " << inputDir << std::endl;
        return;
    }
    if (verbose) std::cout << "[INFO] Found " << allFiles.size() << " files in total" << std::endl;
    
    // Organize files into batches if batch mode is enabled
    if (batchMode) {
        FileBatches batches;
        batches.createBatches(allFiles, filesPerBatch);
        
        if (verbose) {
            std::cout << "[INFO] Organized " << batches.totalFiles << " files into " 
                     << batches.totalBatches << " batches" << std::endl;
        }
        
        // Process either a specific batch or all batches
        if (specificBatchID >= 0) {
            if ((size_t)specificBatchID >= batches.totalBatches) {
                std::cerr << "[ERROR] Requested batch " << specificBatchID 
                         << " is out of range (max: " << (batches.totalBatches-1) << ")" << std::endl;
                return;
            }
            
            auto batchFiles = batches.getBatch(specificBatchID);
            if (verbose) {
                std::cout << "[INFO] Processing batch " << specificBatchID << " with " 
                         << batchFiles.size() << " files" << std::endl;
            }
            
            std::string batchOutName = outName + "_batch" + std::to_string(specificBatchID) 
                                    + "_of_" + std::to_string(batches.totalBatches);
            ProcessBatch(env, batchFiles, outputDir, batchOutName, specificBatchID, batches.totalBatches);
        } else {
            // Process all batches sequentially
            for (size_t i = 0; i < batches.totalBatches; i++) {
                if (verbose) {
                    std::cout << "\n[INFO] Processing batch " << i << " of " 
                             << batches.totalBatches << std::endl;
                }
                
                auto batchFiles = batches.getBatch(i);
                std::string batchOutName = outName + "_batch" + std::to_string(i) 
                                        + "_of_" + std::to_string(batches.totalBatches);
                ProcessBatch(env, batchFiles, outputDir, batchOutName, i, batches.totalBatches);
            }
            
            // Calculate elapsed time for all batches
            auto endTimeTotal = std::chrono::steady_clock::now();
            auto elapsedSecondsTotal = std::chrono::duration_cast<std::chrono::seconds>(endTimeTotal - startTimeTotal).count();
            
            if (verbose) {
                std::cout << "\n[DONE] Processed all " << batches.totalBatches << " batches" << std::endl;
                std::cout << "[INFO] Total processing time for all batches: " << elapsedSecondsTotal << " seconds";
                if (elapsedSecondsTotal > 60) {
                    int minutes = elapsedSecondsTotal / 60;
                    int seconds = elapsedSecondsTotal % 60;
                    std::cout << " (" << minutes << " minutes, " << seconds << " seconds)";
                }
                std::cout << std::endl;
            }
        }
        
        return; // Return after batch processing
    }
    
    // If not in batch mode, process all files as before
    if (verbose) std::cout << "[INFO] Processing all " << allFiles.size() << " files in one go" << std::endl;
    auto files = allFiles;

    // --- 3) Build chains and tree mapping ---
    std::map<std::string, std::string> treeMap;  // Store mapping for branch selection
    std::map<std::string, std::string> treeAliases;
    std::string treesStr = env.GetValue("Trees", "");
    std::istringstream treeStream(treesStr);
    std::string tree;
    while (std::getline(treeStream, tree, ' ')) {
        if (tree.empty()) continue;
        size_t colonPos = tree.find(':');
        if (colonPos != std::string::npos) {
            std::string path = tree.substr(0, colonPos);
            std::string alias = tree.substr(colonPos + 1);
            treeAliases[path] = alias;
            treeMap[path] = alias;  // Store mapping for later use
        }
    }

    // Base chain - make sure we use HiTree as base
    std::string hiEvtPath = "hiEvtAnalyzer/HiTree";
    TChain *base = new TChain(hiEvtPath.c_str());
    for (const auto &file : files) {
        base->Add(file.c_str());
    }

    // Add friends with proper aliases
    std::vector<TChain*> friends;
    for (const auto& [path, alias] : treeAliases) {
        if (path == hiEvtPath) continue; // Skip base tree
        
        TChain *friend_chain = new TChain(path.c_str());
        for (const auto &file : files) {
            friend_chain->Add(file.c_str());
        }
        // Debug friend chain content
        if (verbose > 1) {
            std::cout << "[DEBUG] Adding friend tree " << path << " as " << alias 
                     << " with " << friend_chain->GetEntries() << " entries" << std::endl;
        }
        base->AddFriend(friend_chain, alias.c_str());
        friends.push_back(friend_chain);
    }

    // --- 4) Create RDataFrame ---
    RDataFrame df(*base);

    // Debug available columns
    if (verbose > 1) {
        auto cols = df.GetColumnNames();
        std::cout << "\n[DEBUG] Available RDF columns after friend addition:" << std::endl;
        for (const auto& col : cols) {
            std::cout << " - " << col << std::endl;
        }
    }

    // --- 5) Configure output ---
    if (!dirExists(outputDir)) {
        createDirectories(outputDir);
    }
    std::string outFile = outputDir + "/" + outName + ".root";
    
    // --- 6) Process and save ---
    // Get available columns from RDataFrame
    auto colNames = df.GetColumnNames();
    std::set<std::string> colSet(colNames.begin(), colNames.end());

    // Debug output of available columns
    if (verbose > 1) {
        std::cout << "\n[DEBUG] Available columns in RDataFrame:" << std::endl;
        for (const auto& col : colNames) {
            std::cout << col << std::endl;
        }
    }

    // Debug: Print tree configurations
    if (verbose > 1) {
        std::cout << "\n[DEBUG] Tree configurations from config:" << std::endl;
        for (const auto& [path, alias] : treeMap) {
            std::cout << "Tree path: " << path << " -> Alias: " << alias << std::endl;
        }
    }

    // Parse branch configurations and prepare output columns
    auto branchConfigs = ParseBranchSelection(env, treeMap);
    if (verbose > 1) {
        std::cout << "\n[DEBUG] Branch configurations:" << std::endl;
        for (const auto& cfg : branchConfigs) {
            std::cout << "Tree: " << cfg.treePath << " -> " << cfg.treeAlias << std::endl;
            std::cout << "Requested branches:" << std::endl;
            for (const auto& branch : cfg.branches) {
                std::cout << "  " << branch << " -> " << cfg.branchAliases.at(branch) << std::endl;
            }
        }
    }

    // Debug: Print branch search attempts
    std::vector<std::string> outCols;
    std::vector<std::string> missingCols;

    if (verbose > 1) {
        std::cout << "\n[DEBUG] Branch search attempts:" << std::endl;
    }

    // Process each branch configuration
    for (const auto& cfg : branchConfigs) {
        for (const auto& branch : cfg.branches) {
            std::string fullName = cfg.treeAlias + "." + branch;
            std::string aliasName = cfg.branchAliases.at(branch);
            
            // Try branch names based on tree type
            bool found = false;
            if (cfg.treePath == "hiEvtAnalyzer/HiTree") {
                // Base tree branches have no prefix
                if (colSet.count(branch)) {
                    outCols.push_back(branch);
                    found = true;
                    if (verbose > 1) std::cout << "Found base branch: " << branch << std::endl;
                }
            } else {
                // Friend trees need full prefixed name
                if (colSet.count(fullName)) {
                    outCols.push_back(fullName);
                    found = true;
                    if (verbose > 1) std::cout << "Found friend branch: " << fullName << std::endl;
                }
            }

            if (!found) {
                missingCols.push_back(aliasName);
                if (verbose > 1) std::cout << "Missing: " << aliasName << std::endl;
            }
        }
    }

    // Calculate total branches from all trees
    size_t totalBranches = 0;
    for (const auto& cfg : branchConfigs) {
        totalBranches += cfg.branches.size();
    }

    // Verbose output
    if (verbose > 0) {
        std::cout << "[INFO] " << totalBranches << " branches in config file\n";
        std::cout << "[INFO] " << outCols.size() << " branches selected to be copied\n";
        std::cout << "[INFO] " << colNames.size() << " total columns in RDF\n";
    }

    // Report missing columns
    if (!missingCols.empty()) {
        std::cerr << "[ERROR] The following requested columns are missing:\n";
        for (const auto& name : missingCols) {
            std::cerr << "  - " << name << "\n";
        }
    }

    if (outCols.empty()) {
        std::cerr << "[ERROR] No valid branches selected" << std::endl;
        return;
    }

    // Create output with selected branches
    ROOT::RDF::RSnapshotOptions options;
    options.fMode = "RECREATE";
    
    df.Snapshot("jet_tree", outFile, outCols, options);

    // Calculate elapsed time for the entire process
    auto endTimeTotal = std::chrono::steady_clock::now();
    auto elapsedSecondsTotal = std::chrono::duration_cast<std::chrono::seconds>(endTimeTotal - startTimeTotal).count();
    
    if (verbose) {
        std::cout << "\n[DONE] Output written to " << outFile << std::endl;
        std::cout << "[INFO] Total processing time: " << elapsedSecondsTotal << " seconds";
        if (elapsedSecondsTotal > 60) {
            int minutes = elapsedSecondsTotal / 60;
            int seconds = elapsedSecondsTotal % 60;
            std::cout << " (" << minutes << " minutes, " << seconds << " seconds)";
        }
        std::cout << std::endl;
    }

    // Cleanup
    delete base;
    for (auto *f : friends) delete f;
}

#ifndef __CINT__
int main(int argc, char** argv) {
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: " << argv[0] << " <config_file> [batch_id]" << std::endl;
        return 1;
    }

    std::string cfgPath = argv[1];
    int batchId = -1;
    if (argc == 3) {
        batchId = std::atoi(argv[2]);
    }

    // Initialize ROOT in batch mode
    gROOT->SetBatch(true);
    ROOT::EnableImplicitMT();
    
    // Add current directory to library paths
    gSystem->AddDynamicPath(".");
    gInterpreter->AddIncludePath(".");
    
    // Call the existing function
    SkimHiForest(cfgPath, batchId);
    
    return 0;
}
#endif
