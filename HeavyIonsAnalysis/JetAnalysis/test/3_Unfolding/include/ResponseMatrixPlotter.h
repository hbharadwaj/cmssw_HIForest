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
// SOPHISTICATED MATRIX PLOTTING WITH MULTI-DIMENSIONAL AXES
// ============================================================================

enum class AxisType {
    Measured,
    Truth
};

struct AxisInfo {
    AxisType type;
    std::vector<std::string> varNames;
    std::vector<std::vector<double>> binEdges;
    std::vector<int> nBins;
    
    AxisInfo(AxisType t, const std::vector<std::string>& names, 
             const std::vector<std::vector<double>>& edges) 
        : type(t), varNames(names), binEdges(edges) {
        for (const auto& e : edges) nBins.push_back(e.size() - 1);
    }
};

class ResponseMatrixPlotter {
public:
    // Generic matrix plotting function
    static void plotGeneralizedMatrix(const std::string& setName,
                                    TH2D* h_matrix,
                                    const AxisInfo& xAxisInfo,
                                    const AxisInfo& yAxisInfo,
                                    const std::string& title = "Matrix",
                                    const std::string& outputDir = "./",
                                    TDirectory* outDir = nullptr) {
        if (!h_matrix || xAxisInfo.varNames.empty() || xAxisInfo.binEdges.empty() || 
            yAxisInfo.varNames.empty() || yAxisInfo.binEdges.empty()) {
            log(LOG_WARNING, "Invalid parameters for matrix plot: " + setName);
            return;
        }
        
        int ndim_x = xAxisInfo.varNames.size();
        int ndim_y = yAxisInfo.varNames.size();
        int nGlobalBinsX = h_matrix->GetNbinsX();
        int nGlobalBinsY = h_matrix->GetNbinsY();
        
        logDebugInfo(setName, xAxisInfo, yAxisInfo, nGlobalBinsX, nGlobalBinsY);
        
        // Create canvas with appropriate margins
        auto canvasSize = calculateCanvasSize(ndim_x, ndim_y);
        auto canvas = std::make_unique<TCanvas>((setName+"_matrix_detailed").c_str(), 
                                              (setName+" " + title).c_str(), 
                                              canvasSize.width, canvasSize.height);
        
        setupCanvasMargins(canvas.get(), ndim_x, ndim_y);
        
        // Draw main matrix
        setupMatrix(h_matrix, title, setName, xAxisInfo, yAxisInfo);
        
        // Get canvas coordinates for additional axes
        double x1 = h_matrix->GetXaxis()->GetXmin();
        double x2 = h_matrix->GetXaxis()->GetXmax(); 
        double y1 = h_matrix->GetYaxis()->GetXmin();
        double y2 = h_matrix->GetYaxis()->GetXmax();
        
        // Draw additional axes for multi-dimensional cases
        if (ndim_x > 1 || ndim_y > 1) {
            drawAdditionalAxes(xAxisInfo, nGlobalBinsX, x1, x2, y1, y2, true);  // X-axis (horizontal)
            drawAdditionalAxes(yAxisInfo, nGlobalBinsY, x1, x2, y1, y2, false); // Y-axis (vertical)
        }
        
        // Save output
        if (outDir) {
            outDir->cd();
            canvas->Write((setName+"_"+title+"_canvas").c_str());
        }
    }
    
    // Convenience function for response matrices (measured vs truth)
    static void plotGeneralizedResponseMatrix(const std::string& setName,
                                           TH2D* h_response,
                                           const std::vector<std::string>& measuredVarNames,
                                           const std::vector<std::vector<double>>& measuredBinEdges,
                                           const std::vector<std::string>& truthVarNames,
                                           const std::vector<std::vector<double>>& truthBinEdges,
                                           const std::string& title = "Response Matrix",
                                           const std::string& outputDir = "./",
                                           TDirectory* outDir = nullptr) {
        AxisInfo xAxis(AxisType::Measured, measuredVarNames, measuredBinEdges);
        AxisInfo yAxis(AxisType::Truth, truthVarNames, truthBinEdges);
        plotGeneralizedMatrix(setName, h_response, xAxis, yAxis, title, outputDir, outDir);
    }
    
    // Convenience function for covariance matrices (truth vs truth)
    static void plotCovarianceMatrix(const std::string& setName,
                                   TH2D* h_covariance,
                                   const std::vector<std::string>& truthVarNames,
                                   const std::vector<std::vector<double>>& truthBinEdges,
                                   const std::string& title = "Covariance Matrix",
                                   const std::string& outputDir = "./",
                                   TDirectory* outDir = nullptr) {
        AxisInfo xAxis(AxisType::Truth, truthVarNames, truthBinEdges);
        AxisInfo yAxis(AxisType::Truth, truthVarNames, truthBinEdges);
        plotGeneralizedMatrix(setName, h_covariance, xAxis, yAxis, title, outputDir, outDir);
    }
    
    // Convenience function for probability matrices (truth vs measured)
    static void plotProbabilityMatrix(const std::string& setName,
                                    TH2D* h_probability,
                                    const std::vector<std::string>& truthVarNames,
                                    const std::vector<std::vector<double>>& truthBinEdges,
                                    const std::vector<std::string>& measuredVarNames,
                                    const std::vector<std::vector<double>>& measuredBinEdges,
                                    const std::string& title = "Probability Matrix",
                                    const std::string& outputDir = "./",
                                    TDirectory* outDir = nullptr) {
        AxisInfo xAxis(AxisType::Truth, truthVarNames, truthBinEdges);
        AxisInfo yAxis(AxisType::Measured, measuredVarNames, measuredBinEdges);
        plotGeneralizedMatrix(setName, h_probability, xAxis, yAxis, title, outputDir, outDir);
    }

private:
    struct CanvasSize {
        int width, height;
    };
    
