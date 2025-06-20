#ifndef RESPONSE_MATRIX_PLOTTER_H
#define RESPONSE_MATRIX_PLOTTER_H

#include "UnfoldHelpers.h"
#include "UnfoldConfig.h"
#include <memory>
#include <vector>
#include <string>
#include <TCanvas.h>
#include <TH2D.h>
#include <TStyle.h>
#include <TLine.h>
#include <TLatex.h>

// ============================================================================
// SOPHISTICATED RESPONSE MATRIX PLOTTING WITH MULTI-DIMENSIONAL AXES
// ============================================================================

class ResponseMatrixPlotter {
public:
    static void plotGeneralizedResponseMatrix(const std::string& setName,
                                           TH2D* h_response,
                                           const std::vector<std::string>& measuredVarNames,
                                           const std::vector<std::vector<double>>& measuredBinEdges,
                                           const std::vector<std::string>& truthVarNames,
                                           const std::vector<std::vector<double>>& truthBinEdges,
                                           const std::string& title = "Response Matrix",
                                           const std::string& outputDir = "./",
                                           TDirectory* outDir = nullptr) {
        if (!h_response || measuredVarNames.empty() || measuredBinEdges.empty() || 
            truthVarNames.empty() || truthBinEdges.empty()) {
            log(LOG_WARNING, "Invalid parameters for response matrix plot: " + setName);
            return;
        }
        
        int ndim_meas = measuredVarNames.size();
        int ndim_truth = truthVarNames.size();
        int nGlobalBinsX = h_response->GetNbinsX(); // truth (X axis)
        int nGlobalBinsY = h_response->GetNbinsY(); // measured (Y axis)
        
        // Calculate bin counts for each dimension
        std::vector<int> nBinsMeasured, nBinsTruth;
        for (const auto& edges : measuredBinEdges) nBinsMeasured.push_back(edges.size() - 1);
        for (const auto& edges : truthBinEdges) nBinsTruth.push_back(edges.size() - 1);
        
        logDebugInfo(setName, ndim_truth, ndim_meas, nGlobalBinsX, nGlobalBinsY, 
                    truthVarNames, measuredVarNames, nBinsTruth, nBinsMeasured);
        
        // Create canvas with appropriate margins
        auto canvasSize = calculateCanvasSize(ndim_truth, ndim_meas);
        auto canvas = std::make_unique<TCanvas>((setName+"_response_detailed").c_str(), 
                                              (setName+" " + title).c_str(), 
                                              canvasSize.width, canvasSize.height);
        
        setupCanvasMargins(canvas.get(), ndim_truth, ndim_meas);
        
        // Draw main response matrix
        setupResponseMatrix(h_response, title, setName, ndim_truth, ndim_meas);
        
        // Get canvas coordinates for additional axes
        double x1 = h_response->GetXaxis()->GetXmin();
        double x2 = h_response->GetXaxis()->GetXmax(); 
        double y1 = h_response->GetYaxis()->GetXmin();
        double y2 = h_response->GetYaxis()->GetXmax();
        
        // Draw additional axes for multi-dimensional cases
        if (ndim_truth > 1 || ndim_meas > 1) {
            drawAdditionalTruthAxes(truthVarNames, truthBinEdges, nBinsTruth, 
                                  nGlobalBinsX, x1, x2, y1, y2);
            drawAdditionalMeasuredAxes(measuredVarNames, measuredBinEdges, nBinsMeasured, 
                                     nGlobalBinsY, x1, x2, y1, y2);
        }
        
        // Save output to specified directory (handle path separator properly)
        std::string outputPath = outputDir;
        if (!outputPath.empty() && outputPath.back() != '/') outputPath += "/";
        std::string outputName = outputPath + setName + "_response_matrix.png";
        // canvas->SaveAs(outputName.c_str());
        // Write canvas to output ROOT file if directory is provided
        if (outDir) {
            outDir->cd();
            canvas->Write((setName+"_response_matrix_canvas").c_str());
        }
        log(LOG_INFO, "Enhanced response matrix plot saved as: " + outputName);
    }

private:
    struct CanvasSize {
        int width, height;
    };
    
    static CanvasSize calculateCanvasSize(int ndim_truth, int ndim_meas) {
        CanvasSize size;
        if (ndim_truth <= 1 && ndim_meas <= 1) {
            size.width = 800;
            size.height = 600;
        } else {
            size.width = 1200 + 400 * std::max(ndim_truth, ndim_meas);
            size.height = 900 + 300 * std::max(ndim_truth, ndim_meas);
        }
        return size;
    }
    
