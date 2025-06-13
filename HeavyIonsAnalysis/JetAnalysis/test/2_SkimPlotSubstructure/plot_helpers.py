"""
plot_helpers.py

Helper functions for CMS Heavy Ion Gamma-Jet analysis plotting.
- Config parsing (TEnv-compatible)
- ROOT file navigation and histogram retrieval
- Overlay, normalization, color, legend, and error bar utilities
- Output directory mirroring and error handling
- All functions are non-user-facing and stable
"""

import os
import sys
import re
import math
from typing import Dict, List, Tuple, Optional, Any

# ROOT import with error handling
try:
    import ROOT
    ROOT.gROOT.SetBatch(True)  # Disable graphics by default
    ROOT.gErrorIgnoreLevel = ROOT.kWarning  # Suppress info messages
    HAS_ROOT = True
except ImportError:
    print("ERROR: ROOT is not available. Please ensure ROOT is properly installed and configured.")
    HAS_ROOT = False

# --- Logging System ---

class Logger:
    """Colored logging system with verbosity levels."""
    
    # ANSI color codes
    COLORS = {
        'RED': '\033[91m',
        'GREEN': '\033[92m',
        'YELLOW': '\033[93m',
        'BLUE': '\033[94m',
        'MAGENTA': '\033[95m',
        'CYAN': '\033[96m',
        'WHITE': '\033[97m',
        'BOLD': '\033[1m',
        'RESET': '\033[0m'
    }
    
    # Enhanced Verbosity levels (intuitive numbering: higher = less verbose)
    TRACE = 0      # Very detailed tracing (heavy debugging, positioning details)
    DEBUG = 1      # Development debugging (important debug info)
    INFO = 2       # Normal operation (default level)
    WARNING = 3    # Warnings (potential issues)
    ERROR = 4      # Errors (serious problems)
    
    def __init__(self, verbosity=INFO, use_colors=True):
        self.verbosity = verbosity
        self.use_colors = use_colors and sys.stdout.isatty()  # Only use colors if in terminal
    
    def _colorize(self, text, color):
        """Apply color to text if colors are enabled."""
        if self.use_colors and color in self.COLORS:
            return f"{self.COLORS[color]}{text}{self.COLORS['RESET']}"
        return text
    
    def trace(self, message):
        """Trace level logging (most verbose - detailed positioning, text processing)."""
        if self.verbosity <= self.TRACE:
            print(self._colorize(f"[TRACE] {message}", 'MAGENTA'))
    
    def debug(self, message):
        """Debug level logging (development debugging)."""
        if self.verbosity <= self.DEBUG:
            print(self._colorize(f"[DEBUG] {message}", 'CYAN'))
    
    def info(self, message):
        """Info level logging (normal operation)."""
        if self.verbosity <= self.INFO:
            print(self._colorize(f"[INFO] {message}", 'GREEN'))
    
    def warning(self, message):
        """Warning level logging."""
        if self.verbosity <= self.WARNING:
            print(self._colorize(f"[WARNING] {message}", 'YELLOW'))
    
    def error(self, message):
        """Error level logging."""
        if self.verbosity <= self.ERROR:
            print(self._colorize(f"[ERROR] {message}", 'RED'))
    
    def critical(self, message):
        """Critical level logging (always shown)."""
        print(self._colorize(f"[CRITICAL] {message}", 'BOLD'))
    
    def progress(self, message):
        """Progress messages (always shown, no level prefix)."""
        print(self._colorize(message, 'WHITE'))
    
    def success(self, message):
        """Success messages."""
        if self.verbosity <= self.INFO:
            print(self._colorize(f"[SUCCESS] {message}", 'GREEN'))

# Global logger instance
logger = Logger()

def set_verbosity(level):
    """Set global logging verbosity level."""
    global logger
    logger.verbosity = level

def set_colors(enabled):
    """Enable or disable colored output."""
    global logger
    logger.use_colors = enabled

# =============================================================================
# COLOR SCHEMES
# =============================================================================

def get_color_scheme(scheme_name, color_blind=False):
    """Return color palette based on scheme name.
    
    Args:
        scheme_name (str): Name of color scheme ('petroff6', 'petroff10', 'viridis', 'traditional', 'default')
        color_blind (bool): Use colorblind-friendly palette if True
        
    Returns:
        list: ROOT color codes for the scheme
    """
    if not HAS_ROOT:
        logger.error("ROOT not available, cannot generate color schemes")
        return []
        
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
        'traditional': [
            ROOT.kBlack, ROOT.kBlue, ROOT.kRed, ROOT.kMagenta, 
            ROOT.kGreen+2, ROOT.kOrange, ROOT.kCyan, ROOT.kYellow+2, ROOT.kGray+2
        ],
        'viridis': [
            ROOT.kBlue+2, ROOT.kAzure+7, ROOT.kTeal+2, ROOT.kGreen+2, ROOT.kYellow+1
        ],
        'default': [
            ROOT.kBlack, ROOT.kBlue, ROOT.kRed, ROOT.kMagenta, 
            ROOT.kGreen+2, ROOT.kOrange, ROOT.kCyan, ROOT.kYellow+2, ROOT.kGray+2
        ]
    }
    
    if color_blind:
        # Colorblind-friendly palette based on Paul Tol's schemes
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

# --- Config Parsing ---

def parse_configs(config_paths):
    """Parse TEnv-compatible config files. Returns a dict of config data."""
    config_data = {}
    for path in config_paths:
        if not os.path.exists(path):
            logger.error(f"Config file not found: {path}")
            continue
        with open(path) as f:
            lines = f.readlines()
        for line in lines:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            if ':' in line:
                key, val = line.split(':', 1)
                config_data[key.strip()] = val.strip()
    return config_data

# --- ROOT File Utilities ---

def open_root_files(file_paths):
    """Open ROOT files and return a list of TFile objects."""
    files = []
    for path in file_paths:
        f = ROOT.TFile.Open(path)
        if not f or f.IsZombie():
            logger.error(f"Could not open ROOT file: {path}")
        else:
            files.append(f)
    return files


def list_root_histograms(root_file, path=''):
    """Recursively list histograms in a ROOT file (for debug)."""
    dir_obj = root_file.Get(path) if path else root_file
    if not dir_obj:
        return
    keys = dir_obj.GetListOfKeys()
    for key in keys:
        obj = key.ReadObj()
        name = obj.GetName()
        if obj.InheritsFrom('TDirectory'):
            list_root_histograms(root_file, os.path.join(path, name))
        elif obj.InheritsFrom('TH1') or obj.InheritsFrom('TH2') or obj.InheritsFrom('TProfile'):
            logger.trace(f"    {os.path.join(path, name)} [{obj.ClassName()}]")

def get_histogram_recursive(root_file, hname, path=''):
    """Recursively search for a histogram by name in a ROOT file."""
    dir_obj = root_file.Get(path) if path else root_file
    if not dir_obj:
        return None
    keys = dir_obj.GetListOfKeys()
    for key in keys:
        obj = key.ReadObj()
        name = obj.GetName()
        if obj.InheritsFrom('TDirectory'):
            found = get_histogram_recursive(root_file, hname, os.path.join(path, name))
            if found:
                return found
        elif (obj.InheritsFrom('TH1') or obj.InheritsFrom('TH2') or obj.InheritsFrom('TProfile')) and name == hname:
            return obj
    return None

def get_histogram_recursive(root_file, path):
    """
    Get histogram from ROOT file by path.
    Args:
        root_file: ROOT.TFile object
        path: histogram path like "cent0to60/AK2Z2/hJetPt"
    Returns:
        ROOT histogram object or None
    """
    try:
        obj = root_file.Get(path)
        if obj and obj.InheritsFrom("TH1"):
            return obj
    except:
        pass
    return None

# --- Placeholders for future helpers (overlay, normalization, etc.) ---

# Functions will be implemented in future phases