    static CanvasSize calculateCanvasSize(int ndim_x, int ndim_y) {
        CanvasSize size;
        if (ndim_x <= 1 && ndim_y <= 1) {
            size.width = 800;
            size.height = 600;
        } else {
            size.width = 1200 + 400 * std::max(ndim_x, ndim_y);
            size.height = 900 + 300 * std::max(ndim_x, ndim_y);
        }
        return size;
    }
    
    static void setupCanvasMargins(TCanvas* canvas, int ndim_x, int ndim_y) {
        double bottomMargin = (ndim_x <= 1) ? 0.15 : 0.08 + 0.06 * ndim_x;
        double leftMargin = (ndim_y <= 1) ? 0.15 : 0.08 + 0.06 * ndim_y;
        
        canvas->SetBottomMargin(bottomMargin);
        canvas->SetLeftMargin(leftMargin);
        canvas->SetRightMargin(0.15); // Space for color palette
        canvas->SetTopMargin(0.1);
    }
    
    static void setupMatrix(TH2D* h_matrix, const std::string& title, 
                           const std::string& setName, const AxisInfo& xAxisInfo, const AxisInfo& yAxisInfo) {
        gStyle->SetPaintTextFormat("3.1f");
        h_matrix->SetMarkerSize(0.6);
        h_matrix->Draw("text_COLZ");
        h_matrix->SetStats(0);
        h_matrix->SetTitle((title + " for " + setName).c_str());
        
        if (xAxisInfo.varNames.size() > 1 || yAxisInfo.varNames.size() > 1) {
            h_matrix->GetXaxis()->SetTitle("");
            h_matrix->GetYaxis()->SetTitle("");
            h_matrix->GetXaxis()->SetLabelSize(0);
            h_matrix->GetYaxis()->SetLabelSize(0);
        } else {
            // 1D case: use actual variable names for axis titles, fallback to type if empty
            std::string xTitle = (!xAxisInfo.varNames.empty() && !xAxisInfo.varNames[0].empty())
                ? xAxisInfo.varNames[0]
                : (xAxisInfo.type == AxisType::Measured ? "Measured" : "Truth");
            std::string yTitle = (!yAxisInfo.varNames.empty() && !yAxisInfo.varNames[0].empty())
                ? yAxisInfo.varNames[0]
                : (yAxisInfo.type == AxisType::Measured ? "Measured" : "Truth");
            h_matrix->GetXaxis()->SetTitle(xTitle.c_str());
            h_matrix->GetYaxis()->SetTitle(yTitle.c_str());
        }
    }
    
    static void logDebugInfo(const std::string& setName, const AxisInfo& xAxisInfo, const AxisInfo& yAxisInfo,
                           int nGlobalBinsX, int nGlobalBinsY) {
        log(LOG_DEBUG, "======= Matrix Debug Info for Set: " + setName + " =======");
        
        std::string xAxisTypeStr = (xAxisInfo.type == AxisType::Measured) ? "Measured" : "Truth";
        std::string yAxisTypeStr = (yAxisInfo.type == AxisType::Measured) ? "Measured" : "Truth";
        
        std::string xVarsStr = "X-axis (" + xAxisTypeStr + ", " + std::to_string(xAxisInfo.varNames.size()) + "D): ";
        for (size_t i = 0; i < xAxisInfo.varNames.size(); ++i) {
            xVarsStr += xAxisInfo.varNames[i] + (i < xAxisInfo.varNames.size()-1 ? ", " : "");
        }
        log(LOG_DEBUG, xVarsStr);
        
        std::string yVarsStr = "Y-axis (" + yAxisTypeStr + ", " + std::to_string(yAxisInfo.varNames.size()) + "D): ";
        for (size_t i = 0; i < yAxisInfo.varNames.size(); ++i) {
            yVarsStr += yAxisInfo.varNames[i] + (i < yAxisInfo.varNames.size()-1 ? ", " : "");
        }
        log(LOG_DEBUG, yVarsStr);
        
        log(LOG_DEBUG, "Global bins: X=" + std::to_string(nGlobalBinsX) + ", Y=" + std::to_string(nGlobalBinsY));
        log(LOG_DEBUG, "==================================================");
    }
    
    static void drawAdditionalAxes(const AxisInfo& axisInfo, int nGlobalBins,
                                 double x1, double x2, double y1, double y2, bool isHorizontal) {
        double axisOffset = 0.1;
        for (size_t dim = 0; dim < axisInfo.varNames.size(); ++dim) {
            DimensionCycleInfo cycleInfo = calculateCycleInfo(dim, axisInfo.nBins);
            
            double axisPos;
            if (isHorizontal) {
                axisPos = y1 - axisOffset * (y2-y1) * (dim+1);
            } else {
                axisPos = x1 - axisOffset * (x2-x1) * (dim+1);
            }
            
            drawDimensionCycles(cycleInfo, axisInfo.binEdges[dim], axisInfo.varNames[dim],
                              axisPos, x1, x2, y1, y2, nGlobalBins, isHorizontal);
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
