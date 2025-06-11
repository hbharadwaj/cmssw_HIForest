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
    
    # Verbosity levels
    DEBUG = 0
    INFO = 1
    WARNING = 2
    ERROR = 3
    CRITICAL = 4
    
    def __init__(self, verbosity=INFO, use_colors=True):
        self.verbosity = verbosity
        self.use_colors = use_colors and sys.stdout.isatty()  # Only use colors if in terminal
    
    def _colorize(self, text, color):
        """Apply color to text if colors are enabled."""
        if self.use_colors and color in self.COLORS:
            return f"{self.COLORS[color]}{text}{self.COLORS['RESET']}"
        return text
    
    def debug(self, message):
        """Debug level logging (most verbose)."""
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
    import ROOT
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
    import ROOT
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
            print(f"    {os.path.join(path, name)} [{obj.ClassName()}]")

def get_histogram_recursive(root_file, hname, path=''):
    """Recursively search for a histogram by name in a ROOT file."""
    import ROOT
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

def create_legend(config, entries, overlay=False):
    """
    Create and configure a ROOT.TLegend using config and add entries.
    Args:
        config (dict): Config dictionary.
        entries (list): List of (obj, label, style) tuples.
        overlay (bool): If True, use overlay legend keys.
    Returns:
        ROOT.TLegend
    """
    import ROOT
    if overlay:
        prefix = "Overlay."
    else:
        prefix = "Legend"
    legend_pos = config.get(f"{prefix}LegendPosition", "0.65,0.65,0.9,0.9")
    legend_text_size = float(config.get(f"{prefix}LegendTextSize", 0.03))
    legend_fill_style = int(config.get(f"{prefix}LegendFillStyle", 0))
    legend_border_size = int(config.get(f"{prefix}LegendBorderSize", 0))
    legend_pos = [float(x) for x in legend_pos.split(",")]
    legend = ROOT.TLegend(*legend_pos)
    legend.SetTextSize(legend_text_size)
    legend.SetFillStyle(legend_fill_style)
    legend.SetBorderSize(legend_border_size)
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
    import ROOT
    cms_label = config.get("CMSLabel", "Preliminary")
    cms_energy = config.get("CMSEnergyText", "")
    cms_lumi = config.get("CMSLuminosity", "")
    cms_extra = config.get("CMSExtraText", "")
    if hasattr(ROOT, 'CMS_lumi'):
        if hasattr(ROOT, 'writeExtraText'):
            ROOT.writeExtraText = True
        if hasattr(ROOT, 'extraText'):
            ROOT.extraText = cms_label
        if hasattr(ROOT, 'lumi_13p6TeV'):
            ROOT.lumi_13p6TeV = cms_lumi
        if hasattr(ROOT, 'lumiText'):
            ROOT.lumiText = cms_energy
        ROOT.CMS_lumi(canvas, 0, 0)

def normalize_histogram(hist, method="area"):
    """
    Normalize a ROOT histogram in-place.
    Args:
        hist: ROOT.TH1 or TH2 object
        method: 'area', 'density', or 'none'
    """
    import ROOT
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
    import ROOT
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
    import ROOT
    
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
                    create_overlay_plot(hists, hist_labels, overlay_name, config_data, 
                                      os.path.join(outdir, display_label, "overlays", cent_bin), plot_formats, "single-file")
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
                    create_overlay_plot(hists, hist_labels, overlay_name, config_data, 
                                      os.path.join(outdir, display_label, "overlays", jet_dir), plot_formats, "single-file")
                    overlay_count += 1
    
    # Return total number of overlay plots created
    return overlay_count

def create_overlay_plot(hists, hist_labels, overlay_name, config_data, out_subdir, plot_formats, overlay_type="overlay"):
    """
    Create an overlay plot with given histograms and labels.
    """
    import ROOT
    
    # Suppress ROOT Info messages
    ROOT.gROOT.SetBatch(True)
    old_level = ROOT.gErrorIgnoreLevel
    ROOT.gErrorIgnoreLevel = ROOT.kWarning
    
    try:
        # Create canvas
        c = ROOT.TCanvas(f"c_{overlay_name}", overlay_name, 800, 600)
        colors = [ROOT.kBlue, ROOT.kRed, ROOT.kGreen+2, ROOT.kMagenta, ROOT.kOrange+7, ROOT.kCyan+2]
        markers = [ROOT.kFullCircle, ROOT.kFullSquare, ROOT.kFullTriangleUp, ROOT.kFullTriangleDown, 
                  ROOT.kFullDiamond, ROOT.kFullStar]
        
        # Apply normalization if enabled
        overlay_norm_enable = bool(int(config_data.get("Overlay.Normalize", 1)))
        overlay_norm_method = config_data.get("Overlay.NormalizationMethod", "area")
        
        legend_entries = []
        for i, hist in enumerate(hists):
            if overlay_norm_enable:
                normalize_histogram(hist, overlay_norm_method)
            
            # Set line properties
            hist.SetLineWidth(2)
            hist.SetLineColor(colors[i % len(colors)])
            
            # Set marker properties
            hist.SetMarkerColor(colors[i % len(colors)])
            hist.SetMarkerStyle(markers[i % len(markers)])
            hist.SetMarkerSize(0.8)
            
            hist.SetStats(0)
            
            drawopt = "E1P" if hist.InheritsFrom("TH1") else "COLZ"
            if i == 0:
                hist.Draw(drawopt)
            else:
                hist.Draw(drawopt + " SAME")
            
            # Use "lp" for legend to show both line and markers
            legend_entries.append((hist, hist_labels[i], "lp"))
        
        # Create legend and labels
        legend = create_legend(config_data, legend_entries, overlay=True)
        legend.Draw()
        apply_cms_label(c, config_data)
        
        # Create output directory
        if not os.path.exists(out_subdir):
            os.makedirs(out_subdir, exist_ok=True)
        
        # Save plot
        for ext in plot_formats:
            c.SaveAs(os.path.join(out_subdir, f"{overlay_name}.{ext}"))
        
        logger.info(f"{overlay_type.title()} overlay plotted and saved: {overlay_name}")
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
    import ROOT
    
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
                    create_overlay_plot(hists, hist_labels, overlay_name, config_data, output_path, plot_formats, "multi-file")
                    overlay_count += 1
    
    return overlay_count
