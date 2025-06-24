// Usage: .x calculate_pthat_weights.C("../configs/2018_PbPb_QCDPhoton.config", "2018_pthat_nominal_weights.txt")

#include <TEnv.h>
#include <TFile.h>
#include <TTree.h>
#include <TSystem.h>
#include <TString.h>
#include <TH1F.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>

// Helper function to recursively collect .root files
std::vector<std::string> GetFiles(const std::string &dir, int limit = 999999) {
    std::vector<std::string> out;
    void *dirp = gSystem->OpenDirectory(dir.c_str());
    if (!dirp) return out;
    const char *entry;
    while ((entry = gSystem->GetDirEntry(dirp))) {
        std::string name(entry);
        if (name == "." || name == "..") continue;
        std::string path = dir + "/" + name;
        FileStat_t st;
        if (gSystem->GetPathInfo(path.c_str(), st) == 0) {
            if (R_ISDIR(st.fMode)) {
                auto sub = GetFiles(path, limit - out.size());
                out.insert(out.end(), sub.begin(), sub.end());
            } else if (name.size() > 5 && name.substr(name.size() - 5) == ".root") {
                out.push_back(path);
                if ((int)out.size() >= limit) break;
            }
        }
    }
    gSystem->FreeDirectory(dirp);
    return out;
}

