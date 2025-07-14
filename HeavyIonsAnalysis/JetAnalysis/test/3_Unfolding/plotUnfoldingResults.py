#!/usr/bin/env python3
"""
plotUnfoldingResults.py

Advanced script to plot unfolding results (1D, 2D, 3D) from ROOT files produced by RooUnfoldOptimized.C.
Supports all test types, overlays, efficiency/purity, projections, and storing existing canvases.
CMS style and plotting logic are reused from plotGammaJet.py where possible.

Features:
- Plot unfolded, measured, truth distributions
- Plot efficiency, purity, and their numerators/denominators
- Create ratios (unfolded/truth, measured/truth)
- Project 2D/3D histograms to 1D
- Overlay multiple tests or files
- Store existing canvases (Response, Covariance, Probability matrices)
- CMS style with proper legends and labels

Usage:
    ./plotUnfoldingResults.py -c PlotUnfoldingResults.config -i input1.root [input2.root ...] -o output_dir

Author: CMS Heavy Ion Team
Date: 2025-07-14
"""

import os
import sys
import argparse
import ROOT
import numpy as np
import re
from collections import defaultdict

# Import plot helpers from the plotting infrastructure
sys.path.append('../2_SkimPlotSubstructure')
try:
    import plot_helpers
    HAS_PLOT_HELPERS = True
    logger = plot_helpers.logger
except ImportError:
    print("Warning: plot_helpers.py not found. Using minimal plotting functionality.")
    HAS_PLOT_HELPERS = False
    # Fallback simple logger
    class SimpleLogger:
        def info(self, msg): print(f"INFO: {msg}")
        def warning(self, msg): print(f"WARNING: {msg}")
        def error(self, msg): print(f"ERROR: {msg}")
        def debug(self, msg): print(f"DEBUG: {msg}")
    logger = SimpleLogger()

# ========== CMS Style Setup (enhanced from plotGammaJet.py) ==========
def set_cms_style():
    """Set CMS TDR style"""
    ROOT.gStyle.SetOptStat(0)
    ROOT.gStyle.SetOptTitle(0)
    
    # Canvas
    ROOT.gStyle.SetCanvasBorderMode(0)
    ROOT.gStyle.SetCanvasColor(ROOT.kWhite)
    ROOT.gStyle.SetCanvasDefH(600)
    ROOT.gStyle.SetCanvasDefW(800)
    
    # Frame
    ROOT.gStyle.SetFrameBorderMode(0)
    ROOT.gStyle.SetFrameLineWidth(1)
    ROOT.gStyle.SetFrameFillColor(0)
    
    # Margins
    ROOT.gStyle.SetPadTopMargin(0.08)
    ROOT.gStyle.SetPadBottomMargin(0.12)
    ROOT.gStyle.SetPadLeftMargin(0.15)
    ROOT.gStyle.SetPadRightMargin(0.05)
    
    # Fonts and sizes
    font = 42
    ROOT.gStyle.SetTextFont(font)
    ROOT.gStyle.SetTextSize(0.04)
    ROOT.gStyle.SetLabelFont(font, "XYZ")
    ROOT.gStyle.SetLabelSize(0.04, "XYZ")
    ROOT.gStyle.SetTitleFont(font, "XYZ")
    ROOT.gStyle.SetTitleSize(0.05, "XYZ")
    ROOT.gStyle.SetTitleOffset(1.2, "X")
    ROOT.gStyle.SetTitleOffset(1.2, "Y")
    
    # Legend
    ROOT.gStyle.SetLegendBorderSize(0)
    ROOT.gStyle.SetLegendFillColor(0)
    
    # Error bars
    ROOT.gStyle.SetEndErrorSize(2)
    ROOT.gStyle.SetErrorX(0.5)

def load_tdr_style():
    """Load TDR style if available"""
    tdr_style_path = os.path.join(os.path.dirname(__file__), '../2_SkimPlotSubstructure/include/tdrStyle.C')
    cms_lumi_path = os.path.join(os.path.dirname(__file__), '../2_SkimPlotSubstructure/include/CMS_lumi.C')
    
    if os.path.exists(tdr_style_path):
        ROOT.gROOT.ProcessLine(f'.L {tdr_style_path}')
        try:
            ROOT.setTDRStyle()
        except:
            set_cms_style()
    else:
        set_cms_style()
    
    if os.path.exists(cms_lumi_path):
        ROOT.gROOT.ProcessLine(f'.L {cms_lumi_path}')