def create_legend(config, entries, overlay=False, histograms=None):
    """
    Create and configure a ROOT.TLegend using config and add entries.
    Args:
        config (dict): Config dictionary.
        entries (list): List of (obj, label, style) tuples.
        overlay (bool): If True, use overlay legend keys.
        histograms (list): Optional list of histograms for auto-positioning
    Returns:
        ROOT.TLegend
    """
    if overlay:
        prefix = "Overlay."
    else:
        prefix = "Legend"
    
    # Get legend position - either from config or auto-placement
    if histograms and bool(int(config.get("Legend.AutoPosition", "0"))):
        legend_pos = find_best_legend_position(histograms, config)
    else:
        legend_pos_str = config.get(f"{prefix}LegendPosition", "0.65,0.65,0.9,0.9")
        legend_pos = [float(x) for x in legend_pos_str.split(",")]
    
    # Configure legend appearance
    legend_text_size = float(config.get(f"{prefix}LegendTextSize", 0.03))
    legend_fill_style = int(config.get(f"{prefix}LegendFillStyle", 0))
    legend_border_size = int(config.get(f"{prefix}LegendBorderSize", 0))
    legend_columns = int(config.get(f"{prefix}LegendColumns", 1))
    
    # Create and configure the legend
    legend = ROOT.TLegend(*legend_pos)
    legend.SetTextSize(legend_text_size)
    legend.SetFillStyle(legend_fill_style)
    legend.SetBorderSize(legend_border_size)
    
    # Set number of columns if > 1
    if legend_columns > 1:
        legend.SetNColumns(legend_columns)
    
    # Add all entries
    for obj, label, style in entries:
        legend.AddEntry(obj, label, style)
    
    return legend

def apply_cms_label(canvas, config):
    """
    Apply CMS label and lumi/energy/extra text to the canvas using config.
    Args:
        canvas: ROOT.TCanvas
        config (dict): Config dictionary.
    """
    cms_label = config.get("CMSLabel", "Preliminary")
    cms_energy = config.get("CMSEnergyText", "")
    cms_lumi = config.get("CMSLuminosity", "")
    cms_extra = config.get("CMSExtraText", "")
    
    # Set up CMS_lumi global variables
    if hasattr(ROOT, 'CMS_lumi'):
        if hasattr(ROOT, 'writeExtraText'):
            ROOT.writeExtraText = True
        if hasattr(ROOT, 'extraText'):
            ROOT.extraText = cms_label
        
        # Construct the luminosity text combining energy and luminosity
        lumi_text = ""
        if cms_lumi and cms_energy:
            lumi_text = f"{cms_lumi} ({cms_energy})"
        elif cms_energy:
            lumi_text = cms_energy
        elif cms_lumi:
            lumi_text = cms_lumi
            
        # Set the luminosity text in ROOT globals
        if hasattr(ROOT, 'lumi_sqrtS'):
            ROOT.lumi_sqrtS = lumi_text
            
        # Call CMS_lumi with appropriate parameters
        # iPeriod=0 uses lumi_sqrtS, iPosX=0 for out-of-frame positioning
        ROOT.CMS_lumi(canvas, 0, 0)

def normalize_histogram(hist, method="area"):
    """
    Normalize a ROOT histogram in-place.
    Args:
        hist: ROOT.TH1 or TH2 object
        method: 'area', 'density', or 'none'
    """
    if not hist or method == "none":
        return
    
    # Get integral based on histogram type
    if hist.InheritsFrom("TH2"):
        # For 2D histograms, use simple Integral()
        integral = hist.Integral()
    else:
        # For 1D histograms, include overflow/underflow bins
        integral = hist.Integral(0, hist.GetNbinsX()+1)
    
    if integral == 0:
        return
    
    if method == "area":
        hist.Scale(1.0/integral)
    elif method == "density":
        # Normalize to area and divide by bin width (for 1D only)
        hist.Scale(1.0/integral)
        if hist.InheritsFrom("TH1") and not hist.InheritsFrom("TH2"):
            for i in range(1, hist.GetNbinsX()+1):
                width = hist.GetBinWidth(i)
                if width > 0:
                    hist.SetBinContent(i, hist.GetBinContent(i)/width)
                    hist.SetBinError(i, hist.GetBinError(i)/width)
    # For TH2, area normalization only

def get_root_file_structure(root_file):
    """
    Scan ROOT file and return its directory structure.
    Returns: dict with structure like {"cent0to60": ["General", "AK2Z1", "AK2Z2"], ...}
    """
    structure = {}
    
    def scan_directory(dir_obj, current_path=""):
        keys = dir_obj.GetListOfKeys()
        for key in keys:
            obj = key.ReadObj()
            name = obj.GetName()
            full_path = os.path.join(current_path, name) if current_path else name
            
            if obj.InheritsFrom('TDirectory'):
                # This is a directory - recurse
                if current_path == "":  # Top level (centrality)
                    structure[name] = []
                elif current_path in structure:  # Second level (jet dirs)
                    structure[current_path].append(name)
                scan_directory(obj, full_path)
    
    scan_directory(root_file)
    return structure

def mirror_root_structure(relative_path, base_outdir):
    """
    Create output directory mirroring ROOT structure.
    Args:
        relative_path: path like "Data/cent0to60/AK2Z2"
        base_outdir: base output directory
    Returns:
        full output path
    """
    full_path = os.path.join(base_outdir, relative_path)
    if not os.path.exists(full_path):
        os.makedirs(full_path, exist_ok=True)
        logger.info(f"Created output subdirectory: {relative_path}")
    return full_path

def save_canvas_in_formats(canvas, filepath_base, formats):
    """
    Save canvas in multiple formats.
    Args:
        canvas: ROOT.TCanvas
        filepath_base: file path without extension
        formats: list of formats like ['png', 'root', 'pdf']
    """
    for fmt in formats:
        canvas.SaveAs(f"{filepath_base}.{fmt}")

def detect_mode(config_data):
    """
    Detect if we're in single-file or multi-file mode.
    Returns: "single" or "multi"
    """
    # Check for InputFile.* entries (multi-file mode)
    for k in config_data.keys():
        if k.startswith("InputFile."):
            return "multi"
    return "single"

def generate_smart_overlay_name(jet_collections, centrality_bins, histogram_name, overlay_type):
    """
    Generate smart overlay names based on collections.
    
    Args:
        jet_collections: List of jet collections in overlay
        centrality_bins: List of centrality bins in overlay  
        histogram_name: Base histogram name
        overlay_type: "centrality" or "jetcollection"
    
    Returns:
        Smart overlay filename
    """
    if overlay_type == "centrality":
        # Single jet collection, multiple centralities
        jet_name = jet_collections[0] if jet_collections else "AllJets"
        cent_names = "_".join([c.replace("cent", "").replace("to", "t") for c in centrality_bins])
        return f"{histogram_name}_{jet_name}_{cent_names}_overlay"
    
    elif overlay_type == "jetcollection":
        # Single centrality, multiple jet collections
        cent_name = centrality_bins[0].replace("cent", "") if centrality_bins else "AllCent"
        
        # Smart jet naming: find common patterns
        if not jet_collections:
            jet_name = "AllJets"
        elif len(jet_collections) <= 3:
            # Extract numbers and combine: AK2Z1, AK2Z2, AK2Z3 -> AK2Z123
            try:
                # Check if they follow a pattern like AK2Z1, AK2Z2, AK2Z3
                base = jet_collections[0][:-1]  # Remove last char
                if all(jc.startswith(base) for jc in jet_collections):
                    numbers = "".join([jc[-1] for jc in jet_collections])
                    jet_name = base + numbers
                else:
                    jet_name = "_".join(jet_collections)
            except:
                jet_name = "_".join(jet_collections)
        else:
            jet_name = "_".join(jet_collections)
        
        return f"{histogram_name}_{cent_name}_{jet_name}_overlay"
    
    return f"{histogram_name}_overlay"