    static void setupCanvasMargins(TCanvas* canvas, int ndim_truth, int ndim_meas) {
        double bottomMargin = (ndim_truth <= 1) ? 0.15 : 0.08 + 0.06 * ndim_truth;
        double leftMargin = (ndim_meas <= 1) ? 0.15 : 0.08 + 0.06 * ndim_meas;
        
        canvas->SetBottomMargin(bottomMargin);
        canvas->SetLeftMargin(leftMargin);
        canvas->SetRightMargin(0.15); // Space for color palette
        canvas->SetTopMargin(0.1);
    }
    
    static void setupResponseMatrix(TH2D* h_response, const std::string& title, 
                                   const std::string& setName, int ndim_truth, int ndim_meas) {
        gStyle->SetPaintTextFormat("3.1f");
        h_response->SetMarkerSize(0.6);
        h_response->Draw("text_COLZ");
        h_response->SetStats(0);
        h_response->SetTitle((title + " for " + setName).c_str());
        
        if (ndim_truth > 1 || ndim_meas > 1) {
            h_response->GetXaxis()->SetTitle("");
            h_response->GetYaxis()->SetTitle("");
            h_response->GetXaxis()->SetLabelSize(0);
            h_response->GetYaxis()->SetLabelSize(0);
        } else {
            h_response->GetXaxis()->SetTitle("Truth Bin Index");
            h_response->GetYaxis()->SetTitle("Measured Bin Index");
        }
    }
    
    static void logDebugInfo(const std::string& setName, int ndim_truth, int ndim_meas,
                           int nGlobalBinsX, int nGlobalBinsY,
                           const std::vector<std::string>& truthVarNames,
                           const std::vector<std::string>& measuredVarNames,
                           const std::vector<int>& nBinsTruth,
                           const std::vector<int>& nBinsMeasured) {
        log(LOG_DEBUG, "======= Response Matrix Debug Info for Set: " + setName + " =======");
        
        std::string truthVarsStr = "Truth dimensions (" + std::to_string(ndim_truth) + "): ";
        for (int i = 0; i < ndim_truth; ++i) {
            truthVarsStr += truthVarNames[i] + (i < ndim_truth-1 ? ", " : "");
        }
        log(LOG_DEBUG, truthVarsStr);
        
        std::string measVarsStr = "Measured dimensions (" + std::to_string(ndim_meas) + "): ";
        for (int i = 0; i < ndim_meas; ++i) {
            measVarsStr += measuredVarNames[i] + (i < ndim_meas-1 ? ", " : "");
        }
        log(LOG_DEBUG, measVarsStr);
        
        log(LOG_DEBUG, "Global bins: X=" + std::to_string(nGlobalBinsX) + ", Y=" + std::to_string(nGlobalBinsY));
        log(LOG_DEBUG, "==================================================");
    }
    
    static void drawAdditionalTruthAxes(const std::vector<std::string>& truthVarNames,
                                       const std::vector<std::vector<double>>& truthBinEdges,
                                       const std::vector<int>& nBinsTruth,
                                       int nGlobalBinsX, double x1, double x2, double y1, double y2) {
        double axisYOffset = 0.1;
        for (int dim = 0; dim < truthVarNames.size(); ++dim) {
            DimensionCycleInfo cycleInfo = calculateCycleInfo(dim, nBinsTruth);
            double axisY = y1 - axisYOffset * (y2-y1) * (dim+1);
            
            drawDimensionCycles(cycleInfo, truthBinEdges[dim], truthVarNames[dim],
                              axisY, x1, x2, y1, y2, nGlobalBinsX, true);
        }
    }
    
    static void drawAdditionalMeasuredAxes(const std::vector<std::string>& measuredVarNames,
                                         const std::vector<std::vector<double>>& measuredBinEdges,
                                         const std::vector<int>& nBinsMeasured,
                                         int nGlobalBinsY, double x1, double x2, double y1, double y2) {
        double axisXOffset = 0.1;
        for (int dim = 0; dim < measuredVarNames.size(); ++dim) {
            DimensionCycleInfo cycleInfo = calculateCycleInfo(dim, nBinsMeasured);
            double axisX = x1 - axisXOffset * (x2-x1) * (dim+1);
            
            drawDimensionCycles(cycleInfo, measuredBinEdges[dim], measuredVarNames[dim],
                              axisX, x1, x2, y1, y2, nGlobalBinsY, false);
        }
    }
    
