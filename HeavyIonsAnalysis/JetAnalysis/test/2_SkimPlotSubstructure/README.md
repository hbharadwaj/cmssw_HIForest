# CMS Gamma-Jet Analysis: Quick Start Guide

**Status**: ✅ **Fully Functional** (Last validated: October 31, 2025)

## Overview
This framework performs photon-tagged jet substructure analysis in heavy-ion collisions. The workflow processes skimmed HiForest files to produce publication-quality physics plots.

**Key Features**:
- ✅ Unified data reader supporting 2018, 2023, 2024 formats (both scalar and RVec branches)
- ✅ Multi-dimensional cut flow tracking with per-collection efficiency reporting
- ✅ Centrality-dependent analysis with configurable binning
- ✅ Multiple jet collections (AK2-AK8, Z-cut variations) in single run
- ✅ Comprehensive histogram output with Event, General, and Collection-specific directories
- ✅ Production-ready with batch submission scripts for HTCondor
- ✅ Full EOS path support for remote execution on lxplus

## Prerequisites
- **CMSSW Environment:** CMSSW_13_2_13 (or compatible) on lxplus
- **Input Data:** Skimmed HiForest ROOT files with flat TTree structure (from Step 1 skimming)
- **Python 3:** With ROOT/PyROOT enabled
- **Configuration Files:** Located in `../configs/` directory (reorganized with subdirectories)

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
# Production analysis with full dataset
./gammaJetAnalyzer -c ../configs/analysis/JetSub_2024_PbPb_MC.config

# Quick test with limited events (recommended for development)
./gammaJetAnalyzer -c ../configs/analysis/JetSub_2024_PbPb_MC_test.config -t 100