def create_single_file_overlays(config_data, root_file, file_cfg, outdir, plot_formats, display_label, test_mode=False, remaining_plots=None):
    """
    Create single-file overlays: centrality overlays within jet collections,
    and jet collection overlays within centralities.
    
    Returns:
        Number of overlay plots created
    """
    
    # Check if overlays are enabled
    overlay_by_centrality = bool(int(config_data.get("OverlayByCentrality", 0)))
    overlay_by_jetcollection = bool(int(config_data.get("OverlayByJetCollection", 0)))
    
    logger.debug(f"Single-file overlay settings: centrality={overlay_by_centrality}, jetcollection={overlay_by_jetcollection}")
    
    if not overlay_by_centrality and not overlay_by_jetcollection:
        logger.debug("Single-file overlays disabled in config")
        return 0
    
    # Get ROOT file structure
    structure = get_root_file_structure(root_file)
    logger.debug(f"ROOT file structure: {structure}")
    
    # Get available jet collections and centralities from config/structure
    jet_collections = []
    centrality_bins = list(structure.keys())
    
    # Get overlay-specific jet collections from config
    overlay_jet_collections_str = config_data.get("OverlayJetCollections", "").strip()
    if overlay_jet_collections_str and overlay_jet_collections_str.lower() != "all":
        jet_collections = [x.strip() for x in overlay_jet_collections_str.split(',') if x.strip()]
        logger.debug(f"Using overlay-specific jet collections: {jet_collections}")
    elif "JetDirectories" in file_cfg:
        jet_collections = [x.strip() for x in file_cfg["JetDirectories"].split(',') if x.strip()]
    else:
        # Use what's found in structure
        for subdirs in structure.values():
            jet_collections.extend(subdirs)
        jet_collections = list(set(jet_collections))  # Remove duplicates
    
    # Get histogram list from config
    hist_list = []
    for key in ["GeneralHistograms", "JetHistograms"]:
        if key in file_cfg:
            hist_list.extend([x.strip() for x in file_cfg[key].split(',') if x.strip()])
    
    if not hist_list:
        logger.warning("No histograms found for single-file overlays")
        return 0
    
    # Create centrality overlays (same histogram across different jet collections within centrality)
    overlay_count = 0
    if overlay_by_centrality:
        for cent_bin in centrality_bins:
            available_jets = structure.get(cent_bin, [])
            # Filter available jets to only include those specified in OverlayJetCollections
            filtered_jets = [jet for jet in available_jets if jet in jet_collections]
            
            if len(filtered_jets) < 2:
                continue
                
            for hist_name in hist_list:
                # Check test mode limits
                if test_mode and remaining_plots is not None and overlay_count >= remaining_plots:
                    logger.debug(f"Test mode: reached overlay limit ({remaining_plots}), stopping centrality overlays")
                    return overlay_count
                
                # Resolve histogram ROOT name
                config_hist_key = f"Histogram.{hist_name}.Name"
                root_hist_name = file_cfg.get(config_hist_key, hist_name)
                
                # Collect histograms from different jet collections
                hists = []
                hist_labels = []
                
                for jet_dir in filtered_jets:
                    hist_path = f"{cent_bin}/{jet_dir}/{root_hist_name}"
                    hist = get_histogram_recursive(root_file, hist_path)
                    if hist:
                        # Skip 2D histograms
                        if hist.InheritsFrom("TH2"):
                            logger.debug(f"Skipping 2D histogram for overlay: {hist_path}")
                            continue
                        # Make a copy for overlay to preserve original
                        hist_copy = hist.Clone(f"{hist.GetName()}_overlay_copy_{len(hists)}")
                        hists.append(hist_copy)
                        hist_labels.append(jet_dir)
                
                if len(hists) >= 2:
                    # Create overlay
                    overlay_name = generate_smart_overlay_name(filtered_jets, [cent_bin], hist_name, "jetcollection")
                    hist_names = [hist_name] * len(hists)  # Same histogram name for all in jet collection overlay
                    create_overlay_plot(hists, hist_labels, overlay_name, config_data, 
                                      os.path.join(outdir, display_label, "overlays", cent_bin), plot_formats, "single-file", hist_names)
                    overlay_count += 1
    
    # Create jet collection overlays (same histogram across different centralities within jet collection)
    if overlay_by_jetcollection:
        for jet_dir in jet_collections:
            available_cents = [cent for cent in centrality_bins if jet_dir in structure.get(cent, [])]
            if len(available_cents) < 2:
                continue
                
            for hist_name in hist_list:
                # Check test mode limits
                if test_mode and remaining_plots is not None and overlay_count >= remaining_plots:
                    logger.debug(f"Test mode: reached overlay limit ({remaining_plots}), stopping jet collection overlays")
                    return overlay_count
                
                # Resolve histogram ROOT name
                config_hist_key = f"Histogram.{hist_name}.Name"
                root_hist_name = file_cfg.get(config_hist_key, hist_name)
                
                # Collect histograms from different centralities
                hists = []
                hist_labels = []
                
                for cent_bin in available_cents:
                    hist_path = f"{cent_bin}/{jet_dir}/{root_hist_name}"
                    hist = get_histogram_recursive(root_file, hist_path)
                    if hist:
                        # Skip 2D histograms
                        if hist.InheritsFrom("TH2"):
                            logger.debug(f"Skipping 2D histogram for overlay: {hist_path}")
                            continue
                        # Make a copy for overlay to preserve original
                        hist_copy = hist.Clone(f"{hist.GetName()}_overlay_copy_{len(hists)}")
                        hists.append(hist_copy)
                        # Convert cent_bin to readable format
                        cent_label = cent_bin.replace("cent", "").replace("to", "-") + "%"
                        hist_labels.append(cent_label)
                
                if len(hists) >= 2:
                    # Create overlay
                    overlay_name = generate_smart_overlay_name([jet_dir], available_cents, hist_name, "centrality")
                    hist_names = [hist_name] * len(hists)  # Same histogram name for all in centrality overlay
                    create_overlay_plot(hists, hist_labels, overlay_name, config_data, 
                                      os.path.join(outdir, display_label, "overlays", jet_dir), plot_formats, "single-file", hist_names)
                    overlay_count += 1
    
    # Return total number of overlay plots created
    return overlay_count

