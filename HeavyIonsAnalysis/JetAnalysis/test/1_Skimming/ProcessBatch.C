// ProcessBatch.C
// This file contains the implementation of the ProcessBatch function used by SkimHiForest.C
// for batch processing of ROOT files.

void ProcessBatch(const TEnv& env, const std::vector<std::string>& batchFiles, 
                 const std::string& outputDir, const std::string& outName, 
                 int batchIndex, int totalBatches) {
    
    if (batchFiles.empty()) {
        std::cerr << "[ERROR] Batch " << batchIndex << " is empty" << std::endl;
        return;
    }
    
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
    if (!fs::exists(outputDir)) {
        fs::create_directories(outputDir);
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

    if (verbose) std::cout << "[DONE] Batch " << batchIndex << "/" << (totalBatches-1) 
                           << ": Output written to " << outFile << std::endl;

    // Cleanup
    delete base;
    for (auto *f : friends) delete f;
}