# ========== Config Parsing ==========
def parse_config(config_path):
    """Parse plotting configuration file"""
    config = defaultdict(str)
    with open(config_path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"): 
                continue
            if ':' in line:
                key, val = line.split(':', 1)
                config[key.strip()] = val.strip()
    return config

# ========== Variable Label Mapping ==========
def get_variable_label(var_name):
    """Get proper axis labels for variables based on core variable name"""
    # Extract core variable name (remove jet collection suffix like _AK2Z2)
    core_var = var_name
    for suffix in ['_AK2Z1', '_AK2Z2', '_AK2Z3', '_AK3Z1', '_AK3Z2', '_AK3Z3', 
                   '_AK4Z1', '_AK4Z2', '_AK4Z3', '_AK5Z1', '_AK5Z2', '_AK5Z3',
                   '_AK6Z1', '_AK6Z2', '_AK6Z3', '_AK8Z1', '_AK8Z2', '_AK8Z3']:
        if var_name.endswith(suffix):
            core_var = var_name[:-len(suffix)]
            break
    
    label_map = {
        'xj': 'x_{J,#gamma}',
        'refxj': 'x_{J,#gamma}^{truth}',
        'jetGirth': 'Girth',
        'refJetGirth': 'Girth^{truth}',
        'jetDynDeltaR': 'R_{g}',
        'refJetDynDeltaR': 'R_{g}^{truth}',
        'jetDynKt': 'dyn k_{T} [GeV]',
        'refJetDynKt': 'dyn k_{T}^{truth} [GeV]',
        'jetDynSplit': 'n_{SD}',
        'refJetDynSplit': 'n_{SD}^{truth}',
        'jetDynZ': 'z_{g}',
        'refJetDynZ': 'z_{g}^{truth}',
        'jetPt': 'p_{T}^{jet} [GeV]',
        'refJetPt': 'p_{T}^{jet,truth} [GeV]',
        'jetEta': '#eta^{jet}',
        'refJetEta': '#eta^{jet,truth}',
        'jetPhi': '#phi^{jet}',
        'refJetPhi': '#phi^{jet,truth}',
        'jetMass': 'm^{jet} [GeV]',
        'refJetMass': 'm^{jet,truth} [GeV]',
        'photonEt': 'E_{T}^{#gamma} [GeV]',
        'MCphotonEt': 'E_{T}^{#gamma,truth} [GeV]',
        'photonEta': '#eta^{#gamma}',
        'MCphotonEta': '#eta^{#gamma,truth}',
        'photonPhi': '#phi^{#gamma}',
        'MCphotonPhi': '#phi^{#gamma,truth}'
    }
    return label_map.get(core_var, var_name)

def get_hist_title(hist_name):
    """Get descriptive titles for histograms"""
    title_map = {
        'h_measured_data': 'Measured (Data)',
        'h_measured_mc': 'Measured (MC)',
        'h_truth_mc': 'Truth (MC)',
        'h_unfolded': 'Unfolded',
        'h_unfolded_corrected': 'Unfolded (Efficiency Corrected)',
        'h_data_purity_corrected': 'Data (Purity Corrected)',
        'h_efficiency': 'Efficiency',
        'h_purity': 'Purity',
        'h_eff_num': 'Efficiency Numerator',
        'h_eff_den': 'Efficiency Denominator',
        'h_purity_num': 'Purity Numerator',
        'h_purity_den': 'Purity Denominator',
        'h_response': 'Response Matrix'
    }
    
    # Handle suffixes
    for base_name, title in title_map.items():
        if hist_name.startswith(base_name):
            suffix = hist_name[len(base_name):]
            if suffix:
                return f"{title} {suffix}"
            return title
    
    return hist_name.replace('_', ' ').title()

def get_axis_labels_from_config(unfold_set, hist_name, config):
    """Get axis labels from config based on unfolding set and histogram"""
    x_label = ""
    y_label = ""
    z_label = ""
    
    # First try unfolding set specific labels
    x_label = config.get(f'{unfold_set}.XAxis', '')
    y_label = config.get(f'{unfold_set}.YAxis', '')
    
    # If not found, try to infer from variable names in histogram using existing label_map
    if not x_label or not y_label:
        # For 2D unfolding sets, parse the variable names
        if 'girth_xJ' in unfold_set:
            if not x_label:
                x_label = get_variable_label('jetGirth')
            if not y_label:
                y_label = get_variable_label('xj')
        elif 'dyn_deltaR_xJ' in unfold_set:
            if not x_label:
                x_label = get_variable_label('jetDynDeltaR')
            if not y_label:
                y_label = get_variable_label('xj')
        elif 'dyn_kt_xJ' in unfold_set:
            if not x_label:
                x_label = get_variable_label('jetDynKt')
            if not y_label:
                y_label = get_variable_label('xj')
        elif 'dyn_z_xJ' in unfold_set:
            if not x_label:
                x_label = get_variable_label('jetDynZ')
            if not y_label:
                y_label = get_variable_label('xj')
        elif '1D_xJ' in unfold_set:
            if not x_label:
                x_label = get_variable_label('xj')
    
    # Determine Y label based on histogram type if not set
    if not y_label:
        if 'data_purity_corrected' in hist_name.lower():
            # Data distribution should have differential form
            if 'girth' in unfold_set:
                y_label = config.get('YLabel.Normalized.jetGirth', '#frac{1}{N_{jet}} #frac{dN}{dGirth}')
            elif 'xJ' in unfold_set:
                y_label = config.get('YLabel.Normalized.xj', '#frac{1}{N_{jet}} #frac{dN}{dx_{J,#gamma}}')
            elif 'dyn_deltaR' in unfold_set:
                y_label = config.get('YLabel.Normalized.jetDynDeltaR', '#frac{1}{N_{jet}} #frac{dN}{dR_{g}}')
            elif 'dyn_kt' in unfold_set:
                y_label = config.get('YLabel.Normalized.jetDynKt', '#frac{1}{N_{jet}} #frac{dN}{dk_{T}} [GeV^{-1}]')
            elif 'dyn_z' in unfold_set:
                y_label = config.get('YLabel.Normalized.jetDynZ', '#frac{1}{N_{jet}} #frac{dN}{dz_{g}}')
            else:
                y_label = config.get('YLabel.Normalized.Default', '#frac{1}{N_{jet}} #frac{dN}{dx}')
        elif 'efficiency' in hist_name.lower():
            y_label = config.get('YLabel.Efficiency', 'Efficiency')
        elif 'purity' in hist_name.lower():
            y_label = config.get('YLabel.Purity', 'Purity')
        else:
            y_label = config.get('YLabel.Default', 'Entries')
    
    return x_label, y_label, z_label

def get_normalized_ylabel(unfold_set, axis, config):
    """Get normalized Y-axis label for projections"""
    if 'girth' in unfold_set and axis == 'x':
        return config.get('YLabel.Normalized.jetGirth', '#frac{1}{N_{jet}} #frac{dN}{dGirth}')
    elif 'xJ' in unfold_set and axis == 'y':
        return config.get('YLabel.Normalized.xj', '#frac{1}{N_{jet}} #frac{dN}{dx_{J,#gamma}}')
    elif 'dyn_deltaR' in unfold_set and axis == 'x':
        return config.get('YLabel.Normalized.jetDynDeltaR', '#frac{1}{N_{jet}} #frac{dN}{dR_{g}}')
    elif 'dyn_kt' in unfold_set and axis == 'x':
        return config.get('YLabel.Normalized.jetDynKt', '#frac{1}{N_{jet}} #frac{dN}{dk_{T}} [GeV^{-1}]')
    elif 'dyn_z' in unfold_set and axis == 'x':
        return config.get('YLabel.Normalized.jetDynZ', '#frac{1}{N_{jet}} #frac{dN}{dz_{g}}')
    else:
        return config.get('YLabel.Default', 'Entries')

def should_plot_histogram(hist_name, config):
    """Check if histogram should be plotted based on config whitelist"""
    include_hists = config.get('IncludeHistograms', 'all')
    
    if include_hists.lower() == 'all':
        # Even in 'all' mode, exclude numerator/denominator histograms
        exclude_patterns = ['h_eff_num', 'h_eff_den', 'h_purity_num', 'h_purity_den']
        for pattern in exclude_patterns:
            if pattern in hist_name:
                return False
        return True
    
    # Parse whitelist
    whitelist = [h.strip() for h in include_hists.split(',')]
    
    # Check if histogram base name matches any in whitelist
    for allowed_hist in whitelist:
        if hist_name.startswith(allowed_hist):
            return True
    
    return False

# ========== ROOT File Navigation ==========
def get_unfolding_dirs(rootfile):
    """Return list of unfolding set directories"""
    dirs = []
    for key in rootfile.GetListOfKeys():
        if key.GetClassName() == 'TDirectoryFile':
            dirs.append(key.GetName())
    return dirs

def get_test_dirs(unfold_dir):
    """Return list of test directories"""
    tests = []
    for key in unfold_dir.GetListOfKeys():
        if key.GetClassName() == 'TDirectoryFile':
            tests.append(key.GetName())
    return tests

def extract_histograms_and_canvases(test_dir):
    """Extract all histograms and canvases from test directory"""
    objects = {'histograms': {}, 'canvases': {}, 'bottomline': {}}
    logger.debug(f"Extracting objects from directory: {test_dir.GetName()}")

    def extract_from_dir(directory):
        for key in directory.GetListOfKeys():
            obj_name = key.GetName()
            obj_class = key.GetClassName()
            logger.debug(f"  Found object: {obj_name} ({obj_class}) in {directory.GetName()}")
            obj = key.ReadObj()
            if obj.InheritsFrom("TH1") or obj.InheritsFrom("TH2") or obj.InheritsFrom("TH3"):
                hist = obj.Clone(f"{obj.GetName()}_clone")
                hist.SetDirectory(0)
                objects['histograms'][obj.GetName()] = hist
                logger.debug(f"    -> Added histogram: {obj.GetName()}")
            elif obj.InheritsFrom("TGraph"):
                graph_clone = obj.Clone(f"{obj.GetName()}_clone")
                objects['histograms'][obj.GetName()] = graph_clone
                logger.debug(f"    -> Added graph: {obj.GetName()}")
            elif obj.InheritsFrom("TCanvas"):
                canvas_clone = obj.Clone(f"{obj.GetName()}_clone")
                canvas_clone.SetTitle(obj.GetTitle())
                canvas_key = obj_name
                logger.debug(f"    -> Adding canvas: {canvas_key} (Title: {obj.GetTitle()})")
                objects['canvases'][canvas_key] = canvas_clone
            elif obj.InheritsFrom("TObjString"):
                if 'bottomline' in obj_name.lower():
                    objects['bottomline'][obj_name] = obj.GetString()
                    logger.debug(f"    -> Added bottomline text: {obj_name}")
            elif obj.InheritsFrom("TDirectory"):
                logger.debug(f"    -> Recursing into subdirectory: {obj.GetName()}")
                extract_from_dir(obj)
            else:
                logger.debug(f"    -> Skipping object: {obj_name} ({obj_class})")

    extract_from_dir(test_dir)
    logger.debug(f"Total extracted: {len(objects['histograms'])} histograms, {len(objects['canvases'])} canvases, {len(objects['bottomline'])} bottomline results")
    return objects

# ========== Color and Style Management ==========
def get_color_palette(scheme='petroff6'):
    """Get CMS-style color palette with multiple scheme options"""
    
    color_schemes = {
        'petroff6': [
            ROOT.kBlack,
            ROOT.TColor.GetColor("#5790fc"),  # Blue
            ROOT.TColor.GetColor("#e42536"),  # Red  
            ROOT.TColor.GetColor("#964a8b"),  # Purple
            ROOT.TColor.GetColor("#f89c20"),  # Orange
            ROOT.TColor.GetColor("#9c9ca1"),  # Gray
            ROOT.TColor.GetColor("#7a21dd")   # Violet
        ],
        'petroff10': [
            ROOT.kBlack,
            ROOT.TColor.GetColor("#3f90da"),  # Blue
            ROOT.TColor.GetColor("#bd1f01"),  # Red
            ROOT.TColor.GetColor("#832db6"),  # Purple
            ROOT.TColor.GetColor("#ffa90e"),  # Orange
            ROOT.TColor.GetColor("#a96b59"),  # Brown
            ROOT.TColor.GetColor("#e76300"),  # Dark Orange
            ROOT.TColor.GetColor("#94a4a2"),  # Gray
            ROOT.TColor.GetColor("#b9ac70"),  # Olive
            ROOT.TColor.GetColor("#717581"),  # Dark Gray
            ROOT.TColor.GetColor("#92dadd")   # Light Blue
        ],
        'viridis': [
            ROOT.TColor.GetColor("#440154"),  # Dark Purple
            ROOT.TColor.GetColor("#482777"),  # Purple
            ROOT.TColor.GetColor("#3f4a8a"),  # Blue Purple
            ROOT.TColor.GetColor("#31678e"),  # Blue
            ROOT.TColor.GetColor("#26838f"),  # Teal
            ROOT.TColor.GetColor("#1f9d8a"),  # Green-Teal
            ROOT.TColor.GetColor("#6cce5a"),  # Green
            ROOT.TColor.GetColor("#b6de2b"),  # Yellow-Green
            ROOT.TColor.GetColor("#fee825")   # Yellow
        ],
        'traditional': [
            ROOT.kBlack, 
            ROOT.kBlue, 
            ROOT.kRed, 
            ROOT.kMagenta, 
            ROOT.kGreen+2, 
            ROOT.kOrange, 
            ROOT.kCyan, 
            ROOT.kYellow+2, 
            ROOT.kGray+2
        ],
        'test_comparison': [
            ROOT.TColor.GetColor("#2E86AB"),  # Blue - Nominal
            ROOT.TColor.GetColor("#A23B72"),  # Purple - Closure  
            ROOT.TColor.GetColor("#F18F01"),  # Orange - Split
            ROOT.TColor.GetColor("#C73E1D")   # Red - Bottomline
        ],
        'method_comparison': [
            ROOT.TColor.GetColor("#1f77b4"),  # Blue - Bayes
            ROOT.TColor.GetColor("#ff7f0e"),  # Orange - Matrix Inversion
            ROOT.TColor.GetColor("#2ca02c"),  # Green - SVD
            ROOT.TColor.GetColor("#d62728")   # Red - Bin-by-Bin
        ],
        'iteration_comparison': [
            ROOT.TColor.GetColor("#1f77b4"),  # Blue - base
            ROOT.TColor.GetColor("#aec7e8"),  # Light Blue
            ROOT.TColor.GetColor("#ff7f0e"),  # Orange
            ROOT.TColor.GetColor("#ffbb78"),  # Light Orange
            ROOT.TColor.GetColor("#2ca02c"),  # Green
            ROOT.TColor.GetColor("#98df8a"),  # Light Green
            ROOT.TColor.GetColor("#d62728"),  # Red
            ROOT.TColor.GetColor("#ff9896")   # Light Red
        ]
    }
    
    return color_schemes.get(scheme, color_schemes['petroff6'])

def set_histogram_style(hist, color, marker_style=20, line_style=1, line_width=2):
    """Set histogram style"""
    hist.SetLineColor(color)
    hist.SetMarkerColor(color)
    hist.SetMarkerStyle(marker_style)
    hist.SetMarkerSize(1.2)
    hist.SetLineStyle(line_style)
    hist.SetLineWidth(line_width)

# ========== Plotting Functions ==========
def create_canvas(name, title, width=800, height=600, config=None):
    """Create a canvas with proper settings"""
    if config:
        width = int(config.get('Canvas.Width', 800))
        height = int(config.get('Canvas.Height', 600))
    
    canvas = ROOT.TCanvas(name, title, width, height)
    
    # Set margins from config or use defaults from PlotJetSub config
    if config:
        left_margin = float(config.get('Canvas.LeftMargin', 0.15))
        right_margin = float(config.get('Canvas.RightMargin', 0.05))
        top_margin = float(config.get('Canvas.TopMargin', 0.08))
        bottom_margin = float(config.get('Canvas.BottomMargin', 0.12))
    else:
        left_margin = 0.15
        right_margin = 0.05
        top_margin = 0.08
        bottom_margin = 0.12
    
    canvas.SetLeftMargin(left_margin)
    canvas.SetRightMargin(right_margin)
    canvas.SetTopMargin(top_margin)
    canvas.SetBottomMargin(bottom_margin)
    canvas.SetTicks(1, 1)
    return canvas

def add_cms_label(pad, config, lumi_text="1.72 nb^{-1}", energy_text="5.36 TeV PbPb"):
    """Add CMS label to pad"""
    # Read lumi and energy from config if present
    lumi = config.get('CMSLuminosity', lumi_text)
    energy = config.get('CMSEnergyText', energy_text)

    cms_text = ROOT.TLatex()
    cms_text.SetNDC()
    cms_text.SetTextFont(42)
    cms_text.SetTextSize(0.04)

    # CMS label
    cms_text.SetTextFont(61)  # Bold
    cms_text.SetTextSize(0.05)
    cms_text.DrawLatex(0.16, 0.94, "CMS")

    # Preliminary
    cms_text.SetTextFont(52)  # Italic
    cms_text.SetTextSize(0.04)
    cms_text.DrawLatex(0.24, 0.94, config.get('CMSLabel', 'Preliminary'))

    # Energy and luminosity
    cms_text.SetTextFont(42)
    cms_text.SetTextAlign(31)  # Right align
    cms_text.DrawLatex(0.95, 0.94, f"{lumi} ({energy})")

def create_legend(x1=0.65, y1=0.65, x2=0.90, y2=0.88):
    """Create a legend with proper styling"""
    legend = ROOT.TLegend(x1, y1, x2, y2)
    legend.SetBorderSize(0)
    legend.SetFillStyle(0)
    legend.SetTextFont(42)
    legend.SetTextSize(0.035)
    return legend

def plot_1d_histogram(hist, outname, config, title="", x_label="", y_label="", log_y=False):
    """Plot a 1D histogram with CMS style"""
    canvas = create_canvas("c1d", "c1d", config=config)
    
    if log_y:
        canvas.SetLogy()
    
    # Set axis labels with consistent sizes
    if x_label:
        hist.GetXaxis().SetTitle(x_label)
    if y_label:
        hist.GetYaxis().SetTitle(y_label)
    
    # Standardize axis label and title sizes (following plotGammaJet.py style)
    hist.GetXaxis().SetTitleSize(0.05)
    hist.GetXaxis().SetLabelSize(0.04)
    hist.GetXaxis().SetTitleOffset(1.2)
    hist.GetYaxis().SetTitleSize(0.05)
    hist.GetYaxis().SetLabelSize(0.04)
    hist.GetYaxis().SetTitleOffset(1.2)
    
    hist.Draw("E")
    
    # Add CMS label
    canvas.cd()  # Make canvas active before drawing label
    add_cms_label(canvas, config)
    
    # Save in multiple formats
    formats = config.get('PlotFormats', 'png').split(',')
    for fmt in formats:
        fmt = fmt.strip()
        if fmt:
            save_name = outname.replace('.png', f'.{fmt}')
            canvas.SaveAs(save_name)
    
    canvas.Close()

def plot_2d_histogram(hist, outname, config, title="", x_label="", y_label="", z_label="", log_z=False, draw_option="TEXT_COLZ"):
    """Plot a 2D histogram with CMS style"""
    canvas = create_canvas("c2d", "c2d", config=config)
    
    if log_z:
        canvas.SetLogz()
    
    # Set axis labels with consistent sizes
    if x_label:
        hist.GetXaxis().SetTitle(x_label)
    if y_label:
        hist.GetYaxis().SetTitle(y_label)
    if z_label:
        hist.GetZaxis().SetTitle(z_label)
    
    # Standardize axis label and title sizes (following plotGammaJet.py style)
    hist.GetXaxis().SetTitleSize(0.05)
    hist.GetXaxis().SetLabelSize(0.04)
    hist.GetXaxis().SetTitleOffset(1.2)
    hist.GetYaxis().SetTitleSize(0.05)
    hist.GetYaxis().SetLabelSize(0.04)
    hist.GetYaxis().SetTitleOffset(1.2)
    hist.GetZaxis().SetTitleSize(0.05)
    hist.GetZaxis().SetLabelSize(0.04)
    hist.GetZaxis().SetTitleOffset(1.2)
    
    # Set text format for 2D histogram bin values
    ROOT.gStyle.SetPaintTextFormat("4.3f")
    
    # Use custom draw option from config if available, otherwise default to TEXT_COLZ
    if 'DrawOption2D' in title.lower() or 'drawoption2d' in hist.GetName().lower():
        draw_option = config.get('Histogram.Default.DrawOption2D', 'TEXT_COLZ')
    
    hist.Draw(draw_option)
    
    # Add CMS label
    canvas.cd()  # Make canvas active before drawing label
    add_cms_label(canvas, config)
    
    # Save in multiple formats
    formats = config.get('PlotFormats', 'png').split(',')
    for fmt in formats:
        fmt = fmt.strip()
        if fmt:
            save_name = outname.replace('.png', f'.{fmt}')
            canvas.SaveAs(save_name)
    
    canvas.Close()

def create_ratio_plot(num_hist, den_hist, outname, config, title="Ratio", y_label="Ratio"):
    """Create a ratio plot with bin compatibility check"""
    if not num_hist or not den_hist:
        return
    
    # Check bin compatibility
    if (num_hist.GetNbinsX() != den_hist.GetNbinsX() or 
        abs(num_hist.GetXaxis().GetXmin() - den_hist.GetXaxis().GetXmin()) > 1e-6 or
        abs(num_hist.GetXaxis().GetXmax() - den_hist.GetXaxis().GetXmax()) > 1e-6):
        print(f"Warning: Skipping ratio plot {title} - incompatible binning:")
        print(f"  Numerator: {num_hist.GetNbinsX()} bins, [{num_hist.GetXaxis().GetXmin():.3f}, {num_hist.GetXaxis().GetXmax():.3f}]")
        print(f"  Denominator: {den_hist.GetNbinsX()} bins, [{den_hist.GetXaxis().GetXmin():.3f}, {den_hist.GetXaxis().GetXmax():.3f}]")
        return
    
    ratio_hist = num_hist.Clone(f"{num_hist.GetName()}_ratio")
    ratio_hist.Divide(den_hist)
    
    canvas = create_canvas("cratio", "cratio", config=config)
    
    ratio_hist.GetYaxis().SetTitle(y_label)
    ratio_hist.GetYaxis().SetRangeUser(0.5, 1.5)
    ratio_hist.Draw("E")
    
    # Add horizontal line at 1
    line = ROOT.TLine(ratio_hist.GetXaxis().GetXmin(), 1.0, 
                     ratio_hist.GetXaxis().GetXmax(), 1.0)
    line.SetLineStyle(2)
    line.SetLineColor(ROOT.kBlack)
    line.Draw("same")
    
    # Add CMS label
    canvas.cd()  # Make canvas active before drawing label
    add_cms_label(canvas, config)
    
    # Save in multiple formats
    formats = config.get('PlotFormats', 'png').split(',')
    for fmt in formats:
        fmt = fmt.strip()
        if fmt:
            save_name = outname.replace('.png', f'.{fmt}')
            canvas.SaveAs(save_name)
    
    canvas.Close()

def create_overlay_plot(histograms, labels, outname, config, title="", x_label="", y_label="", log_y=False, color_scheme='petroff6'):
    """Create overlay plot with multiple histograms"""
    if not histograms:
        return
    
    canvas = create_canvas("cov", "cov", 
                          int(config.get('Canvas.Width', 800)), 
                          int(config.get('Canvas.Height', 600)))
    
    if log_y:
        canvas.SetLogy()
    
    colors = get_color_palette(color_scheme)
    legend = create_legend()
    
    for i, (hist, label) in enumerate(zip(histograms, labels)):
        if not hist:
            continue
        
        color = colors[i % len(colors)]
        set_histogram_style(hist, color, marker_style=20+i)
        
        if x_label:
            hist.GetXaxis().SetTitle(x_label)
        if y_label:
            hist.GetYaxis().SetTitle(y_label)
        
        draw_option = "E" if i == 0 else "E SAME"
        hist.Draw(draw_option)
        
        legend.AddEntry(hist, label, "PE")
    
    legend.Draw()
    
    # Add CMS label
    canvas.cd()  # Make canvas active before drawing label
    add_cms_label(canvas, config)
    
    # Save in multiple formats
    formats = config.get('PlotFormats', 'png').split(',')
    for fmt in formats:
        fmt = fmt.strip()
        if fmt:
            save_name = outname.replace('.png', f'.{fmt}')
            canvas.SaveAs(save_name)
    
    canvas.Close()

def project_2d_histogram(hist_2d, axis='x', bin_min=1, bin_max=-1):
    """Project 2D histogram to 1D"""
    if axis.lower() == 'x':
        return hist_2d.ProjectionX(f"{hist_2d.GetName()}_projX", bin_min, bin_max)
    elif axis.lower() == 'y':
        return hist_2d.ProjectionY(f"{hist_2d.GetName()}_projY", bin_min, bin_max)
    return None

def save_existing_canvases(canvases, outdir, prefix, config):
    """Save existing canvases (Response, Covariance, Probability matrices)"""
    if not canvases:
        logger.warning("No canvases found to save")
        return
        
    logger.debug(f"Found {len(canvases)} canvases to save: {list(canvases.keys())}")

    # Temporarily enable titles for matrix plots
    original_opt_title = ROOT.gStyle.GetOptTitle()
    ROOT.gStyle.SetOptTitle(0)
    #! TODO: Fix this so that the CMS label does not overlap with the title.

    matrix_dir = os.path.join(outdir, config.get('Matrix.Subdir', 'matrices'))
    os.makedirs(matrix_dir, exist_ok=True)
    formats = config.get('PlotFormats', 'png').split(',')
    for name, canvas in canvases.items():
        if canvas and canvas.InheritsFrom("TCanvas"):
            # Add CMS label to all matrix canvases
            canvas.cd()  # Make canvas active before drawing label
            add_cms_label(canvas, config)
            clean_name = name.replace(' ', '_').replace('(', '').replace(')', '').replace('#', '').replace(';', '')
            for fmt in formats:
                fmt = fmt.strip()
                if fmt:
                    outname = os.path.join(matrix_dir, f"{clean_name}.{fmt}")
                    try:
                        canvas.SaveAs(outname)
                        logger.debug(f"Saved canvas: {outname} (Title: {canvas.GetTitle()})")
                    except Exception as e:
                        logger.error(f"Failed to save canvas {name} as {fmt}: {e}")
        else:
            logger.warning(f"Object {name} is not a valid TCanvas")

    # Restore original title setting
    ROOT.gStyle.SetOptTitle(original_opt_title)

    # Print unfolding set and directory at info level
    # Expect prefix to be like 'Unfold1D_xJ_Nominal', 'Unfold2D_girth_xJ_Nominal', 'Unfold2D_dyn_deltaR_xJ_Nominal', etc.
    parts = prefix.split('_')
    
    # Known test names that appear as the last component
    known_test_names = {'Nominal', 'Closure', 'Split', 'Bottomline'}
    
    if len(parts) >= 2 and parts[-1] in known_test_names:
        # Last part is a known test name
        unfold_set = '_'.join(parts[:-1])
        directory = parts[-1]
    else:
        # Fallback to old logic for edge cases
        if len(parts) >= 3:
            unfold_set = '_'.join(parts[:2])
            directory = '_'.join(parts[2:])
        elif len(parts) == 2:
            unfold_set, directory = parts
        else:
            unfold_set, directory = prefix, ''
    
    logger.info(f"Unfolding set: {unfold_set}, directory: {directory}")
    logger.debug(f"Saved {len(canvases)} matrix canvases for {prefix} in {len(formats)} format(s)")

# ========== Analysis Functions ==========
def analyze_unfolding_set(objects, unfold_set, test_name, config, output_dir):
    """Analyze a single unfolding set and test"""
    histograms = objects['histograms']
    canvases = objects['canvases']
    
    # Create output subdirectory
    set_dir = os.path.join(output_dir, unfold_set, test_name)
    os.makedirs(set_dir, exist_ok=True)
    
    # Save existing canvases first (matrices only, no histogram replotting)
    save_existing_canvases(canvases, set_dir, f"{unfold_set}_{test_name}", config)
    
    # Store Bayesian iterations if this is Nominal test
    store_bayesian_iterations(objects, unfold_set, test_name, config, output_dir)
    
    # Plot individual histograms (excluding matrix histograms and unfolded iteration histograms)
    for hist_name, hist in histograms.items():
        if not hist:
            continue
        # Skip h_unfolded_iterN and h_unfolded_iterationN histograms
        if re.match(r"h_unfolded_iter\d+$", hist_name) or re.match(r"h_unfolded_iteration_?\d+$", hist_name):
            continue
        # Check if histogram should be plotted based on config whitelist
        if not should_plot_histogram(hist_name, config):
            continue
        # Skip matrix histograms (they're saved as canvases)
        if any(matrix_type in hist_name.lower() for matrix_type in 
               ['response', 'covariance', 'probability', 'correlation']):
            continue
        # Get axis labels from config
        x_label, y_label, z_label = get_axis_labels_from_config(unfold_set, hist_name, config)
        if hist.GetDimension() == 1:
            # Normalize by bin width and integral for main distributions
            normalized_hist = hist
            if ('unfolded' in hist_name.lower() or 'truth' in hist_name.lower() or 
                'measured' in hist_name.lower() or 'data_purity_corrected' in hist_name.lower()) and 'efficiency' not in hist_name.lower():
                normalized_hist = normalize_projection_histogram(hist)  # Use full normalization (bin width + integral)
                # Update y_label for normalized distribution
                if x_label and 'xj' in unfold_set.lower():
                    y_label = get_normalized_ylabel(unfold_set, 'y', config)
                elif x_label:
                    y_label = get_normalized_ylabel(unfold_set, 'x', config)
            
            outname = os.path.join(set_dir, f"{hist_name}.png")
            plot_1d_histogram(normalized_hist, outname, config, 
                            title=get_hist_title(hist_name),
                            x_label=x_label, y_label=y_label)
        
        elif hist.GetDimension() == 2:
            # For 2D histograms, plot with TEXT_COLZ
            outname = os.path.join(set_dir, f"{hist_name}_2D.png")
            plot_2d_histogram(hist, outname, config,
                            title=get_hist_title(hist_name),
                            x_label=x_label, y_label=y_label, z_label=z_label)
            
            # Create configurable projections only for unfolded, measured, and truth
            if any(plot_type in hist_name.lower() for plot_type in 
                   ['unfolded', 'measured', 'truth']):
                create_configurable_projections(hist, set_dir, hist_name, config, unfold_set)
    
    # Create comparison plots
    create_comparison_plots(histograms, set_dir, config, unfold_set, test_name)
    
    # Analyze bottomline test results (if available)
    analyze_bottomline_test(objects, unfold_set, test_name, config, output_dir)
    
    # Create convergence analysis plots
    create_convergence_plots(objects, unfold_set, test_name, config, output_dir)
    
    # Create systematic error plots
    create_systematic_error_plots(objects, unfold_set, test_name, config, output_dir)

def create_comparison_plots(histograms, output_dir, config, unfold_set, test_name):
    """Create comparison plots (unfolded vs truth, ratios, etc.)"""
    
    # Find relevant histograms for comparisons
    measured_data = histograms.get(f'h_measured_data_{test_name}')
    measured_mc = histograms.get(f'h_measured_mc_{test_name}')
    truth_mc = histograms.get(f'h_truth_mc_{test_name}')
    unfolded = histograms.get(f'h_unfolded_{test_name}')
    unfolded_corrected = histograms.get(f'h_unfolded_corrected_{test_name}')
    
    # Unfolded vs Truth comparison - prefer corrected unfolded if available
    unfolded_to_use = unfolded_corrected if unfolded_corrected else unfolded

    # Helper to get projection binning from config or default
    def get_proj_bins(axis, default_min, default_max):
        bins_str = config.get(f'{unfold_set}.Projection.{axis}', None)
        if bins_str:
            try:
                parts = [int(x.strip()) for x in bins_str.split(',')]
                if len(parts) == 2:
                    return parts[0], parts[1]
            except Exception:
                pass
        return default_min, default_max

    # Helper to get projections to compare
    def get_proj_axes():
        axes_str = config.get(f'{unfold_set}.ProjectionsToCompare', None)
        if axes_str:
            return [a.strip().lower() for a in axes_str.split(',') if a.strip().lower() in ['x','y','z']]
        # Default: all axes for dimension
        if unfolded_to_use and hasattr(unfolded_to_use, 'GetDimension'):
            dim = unfolded_to_use.GetDimension()
            if dim == 2:
                return ['x','y']
            elif dim == 3:
                return ['x','y','z']
        return ['x']

    if unfolded_to_use and truth_mc:
        dim = unfolded_to_use.GetDimension() if hasattr(unfolded_to_use, 'GetDimension') else 1
        if dim == 1:
            # 1D: keep old behavior
            truth_normalized = normalize_projection_histogram(truth_mc)
            unfolded_normalized = normalize_projection_histogram(unfolded_to_use)
            hists = [truth_normalized, unfolded_normalized]
            labels = ["Truth (MC)", "Unfolded (Corrected)" if unfolded_corrected else "Unfolded"]
            outname = os.path.join(output_dir, f"comparison_unfolded_vs_truth.png")
            x_label = y_label = ""
            if "xj" in unfold_set.lower():
                x_label = get_variable_label('xj')
                y_label = get_normalized_ylabel(unfold_set, 'y', config)
            elif "girth" in unfold_set.lower():
                x_label = get_variable_label('jetGirth')
                y_label = get_normalized_ylabel(unfold_set, 'x', config)
            elif "dyn" in unfold_set.lower():
                x_label = get_variable_label('jetDynDeltaR')
                y_label = get_normalized_ylabel(unfold_set, 'x', config)
            create_overlay_plot(hists, labels, outname, config,
                              title="Unfolded vs Truth Comparison",
                              x_label=x_label, y_label=y_label)
            ratio_outname = os.path.join(output_dir, f"ratio_unfolded_over_truth.png")
            create_ratio_plot(unfolded_normalized, truth_normalized, ratio_outname, config,
                             title="Unfolded / Truth", y_label="Unfolded / Truth")
        else:
            # Multi-dimensional: do projections as specified
            axes = get_proj_axes()
            # Store all projections under a 'projections/' subdirectory
            proj_dir = os.path.join(output_dir, "projections")
            os.makedirs(proj_dir, exist_ok=True)
            for axis in axes:
                # Get binning for this axis
                if axis == 'x':
                    min_bin, max_bin = get_proj_bins('x', 1, unfolded_to_use.GetNbinsY() if dim==2 else unfolded_to_use.GetNbinsY()*unfolded_to_use.GetNbinsZ())
                    proj_unfolded = unfolded_to_use.ProjectionX(f"{unfolded_to_use.GetName()}_projX_{min_bin}_{max_bin}", min_bin, max_bin)
                    proj_truth = truth_mc.ProjectionX(f"{truth_mc.GetName()}_projX_{min_bin}_{max_bin}", min_bin, max_bin)
                    proj_label = 'X'
                elif axis == 'y':
                    min_bin, max_bin = get_proj_bins('y', 1, unfolded_to_use.GetNbinsX() if dim==2 else unfolded_to_use.GetNbinsX()*unfolded_to_use.GetNbinsZ())
                    proj_unfolded = unfolded_to_use.ProjectionY(f"{unfolded_to_use.GetName()}_projY_{min_bin}_{max_bin}", min_bin, max_bin)
                    proj_truth = truth_mc.ProjectionY(f"{truth_mc.GetName()}_projY_{min_bin}_{max_bin}", min_bin, max_bin)
                    proj_label = 'Y'
                elif axis == 'z' and dim == 3:
                    min_bin, max_bin = get_proj_bins('z', 1, unfolded_to_use.GetNbinsX())
                    proj_unfolded = unfolded_to_use.ProjectionZ(f"{unfolded_to_use.GetName()}_projZ_{min_bin}_{max_bin}", min_bin, max_bin)
                    proj_truth = truth_mc.ProjectionZ(f"{truth_mc.GetName()}_projZ_{min_bin}_{max_bin}", min_bin, max_bin)
                    proj_label = 'Z'
                else:
                    continue
                # Normalize
                norm_unfolded = normalize_projection_histogram(proj_unfolded)
                norm_truth = normalize_projection_histogram(proj_truth)
                # Determine bin range string for filename
                def bin_str(minb, maxb):
                    if minb == 1 and (maxb == -1 or maxb < minb):
                        return "all"
                    if maxb == -1:
                        return f"{minb}_end"
                    return f"{minb}_{maxb}"
                bin_range = bin_str(min_bin, max_bin)
                # Save projection plots (individual)
                out_proj_unfolded = os.path.join(proj_dir, f"{unfolded_to_use.GetName()}_proj{proj_label}_{bin_range}.png")
                out_proj_truth = os.path.join(proj_dir, f"{truth_mc.GetName()}_proj{proj_label}_{bin_range}.png")
                x_label = proj_unfolded.GetXaxis().GetTitle()
                y_label = get_normalized_ylabel(unfold_set, axis, config)
                plot_1d_histogram(norm_unfolded, out_proj_unfolded, config, title=f"Unfolded {proj_label} Projection ({bin_range})", x_label=x_label, y_label=y_label)
                plot_1d_histogram(norm_truth, out_proj_truth, config, title=f"Truth {proj_label} Projection ({bin_range})", x_label=x_label, y_label=y_label)
                # Overlay and ratio plots
                hists = [norm_truth, norm_unfolded]
                labels = ["Truth (MC)", "Unfolded (Corrected)" if unfolded_corrected else "Unfolded"]
                outname = os.path.join(proj_dir, f"comparison_unfolded_vs_truth_proj{proj_label}_{bin_range}.png")
                create_overlay_plot(hists, labels, outname, config,
                                  title=f"Unfolded vs Truth Comparison ({proj_label} {bin_range})",
                                  x_label=x_label, y_label=y_label)
                ratio_outname = os.path.join(proj_dir, f"ratio_unfolded_over_truth_proj{proj_label}_{bin_range}.png")
                create_ratio_plot(norm_unfolded, norm_truth, ratio_outname, config,
                                 title=f"Unfolded / Truth ({proj_label} {bin_range})", y_label="Unfolded / Truth")
    
    # Measured vs Truth comparison (only if enabled in config)
    create_measured_truth = int(config.get('Ratio.MeasuredOverTruth', 0))
    if create_measured_truth and measured_mc and truth_mc:
        # Normalize both histograms for comparison
        truth_normalized = normalize_projection_histogram(truth_mc)
        measured_normalized = normalize_projection_histogram(measured_mc)
        
        hists = [truth_normalized, measured_normalized]
        labels = ["Truth (MC)", "Measured (MC)"]
        outname = os.path.join(output_dir, f"comparison_measured_vs_truth.png")
        
        # Get appropriate y-label
        y_label = ""
        if "xj" in unfold_set.lower():
            y_label = get_normalized_ylabel(unfold_set, 'y', config)
        elif "girth" in unfold_set.lower():
            y_label = get_normalized_ylabel(unfold_set, 'x', config)
        elif "dyn" in unfold_set.lower():
            y_label = get_normalized_ylabel(unfold_set, 'x', config)
        
        create_overlay_plot(hists, labels, outname, config,
                          title="Measured vs Truth Comparison",
                          y_label=y_label)
        
        # Create ratio plot using normalized histograms
        ratio_outname = os.path.join(output_dir, f"ratio_measured_over_truth.png")
        create_ratio_plot(measured_normalized, truth_normalized, ratio_outname, config,
                         title="Measured / Truth", y_label="Measured / Truth")

    # Individual efficiency and purity plots are created separately as individual histograms
    # No combined efficiency_purity_comparison plot needed

def store_bayesian_iterations(objects, unfold_set, test_name, config, output_dir):
    # Do not create or save BayesianIterations or plot h_unfolded_iterN directly
    return

def create_configurable_projections(hist_2d, output_dir, hist_name, config, unfold_set):
    """Create projections based on configuration specifications"""
    if hist_2d.GetDimension() != 2:
        return
    
    proj_dir = os.path.join(output_dir, "projections")
    os.makedirs(proj_dir, exist_ok=True)
    
    # Get projection specifications from config
    # Format: UnfoldSet.Projection.X.Bins: min,max or UnfoldSet.Projection.Y.Bins: min,max
    x_proj_bins = config.get(f'{unfold_set}.Projection.X.Bins', '1,-1').split(',')
    y_proj_bins = config.get(f'{unfold_set}.Projection.Y.Bins', '1,-1').split(',')
    
    try:
        x_min = int(x_proj_bins[0]) if x_proj_bins[0] != '-1' else 1
        x_max = int(x_proj_bins[1]) if len(x_proj_bins) > 1 and x_proj_bins[1] != '-1' else -1
        y_min = int(y_proj_bins[0]) if y_proj_bins[0] != '-1' else 1
        y_max = int(y_proj_bins[1]) if len(y_proj_bins) > 1 and y_proj_bins[1] != '-1' else -1
    except (ValueError, IndexError):
        # Default to full projection if config parsing fails
        x_min, x_max = 1, -1
        y_min, y_max = 1, -1
    
    # Get axis labels from config for the unfolding set
    x_axis_label, y_axis_label, _ = get_axis_labels_from_config(unfold_set, hist_name, config)
    
    # Create X projection
    if x_max == -1:
        x_max = hist_2d.GetNbinsY()
    proj_x = hist_2d.ProjectionX(f"{hist_name}_projX", y_min, y_max)
    
    if proj_x and proj_x.GetEntries() > 0:
        # Normalize the projection by bin width and integral
        normalized_proj_x = normalize_projection_histogram(proj_x)
        
        outname = os.path.join(proj_dir, f"{hist_name}_projX.png")
        proj_x_label = x_axis_label if x_axis_label else hist_2d.GetXaxis().GetTitle()
        proj_y_label = get_normalized_ylabel(unfold_set, 'x', config)
        
        plot_1d_histogram(normalized_proj_x, outname, config,
                        title=f"X Projection (Y bins {y_min}-{y_max})",
                        x_label=proj_x_label, y_label=proj_y_label)
    
    # Create Y projection
    if y_max == -1:
        y_max = hist_2d.GetNbinsX()
    proj_y = hist_2d.ProjectionY(f"{hist_name}_projY", x_min, x_max)
    
    if proj_y and proj_y.GetEntries() > 0:
        # Normalize the projection by bin width and integral
        normalized_proj_y = normalize_projection_histogram(proj_y)
        
        outname = os.path.join(proj_dir, f"{hist_name}_projY.png")
        proj_x_label = y_axis_label if y_axis_label else hist_2d.GetYaxis().GetTitle()
        proj_y_label = get_normalized_ylabel(unfold_set, 'y', config)
        
        plot_1d_histogram(normalized_proj_y, outname, config,
                        title=f"Y Projection (X bins {x_min}-{x_max})",
                        x_label=proj_x_label, y_label=proj_y_label)

def create_convergence_plots(objects, unfold_set, test_name, config, output_dir):
    """Create only ratio_unfolded_over_truth (and projections) for each iteration in convergence_analysis"""
    conv_dir = os.path.join(output_dir, unfold_set, test_name, "convergence_analysis")
    os.makedirs(conv_dir, exist_ok=True)
    histograms = objects['histograms']
    iter_str = config.get('IterationOverlay.Iterations', '')
    if not iter_str:
        return
    iter_nums = [int(x.strip()) for x in iter_str.split(',') if x.strip().isdigit()]
    # Overlay all ratio_unfolded_over_truth for all iterations on a single plot (1D and 2D projections)
    ratio_hists_1d = []
    ratio_labels_1d = []
    ratio_hists_proj = {'x': [], 'y': []}
    ratio_labels_proj = {'x': [], 'y': []}
    histograms = objects['histograms']
    iter_str = config.get('IterationOverlay.Iterations', '')
    if not iter_str:
        return
    iter_nums = [int(x.strip()) for x in iter_str.split(',') if x.strip().isdigit()]
    truth_hist = histograms.get(f'h_truth_mc_{test_name}') or histograms.get('h_truth_mc')
    if not truth_hist:
        return
    dim = None
    for iter_num in iter_nums:
        unfolded_hist = None
        for hname in histograms:
            if re.match(r"h_unfolded_iter{}$".format(iter_num), hname) or re.match(r"h_unfolded_iteration_?{}$".format(iter_num), hname):
                unfolded_hist = histograms[hname]
                break
        if not unfolded_hist:
            continue
        if dim is None:
            dim = unfolded_hist.GetDimension() if hasattr(unfolded_hist, 'GetDimension') else 1
        if dim == 1:
            norm_unfolded = normalize_projection_histogram(unfolded_hist)
            norm_truth = normalize_projection_histogram(truth_hist)
            ratio_hist = norm_unfolded.Clone(f"ratio_unfolded_over_truth_iter{iter_num}")
            ratio_hist.Divide(norm_truth)
            ratio_hists_1d.append(ratio_hist)
            ratio_labels_1d.append(f"Iter {iter_num}")
        elif dim == 2:
            # X projection
            proj_unfolded_x = unfolded_hist.ProjectionX()
            proj_truth_x = truth_hist.ProjectionX()
            norm_unfolded_x = normalize_projection_histogram(proj_unfolded_x)
            norm_truth_x = normalize_projection_histogram(proj_truth_x)
            ratio_proj_x = norm_unfolded_x.Clone(f"ratio_unfolded_over_truth_projX_iter{iter_num}")
            ratio_proj_x.Divide(norm_truth_x)
            ratio_hists_proj['x'].append(ratio_proj_x)
            ratio_labels_proj['x'].append(f"Iter {iter_num}")
            # Y projection
            proj_unfolded_y = unfolded_hist.ProjectionY()
            proj_truth_y = truth_hist.ProjectionY()
            norm_unfolded_y = normalize_projection_histogram(proj_unfolded_y)
            norm_truth_y = normalize_projection_histogram(proj_truth_y)
            ratio_proj_y = norm_unfolded_y.Clone(f"ratio_unfolded_over_truth_projY_iter{iter_num}")
            ratio_proj_y.Divide(norm_truth_y)
            ratio_hists_proj['y'].append(ratio_proj_y)
            ratio_labels_proj['y'].append(f"Iter {iter_num}")
    # 1D overlay
    if dim == 1 and ratio_hists_1d:
        x_label = ''
        if 'get_variable_label' in globals():
            x_label = get_variable_label('_'.join(unfold_set.split('_')[1:]))
        if not x_label and ratio_hists_1d[0].GetXaxis():
            x_label = ratio_hists_1d[0].GetXaxis().GetTitle()
        outname = os.path.join(conv_dir, "ratio_unfolded_over_truth_overlay.png")
        create_overlay_plot(ratio_hists_1d, ratio_labels_1d, outname, config,
                          title="Unfolded / Truth (All Iterations)", x_label=x_label, y_label="Unfolded / Truth")
    # 2D overlays for projections
    if dim == 2:
        if ratio_hists_proj['x']:
            x_label = truth_hist.GetXaxis().GetTitle() if truth_hist.GetXaxis() else 'X'
            outname = os.path.join(conv_dir, "ratio_unfolded_over_truth_projX_overlay.png")
            create_overlay_plot(ratio_hists_proj['x'], ratio_labels_proj['x'], outname, config,
                              title="Unfolded / Truth X Projection (All Iterations)", x_label=x_label, y_label="Unfolded / Truth")
        if ratio_hists_proj['y']:
            y_label = truth_hist.GetYaxis().GetTitle() if truth_hist.GetYaxis() else 'Y'
            outname = os.path.join(conv_dir, "ratio_unfolded_over_truth_projY_overlay.png")
            create_overlay_plot(ratio_hists_proj['y'], ratio_labels_proj['y'], outname, config,
                              title="Unfolded / Truth Y Projection (All Iterations)", x_label=y_label, y_label="Unfolded / Truth")

def create_iteration_overlays(objects, unfold_set, test_name, config, output_dir):
    """Create overlay plots of different Bayesian iterations"""
    histograms = objects['histograms']
    
    # Group histograms by iteration
    iteration_hists = defaultdict(list)
    iteration_labels = defaultdict(list)
    
    for hist_name, hist in histograms.items():
        # Look for iteration patterns in histogram names
        if 'iter' in hist_name.lower() and hist:
            # Extract iteration number
            iter_match = re.search(r'iter(?:ation)?_?(\d+)', hist_name.lower())
            if iter_match:
                iter_num = int(iter_match.group(1))
                
                # Get base histogram type (remove iteration info)
                base_type = hist_name.lower().replace(f'_iter{iter_num}', '').replace(f'_iteration{iter_num}', '')
                base_type = base_type.replace(f'iter{iter_num}_', '').replace(f'iteration{iter_num}_', '')
                
                iteration_hists[base_type].append((iter_num, hist))
                iteration_labels[base_type].append(f"Iteration {iter_num}")
    
    # Create overlay plots for each histogram type
    for base_type, hist_iter_pairs in iteration_hists.items():
        if len(hist_iter_pairs) > 1:
            # Sort by iteration number
            hist_iter_pairs.sort(key=lambda x: x[0])
            hists = [pair[1] for pair in hist_iter_pairs]
            labels = [f"Iter {pair[0]}" for pair in hist_iter_pairs]
            
            outname = os.path.join(output_dir, f"{base_type}_iteration_comparison.png")
            create_overlay_plot(hists, labels, outname, config,
                              title=f"{get_hist_title(base_type)} - Iteration Comparison",
                              color_scheme='iteration_comparison')

def create_systematic_error_plots(objects, unfold_set, test_name, config, output_dir):
    """Create systematic error analysis plots"""
    # Do not create or save systematic_errors directory
    return

def create_binned_projections(hist_2d, output_dir, hist_name, config):
    """Create projections in specific bin ranges (like the reference script)"""
    if hist_2d.GetDimension() != 2:
        return
    
    proj_dir = os.path.join(output_dir, "binned_projections")
    os.makedirs(proj_dir, exist_ok=True)
    
    # Create projections for different bin ranges
    nx_bins = hist_2d.GetNbinsX()
    ny_bins = hist_2d.GetNbinsY()
    
    # Create multiple X projections for different Y ranges
    for y_start in range(1, ny_bins, 2):  # Every other bin
        y_end = min(y_start + 1, ny_bins)
        proj_x = hist_2d.ProjectionX(f"{hist_name}_projX_Y{y_start}to{y_end}", 
                                   y_start, y_end)
        
        if proj_x.GetEntries() > 0:
            outname = os.path.join(proj_dir, f"{hist_name}_projX_Y{y_start}to{y_end}.png")
            
            # Get appropriate axis labels
            x_label = hist_2d.GetXaxis().GetTitle()
            y_label = "Entries"
            
            plot_1d_histogram(proj_x, outname, config,
                            title=f"X Projection (Y bins {y_start}-{y_end})",
                            x_label=x_label, y_label=y_label)
    
    # Create multiple Y projections for different X ranges
    for x_start in range(1, nx_bins, 2):  # Every other bin
        x_end = min(x_start + 1, nx_bins)
        proj_y = hist_2d.ProjectionY(f"{hist_name}_projY_X{x_start}to{x_end}", 
                                   x_start, x_end)
        
        if proj_y.GetEntries() > 0:
            outname = os.path.join(proj_dir, f"{hist_name}_projY_X{x_start}to{x_end}.png")
            
            # Get appropriate axis labels
            x_label = hist_2d.GetYaxis().GetTitle()
            y_label = "Entries"
            
            plot_1d_histogram(proj_y, outname, config,
                            title=f"Y Projection (X bins {x_start}-{x_end})",
                            x_label=x_label, y_label=y_label)

def normalize_histogram_by_width(hist):
    """Normalize histogram by bin width using ROOT's Scale method"""
    normalized = hist.Clone(f"{hist.GetName()}_normalized")
    normalized.Scale(1.0, "width")  # Use ROOT's built-in width normalization
    return normalized

def normalize_projection_histogram(hist):
    """Normalize projection histogram by both bin width and integral"""
    normalized = hist.Clone(f"{hist.GetName()}_normalized")
    
    # First normalize by bin width using ROOT's built-in method
    normalized.Scale(1.0, "width")
    
    # Then normalize by integral
    integral = normalized.Integral()
    if integral > 0:
        normalized.Scale(1.0 / integral)
    
    return normalized

def create_method_overlays(all_files_objects, unfold_set, test_name, config, output_dir):
    """Create overlay plots comparing different unfolding methods across files"""
    method_dir = os.path.join(output_dir, unfold_set, test_name, "method_comparisons")
    os.makedirs(method_dir, exist_ok=True)
    
    # Extract method from filename and group histograms
    method_hists = defaultdict(list)
    method_labels = defaultdict(list)
    
    for file_path, file_objects in all_files_objects.items():
        if unfold_set in file_objects and test_name in file_objects[unfold_set]:
            # Extract method from filename
            method = "Unknown"
            if "Bayes" in file_path or "bayes" in file_path:
                method = "Bayes"
            elif "Invert" in file_path or "Matrix" in file_path:
                method = "Matrix Inversion"
            elif "SVD" in file_path or "svd" in file_path:
                method = "SVD"
            elif "BinByBin" in file_path:
                method = "Bin-by-Bin"
            
            histograms = file_objects[unfold_set][test_name]['histograms']
            
            for hist_name, hist in histograms.items():
                if hist and not ('iter' in hist_name.lower()):  # Skip iteration-specific hists
                    method_hists[hist_name].append(hist)
                    method_labels[hist_name].append(method)
    
    # Create overlay plots for each histogram type
    for hist_name, hists in method_hists.items():
        if len(hists) > 1:
            labels = method_labels[hist_name]
            outname = os.path.join(method_dir, f"{hist_name}_method_comparison.png")
            create_overlay_plot(hists, labels, outname, config,
                              title=f"{get_hist_title(hist_name)} - Method Comparison",
                              color_scheme='method_comparison')

def create_method_overlays(all_files_objects, unfold_set, test_name, config, output_dir):
    """Create overlay plots comparing different unfolding methods"""
    overlay_dir = os.path.join(output_dir, unfold_set, "method_comparison")
    os.makedirs(overlay_dir, exist_ok=True)
    
    logger.info(f"Creating method comparison plots for {unfold_set}/{test_name}")
    
    # Collect histograms from all files (methods)
    method_histograms = {}
    method_labels = []
    
    for file_path, objects in all_files_objects.items():
        # Extract method name from file path or use filename
        method_name = os.path.basename(file_path).replace('.root', '')
        if test_name in objects and unfold_set in objects[test_name]:
            test_objects = objects[test_name][unfold_set]
            histograms = test_objects.get('histograms', {})
            
            # Look for unfolded histogram
            unfolded_key = f'h_unfolded_{test_name}'
            unfolded_corrected_key = f'h_unfolded_corrected_{test_name}'
            
            unfolded = histograms.get(unfolded_corrected_key) or histograms.get(unfolded_key)
            if unfolded:
                method_histograms[method_name] = unfolded
                method_labels.append(method_name)
    
    if len(method_histograms) > 1:
        # Create overlay plot
        hists = [normalize_projection_histogram(hist) for hist in method_histograms.values()]
        outname = os.path.join(overlay_dir, f"method_comparison_{test_name}.png")
        
        # Determine axis labels
        x_label = get_variable_label(unfold_set.split('_')[0])
        y_label = get_normalized_ylabel(unfold_set, 'x', config)
        
        create_overlay_plot(hists, method_labels, outname, config,
                          title=f"Method Comparison - {unfold_set}",
                          x_label=x_label, y_label=y_label)
        
        logger.debug(f"Created method comparison plot for {unfold_set}/{test_name}")

# ========== Bottomline Test Analysis ==========
def analyze_bottomline_test(objects, unfold_set, test_name, config, output_dir):
    """Analyze and plot bottomline test results"""
    if 'bottomline' not in objects or not objects['bottomline']:
        logger.debug(f"No bottomline test results found for {unfold_set}/{test_name}")
        return
    
    bottomline_dir = os.path.join(output_dir, unfold_set, test_name, "bottomline_test")
    os.makedirs(bottomline_dir, exist_ok=True)
    
    logger.info(f"Analyzing bottomline test results for {unfold_set}/{test_name}")
    
    # Extract bottomline text results
    bottomline_data = objects['bottomline']
    
    # Save summary text files
    for result_name, result_text in bottomline_data.items():
        if isinstance(result_text, str):
            summary_file = os.path.join(bottomline_dir, f"{result_name}.txt")
            with open(summary_file, 'w') as f:
                f.write(result_text)
            logger.debug(f"Saved bottomline summary to {summary_file}")
    
    # Look for bottomline histograms and graphs
    histograms = objects.get('histograms', {})
    
    # Save forward-folded model histogram
    forward_folded = histograms.get(f'h_forward_folded_{test_name}')
    if forward_folded:
        outname = os.path.join(bottomline_dir, "forward_folded_model.png")
        x_label = get_variable_label(unfold_set.split('_')[0])
        y_label = "Events"
        plot_1d_histogram(forward_folded, outname, config,
                         title="Forward Folded Model (K × λ')",
                         x_label=x_label, y_label=y_label)
    
    # Create graphical bottomline test (data vs forward-folded model overlay)
    data_hist = histograms.get(f'h_measured_data_{test_name}')
    if data_hist and forward_folded:
        hists = [data_hist, forward_folded]
        labels = ["Data (Measured)", "Model (Forward Folded)"]
        outname = os.path.join(bottomline_dir, "bottomline_overlay.png")
        
        x_label = get_variable_label(unfold_set.split('_')[0])
        y_label = "Events"
        
        create_overlay_plot(hists, labels, outname, config,
                          title="Bottomline Test: Data vs Forward-Folded Model",
                          x_label=x_label, y_label=y_label)
        
        # Create residuals plot
        if hasattr(data_hist, 'GetNbinsX'):  # Check if it's a histogram
            residuals_name = os.path.join(bottomline_dir, "bottomline_residuals.png")
            create_bottomline_residuals_plot(data_hist, forward_folded, residuals_name, config)
    
    # Handle p-value vs iteration plots (if Bayesian unfolding)
    pvalue_graphs = {name: obj for name, obj in histograms.items() 
                    if 'pvalue' in name.lower() and hasattr(obj, 'GetN')}
    
    if pvalue_graphs:
        create_bottomline_pvalue_plots(pvalue_graphs, bottomline_dir, config, test_name)
    
    # Parse numerical results if available
    numbers_key = f'bottomline_numbers_{test_name}'
    if numbers_key in bottomline_data:
        create_bottomline_summary_table(bottomline_data[numbers_key], bottomline_dir, test_name)

def create_bottomline_residuals_plot(data_hist, model_hist, outname, config):
    """Create residuals/pulls plot for bottomline test"""
    try:
        canvas = create_canvas("c_bottomline_residuals", "Bottomline Residuals", 800, 600, config)
        
        # Create residuals histogram
        residuals_hist = data_hist.Clone("residuals")
        residuals_hist.Add(model_hist, -1)  # residuals = data - model
        
        # Convert to pulls if possible
        for i in range(1, residuals_hist.GetNbinsX() + 1):
            error = data_hist.GetBinError(i)
            if error > 0:
                residual = residuals_hist.GetBinContent(i)
                pull = residual / error
                residuals_hist.SetBinContent(i, pull)
                residuals_hist.SetBinError(i, 1.0)  # Pull errors are 1
        
        residuals_hist.SetTitle("Bottomline Test: Pulls (Data - Model) / σ")
        residuals_hist.GetYaxis().SetTitle("Pull")
        residuals_hist.SetLineColor(ROOT.kBlack)
        residuals_hist.SetMarkerColor(ROOT.kBlack)
        residuals_hist.SetMarkerStyle(20)
        
        residuals_hist.Draw("EP")
        
        # Add zero line
        x_min = residuals_hist.GetXaxis().GetXmin()
        x_max = residuals_hist.GetXaxis().GetXmax()
        zero_line = ROOT.TLine(x_min, 0, x_max, 0)
        zero_line.SetLineColor(ROOT.kRed)
        zero_line.SetLineStyle(2)
        zero_line.Draw("SAME")
        
        # Add ±2σ lines
        plus_two = ROOT.TLine(x_min, 2, x_max, 2)
        minus_two = ROOT.TLine(x_min, -2, x_max, -2)
        plus_two.SetLineColor(ROOT.kBlue)
        minus_two.SetLineColor(ROOT.kBlue)
        plus_two.SetLineStyle(3)
        minus_two.SetLineStyle(3)
        plus_two.Draw("SAME")
        minus_two.Draw("SAME")
        
        add_cms_label(canvas, config)
        canvas.SaveAs(outname)
        canvas.Close()
        
        logger.debug(f"Created bottomline residuals plot: {outname}")
    except Exception as e:
        logger.error(f"Failed to create bottomline residuals plot: {e}")

def create_bottomline_pvalue_plots(pvalue_graphs, output_dir, config, test_name):
    """Create p-value vs iteration plots for bottomline test"""
    try:
        canvas = create_canvas("c_bottomline_pvalue", "Bottomline p-value", 800, 600, config)
        
        colors = [ROOT.kBlue, ROOT.kRed, ROOT.kGreen+2, ROOT.kMagenta]
        legend = create_legend(0.65, 0.15, 0.9, 0.4)
        
        first_graph = True
        for i, (name, graph) in enumerate(pvalue_graphs.items()):
            graph.SetLineColor(colors[i % len(colors)])
            graph.SetMarkerColor(colors[i % len(colors)])
            graph.SetLineWidth(2)
            graph.SetMarkerStyle(20 + i)
            
            # Determine label from graph name
            if 'smeared' in name:
                label = "Measured space"
            elif 'unfolded' in name:
                label = "Unfolded space"
            else:
                label = name.replace('_', ' ').title()
            
            draw_option = "ALP" if first_graph else "LP SAME"
            graph.Draw(draw_option)
            
            if first_graph:
                graph.GetXaxis().SetTitle("Iteration")
                graph.GetYaxis().SetTitle("p-value")
                graph.SetTitle("Bottomline Test: p-value vs Iteration")
                first_graph = False
            
            legend.AddEntry(graph, label, "lp")
        
        # Add threshold line if we can determine it from config
        threshold = 0.95  # Default threshold
        if hasattr(config, 'get'):
            threshold = float(config.get('BottomlinePValueThreshold', 0.95))
        
        x_max = 20  # Default, should be determined from graphs
        for graph in pvalue_graphs.values():
            if hasattr(graph, 'GetN') and graph.GetN() > 0:
                x_max = max(x_max, graph.GetX()[graph.GetN()-1])
        
        threshold_line = ROOT.TLine(1, threshold, x_max, threshold)
        threshold_line.SetLineColor(ROOT.kGreen)
        threshold_line.SetLineStyle(2)
        threshold_line.SetLineWidth(2)
        threshold_line.Draw("SAME")
        
        legend.AddEntry(threshold_line, "Threshold", "l")
        legend.Draw()
        
        add_cms_label(canvas, config)
        
        outname = os.path.join(output_dir, "pvalue_vs_iteration.png")
        canvas.SaveAs(outname)
        canvas.Close()
        
        logger.debug(f"Created bottomline p-value plot: {outname}")
    except Exception as e:
        logger.error(f"Failed to create bottomline p-value plot: {e}")

def create_bottomline_summary_table(numbers_text, output_dir, test_name):
    """Create a formatted summary table from bottomline numerical results"""
    try:
        # Parse the numbers text
        results = {}
        for line in numbers_text.strip().split('\n'):
            if ' ' in line:
                key, value = line.split(' ', 1)
                try:
                    results[key] = float(value)
                except ValueError:
                    results[key] = value
        
        # Create formatted summary
        summary_file = os.path.join(output_dir, f"bottomline_summary_table_{test_name}.txt")
        with open(summary_file, 'w') as f:
            f.write("=" * 60 + "\n")
            f.write(f"BOTTOMLINE TEST SUMMARY - {test_name.upper()}\n")
            f.write("=" * 60 + "\n\n")
            
            # Measured space results
            if 'chi2_smeared' in results:
                f.write("MEASURED SPACE:\n")
                f.write(f"  χ² = {results.get('chi2_smeared', 'N/A'):.3f}\n")
                f.write(f"  NDF = {int(results.get('ndf_smeared', 0))}\n")
                f.write(f"  p-value = {results.get('pvalue_smeared', 'N/A'):.4f}\n\n")
            
            # Unfolded space results
            if 'chi2_unfolded' in results and results['chi2_unfolded'] >= 0:
                f.write("UNFOLDED SPACE:\n")
                f.write(f"  χ² = {results.get('chi2_unfolded', 'N/A'):.3f}\n")
                f.write(f"  NDF = {int(results.get('ndf_unfolded', 0))}\n")
                f.write(f"  p-value = {results.get('pvalue_unfolded', 'N/A'):.4f}\n\n")
            
            # Optimal iteration
            if 'optimal_iteration' in results and results['optimal_iteration'] > 0:
                f.write("BAYESIAN OPTIMIZATION:\n")
                f.write(f"  Optimal iteration: {int(results['optimal_iteration'])}\n\n")
            
            # Interpretation
            f.write("INTERPRETATION:\n")
            pval_smeared = results.get('pvalue_smeared', -1)
            if pval_smeared >= 0:
                if pval_smeared < 0.05:
                    f.write("  ⚠ Poor agreement in measured space (p < 0.05)\n")
                elif pval_smeared < 0.1:
                    f.write("  ⚠ Marginal agreement in measured space (p < 0.1)\n")
                else:
                    f.write("  ✓ Good agreement in measured space\n")
            
            pval_unfolded = results.get('pvalue_unfolded', -1)
            if pval_unfolded >= 0:
                if pval_unfolded < 0.05:
                    f.write("  ⚠ Poor agreement in unfolded space (p < 0.05)\n")
                elif pval_unfolded < 0.1:
                    f.write("  ⚠ Marginal agreement in unfolded space (p < 0.1)\n")
                else:
                    f.write("  ✓ Good agreement in unfolded space\n")
        
        logger.debug(f"Created bottomline summary table: {summary_file}")
    except Exception as e:
        logger.error(f"Failed to create bottomline summary table: {e}")

# ========== Main Logic ==========
def main():
    parser = argparse.ArgumentParser(
        description="Plot unfolding results from ROOT files with CMS style.")
    parser.add_argument('-c', '--config', required=True, 
                       help='Plotting config file')
    parser.add_argument('-i', '--inputs', nargs='+', required=True, 
                       help='Input ROOT files from unfolding')
    parser.add_argument('-o', '--output', default='./UnfoldingPlots', 
                       help='Output directory for plots')
    parser.add_argument('--test', action='store_true',
                       help='Test mode: process limited number of plots')
    parser.add_argument('-v', '--verbose', action='count', default=0,
                       help='Increase verbosity (-v for DEBUG, -vv for TRACE). Default is INFO level.')
    parser.add_argument('--compare-methods', action='store_true',
                       help='Compare different unfolding methods across input files')
    args = parser.parse_args()

    # Setup ROOT
    ROOT.gROOT.SetBatch(True)
    ROOT.gErrorIgnoreLevel = ROOT.kWarning
    
    # Configure logging based on verbosity (following plotGammaJet.py pattern)
    if HAS_PLOT_HELPERS:
        if args.verbose >= 2:
            plot_helpers.set_verbosity(plot_helpers.Logger.TRACE)   # -vv gives TRACE (most verbose)
        elif args.verbose >= 1:
            plot_helpers.set_verbosity(plot_helpers.Logger.DEBUG)   # -v gives DEBUG
        else:
            plot_helpers.set_verbosity(plot_helpers.Logger.INFO)    # Default is INFO level
    
    # Load TDR style
    load_tdr_style()
    
    # Parse configuration
    config = parse_config(args.config)
    
    # Create output directory
    os.makedirs(args.output, exist_ok=True)
    
    logger.info(f"Processing {len(args.inputs)} input file(s)...")
    logger.info(f"Output directory: {args.output}")
    
    # Store all objects from all files for method comparisons
    all_files_objects = {}
    
    # Process each input file
    for input_file in args.inputs:
        logger.info(f"\nProcessing: {input_file}")
        
        f = ROOT.TFile.Open(input_file)
        if not f or f.IsZombie():
            logger.error(f"Cannot open {input_file}")
            continue
        
        # Get unfolding sets
        unfold_sets = get_unfolding_dirs(f)
        if not unfold_sets:
            logger.warning("No unfolding sets found in file")
            f.Close()
            continue
        
        logger.info(f"Found unfolding sets: {unfold_sets}")
        
        # Store all objects for multi-test and multi-file comparisons
        file_objects = defaultdict(lambda: defaultdict(dict))
        
        # Process each unfolding set
        for unfold_set in unfold_sets:
            logger.debug(f"  Processing unfolding set: {unfold_set}")
            
            unfold_dir = f.Get(unfold_set)
            if not unfold_dir:
                continue
            
            # Get test directories
            tests = get_test_dirs(unfold_dir)
            if not tests:
                continue
            
            logger.debug(f"    Found tests: {tests}")
            
            # Process each test
            for test in tests:
                logger.debug(f"    Processing test: {test}")
                
                test_dir = unfold_dir.Get(test)
                if not test_dir:
                    continue
                
                # Extract all objects
                objects = extract_histograms_and_canvases(test_dir)
                file_objects[unfold_set][test] = objects
                
                logger.debug(f"      Found {len(objects['histograms'])} histograms, "
                          f"{len(objects['canvases'])} canvases")
                
                # Analyze this set and test
                analyze_unfolding_set(objects, unfold_set, test, config, args.output)
                
                if args.test:
                    break  # Test mode: only process first test
            
            if args.test:
                break  # Test mode: only process first unfolding set
        
        # Store file objects for multi-file comparisons
        all_files_objects[input_file] = file_objects
        
        f.Close()
    
    # Create method comparison plots across files
    if args.compare_methods and len(all_files_objects) > 1:
        logger.info("\nCreating method comparison plots...")
        
        # Find common unfolding sets and tests across files
        common_sets = set.intersection(*[set(file_obj.keys()) for file_obj in all_files_objects.values()])
        
        for unfold_set in common_sets:
            common_tests = set.intersection(*[set(file_obj[unfold_set].keys()) 
                                            for file_obj in all_files_objects.values()])
            
            for test in common_tests:
                create_method_overlays(all_files_objects, unfold_set, test, config, args.output)
    
    logger.info(f"\nPlotting completed. Results saved to: {args.output}")

if __name__ == "__main__":
    main()