def create_overlay_plot(hists, hist_labels, overlay_name, config_data, out_subdir, plot_formats, overlay_type="overlay", hist_names=None):
    """
    Create an overlay plot with given histograms and labels.
    Supports optional ratio plots underneath the main plot.
    
    Args:
        hists: List of histogram objects
        hist_labels: List of labels for histograms
        overlay_name: Name for the overlay plot
        config_data: Configuration dictionary
        out_subdir: Output subdirectory
        plot_formats: List of output formats
        overlay_type: Type of overlay ("overlay", "single-file", "multi-file")
        hist_names: List of histogram names (for individual normalization settings)
    """
    
    # Suppress ROOT Info messages
    ROOT.gROOT.SetBatch(True)
    old_level = ROOT.gErrorIgnoreLevel
    ROOT.gErrorIgnoreLevel = ROOT.kWarning
    
    try:
        # Check if ratio plots are enabled
        create_ratio = bool(int(config_data.get("CreateRatioPlots", "0")))
        
        # Canvas size - increase height if ratio plot is enabled
        canvas_width = int(config_data.get("Canvas.Width", "800"))
        canvas_height = int(config_data.get("Canvas.Height", "600"))
        if create_ratio:
            canvas_height = int(canvas_height * 1.25)  # Make canvas taller for ratio
        
        # Create canvas
        c = ROOT.TCanvas(f"c_{overlay_name}", overlay_name, canvas_width, canvas_height)
        
        # Apply canvas settings from config
        apply_canvas_settings(c, config_data, is_ratio=create_ratio)
        
        # Get color scheme from config
        color_scheme = config_data.get("ColorScheme", "default")
        use_color_blind = bool(int(config_data.get("UseColorBlind", "0")))
        colors = get_color_scheme(color_scheme, use_color_blind)
        
        # Standard marker styles
        markers = [ROOT.kFullCircle, ROOT.kFullSquare, ROOT.kFullTriangleUp, ROOT.kFullTriangleDown, 
                  ROOT.kFullDiamond, ROOT.kFullStar]
        
        # Apply normalization if enabled
        overlay_norm_enable = bool(int(config_data.get("Overlay.Normalize", "1")))
        overlay_norm_method = config_data.get("Overlay.NormalizationMethod", "area")
        
        # For ratio plots, create pads
        if create_ratio and len(hists) >= 2:
            # Get pad height ratios from config
            upper_pad_height = float(config_data.get("Ratio.UpperPadHeight", "0.7"))
            lower_pad_height = float(config_data.get("Ratio.LowerPadHeight", "0.3"))
            
            # Normalize to ensure they sum to 1.0
            total_height = upper_pad_height + lower_pad_height
            upper_pad_height = upper_pad_height / total_height
            lower_pad_height = lower_pad_height / total_height
            
            # Calculate text scaling factors for consistent visual appearance
            # In ROOT: smaller pads need LARGER text sizes to appear same visual size
            reference_height = 1.0  # Single pad reference
            upper_text_scale = reference_height / upper_pad_height  # 1.0/0.7 = 1.43
            lower_text_scale = reference_height / lower_pad_height  # 1.0/0.3 = 3.33
            
            # Create pads: upper for histograms, lower for ratio
            pad1 = ROOT.TPad("pad1", "pad1", 0, lower_pad_height, 1, 1.0)
            
            # Apply margins with consistent left/right margins between pads
            left_margin = float(config_data.get("Canvas.LeftMargin", "0.15"))
            right_margin = float(config_data.get("Canvas.RightMargin", "0.05"))
            
            # Adjust margins for ratio plots - need more top margin for CMS labels
            pad1.SetBottomMargin(0.02)  # Small gap between pads
            pad1.SetTopMargin(0.12)     # Larger space for CMS labels in ratio plots
            pad1.SetLeftMargin(left_margin)
            pad1.SetRightMargin(right_margin)
            pad1.Draw()
            
            pad2 = ROOT.TPad("pad2", "pad2", 0, 0.0, 1, lower_pad_height)
            pad2.SetTopMargin(0.02)     # Small gap between pads
            pad2.SetBottomMargin(0.45)  # Much larger bottom margin for scaled X-axis labels
            pad2.SetLeftMargin(left_margin)   # Match upper pad
            pad2.SetRightMargin(right_margin) # Match upper pad
            pad2.Draw()
            
            # Use upper pad for histograms
            pad1.cd()
        
        # Apply log scale if configured
        use_logx = bool(int(config_data.get("LogX", "0")))
        use_logy = bool(int(config_data.get("LogY", "0")))
        if create_ratio:
            pad1.SetLogx(use_logx)
            pad1.SetLogy(use_logy)
        else:
            c.SetLogx(use_logx)
            c.SetLogy(use_logy)
            
        # Draw grid if configured
        draw_grid = bool(int(config_data.get("DrawGrid", "0")))
        if create_ratio:
            pad1.SetGrid(draw_grid, draw_grid)
        else:
            c.SetGrid(draw_grid, draw_grid)
        
        # Process histograms
        legend_entries = []
        reference_hist_index = 0  # Default: first histogram is reference
        
        # Calculate global Y-axis range to show all data points
        global_ymin = float('inf')
        global_ymax = float('-inf')
        
        # Apply normalization first if enabled (affects Y range)
        overlay_norm_enable = bool(int(config_data.get("Overlay.Normalize", "1")))
        overlay_norm_method = config_data.get("Overlay.NormalizationMethod", "area")
        
        for i, hist in enumerate(hists):
            # Check individual histogram normalization setting
            hist_name = hist_names[i] if hist_names and i < len(hist_names) else None
            individual_normalize = None
            
            if hist_name:
                # Check for individual histogram normalization setting
                individual_normalize = config_data.get(f"Histogram.{hist_name}.Normalize")
                if individual_normalize is not None:
                    individual_normalize = bool(int(individual_normalize))
            
            # Use individual setting if available, otherwise use overlay setting
            should_normalize = individual_normalize if individual_normalize is not None else overlay_norm_enable
            
            if should_normalize:
                normalize_histogram(hist, overlay_norm_method)
                logger.debug(f"Normalized histogram {hist_name or i} using {overlay_norm_method} method")
            elif individual_normalize is False:
                logger.debug(f"Skipped normalization for histogram {hist_name or i} (individual setting)")
            elif not overlay_norm_enable:
                logger.debug(f"Skipped normalization for histogram {hist_name or i} (overlay setting)")
        
        # Now calculate global range after normalization
        for hist in hists:
            for bin_i in range(1, hist.GetNbinsX() + 1):
                bin_content = hist.GetBinContent(bin_i)
                bin_error = hist.GetBinError(bin_i)
                if bin_content > 0:  # Only consider non-zero bins
                    global_ymin = min(global_ymin, max(0, bin_content - bin_error))
                    global_ymax = max(global_ymax, bin_content + bin_error)
        
        # Add some margin to the range (10% padding)
        if global_ymin != float('inf') and global_ymax != float('-inf'):
            y_range = global_ymax - global_ymin
            global_ymin = max(0, global_ymin - 0.1 * y_range)
            global_ymax = global_ymax + 0.1 * y_range
        
        # Determine which histogram to use as ratio denominator
        ratio_divide_by = config_data.get("Ratio.DivideBy", "first").lower()
        if ratio_divide_by == "first":
            reference_hist_index = 0
        elif ratio_divide_by == "mc" or ratio_divide_by == "simulation":
            # Look for MC/simulation in labels
            for i, label in enumerate(hist_labels):
                if "mc" in label.lower() or "pythia" in label.lower() or "simulation" in label.lower():
                    reference_hist_index = i
                    break
        elif ratio_divide_by == "data":
            # Look for data in labels
            for i, label in enumerate(hist_labels):
                if "data" in label.lower():
                    reference_hist_index = i
                    break
        elif ratio_divide_by.isdigit():
            # Try to use as index
            idx = int(ratio_divide_by)
            if 0 <= idx < len(hists):
                reference_hist_index = idx
        
        # Draw histograms
        for i, hist in enumerate(hists):
            # Note: normalization already applied above
            
            # Set line properties
            hist.SetLineWidth(2)
            hist.SetLineColor(colors[i % len(colors)])
            
            # Set marker properties
            hist.SetMarkerColor(colors[i % len(colors)])
            hist.SetMarkerStyle(markers[i % len(markers)])
            hist.SetMarkerSize(0.8)
            
            hist.SetStats(0)
            
            # Apply global Y-axis range or custom ranges if provided
            if i == 0:  # Apply to first histogram only
                if "XMin" in config_data and "XMax" in config_data:
                    xmin = float(config_data.get("XMin"))
                    xmax = float(config_data.get("XMax"))
                    hist.GetXaxis().SetRangeUser(xmin, xmax)
                
                # Use custom Y range if specified, otherwise use calculated global range
                if "YMin" in config_data and "YMax" in config_data:
                    ymin = float(config_data.get("YMin"))
                    ymax = float(config_data.get("YMax"))
                    hist.GetYaxis().SetRangeUser(ymin, ymax)
                elif global_ymin != float('inf') and global_ymax != float('-inf'):
                    hist.GetYaxis().SetRangeUser(global_ymin, global_ymax)
            
            drawopt = "E1P" if hist.InheritsFrom("TH1") else "COLZ"
            if i == 0:
                hist.Draw(drawopt)
                
                # Apply standardized text sizing
                if create_ratio:
                    # Use standardized function for upper pad
                    standardize_text_sizes(hist, upper_pad_height, config_data)
                    
                    # Hide X-axis labels in upper pad
                    hist.GetXaxis().SetLabelSize(0)
                    hist.GetXaxis().SetTitleSize(0)
                else:
                    # Single plot mode - use full canvas height
                    standardize_text_sizes(hist, 1.0, config_data)
            else:
                hist.Draw(drawopt + " SAME")
            
            # Use "lp" for legend to show both line and markers
            legend_entries.append((hist, hist_labels[i], "lp"))
        
        # Create legend and labels
        legend = create_legend(config_data, legend_entries, overlay=True, histograms=hists)
        legend.Draw()
        
        # Apply CMS label to the main pad/canvas
        if create_ratio:
            pad1.cd()
        apply_cms_label(c, config_data)
        
        # Note: Selection text will be added after all pad operations are complete
        
        # Create ratio plots if enabled
        if create_ratio and len(hists) >= 2:
            pad2.cd()
            pad2.SetGridy(True)  # Always show horizontal grid in ratio
            
            ratio_histograms = []
            
            # Create ratio histograms
            ref_hist = hists[reference_hist_index]
            
            for i, hist in enumerate(hists):
                if i == reference_hist_index:
                    # Skip reference histogram (would be ratio=1.0 everywhere)
                    continue
                
                # Create ratio histogram
                ratio = create_ratio_histogram(hist, ref_hist, config_data)
                
                # Apply standardized text sizing for lower pad
                standardize_text_sizes(ratio, lower_pad_height, config_data)
                
                # Reduce number of Y-axis divisions for cleaner ratio plot
                ratio.GetYaxis().SetNdivisions(505)  # 5 primary, 0 secondary, 3 tertiary - fewer labels
                
                # Use same colors as main histogram
                ratio.SetLineColor(hist.GetLineColor())
                ratio.SetMarkerColor(hist.GetMarkerColor())
                ratio.SetMarkerStyle(hist.GetMarkerStyle())
                
                ratio_histograms.append(ratio)
            
            # Set Y-axis range for all ratio histograms based on global min/max
            if ratio_histograms:
                auto_range = bool(int(config_data.get("Ratio.AutoRange", "0")))
                logger.debug(f"Ratio auto range setting: {auto_range} (from config: {config_data.get('Ratio.AutoRange', 'not set')})")
                
                if auto_range:
                    # Find global min/max across all ratio histograms
                    global_min = float('inf')
                    global_max = float('-inf')
                    
                    for ratio in ratio_histograms:
                        for i in range(1, ratio.GetNbinsX() + 1):
                            content = ratio.GetBinContent(i)
                            error = ratio.GetBinError(i)
                            
                            # Include all points that have either content or error
                            if content != 0 or error > 0:
                                val_min = content - error
                                val_max = content + error
                                global_min = min(global_min, val_min)
                                global_max = max(global_max, val_max)
                    
                    if global_min != float('inf') and global_max != float('-inf'):
                        # Add 15% padding to range for better visibility
                        range_span = global_max - global_min
                        range_padding = max(range_span * 0.15, 0.1)  # At least 0.1 padding
                        
                        auto_min = global_min - range_padding
                        auto_max = global_max + range_padding
                        
                        # Ensure reasonable bounds for ratio plots
                        if auto_min < 0 and global_min >= 0:
                            auto_min = 0.0  # Don't go below 0 if all data is positive
                        
                        logger.debug(f"Global auto Y-axis range for {len(ratio_histograms)} ratio histograms: [{auto_min:.3f}, {auto_max:.3f}] (data range: [{global_min:.3f}, {global_max:.3f}])")
                        
                        # Apply range to all ratio histograms
                        for ratio in ratio_histograms:
                            ratio.GetYaxis().SetRangeUser(auto_min, auto_max)
                    else:
                        # Fallback to config values if no valid data points
                        ratio_min = float(config_data.get("Ratio.Min", "0.5"))
                        ratio_max = float(config_data.get("Ratio.Max", "1.5"))
                        for ratio in ratio_histograms:
                            ratio.GetYaxis().SetRangeUser(ratio_min, ratio_max)
                else:
                    # Use fixed range from config for all ratio histograms
                    ratio_min = float(config_data.get("Ratio.Min", "0.5"))
                    ratio_max = float(config_data.get("Ratio.Max", "1.5"))
                    for ratio in ratio_histograms:
                        ratio.GetYaxis().SetRangeUser(ratio_min, ratio_max)
            
            # Draw ratio histograms
            for i, ratio in enumerate(ratio_histograms):
                if i == 0:
                    ratio.Draw("E1P")
                else:
                    ratio.Draw("E1P SAME")
            
            # Draw horizontal line at 1.0
            if ratio_histograms:
                xmin = ratio_histograms[0].GetXaxis().GetXmin()
                xmax = ratio_histograms[0].GetXaxis().GetXmax()
                
                line = ROOT.TLine(xmin, 1.0, xmax, 1.0)
                line.SetLineColor(ROOT.kBlack)
                line.SetLineWidth(1)
                line.SetLineStyle(2)  # Dashed
                line.Draw("SAME")
                
                # Set X-axis range same as main plot if configured
                if "XMin" in config_data and "XMax" in config_data:
                    xmin = float(config_data.get("XMin"))
                    xmax = float(config_data.get("XMax"))
                    for ratio in ratio_histograms:
                        ratio.GetXaxis().SetRangeUser(xmin, xmax)
            
            # Switch back to upper pad for CMS labels and selection text
            pad1.cd()
        
        # Create output directory
        if not os.path.exists(out_subdir):
            os.makedirs(out_subdir, exist_ok=True)
            logger.info(f"Created output subdirectory: {os.path.basename(out_subdir)}")
        
        # Add selection text as the final step (after all pad operations)
        # This ensures it doesn't get overwritten by subsequent canvas operations
        if create_ratio:
            pad1.cd()  # Switch to upper pad for selection text
            add_selection_text(pad1, config_data, hists)  # Pass histograms for content analysis
        else:
            c.cd()  # Ensure we're on the main canvas
            add_selection_text(c, config_data, hists)  # Pass histograms for content analysis
        
        # Save plot
        c.cd()  # Return to canvas for saving
        for ext in plot_formats:
            c.SaveAs(os.path.join(out_subdir, f"{overlay_name}.{ext}"))
        
        logger.info(f"{overlay_type.title()} overlay{' with ratio' if create_ratio else ''} plotted and saved: {overlay_name}")
        c.Close()
    
    finally:
        # Restore ROOT error level
        ROOT.gErrorIgnoreLevel = old_level

