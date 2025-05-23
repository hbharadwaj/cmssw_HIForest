#ifndef JET_SUBSTRUCTURE_H
#define JET_SUBSTRUCTURE_H

#include <string>
#include <vector>
#include <map>
#include <TH1D.h>
#include <TH2D.h>
#include <TFile.h>
#include <TTree.h>
#include <cmath>

class JetSubstructureAnalyzer {
public:
    // Constructor with config
    JetSubstructureAnalyzer(const std::string& jetCollection, float jetPtMin, float jetEtaMax) 
        : jetCollection_(jetCollection), jetPtMin_(jetPtMin), jetEtaMax_(jetEtaMax) {
        // Parse jet collection name (e.g., "AK4Z2")
        if (jetCollection_.length() >= 4) {
            std::string jetType = jetCollection_.substr(0, 3); // AK2, AK3, etc.
            std::string zLevel = jetCollection_.substr(3);    // Z1, Z2, etc.
            
            jetRadius_ = 0.1f * (jetCollection_[2] - '0'); // Convert char to radius (2->0.2, 4->0.4)
            zCut_ = 0.1f * (zLevel[1] - '0');              // Convert char to zCut (1->0.1, 2->0.2)
        }
    }
    
    // Initialize histograms
    void initializeHistograms(TFile* outputFile, const std::vector<double>& centBins) {
        if (!outputFile) return;
        
        // Change to output file directory
        outputFile->cd();
        
        // Create a directory for this jet collection
        TDirectory* jetDir = outputFile->mkdir(jetCollection_.c_str());
        jetDir->cd();
        
        // Create histograms for each centrality bin
        for (size_t i = 0; i < centBins.size() - 1; ++i) {
            std::string centRange = std::to_string(int(centBins[i])) + "-" + std::to_string(int(centBins[i+1])) + "%";
            
            // Basic jet observables
            histograms_["h_jetPt_" + centRange] = new TH1D(("h_jetPt_" + centRange).c_str(), 
                                                     ("Jet p_{T} (" + centRange + ");p_{T} [GeV];Events").c_str(), 50, 0, 500);
            histograms_["h_jetEta_" + centRange] = new TH1D(("h_jetEta_" + centRange).c_str(), 
                                                      ("Jet #eta (" + centRange + ");#eta;Events").c_str(), 50, -2.5, 2.5);
            
            // Substructure observables
            histograms_["h_girth_" + centRange] = new TH1D(("h_girth_" + centRange).c_str(), 
                                                     ("Jet Girth (" + centRange + ");Girth;Events").c_str(), 50, 0, 0.5);
            histograms_["h_thrust_" + centRange] = new TH1D(("h_thrust_" + centRange).c_str(), 
                                                      ("Jet Thrust (" + centRange + ");Thrust;Events").c_str(), 50, 0, 1);
            histograms_["h_lha_" + centRange] = new TH1D(("h_lha_" + centRange).c_str(), 
                                                   ("LHA (" + centRange + ");LHA;Events").c_str(), 50, 0, 3);
            histograms_["h_width_" + centRange] = new TH1D(("h_width_" + centRange).c_str(), 
                                                     ("Jet Width (" + centRange + ");Width;Events").c_str(), 50, 0, 0.5);
            histograms_["h_mass_" + centRange] = new TH1D(("h_mass_" + centRange).c_str(), 
                                                    ("Jet Mass (" + centRange + ");Mass [GeV];Events").c_str(), 50, 0, 50);
            
            // 2D correlations
            histograms2D_["h_girth_vs_pt_" + centRange] = new TH2D(("h_girth_vs_pt_" + centRange).c_str(), 
                                                           ("Girth vs p_{T} (" + centRange + ");p_{T} [GeV];Girth").c_str(), 
                                                           50, 0, 500, 50, 0, 0.5);
            histograms2D_["h_mass_vs_pt_" + centRange] = new TH2D(("h_mass_vs_pt_" + centRange).c_str(), 
                                                          ("Mass vs p_{T} (" + centRange + ");p_{T} [GeV];Mass [GeV]").c_str(), 
                                                          50, 0, 500, 50, 0, 50);
        }
    }
    
    // Process jets for one event
    template <typename JetContainer>
    void processJets(const JetContainer& jetPt, const JetContainer& jetEta, const JetContainer& jetPhi,
                    const JetContainer& jetMass, const JetContainer& jetGirth, const JetContainer& jetThrust,
                    const JetContainer& jetLHA, const JetContainer& jetWidth, int nJets, int centBin,
                    const std::vector<double>& centBins) {
        
        // Find which centrality range this belongs to
        std::string centRange = "";
        for (size_t i = 0; i < centBins.size() - 1; ++i) {
            if (centBin >= centBins[i] && centBin < centBins[i+1]) {
                centRange = std::to_string(int(centBins[i])) + "-" + std::to_string(int(centBins[i+1])) + "%";
                break;
            }
        }
        
        if (centRange.empty()) return;
        
        for (int i = 0; i < nJets; ++i) {
            // Apply basic jet selection
            if (jetPt[i] <= jetPtMin_) continue;
            if (std::abs(jetEta[i]) >= jetEtaMax_) continue;
            
            // Fill histograms
            if (histograms_.count("h_jetPt_" + centRange))
                histograms_["h_jetPt_" + centRange]->Fill(jetPt[i]);
                
            if (histograms_.count("h_jetEta_" + centRange))
                histograms_["h_jetEta_" + centRange]->Fill(jetEta[i]);
                
            if (histograms_.count("h_girth_" + centRange))
                histograms_["h_girth_" + centRange]->Fill(jetGirth[i]);
                
            if (histograms_.count("h_thrust_" + centRange))
                histograms_["h_thrust_" + centRange]->Fill(jetThrust[i]);
                
            if (histograms_.count("h_lha_" + centRange))
                histograms_["h_lha_" + centRange]->Fill(jetLHA[i]);
                
            if (histograms_.count("h_width_" + centRange))
                histograms_["h_width_" + centRange]->Fill(jetWidth[i]);
                
            if (histograms_.count("h_mass_" + centRange))
                histograms_["h_mass_" + centRange]->Fill(jetMass[i]);
                
            // 2D correlations
            if (histograms2D_.count("h_girth_vs_pt_" + centRange))
                histograms2D_["h_girth_vs_pt_" + centRange]->Fill(jetPt[i], jetGirth[i]);
                
            if (histograms2D_.count("h_mass_vs_pt_" + centRange))
                histograms2D_["h_mass_vs_pt_" + centRange]->Fill(jetPt[i], jetMass[i]);
        }
    }
    
    // Getter for the jet collection name
    std::string getJetCollection() const { return jetCollection_; }
    
    // Getter for jet radius
    float getJetRadius() const { return jetRadius_; }
    
    // Getter for zCut
    float getZCut() const { return zCut_; }
    
private:
    std::string jetCollection_;
    float jetPtMin_;
    float jetEtaMax_;
    float jetRadius_;
    float zCut_;
    std::map<std::string, TH1D*> histograms_;
    std::map<std::string, TH2D*> histograms2D_;
};

#endif // JET_SUBSTRUCTURE_H
