# Photon-Jet Analysis Framework

## Directory Structure
```
HeavyIonsAnalysis/JetAnalysis/test/
├── input/                   # Common configuration files
│   ├── RDF.config          # Skimming configurations
│   ├── jetSubstructure.config  # Analysis parameters
│   └── HistParam.config    # Histogram and plotting settings
│
├── 1_Skimming/             # First stage: Skimming
│   └── SkimWithRDF.C      # RDataFrame based skimming
│
└── 2_Analysis/             # Second stage: Analysis
    ├── include/           # Header files
    │   ├── OutputTreeData.h
    │   ├── PhotonJetAnalysis.h
    │   └── TreeInspector.h
    │
    ├── src/              # Implementation files
    │   ├── OutputTreeData.C
    │   ├── PhotonJetAnalysis.C
    │   └── TreeInspector.C
    │
    ├── analyzeJetTree.C  # Main analysis macro
    └── README.md         # Analysis documentation
```

## Workflow

### Step 1: Skimming
Create skimmed trees with selected branches:
```bash
cd HeavyIonsAnalysis/JetAnalysis/test/1_Skimming
root -l -b -q 'SkimWithRDF.C("../input/RDF.config")'
```

### Step 2: Analysis
Process skimmed trees and create outputs:
```bash
cd ../2_Analysis
root -l
.L analyzeJetTree.C+
analyzeJetTree("../input/jetSubstructure.config")
```

## Configuration Files

### /input/RDF.config
- Input/output paths
- Branch selection for each tree
- Tree aliases and prefixes
- Jet collections configuration

### /input/jetSubstructure.config
- Analysis parameters
- Selection cuts (photon, jet, electron)
- MC/data settings
- Path to histogram config

### /input/HistParam.config
- Histogram definitions
- CMS plot style settings
- Canvas properties
- Color schemes

## Output Structure
```
/eos/user/b/bharikri/CERNBox_synced/Projects/HeavyIon/Run3GammaJet/
└── output/
    ├── skimmed/           # Step 1 output
    │   └── jet_tree.root
    └── analysis/          # Step 2 output
        ├── histograms/
        └── plots/
```

## Requirements
- CMSSW_13_2_13
- ROOT 6.x
- Valid grid certificate
- EOS access

## Quick Start
1. Set up CMSSW environment:
```bash
cmsenv
```

2. Run skimming:
```bash
cd 1_Skimming
root -l -b -q 'SkimWithRDF.C("../input/RDF.config")'
```

3. Run analysis:
```bash
cd ../2_Analysis
root -l
.L analyzeJetTree.C+
analyzeJetTree("../input/jetSubstructure.config")
```

## Data Output
1. Skimmed trees with:
   - Selected branches
   - Applied aliases
   - Proper friend structure

2. Analysis output:
   - Selected events
   - Histogram ROOT file
   - CMS-style plots
   - MC truth matching (if MC)

## Contact
Bharadwaj Harikrishnan (bharadwaj.harikrishnan@cern.ch)
