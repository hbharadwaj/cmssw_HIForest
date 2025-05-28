# CMS Gamma-Jet Analysis Framework: Quick Start

## Requirements
- CMSSW 13.x environment (for C++ analysis)
- Python 3 (for plotting)
- ROOT (PyROOT enabled)

## 1. Data Processing: Event Selection & ROOT File Creation

Run the main C++ analyzer to process data/MC and produce ROOT files:

```bash
cd /path/to/CMSSW_13_2_13/src/HeavyIonsAnalysis/JetAnalysis/test/2_SkimPlotSubstructure
cmsenv
make  # if needed
./gammaJetAnalyzer -c ../configs/JetSub_2023_PbPb_Data.config -p ../configs/PlotJetSub_2023_PbPb_Data.config -t 1000
```
- Edit the config files as needed for your dataset and selection.
- Output: ROOT file with all selected events and histograms.

## 2. Plotting: Publication-Quality Plots

Use the Python script to generate CMS-style plots from the ROOT output:

```bash
python3 plotGammaJet.py -r /path/to/output.root -c ../configs/DataMC_overlay.config -o /path/to/plots --batch
```
- `-r`: Input ROOT file
- `-c`: Plotting config (see `configs/DataMC_overlay.config` for examples)
- `-o`: Output directory for plots
- `--batch`: Run without GUI (recommended for batch jobs)

### Common Options
- Overlay plots, ratio plots, and Data/MC comparisons are controlled via the config file.
- Output formats: PDF, PNG, ROOT (set in config)
- Jet collections and centrality bins are auto-detected or set in config.

## 3. Customization
- Edit config files to change selection, plot appearance, overlays, and output structure.
- See `DOCUMENTATION.md` for project history and advanced usage.

## Troubleshooting
- Ensure your environment has PyROOT and all dependencies.
- Check config file paths and ROOT file structure if plots are missing.
- For advanced debugging, use the `--verbose` flag.

---
For full documentation and development history, see `DOCUMENTATION.md`.