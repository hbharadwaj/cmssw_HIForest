# Photon-Tagged Jet Substructure Analysis: Implementation Plan

## Overview
Analysis of photon-jet correlations in heavy ion collisions for:
- 2023 PbPb (Data/MC)
- 2024 PbPb (Data/MC)
- 2024 ppRef (Data/MC)

## Key Features

### 1. Dynamic Branch Management
- Automatic detection of input file structure
- System-specific branch handling
- Conditional MC truth handling
- Branch type updates without code changes

### 2. Configuration System (TEnv)
```
# System settings
System 2023_PbPb
DataType MC
InputDir /path/to/input
OutputDir /path/to/output

# Selection criteria
PhotonEtMin 30.0
PhotonEtaMax 1.44
...
```

### 3. Output Organization
```
output/
├── 2023_PbPb/
│   ├── Data/
│   │   ├── plots/      # PNG/PDF files
│   │   └── root/       # Histograms & trees
│   └── MC/
│       ├── plots/
│       └── root/
└── 2024_PbPb/
    ├── Data/
    └── MC/
```

## Implementation Phases

### Phase 1: Foundation
1. **Input Processing**
   - [x] Basic file reading
   - [x] Branch structure detection
   - [x] System & data type detection
   - [x] Dynamic class generation

2. **Configuration**
   - [x] TEnv-based config reading
   - [x] Basic selection parameters
   - [x] System-specific parameters

3. **Output Setup**
   - [x] Directory structure
   - [x] Basic histograms
   - [ ] Automated plot generation

### Phase 2: Analysis Core
1. **Branch Management**
   ```cpp
   // Function to check/update class files
   bool CheckAndUpdateClass(const string& inputFile,
                          const string& system,
                          bool isMC) {
       TFile* f = TFile::Open(inputFile.c_str());
       if (!f || f->IsZombie()) {
           cerr << "Error: Could not open file " << inputFile << endl;
           return false;
       }

       TTree* t = (TTree*)f->Get("jet_tree");
       if (!t) {
           cerr << "Error: Could not find jet_tree in " << inputFile << endl;
           f->Close();
           return false;
       }
       
       // Check if update needed
       string classFile = Form("GammaJet_%s_%s.h",
                             system.c_str(),
                             isMC ? "MC" : "Data");
       
       if (needsUpdate(t, classFile)) {
           // Set branch status before generating class
           setBranchStatus(t, system, isMC);
           
           // Generate class with error handling
           if (t->MakeClass(classFile.c_str()) != 0) {
               cerr << "Error: Failed to generate class " << classFile << endl;
               f->Close();
               return false;
           }
           
           // Move files to correct location
           string cmd = Form("mv %s* ../include/", classFile.c_str());
           if (gSystem->Exec(cmd.c_str()) != 0) {
               cerr << "Error: Failed to move generated files" << endl;
               f->Close();
               return false;
           }
           
           f->Close();
           return true;
       }
       
       f->Close();
       return false;
   }

   // Helper function to set branch status
   void setBranchStatus(TTree* t, const string& system, bool isMC) {
       // Deactivate all branches first
       t->SetBranchStatus("*", 0);
       
       // Activate common branches
       vector<string> commonBranches = {
           "photon_pt", "photon_eta", "photon_phi",
           "jet_pt", "jet_eta", "jet_phi"
       };
       for (const auto& branch : commonBranches) {
           t->SetBranchStatus(branch.c_str(), 1);
       }
       
       // System-specific branches
       if (system == "2023_PbPb") {
           t->SetBranchStatus("centrality", 1);
           t->SetBranchStatus("vtx", 1);
       }
       
       // MC truth branches
       if (isMC) {
           t->SetBranchStatus("genPhoton_*", 1);
           t->SetBranchStatus("genJet_*", 1);
       }
   }
   ```

2. **Event Processing**
   - [x] Branch activation system
   - [x] MC truth handling
   - [ ] Selection implementation

3. **Validation**
   - [ ] Cut flow monitoring
   - [ ] Basic plots
   - [ ] MC closure tests

## Implementation Details

### 1. Branch Structure Detection
The system now automatically detects:
- File structure and available branches
- System-specific branches (2023PbPb, 2024PbPb, 2024ppRef)
- MC truth information availability
- Array branch types for RVec conversion

### 2. Class Generation System
Improvements made:
- Robust error handling
- Automatic file placement
- Branch status management
- System-specific customization

### 3. Configuration Management
Enhanced features:
- System detection from input files
- Automatic parameter selection
- MC/Data specific settings
- Branch activation rules

### 4. Analysis Variables and Selection

