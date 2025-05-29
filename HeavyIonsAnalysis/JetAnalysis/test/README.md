# Photon-Jet Analysis Framework

## Directory Structure
```
HeavyIonsAnalysis/JetAnalysis/test/
├── configs/                # Primary configuration files
│   ├── 2023_PbPb_QCDPhoton.config        # Skimming configurations
│   ├── JetSub_2023_PbPb_Data.config      # Analysis parameters  
│   ├── JetSub_2023_PbPb_MC.config        # MC analysis parameters
│   ├── histParams.config                 # Histogram settings
│   └── plotting_config.config            # Plotting configurations
│
├── 1_Skimming/            # First stage: Skimming
│   ├── SkimHiForest.C     # Primary skimming script (C++)
│   ├── SkimHiForest       # Compiled executable
│   ├── Makefile           # Build configuration
│   ├── submit_condor_jobs.sh    # Batch job submission
│   ├── merge_batch_outputs.sh   # Output merging
│   └── batch/             # Batch processing scripts
│
└── 2_SkimPlotSubstructure/ # Second stage: Analysis & Plotting
    ├── gammaJetAnalyzer.cpp    # Main analysis program
    ├── Makefile                # Build configuration
    ├── compile.sh              # Compilation script
    ├── submit_production.sh    # Production job submission
    ├── include/                # Header files
    ├── scripts/                # Analysis and plotting scripts
    └── batch/                  # Batch processing scripts
```

## Workflow

### Step 1: Skimming
Compile and run the skimming program with configuration:
```bash
cd HeavyIonsAnalysis/JetAnalysis/test/1_Skimming
make                    # Compile SkimHiForest.C → SkimHiForest executable
./SkimHiForest ../configs/2023_PbPb_QCDPhoton.config
```

### Step 2: Analysis & Plotting
Compile and run the analysis program:
```bash
cd ../2_SkimPlotSubstructure
./compile.sh            # Compile gammaJetAnalyzer.cpp
./gammaJetAnalyzer ../configs/JetSub_2023_PbPb_MC.config
```

## Configuration Files

### /configs/2023_PbPb_QCDPhoton.config
- Input/output paths for skimming
- Branch selection for each tree
- Tree aliases and prefixes  
- Jet collections configuration
- Batch processing settings

### /configs/JetSub_2023_PbPb_MC.config
- Analysis parameters for MC data
- Selection cuts (photon, jet, electron)
- MC-specific settings
- Path to histogram config

### /configs/JetSub_2023_PbPb_Data.config
- Analysis parameters for real data
- Selection cuts (photon, jet, electron)
- Data-specific settings
- Trigger requirements

### /configs/histParams.config
- Histogram definitions
- CMS plot style settings
- Canvas properties
- Color schemes

### /configs/plotting_config.config
- Plotting parameters
- Style configurations
- Output formatting

### /configs/PlotJetSub_2023_PbPb_MC.config
- MC-specific plotting configuration
- Histogram definitions and styling
- Canvas and legend settings
- CMS style parameters

### /configs/PlotJetSub_2023_PbPb_Data.config
- Data-specific plotting configuration
- Histogram definitions and styling
- Canvas and legend settings
- CMS style parameters

### /configs/DataMC_overlay.config
- Data-MC comparison plotting
- Overlay plot configurations
- Ratio plot settings
- Combined analysis parameters

## Output Structure
```
/eos/cms/store/group/phys_heavyions/bharikri/Run3GammaJet/2025_05_26/2023_PbPb/
├── MC/                    # Step 1 output (Skimmed MC)
│   └── 2025_05_26_QCDPhoton30_skimmed_*.root
│── Data/                  # Step 1 output (Skimmed Data)
/eos/user/b/bharikri/www/Run3GammaJet/     # Step 2 output
    ├── histograms/        # Analysis histograms
    └── plots/             # Generated plots and figures
        ├── MC/            # MC-specific plots
        ├── Data/          # Data-specific plots
        └── DataMC_comparison/  # Overlay comparisons
```

## Requirements
- CMSSW_13_2_13
- ROOT 6.x
- Valid grid certificate
- EOS access

## Build System & Batch Processing

### Compilation
Both stages use dedicated build systems:
- **Stage 1**: Uses `Makefile` to compile `SkimHiForest.C` → `SkimHiForest` executable
- **Stage 2**: Uses `compile.sh` script to build `gammaJetAnalyzer.cpp`