def create_multi_file_overlays(config_data, root_files, file_cfgs, outdir, plot_formats, test_mode=False, max_plots=None):
    """
    Create multi-file overlays by comparing histograms at identical ROOT paths across files.
    
    Returns:
        Number of overlay plots created
    """
    
    if len(root_files) < 2:
        logger.info("Multi-file overlays require at least 2 files")
        return 0
    
    # Get histogram lists from configs
    hist_lists = []
    for file_cfg in file_cfgs:
        hist_list = []
        for key in ["GeneralHistograms", "JetHistograms"]:
            if key in file_cfg:
                hist_list.extend([x.strip() for x in file_cfg[key].split(',') if x.strip()])
        hist_lists.append(hist_list)
    
    # Find common histograms
    common_hists = set(hist_lists[0]) if hist_lists else set()
    for hist_list in hist_lists[1:]:
        common_hists = common_hists.intersection(set(hist_list))
    
    if not common_hists:
        logger.warning("No common histograms found for multi-file overlays")
        return 0
    
    # Get file structure from first file
    structure = get_root_file_structure(root_files[0])
    
    # Verify all files have similar structure
    for i, rf in enumerate(root_files[1:], 1):
        other_structure = get_root_file_structure(rf)
        if structure != other_structure:
            logger.warning(f"File {i+1} has different structure than file 1")
            logger.warning(f"  File 1: {structure}")
            logger.warning(f"  File {i+1}: {other_structure}")
    
    # Get display labels
    display_labels = []
    for i, file_cfg in enumerate(file_cfgs):
        label = file_cfg.get("CMSExtraText", f"File{i+1}")
        if label == "Simulation":
            label = "PYTHIA8"
        elif not label:
            label = "Data"
        display_labels.append(label)
    
    # Create overlays for each path and histogram
    overlay_count = 0
    for cent_bin, jet_dirs in structure.items():
        for jet_dir in jet_dirs:
            for hist_name in common_hists:
                # Check test mode limits
                if test_mode and max_plots is not None and overlay_count >= max_plots:
                    logger.debug(f"Test mode: reached {max_plots} multi-file overlay limit, stopping")
                    return overlay_count
                
                # Resolve histogram ROOT names for each file
                root_hist_names = []
                for file_cfg in file_cfgs:
                    config_hist_key = f"Histogram.{hist_name}.Name"
                    root_hist_name = file_cfg.get(config_hist_key, hist_name)
                    root_hist_names.append(root_hist_name)
                
                # Collect histograms from all files
                hists = []
                hist_labels = []
                
                for i, (rf, root_hist_name) in enumerate(zip(root_files, root_hist_names)):
                    if jet_dir == "General":
                        hist_path = f"{cent_bin}/{jet_dir}/{root_hist_name}"
                    else:
                        hist_path = f"{cent_bin}/{jet_dir}/{root_hist_name}"
                    
                    hist = get_histogram_recursive(rf, hist_path)
                    if hist:
                        # Skip 2D histograms
                        if hist.InheritsFrom("TH2"):
                            logger.debug(f"Skipping 2D histogram for multi-file overlay: {hist_path}")
                            continue
                        hists.append(hist)
                        hist_labels.append(display_labels[i])
                
                if len(hists) >= 2:
                    # Create overlay
                    overlay_name = f"{hist_name}_overlay"
                    output_path = mirror_root_structure(f"overlays/{cent_bin}/{jet_dir}", outdir)
                    hist_names = [hist_name] * len(hists)  # Same histogram name for all in multi-file overlay
                    create_overlay_plot(hists, hist_labels, overlay_name, config_data, output_path, plot_formats, "multi-file", hist_names)
                    overlay_count += 1
    
    return overlay_count

def create_ratio_histogram(num_hist, den_hist, config_data):
    """
    Create ratio histogram by dividing two histograms.
    
    Args:
        num_hist: Numerator histogram
        den_hist: Denominator histogram
        config_data: Config dictionary with ratio settings
    
    Returns:
        ROOT.TH1 ratio histogram
    """
    
    # Clone the histograms to avoid modifying originals
    ratio = num_hist.Clone(f"{num_hist.GetName()}_ratio")
    ratio.SetDirectory(0)  # Detach from current directory
    
    # Configure ratio histogram appearance
    ratio.SetTitle("")
    ratio.GetYaxis().SetTitle(config_data.get("Ratio.YTitle", "Ratio"))
    
    # Y-axis range will be set globally after all ratio histograms are created
    
    # Increase Y-axis title size and offset
    ratio.GetYaxis().SetTitleSize(0.12)
    ratio.GetYaxis().SetTitleOffset(0.5)
    ratio.GetYaxis().SetLabelSize(0.10)
    
    # Increase X-axis title size and offset
    ratio.GetXaxis().SetTitleSize(0.12)
    ratio.GetXaxis().SetTitleOffset(1.0)
    ratio.GetXaxis().SetLabelSize(0.10)
    
    # Divide by denominator
    ratio.Divide(den_hist)
    
    return ratio

