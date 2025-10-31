# Configuration Files Directory Structure

**Last Updated**: October 31, 2025

## Directory Organization

```
configs/
├── shared/           # Shared configuration files
│   └── Histograms.config        # Common histogram definitions
│
├── branch_maps/      # Branch mapping configurations
│   ├── BranchMapping_2018_PbPb_Data.config  # For renamed 2018 data
│   ├── BranchMap_2018_PP.config              # For 2018 PP data
│   └── BranchMap_2023.config                 # For 2023/2024 data (both PbPb & PP)
│
├── analysis/         # Analysis configuration files (gammaJetAnalyzer)
│   ├── JetSub_2018_PbPb_Data.config
│   ├── JetSub_2018_PbPb_MC.config
│   ├── JetSub_2023_PbPb_Data.config
│   ├── JetSub_2023_PbPb_MC.config
│   ├── JetSub_2024_PbPb_MC.config
│   ├── JetSub_2024_PP_Data.config
│   ├── JetSub_2024_PP_MC.config
│   ├── JetSub_2024_*_test.config             # Test configs (local paths)
│   └── ...
│
├── plotting/         # Plotting configuration files
│   ├── PlotJetSub_2018_PbPb_Data.config
│   ├── PlotJetSub_2023_PbPb_Data.config
│   ├── PlotJetSub_2023_PbPb_MC.config
│   ├── PlotJetSub_*_overlay.config           # Comparison plots
│   └── ...
│
├── unfolding/        # Unfolding configuration files
│   ├── UnfoldJetSub_2023_PbPb.config
│   ├── UnfoldJetSub_2024_PP.config
│   ├── UnfoldJetSub_template.config
│   ├── PlotUnfoldingResults_*.config
│   └── ...
│
├── skimming/         # Skimming configuration files (SkimHiForest.C)
│   ├── 2018_PbPb_QCDPhoton.config
│   ├── 2023_PbPb_Data_HirawPrime.config
│   ├── 2023_PbPb_QCDPhoton.config
│   ├── 2024_PbPb_QCDPhoton.config
│   ├── 2024_PP_Data_HardProbes.config
│   └── ...
│
├── templates/        # Template configurations (for future use)
│
└── archive_obsolete/ # Archived/obsolete configs
    ├── histParams.config                     # Old histogram params
    ├── jetSubstructure.config                # Old analysis config
    ├── JetSub_2018_*_BranchTest.config      # Test files
    └── BranchMap_2018_PbPb.config           # Duplicate (unused)
```

## Usage Guidelines

### Production Configs
- **Location**: `analysis/`, `plotting/`, `unfolding/`
- **Paths**: Use **/eos/** or **/afs/** paths for I/O
- **Use Case**: Batch jobs on lxplus/Condor

### Test Configs
- **Naming**: `*_test.config`
- **Paths**: Local paths for quick testing
- **Use Case**: Development and debugging

### Branch Mapping
- **2018 PbPb Data**: Use `branch_maps/BranchMapping_2018_PbPb_Data.config`
- **2018 PP**: Use `branch_maps/BranchMap_2018_PP.config`
- **2023/2024 (both PbPb & PP)**: Use `branch_maps/BranchMap_2023.config`

## Quick Start Examples

### Run Analysis (Production)
```bash
cd 2_SkimPlotSubstructure/
./gammaJetAnalyzer -c ../configs/analysis/JetSub_2024_PbPb_MC.config
```

### Run Analysis (Test)
```bash
./gammaJetAnalyzer -c ../configs/analysis/JetSub_2024_PbPb_MC_test.config -t 100
```

### Run Plotting
```bash
python plotGammaJet.py -c ../configs/plotting/PlotJetSub_2024_PbPb_MC.config
```

### Run Unfolding
```bash
root -l -b -q 'RooUnfoldOptimized.C("../configs/unfolding/UnfoldJetSub_2024_PP.config")'
```

## Recent Changes (October 31, 2025)

### Phase 1: Critical Fixes
- ✅ Added missing `BranchMappingFile` entries to all JetSub_* configs
- ✅ Updated weight file paths to use /afs/cern.ch
- ✅ Ensured production configs use /eos/ paths

### Phase 2: Cleanup
- ✅ Removed duplicate `BranchMap_2018_PbPb.config`
- ✅ Archived obsolete files (`histParams.config`, `jetSubstructure.config`, test files)
- ✅ Archived `*BranchTest*` and `*MinimalTest*` configs

### Phase 3: Reorganization
- ✅ Created 1-level subdirectory structure
- ✅ Moved files to appropriate subdirectories
- ✅ Updated all internal paths to new structure

## File Counts

- **Shared**: 1 file
- **Branch Maps**: 3 files
- **Analysis**: 10 files
- **Plotting**: 13 files
- **Unfolding**: 4 files
- **Skimming**: 6 files
- **Archived**: 5 files

**Total Active Configs**: 37 files (down from 47)

## Path Standards

### Production Configs Must Use:
- **InputDir**: `/eos/cms/store/...` (for centralized data) or `/eos/user/...` (for user data)
- **OutputDir**: `/eos/user/b/bharikri/www/...` (for web-accessible results)
- **HistogramConfigFile**: `/eos/user/b/bharikri/lxplus_private/run3_gamma_jet/CMSSW_13_2_13/src/HeavyIonsAnalysis/JetAnalysis/test/configs/shared/Histograms.config`
- **BranchMappingFile**: `/eos/user/b/bharikri/lxplus_private/run3_gamma_jet/CMSSW_13_2_13/src/HeavyIonsAnalysis/JetAnalysis/test/configs/branch_maps/...`
- **WeightFile_***: `/eos/user/b/bharikri/lxplus_private/run3_gamma_jet/CMSSW_13_2_13/src/HeavyIonsAnalysis/JetAnalysis/test/2_SkimPlotSubstructure/data/...`

**Note**: All production configs have been updated to use EOS paths as of October 31, 2025. This ensures compatibility with HTCondor batch jobs and direct execution on lxplus nodes.

### Test Configs Can Use:
- **InputDir**: Local paths (e.g., `/home/el9/mount_lxplus/lxplus_private/...`)
- **OutputDir**: Local paths (e.g., `output/`)
- Other paths: Can use relative or absolute local paths for rapid development/testing

**Note**: Test configs use local sshfs mount paths for development on local machines. These should not be used for production batch jobs.

## Maintenance

### Adding New Configs
1. Choose appropriate subdirectory (`analysis/`, `plotting/`, etc.)
2. Follow naming convention: `{Type}_{Year}_{System}_{DataType}.config`
3. Use absolute `/afs/cern.ch/` paths for production
4. Specify `BranchMappingFile` and `HistogramConfigFile`

### Updating Configs
1. Test changes with `*_test.config` first
2. Update production config after validation
3. Update this README if structure changes

---

For detailed analysis documentation, see `../2_SkimPlotSubstructure/DOCUMENTATION.md`