### Batch Processing Capabilities
**1_Skimming/** includes advanced batch processing:
```bash
# Submit batch jobs to Condor
./submit_condor_jobs.sh ../configs/2023_PbPb_QCDPhoton.config

# Merge batch outputs
./merge_batch_outputs.sh
```

**2_SkimPlotSubstructure/** includes production job submission:
```bash
# Submit analysis production jobs
./submit_production.sh ../configs/JetSub_2023_PbPb_MC.config
```

### Configuration Features
- **FilesPerOutput**: Process multiple input files per output file
- **BatchMode**: Enable batch processing mode
- **Progress Monitoring**: Built-in progress tracking
- **Directory Auto-creation**: Automatic output directory setup

## Quick Start
1. Set up CMSSW environment:
```bash
cmsenv
```

2. Run skimming:
```bash
cd 1_Skimming
make                    # Compile SkimHiForest.C → SkimHiForest executable
./SkimHiForest ../configs/2023_PbPb_QCDPhoton.config
```

3. Run analysis:
```bash
cd ../2_SkimPlotSubstructure
./compile.sh            # Compile gammaJetAnalyzer.cpp
./gammaJetAnalyzer ../configs/JetSub_2023_PbPb_MC.config
```

## Usage Examples

### Processing Specific Batch
```bash
# Process only batch 5 of the skimming
./SkimHiForest ../configs/2023_PbPb_QCDPhoton.config 5
```

### Data vs MC Analysis
```bash
# Process MC data
./gammaJetAnalyzer ../configs/JetSub_2023_PbPb_MC.config

# Process real data
./gammaJetAnalyzer ../configs/JetSub_2023_PbPb_Data.config
```

### Plotting and Comparison
```bash
# Generate MC-specific plots
./plotting_script ../configs/PlotJetSub_2023_PbPb_MC.config

# Generate Data-MC comparison plots
./plotting_script ../configs/DataMC_overlay.config
```

## Data Output
1. Skimmed trees with:
   - Selected branches from multiple ROOT trees
   - Applied aliases and tree friendships
   - Proper friend structure for analysis
   - Batch processing with configurable files per output

2. Analysis output:
   - Selected events based on physics cuts
   - Comprehensive histogram ROOT files
   - CMS-style plots and figures
   - MC truth matching (if MC data)
   - Jet substructure observables
   - Photon-jet correlations

## Advanced Features

### Skimming Stage (SkimHiForest.C)
- **Multi-tree Processing**: Handles multiple ROOT trees with proper friendships
- **Selective Branch Reading**: Only processes configured branches for efficiency
- **Batch File Management**: Configurable files per output batch
- **Progress Monitoring**: Real-time progress tracking with timing
- **Memory Optimization**: Smart memory management for large datasets
- **Error Handling**: Comprehensive error checking and reporting

### Analysis Stage (gammaJetAnalyzer.cpp)
- **Physics Selection**: Photon, jet, and event-level cuts
- **Substructure Analysis**: Jet substructure observables (dynamical grooming, girth, thrust)
- **Correlation Studies**: Photon-jet angular and momentum correlations
- **MC Truth Matching**: Generator-level particle matching for MC
- **Multi-centrality Support**: Centrality-dependent analysis for heavy-ion data
- **Systematic Studies**: Support for systematic uncertainty evaluation

## Detailed Documentation

For comprehensive technical details, see:
- **Skimming Stage**: [`1_Skimming/DOCUMENTATION.md`](1_Skimming/DOCUMENTATION.md) - Complete technical reference for HiForest processing
- **Analysis Stage**: [`2_SkimPlotSubstructure/DOCUMENTATION.md`](2_SkimPlotSubstructure/DOCUMENTATION.md) - Detailed analysis framework documentation
- **Quick Start**: [`2_SkimPlotSubstructure/README.md`](2_SkimPlotSubstructure/README.md) - Analysis stage quick reference

## Troubleshooting

Common issues and solutions:
1. **Compilation errors**: Ensure CMSSW environment is properly set up with `cmsenv`
2. **Missing config files**: Check that all paths in config files are accessible
3. **Batch job failures**: Verify EOS access and grid certificate validity
4. **Empty output**: Check input file paths and branch selection in configs

## Contact
Bharadwaj Harikrishnan (bharadwaj.harikrishnan@cern.ch)