def add_selection_text(pad, config_data, hists=None):
    """
    Add selection text/labels to the plot with content-aware positioning.
    Args:
        pad: ROOT.TPad or ROOT.TCanvas to draw on
        config_data (dict): Config dictionary with selection text settings
        hists: List of histograms for content-aware positioning (optional)
    """
    
    # Check if selection text is configured
    selection_text = config_data.get("SelectionText.Custom", "")
    
    if not selection_text:
        return  # No selection text to add
    
    # DEBUG: Log the raw selection text from config
    logger.trace(f"[TEXT TRACE] Raw selection text from config: '{selection_text}'")
    
    # Parse selection text - staged approach for boundary checking
    # Stage 1: Default split by \\n only (no comma splitting initially)
    default_text_lines = [line.rstrip('\\').strip() for line in selection_text.split("\\n") if line.rstrip('\\').strip()]
    
    # DEBUG: Log the processed text lines
    logger.trace(f"[TEXT TRACE] After \\n split, text lines: {default_text_lines}")
    
    # Position options
    position = config_data.get("SelectionText.Position", "top-left").lower()
    text_size = float(config_data.get("SelectionText.TextSize", "0.04"))
    
    # Default positions based on named position - adjusted to avoid axis overlap
    positions = {
        "top-left": (0.20, 0.80),      # Moved down from 0.85 to avoid Y-axis top
        "top-right": (0.70, 0.80),     # Moved down from 0.85
        "bottom-left": (0.20, 0.30),   # Moved up from 0.25 to avoid X-axis
        "bottom-right": (0.70, 0.30),  # Moved up from 0.25
        "top-center": (0.50, 0.80),    # Moved down from 0.85
        "bottom-center": (0.50, 0.30), # Moved up from 0.25
        "center": (0.50, 0.50)
    }
    
    # Get coordinates - either from named position or from explicit config
    if "SelectionText.X" in config_data and "SelectionText.Y" in config_data:
        x = float(config_data.get("SelectionText.X"))
        y = float(config_data.get("SelectionText.Y"))
    else:
        x, y = positions.get(position, positions["top-left"])
    
    # Create and configure text objects
    text_objects = []
    line_spacing = float(config_data.get("SelectionText.LineSpacing", "0.06"))  # Configurable line spacing
    check_boundaries = bool(int(config_data.get("SelectionText.CheckBoundaries", "0")))  # Boundary checking
    
    # Apply comprehensive boundary checking with content awareness
    text_lines = default_text_lines  # Start with default split
    
    if check_boundaries:
        # Use the comprehensive boundary checking function with histogram data
        x, y, text_lines, boundary_adjusted = check_text_boundaries(
            text_lines, x, y, text_size, line_spacing, positions, position, hists, config_data, text_type="SelectionText"
        )
    
    # Make sure we're drawing on the correct pad
    pad.cd()
    
    for i, line in enumerate(text_lines):
        if line.strip():  # Skip empty lines
            # DEBUG: Log each line before creating TLatex
            logger.trace(f"[TEXT TRACE] Creating TLatex for line {i}: '{line}'")
            
            text = ROOT.TLatex(x, y - i * line_spacing, line)
            text.SetNDC(True)  # Use normalized coordinates (0-1)
            text.SetTextSize(text_size)
            text.SetTextFont(42)
            
            # Check for custom color
            text_color = int(config_data.get("SelectionText.Color", "1"))  # Default is black
            text.SetTextColor(text_color)
            
            text_objects.append(text)
    
    # Draw all text objects on the specified pad and add to primitives for persistence
    for i, text in enumerate(text_objects):
        text.Draw("SAME")
        
        # IMPORTANT: Add the text object to the pad so it gets saved with the canvas
        # This ensures TLatex objects persist when canvas is saved
        pad.GetListOfPrimitives().Add(text)
    
    # Force update
    pad.Update()

def check_text_boundaries(text_lines, x, y, text_size, line_spacing, positions, position="top-left", hists=None, config_data=None, text_type=None):
    """
    Comprehensive boundary checking for selection text placement.
    Now includes content-aware positioning and collision detection.
    
    Args:
        text_lines: List of text lines to check
        x, y: Current position coordinates (NDC, 0-1)
        text_size: Text size 
        line_spacing: Spacing between lines
        positions: Dictionary of alternative positions
        position: Current position name for relocation logic
        hists: List of histograms for content analysis (optional)
        config_data: Configuration dictionary (optional)
        text_type: Type of text ("SelectionText", etc.) for context-aware processing
        
    Returns:
        tuple: (adjusted_x, adjusted_y, final_text_lines, boundary_violated)
    """
    if len(text_lines) <= 1:
        return x, y, text_lines, False
    
    # Calculate text dimensions and bounding box
    text_box = get_text_bounding_box(x, y, text_lines, text_size, line_spacing)
    
    # Check basic boundaries first
    margin = 0.05
    total_text_height = (len(text_lines) - 1) * line_spacing + text_size
    max_line_length = max(len(line) for line in text_lines)
    estimated_text_width = max_line_length * text_size * 0.6
    
    vertical_overflow = (y - total_text_height) < margin or y > (1.0 - margin)
    horizontal_overflow = x < margin or (x + estimated_text_width) > (1.0 - margin)
    
    boundary_violated = vertical_overflow or horizontal_overflow
    
    # If no boundary issues and content awareness is disabled, return early
    if not boundary_violated:
        content_aware = config_data and bool(int(config_data.get("SelectionText.AvoidData", "0")))
        if not content_aware or not hists:
            return x, y, text_lines, False

    # Try content-aware positioning if enabled and histograms available
    avoid_data_enabled = config_data and bool(int(config_data.get("SelectionText.AvoidData", "0")))
    
    if avoid_data_enabled and hists:
        logger.debug(f"Content-aware positioning enabled, analyzing {len(hists)} histograms")
        # Use original text lines for content analysis
        content_text_box = get_text_bounding_box(x, y, text_lines, text_size, line_spacing)
        logger.trace(f"Text box: {content_text_box}")
        content_result = find_content_aware_position(content_text_box, hists, config_data, "SelectionText")
        
        if content_result:
            best_x, best_y, score = content_result
            logger.debug(f"Content-aware positioning found better position: ({best_x:.2f}, {best_y:.2f}) with score {score:.3f}")
            # Use content-aware position with original text lines
            return best_x, best_y, text_lines, True
        else:
            logger.debug("Content-aware positioning returned no result")
    elif avoid_data_enabled:
        logger.debug("Content-aware positioning enabled but no histograms provided")
    else:
        logger.debug("Content-aware positioning disabled (AvoidData=0)")
    
    # Fallback to original boundary-only logic
    new_total_height = (len(text_lines) - 1) * line_spacing + text_size
    new_max_length = max(len(line) for line in text_lines)
    new_estimated_width = new_max_length * text_size * 0.6
    
    new_vertical_overflow = (y - new_total_height) < margin or y > (1.0 - margin)
    new_horizontal_overflow = x < margin or (x + new_estimated_width) > (1.0 - margin)
    
    if not (new_vertical_overflow or new_horizontal_overflow):
        # Original text lines work fine!
        return x, y, text_lines, True
    
    # Stage 3: Position relocation using original logic
    relocation_order = []
    
    if position in ["top-left", "top-center", "top-right"] or y > 0.5:
        if x < 0.5:  # Left side
            relocation_order = ["bottom-left", "bottom-right", "top-right", "bottom-center"]
        else:  # Right side
            relocation_order = ["bottom-right", "bottom-left", "top-left", "bottom-center"]
    else:
        if x < 0.5:  # Left side
            relocation_order = ["top-left", "top-right", "bottom-right", "top-center"]
        else:  # Right side
            relocation_order = ["top-right", "top-left", "bottom-left", "top-center"]
    
    # Try each relocation position
    for new_position_name in relocation_order:
        if new_position_name in positions:
            new_x, new_y = positions[new_position_name]
            
            # Check if this position works with original text lines
            new_vert_overflow = (new_y - new_total_height) < margin or new_y > (1.0 - margin)
            new_horiz_overflow = new_x < margin or (new_x + new_estimated_width) > (1.0 - margin)
            
            if not (new_vert_overflow or new_horiz_overflow):
                return new_x, new_y, text_lines, True
    
    # If all else fails, use the best fallback position
    fallback_x, fallback_y = positions.get("top-right", (0.7, 0.8))
    return fallback_x, fallback_y, text_lines, True

