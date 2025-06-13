#!/usr/bin/env python3
"""
plotGammaJet.py

Main script for CMS Heavy Ion Gamma-Jet analysis plotting.
- Reads TEnv-compatible config(s) and ROOT file(s)
- Produces CMS-style plots using ROOT's TCanvas
- Supports overlays, normalization, error bars, color schemes,                         # Check test mode limit befor                        # Save plot
                        for ext in plot_formats:
                            c.SaveAs(os.path.join(output_path, f"{config_key}.{ext}"))
                        
                        logger.info(f"Plotted and saved: {config_key} in {output_path}")
                        c.Close()
                        
                        # Increment counter for test mode
                        total_plot_count += 1g plot
                        if args.test and total_plot_count >= args.maxplots:
                            logger.debug(f"Test mode: reached {args.maxplots} plot limit, stopping")
                            break_outer = True
                            breakds, labels, centrality/jet overlays, ratio plots, and batch/test mode
- All customization is config-driven
"""

import sys
import os
import argparse

# Helper functions will be imported from plot_helpers.py
# ROOT, tdrstyle, CMS_lumi will be loaded in main()

def main():
    import sys
    import os
    import argparse
    
    # Early startup message (before logger is configured)
    
    parser = argparse.ArgumentParser(
        description="CMS Heavy Ion Gamma-Jet analysis plotting script (ROOT TCanvas, config-driven)")
    parser.add_argument('-i', '--input', nargs='+', default=None,
                        help='Input ROOT file(s). For single-file mode or to override config-specified files.')
    parser.add_argument('-c', '--config', nargs='+', required=True,
                        help='TEnv-compatible config file(s). For overlays, specify overlay config and/or per-file configs.')
    parser.add_argument('-o', '--outdir', default='/eos/user/b/bharikri/www/Run3GammaJet/2025_06_11/2023_PbPb/plots',
                        help='Output directory for plots (default: plots)')
    parser.add_argument('--test', action='store_true',
                        help='Test mode: only produce a small number of plots for verification')
    parser.add_argument('--plotlist', nargs='+', default=None,
                        help='Optional: list of histogram names to plot (overrides config list)')
    parser.add_argument('--maxplots', type=int, default=5,
                        help='Maximum number of plots to produce in test mode (default: 5)')
    parser.add_argument('--batch', action='store_true',
                        help='Batch mode: do not show canvases interactively')
    parser.add_argument('-v', '--verbose', action='count', default=0,
                        help='Increase verbosity (-v for DEBUG, -vv for TRACE). Default is INFO level.')
    parser.add_argument('--no-colors', action='store_true',
                        help='Disable colored output')
    args = parser.parse_args()

    # Import helpers to get logger
    import plot_helpers
    
    # Configure logging based on verbosity (new intuitive hierarchy)
    if args.verbose >= 2:
        plot_helpers.set_verbosity(plot_helpers.Logger.TRACE)   # -vv gives TRACE (most verbose)
    elif args.verbose >= 1:
        plot_helpers.set_verbosity(plot_helpers.Logger.DEBUG)   # -v gives DEBUG
    else:
        plot_helpers.set_verbosity(plot_helpers.Logger.INFO)    # Default is INFO level
    
    if args.no_colors:
        plot_helpers.set_colors(False)
    
    logger = plot_helpers.logger
    logger.debug("Starting CMS Heavy Ion Gamma-Jet plotting script")
    logger.debug(f"Arguments: {args}")

    # Import ROOT and helper functions
    logger.debug("Importing ROOT...")
    try:
        import ROOT
        # Suppress ROOT Info messages
        ROOT.gROOT.SetBatch(True)
        ROOT.gErrorIgnoreLevel = ROOT.kWarning
        logger.debug("ROOT imported successfully")
    except ImportError:
        logger.error("Could not import ROOT. Please ensure PyROOT is available in your environment.")
        sys.exit(1)

    # Load tdrStyle.C and CMS_lumi.C from local include directory
    tdrstyle_path = os.path.join(os.path.dirname(__file__), 'include', 'tdrStyle.C')
    cmslumi_path = os.path.join(os.path.dirname(__file__), 'include', 'CMS_lumi.C')
    if not os.path.exists(tdrstyle_path) or not os.path.exists(cmslumi_path):
        logger.error(f"tdrStyle.C or CMS_lumi.C not found in include directory: {tdrstyle_path}, {cmslumi_path}")
        sys.exit(1)
    ROOT.gROOT.ProcessLine(f'.L {tdrstyle_path}')
    ROOT.gROOT.ProcessLine(f'.L {cmslumi_path}')
    try:
        ROOT.setTDRStyle()
    except Exception as e:
        logger.warning(f"Could not set TDR style: {e}")

    # Parse config(s) - Priority: main config first, then individual configs
    config_data = plot_helpers.parse_configs(args.config)

    # Determine input files: from command line or from config
    input_files = args.input
    if not input_files:
        # Extract from config InputFile.* entries
        input_files = []
        for k, v in config_data.items():
            if k.startswith("InputFile.") and ":" in v:
                parts = v.split(":")
                if len(parts) >= 1:
                    input_files.append(parts[0])  # First part is the ROOT file path
        
        if not input_files:
            logger.error("No input files specified. Use -i argument or InputFile.* entries in config.")
            sys.exit(1)
        
        logger.debug(f"Using input files from config: {input_files}")

    # Open ROOT file(s)
    root_files = plot_helpers.open_root_files(input_files)
    logger.info(f"Successfully opened {len(root_files)} ROOT file(s)")

    # Detect mode
    mode = plot_helpers.detect_mode(config_data)
    logger.info(f"Mode detected: {mode}")

    # List available histograms (for debug)
    if logger.verbosity <= logger.DEBUG:
        logger.debug("Listing histograms in input file(s):")
        for idx, rf in enumerate(root_files):
            logger.debug(f"  File {idx+1}: {input_files[idx]}")
            plot_helpers.list_root_histograms(rf)

    logger.debug("Config parsing and ROOT file navigation complete. Ready for plotting logic.")

    # Output directory setup
    outdir = args.outdir
    if not os.path.exists(outdir):
        os.makedirs(outdir)
    logger.info(f"Output directory: {outdir}")

    # Determine output formats from config
    plot_formats = config_data.get("PlotFormats", "png,root").split(",")
    plot_formats = [fmt.strip() for fmt in plot_formats if fmt.strip()]
    logger.info(f"Plot formats: {', '.join(plot_formats)}")

    if mode == "multi":
        # Multi-file mode: Data vs MC overlays
        logger.info("Running in multi-file mode")
        
        # Parse InputFile.* entries for (root_file, config_file, label)
        overlay_inputs = []
        for k, v in config_data.items():
            if k.startswith("InputFile.") and ":" in v:
                parts = v.split(":")
                if len(parts) >= 3:
                    overlay_inputs.append((parts[0], parts[1], parts[2]))
        
        # Parse individual file configs
        file_cfgs = []
        for root_path, config_path, display_label in overlay_inputs:
            file_cfg_data = plot_helpers.parse_configs([config_path])
            # If HistogramConfigFile is present, parse it as well
            if "HistogramConfigFile" in file_cfg_data:
                hist_cfg_path = file_cfg_data["HistogramConfigFile"]
                if not os.path.isabs(hist_cfg_path):
                    hist_cfg_path = os.path.join(os.path.dirname(config_path), hist_cfg_path)
                hist_cfg_data = plot_helpers.parse_configs([hist_cfg_path])
                file_cfg_data.update(hist_cfg_data)
            file_cfgs.append(file_cfg_data)
        
        # Create multi-file overlays
        overlay_count = plot_helpers.create_multi_file_overlays(
            config_data, root_files, file_cfgs, outdir, plot_formats, 
            args.test, args.maxplots if args.test else None
        )
        
        if args.test:
            logger.info(f"Test mode: created {overlay_count} multi-file overlay plots")
        else:
            logger.info(f"Created {overlay_count} multi-file overlay plots")
        
    else:
        # Single-file mode: Individual plots + intra-file overlays
        logger.info("Running in single-file mode")
        
        if len(root_files) != 1:
            logger.error("Single-file mode requires exactly one input file")
            sys.exit(1)
        
        root_file = root_files[0]
        file_cfg = config_data  # Config is the PlotJetSub config
        
        # Load HistogramConfigFile if present
        if "HistogramConfigFile" in file_cfg:
            hist_cfg_path = file_cfg["HistogramConfigFile"]
            if not os.path.isabs(hist_cfg_path):
                hist_cfg_path = os.path.join(os.path.dirname(args.config[0]), hist_cfg_path)
            logger.debug(f"Loading histogram config: {hist_cfg_path}")
            hist_cfg_data = plot_helpers.parse_configs([hist_cfg_path])
            file_cfg.update(hist_cfg_data)
        
        # Get display label from config or use default
        display_label = file_cfg.get("CMSExtraText", "Data")
        if display_label == "Simulation":
            display_label = "PYTHIA8"
        elif not display_label:
            display_label = "Data"
        
        # Get ROOT file structure
        structure = plot_helpers.get_root_file_structure(root_file)
        
        # Create individual plots mirroring ROOT structure
        hist_map = {}
        plot_list = []
        
        if args.plotlist:
            plot_list = args.plotlist
        else:
            # Get from config - check multiple possible keys
            for key in ["OverlayPlots", "GeneralHistograms", "JetHistograms"]:
                if key in file_cfg:
                    plot_list.extend([x.strip() for x in file_cfg[key].split(',') if x.strip()])
        
        logger.debug(f"Plot list from config: {plot_list}")
        
        # Resolve histogram names from config
        for key in plot_list:
            config_hist_key = f"Histogram.{key}.Name"
            hist_map[key] = file_cfg.get(config_hist_key, key)
        
        logger.debug(f"Histogram name mapping: {hist_map}")
        
        if not hist_map:
            logger.error("No histograms specified to plot. Use --plotlist or set in config.")
            sys.exit(1)
        
        # Test mode: limit total number of plots across entire script
        total_plot_count = 0
        if args.test:
            logger.info(f"Test mode: limiting to {args.maxplots} total plots")
        
        # Create single-file overlays FIRST (higher priority than individual plots)
        if args.test and args.maxplots > 0:
            remaining_for_overlays = max(1, args.maxplots // 2)  # Reserve at least half for overlays
            logger.debug(f"Test mode: reserving {remaining_for_overlays} plots for overlays")
        else:
            remaining_for_overlays = None
            
        overlay_count = plot_helpers.create_single_file_overlays(
            config_data, root_file, file_cfg, outdir, plot_formats, display_label, 
            args.test, remaining_for_overlays
        )
        
        if args.test:
            total_plot_count += overlay_count
            logger.info(f"Test mode: created {overlay_count} overlay plots, {total_plot_count}/{args.maxplots} total")
        else:
            total_plot_count += overlay_count
            logger.info(f"Created {overlay_count} single-file overlay plots")
        
        # Create individual plots (with remaining budget)
        individual_plot_count = 0
        for config_key, root_hist_name in hist_map.items():
            # Check test mode limit
            if args.test and total_plot_count >= args.maxplots:
                logger.debug(f"Test mode: reached {args.maxplots} plot limit, stopping individual plots")
                break
                
            found = False
            
            # Search through ROOT file structure
            break_outer = False
            for cent_bin, jet_dirs in structure.items():
                if break_outer:
                    break
                for jet_dir in jet_dirs:
                    hist_path = f"{cent_bin}/{jet_dir}/{root_hist_name}"
                    hist = plot_helpers.get_histogram_recursive(root_file, hist_path)
                    if hist:
                        found = True
                        
                        # Check test mode limit before creating plot
                        if args.test and total_plot_count >= args.maxplots:
                            logger.info(f"Test mode: reached {args.maxplots} plot limit, stopping")
                            break_outer = True
                            break
                        
                        # Apply normalization if enabled
                        norm_enable = bool(int(file_cfg.get("NormalizeHistograms", 0)))
                        norm_method = file_cfg.get("NormalizationMethod", "area")
                        if norm_enable:
                            plot_helpers.normalize_histogram(hist, norm_method)
                        
                        # Create canvas and plot
                        c = ROOT.TCanvas(f"c_{config_key}_{cent_bin}_{jet_dir}", config_key, 800, 600)
                        
                        # Apply consistent canvas margins to match overlay plots
                        plot_helpers.apply_canvas_settings(c, file_cfg)
                        
                        # Apply histogram styling from config
                        hist_config_key = f"Histogram.{config_key}"
                        hist_color = file_cfg.get(f"{hist_config_key}.Color", "1")  # Default black
                        hist_line_width = int(file_cfg.get(f"{hist_config_key}.LineWidth", "2"))
                        hist_marker_style = int(file_cfg.get(f"{hist_config_key}.MarkerStyle", "20"))
                        hist_marker_size = float(file_cfg.get(f"{hist_config_key}.MarkerSize", "0.8"))
                        
                        # Parse color (could be named or numeric)
                        try:
                            if hist_color.isdigit():
                                color_val = int(hist_color)
                            else:
                                # Handle named colors like "kBlue", "kRed"
                                color_val = getattr(ROOT, hist_color) if hasattr(ROOT, hist_color) else ROOT.kBlack
                        except:
                            color_val = ROOT.kBlack
                        
                        hist.SetLineWidth(hist_line_width)
                        hist.SetLineColor(color_val)
                        hist.SetMarkerColor(color_val)
                        hist.SetMarkerStyle(hist_marker_style)
                        hist.SetMarkerSize(hist_marker_size)
                        hist.SetStats(0)
                        
                        # Apply standardized text sizing for consistency with overlay plots
                        plot_helpers.standardize_text_sizes(hist, 1.0, file_cfg)  # pad_height=1.0 for single plots
                        
                        drawopt = "E1P" if hist.InheritsFrom("TH1") else "COLZ"
                        hist.Draw(drawopt)
                        
                        # No legend for single plots
                        plot_helpers.apply_cms_label(c, file_cfg)
                        
                        # Mirror ROOT structure in output
                        output_path = plot_helpers.mirror_root_structure(f"{display_label}/{cent_bin}/{jet_dir}", outdir)
                        if not os.path.exists(output_path):
                            os.makedirs(output_path, exist_ok=True)
                        
                        # Save plot
                        for ext in plot_formats:
                            c.SaveAs(os.path.join(output_path, f"{config_key}.{ext}"))
                        
                        logger.info(f"Plotted and saved: {config_key} in {output_path}")
                        c.Close()
                        
                        # Increment counter for test mode
                        total_plot_count += 1
                        individual_plot_count += 1
            
            # Break from outer loop if we hit the limit
            if break_outer:
                break
            
            if not found:
                logger.warning(f"Histogram not found: {config_key} (ROOT name: {root_hist_name})")
        
        # Summary for single-file mode
        logger.info(f"Created {individual_plot_count} individual plots and {overlay_count} overlay plots")

    logger.success("Plotting complete.")

if __name__ == "__main__":
    main()