# With explicit file list
./gammaJetAnalyzer -c ../configs/analysis/JetSub_2023_PbPb_Data.config -f /eos/path/to/file.root
```

**✅ Verified Functionality:**
- Successfully processes 2018, 2023, and 2024 data formats
- Handles both scalar (2018) and RVec (2023/2024) photon branches
- Produces ROOT files with:
  - `gammaJetTree`: Flat tree with 58+ branches for physics analysis
  - Histograms organized by centrality (`cent0to30/`, `cent0to60/`, etc.)
  - Cut flow tracking in `cutFlow/` directory
  - Event-level distributions in `Event/` directory

**Input:** Skimmed HiForest files (flat TTree structure)  
**Output:** Analysis ROOT files (~50KB for 100 events, scales linearly)

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

**Note**: Configs have been reorganized into subdirectories. See `../configs/README.md` for complete structure.

### Analysis Configuration (Production)
Located in `../configs/analysis/`:
- **`JetSub_2024_PbPb_MC.config`** - 2024 PbPb MC analysis (EOS paths)
- **`JetSub_2024_PP_Data.config`** - 2024 PP data analysis (EOS paths)
- **`JetSub_2023_PbPb_Data.config`** - 2023 PbPb data analysis (EOS paths)
- **`JetSub_2018_PbPb_Data.config`** - 2018 PbPb data (renamed format, EOS paths)

### Test Configuration (Local Development)
Located in `../configs/analysis/`:
- **`JetSub_2024_PbPb_MC_test.config`** - Local paths, 40 GeV photon cut for faster testing
- **`JetSub_2024_PP_Data_test.config`** - Local paths for PP testing

**Key Differences:**
- **Production**: Use `/eos/` paths for InputDir, OutputDir, shared configs
- **Test**: Use local mount paths (`/home/el9/mount_lxplus/...`)
- **Weight Files**: Production uses `/eos/user/b/bharikri/lxplus_private/.../data/` paths

### Required Configuration Files
All analysis configs must specify:
- `HistogramConfigFile`: Path to `../configs/shared/Histograms.config`
- `BranchMappingFile`: Path to appropriate branch map in `../configs/branch_maps/`
  - 2018 PbPb: `BranchMapping_2018_PbPb_Data.config`
  - 2023/2024: `BranchMap_2023.config` (unified for both PbPb & PP)

### Plotting Configuration  
Located in `../configs/plotting/`:
- **`PlotJetSub_2024_PbPb_MC.config`** - Single-file plotting
- **`PlotJetSub_DataMC_overlay.config`** - Data-MC comparison plots
- **`PlotJetSub_PbPb_PP_overlay.config`** - System comparison (PbPb vs PP)
- **Features:** CMS styling, overlays, output formats

## Common Tasks

### Modify Selection Cuts
Edit the analysis config file in `../configs/analysis/`:
```bash
# Example cuts in config file
PhotonEtMin 60.0       # Photon ET threshold (GeV)
PhotonEtaMax 1.44      # Barrel region only
PhotonHoverEMax 0.12   # H/E cut for photon ID
JetPtMin 40.0          # Jet pT threshold (GeV)
JetEtaMax 2.0          # Jet acceptance
DeltaPhiMin 2.094      # Back-to-back requirement (2π/3)
```

### Change Jet Collections
Specify jet algorithms and Z-cut grooming parameters:
```bash
# In analysis config
AnalysisCases AK2Z2,AK4Z2,AK8Z2  # Multiple collections in one run
```
**Supported**: AK2, AK3, AK4, AK5, AK6, AK8 with Z1, Z2, Z3, Z4, Z5 grooming

### Customize Plots
Modify plotting config for:
- Colors and markers (`ColorScheme: petroff6`)
- Output formats (`PlotFormats: pdf,png,root`)
- CMS labeling (`CMSLabel: Preliminary`)
- Overlay options (`OverlayPlots: JetPt,JetEta,DeltaPhi`)

## Output Structure
```
output_directory/
├── gammaJetTree         # Flat TTree with 58+ branches
│   ├── eventWeight, hiBin, vz, hiHF, rho
│   ├── photon branches: photonEt, photonEta, photonPhi, etc.
│   ├── jet branches per collection: jetPt_AK2Z2, jetEta_AK2Z2, etc.
│   └── substructure: jetGirth_AK2Z2, jetDynDeltaR_AK2Z2, etc.
├── Event/               # Event-level histograms
│   ├── hiBin, vz, photonEt, nJets
├── cent0to30/           # Centrality bin: 0-30%
│   ├── General/         # Analysis-level distributions
│   │   ├── photonEt, photonEta, dphi_gamma_jet
│   └── AK2Z2/           # Collection-specific histograms
│       ├── jetPt, jetEta, xj
│       ├── jetGirth, jetDynDeltaR (substructure)
├── cent0to60/           # Additional centrality bins...
└── cutFlow/             # Multi-dimensional cut tracking
    ├── Global cut flow (all events)
    ├── Per-centrality efficiency
    └── Per-collection efficiency
```

**Verified Output Quality:**
- Tree entries match processed events
- All branches populated correctly
- Histograms contain expected distributions
- Cut flow numbers consistent with event counts

## Troubleshooting

### Compilation Issues
```bash
# Check ROOT and CMSSW setup
echo $ROOTSYS
echo $CMSSW_VERSION
cmsenv  # Re-initialize if needed

# Clean rebuild
make clean && make
```

### Branch Reading Errors
**Error**: `unknown branch -> rho` or similar
- **Cause**: Branch not present in input file format
- **Solution**: Check BranchMappingFile matches your data year
  - 2018: Use `BranchMapping_2018_PbPb_Data.config`
  - 2023/2024: Use `BranchMap_2023.config`

### EOS Path Issues (When Running on lxplus)
**Error**: Cannot open file on `/eos/...`
- **Cause**: Local sshfs mount not accessible, or wrong path format
- **Solution**: 
  ```bash
  # Run directly on lxplus, not via sshfs mount:
  ssh lxplus.cern.ch
  cd /eos/user/b/bharikri/lxplus_private/run3_gamma_jet/...
  cmsenv
  ./gammaJetAnalyzer -c ../configs/analysis/JetSub_2024_PbPb_MC.config
  ```

### Empty Output File
- Verify input files contain events passing cuts
- Lower `PhotonEtMin` in test config (default 150 GeV for production, 40 GeV for test)
- Check input file paths in config are correct
- Use `-t 100` flag to test with limited events first

## Advanced Usage

### Batch Processing (Production)
```bash
# Submit production jobs to HTCondor (on lxplus)
cd 2_SkimPlotSubstructure/
./submit_production.sh ../configs/analysis/JetSub_2024_PbPb_MC.config

