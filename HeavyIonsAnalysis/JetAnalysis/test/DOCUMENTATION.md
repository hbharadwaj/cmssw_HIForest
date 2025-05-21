# Photon-Tagged Jet Analysis Documentation

## Core Summary
Analysis to study substructure of photon-tagged jets in heavy ion collisions, comparing:
- 2023 PbPb
- 2024 PbPb  
- 2024 ppRef

Previous analysis focused on Rg and girth/angularity for 2018 PbPb and 2017 ppRef. This new analysis expands the scope with more variables and flexibility.

## Project Plan Overview

1. Skimming Step
   - Convert HiForest format to analysis TTrees
   - Config-driven approach
   - Condor submission capability
   - Handling of multiple input files and pthat samples

2. Analysis Framework Development
   - Flexible jet collection handling
   - Consistent variable structure
   - Automated MC weight calculation
   - Data/MC comparison tools

## Detailed Requirements

### 1. Config-Driven Design
- Selection criteria moved to config file
- Support for different forest structures:
  - PbPb vs pp differences
  - 2023 vs 2024 format changes
- Jet collection configuration
- Branch selection and aliasing
- Input/output path management

### 2. Code Structure
- Single efficient C++ implementation
- Human readable and maintainable
- Minimize redundancy and error-prone manual operations
- Clear separation of:
  - Configuration
  - Core processing
  - Helper functions

### 3. Processing Requirements
- HTCondor submission support
- Handling of multiple input files
- MC pthat sample combination
- Efficient memory usage
- Progress monitoring

### 4. Technical Specifications

#### Input Format
- Multiple HiForest TTrees
- Variable forest structure between years/collision systems
- MC truth information for simulation
- Different jet collections

#### Output Format
- Single TTree with consistent structure
- Flexible addition of new variables
- Efficient storage format
- Clear branch naming convention

#### Selection Implementation
- Config-based criteria definition
- Separate PbPb and pp selection handling
- Flexible trigger requirements
- Quality criteria configuration

## Outstanding Questions

1. Specific details of 2023/2024 forest differences?
2. Expected number and size of input files per sample?
3. Priority order for implementation of features?
4. Preferred config file format (current vs alternative)?
5. Required monitoring/logging detail level?
6. Specific memory/performance constraints?

## Enhancement Plan

### 1. Configuration System Improvements

#### A. System-Specific Configuration
- All configs stored in test/configs/ directory
- Naming convention: {YEAR}_{SYSTEM}_{SAMPLE}.config
- Example structure:
  ```
  configs/
  ├── 2023_PbPb_QCDPhoton.config
  ├── 2023_PbPb_Data.config
  ├── 2024_PbPb_QCDPhoton.config
  ├── 2024_PbPb_Data.config
  ├── 2024_pp_QCDPhoton.config
  └── 2024_pp_Data.config
  ```
- Each config contains:
  - Processing parameters
  - Input/output paths
  - Tree and branch selection
  - Event selection criteria
  - Jet configurations

#### B. Selection Criteria Configuration
Configuration format:
```
# Basic Processing Parameters
DoCondor 0
Verbose 1

# Input/Output 
InputDir /path/to/input
OutputDir /path/to/output
OutName output_name

# Event Selection
SelectionCuts pprimaryVertexFilter pclusterCompatibilityFilter phfCoincFilter2Th4
TriggerBits HLT_HIGEDPhoton30_v1 
CentralityMax 180
VzMax 15

# Optional Photon Selection
PhotonMaxEt 999999

# Trees and Collections
Trees ggHiNtuplizer/EventTree:ggHi hltanalysis/HltTree:hlt
JetCollections {{2,1},{2,2}}

# Branch Selection
Branches_ggHi mcPID mcPt mcEta
```

Key Changes:
1. Simplified directory structure
2. Consistent naming convention
3. Space-separated key-value pairs
4. Grouped configuration sections
5. No inline comments
6. Minimal but flexible selection criteria

### 2. Processing Framework Enhancements

#### A. Selection Framework
- Create modular selection classes:
  - PhotonSelector
  - JetSelector
  - EventSelector
- Implement strategy pattern for system-specific selections
- Allow runtime configuration of cuts

#### B. HTCondor Integration
- Create condor submission wrapper
- Split processing by:
  - Input file chunks
  - PThat bins
  - Collision system
- Handle output merging
- Progress monitoring

#### C. Data Processing
- Implement PThat weighting class
  - Handle multiple samples
  - Apply corrections
  - Calculate uncertainties
- Graceful branch handling:
  - Required vs optional branches
  - System-specific branches
  - Version-dependent mappings

#### D. Output Optimization
- Branch naming convention:
  ```
  obj_property[_detail][_correction]
  Examples: pho_et, jet_pt_raw, jet_rg_corr
  ```
- Compression optimization
- Validation suite:
  - Input file checks
  - Selection monitoring
  - Output consistency

### PThat Weighting Implementation

#### A. Configuration Structure
```
# PThat Weighting Configuration

# General settings
DoWeighting 1
WeightingScheme default

# PThat bin settings
PThatBins 10 20 30 50 80 120
PThatBinEdges 0 10 20 30 50 80 120 200

# Sample-specific settings
SampleWeights sample1_weight.txt sample2_weight.txt
```

#### B. Weighting Class Design
- Create PThatWeighting class
- Handle configuration parsing
- Implement weighting algorithms
- Provide interface for:
  - Setting up bins and weights
  - Applying weights to events
  - Retrieving weight information

## Implementation TODO

### Current Status
- Base skimming framework implemented
- Branch selection system in place
- Progress monitoring added
- Basic error handling implemented

### Phase 1: Testing & Validation (Current Priority)
1. Test Configuration System
   - [ ] Test branch selection with 2023_PbPb_QCDPhoton.config
   - [ ] Verify friend tree handling
   - [ ] Validate output structure
   - [ ] Test with different branch combinations

2. Basic Selection Implementation
   - [ ] Add event selection from config
   - [ ] Implement photon selection
   - [ ] Add centrality selection
   - [ ] Test selection efficiency

3. Memory & Performance
   - [ ] Test with large input files
   - [ ] Monitor memory usage
   - [ ] Optimize chain building
   - [ ] Add batch size control

4. Output Validation
   - [ ] Compare output with Run2 format
   - [ ] Verify branch content
   - [ ] Check event counts
   - [ ] Validate friend tree contents

### Phase 2: Feature Implementation (Next Stage)
1. PThat Weighting System
2. HTCondor Integration
3. Extended Validation Suite
4. Monitoring Tools

### Phase 3: Production Setup
1. Automation Scripts
2. Documentation
3. User Guide
4. Performance Optimization

## Testing Plan

### 1. Configuration Testing
- Test with minimal branch selection
- Test with full branch set
- Test friend tree relationships
- Validate output structure

### 2. Data Processing Tests
- Test with small sample
- Test with full file
- Test with multiple files
- Test memory usage

### 3. Validation Requirements
- Compare output with Run2 ntuples
- Check branch presence and content
- Verify event counts
- Validate selection criteria
