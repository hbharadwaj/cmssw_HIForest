#!/usr/bin/env python3
"""
CMS Gamma-Jet Analysis Enhanced Plotting Script

This script provides comprehensive plotting capabilities for gamma-jet analysis
results, including support for nested ROOT file structures, overlay plots, 
ratio plots, and CMS-compliant styling.

Author: Generated for CMS Heavy Ion Analysis
Date: 2025-05-25
"""

import os
import sys
import ROOT
import argparse
import re



def parse_config(config_path):
    """Parse the plotting config file into a dictionary of keys and values."""
    config = {}
    with open(config_path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            if ':' in line:
                key, value = line.split(':', 1)
                config[key.strip()] = value.strip()
    return config


class HistogramConfig:
    """Class to hold histogram configuration."""
    def __init__(self, name, config_dict, plot_key):
        self.name = name
        self.plot_key = plot_key
        self.title = config_dict.get(f'Histogram.{plot_key}.Title', '')
        self.bins = int(config_dict.get(f'Histogram.{plot_key}.Bins', 100))
        self.x_min = float(config_dict.get(f'Histogram.{plot_key}.XMin', 0))
        self.x_max = float(config_dict.get(f'Histogram.{plot_key}.XMax', 100))
        self.log_y = config_dict.get(f'Histogram.{plot_key}.LogY', '0') == '1'
        self.plot_type = config_dict.get(f'Histogram.{plot_key}.PlotType', '1D')
        self.color = config_dict.get(f'Histogram.{plot_key}.Color', 'auto')
        self.line_width = int(config_dict.get(f'Histogram.{plot_key}.LineWidth', 2))
        self.draw_option = config_dict.get(f'Histogram.{plot_key}.DrawOption', 'hist')
        self.marker_style = config_dict.get(f'Histogram.{plot_key}.MarkerStyle', 'auto')
        self.marker_size = float(config_dict.get(f'Histogram.{plot_key}.MarkerSize', 0.8))
        
        # For 2D histograms
        self.x_bins = int(config_dict.get(f'Histogram.{plot_key}.XBins', self.bins))
        self.y_bins = int(config_dict.get(f'Histogram.{plot_key}.YBins', 100))
        self.y_min = float(config_dict.get(f'Histogram.{plot_key}.YMin', 0))
        self.y_max = float(config_dict.get(f'Histogram.{plot_key}.YMax', 100))
        self.log_z = config_dict.get(f'Histogram.{plot_key}.LogZ', '0') == '1'
        self.color_map = config_dict.get(f'Histogram.{plot_key}.ColorMap', 'default')
        self.draw_option_2d = config_dict.get(f'Histogram.{plot_key}.DrawOption2D', 'colz')


class ProfileConfig:
    """Class to hold profile configuration."""
    def __init__(self, name, config_dict, plot_key):
        self.name = name
        self.plot_key = plot_key
        self.title = config_dict.get(f'Profile.{plot_key}.Title', '')
        self.x_bins = int(config_dict.get(f'Profile.{plot_key}.XBins', 100))
        self.x_min = float(config_dict.get(f'Profile.{plot_key}.XMin', 0))
        self.x_max = float(config_dict.get(f'Profile.{plot_key}.XMax', 100))


class DataMCConfig:
    """Class to hold Data-MC comparison configuration."""
    def __init__(self, config_dict):
        # Data styling
        self.data_color = int(config_dict.get('DataMC.DataColor', 1))  # Black
        self.data_marker_style = int(config_dict.get('DataMC.DataMarkerStyle', 20))  # Full circle
        self.data_marker_size = float(config_dict.get('DataMC.DataMarkerSize', 0.8))
        self.data_line_width = int(config_dict.get('DataMC.DataLineWidth', 2))
        self.data_label = config_dict.get('DataMC.DataLabel', 'Data')
        
        # MC styling
        self.mc_color = int(config_dict.get('DataMC.MCColor', 2))  # Red
        self.mc_marker_style = int(config_dict.get('DataMC.MCMarkerStyle', 21))  # Full square
        self.mc_marker_size = float(config_dict.get('DataMC.MCMarkerSize', 0.8))
        self.mc_line_width = int(config_dict.get('DataMC.MCLineWidth', 2))
        self.mc_label = config_dict.get('DataMC.MCLabel', 'MC')
        
        # Ratio plot settings
        self.ratio_y_min = float(config_dict.get('DataMC.RatioYMin', 0.5))
        self.ratio_y_max = float(config_dict.get('DataMC.RatioYMax', 1.5))
        self.ratio_title = config_dict.get('DataMC.RatioTitle', 'Data/MC')
        self.ratio_color = int(config_dict.get('DataMC.RatioColor', 1))
        self.ratio_marker_style = int(config_dict.get('DataMC.RatioMarkerStyle', 20))
        self.ratio_marker_size = float(config_dict.get('DataMC.RatioMarkerSize', 0.6))
        
        # Canvas settings
        self.canvas_width = int(config_dict.get('DataMC.CanvasWidth', 800))
        self.canvas_height = int(config_dict.get('DataMC.CanvasHeight', 800))
        self.upper_pad_height = float(config_dict.get('DataMC.UpperPadHeight', 0.7))
        self.lower_pad_height = float(config_dict.get('DataMC.LowerPadHeight', 0.3))
        
        # Legend settings
        self.legend_x1 = float(config_dict.get('DataMC.LegendX1', 0.65))
        self.legend_y1 = float(config_dict.get('DataMC.LegendY1', 0.75))
        self.legend_x2 = float(config_dict.get('DataMC.LegendX2', 0.85))
        self.legend_y2 = float(config_dict.get('DataMC.LegendY2', 0.85))
        
        # Plot scaling settings
        self.log_y = config_dict.get('DataMC.LogY', '0') == '1'


def get_histogram_configs(config):
    """Return a dict of {plot_key: HistogramConfig} for all defined histograms."""
    hist_configs = {}
    for key, value in config.items():
        if key.startswith('Histogram.') and key.endswith('.Name'):
            plot_key = key.split('.')[1]
            hist_configs[plot_key] = HistogramConfig(value, config, plot_key)
    return hist_configs


def get_profile_configs(config):
    """Return a dict of {plot_key: ProfileConfig} for all defined profiles."""
    profile_configs = {}
    for key, value in config.items():
        if key.startswith('Profile.') and key.endswith('.Name'):
            plot_key = key.split('.')[1]
            profile_configs[plot_key] = ProfileConfig(value, config, plot_key)
    return profile_configs


def get_datamc_config(config):
    """Return DataMCConfig from the configuration."""
    return DataMCConfig(config)


def get_overlay_plots(config):
    """Return a list of plot_keys to overlay."""
    overlays = config.get('OverlayPlots', '')
    return [x.strip() for x in overlays.split(',') if x.strip()]


def get_plot_formats(config):
    """Return a list of output formats."""
    return [fmt.strip() for fmt in config.get('PlotFormats', 'png').split(',')]


def get_color_scheme(scheme_name, color_blind=False):
    """Return color palette based on scheme name."""
    color_schemes = {
        'petroff6': [
            ROOT.kBlack,
            ROOT.TColor.GetColor("#5790fc"),  # Blue
            ROOT.TColor.GetColor("#e42536"),  # Red
            ROOT.TColor.GetColor("#964a8b"),  # Purple
            ROOT.TColor.GetColor("#9c9ca1"),  # Gray
            ROOT.TColor.GetColor("#f89c20"),  # Orange
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
        'traditional': [ROOT.kBlack, ROOT.kBlue, ROOT.kRed, ROOT.kMagenta, ROOT.kGreen+2, ROOT.kOrange, ROOT.kCyan, ROOT.kYellow+2, ROOT.kGray+2],
        'viridis': [ROOT.kBlue+2, ROOT.kAzure+7, ROOT.kTeal+2, ROOT.kGreen+2, ROOT.kYellow+1],
        'default': [ROOT.kBlack, ROOT.kBlue, ROOT.kRed, ROOT.kMagenta, ROOT.kGreen+2, ROOT.kOrange, ROOT.kCyan, ROOT.kYellow+2, ROOT.kGray+2]
    }
    
    if color_blind:
        return [
            ROOT.kBlack,
            ROOT.TColor.GetColor("#0173b2"),  # Blue
            ROOT.TColor.GetColor("#de8f05"),  # Orange
            ROOT.TColor.GetColor("#029e73"),  # Green
            ROOT.TColor.GetColor("#cc78bc"),  # Pink
            ROOT.TColor.GetColor("#ca9161"),  # Brown
            ROOT.TColor.GetColor("#fbafe4"),  # Light Pink
            ROOT.TColor.GetColor("#949494"),  # Gray
        ]
    
    return color_schemes.get(scheme_name, color_schemes['default'])


def set_cms_style(config=None):
    """Set comprehensive CMS style matching C++ plot_jet.cc implementation."""
    ROOT.gROOT.SetBatch(True)  # For batch mode
    ROOT.gStyle.SetOptStat(0)  # No Stat Box
    ROOT.gStyle.SetOptTitle(0)  # No Title
    ROOT.gStyle.SetPadTickX(1)  # Tick marks on top and right
    ROOT.gStyle.SetPadTickY(1)
    ROOT.gStyle.SetHistTopMargin(0)  # Added from plot_jet.cc
    
    # Canvas settings with proper margins - increased margins to prevent cutoff
    if config:
        margin_left = float(config.get('CanvasMarginLeft', 0.18))  # Increased for y-axis labels
        margin_right = float(config.get('CanvasMarginRight', 0.08))  # Increased for right side space
        margin_top = float(config.get('CanvasMarginTop', 0.12))  # Sufficient for CMS label
        margin_bottom = float(config.get('CanvasMarginBottom', 0.15))  # Increased for x-axis labels and selection text
    else:
        margin_left, margin_right, margin_top, margin_bottom = 0.18, 0.08, 0.12, 0.15
    
    ROOT.gStyle.SetPadLeftMargin(margin_left)
    ROOT.gStyle.SetPadRightMargin(margin_right)
    ROOT.gStyle.SetPadTopMargin(margin_top)
    ROOT.gStyle.SetPadBottomMargin(margin_bottom)
    
    # Text and labels - All Helvetica fonts
    ROOT.gStyle.SetTextFont(42)  # Helvetica regular
    ROOT.gStyle.SetTextSize(0.035)  # Same as in plot_jet.cc (0.035)
    ROOT.gStyle.SetLabelFont(42, "XYZ")  # Helvetica for axis labels
    ROOT.gStyle.SetLabelSize(0.035, "XYZ")
    ROOT.gStyle.SetTitleFont(42, "XYZ")  # Helvetica for axis titles
    ROOT.gStyle.SetTitleSize(0.035, "XYZ")  # Match text size for consistency
    ROOT.gStyle.SetTitleFontSize(0.04)
    ROOT.gStyle.SetLegendFont(42)  # Helvetica for legend
    ROOT.gStyle.SetLegendTextSize(0.035)  # Increased to match text size
    
    # Axis settings
    ROOT.gStyle.SetNdivisions(510, "XYZ")
    ROOT.gStyle.SetTickLength(0.02, "XYZ")
    ROOT.gStyle.SetLineWidth(2)
    ROOT.gStyle.SetHistLineWidth(2)
    ROOT.gStyle.SetFrameLineWidth(2)
    
    # Error bars
    ROOT.gStyle.SetEndErrorSize(0)
    ROOT.gStyle.SetErrorX(0.5)
    
    # Palette for 2D plots
    ROOT.gStyle.SetPalette(ROOT.kViridis)
    ROOT.gStyle.SetNumberContours(255)


def get_auto_color(color_scheme, index, color_blind=False):
    """Get color from scheme by index."""
    colors = get_color_scheme(color_scheme, color_blind)
    return colors[index % len(colors)]


def get_marker_styles():
    """Return marker styles following CMS guidelines."""
    return [
        ROOT.kFullCircle,        # 20
        ROOT.kFullSquare,        # 21  
        ROOT.kFullTriangleUp,    # 22
        ROOT.kFullTriangleDown,  # 23
        ROOT.kFullDiamond,       # 33
        ROOT.kFullStar,          # 29
        ROOT.kOpenCircle,        # 24
        ROOT.kOpenSquare,        # 25
        ROOT.kOpenTriangleUp     # 26
    ]


def get_auto_marker(index):
    """Get marker style from list by index."""
    markers = get_marker_styles()
    return markers[index % len(markers)]


def get_cms_text(config):
    """Generate CMS text label from configuration."""
    cms_label = config.get('CMSLabel', 'Preliminary')
    energy_text = config.get('CMSEnergyText', '')
    luminosity = config.get('CMSLuminosity', '')
    extra_text = config.get('CMSExtraText', '')
    
    # Build the main CMS text
    cms_text = "CMS"
    
    # Add label (Preliminary, Simulation, etc.)
    if cms_label and cms_label.lower() != 'none':
        if extra_text and extra_text.lower() == "simulation":
            cms_text += " Simulation"
        else:
            cms_text += f" {cms_label}"
    
    # Add energy and luminosity if available
    if energy_text and luminosity:
        cms_text += f", {energy_text}, {luminosity}"
    elif energy_text:
        cms_text += f", {energy_text}"
    elif luminosity:
        cms_text += f", {luminosity}"
    
    return cms_text


def get_available_jet_dirs(root_file, config):
    """Get list of available jet directories in ROOT file."""
    jet_dirs_config = config.get('JetDirectories', 'AK4Z2')
    available_dirs = []
    verbose = config.get('Verbose', '0') == '1'
    if verbose:
        print(f"DEBUG: Checking for jet directories from config: {jet_dirs_config}")
        
        for jet_dir in jet_dirs_config.split(','):
            jet_dir = jet_dir.strip()
            if root_file.GetDirectory(jet_dir):
                available_dirs.append(jet_dir)
                print(f"DEBUG: Found jet directory: {jet_dir}")
            else:
                print(f"DEBUG: Jet directory NOT found: {jet_dir}")
    
        print(f"DEBUG: Total available jet directories: {len(available_dirs)} out of {len(jet_dirs_config.split(','))}")
    
    return available_dirs


def get_available_centrality_bins(root_file, jet_dir, config):
    """Get list of available centrality bins for a given jet directory."""
    cent_bins_config = config.get('CentralityBins', '0,60,180')
    available_bins = []
    
    # Parse centrality bins from config
    try:
        bins = [float(x.strip()) for x in cent_bins_config.split(',')]
        cent_bins = []
        for i in range(len(bins)-1):
            cent_bins.append(f"cent{int(bins[i])}to{int(bins[i+1])}")
    except:
        cent_bins = ['cent0to60', 'cent60to180']  # Default
    
    # Check which bins exist in the file
    if jet_dir:
        jet_dir_obj = root_file.GetDirectory(jet_dir)
        if jet_dir_obj:
            for cent_bin in cent_bins:
                if jet_dir_obj.GetDirectory(cent_bin):
                    available_bins.append(cent_bin)
    else:
        # Check directly in root file
        for cent_bin in cent_bins:
            if root_file.GetDirectory(cent_bin):
                available_bins.append(cent_bin)
    
    return available_bins


def get_histogram_from_path(root_file, jet_dir, cent_bin, hist_name, config):
    """Get histogram from nested ROOT file structure."""
    # Handle both nested and flat structures
    use_nested = config.get('UseNestedStructure', '1') == '1'
    hist_subdir = config.get('HistogramSubdir', '').strip()
    
    if use_nested:
        # Nested structure: JetDir/CentBin/[HistogramSubdir/]HistName
        if hist_subdir:
            full_path = f"{jet_dir}/{cent_bin}/{hist_subdir}/{hist_name}"
        else:
            full_path = f"{jet_dir}/{cent_bin}/{hist_name}"
    else:
        # Flat structure: HistName_JetDir_CentBin
        full_path = f"{hist_name}_{jet_dir}_{cent_bin}"
    
    hist = root_file.Get(full_path)
    if not hist:
        # Try alternative paths
        alt_paths = [
            f"{jet_dir}/{cent_bin}/{hist_name}",  # Direct path
            f"{cent_bin}/{hist_name}",            # Without jet dir
            f"{hist_name}_{cent_bin}",            # Flat with cent only
            hist_name                             # Just histogram name
        ]
        
        for alt_path in alt_paths:
            hist = root_file.Get(alt_path)
            if hist:
                break
    
    return hist


def load_histogram_from_file(root_file, hist_path):
    """Load a histogram from a ROOT file with proper error handling."""
    hist = root_file.Get(hist_path)
    if not hist:
        return None
    
    # Clone the histogram to avoid issues when closing files
    cloned_hist = hist.Clone(f"{hist.GetName()}_clone")
    cloned_hist.SetDirectory(0)  # Detach from file
    return cloned_hist


def find_histogram_in_structure(root_file, hist_name, jet_dir=None, cent_bin=None):
    """
    Find histogram in nested ROOT structure.
    Returns the full path to the histogram if found, None otherwise.
    """
    search_paths = []
    
    if jet_dir and cent_bin:
        # Nested structure: JetDir/CentBin/HistName
        search_paths.append(f"{jet_dir}/{cent_bin}/{hist_name}")
    elif jet_dir:
        # Only jet directory specified
        search_paths.append(f"{jet_dir}/{hist_name}")
    elif cent_bin:
        # Only centrality specified
        search_paths.append(f"{cent_bin}/{hist_name}")
    else:
        # Flat structure
        search_paths.append(hist_name)
    
    # Try each path
    for path in search_paths:
        hist = root_file.Get(path)
        if hist:
            return path
    
    return None


def get_centrality_label(cent_bin, config=None):
    """Convert centrality bin to display label using config-defined labels."""
    if config:
        # Try to get label from config first
        config_key = f"CentralityLabels.{cent_bin.replace('cent', '')}"
        if config_key in config:
            return config[config_key]
    
    # Fallback to parsing the bin name
    match = re.search(r'cent(\d+)to(\d+)', cent_bin)
    if match:
        low, high = match.groups()
        return f"Centrality {low}-{high}%"
    return cent_bin


def get_jet_label(jet_dir):
    """Convert jet directory to display label."""
    jet_labels = {
        'AK2Z1': 'AK R=0.2, Z_{cut}=0.1',
        'AK4Z2': 'AK R=0.4, Z_{cut}=0.2', 
        'AK6Z3': 'AK R=0.6, Z_{cut}=0.3',
        'AK8Z4': 'AK R=0.8, Z_{cut}=0.4',
        'AK8Z5': 'AK R=0.8, Z_{cut}=0.5'
    }
    return jet_labels.get(jet_dir, jet_dir)


def parse_overlay_jets(config):
    """Parse which jet collections to use for overlays."""
    overlay_jets = config.get('OverlayJetCollections', 'all').strip()
    if overlay_jets.lower() == 'all':
        return config.get('JetDirectories', 'AK4Z2').split(',')
    else:
        return [x.strip() for x in overlay_jets.split(',')]


def parse_overlay_centrality(config):
    """Parse which centrality bins to use for overlays."""
    overlay_cents = config.get('OverlayCentralityBins', 'all').strip()
    if overlay_cents.lower() == 'all':
        # Parse from CentralityBins config
        cent_bins_config = config.get('CentralityBins', '0,60,180')
        try:
            bins = [float(x.strip()) for x in cent_bins_config.split(',')]
            cent_bins = []
            for i in range(len(bins)-1):
                cent_bins.append(f"cent{int(bins[i])}to{int(bins[i+1])}")
            return cent_bins
        except:
            return ['cent0to60', 'cent60to180']
    else:
        return [x.strip() for x in overlay_cents.split(',')]


def apply_histogram_style(hist, hist_config, color_index, config):
    """Apply styling to histogram based on configuration."""
    if hist_config.color == 'auto':
        color_scheme = config.get('ColorScheme', 'default')
        color_blind = config.get('UseColorBlind', '0') == '1'
        color = get_auto_color(color_scheme, color_index, color_blind)
    else:
        color = getattr(ROOT, hist_config.color, ROOT.kBlack)
    
    hist.SetLineColor(color)
    hist.SetMarkerColor(color)
    hist.SetLineWidth(hist_config.line_width)
    
    # Set marker style
    if hist_config.marker_style == 'auto':
        hist.SetMarkerStyle(get_auto_marker(color_index))
    else:
        marker_code = getattr(ROOT, hist_config.marker_style, ROOT.kFullCircle)
        hist.SetMarkerStyle(marker_code)
    
    hist.SetMarkerSize(hist_config.marker_size)
    hist.SetTitle('')
    
    # Set axis titles
    if ';' in hist_config.title:
        parts = hist_config.title.split(';')
        if len(parts) > 1:
            hist.GetXaxis().SetTitle(parts[1])
        if len(parts) > 2:
            hist.GetYaxis().SetTitle(parts[2])


def draw_cms_label(canvas, config, selection_text=""):
    """Draw CMS label in the top margin area of the pad using proper coordinate system."""
    canvas.cd()
    
    # Get configuration parameters
    cms_label = config.get('CMSLabel', 'Preliminary')
    energy_text = config.get('CMSEnergyText', '5.36 TeV PbPb')
    luminosity = config.get('CMSLuminosity', '1.72 nb^{-1}')
    extra_text = config.get('CMSExtraText', '')
    
    # Create the label with proper styling
    latex = ROOT.TLatex()
    latex.SetNDC()
    latex.SetTextAngle(0)
    latex.SetTextColor(ROOT.kBlack)
    
    # Find the current active pad (could be canvas or a sub-pad)
    pad = ROOT.gPad
    if not pad:
        pad = canvas
    
    # Get the top margin from the pad (this is where we should place text)
    top_margin = pad.GetTopMargin()
    
    # Position CMS label in the top margin area (within the pad coordinate system)
    # The plot frame goes from bottom_margin to (1 - top_margin) in normalized pad coordinates
    # We want to place text in the margin area: from (1 - top_margin) to 1.0
    # Use a small offset from the top margin to avoid overlap with the plot
    y_cms = 1.0 - (0.5 * top_margin)  
    
    # Position CMS label on the left side with small offset
    left_margin = canvas.GetLeftMargin()
    right_margin = canvas.GetRightMargin()
    x_cms = left_margin + 0.02  # 3% from left edge in pad coordinates

    # Draw CMS in bold (font 61)
    latex.SetTextFont(61)  # Bold font
    latex.SetTextSize(0.045)  # Appropriate size for margin area
    latex.SetTextAlign(11)  # Left-aligned
    latex.DrawLatexNDC(x_cms, y_cms, "CMS")
    
    # Draw Preliminary/Simulation text in italic (font 52)
    x_prelim = x_cms + 0.09  # Offset from CMS text
    latex.SetTextFont(52)  # Italic font
    latex.SetTextSize(0.038)  # Slightly smaller than CMS
    
    if extra_text and extra_text.lower() == "simulation":
        if cms_label and cms_label.lower() != 'none':
            latex.DrawLatexNDC(x_prelim, y_cms, f"{cms_label} {extra_text}")
        else:
            latex.DrawLatexNDC(x_prelim, y_cms, f"{extra_text}")
    elif cms_label and cms_label.lower() != 'none':
        latex.DrawLatexNDC(x_prelim, y_cms, f"{cms_label}")
    
    # Add energy and luminosity information on the right side
    latex.SetTextFont(42)  # Regular font (Helvetica)
    latex.SetTextAlign(31)  # Right-aligned
    latex.SetTextSize(0.032)  # Smaller for right side info
    
    if energy_text and luminosity:
        x_lumi = 1.0 - right_margin - 0.035
        
        # Try to keep energy and luminosity on same line
        combined_text = f"{energy_text}, {luminosity}"
        # if len(combined_text) > 40:  # Split if too long
        #     latex.DrawLatexNDC(x_lumi, y_cms, f"{energy_text}")
        #     latex.DrawLatexNDC(x_lumi, y_cms - 0.04, f"{luminosity}")
        # else:
        latex.DrawLatexNDC(x_lumi, y_cms, combined_text)
    
    # Don't add selection text here - it will be handled by the plotting functions
    # based on legend position


def create_output_dirs(base_dir, config, jet_dirs=None):
    """Create organized output directory structure with jet collection subdirectories."""
    dirs = {
        'base': base_dir,
        '1D': os.path.join(base_dir, '1D'),
        '2D': os.path.join(base_dir, '2D'),
        'profiles': os.path.join(base_dir, 'profiles'),
        'overlays': os.path.join(base_dir, 'overlays'),
        'ratios': os.path.join(base_dir, 'ratios'),
        'comparisons': os.path.join(base_dir, 'comparisons')
    }
    
    # Create the basic directory structure
    for dir_path in dirs.values():
        os.makedirs(dir_path, exist_ok=True)
    
    # If jet directories are provided, create subdirectories for each jet collection
    if jet_dirs:
        # Create specific jet directories for 1D, 2D and profile plots
        for jet_dir in jet_dirs:
            jet_dir = jet_dir.strip()
            # Create jet-specific subdirectories in 1D, 2D and profiles
            dirs[f'1D_{jet_dir}'] = os.path.join(dirs['1D'], jet_dir)
            dirs[f'2D_{jet_dir}'] = os.path.join(dirs['2D'], jet_dir)
            dirs[f'profiles_{jet_dir}'] = os.path.join(dirs['profiles'], jet_dir)
            
            # Make the directories
            os.makedirs(dirs[f'1D_{jet_dir}'], exist_ok=True)
            os.makedirs(dirs[f'2D_{jet_dir}'], exist_ok=True)
            os.makedirs(dirs[f'profiles_{jet_dir}'], exist_ok=True)
    
    return dirs


def get_selection_text(config, jet_dir="", cent_bin=""):
    """Generate selection text for plots based on configuration and context."""
    selection_parts = []
    
    # Get selection criteria from config
    photon_pt_min = config.get('SelectionPhotonPtMin', '')
    photon_pt_max = config.get('SelectionPhotonPtMax', '')
    photon_eta_max = config.get('SelectionPhotonEtaMax', '')
    
    jet_pt_min = config.get('SelectionJetPtMin', '')
    jet_pt_max = config.get('SelectionJetPtMax', '')
    jet_eta_max = config.get('SelectionJetEtaMax', '')
    
    dphi_min = config.get('SelectionDeltaPhiMin', '')
    
    # Custom selection text from config
    custom_selection = config.get('SelectionText', '')
    
    if custom_selection:
        selection_parts.append(custom_selection)
    else:
        # Build selection text from individual criteria
        if photon_pt_min or photon_pt_max:
            pt_text = "p_{T}^{#gamma}"
            if photon_pt_min and photon_pt_max:
                pt_text += f" = {photon_pt_min}-{photon_pt_max} GeV"
            elif photon_pt_min:
                pt_text += f" > {photon_pt_min} GeV"
            elif photon_pt_max:
                pt_text += f" < {photon_pt_max} GeV"
            selection_parts.append(pt_text)
        
        if photon_eta_max:
            selection_parts.append(f"|#eta^{{#gamma}}| < {photon_eta_max}")
        
        if jet_pt_min or jet_pt_max:
            jet_pt_text = "p_{T}^{jet}"
            if jet_pt_min and jet_pt_max:
                jet_pt_text += f" = {jet_pt_min}-{jet_pt_max} GeV"
            elif jet_pt_min:
                jet_pt_text += f" > {jet_pt_min} GeV"
            elif jet_pt_max:
                jet_pt_text += f" < {jet_pt_max} GeV"
            selection_parts.append(jet_pt_text)
        
        if jet_eta_max:
            selection_parts.append(f"|#eta^{{jet}}| < {jet_eta_max}")
        
        if dphi_min:
            selection_parts.append(f"#Delta#phi > {dphi_min}")
    
    # Add jet collection information
    if jet_dir:
        # Format jet collection more nicely
        jet_r = '?'  # Default value
        jet_z = '?'  # Default value
        
        if 'AK' in jet_dir and 'Z' in jet_dir:
            # Extract R and Z values from AK*Z* format
            parts = jet_dir.replace('AK', '').split('Z')
            if len(parts) >= 2:
                r_value = parts[0]
                z_value = parts[1]
                if r_value.isdigit():
                    jet_r = str(int(r_value)/10.0)  # Convert 4 to 0.4, etc.
                if z_value.isdigit():
                    jet_z = str(int(z_value)/10.0)  # Convert 2 to 0.2, etc.
                
                jet_label = f"Anti-k_{{T}} R={jet_r}, Z_{{cut}}={jet_z}"
        elif 'AK' in jet_dir:
            # Just AK* format without Z
            r_value = jet_dir.replace('AK', '')
            if r_value.isdigit():
                jet_r = str(int(r_value)/10.0)
                jet_label = f"Anti-k_{{T}} R={jet_r}"
        else:
            jet_label = jet_dir
            
        selection_parts.append(jet_label)
    
    # Add centrality information
    if cent_bin:
        # Format centrality more nicely
        if 'to' in cent_bin:
            # Extract centrality range and format it
            parts = cent_bin.replace('cent', '').split('to')
            if len(parts) >= 2 and all(p.isdigit() for p in parts):
                cent_min, cent_max = int(parts[0]) / 2.0, int(parts[1]) / 2.0
                cent_label = f"Centrality: {cent_min:g}-{cent_max:g}%"
                selection_parts.append(cent_label)
            else:
                # Use the centrality label function as fallback
                selection_parts.append(get_centrality_label(cent_bin, config))
        else:
            # Use the centrality label function
            selection_parts.append(get_centrality_label(cent_bin, config))
    
    # Join the parts with commas
    return ", ".join(selection_parts)


def save_canvas(canvas, outdir, name, formats, jet_dir="", cent_bin="", config=None):
    """Save canvas in multiple formats with jet directory and centrality bin in filename."""
    verbose = config.get('Verbose', '0') == '1' if config else False
    
    # Create a filename with jet_dir and cent_bin information if provided
    filename = name
    if jet_dir and cent_bin:
        filename = f"{name}_{jet_dir}_{cent_bin}"
    elif jet_dir:
        filename = f"{name}_{jet_dir}"
    elif cent_bin:
        filename = f"{name}_{cent_bin}"
    
    # Suppress ROOT info messages unless in verbose mode
    if not verbose:
        original_error_level = ROOT.gErrorIgnoreLevel
        ROOT.gErrorIgnoreLevel = ROOT.kError  # Only show errors and above (suppresses Info messages)
    
    for fmt in formats:
        outpath = os.path.join(outdir, f"{filename}.{fmt}")
        canvas.SaveAs(outpath)
        if verbose:
            # Show only the relative path from the base output directory
            rel_path = os.path.relpath(outpath, start=os.path.dirname(outdir))
            print(f"  Saved: {rel_path}")
    
    # Restore original ROOT error level
    if not verbose:
        ROOT.gErrorIgnoreLevel = original_error_level


def plot_histogram_1d(hist, hist_config, config, outdir, formats, jet_dir="", cent_bin=""):
    """Plot 1D histogram with full styling and configurable draw options."""
    canvas_name = f"c_{hist_config.name}"
    width = int(config.get('CanvasWidth', 800))
    height = int(config.get('CanvasHeight', 600))
    
    # Use jet directory-specific output path if available
    if jet_dir and outdir == os.path.join(os.path.dirname(outdir), '1D'):
        jet_specific_dir = os.path.join(outdir, jet_dir)
        if os.path.exists(jet_specific_dir):
            outdir = jet_specific_dir
    
    c = ROOT.TCanvas(canvas_name, canvas_name, width, height)
    c.cd()
    
    if hist_config.log_y:
        c.SetLogy()
    
    # Apply styling
    apply_histogram_style(hist, hist_config, 0, config)
    
    # Set axis ranges if specified
    if hist_config.x_min != hist_config.x_max:
        hist.GetXaxis().SetRangeUser(hist_config.x_min, hist_config.x_max)
    
    # Use the configured draw option
    hist.Draw(hist_config.draw_option)
    
    # Draw CMS label
    draw_cms_label(c, config, "")
    
    # Get selection text
    selection_text = get_selection_text(config, jet_dir, cent_bin)
    
    # Handle selection text with proper formatting and positioning
    if selection_text:
        latex = ROOT.TLatex()
        latex.SetNDC()
        latex.SetTextFont(42)  # Helvetica regular
        latex.SetTextSize(0.032)  # Increased text size for better visibility
        latex.SetTextAlign(11)  # Left-aligned
        
        # Split text into manageable chunks (shorter lines)
        max_chars_per_line = 45  # Reduced for better formatting
        if len(selection_text) > max_chars_per_line:
            chunks = []
            current_chunk = ""
            
            # Split by commas first
            parts = selection_text.split(', ')
            for part in parts:
                # Check if adding this part would exceed the line limit
                test_chunk = current_chunk + (", " if current_chunk else "") + part
                if len(test_chunk) <= max_chars_per_line:
                    current_chunk = test_chunk
                else:
                    if current_chunk:
                        chunks.append(current_chunk)
                        current_chunk = part
                    else:
                        # Single part is too long, try to split it
                        if len(part) > max_chars_per_line:
                            # Split at reasonable points (spaces, underscores, etc.)
                            words = part.replace('_', ' ').split()
                            current_word_chunk = ""
                            for word in words:
                                test_word_chunk = current_word_chunk + (" " if current_word_chunk else "") + word
                                if len(test_word_chunk) <= max_chars_per_line:
                                    current_word_chunk = test_word_chunk
                                else:
                                    if current_word_chunk:
                                        chunks.append(current_word_chunk)
                                        current_word_chunk = word
                                    else:
                                        chunks.append(word)  # Single word too long, just add it
                            if current_word_chunk:
                                current_chunk = current_word_chunk
                        else:
                            current_chunk = part
            
            if current_chunk:
                chunks.append(current_chunk)
        else:
            chunks = [selection_text]
        
        # Position text in bottom left, well within the margin
        left_margin = c.GetLeftMargin()
        bottom_margin = c.GetBottomMargin()
        
        start_x = left_margin + 0.02  # Small offset from left margin
        start_y = bottom_margin + 0.02  # Small offset from bottom margin
        line_spacing = 0.04  # Spacing between lines
        
        # Draw each chunk on a separate line, starting from bottom
        for i, chunk in enumerate(chunks):
            y_pos = start_y + i * line_spacing
            latex.DrawLatexNDC(start_x, y_pos, chunk)
    
    # Create filename with jet directory and centrality bin information
    filename = hist_config.name
    if jet_dir and cent_bin:
        filename = f"{hist_config.name}_{jet_dir}_{cent_bin}"
    elif jet_dir:
        filename = f"{hist_config.name}_{jet_dir}"
    elif cent_bin:
        filename = f"{hist_config.name}_{cent_bin}"
    
    # Suppress ROOT info messages unless in verbose mode
    verbose = config.get('Verbose', '0') == '1'
    if not verbose:
        original_error_level = ROOT.gErrorIgnoreLevel
        ROOT.gErrorIgnoreLevel = ROOT.kError  # Only show errors and above (suppresses Info messages)
    
    # Save
    for fmt in formats:
        outpath = os.path.join(outdir, f"{filename}.{fmt}")
        c.SaveAs(outpath)
        if verbose:
            # Show only the relative path from the base output directory
            rel_path = os.path.relpath(outpath, start=os.path.dirname(outdir))
            print(f"  Saved: {rel_path}")
    
    # Restore original ROOT error level
    if not verbose:
        ROOT.gErrorIgnoreLevel = original_error_level
    
    c.Close()


def plot_histogram_2d(hist, hist_config, config, outdir, formats, jet_dir="", cent_bin=""):
    """Plot 2D histogram with proper styling and configurable draw options."""
    canvas_name = f"c_{hist_config.name}"
    width = int(config.get('CanvasWidth', 800))
    height = int(config.get('CanvasHeight', 600))
    
    # Use jet directory-specific output path if available
    if jet_dir and outdir == os.path.join(os.path.dirname(outdir), '2D'):
        jet_specific_dir = os.path.join(outdir, jet_dir)
        if os.path.exists(jet_specific_dir):
            outdir = jet_specific_dir
    
    c = ROOT.TCanvas(canvas_name, canvas_name, width, height)
    c.cd()
    
    # Set right margin to accommodate color palette
    c.SetRightMargin(0.15)
    
    if hist_config.log_z:
        c.SetLogz()
    
    # Set color palette
    if hist_config.color_map == 'viridis':
        ROOT.gStyle.SetPalette(ROOT.kViridis)
    elif hist_config.color_map == 'plasma':
        ROOT.gStyle.SetPalette(ROOT.kPlasma)
    else:
        ROOT.gStyle.SetPalette(ROOT.kBird)
    
    # Apply basic styling
    hist.SetTitle('')
    if ';' in hist_config.title:
        parts = hist_config.title.split(';')
        if len(parts) > 1:
            hist.GetXaxis().SetTitle(parts[1])
        if len(parts) > 2:
            hist.GetYaxis().SetTitle(parts[2])
    
    # Use the configured 2D draw option
    hist.Draw(hist_config.draw_option_2d)
    
    # Get selection text
    selection_text = get_selection_text(config, jet_dir, cent_bin)
    
    # Draw CMS label
    draw_cms_label(c, config, "")
    
    # Display selection text
    if selection_text:
        latex = ROOT.TLatex()
        latex.SetNDC()
        latex.SetTextFont(42)  # Helvetica regular
        latex.SetTextSize(0.035)
        
        # Split text into manageable chunks
        if len(selection_text) > 60:  # Long text needs special handling
            chunks = []
            current_chunk = ""
            for part in selection_text.split(', '):
                if len(current_chunk) + len(part) + 2 <= 60:
                    if current_chunk:
                        current_chunk += ", " + part
                    else:
                        current_chunk = part
                else:
                    chunks.append(current_chunk)
                    current_chunk = part
            if current_chunk:
                chunks.append(current_chunk)
        else:
            # Short enough to be one chunk
            chunks = [selection_text]
        
        # Position text at bottom left with proper spacing
        for i, chunk in enumerate(chunks):
            latex.DrawLatexNDC(0.2, 0.3 - i*0.04, chunk)
    
    # Create filename with jet directory and centrality bin information
    filename = hist_config.name
    if jet_dir and cent_bin:
        filename = f"{hist_config.name}_{jet_dir}_{cent_bin}"
    elif jet_dir:
        filename = f"{hist_config.name}_{jet_dir}"
    elif cent_bin:
        filename = f"{hist_config.name}_{cent_bin}"
    
    # Suppress ROOT info messages unless in verbose mode
    verbose = config.get('Verbose', '0') == '1'
    if not verbose:
        original_error_level = ROOT.gErrorIgnoreLevel
        ROOT.gErrorIgnoreLevel = ROOT.kError  # Only show errors and above (suppresses Info messages)
    
    # Save
    for fmt in formats:
        outpath = os.path.join(outdir, f"{filename}.{fmt}")
        c.SaveAs(outpath)
        if verbose:
            # Show only the relative path from the base output directory
            rel_path = os.path.relpath(outpath, start=os.path.dirname(outdir))
            print(f"  Saved: {rel_path}")
    
    # Restore original ROOT error level
    if not verbose:
        ROOT.gErrorIgnoreLevel = original_error_level
    
    c.Close()


def plot_overlay_advanced(hists, labels, config, outdir, name, formats, hist_config=None, jet_dir="", cent_bin=""):
    """Advanced overlay plotting with proper legend and styling."""
    canvas_name = f"c_{name}"
    width = int(config.get('CanvasWidth', 800))
    height = int(config.get('CanvasHeight', 600))
    
    c = ROOT.TCanvas(canvas_name, canvas_name, width, height)
    c.cd()
    
    if hist_config and hist_config.log_y:
        c.SetLogy()
    
    # Get color scheme
    color_scheme = config.get('ColorScheme', 'default')
    color_blind = config.get('UseColorBlind', '0') == '1'
    colors = get_color_scheme(color_scheme, color_blind)
    
    # Configure legend position (similar to plot_jet.cc)
    leg_pos = config.get('Overlay.LegendPosition', '0.65,0.65,0.9,0.9')
    leg_x1, leg_y1, leg_x2, leg_y2 = map(float, leg_pos.split(','))
    
    # Support different legend positions based on options
    legend_opt = config.get('Legend.Position', 'default')
    if legend_opt == 'left':
        leg_x1, leg_y1, leg_x2, leg_y2 = 0.15, 0.55, 0.35, 0.88
    elif legend_opt == 'right':
        leg_x1, leg_y1, leg_x2, leg_y2 = 0.65, 0.58, 0.85, 0.88
    elif legend_opt == 'bcenter':
        leg_x1, leg_y1, leg_x2, leg_y2 = 0.4, 0.15, 0.6, 0.3
    
    # Create legend with configured position
    leg = ROOT.TLegend(leg_x1, leg_y1, leg_x2, leg_y2)
    leg.SetBorderSize(int(config.get('Overlay.LegendBorderSize', '0')))
    leg.SetFillStyle(int(config.get('Overlay.LegendFillStyle', '0')))
    leg.SetTextFont(42)
    leg.SetTextSize(float(config.get('Overlay.LegendTextSize', '0.035')))
    
    y_max = 0
    for i, (hist, label) in enumerate(zip(hists, labels)):
        color = colors[i % len(colors)]
        hist.SetLineColor(color)
        hist.SetMarkerColor(color)
        hist.SetLineWidth(2)
        hist.SetMarkerStyle(20 + i)
        hist.SetMarkerSize(0.8)
        hist.SetTitle('')
        
        # Find maximum for proper y-axis scaling
        hist_max = hist.GetMaximum()
        if hist_max > y_max:
            y_max = hist_max
        
        # Use configurable draw option for overlays
        draw_opt = hist_config.draw_option if (i == 0 and hist_config) else "hist same"
        if hist_config and hist_config.draw_option and i == 0:
            draw_opt = hist_config.draw_option
        elif hist_config and hist_config.draw_option and i > 0:
            draw_opt = hist_config.draw_option + " same"
        else:
            draw_opt = "hist" if i == 0 else "hist same"
        
        hist.Draw(draw_opt)
        
        # Add to legend
        leg.AddEntry(hist, label, "lep")
    
    # Scale y-axis for log plots
    if hist_config and hist_config.log_y:
        hists[0].SetMaximum(y_max * 10)
        hists[0].SetMinimum(0.1)
    else:
        hists[0].SetMaximum(y_max * 1.2)
    
    leg.Draw()
    
    # Get selection text for overlays
    selection_text = get_selection_text(config, jet_dir, cent_bin)
    
    # Draw CMS label and energy/lumi
    draw_cms_label(c, config, "")
    
    # Handle selection text similarly to plot_jet.cc:
    # Display additional text below or above the legend
    if selection_text:
        latex = ROOT.TLatex()
        latex.SetNDC()
        latex.SetTextFont(42)  # Helvetica regular
        latex.SetTextSize(0.035)
        
        legend_opt = config.get('Legend.Position', 'default')
        text_parts = selection_text.split(', ')
        
        # Display text based on legend position
        if legend_opt == 'bcenter':
            # Text above legend
            y_pos = leg_y2 + 0.05
            for i, text in enumerate(text_parts):
                if text:
                    latex.DrawLatexNDC(leg_x1, y_pos + i*0.05, text)
        else:
            # Text below legend
            y_pos = leg_y1 - 0.05
            for i, text in enumerate(text_parts):
                if text:
                    latex.DrawLatexNDC(leg_x1, y_pos - i*0.05, text)
    
    # Create filename with jet directory and centrality bin information
    filename = name
    if jet_dir and cent_bin:
        if not filename.endswith("overlay"):
            filename = f"{name}_{jet_dir}_{cent_bin}"
    elif jet_dir and not "jetcollection" in filename:
        filename = f"{name}_{jet_dir}"
    elif cent_bin and not "centrality" in filename:
        filename = f"{name}_{cent_bin}"
    
    # Suppress ROOT info messages unless in verbose mode
    verbose = config.get('Verbose', '0') == '1'
    if not verbose:
        original_error_level = ROOT.gErrorIgnoreLevel
        ROOT.gErrorIgnoreLevel = ROOT.kError  # Only show errors and above (suppresses Info messages)
    
    # Save
    for fmt in formats:
        outpath = os.path.join(outdir, f"{filename}.{fmt}")
        c.SaveAs(outpath)
        if verbose:
            # Show only the relative path from the base output directory
            rel_path = os.path.relpath(outpath, start=os.path.dirname(outdir))
            print(f"  Saved: {rel_path}")
    
    # Restore original ROOT error level
    if not verbose:
        ROOT.gErrorIgnoreLevel = original_error_level
    
    c.Close()


def plot_centrality_overlay(root_file, jet_dir, hist_name, config, outdir, formats):
    """Create overlay plot comparing different centrality bins."""
    overlay_hists = []
    overlay_labels = []
    
    # Get centrality bins to overlay
    cent_bins = parse_overlay_centrality(config)
    available_cents = get_available_centrality_bins(root_file, jet_dir, config)
    
    # Only use centrality bins that exist in file and are requested
    use_cents = [c for c in cent_bins if c in available_cents]
    
    for cent_bin in use_cents:
        hist = get_histogram_from_path(root_file, jet_dir, cent_bin, hist_name, config)
        if hist:
            overlay_hists.append(hist)
            overlay_labels.append(get_centrality_label(cent_bin, config))
    
    if len(overlay_hists) > 1:
        plot_name = f"{hist_name}_{jet_dir}_centrality_overlay"
        
        # Create a dummy hist_config for the overlay
        class DummyHistConfig:
            def __init__(self, name, config):
                self.name = name
                self.log_y = config.get(f'Histogram.{hist_name.replace("h", "")}.LogY', '0') == '1'
                self.draw_option = config.get(f'Histogram.{hist_name.replace("h", "")}.DrawOption', 'hist')
        
        hist_config = DummyHistConfig(plot_name, config)
        plot_overlay_advanced(overlay_hists, overlay_labels, config, outdir, 
                            plot_name, formats, hist_config, jet_dir, "")
        return True
    
    return False


def plot_jet_collection_overlay(root_file, cent_bin, hist_name, config, outdir, formats):
    """Create overlay plot comparing different jet collections."""
    overlay_hists = []
    overlay_labels = []
    
    # Get jet collections to overlay
    jet_dirs = parse_overlay_jets(config)
    available_jets = get_available_jet_dirs(root_file, config)
    
    # Only use jet collections that exist in file and are requested
    use_jets = [j for j in jet_dirs if j in available_jets]
    
    for jet_dir in use_jets:
        hist = get_histogram_from_path(root_file, jet_dir, cent_bin, hist_name, config)
        if hist:
            overlay_hists.append(hist)
            overlay_labels.append(get_jet_label(jet_dir))
    
    if len(overlay_hists) > 1:
        plot_name = f"{hist_name}_{cent_bin}_jetcollection_overlay"
        
        # Create a dummy hist_config for the overlay
        class DummyHistConfig:
            def __init__(self, name, config):
                self.name = name
                self.log_y = config.get(f'Histogram.{hist_name.replace("h", "")}.LogY', '0') == '1'
                self.draw_option = config.get(f'Histogram.{hist_name.replace("h", "")}.DrawOption', 'hist')
        
        hist_config = DummyHistConfig(plot_name, config)
        plot_overlay_advanced(overlay_hists, overlay_labels, config, outdir, 
                            plot_name, formats, hist_config, "", cent_bin)
        return True
    
    return False


def plot_datamc_comparison(data_file, mc_file, hist_name, config, outdir, formats, jet_dir="", cent_bin=""):
    """
    Create Data-MC comparison plots with overlay and ratio panels.
    
    Args:
        data_file: ROOT file containing data histograms
        mc_file: ROOT file containing MC histograms  
        hist_name: Name of histogram to compare
        config: Configuration dictionary
        outdir: Output directory
        formats: List of output formats
        jet_dir: Jet directory name (optional)
        cent_bin: Centrality bin name (optional)
    """
    # Get Data-MC configuration
    datamc_config = get_datamc_config(config)
    
    # Find histograms in both files
    data_path = find_histogram_in_structure(data_file, hist_name, jet_dir, cent_bin)
    mc_path = find_histogram_in_structure(mc_file, hist_name, jet_dir, cent_bin)
    
    if not data_path or not mc_path:
        if config.get('Verbose', '0') == '1':
            print(f"Warning: Could not find {hist_name} in both files")
            if not data_path:
                print(f"  Missing in data file: {hist_name}")
            if not mc_path:
                print(f"  Missing in MC file: {hist_name}")
        return False
    
    # Load histograms
    data_hist = load_histogram_from_file(data_file, data_path)
    mc_hist = load_histogram_from_file(mc_file, mc_path)
    
    if not data_hist or not mc_hist:
        return False
    
    # Normalize histograms if requested
    if config.get('DataMC.Normalize', '0') == '1':
        if data_hist.Integral() > 0:
            data_hist.Scale(1.0 / data_hist.Integral())
        if mc_hist.Integral() > 0:
            mc_hist.Scale(1.0 / mc_hist.Integral())
    
    # Create canvas with two pads (main plot + ratio)
    canvas_name = f"c_datamc_{hist_name}"
    if jet_dir:
        canvas_name += f"_{jet_dir}"
    if cent_bin:
        canvas_name += f"_{cent_bin}"
    
    c = ROOT.TCanvas(canvas_name, canvas_name, datamc_config.canvas_width, datamc_config.canvas_height)
    
    # Upper pad for main comparison
    upper_pad = ROOT.TPad("upper", "upper", 0, datamc_config.lower_pad_height, 1, 1)
    upper_pad.SetBottomMargin(0.02)
    upper_pad.SetTopMargin(0.08)
    upper_pad.SetLeftMargin(0.18)  # Increased for y-axis labels
    upper_pad.SetRightMargin(0.08)
    upper_pad.Draw()
    
    # Lower pad for ratio
    lower_pad = ROOT.TPad("lower", "lower", 0, 0, 1, datamc_config.lower_pad_height)
    lower_pad.SetTopMargin(0.02)
    lower_pad.SetBottomMargin(0.35)  # Increased for x-axis labels
    lower_pad.SetLeftMargin(0.18)  # Match upper pad
    lower_pad.SetRightMargin(0.08)
    lower_pad.Draw()
    
    # Plot main comparison in upper pad
    upper_pad.cd()
    
    # Check for logY scaling - look for histogram-specific LogY setting
    # Try to find the histogram configuration for LogY setting
    hist_plot_key = hist_name.replace('h', '') if hist_name.startswith('h') else hist_name
    use_log_y = False
    
    # Check histogram-specific LogY setting
    if config.get(f'Histogram.{hist_plot_key}.LogY', '0') == '1':
        use_log_y = True
    # Check global DataMC LogY setting as fallback
    elif config.get('DataMC.LogY', '0') == '1':
        use_log_y = True
    
    if use_log_y:
        upper_pad.SetLogy()
    
    # Style data histogram
    data_hist.SetMarkerColor(datamc_config.data_color)
    data_hist.SetMarkerStyle(datamc_config.data_marker_style)
    data_hist.SetMarkerSize(datamc_config.data_marker_size)
    data_hist.SetLineColor(datamc_config.data_color)
    data_hist.SetLineWidth(datamc_config.data_line_width)
    
    # Style MC histogram
    mc_hist.SetMarkerColor(datamc_config.mc_color)
    mc_hist.SetMarkerStyle(datamc_config.mc_marker_style)
    mc_hist.SetMarkerSize(datamc_config.mc_marker_size)
    mc_hist.SetLineColor(datamc_config.mc_color)
    mc_hist.SetLineWidth(datamc_config.mc_line_width)
    
    # Determine y-axis range
    max_data = data_hist.GetMaximum()
    max_mc = mc_hist.GetMaximum()
    y_max = max(max_data, max_mc) * 1.3
    
    # Draw histograms
    draw_option_data = "PE"
    draw_option_mc = "PE SAME"
    
    data_hist.SetMaximum(y_max)
    data_hist.GetXaxis().SetLabelSize(0)  # Hide x-axis labels in upper pad
    data_hist.GetYaxis().SetTitle("Events")
    data_hist.GetYaxis().SetTitleSize(0.06)
    data_hist.GetYaxis().SetLabelSize(0.05)
    
    data_hist.Draw(draw_option_data)
    mc_hist.Draw(draw_option_mc)
    
    # Create legend
    legend = ROOT.TLegend(datamc_config.legend_x1, datamc_config.legend_y1, 
                         datamc_config.legend_x2, datamc_config.legend_y2)
    legend.SetBorderSize(0)
    legend.SetFillStyle(0)
    legend.SetTextFont(42)
    legend.SetTextSize(0.04)
    legend.AddEntry(data_hist, datamc_config.data_label, "PE")
    legend.AddEntry(mc_hist, datamc_config.mc_label, "PE")
    legend.Draw()
    
    # Add CMS labeling and selection text - use the proper function
    draw_cms_label(upper_pad, config, "")
    
    # Add selection text
    selection_text = get_selection_text(config, jet_dir, cent_bin)
    if selection_text:
        latex_sel = ROOT.TLatex()
        latex_sel.SetNDC()
        latex_sel.SetTextFont(42)
        latex_sel.SetTextSize(0.04)  # Increased text size for better visibility
        latex_sel.SetTextAlign(11)  # Left-aligned
        
        # Split selection text into lines for Data-MC plots with shorter lines
        max_chars_per_line = 35  # Reduced for better formatting
        if len(selection_text) > max_chars_per_line:
            chunks = []
            current_chunk = ""
            parts = selection_text.split(', ')
            for part in parts:
                test_chunk = current_chunk + (", " if current_chunk else "") + part
                if len(test_chunk) <= max_chars_per_line:
                    current_chunk = test_chunk
                else:
                    if current_chunk:
                        chunks.append(current_chunk)
                        current_chunk = part
                    else:
                        current_chunk = part
            if current_chunk:
                chunks.append(current_chunk)
        else:
            chunks = [selection_text]
        
        # Position selection text in the plot area, avoiding legend
        left_margin = upper_pad.GetLeftMargin()
        start_x = left_margin + 0.03
        start_y = 0.73  # Start high in the plot area, just below CMS label
        line_spacing = 0.04  # Increased spacing for better readability
        
        for i, chunk in enumerate(chunks):
            y_pos = start_y - i * line_spacing
            latex_sel.DrawLatexNDC(start_x, y_pos, chunk)
    
    # Plot ratio in lower pad
    lower_pad.cd()
    
    # Create ratio histogram
    ratio_hist = data_hist.Clone(f"{data_hist.GetName()}_ratio")
    ratio_hist.Divide(mc_hist)
    
    # Style ratio histogram
    ratio_hist.SetMarkerColor(datamc_config.ratio_color)
    ratio_hist.SetMarkerStyle(datamc_config.ratio_marker_style)
    ratio_hist.SetMarkerSize(datamc_config.ratio_marker_size)
    ratio_hist.SetLineColor(datamc_config.ratio_color)
    
    # Configure ratio plot axes
    ratio_hist.GetYaxis().SetTitle(datamc_config.ratio_title)
    ratio_hist.GetYaxis().SetTitleSize(0.12)
    ratio_hist.GetYaxis().SetTitleOffset(0.5)
    ratio_hist.GetYaxis().SetLabelSize(0.1)
    ratio_hist.GetYaxis().SetRangeUser(datamc_config.ratio_y_min, datamc_config.ratio_y_max)
    ratio_hist.GetYaxis().SetNdivisions(505)
    
    ratio_hist.GetXaxis().SetTitleSize(0.12)
    ratio_hist.GetXaxis().SetTitleOffset(1.0)
    ratio_hist.GetXaxis().SetLabelSize(0.1)
    
    ratio_hist.Draw("PE")
    
    # Add reference line at y=1
    line = ROOT.TLine(ratio_hist.GetXaxis().GetXmin(), 1.0, 
                     ratio_hist.GetXaxis().GetXmax(), 1.0)
    line.SetLineStyle(2)
    line.SetLineColor(ROOT.kBlack)
    line.Draw()
    
    # Save canvas
    filename = f"datamc_{hist_name}"
    if jet_dir:
        filename += f"_{jet_dir}"
    if cent_bin:
        filename += f"_{cent_bin}"
    
    save_canvas(c, outdir, filename, formats, jet_dir, cent_bin, config)
    
    c.Close()
    return True


def plot_datamc_overlay_multiple(data_file, mc_file, hist_configs, config, outdir, formats, jet_dir="", cent_bin=""):
    """
    Create overlay plot comparing multiple histograms between Data and MC.
    
    Args:
        data_file: ROOT file containing data histograms
        mc_file: ROOT file containing MC histograms
        hist_configs: List of HistogramConfig objects to overlay
        config: Configuration dictionary
        outdir: Output directory
        formats: List of output formats
        jet_dir: Jet directory name (optional)
        cent_bin: Centrality bin name (optional)
    """
    datamc_config = get_datamc_config(config)
    
    data_hists = []
    mc_hists = []
    labels = []
    
    # Load all histograms
    for hist_config in hist_configs:
        data_path = find_histogram_in_structure(data_file, hist_config.name, jet_dir, cent_bin)
        mc_path = find_histogram_in_structure(mc_file, hist_config.name, jet_dir, cent_bin)
        
        if data_path and mc_path:
            data_hist = load_histogram_from_file(data_file, data_path)
            mc_hist = load_histogram_from_file(mc_file, mc_path)
            
            if data_hist and mc_hist:
                data_hists.append(data_hist)
                mc_hists.append(mc_hist)
                labels.append(hist_config.title or hist_config.name)
    
    if not data_hists or not mc_hists:
        return False
    
    # Create overlay plot
    all_hists = data_hists + mc_hists
    all_labels = [f"{label} (Data)" for label in labels] + [f"{label} (MC)" for label in labels]
    
    plot_name = f"datamc_overlay"
    if jet_dir:
        plot_name += f"_{jet_dir}"
    if cent_bin:
        plot_name += f"_{cent_bin}"
    
    # Use the first histogram config for styling
    plot_overlay_advanced(all_hists, all_labels, config, outdir, plot_name, formats, 
                         hist_configs[0], jet_dir, cent_bin)
    
    return True


def process_individual_file(root_file, config, base_outdir, formats, hist_configs, jet_dirs, args, file_type=""):
    """
    Process a single ROOT file individually, similar to the main single-file processing logic.
    Used by enhanced Data-MC mode to create individual plots alongside comparison plots.
    
    Args:
        root_file: Opened ROOT file
        config: Configuration dictionary
        base_outdir: Base output directory for this file
        formats: List of output formats
        hist_configs: Dictionary of histogram configurations
        jet_dirs: List of jet directories to process
        args: Command line arguments
        file_type: String identifier for the file type (e.g., "Data", "MC")
        
    Returns:
        int: Number of plots created
    """
    print(f"Processing {file_type} file: {root_file.GetName()}")
    
    # Create output directories for this file similar to main processing
    individual_output_dirs = create_output_dirs(base_outdir, config, jet_dirs)
    
    plotted_count = 0
    use_nested = config.get('UseNestedStructure', '1') == '1'
    
    if use_nested:
        # Process nested structure (same logic as main function)
        for jet_dir in jet_dirs:
            jet_dir = jet_dir.strip()
            if args.verbose:
                print(f"  Processing Jet Collection: {jet_dir}")
            
            cent_bins = [args.cent_bin] if args.cent_bin else get_available_centrality_bins(root_file, jet_dir, config)
            if not cent_bins:
                # Parse from config
                cent_config = config.get('CentralityBins', '0,60,180').split(',')
                cent_bins = []
                for i in range(len(cent_config)-1):
                    cent_bins.append(f"cent{int(float(cent_config[i]))}to{int(float(cent_config[i+1]))}")
            
            for cent_bin in cent_bins:
                cent_bin = cent_bin.strip()
                if args.verbose:
                    print(f"    Processing Centrality Bin: {cent_bin}")
                
                # Plot individual histograms
                for plot_key, hist_config in hist_configs.items():
                    if args.test and plotted_count >= 5:
                        break
                        
                    hist = get_histogram_from_path(root_file, jet_dir, cent_bin, hist_config.name, config)
                    if not hist:
                        if args.verbose:
                            print(f"      Warning: {hist_config.name} not found in {jet_dir}/{cent_bin}")
                        continue
                    
                    if hist_config.plot_type == '1D':
                        # Use jet-specific directory if available, otherwise use default
                        jet_specific_dir_key = f'1D_{jet_dir}'
                        if jet_specific_dir_key in individual_output_dirs:
                            outdir = individual_output_dirs[jet_specific_dir_key]
                        else:
                            outdir = individual_output_dirs.get('1D', individual_output_dirs['base'])
                            
                        plot_histogram_1d(hist, hist_config, config, outdir, formats, jet_dir, cent_bin)
                        if args.verbose:
                            print(f"      Plotted 1D: {hist_config.name}_{jet_dir}_{cent_bin}")
                        plotted_count += 1
                        
                    elif hist_config.plot_type == '2D':
                        # Use jet-specific directory if available, otherwise use default
                        jet_specific_dir_key = f'2D_{jet_dir}'
                        if jet_specific_dir_key in individual_output_dirs:
                            outdir = individual_output_dirs[jet_specific_dir_key]
                        else:
                            outdir = individual_output_dirs.get('2D', individual_output_dirs['base'])
                            
                        plot_histogram_2d(hist, hist_config, config, outdir, formats, jet_dir, cent_bin)
                        if args.verbose:
                            print(f"      Plotted 2D: {hist_config.name}_{jet_dir}_{cent_bin}")
                        plotted_count += 1
            
            # Create centrality overlay plots for this jet collection if enabled
            if config.get('OverlayByCentrality', '0') == '1' and not args.test:
                if args.verbose:
                    print(f"    Creating Centrality Overlays for {jet_dir}")
                overlays = get_overlay_plots(config)
                for overlay_key in overlays:
                    hist_config = hist_configs.get(overlay_key)
                    if hist_config:
                        outdir = os.path.join(individual_output_dirs.get('overlays', individual_output_dirs['base']), jet_dir)
                        os.makedirs(outdir, exist_ok=True)
                        
                        if plot_centrality_overlay(root_file, jet_dir, hist_config.name, config, outdir, formats):
                            if args.verbose:
                                print(f"      Created centrality overlay: {overlay_key} for {jet_dir}")
                            plotted_count += 1
        
        # Create jet collection overlay plots if enabled
        if config.get('OverlayByJetCollection', '0') == '1' and not args.test:
            if args.verbose:
                print(f"  Creating Jet Collection Overlays")
            overlays = get_overlay_plots(config)
            for overlay_key in overlays:
                hist_config = hist_configs.get(overlay_key)
                if hist_config:
                    # Create overlays for each centrality bin
                    for cent_bin in get_available_centrality_bins(root_file, jet_dirs[0], config):
                        cent_specific_dir = os.path.join(individual_output_dirs.get('overlays', individual_output_dirs['base']), cent_bin)
                        os.makedirs(cent_specific_dir, exist_ok=True)
                        
                        if plot_jet_collection_overlay(root_file, cent_bin, hist_config.name, config, cent_specific_dir, formats):
                            if args.verbose:
                                print(f"    Created jet collection overlay: {overlay_key} for {cent_bin}")
                            plotted_count += 1
    
    else:
        # Process flat structure (backward compatibility)
        if args.verbose:
            print(f"  Processing Flat ROOT Structure")
        for plot_key, hist_config in hist_configs.items():
            if args.test and plotted_count >= 5:
                break
                
            hist = root_file.Get(hist_config.name)
            if not hist:
                if args.verbose:
                    print(f"    Warning: Histogram {hist_config.name} not found in file.")
                continue
            
            if hist_config.plot_type == '1D':
                outdir = individual_output_dirs.get('1D', individual_output_dirs['base'])
                plot_histogram_1d(hist, hist_config, config, outdir, formats)
                if args.verbose:
                    print(f"    Plotted 1D: {hist_config.name}")
                plotted_count += 1
    
    print(f"  {file_type} file processing completed: {plotted_count} plots created")
    return plotted_count


def clean_output_directory(outdir, batch_mode=False, verbose=False):
    """
    Clean output directory with user confirmation.
    
    Args:
        outdir: Output directory path
        batch_mode: If True, skip interactive confirmation and auto-clean
        verbose: Enable verbose output
        
    Returns:
        bool: True if cleanup was performed or directory was empty, False if cancelled
    """
    import shutil
    import glob
    
    if not os.path.exists(outdir):
        if verbose:
            print(f"Output directory {outdir} does not exist, will be created.")
        return True
    
    # Check if directory contains any files (recursively)
    file_count = 0
    total_size = 0
    
    for root, dirs, files in os.walk(outdir):
        file_count += len(files)
        for file in files:
            file_path = os.path.join(root, file)
            try:
                total_size += os.path.getsize(file_path)
            except OSError:
                pass  # Handle broken symlinks or permission issues
    
    if file_count == 0:
        if verbose:
            print(f"Output directory {outdir} is empty.")
        return True
    
    # Convert size to human readable format
    def format_size(size_bytes):
        if size_bytes == 0:
            return "0 B"
        size_names = ["B", "KB", "MB", "GB"]
        import math
        i = int(math.floor(math.log(size_bytes, 1024)))
        p = math.pow(1024, i)
        s = round(size_bytes / p, 2)
        return f"{s} {size_names[i]}"
    
    print(f"\n⚠️  Output directory contains {file_count} files ({format_size(total_size)})")
    print(f"📁 Directory: {outdir}")
    
    if batch_mode:
        print("🔄 Batch mode: Automatically cleaning output directory...")
        try:
            shutil.rmtree(outdir)
            os.makedirs(outdir, exist_ok=True)
            print("✅ Output directory cleaned successfully.")
            return True
        except Exception as e:
            print(f"❌ Error cleaning directory: {e}")
            return False
    
    # Interactive confirmation
    print("\nOptions:")
    print("  [y/yes] - Delete all files and continue")
    print("  [n/no]  - Keep existing files and continue")
    print("  [c/cancel] - Cancel and exit")
    
    while True:
        try:
            choice = input("\nYour choice (y/n/c): ").lower().strip()
            
            if choice in ['y', 'yes']:
                try:
                    print("🔄 Cleaning output directory...")
                    shutil.rmtree(outdir)
                    os.makedirs(outdir, exist_ok=True)
                    print("✅ Output directory cleaned successfully.")
                    return True
                except Exception as e:
                    print(f"❌ Error cleaning directory: {e}")
                    return False
                    
            elif choice in ['n', 'no']:
                print("📝 Keeping existing files, new plots will be added/overwritten.")
                return True
                
            elif choice in ['c', 'cancel']:
                print("❌ Operation cancelled by user.")
                return False
                
            else:
                print("Invalid choice. Please enter 'y', 'n', or 'c'.")
                
        except KeyboardInterrupt:
            print("\n❌ Operation cancelled by user.")
            return False
        except EOFError:
            print("\n❌ Operation cancelled.")
            return False


def main():
    """Enhanced main function with nested ROOT structure support and Data-MC comparison."""
    parser = argparse.ArgumentParser(description="CMS GammaJet Advanced Plotter - Enhanced Version with Data-MC Comparison")
    parser.add_argument('-r', '--rootfile', required=True, help='Input ROOT file (or Data file for Data-MC comparison)')
    parser.add_argument('-c', '--config', required=True, help='Plotting config file')
    parser.add_argument('-o', '--outdir', default='/eos/user/b/bharikri/www/Run3GammaJet/2025_05_24/2023_PbPb/MC/plots', help='Output directory for images')
    parser.add_argument('--batch', action='store_true', help='Run in batch mode (no GUI)')
    parser.add_argument('--verbose', action='store_true', help='Verbose output')
    parser.add_argument('--test', action='store_true', help='Test mode - only plot a few histograms')
    parser.add_argument('--jet-dir', help='Specific jet directory to process (e.g., AK4Z2)')
    parser.add_argument('--cent-bin', help='Specific centrality bin to process (e.g., cent0to60)')
    parser.add_argument('--overlay-jets', help='Comma-separated list of jet collections to overlay')
    parser.add_argument('--overlay-cents', help='Comma-separated list of centrality bins to overlay')
    
    # Data-MC comparison options
    parser.add_argument('--mc-file', help='MC ROOT file for Data-MC comparison')
    parser.add_argument('--datamc-mode', action='store_true', help='Enable Data-MC comparison mode')
    parser.add_argument('--datamc-plots', help='Comma-separated list of plots to compare (if not specified, all available plots will be compared)')
    
    # Output management options
    parser.add_argument('--clean-output', action='store_true', help='Clean output directory before generating new plots (with user confirmation unless in batch mode)')
    
    args = parser.parse_args()

    if args.batch:
        ROOT.gROOT.SetBatch(True)

    # Parse configuration
    config = parse_config(args.config)
    
    # Override config with command line options
    if args.overlay_jets:
        config['OverlayJetCollections'] = args.overlay_jets
    if args.overlay_cents:
        config['OverlayCentralityBins'] = args.overlay_cents
    if args.verbose:
        config['Verbose'] = '1'
    
    # Handle output directory cleanup if requested
    if args.clean_output:
        if not clean_output_directory(args.outdir, args.batch, args.verbose):
            print("❌ Operation cancelled due to output directory cleanup.")
            return 1
    
    # Check for Data-MC comparison mode
    if args.datamc_mode or args.mc_file:
        if not args.mc_file:
            print("Error: --mc-file is required for Data-MC comparison mode")
            return 1
        
        # Open both files
        data_file = ROOT.TFile.Open(args.rootfile)
        mc_file = ROOT.TFile.Open(args.mc_file)
        
        if not data_file or data_file.IsZombie():
            print(f"Error: Could not open data ROOT file: {args.rootfile}")
            return 1
        if not mc_file or mc_file.IsZombie():
            print(f"Error: Could not open MC ROOT file: {args.mc_file}")
            return 1
        
        print(f"=== Data-MC Comparison Mode ===")
        print(f"Data file: {args.rootfile}")
        print(f"MC file: {args.mc_file}")
        
        # Get available jet directories (use data file as reference)
        jet_dirs = [args.jet_dir] if args.jet_dir else get_available_jet_dirs(data_file, config)
        if not jet_dirs:
            jet_dirs = [x.strip() for x in config.get('JetDirectories', 'AK4Z2').split(',')]
        
        # Set up output directories
        output_dirs = create_output_dirs(args.outdir, config, jet_dirs)
        
        # Add DataMC subdirectory
        datamc_outdir = os.path.join(args.outdir, 'DataMC')
        os.makedirs(datamc_outdir, exist_ok=True)
        
        # Set CMS style
        set_cms_style(config)
        
        # Get plot formats
        formats = get_plot_formats(config)
        
        # Get histogram configurations
        hist_configs = get_histogram_configs(config)
        
        # Determine which plots to compare
        if args.datamc_plots:
            compare_plots = [x.strip() for x in args.datamc_plots.split(',')]
            # Filter hist_configs to only include requested plots
            hist_configs = {k: v for k, v in hist_configs.items() 
                           if k in compare_plots or v.name in compare_plots}
        
        print(f"Comparing {len(hist_configs)} histogram types across {len(jet_dirs)} jet collections")
        
        plotted_count = 0
        use_nested = config.get('UseNestedStructure', '1') == '1'
        
        # Process Data-MC comparisons
        if use_nested:
            for jet_dir in jet_dirs:
                jet_dir = jet_dir.strip()
                print(f"\n=== Data-MC Comparison for Jet Collection: {jet_dir} ===")
                
                cent_bins = [args.cent_bin] if args.cent_bin else get_available_centrality_bins(data_file, jet_dir, config)
                if not cent_bins:
                    cent_config = config.get('CentralityBins', '0,60,180').split(',')
                    cent_bins = [f"cent{cent_config[i]}to{cent_config[i+1]}" 
                               for i in range(len(cent_config)-1)]
                
                for cent_bin in cent_bins:
                    cent_bin = cent_bin.strip()
                    print(f"  Processing centrality bin: {cent_bin}")
                    
                    jet_cent_outdir = os.path.join(datamc_outdir, jet_dir, cent_bin)
                    os.makedirs(jet_cent_outdir, exist_ok=True)
                    
                    for plot_key, hist_config in hist_configs.items():
                        if args.test and plotted_count >= 10:
                            break
                        
                        if plot_datamc_comparison(data_file, mc_file, hist_config.name, 
                                                config, jet_cent_outdir, formats, 
                                                jet_dir, cent_bin):
                            if args.verbose:
                                print(f"    Created Data-MC comparison: {hist_config.name}")
                            plotted_count += 1
        else:
            # Flat structure Data-MC comparison
            print(f"\n=== Data-MC Comparison (Flat Structure) ===")
            for plot_key, hist_config in hist_configs.items():
                if args.test and plotted_count >= 5:
                    break
                
                if plot_datamc_comparison(data_file, mc_file, hist_config.name, 
                                        config, datamc_outdir, formats):
                    if args.verbose:
                        print(f"Created Data-MC comparison: {hist_config.name}")
                    plotted_count += 1
        
        # Close files
        data_file.Close()
        mc_file.Close()
        
        # Enhanced Data-MC Mode: Also create individual plots for Data and MC files
        print(f"\n=== Creating Individual Data and MC Plots ===")
        
        # Create Data and MC subdirectories
        data_outdir = os.path.join(args.outdir, 'Data')
        mc_outdir = os.path.join(args.outdir, 'MC')
        os.makedirs(data_outdir, exist_ok=True)
        os.makedirs(mc_outdir, exist_ok=True)
        
        # Keep track of individual plots created
        data_plots = 0
        mc_plots = 0
        
        # Process Data file individually
        print(f"\n--- Processing Data file individually ---")
        data_file_individual = ROOT.TFile.Open(args.rootfile)
        if data_file_individual and not data_file_individual.IsZombie():
            data_plots += process_individual_file(data_file_individual, config, data_outdir, 
                                                formats, hist_configs, jet_dirs, args, "Data")
            data_file_individual.Close()
        else:
            print(f"Warning: Could not reopen data file for individual processing")
        
        # Process MC file individually  
        print(f"\n--- Processing MC file individually ---")
        mc_file_individual = ROOT.TFile.Open(args.mc_file)
        if mc_file_individual and not mc_file_individual.IsZombie():
            mc_plots += process_individual_file(mc_file_individual, config, mc_outdir,
                                              formats, hist_configs, jet_dirs, args, "MC")
            mc_file_individual.Close()
        else:
            print(f"Warning: Could not reopen MC file for individual processing")
        
        print(f"\n=== Enhanced Data-MC Mode Summary ===")
        print(f"Data-MC comparison plots: {plotted_count}")
        print(f"Individual Data plots: {data_plots}")
        print(f"Individual MC plots: {mc_plots}")
        print(f"Total plots created: {plotted_count + data_plots + mc_plots}")
        print(f"DataMC comparison output: {datamc_outdir}")
        print(f"Data individual output: {data_outdir}")  
        print(f"MC individual output: {mc_outdir}")
        print("Enhanced Data-MC analysis completed successfully!")
        
        return 0
    
    # Original single-file processing mode
    # Open ROOT file first to get jet directories
    f = ROOT.TFile.Open(args.rootfile)
    if not f or f.IsZombie():
        print(f"Error: Could not open ROOT file: {args.rootfile}")
        return 1

    # Get available jet directories
    jet_dirs = [args.jet_dir] if args.jet_dir else get_available_jet_dirs(f, config)
    if not jet_dirs:
        jet_dirs = [x.strip() for x in config.get('JetDirectories', 'AK4Z2').split(',')]
    
    # Now set up output directories with jet-specific subdirectories
    output_dirs = create_output_dirs(args.outdir, config, jet_dirs)
    
    # Set CMS style
    set_cms_style(config)
    
    # Get plot formats
    formats = get_plot_formats(config)

    print(f"Processing ROOT file: {args.rootfile}")
    print(f"Using config file: {args.config}")
    print(f"Output directory: {args.outdir}")
    print(f"Found jet directories: {jet_dirs}")
    
    # Get histogram configurations
    hist_configs = get_histogram_configs(config)
    
    plotted_count = 0
    use_nested = config.get('UseNestedStructure', '1') == '1'
    
    if use_nested:
        # Process nested structure
        for jet_dir in jet_dirs:
            jet_dir = jet_dir.strip()
            print(f"\n=== Single File Processing for Jet Collection: {jet_dir} ===")
            
            cent_bins = [args.cent_bin] if args.cent_bin else get_available_centrality_bins(f, jet_dir, config)
            if not cent_bins:
                # Parse from config
                cent_config = config.get('CentralityBins', '0,60,180').split(',')
                cent_bins = []
                for i in range(len(cent_config)-1):
                    cent_bins.append(f"cent{int(float(cent_config[i]))}to{int(float(cent_config[i+1]))}")
            
            for cent_bin in cent_bins:
                cent_bin = cent_bin.strip()
                print(f"  Processing centrality bin: {cent_bin}")
                
                # Plot individual histograms
                for plot_key, hist_config in hist_configs.items():
                    if args.test and plotted_count >= 5:
                        break
                        
                    hist = get_histogram_from_path(f, jet_dir, cent_bin, hist_config.name, config)
                    if not hist:
                        if args.verbose:
                            print(f"Warning: {hist_config.name} not found in {jet_dir}/{cent_bin}")
                        continue
                    
                    if hist_config.plot_type == '1D':
                        # Use jet-specific directory if available, otherwise use default
                        jet_specific_dir_key = f'1D_{jet_dir}'
                        if jet_specific_dir_key in output_dirs:
                            outdir = output_dirs[jet_specific_dir_key]
                        else:
                            outdir = output_dirs.get('1D', output_dirs['base'])
                            
                        plot_histogram_1d(hist, hist_config, config, outdir, formats, jet_dir, cent_bin)
                        if args.verbose:
                            print(f"Plotted 1D: {hist_config.name}_{jet_dir}_{cent_bin}")
                        plotted_count += 1
                    elif hist_config.plot_type == '2D':
                        # Use jet-specific directory if available, otherwise use default
                        jet_specific_dir_key = f'2D_{jet_dir}'
                        if jet_specific_dir_key in output_dirs:
                            outdir = output_dirs[jet_specific_dir_key]
                        else:
                            outdir = output_dirs.get('2D', output_dirs['base'])
                            
                        plot_histogram_2d(hist, hist_config, config, outdir, formats, jet_dir, cent_bin)
                        if args.verbose:
                            print(f"Plotted 2D: {hist_config.name}_{jet_dir}_{cent_bin}")
                        plotted_count += 1
            
            # Create centrality overlay plots for this jet collection
            if config.get('OverlayByCentrality', '0') == '1':
                print(f"\n--- Creating Centrality Overlays for {jet_dir} ---")
                overlays = get_overlay_plots(config)
                for overlay_key in overlays:
                    if args.test and plotted_count >= 10:
                        break
                        
                    hist_config = hist_configs.get(overlay_key)
                    if hist_config:
                        outdir = os.path.join(output_dirs.get('overlays', output_dirs['base']), jet_dir)
                        os.makedirs(outdir, exist_ok=True)
                        
                        if plot_centrality_overlay(f, jet_dir, hist_config.name, config, outdir, formats):
                            if args.verbose:
                                print(f"Created centrality overlay: {overlay_key} for {jet_dir}")
                            plotted_count += 1
        
        # Create jet collection overlay plots
        if config.get('OverlayByJetCollection', '0') == '1':
            print(f"\n=== Creating Jet Collection Overlays ===")
            overlays = get_overlay_plots(config)
            for overlay_key in overlays:
                if args.test and plotted_count >= 15:
                    break
                    
                hist_config = hist_configs.get(overlay_key)
                if hist_config:
                    # Create overlays for each centrality bin
                    for cent_bin in get_available_centrality_bins(f, jet_dirs[0], config):
                        cent_specific_dir = os.path.join(output_dirs.get('overlays', output_dirs['base']), cent_bin)
                        os.makedirs(cent_specific_dir, exist_ok=True)
                        
                        if plot_jet_collection_overlay(f, cent_bin, hist_config.name, config, cent_specific_dir, formats):
                            if args.verbose:
                                print(f"Created jet collection overlay: {overlay_key} for {cent_bin}")
                            plotted_count += 1
    
    else:
        # Process flat structure (backward compatibility)
        print("\n=== Processing Flat ROOT Structure ===")
        for plot_key, hist_config in hist_configs.items():
            if args.test and plotted_count >= 5:
                break
                
            hist = f.Get(hist_config.name)
            if not hist:
                if args.verbose:
                    print(f"Warning: Histogram {hist_config.name} not found in file.")
                continue
            
            if hist_config.plot_type == '1D':
                outdir = output_dirs.get('1D', output_dirs['base'])
                plot_histogram_1d(hist, hist_config, config, outdir, formats)
                if args.verbose:
                    print(f"Plotted 1D: {hist_config.name}")
                plotted_count += 1

    f.Close()
    
    print(f"\n=== Summary ===")
    print(f"Total plots created: {plotted_count}")
    print(f"Output saved to: {args.outdir}")
    print("Plotting completed successfully!")
    
    return 0


if __name__ == "__main__":
    sys.exit(main())