#### Photon Variables
| Variable Type | Variable Names | Description |
|---------------|----------------|-------------|
| Basic Properties | `ggHi_phoEt`, `ggHi_phoEta`, `ggHi_phoPhi` | Basic kinematics |
| ID Variables | `ggHi_phoHoverE`, `ggHi_phoSigmaIEtaIEta`, `ggHi_phoR9`, `ggHi_phoHasPixelSeed` | Shower shape and conversion rejection |
| ECAL Isolation | `ggHi_pho_ecalClusterIsoR1` through `R5` | ECAL energy in isolation cone (R=0.1-0.5) |
| HCAL Isolation | `ggHi_pho_hcalRechitIsoR1` through `R5` | HCAL energy in isolation cone (R=0.1-0.5) |
| PF Isolation | `ggHi_pfcIso1` through `ggHi_pfcIso5` | Charged PF isolation in different cone sizes |
| | `ggHi_pfpIso1` through `ggHi_pfpIso5` | Photon PF isolation in different cone sizes |
| | `ggHi_pfnIso1` through `ggHi_pfnIso5` | Neutral hadron PF isolation in different cone sizes |
| | `ggHi_pfpIso1subSC` through `ggHi_pfpIso5subSC` | Photon PF isolation with SC energy subtracted |

#### System-Specific Selection
For 2023 PbPb, the following selections are applied:
- `PhotonEtMin = 30.0 GeV`
- `PhotonEtaMax = 1.44` (barrel only)
- `PhotonHoverEMax = 0.119947`
- `PhotonSigmaIEtaIEtaMax = 0.010392`
- `PhotonIsoMax = 2.099277` (needs to be defined which isolation variable to use)
- `PhotonR9Min = 0.8`

The default isolation calculation should be modified to use one of:
1. Sum of ECAL+HCAL+Track isolation: `ggHi_pho_ecalClusterIsoR3 + ggHi_pho_hcalRechitIsoR3 + [track isolation]`
2. PF isolation sum: `ggHi_pfcIso3 + ggHi_pfnIso3 + ggHi_pfpIso3subSC`

## Current Implementation Details

### Current Script Implementation
The main analysis script `scripts/photonJet.C` implements:

1. **Configuration System**
   - Uses the Config structure defined in `helpers.h`
   - Loads parameters from TEnv configuration files
   - Supports centrality bins from both config files and environment variables
   - Provides command-line options for test/production modes

2. **File Handling**
   - Recursively collects ROOT files from input directory
   - Supports both test mode (first file only) and production mode (all files)
   - Uses TChain for efficient processing of multiple files

3. **Analysis Class Structure**
   - Uses a base `GammaJetAnalysis` class for common functionality
   - System-specific implementations (currently only `GammaJet2023_PbPbMC`)
   - Factory pattern with `createAnalyzer()` function for class selection

4. **Photon Selection**
   - Basic kinematic selection (ET, eta)
   - ID variable selection (HoverE, SigmaIEtaIEta, R9)
   - Simple ECAL isolation implementation

5. **Output Handling**
   - Creates histograms for photon properties
   - Produces output ROOT file with histograms and selected event tree
   - Applies directory structure based on system and data type

### Running the Script

#### Configuration Files
The script uses two main config files from the `/configs` directory:
- `photon_only.config`: System settings and selection parameters
- `histParams.config`: Histogram parameters and styling options

Example from `photon_only.config`:
```
# System Configuration 
System 2023_PbPb
DataType MC
InputDir /eos/cms/store/group/.../
OutputDir /eos/user/.../
OutputPrefix 2025_05_20_photon_only_debug

# Photon Selection
PhotonEtMin 30.0
PhotonEtaMax 1.44
PhotonHoverEMax 0.119947
PhotonSigmaIEtaIEtaMax 0.010392
PhotonIsoMax 2.099277
PhotonR9Min 0.8
```

#### Command-Line Interface
Basic usage:
```bash
# Interactive mode
root -l 'scripts/photonJet.C("../configs/photon_only.config")'

# Compiled mode with options
root -l 'photonJet.C+' -- --production
```

Available options:
- `--production`, `-p`: Run in production mode (all events)
- `--test`, `-t [n]`: Run in test mode with n events (default: 10,000)
- `--config`, `-c FILE`: Specify config file
- `--hist`, `-h FILE`: Specify histogram config file

### 5. Next Steps
1. Complete selection implementation:
   - [ ] Photon isolation definition
   - [ ] Jet quality criteria
   - [ ] Event-level selections

2. Implement validation framework:
   - [ ] Cut flow histograms
   - [ ] Efficiency calculations
   - [ ] MC truth matching

3. Setup production system:
   - [ ] HTCondor submission
   - [ ] Output validation
   - [ ] Log monitoring