    static void drawDimensionCycles(const DimensionCycleInfo& cycleInfo,
                                   const std::vector<double>& binEdges,
                                   const std::string& varName,
                                   double axisPos, double x1, double x2, double y1, double y2,
                                   int nGlobalBins, bool isHorizontal) {
        for (int cycle = 0; cycle < cycleInfo.nCycles; ++cycle) {
            int startGlobalBin = cycle * cycleInfo.stride * cycleInfo.binsPerCycle;
            
            std::vector<double> binPositions;
            for (int i = 0; i <= cycleInfo.binsPerCycle; ++i) {
                int globalBin = startGlobalBin + i * cycleInfo.stride;
                double canvasPos = isHorizontal ? 
                    x1 + (x2-x1) * (double)globalBin / nGlobalBins :
                    y1 + (y2-y1) * (double)globalBin / nGlobalBins;
                binPositions.push_back(canvasPos);
            }
            
            // Draw axis line and ticks
            drawAxisLine(binPositions, axisPos, isHorizontal);
            drawTickMarks(binPositions, axisPos, x1, x2, y1, y2, isHorizontal);
            drawAxisLabels(binPositions, binEdges, axisPos, x1, x2, y1, y2, cycle, isHorizontal);
            
            // Add dimension title for last cycle
            if (cycle == cycleInfo.nCycles - 1) {
                drawDimensionTitle(varName, binPositions.back(), axisPos, x1, x2, y1, y2, isHorizontal);
            }
            
            // Draw separator lines between cycles
            if (cycle < cycleInfo.nCycles - 1) {
                drawCycleSeparator(binPositions.back(), axisPos, x1, x2, y1, y2, isHorizontal);
            }
        }
    }
    
    static void drawAxisLine(const std::vector<double>& positions, double axisPos, bool isHorizontal) {
        TLine* line = isHorizontal ? 
            new TLine(positions.front(), axisPos, positions.back(), axisPos) :
            new TLine(axisPos, positions.front(), axisPos, positions.back());
        line->SetLineColor(kBlack);
        line->SetLineWidth(1);
        line->Draw();
    }
    
    static void drawTickMarks(const std::vector<double>& positions, double axisPos,
                            double x1, double x2, double y1, double y2, bool isHorizontal) {
        for (double pos : positions) {
            TLine* tick = isHorizontal ?
                new TLine(pos, axisPos, pos, axisPos - 0.01*(y2-y1)) :
                new TLine(axisPos, pos, axisPos - 0.01*(x2-x1), pos);
            tick->SetLineColor(kBlack);
            tick->SetLineWidth(1);
            tick->Draw();
        }
    }
    
    static void drawAxisLabels(const std::vector<double>& positions, 
                             const std::vector<double>& binEdges,
                             double axisPos, double x1, double x2, double y1, double y2, 
                             int cycle, bool isHorizontal) {
        for (size_t i = 0; i < positions.size() && i < binEdges.size(); ++i) {
            TLatex* label = new TLatex();
            label->SetTextSize(0.018);
            label->SetTextColor(kBlack);
            
            std::string labelText = (cycle > 0 && i == 0) ? " " : formatAxisValue(binEdges[i]);
            
            if (isHorizontal) {
                label->SetTextAlign(21); // Center aligned
                label->DrawLatex(positions[i], axisPos - 0.025*(y2-y1), labelText.c_str());
            } else {
                label->SetTextAlign(32); // Right aligned
                label->DrawLatex(axisPos - 0.025*(x2-x1), positions[i], labelText.c_str());
            }
        }
    }
    
    static void drawDimensionTitle(const std::string& varName, double pos, double axisPos,
                                 double x1, double x2, double y1, double y2, bool isHorizontal) {
        TLatex* title = new TLatex();
        title->SetTextSize(0.022);
        title->SetTextColor(kBlack);
        
        if (isHorizontal) {
            title->SetTextAlign(31); // Right aligned
            title->DrawLatex(pos + 0.05*(x2-x1), axisPos + 0.04*(y2-y1), varName.c_str());
        } else {
            title->SetTextAlign(32); // Right aligned
            title->SetTextAngle(90); // Vertical text
            title->DrawLatex(axisPos + 0.04*(x2-x1), pos + 0.05*(y2-y1), varName.c_str());
        }
    }
    
    static void drawCycleSeparator(double pos, double axisPos, double x1, double x2, double y1, double y2, 
                                 bool isHorizontal) {
        TLine* separator = isHorizontal ?
            new TLine(pos, axisPos, pos, axisPos + 0.04*(y2-y1)) :
            new TLine(axisPos, pos, axisPos + 0.04*(x2-x1), pos);
        separator->SetLineColor(kRed);
        separator->SetLineStyle(2);
        separator->SetLineWidth(2);
        separator->Draw();
    }
};

#endif // RESPONSE_MATRIX_PLOTTER_H