def find_best_legend_position(hists, config_data):
    """
    Find the best position for the legend by analyzing histogram content.
    Tries to place legend in empty areas of the plot.
    
    Args:
        hists: List of ROOT histograms
        config_data: Config dictionary
    
    Returns:
        tuple: (x1, y1, x2, y2) coordinates for legend placement
    """
    
    # Skip auto-positioning if disabled
    auto_position = bool(int(config_data.get("Legend.AutoPosition", "0")))
    if not auto_position:
        # Use fixed position from config
        legend_pos = config_data.get("OverlayLegendPosition" if "OverlayLegendPosition" in config_data 
                                    else "LegendPosition", "0.65,0.65,0.9,0.9")
        return [float(x) for x in legend_pos.split(",")]
    
    # Default fallback positions in order of preference
    preferred_positions = config_data.get("Legend.PreferredPositions", "top-right,top-left,bottom-right,bottom-left").lower()
    preferred_positions = [pos.strip() for pos in preferred_positions.split(",")]
    
    # Define candidate positions (x1, y1, x2, y2) in NDC coordinates
    positions = {
        "top-right": (0.65, 0.65, 0.89, 0.89),
        "top-left": (0.15, 0.65, 0.39, 0.89),
        "bottom-right": (0.65, 0.15, 0.89, 0.39),
        "bottom-left": (0.15, 0.15, 0.39, 0.39),
        "center-right": (0.65, 0.40, 0.89, 0.64),
        "center-left": (0.15, 0.40, 0.39, 0.64)
    }
    
    # If avoid overlap is disabled, just return the first preferred position
    avoid_overlap = bool(int(config_data.get("Legend.AvoidOverlap", "1")))
    if not avoid_overlap or not hists:
        default_pos = positions.get(preferred_positions[0], positions["top-right"])
        return default_pos
    
    # Analyze histogram content density
    # We'll use a simple scoring system - lower score is better for legend placement
    scores = {}
    ref_hist = hists[0]  # Use first histogram as reference
    
    # Divide canvas into zones and score each zone
    nbins_x = ref_hist.GetNbinsX()
    
    # For each potential legend position
    for pos_name, (x1, y1, x2, y2) in positions.items():
        # Map NDC coordinates to bin ranges
        bin_x1 = max(1, int(x1 * nbins_x))
        bin_x2 = min(nbins_x, int(x2 * nbins_x))
        
        # Get content in this region
        total_content = 0
        bin_count = 0
        
        for i in range(bin_x1, bin_x2 + 1):
            for hist in hists:
                # Use bin content and bin error to estimate "importance"
                bin_content = hist.GetBinContent(i)
                bin_error = hist.GetBinError(i)
                
                # Higher content means more important to see, so higher score
                total_content += abs(bin_content) + bin_error
                bin_count += 1
        
        # Average content per bin in this region (avoid division by zero)
        avg_content = total_content / max(1, bin_count)
        scores[pos_name] = avg_content
    
    # Sort positions by score (lower is better)
    sorted_positions = sorted(scores.items(), key=lambda x: x[1])
    
    # Check the preferred positions first
    for preferred in preferred_positions:
        for pos_name, score in sorted_positions:
            if pos_name == preferred:
                return positions[pos_name]
    
    # Fallback to the position with lowest score
    return positions[sorted_positions[0][0]]

def apply_canvas_settings(canvas, config_data, is_ratio=False):
    """
    Apply canvas settings from config to a ROOT.TCanvas.
    
    Args:
        canvas: ROOT.TCanvas to configure
        config_data: Config dictionary with canvas settings
        is_ratio: Whether this canvas will contain a ratio plot
    """
    if not canvas:
        return
        
    # Apply canvas margins
    left_margin = float(config_data.get("Canvas.LeftMargin", "0.15"))
    right_margin = float(config_data.get("Canvas.RightMargin", "0.05"))
    top_margin = float(config_data.get("Canvas.TopMargin", "0.08"))
    bottom_margin = float(config_data.get("Canvas.BottomMargin", "0.12"))
    
    # For ratio plots, we might want to adjust the bottom margin
    if is_ratio:
        bottom_margin = float(config_data.get("Canvas.RatioPadBottomMargin", "0.3"))
    
    canvas.SetLeftMargin(left_margin)
    canvas.SetRightMargin(right_margin)
    canvas.SetTopMargin(top_margin)
    canvas.SetBottomMargin(bottom_margin)

def standardize_text_sizes(histogram, pad_height, config_data):
    """
    Apply consistent text sizing that accounts for pad height for visual consistency.
    Ensures identical visual appearance regardless of single vs multi-pad layout.
    
    Args:
        histogram: ROOT.TH1 object to modify
        pad_height: Height fraction of the pad (1.0 for single plot, 0.7/0.3 for ratio plots)
        config_data: Configuration dictionary with base text size settings
    """
    
    if not histogram:
        return
    
    # Get base text sizes from config with defaults
    base_axis_title_size = float(config_data.get("Axis.TitleSize", "0.05"))
    base_axis_label_size = float(config_data.get("Axis.LabelSize", "0.04"))
    
    # Adaptive offsets based on plot type
    # Single plots (pad_height = 1.0) need smaller offsets
    # Ratio plots (pad_height < 1.0) need larger offsets
    if pad_height >= 1.0:  # Single plot
        base_title_offset_x = float(config_data.get("Axis.TitleOffsetX.Single", "1.2"))  # Smaller for single plots
        base_title_offset_y = float(config_data.get("Axis.TitleOffsetY.Single", "1.0"))
    else:  # Multi-pad plot (ratio plot)
        base_title_offset_x = float(config_data.get("Axis.TitleOffsetX", "3.5"))  # Larger for ratio plots
        base_title_offset_y = float(config_data.get("Axis.TitleOffsetY", "1.4"))
        
    base_marker_size = float(config_data.get("Marker.Size", "0.8"))
    
    # Calculate scaling factor based on pad height
    # Reference: A full canvas (pad_height = 1.0) uses base sizes
    # Smaller pads need larger text sizes for visual consistency
    scale_factor = 1.0 / pad_height
    
    # Apply scaled sizes to X-axis
    x_axis = histogram.GetXaxis()
    x_axis.SetTitleSize(base_axis_title_size * scale_factor)
    x_axis.SetLabelSize(base_axis_label_size * scale_factor)
    x_axis.SetTitleOffset(base_title_offset_x / scale_factor)
    
    # Apply scaled sizes to Y-axis (same logic as X-axis)
    y_axis = histogram.GetYaxis()
    y_axis.SetTitleSize(base_axis_title_size * scale_factor)
    y_axis.SetLabelSize(base_axis_label_size * scale_factor)
    y_axis.SetTitleOffset(base_title_offset_y / scale_factor)
    
    # Marker sizes are absolute and should NOT be scaled with pad size
    if histogram.GetMarkerStyle() > 0:  # Has markers
        histogram.SetMarkerSize(base_marker_size)  # No scaling for markers

def analyze_region_density(hists, x1, y1, x2, y2):
    """
    Calculate how much important data exists in a region.
    
    Args:
        hists: List of ROOT histograms
        x1, y1, x2, y2: Region coordinates in NDC (0-1)
    
    Returns:
        float: Density score (lower is better for placing elements)
    """
    if not hists:
        return 0.0
    
    total_score = 0.0
    ref_hist = hists[0]
    nbins_x = ref_hist.GetNbinsX()
    
    # Map NDC coordinates to bin ranges
    bin_x1 = max(1, int(x1 * nbins_x))
    bin_x2 = min(nbins_x, int(x2 * nbins_x))
    
    # Calculate average content in the region
    for i in range(bin_x1, bin_x2 + 1):
        for hist in hists:
            bin_content = hist.GetBinContent(i)
            bin_error = hist.GetBinError(i)
            # Weight both content and error (error regions are important to see)
            total_score += abs(bin_content) + bin_error * 0.5
    
    # Normalize by number of bins and histograms
    bin_count = max(1, bin_x2 - bin_x1 + 1)
    avg_score = total_score / (bin_count * len(hists))
    
    return avg_score

def check_element_collision(elem1_box, elem2_box, padding=0.02):
    """
    Check if two rectangular elements would overlap.
    
    Args:
        elem1_box: (x1, y1, x2, y2) tuple for first element
        elem2_box: (x1, y1, x2, y2) tuple for second element  
        padding: Additional spacing buffer between elements
    
    Returns:
        bool: True if collision detected
    """
    if not elem1_box or not elem2_box:
        return False
    
    x1a, y1a, x2a, y2a = elem1_box
    x1b, y1b, x2b, y2b = elem2_box
    
    # Add padding to the boxes
    x1a -= padding
    y1a -= padding
    x2a += padding
    y2a += padding
    
    # Check if rectangles overlap
    return not (x2a < x1b or x2b < x1a or y2a < y1b or y2b < y1a)