void calculate_pthat_weights(const char* config_path, const char* output_txt) {
    TEnv env;
    if (env.ReadFile(config_path, kEnvGlobal) < 0) {
        std::cerr << "[ERROR] Cannot read config: " << config_path << std::endl;
        return;
    }

    // Get PThatBins
    std::string pthat_bins_str = env.GetValue("PThatBins", "");
    if (pthat_bins_str.empty()) {
        std::cerr << "[ERROR] No PThatBins found in config." << std::endl;
        return;
    }
    std::vector<float> pthat_bins;
    std::istringstream ss(pthat_bins_str);
    float val;
    while (ss >> val) pthat_bins.push_back(val);

    // For each bin, get InputDir_<bin>
    std::vector<std::string> input_dirs;
    for (float bin : pthat_bins) {
        std::string key = Form("InputDir_%.0f", bin);
        std::string dir = env.GetValue(key.c_str(), "");
        if (dir.empty()) {
            std::cerr << "[ERROR] Missing " << key << " in config." << std::endl;
            return;
        }
        input_dirs.push_back(dir);
    }

    // Prepare to store sum of weights for each bin, for each lower edge
    size_t nBins = pthat_bins.size();
    std::vector<std::vector<double>> sum_weights(nBins, std::vector<double>(nBins, 0.0)); // sum_weights[bin][cut]
    std::vector<std::vector<long long>> n_events(nBins, std::vector<long long>(nBins, 0)); // n_events[bin][cut]

    // For histogram binning, find min and max pthat across all samples
    float min_pthat = 1e9, max_pthat = -1e9;
    for (size_t i = 0; i < nBins; ++i) {
        auto files = GetFiles(input_dirs[i]);
        for (const auto& file : files) {
            TFile* f = TFile::Open(file.c_str());
            if (!f || f->IsZombie()) continue;
            TTree* t = (TTree*)f->Get("hiEvtAnalyzer/HiTree");
            if (!t) { f->Close(); delete f; continue; }
            float pthat = 0.0;
            t->SetBranchStatus("*", 0);
            t->SetBranchStatus("pthat", 1);
            t->SetBranchAddress("pthat", &pthat);
            Long64_t n = t->GetEntries();
            for (Long64_t j = 0; j < n; ++j) {
                t->GetEntry(j);
                if (pthat < min_pthat) min_pthat = pthat;
                if (pthat > max_pthat) max_pthat = pthat;
            }
            f->Close();
            delete f;
        }
    }
    if (min_pthat >= max_pthat) { min_pthat = 0; max_pthat = 500; }
    float hist_min = std::max(0.f, min_pthat - 5);
    float hist_max = max_pthat + 10;
    int nbins = 100;

    // Create global histograms
    TH1F* h_pthat_raw = new TH1F("h_pthat_raw", "Raw pthat distribution;#hat{p}_{T};Events (sum weights)", nbins, hist_min, hist_max);
    TH1F* h_pthat_reweighted = new TH1F("h_pthat_reweighted", "Reweighted pthat distribution;#hat{p}_{T};Events (sum weights*recursive weight)", nbins, hist_min, hist_max);

    // Create per-bin raw histograms
    std::vector<TH1F*> h_pthat_raw_bins(nBins, nullptr);
    for (size_t i = 0; i < nBins; ++i) {
        TString hname = Form("h_pthat_raw_bin%zu", i);
        TString htitle = Form("Raw pthat for bin %.0f;#hat{p}_{T};Events (sum weights)", pthat_bins[i]);
        h_pthat_raw_bins[i] = new TH1F(hname, htitle, nbins, hist_min, hist_max);
    }

    // For each bin, for each lower edge, sum weights for pthat >= lower_edge
    for (size_t i = 0; i < nBins; ++i) {
        std::cout << "[INFO] Processing pthat bin " << pthat_bins[i] << " in dir: " << input_dirs[i] << std::endl;
        auto files = GetFiles(input_dirs[i]);
        for (size_t cutIdx = 0; cutIdx < nBins; ++cutIdx) {
            float cut = pthat_bins[cutIdx];
            double total_weight = 0.0;
            long long total_events = 0;
            for (const auto& file : files) {
                TFile* f = TFile::Open(file.c_str());
                if (!f || f->IsZombie()) {
                    std::cerr << "[WARNING] Could not open file: " << file << std::endl;
                    continue;
                }
                TTree* t = (TTree*)f->Get("hiEvtAnalyzer/HiTree");
                if (!t) {
                    std::cerr << "[WARNING] No hiEvtAnalyzer/HiTree in file: " << file << std::endl;
                    f->Close();
                    delete f;
                    continue;
                }
                float weight = 1.0, pthat = 0.0;
                t->SetBranchStatus("*", 0);
                t->SetBranchStatus("weight", 1);
                t->SetBranchStatus("pthat", 1);
                t->SetBranchAddress("weight", &weight);
                t->SetBranchAddress("pthat", &pthat);
                Long64_t n = t->GetEntries();
                for (Long64_t j = 0; j < n; ++j) {
                    t->GetEntry(j);
                    if (pthat >= cut) {
                        total_weight += 1;
                        total_events++;
                    }
                }
                f->Close();
                delete f;
            }
            sum_weights[i][cutIdx] = total_weight;
            n_events[i][cutIdx] = total_events;
        }
    }

    // Now, calculate recursive weights
    std::vector<double> weights(nBins, 0.0);
    if (sum_weights[0][0] > 0)
        weights[0] = 1.0 / sum_weights[0][0];
    else
        weights[0] = 0.0;

    for (size_t i = 1; i < nBins; ++i) {
        double numerator = 0.0;
        for (size_t j = 0; j < i; ++j) {
            numerator += sum_weights[j][i];
        }
        double denominator = sum_weights[i][i];
        if (denominator > 0)
            weights[i] = (numerator / denominator) * weights[i-1];
        else
            weights[i] = 0.0;
    }

    // Fill histograms: loop again over all events, fill raw and reweighted, and per-bin raw
    for (size_t i = 0; i < nBins; ++i) {
        auto files = GetFiles(input_dirs[i]);
        for (const auto& file : files) {
            TFile* f = TFile::Open(file.c_str());
            if (!f || f->IsZombie()) continue;
            TTree* t = (TTree*)f->Get("hiEvtAnalyzer/HiTree");
            if (!t) { f->Close(); delete f; continue; }
            float weight = 1.0, pthat = 0.0;
            t->SetBranchStatus("*", 0);
            t->SetBranchStatus("weight", 1);
            t->SetBranchStatus("pthat", 1);
            t->SetBranchAddress("weight", &weight);
            t->SetBranchAddress("pthat", &pthat);
            Long64_t n = t->GetEntries();
            for (Long64_t j = 0; j < n; ++j) {
                t->GetEntry(j);
                h_pthat_raw->Fill(pthat, weight);
                h_pthat_reweighted->Fill(pthat, weight * weights[i]);
                h_pthat_raw_bins[i]->Fill(pthat, weight);
            }
            f->Close();
            delete f;
        }
    }

    // Write output weights
    std::ofstream fout(output_txt);
    if (!fout) {
        std::cerr << "[ERROR] Could not open output file: " << output_txt << std::endl;
        return;
    }
    fout << "# pthat_bin recursive_weight\n";
    fout << std::fixed << std::setprecision(15);
    for (size_t i = 0; i < nBins; ++i) {
        fout << pthat_bins[i] << " " << weights[i] << std::endl;
    }
    fout.close();
    std::cout << "[DONE] Wrote recursive weights to " << output_txt << std::endl;

    // Save histograms to a ROOT file
    TFile* hout = new TFile("pthat_weighting_histos.root", "RECREATE");
    h_pthat_raw->Write();
    h_pthat_reweighted->Write();
    for (size_t i = 0; i < nBins; ++i) {
        h_pthat_raw_bins[i]->Write();
    }
    hout->Close();
    std::cout << "[DONE] Wrote pthat histograms to pthat_weighting_histos.root" << std::endl;
}