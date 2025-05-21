#include <TFile.h>
#include <TTree.h>
#include <TSystemDirectory.h>
#include <TSystemFile.h>
#include <TList.h>
#include <TBranch.h>
#include <TObjArray.h>

#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

// Recursively collect up to 'limit' ROOT files under 'dir'
void GetFiles(const char* dir, std::vector<std::string>& files, int limit=1) {
    TSystemDirectory sd(dir, dir);
    TList* lst = sd.GetListOfFiles();
    if (!lst) return;

    int n = lst->GetEntries();
    for (int i=0; i<n && (int)files.size()<limit; ++i) {
        TSystemFile* f = static_cast<TSystemFile*>(lst->At(i));
        std::string name = f->GetName();
        if (f->IsDirectory() && name.find('.')==std::string::npos) {
            GetFiles((std::string(dir)+"/"+name).c_str(), files, limit);
        }
        else if (name.rfind(".root")!=std::string::npos) {
            files.push_back(std::string(dir)+"/"+name);
        }
    }
}

// Replace '/' and '-' with '_' for config keys
std::string sanitize(const std::string& path) {
    std::string out;
    for (char c: path) {
        if (c=='/'||c=='-') out.push_back('_');
        else                out.push_back(c);
    }
    return out;
}

void GenerateConfig(const char* inputDir,
                    const char* exampleFile = nullptr,
                    const char* outputConfig = "basic.config",
                    int fileLimit = 1)
{
    // 1) Find one example ROOT file if needed
    std::string example = exampleFile ? exampleFile : "";
    if (example.empty()) {
        std::vector<std::string> files;
        GetFiles(inputDir, files, fileLimit);
        if (files.empty()) {
            std::cerr<<"[Error] No .root files found under "<<inputDir<<"\n";
            return;
        }
        example = files.front();
    }

    // 2) Open the example file
    TFile* f = TFile::Open(example.c_str());
    if (!f || f->IsZombie()) {
        std::cerr<<"[Error] Could not open "<<example<<"\n";
        return;
    }

    // 3) Define tree paths with their aliases (matching current config format)
    std::vector<std::pair<std::string, std::string>> treePaths = {
        {"hiEvtAnalyzer/HiTree", "hiEvt"},
        {"ggHiNtuplizer/EventTree", "ggHi"},
        {"skimanalysis/HltTree", "skim"},
        {"hltanalysis/HltTree", "hlt"}
    };

    // Add jet analyzers with corresponding aliases
    std::vector<std::pair<int,int>> jetCfg = {{2,1},{2,2},{2,3},{2,4},{3,5},{3,1},{3,2},{3,3},{3,4},{3,5},{4,1},{4,2},{4,3},{4,4},{4,5},{5,1},{5,2},{5,3},{5,4},{5,5},{6,1},{6,2},{6,3},{6,4},{6,5},{8,1},{8,2},{8,3},{8,4},{8,5}};  // Extend as needed
    for (auto& p: jetCfg) {
        treePaths.push_back({
            Form("akCs%dPFJetAnalyzerSDZcut%d/t", p.first, p.second),
            Form("AK%dZ%d", p.first, p.second)
        });
    }

    // 4) Write config header
    std::ofstream out(outputConfig);
    out << "# 2023 PbPb QCDPhoton Configuration\n\n"
        << "# Processing Control\n"
        << "DoCondor 0\n"
        << "Verbose 1\n"
        << "FileLimit " << fileLimit << "\n"
        << "FilesPerOutput 2\n\n"
        << "# I/O Configuration\n"
        << "InputDir " << inputDir << "\n"
        << "OutputDir /eos/cms/store/group/phys_heavyions/bharikri/Run3GammaJet/2023_PbPb/MC/\n"
        << "OutName generated_config\n\n"
        << "# Trees and Collections\n"
        << "Trees";

    // Write tree paths with aliases
    for (const auto& [path, alias] : treePaths) {
        out << " " << path << ":" << alias;
    }
    out << "\n\n# Branch Selection - matching the tree paths exactly\n";

    // 5) For each tree, dump its branches
    for (const auto& [path, alias] : treePaths) {
        TTree* tr = static_cast<TTree*>(f->Get(path.c_str()));
        if (!tr) {
            out << "# (missing) Branches_" << path << " =\n";
            continue;
        }

        TObjArray* brl = tr->GetListOfBranches();
        int n = brl->GetEntries();
        
        out << "Branches_" << path << " ";
        for (int i=0; i<n; ++i) {
            TBranch* br = static_cast<TBranch*>(brl->At(i));
            out << br->GetName();
            if (i+1<n) out << ",";
        }
        out << "\n\n";
    }

    out << "# Jet Configuration\n"
        << "JetCollections {";
    for (size_t i=0; i<jetCfg.size(); ++i) {
        out << "{" << jetCfg[i].first << "," << jetCfg[i].second << "}";
        if (i+1<jetCfg.size()) out << ",";
    }
    out << "}\n\n"
        << "# PThat Configuration\n"
        << "PThatBins 30 50 80 120 170\n"
        << "WeightMethod recursive\n";

    out.close();
    f->Close();
    std::cout << "[Info] Wrote template config to " << outputConfig << "\n";
}