def get_accurate_text_width(text, text_size, font=42):
    """
    Get accurate text width using ROOT's TLatex measurement.
    Handles LaTeX syntax correctly.
    
    Args:
        text: Text string (may contain LaTeX)
        text_size: Text size in NDC coordinates
        font: ROOT font number
        
    Returns:
        float: Actual text width in NDC coordinates
    """
    try:
        
        # Create temporary TLatex object for measurement
        temp_text = ROOT.TLatex(0, 0, text)
        temp_text.SetTextSize(text_size)
        temp_text.SetTextFont(font)
        
        # Get actual rendered width
        width = temp_text.GetXsize()
        
        # Clean up
        del temp_text
        
        return width
        
    except Exception as e:
        # Fallback to improved character-based estimation
        logger.debug(f"TLatex measurement failed: {e}, using fallback")
        return fallback_text_width_estimation(text, text_size)

def fallback_text_width_estimation(text, text_size):
    """
    Fallback text width estimation when ROOT measurement fails.
    More accurate than simple character counting.
    """
    import re
    
    # Handle common LaTeX patterns
    clean_text = text
    latex_replacements = {
        r'#gamma': 'γ',
        r'#pi': 'π', 
        r'#eta': 'η',
        r'#phi': 'φ',
        r'#Delta': 'Δ',
        r'#varphi': 'φ',
        r'#alpha': 'α',
        r'#beta': 'β',
        r'#theta': 'θ',
        r'\^{([^}]*)}': r'\1',  # Superscript - treat as normal size for safety
        r'_{([^}]*)}': r'\1',   # Subscript - treat as normal size for safety
    }
    
    for pattern, replacement in latex_replacements.items():
        clean_text = re.sub(pattern, replacement, clean_text)
    
    # More realistic character width (calibrated for typical ROOT fonts)
    avg_char_width = 0.45 * text_size
    return len(clean_text) * avg_char_width

def get_text_bounding_box(x, y, text_lines, text_size, line_spacing):
    """
    Calculate the bounding box for text elements using accurate ROOT measurements.
    
    Args:
        x, y: Starting position
        text_lines: List of text lines
        text_size: Text size
        line_spacing: Spacing between lines
    
    Returns:
        tuple: (x1, y1, x2, y2) bounding box
    """
    if not text_lines:
        return None
    
    # Calculate accurate width for each line and use the maximum
    max_width = 0
    for line in text_lines:
        line_width = get_accurate_text_width(line, text_size)
        max_width = max(max_width, line_width)
    
    # Calculate total height
    total_height = (len(text_lines) - 1) * line_spacing + text_size
    
    # Text starts at (x,y) and goes down and right
    x1 = x
    y1 = y - total_height  # Bottom of text
    x2 = x + max_width     # Right edge (accurate width)
    y2 = y                 # Top of text
    
    return (x1, y1, x2, y2)

def find_content_aware_position(element_box, hists, config_data, element_type="text"):
    """
    Find the best position considering both boundaries and content density.
    
    Args:
        element_box: Current bounding box (x1, y1, x2, y2)
        hists: List of histograms for content analysis
        config_data: Configuration dictionary
        element_type: "text" or "legend" for different config keys
    
    Returns:
        tuple: (best_x, best_y, score) - best position and its score
    """
    if not element_box or not hists:
        return None
    
    # Check if content awareness is enabled
    avoid_data_key = f"{element_type.title()}.AvoidData" if element_type in ["Text", "Legend"] else f"SelectionText.AvoidData"
    avoid_data = bool(int(config_data.get(avoid_data_key, "0")))
    
    if not avoid_data:
        return None  # Content awareness disabled
    
    # Define candidate positions with priority ordering (better positions first)
    candidate_positions = [
        ("top-left", (0.20, 0.80)),
        ("top-right", (0.70, 0.80)),
        ("bottom-left", (0.20, 0.30)),
        ("bottom-right", (0.70, 0.30)),
        ("center-right", (0.65, 0.50)),  # New: center-right position
        ("center-left", (0.20, 0.50)),   # New: center-left position
        ("top-center", (0.50, 0.80)),
        ("bottom-center", (0.50, 0.30)),
    ]
    
    best_position = None
    best_score = float('inf')
    best_pos_name = ""
    
    x1, y1, x2, y2 = element_box
    width = x2 - x1
    height = y2 - y1
    
    logger.debug(f"Evaluating {len(candidate_positions)} candidate positions for element box {element_box}")
    
    # Test each candidate position
    for pos_name, (test_x, test_y) in candidate_positions:
        # Adjust for element dimensions
        test_x1 = test_x
        test_y1 = test_y - height
        test_x2 = test_x + width
        test_y2 = test_y
        
        # Check boundaries
        margin = 0.05
        if (test_x1 < margin or test_x2 > (1.0 - margin) or 
            test_y1 < margin or test_y2 > (1.0 - margin)):
            logger.debug(f"Position {pos_name} rejected: out of bounds ({test_x1:.2f}, {test_y1:.2f}, {test_x2:.2f}, {test_y2:.2f})")
            continue  # Out of bounds
        
        # Check for collision with legend if AvoidLegend is enabled
        avoid_legend = bool(int(config_data.get(f"{element_type}.AvoidLegend", "0")))
        if avoid_legend:
            # Assume legend is typically at (0.65, 0.65, 0.9, 0.9) - can be made configurable
            legend_box = (0.65, 0.65, 0.9, 0.9)
            element_test_box = (test_x1, test_y1, test_x2, test_y2)
            
            if check_element_collision(element_test_box, legend_box, padding=0.02):
                logger.debug(f"Position {pos_name} rejected: collides with legend")
                continue  # Collision with legend
        
        # Calculate content density score
        density_score = analyze_region_density(hists, test_x1, test_y1, test_x2, test_y2)
        logger.trace(f"Position {pos_name} at ({test_x:.2f}, {test_y:.2f}): density score = {density_score:.4f}")
        
        if density_score < best_score:
            best_score = density_score
            best_position = (test_x, test_y)
            best_pos_name = pos_name
            logger.trace(f"New best position: {pos_name} with score {density_score:.4f}")
    
    # Apply scoring thresholds - reject positions with too much data overlap
    # Handle different element types and their config key formats
    if element_type == "text":
        score_threshold = float(config_data.get("SelectionText.ScoreThreshold", "0.1"))
    else:
        score_threshold = float(config_data.get(f"{element_type.title()}.ScoreThreshold", "0.1"))
    
    if best_position and best_score > score_threshold:
        logger.warning(f"Best position {best_pos_name} has high data overlap (score: {best_score:.3f} > threshold: {score_threshold:.3f})")
        
        # For high-overlap cases, try to find any position below threshold
        acceptable_threshold = score_threshold * 2.0  # Be more lenient for backup positions
        
        # Re-evaluate all positions looking for anything below acceptable threshold
        for pos_name, (test_x, test_y) in candidate_positions:
            test_x1 = test_x
            test_y1 = test_y - height  
            test_x2 = test_x + width
            test_y2 = test_y
            
            # Quick boundary check
            margin = 0.05
            if (test_x1 < margin or test_x2 > (1.0 - margin) or 
                test_y1 < margin or test_y2 > (1.0 - margin)):
                continue
                
            # Quick collision check
            avoid_legend = bool(int(config_data.get(f"{element_type}.AvoidLegend", "0")))
            if avoid_legend:
                legend_box = (0.65, 0.65, 0.9, 0.9)
                element_test_box = (test_x1, test_y1, test_x2, test_y2)
                if check_element_collision(element_test_box, legend_box, padding=0.02):
                    continue
                    
            # Calculate score for this alternative
            test_score = analyze_region_density(hists, test_x1, test_y1, test_x2, test_y2)
            
            if test_score < acceptable_threshold and test_score < best_score:
                logger.debug(f"Found better alternative: {pos_name} with score {test_score:.3f}")
                best_position = (test_x, test_y)
                best_score = test_score
                best_pos_name = pos_name
                break
    
    if best_position:
        logger.debug(f"Content-aware positioning selected {best_pos_name} at ({best_position[0]:.2f}, {best_position[1]:.2f}) with score {best_score:.4f}")
        return best_position + (best_score,)
    else:
        logger.warning("No suitable position found by content-aware algorithm")
        return None