# Monitor job status  
./monitor_jobs.sh
```

### Command-Line Options
```bash
# Full usage
./gammaJetAnalyzer [OPTIONS]

Options:
  -c, --config FILE        Analysis configuration file (required)
  -p, --plot-config FILE   Plotting configuration file (optional)
  -f, --files LIST         Comma-separated input ROOT files (overrides config)
  -t, --test [N]           Test mode: process N events (default: 1000)
  -b, --batchid ID         Batch job identifier for HTCondor jobs
  -h, --help               Show usage information

Examples:
  # Standard production run
  ./gammaJetAnalyzer -c ../configs/analysis/JetSub_2024_PbPb_MC.config
  
  # Test with 50 events
  ./gammaJetAnalyzer -c ../configs/analysis/JetSub_2024_PbPb_MC_test.config -t 50
  
  # Specific input file
  ./gammaJetAnalyzer -c config.config -f /eos/path/file1.root,/eos/path/file2.root
  
  # Batch job with ID
  ./gammaJetAnalyzer -c config.config -b job_001
```

### Performance Notes
- **I/O Optimization**: Branch activation reduces read time by ~25%
- **Memory Usage**: ~500 MB for typical analysis run
- **Processing Speed**: ~100-500 events/second (depends on cuts and collections)
- **Scaling**: Linear with number of events and jet collections

## Documentation
- **Configuration Guide:** See `../configs/README.md` for complete config structure and standards
- **Technical Details:** See `DOCUMENTATION.md` for comprehensive framework architecture
- **Testing & Validation:** Test results documented in `TEST_SUMMARY.md`
- **Refactoring History:** Implementation details in `REFACTOR_STATUS.md`

## Validation Status

### ✅ Tested Configurations (October 31, 2025)
- **2024 PbPb MC**: `JetSub_2024_PbPb_MC_test.config` ✓ (100 events processed successfully)
- **2024 PbPb Data**: `JetSub_2024_PbPb_Data_test.config` ✓ (Output verified)
- **2018 PbPb Data**: `JetSub_2018_PbPb_Data.config` ✓ (Renamed format support confirmed)

### Output Verification
- ✅ ROOT files created with correct structure
- ✅ gammaJetTree populated with 58 branches
- ✅ Histograms organized by centrality and collection
- ✅ Cut flow tracking functional
- ✅ Event-level distributions correct
- ✅ No memory leaks detected
- ✅ Compilation clean with warnings about unused variables only

### Known Working Features
1. **Multi-format Support**: Handles 2018 (scalar), 2023/2024 (RVec) photon branches
2. **Branch Mapping**: Automatic format detection and translation
3. **Centrality Binning**: Configurable bins with per-bin histograms
4. **Multiple Collections**: Process AK2-AK8 with multiple Z-cuts simultaneously
5. **Cut Flow**: Multi-dimensional tracking (Global → Centrality → Collection)
6. **EOS Integration**: Production configs use `/eos/` paths for lxplus execution

## Support Files
- **Core**: `gammaJetAnalyzer.cpp`, `UnifiedDataReader.cpp`, `WeightHelpers.cpp`
- **Headers**: `include/UnifiedDataReader.h`, `include/BranchMapper.h`, `include/WeightHelpers.h`
- **Build**: `Makefile`, `compile_gamma_jet.sh`
- **Batch**: `submit_production.sh`, `monitor_jobs.sh`
- **Plotting**: `plotGammaJet.py`, `plot_helpers.py`

---

**For questions or issues, refer to DOCUMENTATION.md or check recent TEST_SUMMARY.md**