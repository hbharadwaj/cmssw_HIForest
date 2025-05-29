# CMS Gamma-Jet Analysis: Quick Start Guide

## Overview
This framework performs photon-tagged jet substructure analysis in heavy-ion collisions. The workflow processes skimmed HiForest files to produce publication-quality physics plots.

## Prerequisites
- **CMSSW Environment:** CMSSW_13_2_13 (or compatible)
- **Input Data:** Skimmed HiForest ROOT files with flat TTree structure (from Step 1 skimming)
- **Python 3:** With ROOT/PyROOT enabled
- **Configuration Files:** Located in `../configs/` directory

## Quick Start Workflow

### 1. Setup Environment
```bash
cd 2_SkimPlotSubstructure/
cmsenv  # Ensure CMSSW environment is active
```

### 2. Compile Analysis Code
```bash
# Production compilation (recommended)
make clean && make

# Alternative: Development compilation
./compile.sh
```

### 3. Run Analysis
Process skimmed HiForest files to create analysis ROOT files:

```bash
# Basic analysis run
./gammaJetAnalyzer ../configs/2023_PbPb_QCDPhoton.config

# With specific options
./gammaJetAnalyzer ../configs/2023_PbPb_Data.config
```

**Input:** Skimmed HiForest files (flat TTree structure)  
**Output:** Analysis ROOT files with histograms and event selections

### 4. Generate Plots
Create publication-quality plots from analysis results:

```bash
# Single file plotting
python3 plotGammaJet.py \
  -r analysis_output.root \
  -c ../configs/PlotJetSub_2023_PbPb_Data.config \
  -o output_plots/ \
  --batch

# Data-MC comparison
python3 plotGammaJet.py \
  -r data_analysis.root \
  --mc-file mc_analysis.root \
  -c ../configs/DataMC_overlay.config \
  -o comparison_plots/ \
  --datamc-mode \
  --batch
```

## Configuration Files

### Analysis Configuration
- **`2023_PbPb_QCDPhoton.config`** - MC photon analysis
- **`2023_PbPb_Data.config`** - Data analysis settings
- **Key parameters:** Photon/jet cuts, centrality bins, jet collections

### Plotting Configuration  
- **`PlotJetSub_2023_PbPb_Data.config`** - Single-file plotting
- **`DataMC_overlay.config`** - Data-MC comparison plots
- **Features:** CMS styling, overlays, output formats

## Common Tasks

### Modify Selection Cuts
Edit the analysis config file:
```bash
# Example cuts in config file
PhotonPtMin: 60.0
PhotonEtaMax: 1.44
JetPtMin: 40.0
DeltaPhiMin: 2.094  # 2π/3
```

### Change Jet Collections
Specify jet algorithms and grooming parameters:
```bash
JetCollections: AK4Z1,AK4Z2,AK8Z2,AK8Z4
```

### Customize Plots
Modify plotting config for:
- Colors and markers (`ColorScheme: petroff6`)
- Output formats (`PlotFormats: pdf,png,root`)
- CMS labeling (`CMSLabel: Preliminary`)
- Overlay options (`OverlayPlots: JetPt,JetEta,DeltaPhi`)

## Output Structure
```
output_directory/
├── 1D/          # Individual 1D histograms
├── 2D/          # 2D correlation plots  
├── overlays/    # Multi-histogram overlays
├── datamc/      # Data-MC comparisons (if enabled)
└── AK4Z2/       # Jet-collection specific plots
    ├── cent0to60/
    └── cent60to180/
```

## Troubleshooting

### Compilation Issues
```bash
# Check ROOT and CMSSW setup
echo $ROOTSYS
echo $CMSSW_VERSION

# Clean rebuild
make clean && make
```

### Missing Plots
- Verify ROOT file structure matches config settings
- Check `UseNestedStructure` setting in config
- Use `--verbose` flag for debugging info

### Input File Issues
- Ensure skimmed HiForest files have flat TTree structure
- Verify file paths in configuration
- Check that required branches exist in input files

## Advanced Usage

### Batch Processing
```bash
# Submit production jobs (if scripts are available)
./submit_production.sh config_file.config

# Monitor job status  
./monitor_jobs.sh
```

### Development and Testing
```bash
# Test specific plots
python3 plotGammaJet.py -r file.root -c config.config --test --datamc-plots JetPt,DeltaPhi

# Validate configuration
python3 test_datamc_config.py config.config
```

## Documentation
- **Technical Details:** See `DOCUMENTATION.md` for comprehensive framework documentation
- **Configuration Reference:** All available config options with examples
- **Function Documentation:** Complete `plotGammaJet.py` API reference
- **Development History:** Implementation notes and lessons learned

## Support Files
- **Essential:** `gammaJetAnalyzer.cpp`, `plotGammaJet.py`, `Makefile`
- **Utilities:** `test_datamc_config.py`, `run_plotting.sh`
- **Templates:** `scripts/` directory contains development templates for future features