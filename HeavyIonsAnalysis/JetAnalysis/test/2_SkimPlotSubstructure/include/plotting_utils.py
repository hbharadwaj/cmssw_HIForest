#!/usr/bin/env python3
"""
CMS Gamma-Jet Analysis Plotting Utilities

Comprehensive utility module for gamma-jet analysis plotting,
including configuration management, histogram handling, styling,
and multi-file overlay capabilities.
"""

import os
import sys
import re
import ROOT
import logging
import subprocess
from collections import defaultdict

# =============================================================================
# CONFIGURATION MANAGEMENT
# =============================================================================
def parse_config(config_path):
    """Parse the plotting config file into a dictionary of keys and values. Supports HistogramConfigFile chaining."""
    config = {}
    with open(config_path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            if ':' in line:
                key, value = line.split(':', 1)
                config[key.strip()] = value.strip()
    # If HistogramConfigFile is present, load and merge it
    hist_config_file = config.get('HistogramConfigFile', '')
    if hist_config_file:
        # Support relative paths
        if not os.path.isabs(hist_config_file):
            hist_config_file = os.path.join(os.path.dirname(config_path), hist_config_file)
        with open(hist_config_file) as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith('#'):
                    continue
                if ':' in line:
                    key, value = line.split(':', 1)
                    # Only add if not already present (main config overrides)
                    if key.strip() not in config:
                        config[key.strip()] = value.strip()
    return config

# =============================================================================
# DATA CLASSES AND CONFIGURATION OBJECTS
# =============================================================================
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
        # Default draw options
        if self.plot_type == '2D':
            self.draw_option = config_dict.get(f'Histogram.{plot_key}.DrawOption', 'colz')
        else:
            self.draw_option = config_dict.get(f'Histogram.{plot_key}.DrawOption', 'E1 P0')
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
        self.data_color = int(config_dict.get('DataMC.DataColor', 1))
        self.data_marker_style = int(config_dict.get('DataMC.DataMarkerStyle', 20))
        self.data_marker_size = float(config_dict.get('DataMC.DataMarkerSize', 0.8))
        self.data_line_width = int(config_dict.get('DataMC.DataLineWidth', 2))
        self.data_label = config_dict.get('DataMC.DataLabel', 'Data')
        self.mc_color = int(config_dict.get('DataMC.MCColor', 2))
        self.mc_marker_style = int(config_dict.get('DataMC.MCMarkerStyle', 21))
        self.mc_marker_size = float(config_dict.get('DataMC.MCMarkerSize', 0.8))
        self.mc_line_width = int(config_dict.get('DataMC.MCLineWidth', 2))
        self.mc_label = config_dict.get('DataMC.MCLabel', 'MC')
        self.ratio_y_min = float(config_dict.get('DataMC.RatioYMin', 0.5))
        self.ratio_y_max = float(config_dict.get('DataMC.RatioYMax', 1.5))
        self.ratio_title = config_dict.get('DataMC.RatioTitle', 'Data/MC')
        self.ratio_color = int(config_dict.get('DataMC.RatioColor', 1))
        self.ratio_marker_style = int(config_dict.get('DataMC.RatioMarkerStyle', 20))
        self.ratio_marker_size = float(config_dict.get('DataMC.RatioMarkerSize', 0.6))
        self.canvas_width = int(config_dict.get('DataMC.CanvasWidth', 800))
        self.canvas_height = int(config_dict.get('DataMC.CanvasHeight', 800))
        self.upper_pad_height = float(config_dict.get('DataMC.UpperPadHeight', 0.7))
        self.lower_pad_height = float(config_dict.get('DataMC.LowerPadHeight', 0.3))
        self.legend_x1 = float(config_dict.get('DataMC.LegendX1', 0.65))
        self.legend_y1 = float(config_dict.get('DataMC.LegendY1', 0.75))
        self.legend_x2 = float(config_dict.get('DataMC.LegendX2', 0.85))
        self.legend_y2 = float(config_dict.get('DataMC.LegendY2', 0.85))
        self.log_y = config_dict.get('DataMC.LogY', '0') == '1'
        self.show_ratio_plot = config_dict.get('DataMC.ShowRatioPlot', '1') == '1'  # NEW

# =============================================================================
# HISTOGRAM SOURCE AND CACHING
# =============================================================================
class HistogramSource:
    """Unified class for handling histogram retrieval from ROOT files with caching."""
    def __init__(self, root_file, name=None, config=None):
        self.file = root_file
        self.name = name or os.path.basename(root_file.GetName())
        self.config = config or {}
        self._cache = {}
        self._path_cache = {}
        self.use_nested = self.config.get('UseNestedStructure', '1') == '1'
        self.hist_subdir = self.config.get('HistogramSubdir', '').strip()
        self.verbose = self.config.get('Verbose', '0') == '1'

    def get_histogram(self, hist_name, jet_dir=None, cent_bin=None, clone=True):
        cache_key = f"{hist_name}_{jet_dir or 'None'}_{cent_bin or 'None'}"
        if cache_key in self._cache:
            hist = self._cache[cache_key]
            if hist and clone:
                cloned = hist.Clone(f"{hist.GetName()}_clone_{id(self)}")
                cloned.SetDirectory(0)
                return cloned
            return hist
        hist_path = self._find_histogram_path(hist_name, jet_dir, cent_bin)
        if not hist_path:
            if self.verbose:
                context = f"({jet_dir}/{cent_bin})" if jet_dir and cent_bin else ""
                logging.warning(f"    Warning: {hist_name} not found {context} in {self.name}")
            self._cache[cache_key] = None
            return None
        hist = self.file.Get(hist_path)
        if not hist:
            if self.verbose:
                logging.warning(f"    Warning: Failed to load {hist_path} from {self.name}")
            self._cache[cache_key] = None
            return None
        cached_hist = hist.Clone(f"{hist.GetName()}_cached")
        cached_hist.SetDirectory(0)
        self._cache[cache_key] = cached_hist
        self._path_cache[cache_key] = hist_path
        if clone:
            cloned = cached_hist.Clone(f"{hist.GetName()}_clone_{id(self)}")
            cloned.SetDirectory(0)
            return cloned
        return cached_hist

    def _find_histogram_path(self, hist_name, jet_dir=None, cent_bin=None):
        cache_key = f"{hist_name}_{jet_dir or 'None'}_{cent_bin or 'None'}"
        if cache_key in self._path_cache:
            return self._path_cache[cache_key]
        paths_to_try = []
        if self.use_nested and jet_dir and cent_bin:
            if self.hist_subdir:
                paths_to_try.extend([
                    f"{jet_dir}/{cent_bin}/{self.hist_subdir}/{hist_name}",
                    f"{cent_bin}/{jet_dir}/{self.hist_subdir}/{hist_name}",
                    f"{cent_bin}/General/{self.hist_subdir}/{hist_name}"
                ])
            else:
                paths_to_try.extend([
                    f"{jet_dir}/{cent_bin}/{hist_name}",
                    f"{cent_bin}/{jet_dir}/{hist_name}",
                    f"{cent_bin}/General/{hist_name}"
                ])
        if jet_dir:
            paths_to_try.append(f"{jet_dir}/{hist_name}")
        if cent_bin:
            paths_to_try.extend([
                f"{cent_bin}/General/{hist_name}",
                f"{cent_bin}/{hist_name}"
            ])
        if not self.use_nested:
            if jet_dir and cent_bin:
                paths_to_try.append(f"{hist_name}_{jet_dir}_{cent_bin}")
            if jet_dir:
                paths_to_try.append(f"{hist_name}_{jet_dir}")
            if cent_bin:
                paths_to_try.append(f"{hist_name}_{cent_bin}")
        paths_to_try.append(hist_name)
        for path in paths_to_try:
            if self.file.Get(path):
                return path
        return None

    def list_available_histograms(self, jet_dir=None, cent_bin=None, pattern=None):
        hist_names = []
        search_dirs = []
        if self.use_nested and jet_dir and cent_bin:
            search_dirs.extend([
                f"{jet_dir}/{cent_bin}",
                f"{cent_bin}/{jet_dir}",
                f"{cent_bin}/General"
            ])
        elif jet_dir:
            search_dirs.append(jet_dir)
        elif cent_bin:
            search_dirs.extend([f"{cent_bin}/General", cent_bin])
        else:
            search_dirs.append("")
        for search_dir in search_dirs:
            dir_obj = self.file.GetDirectory(search_dir) if search_dir else self.file
            if not dir_obj:
                continue
            key_list = dir_obj.GetListOfKeys()
            for key in key_list:
                obj = key.ReadObj()
                if obj and (obj.InheritsFrom("TH1") or obj.InheritsFrom("TH2")):
                    hist_name = obj.GetName()
                    if pattern and not re.match(pattern, hist_name):
                        continue
                    if hist_name not in hist_names:
                        hist_names.append(hist_name)
        return sorted(hist_names)

    def get_available_jet_dirs(self):
        """
        Return all unique jet directories found recursively under all centrality bins or at the top level.
        If JetDirectories is specified in config, use those as candidates, but only return those that exist somewhere in the file.
        """
        jet_dirs_config = self.config.get('JetDirectories', '')
        candidates = set()
        # Try config first
        if jet_dirs_config:
            for jet_dir in jet_dirs_config.split(','):
                jet_dir = jet_dir.strip()
                if jet_dir:
                    candidates.add(jet_dir)
        # Recursively search for jet dirs under all centrality bins
        cent_bins = self.get_available_centrality_bins()
        for cent_bin in cent_bins:
            cent_dir = self.file.GetDirectory(cent_bin)
            if cent_dir:
                for key in cent_dir.GetListOfKeys():
                    obj = key.ReadObj()
                    if obj.IsA().InheritsFrom('TDirectory'):  # Only directories
                        candidates.add(obj.GetName())
        # Also check top-level dirs
        for key in self.file.GetListOfKeys():
            obj = key.ReadObj()
            if obj.IsA().InheritsFrom('TDirectory'):
                candidates.add(obj.GetName())
        return sorted(candidates)

    def get_available_centrality_bins(self, jet_dir=None):
        """
        Return all unique centrality bins found recursively under all jet dirs or at the top level.
        If CentralityBins is specified in config, use those as candidates, but only return those that exist somewhere in the file.
        """
        cent_bins_config = self.config.get('CentralityBins', '')
        candidates = set()
        # Try config first
        if cent_bins_config:
            try:
                bins = [float(x.strip()) for x in cent_bins_config.split(',')]
                for i in range(len(bins)-1):
                    candidates.add(f"cent{int(bins[i])}to{int(bins[i+1])}")
            except:
                pass
        # Recursively search for cent bins under all jet dirs
        jet_dirs = []
        if jet_dir:
            jet_dirs = [jet_dir]
        else:
            for key in self.file.GetListOfKeys():
                obj = key.ReadObj()
                if obj.IsA().InheritsFrom('TDirectory'):
                    jet_dirs.append(obj.GetName())
        for jet in jet_dirs:
            jet_dir_obj = self.file.GetDirectory(jet)
            if jet_dir_obj:
                for key in jet_dir_obj.GetListOfKeys():
                    obj = key.ReadObj()
                    if obj.IsA().InheritsFrom('TDirectory'):
                        candidates.add(obj.GetName())
        # Also check top-level dirs
        for key in self.file.GetListOfKeys():
            obj = key.ReadObj()
            if obj.IsA().InheritsFrom('TDirectory') and obj.GetName().startswith('cent'):
                candidates.add(obj.GetName())
        return sorted(candidates)

    def clear_cache(self):
        self._cache.clear()
        self._path_cache.clear()

    def get_cache_stats(self):
        return {
            'cached_histograms': len(self._cache),
            'cached_paths': len(self._path_cache),
            'cache_hit_rate': len([h for h in self._cache.values() if h is not None]) / max(1, len(self._cache))
        }

# =============================================================================
# UTILITY FUNCTIONS (LABELS, COLORS, SELECTIONS, ETC.)
# =============================================================================
def get_centrality_label(cent_bin, config=None):
    if config:
        config_key = f"CentralityLabels.{cent_bin.replace('cent', '')}"
        if config_key in config:
            return config[config_key]
    match = re.search(r'cent(\d+)to(\d+)', cent_bin)
    if match:
        low, high = match.groups()
        return f"Centrality {low}-{high}%"
    return cent_bin

def get_jet_label(jet_dir):
    jet_labels = {
        'AK2Z1': 'AK R=0.2, Z_{cut}=0.1',
        'AK4Z2': 'AK R=0.4, Z_{cut}=0.2',
        'AK6Z3': 'AK R=0.6, Z_{cut}=0.3',
        'AK8Z4': 'AK R=0.8, Z_{cut}=0.4',
        'AK8Z5': 'AK R=0.8, Z_{cut}=0.5'
    }
    return jet_labels.get(jet_dir, jet_dir)

def parse_overlay_jets(config):
    overlay_jets = config.get('OverlayJetCollections', 'all').strip()
    if overlay_jets.lower() == 'all':
        return config.get('JetDirectories', 'AK4Z2').split(',')
    else:
        return [x.strip() for x in overlay_jets.split(',')]

def parse_overlay_centrality(config):
    overlay_cents = config.get('OverlayCentralityBins', 'all').strip()
    if overlay_cents.lower() == 'all':
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

def resolve_plot_type_path(overlay_name, config, jet_dir="", cent_bin="", histogram=""):
    """
    Resolve PlotType path for an overlay based on pattern matching and wildcard substitution.
    
    Args:
        overlay_name: Name of the overlay (e.g., "JetPt_AllJets_Central")
        config: Configuration dictionary
        jet_dir: Jet directory (e.g., "AK4Z2")
        cent_bin: Centrality bin (e.g., "cent0to60")
        histogram: Histogram name (e.g., "JetPt")
    
    Returns:
        Resolved path string with wildcards substituted
    """
    # Look for matching PlotType patterns in config
    plot_type_path = None
    
    # Try exact match first
    exact_key = f"PlotType.{overlay_name}"
    if exact_key in config:
        plot_type_path = config[exact_key]
    else:
        # Try pattern matching
        for key, value in config.items():
            if key.startswith('PlotType.'):
                pattern = key.replace('PlotType.', '')
                # Convert pattern to regex (simple * wildcard support)
                regex_pattern = pattern.replace('*', '.*')
                if re.match(f'^{regex_pattern}$', overlay_name):
                    plot_type_path = value
                    break
    
    # Use default if no match found
    if not plot_type_path:
        plot_type_path = config.get('PlotType.Default', 'uncategorized')
    
    # Substitute wildcards
    resolved_path = substitute_plot_type_wildcards(plot_type_path, jet_dir, cent_bin, histogram, overlay_name)
    
    return resolved_path


def substitute_plot_type_wildcards(path_template, jet_dir="", cent_bin="", histogram="", overlay_name=""):
    """
    Substitute wildcards in PlotType path template.
    
    Available wildcards:
    - {centrality} → cent0to60, cent60to180
    - {jetdir} → AK2Z1, AK4Z2, etc.
    - {histogram} → JetPt, DeltaPhi, etc.
    - {overlay_type} → centrality, jetcollection, datamc
    - {overlay_name} → full overlay name
    
    Args:
        path_template: Path template with wildcards
        jet_dir: Jet directory
        cent_bin: Centrality bin  
        histogram: Histogram name
        overlay_name: Full overlay name
    
    Returns:
        Path with wildcards resolved
    """
    
    # Determine overlay type from overlay name patterns
    overlay_type = "generic"
    overlay_lower = overlay_name.lower()
    if "centrality" in overlay_lower or "cent" in overlay_lower:
        overlay_type = "centrality"
    elif "jetcollection" in overlay_lower or "alljets" in overlay_lower:
        overlay_type = "jetcollection"  
    elif "datamc" in overlay_lower or overlay_name.startswith(("Data", "MC")):
        overlay_type = "datamc"
    
    # Extract histogram name from overlay name if not provided
    if not histogram:
        # Try to extract from overlay name (e.g., "JetPt_AllJets_Central" → "JetPt")
        parts = overlay_name.split('_')
        if parts:
            histogram = parts[0]
    
    # Perform substitutions
    resolved_path = path_template
    resolved_path = resolved_path.replace('{centrality}', cent_bin)
    resolved_path = resolved_path.replace('{jetdir}', jet_dir)
    resolved_path = resolved_path.replace('{histogram}', histogram)
    resolved_path = resolved_path.replace('{overlay_type}', overlay_type)
    resolved_path = resolved_path.replace('{overlay_name}', overlay_name)
    
    return resolved_path

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
    if hist_config.marker_style == 'auto':
        hist.SetMarkerStyle(get_auto_marker(color_index))
    else:
        marker_code = getattr(ROOT, hist_config.marker_style, ROOT.kFullCircle)
        hist.SetMarkerStyle(marker_code)
    hist.SetMarkerSize(hist_config.marker_size)
    hist.SetTitle('')
    if ';' in hist_config.title:
        parts = hist_config.title.split(';')
        if len(parts) > 1:
            hist.GetXaxis().SetTitle(parts[1])
        if len(parts) > 2:
            hist.GetYaxis().SetTitle(parts[2])

def get_auto_color(color_scheme, index, color_blind=False):
    """Get automatic color based on scheme and index."""
    if color_scheme == 'petroff6':
        colors = [ROOT.kBlue, ROOT.kRed, ROOT.kGreen+2, ROOT.kOrange+7, ROOT.kViolet, ROOT.kCyan+2]
    elif color_scheme == 'petroff10':
        colors = [ROOT.kBlue, ROOT.kRed, ROOT.kGreen+2, ROOT.kOrange+7, ROOT.kViolet, 
                  ROOT.kCyan+2, ROOT.kMagenta+2, ROOT.kYellow+2, ROOT.kGray+2, ROOT.kBlack]
    else:
        colors = [ROOT.kBlack, ROOT.kRed, ROOT.kBlue, ROOT.kGreen+2, ROOT.kOrange+7, ROOT.kViolet]
    
    return colors[index % len(colors)]

def get_auto_marker(index):
    """Get automatic marker style based on index."""
    markers = [ROOT.kFullCircle, ROOT.kFullSquare, ROOT.kFullTriangleUp, 
               ROOT.kFullDiamond, ROOT.kFullStar, ROOT.kFullCross]
    return markers[index % len(markers)]

def draw_cms_label(canvas, config, selection_text=""):
    """Draw CMS label on canvas with improved positioning."""
    
    canvas.cd()
    cms_label = config.get('CMSLabel', 'Preliminary')
    energy_text = config.get('CMSEnergyText', '5.36 TeV PbPb')
    luminosity = config.get('CMSLuminosity', '1.72 nb^{-1}')
    extra_text = config.get('CMSExtraText', '')
    
    # Get positioning from config
    cms_x = float(config.get('CMS.Position.X', '0.18'))
    cms_y = float(config.get('CMS.Position.Y', '0.88'))
    cms_size = float(config.get('CMS.TextSize', '0.042'))
    
    latex = ROOT.TLatex()
    latex.SetNDC()
    latex.SetTextAngle(0)
    latex.SetTextColor(ROOT.kBlack)
    
    # Draw CMS label
    latex.SetTextFont(61)  # CMS font
    latex.SetTextSize(cms_size)
    latex.SetTextAlign(11)  # left-aligned
    latex.DrawLatexNDC(cms_x, cms_y, "CMS")
    
    # Draw preliminary/simulation text
    if cms_label and cms_label.lower() != 'none':
        latex.SetTextFont(52)  # Italic font for Preliminary
        latex.SetTextSize(cms_size * 0.76)
        latex.DrawLatexNDC(cms_x + 0.12, cms_y, cms_label)
    
    # Draw energy and luminosity in top right
    if energy_text or luminosity:
        right_text = f"{energy_text}"
        if luminosity:
            right_text += f", {luminosity}"
        latex.SetTextFont(42)
        latex.SetTextAlign(31)  # right-aligned
        latex.DrawLatexNDC(0.95, cms_y, right_text)

def draw_selection_text_smart(canvas, config, jet_dir="", cent_bin=""):
    """Draw selection text with smart positioning."""
    
    # Get selection text
    selection_text = config.get('SelectionText', '')
    
    if not selection_text:
        return
    
    # Get positioning
    text_x = float(config.get('SelectionText.Position.X', '0.18'))
    text_y = float(config.get('SelectionText.Position.Y', '0.82'))
    text_size = float(config.get('SelectionText.TextSize', '0.032'))
    line_spacing = float(config.get('SelectionText.LineSpacing', '0.04'))
    
    # Add jet and centrality info
    text_lines = [selection_text]
    if jet_dir:
        jet_label = get_jet_label(jet_dir)
        text_lines.append(f"Jet: {jet_label}")
    if cent_bin:
        cent_label = get_centrality_label(cent_bin, config)
        text_lines.append(cent_label)
    
    # Draw text
    canvas.cd()
    latex = ROOT.TLatex()
    latex.SetNDC()
    latex.SetTextFont(42)
    latex.SetTextSize(text_size)
    latex.SetTextAlign(11)
    latex.SetTextColor(ROOT.kBlack)
    
    for i, line in enumerate(text_lines):
        y_pos = text_y - i * line_spacing
        latex.DrawLatexNDC(text_x, y_pos, line)

def get_plot_formats(config):
    """Get list of plot formats from config."""
    formats_str = config.get('PlotFormats', 'png')
    return [fmt.strip() for fmt in formats_str.split(',')]

def get_histogram_configs(config):
    """Get histogram configurations from config."""
    hist_configs = {}
    
    for key, value in config.items():
        if key.startswith('Histogram.') and key.endswith('.Name'):
            plot_key = key.replace('Histogram.', '').replace('.Name', '')
            hist_configs[plot_key] = HistogramConfig(value, config, plot_key)
    
    return hist_configs

def get_profile_configs(config):
    """Get profile configurations from config."""
    profile_configs = {}
    
    for key, value in config.items():
        if key.startswith('Profile.') and key.endswith('.Name'):
            plot_key = key.replace('Profile.', '').replace('.Name', '')
            profile_configs[plot_key] = ProfileConfig(value, config, plot_key)
    
    return profile_configs

def get_datamc_config(config):
    """Get Data-MC comparison configuration."""
    return DataMCConfig(config)

def get_overlay_plots(config):
    """Get overlay plot specifications from config."""
    overlay_plots = config.get('OverlayPlots', '')
    if overlay_plots:
        return [plot.strip() for plot in overlay_plots.split(',')]
    return []

def create_output_dirs(base_dir, config, jet_dirs=None):
    """Create output directory structure."""
    output_dirs = {'base': base_dir}
    
    os.makedirs(base_dir, exist_ok=True)
    
    # Create subdirectories if specified
    if config.get('CreateSubdirectories', '0') == '1':
        for subdir_key, subdir_name in config.items():
            if subdir_key.startswith('Subdirectory.'):
                subdir_path = os.path.join(base_dir, subdir_name)
                os.makedirs(subdir_path, exist_ok=True)
                output_dirs[subdir_key.replace('Subdirectory.', '')] = subdir_path
    
    return output_dirs

def set_cms_style(config):
    """Set CMS plotting style globally. Always removes stat and title boxes if UseCMSStyle is set."""
    if config.get('UseCMSStyle', '0') == '1':
        ROOT.gROOT.SetStyle("Plain")
        ROOT.gStyle.SetOptStat(0)              # Remove stat box
        ROOT.gStyle.SetOptTitle(0)             # Remove default title
        ROOT.gStyle.SetCanvasColor(0)          # White canvas
        ROOT.gStyle.SetPadColor(0)             # White pad
        ROOT.gStyle.SetFrameBorderMode(0)
        ROOT.gStyle.SetPadTickX(1)
        ROOT.gStyle.SetPadTickY(1)
        ROOT.gStyle.SetTitleBorderSize(0)
        ROOT.gStyle.SetLegendBorderSize(0)
        ROOT.gStyle.SetLabelFont(42, "XYZ")
        ROOT.gStyle.SetTitleFont(42, "XYZ")
        ROOT.gStyle.SetLabelSize(0.05, "XYZ")
        ROOT.gStyle.SetTitleSize(0.06, "XYZ")
        ROOT.gStyle.SetHistLineWidth(2)
        ROOT.gStyle.SetMarkerStyle(20)

        # You can add more CMS style settings here if needed

def setup_plot_style(config):
    """Set up general plot style options (non-global adjustments)."""
    ROOT.gStyle.SetPadTickX(1)
    ROOT.gStyle.SetPadTickY(1)
    ROOT.gStyle.SetFrameLineWidth(2)
    ROOT.gStyle.SetHistLineWidth(2)

def get_selection_text(config, jet_dir="", cent_bin=""):
    """Generate selection text based on config and context."""
    selection_parts = []
    photon_pt_min = config.get('SelectionPhotonPtMin', '')
    photon_pt_max = config.get('SelectionPhotonPtMax', '')
    photon_eta_max = config.get('SelectionPhotonEtaMax', '')
    jet_pt_min = config.get('SelectionJetPtMin', '')
    jet_pt_max = config.get('SelectionJetPtMax', '')
    jet_eta_max = config.get('SelectionJetEtaMax', '')
    dphi_min = config.get('SelectionDeltaPhiMin', '')
    custom_selection = config.get('SelectionText', '')
    
    if custom_selection:
        selection_parts.append(custom_selection)
    else:
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
    
    if jet_dir:
        jet_label = get_jet_label(jet_dir)
        selection_parts.append(jet_label)
    if cent_bin:
        if 'to' in cent_bin:
            parts = cent_bin.replace('cent', '').split('to')
            if len(parts) >= 2 and all(p.isdigit() for p in parts):
                cent_min, cent_max = int(parts[0]) / 2.0, int(parts[1]) / 2.0
                cent_label = f"Centrality: {cent_min:g}-{cent_max:g}%"
                selection_parts.append(cent_label)
            else:
                selection_parts.append(get_centrality_label(cent_bin, config))
        else:
            selection_parts.append(get_centrality_label(cent_bin, config))
    
    return ", ".join(selection_parts)

def make_output_dir(path):
    """Create output directory, using EOS mkdir for /eos/ paths except CERNBOX, else os.makedirs."""
    import subprocess
    if os.path.exists(path):
        return
    if path.startswith('/eos/user/') or path.startswith('/eos/project/'):
        if '/eos/user/' in path and os.environ.get('USER') and f"/eos/user/{os.environ['USER'][0]}/{os.environ['USER']}" in path:
            os.makedirs(path, exist_ok=True)
        else:
            parent = os.path.dirname(path)
            if parent and not os.path.exists(parent):
                make_output_dir(parent)
            try:
                subprocess.check_call(['eos', 'mkdir', '-p', path])
            except Exception as e:
                print(f"[ERROR] Failed to create EOS directory {path}: {e}")
    else:
        os.makedirs(path, exist_ok=True)

def ensure_output_dir_for_file(filepath):
    """Ensure parent directory exists before saving a file."""
    dirpath = os.path.dirname(filepath)
    if dirpath and not os.path.exists(dirpath):
        make_output_dir(dirpath)

def save_canvas(canvas, outdir, name, formats, jet_dir="", cent_bin="", config=None):
    """Save canvas to multiple formats with proper directory creation."""
    verbose = config.get('Verbose', '0') == '1' if config else False
    filename = name
    if jet_dir and cent_bin:
        filename = f"{name}_{jet_dir}_{cent_bin}"
    elif jet_dir:
        filename = f"{name}_{jet_dir}"
    elif cent_bin:
        filename = f"{name}_{cent_bin}"
    
    if not verbose:
        original_error_level = ROOT.gErrorIgnoreLevel
        ROOT.gErrorIgnoreLevel = ROOT.kError
    
    for fmt in formats:
        outpath = os.path.join(outdir, f"{filename}.{fmt}")
        ensure_output_dir_for_file(outpath)
        canvas.SaveAs(outpath)
        if verbose:
            rel_path = os.path.relpath(outpath, start=os.path.dirname(outdir))
            logging.info(f"  Saved: {rel_path}")
    
    if not verbose:
        ROOT.gErrorIgnoreLevel = original_error_level

def normalize_histogram(hist, method='area', verbose=False):
    """Normalize histogram according to specified method."""
    if method.lower() == 'none':
        return 1.0
    
    original_integral = hist.Integral()
    
    if original_integral <= 0:
        if verbose:
            logging.warning(f"Cannot normalize histogram {hist.GetName()}: integral = {original_integral}")
        return 1.0
    
    normalization_factor = 1.0
    
    if method.lower() == 'area':
        # Normalize to unit area (integral = 1)
        normalization_factor = 1.0 / original_integral
        hist.Scale(normalization_factor)
        
    elif method.lower() == 'density':
        # Normalize to unit area and divide by bin width (proper density)
        normalization_factor = 1.0 / original_integral
        hist.Scale(normalization_factor, "width")  # ROOT's "width" option divides by bin width
    
    else:
        if verbose:
            logging.warning(f"Unknown normalization method: {method}. Available methods: 'none', 'area', 'density'")
        return 1.0
    
    if verbose:
        logging.info(f"Normalized {hist.GetName()} using method '{method}': factor = {normalization_factor:.6f}")
    
    return normalization_factor

def normalize_histogram_list(hists, config, verbose=False):
    """Normalize a list of histograms according to configuration."""
    normalize_overlays = config.get('Overlay.Normalize', '1') == '1'  # Default to True
    if not normalize_overlays:
        return [1.0] * len(hists)
    
    method = config.get('Overlay.NormalizationMethod', 'area').lower()
    
    # Apply same normalization method to all histograms
    normalization_factors = []
    for hist in hists:
        factor = normalize_histogram(hist, method, verbose)
        normalization_factors.append(factor)
    
    return normalization_factors

# Enhanced plotting functions
def plot_histogram_1d(hist, hist_config, config, outdir, formats, jet_dir="", cent_bin=""):
    """Plot 1D histogram with enhanced positioning and stat box control."""
    
    canvas_name = f"c_{hist_config.name}"
    width = int(config.get('CanvasWidth', 800))
    height = int(config.get('CanvasHeight', 600))
    
    # Check for jet-specific directory
    if jet_dir and outdir == os.path.join(os.path.dirname(outdir), '1D'):
        jet_specific_dir = os.path.join(outdir, jet_dir)
        if os.path.exists(jet_specific_dir):
            outdir = jet_specific_dir
    
    c = ROOT.TCanvas(canvas_name, canvas_name, width, height)
    c.cd()
    
    # Setup plot style and remove stat boxes
    setup_plot_style(config)
    
    # Set canvas margins from config
    left_margin = float(config.get('CanvasMarginLeft', '0.15'))
    right_margin = float(config.get('CanvasMarginRight', '0.05'))
    top_margin = float(config.get('CanvasMarginTop', '0.08'))
    bottom_margin = float(config.get('CanvasMarginBottom', '0.12'))
    
    c.SetLeftMargin(left_margin)
    c.SetRightMargin(right_margin)
    c.SetTopMargin(top_margin)
    c.SetBottomMargin(bottom_margin)
    
    if hist_config.log_y:
        c.SetLogy()
    
    # Remove stat box from histogram
    hist.SetStats(0)
    
    apply_histogram_style(hist, hist_config, 0, config)
    
    if hist_config.x_min != hist_config.x_max:
        hist.GetXaxis().SetRangeUser(hist_config.x_min, hist_config.x_max)
    
    hist.Draw(hist_config.draw_option)
    
    # Draw CMS label and selection text with smart positioning
    draw_cms_label(c, config, "")
    draw_selection_text_smart(c, config, jet_dir, cent_bin)
    
    # Save using enhanced save_canvas function
    save_canvas(c, outdir, hist_config.name, formats, jet_dir, cent_bin, config)
    c.Close()

def plot_histogram_2d(hist, hist_config, config, outdir, formats, jet_dir="", cent_bin=""):
    """Plot 2D histogram with enhanced positioning and stat box control."""
    
    canvas_name = f"c_{hist_config.name}"
    width = int(config.get('CanvasWidth', 800))
    height = int(config.get('CanvasHeight', 600))
    
    # Check for jet-specific directory
    if jet_dir and outdir == os.path.join(os.path.dirname(outdir), '2D'):
        jet_specific_dir = os.path.join(outdir, jet_dir)
        if os.path.exists(jet_specific_dir):
            outdir = jet_specific_dir
    
    c = ROOT.TCanvas(canvas_name, canvas_name, width, height)
    c.cd()
    
    # Setup plot style and remove stat boxes
    setup_plot_style(config)
    
    # Set canvas margins from config with extra right margin for color palette
    left_margin = float(config.get('CanvasMarginLeft', '0.15'))
    right_margin = max(float(config.get('CanvasMarginRight', '0.05')), 0.15)  # Ensure space for palette
    top_margin = float(config.get('CanvasMarginTop', '0.08'))
    bottom_margin = float(config.get('CanvasMarginBottom', '0.12'))
    
    c.SetLeftMargin(left_margin)
    c.SetRightMargin(right_margin)
    c.SetTopMargin(top_margin)
    c.SetBottomMargin(bottom_margin)
    
    if hist_config.log_z:
        c.SetLogz()
    
    # Set color palette
    if hist_config.color_map == 'viridis':
        ROOT.gStyle.SetPalette(ROOT.kViridis)
    elif hist_config.color_map == 'plasma':
        ROOT.gStyle.SetPalette(ROOT.kPlasma)
    else:
        ROOT.gStyle.SetPalette(ROOT.kBird)
    
    # Remove stat box from histogram
    hist.SetStats(0)
    hist.SetTitle('')
    
    if ';' in hist_config.title:
        parts = hist_config.title.split(';')
        if len(parts) > 1:
            hist.GetXaxis().SetTitle(parts[1])
        if len(parts) > 2:
            hist.GetYaxis().SetTitle(parts[2])
    
    hist.Draw(hist_config.draw_option_2d)
    
    # Draw CMS label and selection text
    draw_cms_label(c, config, "")
    draw_selection_text_smart(c, config, jet_dir, cent_bin)
    
    # Save using enhanced save_canvas function
    save_canvas(c, outdir, hist_config.name, formats, jet_dir, cent_bin, config)
    c.Close()

def plot_overlay_advanced(hist_list, config, output_dir, formats):
    """Create advanced overlay plot (placeholder)."""
    pass

def plot_generic_overlay(hist_list, config, output_dir, formats):
    """Create generic overlay plot (placeholder)."""
    pass

def plot_jet_collection_overlay(hist_source, cent_bin, hist_name, config, output_dir, formats):
    """Create jet collection overlay plot (placeholder)."""
    pass

def plot_centrality_overlay(hist_source, jet_dir, hist_name, config, output_dir, formats):
    """Create centrality overlay plot (placeholder)."""
    pass

# =============================================================================
# MULTI-FILE OVERLAY MANAGEMENT CLASSES
# =============================================================================

class MultiFileHistogramManager:
    """Manager for handling histograms from multiple ROOT files with different configs."""
    
    def __init__(self, input_files):
        """
        Initialize with input files dictionary.
        
        Args:
            input_files: Dict of {label: {'file': file_path, 'config': config_path, 'display': display_name}}
        """
        self.input_files = input_files
        self.hist_sources = {}
        self.configs = {}
        self._initialize_sources()
    
    def _initialize_sources(self):
        """Initialize HistogramSource objects for each input file."""
        for label, file_info in self.input_files.items():
            try:
                if isinstance(file_info, dict):
                    file_path = file_info['file']
                    config_path = file_info.get('config', '')
                    display_name = file_info.get('display', label)
                else:
                    # Simple format: just file path
                    file_path = file_info
                    config_path = ''
                    display_name = label
                
                # Load config for this file
                if config_path and os.path.exists(config_path):
                    file_config = parse_config(config_path)
                else:
                    file_config = {}
                
                # Open ROOT file
                root_file = ROOT.TFile.Open(file_path)
                if root_file and not root_file.IsZombie():
                    self.hist_sources[label] = HistogramSource(root_file, name=display_name, config=file_config)
                    self.configs[label] = file_config
                    logging.info(f"    Loaded {label} from {file_path}")
                else:
                    logging.error(f"    Failed to open {file_path} for {label}")
            except Exception as e:
                logging.error(f"    Error initializing {label}: {e}")
    
    def get_histogram(self, hist_name, jet_dir=None, cent_bin=None, source_label=None):
        """Get histogram from specified source or all sources."""
        if source_label:
            if source_label in self.hist_sources:
                return self.hist_sources[source_label].get_histogram(hist_name, jet_dir, cent_bin)
            return None
        
        # Return dictionary of histograms from all sources
        result = {}
        for label, source in self.hist_sources.items():
            hist = source.get_histogram(hist_name, jet_dir, cent_bin)
            if hist:
                result[label] = hist
        return result
    
    def get_available_jet_dirs(self):
        """Get union of all available jet directories across all sources."""
        all_jets = set()
        for source in self.hist_sources.values():
            all_jets.update(source.get_available_jet_dirs())
        return sorted(all_jets)
    
    def get_available_centrality_bins(self):
        """Get union of all available centrality bins across all sources."""
        all_cents = set()
        for source in self.hist_sources.values():
            all_cents.update(source.get_available_centrality_bins())
        return sorted(all_cents)


class ImplicitOverlayHandler:
    """Handler for creating overlay plots from ImplicitOverlay config specifications."""
    
    def __init__(self, hist_manager=None, config=None):
        """Initialize with histogram manager and config."""
        self.hist_manager = hist_manager
        self.config = config or {}
        self.verbose = config.get('Verbose', '0') == '1' if config else False
    
    def parse_implicit_overlays(self):
        """Parse ImplicitOverlay configurations from config."""
        overlays = []
        
        for key, value in self.config.items():
            if key.startswith('ImplicitOverlay.'):
                overlay_name = key.replace('ImplicitOverlay.', '')
                
                # Parse the overlay specification
                # Format: <histogram_key>:<jet_pattern>:<cent_pattern>
                parts = value.split(':')
                if len(parts) >= 3:
                    hist_key = parts[0].strip()
                    jet_pattern = parts[1].strip()
                    cent_pattern = parts[2].strip()
                    
                    overlays.append({
                        'name': overlay_name,
                        'histogram': hist_key,
                        'jet_pattern': jet_pattern,
                        'cent_pattern': cent_pattern,
                        'specification': value
                    })
                    
                    if self.verbose:
                        logging.debug(f"    Parsed overlay: {overlay_name} -> {value}")
        
        logging.info(f"Found {len(overlays)} implicit overlay specifications")
        return overlays
    
    def create_overlay_plots(self, overlays, output_dir, formats):
        """Create overlay plots based on parsed specifications."""
        
        if not self.hist_manager:
            logging.error("No histogram manager available for overlay creation")
            return
        
        # Get available jet dirs and centrality bins
        available_jets = self.hist_manager.get_available_jet_dirs()
        available_cents = self.hist_manager.get_available_centrality_bins()
        
        for overlay in overlays:
            self._create_single_overlay(overlay, available_jets, available_cents, output_dir, formats)
    
    def _create_single_overlay(self, overlay, available_jets, available_cents, output_dir, formats):
        """Create a single overlay plot with PlotType-based subdirectory organization."""
        
        overlay_name = overlay['name']
        hist_key = overlay['histogram']
        jet_pattern = overlay['jet_pattern']
        cent_pattern = overlay['cent_pattern']
        
        # Expand patterns to actual jet dirs and centrality bins
        matching_jets = self._expand_pattern(jet_pattern, available_jets)
        matching_cents = self._expand_pattern(cent_pattern, available_cents)
        
        if self.verbose:
            logging.debug(f"    Creating overlay {overlay_name}:")
            logging.debug(f"      Jets: {matching_jets}")
            logging.debug(f"      Centralities: {matching_cents}")
        
        # Create overlays for each combination
        for cent_bin in matching_cents:
            for jet_dir in matching_jets:
                
                # Get histograms from all sources
                hist_dict = self.hist_manager.get_histogram(hist_key, jet_dir, cent_bin)
                
                if not hist_dict:
                    if self.verbose:
                        logging.warning(f"      No histograms found for {hist_key} in {jet_dir}/{cent_bin}")
                    continue
                
                # Use PlotType system to determine output path
                resolved_path = resolve_plot_type_path(
                    overlay_name, self.config, jet_dir, cent_bin, hist_key
                )
                
                # Create full output directory path
                full_output_dir = os.path.join(output_dir, resolved_path)
                make_output_dir(full_output_dir)
                
                # Create the overlay plot
                success = self._plot_data_mc_overlay(
                    hist_dict, overlay_name, jet_dir, cent_bin, 
                    full_output_dir, formats
                )
                
                if success and self.verbose:
                    logging.info(f"      Created overlay: {resolved_path}/{overlay_name}_{jet_dir}_{cent_bin}")
    
    def _expand_pattern(self, pattern, available_items):
        """Expand wildcard pattern to matching items."""
        if pattern == '*':
            return available_items
        
        if '*' in pattern:
            import fnmatch
            return [item for item in available_items if fnmatch.fnmatch(item, pattern)]
        
        # Exact match
        if pattern in available_items:
            return [pattern]
        
        return []
    
    def _plot_data_mc_overlay(self, hist_dict, overlay_name, jet_dir, cent_bin, output_dir, formats):
        """Create Data-MC overlay plot."""
        
        try:
            if len(hist_dict) < 2:
                if self.verbose:
                    logging.warning(f"      Need at least 2 histograms for overlay, got {len(hist_dict)}")
                return False
            
            # Get DataMC configuration
            datamc_config = DataMCConfig(self.config)
            
            # Create canvas
            canvas = ROOT.TCanvas(f"c_{overlay_name}_{jet_dir}_{cent_bin}", 
                                 f"{overlay_name} {jet_dir} {cent_bin}",
                                 datamc_config.canvas_width, datamc_config.canvas_height)
            
            # Create pads for main plot and ratio
            canvas.cd()
            pad1 = ROOT.TPad("pad1", "pad1", 0, datamc_config.lower_pad_height, 1, 1)
            pad1.SetBottomMargin(0.02)
            pad1.Draw()
            
            pad2 = ROOT.TPad("pad2", "pad2", 0, 0, 1, datamc_config.lower_pad_height)
            pad2.SetTopMargin(0.02)
            pad2.SetBottomMargin(0.3)
            pad2.Draw()
            
            # Get histograms (assume first is Data, second is MC)
            hist_labels = list(hist_dict.keys())
            data_hist = hist_dict[hist_labels[0]]
            mc_hist = hist_dict[hist_labels[1]] if len(hist_labels) > 1 else None
            
            if not data_hist or not mc_hist:
                logging.warning(f"      Missing histograms for overlay")
                return False
            
            # Style histograms
            data_hist.SetLineColor(datamc_config.data_color)
            data_hist.SetMarkerColor(datamc_config.data_color)
            data_hist.SetMarkerStyle(datamc_config.data_marker_style)
            data_hist.SetMarkerSize(datamc_config.data_marker_size)
            data_hist.SetLineWidth(datamc_config.data_line_width)
            
            mc_hist.SetLineColor(datamc_config.mc_color)
            mc_hist.SetMarkerColor(datamc_config.mc_color)
            mc_hist.SetMarkerStyle(datamc_config.mc_marker_style)
            mc_hist.SetMarkerSize(datamc_config.mc_marker_size)
            mc_hist.SetLineWidth(datamc_config.mc_line_width)
            
            # Normalize if requested
            if self.config.get('DataMC.Normalize', '1') == '1':
                if data_hist.Integral() > 0:
                    data_hist.Scale(1.0 / data_hist.Integral())
                if mc_hist.Integral() > 0:
                    mc_hist.Scale(1.0 / mc_hist.Integral())
            
            # Draw main plot
            pad1.cd()
            if datamc_config.log_y:
                pad1.SetLogy()
            
            # Determine y-axis range
            max_val = max(data_hist.GetMaximum(), mc_hist.GetMaximum())
            data_hist.SetMaximum(max_val * 1.2)
            
            data_hist.Draw("E1")
            mc_hist.Draw("HIST SAME")
            data_hist.Draw("E1 SAME")  # Draw data on top
            
            # Create legend
            legend = ROOT.TLegend(datamc_config.legend_x1, datamc_config.legend_y1,
                                 datamc_config.legend_x2, datamc_config.legend_y2)
            legend.SetBorderSize(0)
            legend.SetFillStyle(0)
            legend.AddEntry(data_hist, datamc_config.data_label, "PE")
            legend.AddEntry(mc_hist, datamc_config.mc_label, "L")
            legend.Draw()
            
            # Draw CMS label and selection text
            draw_cms_label(pad1, self.config)
            draw_selection_text_smart(pad1, self.config, jet_dir, cent_bin)
            
            # Create ratio plot
            pad2.cd()
            ratio_hist = data_hist.Clone(f"ratio_{overlay_name}_{jet_dir}_{cent_bin}")
            ratio_hist.Divide(mc_hist)
            ratio_hist.SetTitle("")
            ratio_hist.GetYaxis().SetTitle(datamc_config.ratio_title)
            ratio_hist.GetYaxis().SetRangeUser(datamc_config.ratio_y_min, datamc_config.ratio_y_max)
            ratio_hist.SetLineColor(datamc_config.ratio_color)
            ratio_hist.SetMarkerColor(datamc_config.ratio_color)
            ratio_hist.SetMarkerStyle(datamc_config.ratio_marker_style)
            ratio_hist.SetMarkerSize(datamc_config.ratio_marker_size)
            ratio_hist.Draw("E1")
            
            # Draw ratio = 1 line
            line = ROOT.TLine(ratio_hist.GetXaxis().GetXmin(), 1.0,
                             ratio_hist.GetXaxis().GetXmax(), 1.0)
            line.SetLineStyle(2)
            line.SetLineColor(ROOT.kBlack)
            line.Draw()
            
            # Save the plot using our enhanced save_canvas function
            base_filename = f"{overlay_name}_{jet_dir}_{cent_bin}_overlay"
            save_canvas(canvas, output_dir, base_filename, formats, jet_dir, cent_bin, self.config)
            
            canvas.Close()
            return True
            
        except Exception as e:
            logging.error(f"      Error creating overlay plot: {e}")
            return False


class ExplicitOverlayHandler:
    """Handler for creating overlay plots from ExplicitOverlay config specifications."""
    
    def __init__(self, hist_manager=None, config=None):
        """Initialize with histogram manager and config."""
        self.hist_manager = hist_manager
        self.config = config or {}
    
    def create_overlay_plots(self, output_dir, formats):
        """Create explicit overlay plots (placeholder for future implementation)."""
        logging.info("Explicit overlay creation not yet implemented")
        pass
