# Photon-Jet Analysis for Heavy Ion Collisions

This directory contains scripts and tools for analyzing photon-jet correlations in heavy ion collisions, focusing on 2023/2024 PbPb and 2024 ppRef data.

## Quick Start

1. **Run the basic photon-jet analysis:**
   ```bash
   cd scripts
   root -l 'photonJet.C("../../configs/photon_only.config")'
   ```

2. **Run in test mode with limited events:**
   ```bash
   root -l 'photonJet.C("../../configs/photon_only.config", "../../configs/histParams.config", true, 1000)'
   ```

Note: The configuration files are located in the `/afs/cern.ch/user/b/bharikri/private/HeavyIon/run3_gamma_jet/CMSSW_13_2_13/src/HeavyIonsAnalysis/JetAnalysis/test/configs/` directory.

3. **Run from command line with options:**
   ```bash
   root -l 'photonJet.C("../../configs/photon_only.config", "", false)'
   ```

## Command Line Options

The script supports the following command line arguments when compiled:
```bash
root -l 'photonJet.C+' -- [options]
```

Available options:
- `--production`, `-p`: Run in production mode (all events)
- `--test`, `-t [n]`: Run in test mode with n events (default: 10,000)
- `--config`, `-c FILE`: Specify config file
- `--hist`, `-h FILE`: Specify histogram config file

## Configuration

The analysis uses TEnv-based configuration files:

- **photon_only.config**: Basic photon selection configuration
- **histParams.config**: Histogram parameters configuration

Example configuration:
```
# System settings
System 2023_PbPb
DataType MC
InputDir /path/to/input/files
OutputDir output/2023_PbPb/MC

# Photon selection
PhotonEtMin 30.0
PhotonEtaMax 1.44
PhotonHoverEMax 0.119947
PhotonSigmaIEtaIEtaMax 0.010392
PhotonIsoMax 2.099277
PhotonR9Min 0.8

# Event selection
VzCut 15.0
HiHFCutMin 0.0
HiHFCutMax 7000.0
```

## Script Components

- **scripts/photonJet.C**: Main analysis script for photon-jet correlations
- **include/helpers.h**: Common helper functions and configuration structures
- **include/GammaJetAnalysis.h**: Base class for photon-jet analysis
- **include/GammaJet2023_PbPbMC.h**: Implementation for 2023 PbPb MC data

## Implementation Features

The current implementation includes:

1. **Dynamic Branch Management**
   - Handles different input file structures
   - Auto-detects system-specific branches
   - Conditional MC truth handling

2. **Configuration System**
   - TEnv-based parameter management
   - System detection from input files
   - MC/Data specific settings

3. **Output Organization**
   - Structured directory creation
   - Histogram generation
   - Event tree output

4. **Photon Selection**
   - Basic kinematic cuts (ET, eta)
   - ID variable selection (HoverE, SigmaIEtaIEta, R9)
   - Configurable isolation requirements

## Current Status

The photonJet.C script implements:
- Basic photon selection
- Event filtering
- Simple histogramming
- ROOT file output with histograms and trees

For full implementation details, see [DOCUMENTATION.md](DOCUMENTATION.md).

## Next Steps

See the [Implementation Plan](DOCUMENTATION.md) for upcoming development tasks:
1. Complete photon isolation definition
2. Add jet quality criteria
3. Implement validation framework
4. Set up production system for full dataset processing