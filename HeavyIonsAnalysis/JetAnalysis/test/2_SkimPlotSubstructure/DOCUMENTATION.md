# CMS Gamma-Jet Analysis: Comprehensive Technical Documentation

## Project Overview
This project provides a robust framework for photon-tagged jet substructure analysis in heavy ion collisions, with a focus on modularity, reproducibility, and publication-quality outputs. The framework supports both data and MC workflows, advanced ROOT file handling, and CMS-compliant plotting.

**Physics Goal:** Study jet quenching and medium effects in heavy-ion collisions using photon-tagged jets as calibrated probes of the QGP.

## Analysis Framework Architecture

## Core Analysis Files

### Essential Components
1. **`gammaJetAnalyzer.cpp`** - Main analysis executable (C++17)
2. **`plotGammaJet.py`** - Advanced plotting framework (Python 3)
3. **`Makefile`** - Production build system
4. **`compile.sh`** - Development compilation script

### Supporting Files
- **`test_datamc_config.py`** - Configuration validation utility
- **`scripts/tdrstyle.C`** - CMS plotting style definitions
- **`run_plotting.sh`** - Automated plotting execution

### Development Templates
- **`scripts/photonJet.C`** - Legacy ROOT macro template
- **`scripts/generateTemplates.C`** - Code generation utilities
- **`scripts/testHeaderGeneration.C`** - Header file testing

**Note:** Template files in `scripts/` contain features to be implemented in future versions and serve as development references.

### Data Flow
```
Skimmed HiForest → gammaJetAnalyzer → Analysis ROOT Files → plotGammaJet → Publication Plots
(Flat TTree)            ↑                      ↑                     ↑
                  .config files        Helper Scripts        CMS Style Guide
```

## gammaJetAnalyzer.cpp: Analysis Engine

### Core Functionality
- **Input Processing:** Reads skimmed HiForest files with flat TTree structure (output from Step 1)
- **Event Selection:** Photon isolation, jet quality cuts, kinematic selections
- **Multi-Collection Support:** Handles different jet algorithms (AK4, AK8) and Z-cut values
- **Centrality Binning:** Configurable centrality selections for heavy-ion analysis
- **Output Management:** Produces both flat and nested ROOT file structures
- **Memory Optimization:** Efficient ROOT tree processing with selective branch reading

### Key Classes and Methods
- **`JetCollectionManager`:** Manages multiple jet algorithms and configurations
- **`CentralityManager`:** Handles centrality binning and event classification  
- **`PhotonSelector`:** Implements photon identification and isolation cuts
- **`JetAnalyzer`:** Core jet analysis algorithms including substructure
- **`HistogramManager`:** Centralized histogram booking and filling

### Configuration Parameters
```cpp
// Example config entries for gammaJetAnalyzer
PhotonPtMin: 60.0
PhotonEtaMax: 1.44
JetPtMin: 40.0
JetEtaMax: 2.0
DeltaPhiMin: 2.094  // 2π/3
CentralityBins: 0,30,60,90,180
JetCollections: AK4Z1,AK4Z2,AK8Z2,AK8Z4
UseNestedOutput: 1
MaxEvents: -1  // Process all events
```

### Performance Optimizations
- **Selective Branch Reading:** Only loads required branches from HiForest
- **Memory Pool Management:** Efficient histogram and tree memory handling
- **Parallel Processing Ready:** Thread-safe design for future parallelization
- **Batch Processing:** Optimized for HTCondor job submission

### Event Selection Criteria
1. **Photon Selection:**
   - Isolated photons with ET > 60 GeV
   - |η| < 1.44 (barrel region)
   - Shower shape and isolation requirements
   
2. **Jet Selection:**
   - Anti-kT jets with pT > 40 GeV  
   - |η| < 2.0 for tracking efficiency
   - Jet quality cuts and pile-up mitigation
   
3. **Correlation Requirements:**
   - Δφ(γ,jet) > 2π/3 for back-to-back topology
   - Leading jet matching to photon

### plotGammaJet.py: Advanced Plotting Framework

### Design Philosophy
- **Configuration-Driven:** All plot parameters controlled via `.config` files
- **CMS Compliance:** Official CMS style guidelines and formatting
- **Modular Design:** Separate functions for different plot types and overlays
- **Publication Ready:** High-quality output in multiple formats (PDF, PNG, ROOT)

### Advanced Features
- **Data-MC Comparison:** Ratio plots with statistical uncertainty bands
- **Multi-Dimensional Overlays:** Compare across centrality bins and jet collections
- **Color-Blind Support:** Accessible color schemes and marker combinations
- **Batch Processing:** Command-line interface for automated plot production
- **Smart File Handling:** Automatic detection of ROOT file structure (nested vs flat)

### Configuration System Architecture

### Design Rationale
The framework uses simple key-value `.config` files compatible with both C++ (TEnv) and Python parsing. This approach provides:
- **Legacy Compatibility:** Works with existing CMS/ROOT workflows
- **Simplicity:** Easy editing without specialized tools
- **Bi-directional Use:** Same files work for both analysis and plotting
- **Version Control Friendly:** Plain text format for easy diff tracking

### Configuration Categories
1. **Analysis Parameters:** Physics cuts, selections, algorithms
2. **I/O Settings:** File paths, output formats, directory structures  
3. **Plotting Options:** Colors, styles, CMS labeling, overlays
4. **Batch Settings:** Job submission, resource requirements, output management

### Build System and Compilation

### Compilation Methods
1. **Makefile:** Production compilation with optimization
   ```bash
   make clean && make
   ./gammaJetAnalyzer configs/2023_PbPb_Data.config
   ```

2. **compile.sh:** Development compilation script
   ```bash
   ./compile.sh
   ./gammaJetAnalyzer configs/2023_PbPb_Data.config
   ```
   
### Compiler Optimizations
- **Release Mode:** `-O3` optimization for production
- **Debug Mode:** `-g` symbols for development
- **C++17 Features:** Modern C++ features and STL algorithms
- **ROOT Integration:** Optimized ROOT library linking

## Batch Processing and Job Management

### Available Scripts
The framework includes basic job management capabilities in development:

- **`submit_production.sh`:** Job submission script for production analysis
- **`monitor_jobs.sh`:** Basic job monitoring and status checking
- **`run_plotting.sh`:** Automated plotting script execution

### Current Status
- **Batch System:** HTCondor integration under development
- **Job Templates:** Located in `batch/` directory (currently empty - templates to be added)
- **Resource Management:** Basic resource allocation and monitoring

## Advanced Analysis Features

### Jet Substructure Analysis
- **Grooming Algorithms:** Soft Drop, Z-cut grooming for jet substructure
- **Substructure Variables:** Mass, groomed momentum fraction, splitting scales
- **Comparative Studies:** Multiple grooming parameters in parallel

### Systematic Studies
- **Uncertainty Propagation:** JEC, JER, photon energy scale uncertainties
- **Background Estimation:** Data-driven methods for photon purity
- **Centrality Dependencies:** Medium effects across different collision geometries

### Data Quality and Validation
- **Automated Checks:** Event counts, histogram sanity checks, file integrity
- **Cross-Validation:** Data vs MC comparisons at multiple analysis stages
- **Performance Monitoring:** Processing time and memory usage tracking

## Integration with CMS Software

### CMSSW Compatibility
- **Framework Version:** CMSSW_13_2_13 for Run 3 data processing
- **Data Formats:** HiForest ntuples from centralized production
- **Calibrations:** Latest JEC/JER corrections and photon energy scales

### CMS Data Management
- **Input Data:** Centralized HiForest production on `/eos/cms/`
- **Output Storage:** User space on `/eos/user/` with organized directory structure
- **Backup Strategy:** Critical analysis outputs backed up to CMS DAS

## Development Timeline & Milestones
- **May 2025:** Entire framework (C++ analysis, Python plotting, and all configs) designed, implemented, and documented in a single rapid development sprint. All major features, including batch processing, config-driven analysis, multi-jet/centrality support, overlays, ratio plots, and CMS-style outputs, were completed within this period.

## Major Issues & Lessons Learned
- **ROOT file structure drift:** Early in the sprint, standardization was enforced via config files to ensure compatibility.
- **Batch processing bugs:** Addressed by modularizing job submission and output handling.
- **Documentation drift:** Multiple outdated .md files consolidated into this single source.
- **User feedback:** Led to improved error handling, clearer config options, and better quick start guides.
- **Memory Management:** Careful ROOT object handling to prevent memory leaks in long batch jobs
- **Configuration Complexity:** Balance between flexibility and usability in config file design

## Future Development Plans
- **CMS Integration:** Incorporate into official CMS Heavy Ion analysis workflows
- **Machine Learning:** Jet substructure analysis using deep learning techniques  
- **Real-time Analysis:** Integration with CMS online selection and monitoring
- **Systematic Automation:** Automated systematic uncertainty evaluation
- **Performance Optimization:** GPU acceleration for computationally intensive tasks
- **Analysis Preservation:** Integration with CMS Analysis Preservation service

---

# MEMORY_PLOTTING.md

# CMS Gamma-Jet Analysis Plotting Functions Documentation

This document provides comprehensive documentation of all functions available in the `plotGammaJet.py` script for CMS Heavy Ion gamma-jet analysis plotting.

## Table of Contents

1. [Configuration and Data Classes](#configuration-and-data-classes)
2. [Core Configuration Functions](#core-configuration-functions)
3. [Styling and Color Functions](#styling-and-color-functions)
4. [ROOT File Access Functions](#root-file-access-functions)
5. [Plotting Functions](#plotting-functions)
6. [Data-MC Comparison Functions](#data-mc-comparison-functions)
7. [Utility Functions](#utility-functions)
8. [Main Function](#main-function)
9. [Available Config Options](#available-config-options)
10. [Usage Examples](#usage-examples)

---

## Configuration and Data Classes

### `HistogramConfig`
**Class for histogram configuration management**
- **Purpose**: Holds all styling and plotting parameters for histograms
- **Key attributes**: `name`, `title`, `bins`, `x_min`, `x_max`, `log_y`, `plot_type`, `color`, `line_width`, `draw_option`, `marker_style`, `marker_size`
- **Supports**: Both 1D and 2D histograms with separate configurations

### `ProfileConfig`
**Class for ROOT profile histogram configuration**
- **Purpose**: Configuration for TProfile plotting
- **Key attributes**: `name`, `title`, `x_bins`, `x_min`, `x_max`

### `DataMCConfig`
**Class for Data-MC comparison styling**
- **Purpose**: Manages styling for data vs MC comparison plots
- **Key attributes**: Data styling, MC styling, ratio plot settings, canvas dimensions, legend positioning

---

## Core Configuration Functions

### `parse_config(config_path)`
**Parse configuration file into dictionary**
- **Input**: Path to config file
- **Output**: Dictionary of configuration key-value pairs
- **Format**: Supports `key: value` format with `#` comments

### `get_histogram_configs(config)`
**Extract histogram configurations from config**
- **Input**: Configuration dictionary
- **Output**: Dict of `{plot_key: HistogramConfig}` for all defined histograms
- **Usage**: Primary method to get all available histogram definitions

### `get_profile_configs(config)`
**Extract profile configurations from config**
- **Input**: Configuration dictionary  
- **Output**: Dict of `{plot_key: ProfileConfig}` for all defined profiles

### `get_datamc_config(config)`
**Get Data-MC comparison configuration**
- **Input**: Configuration dictionary
- **Output**: `DataMCConfig` object with all Data-MC styling parameters

### `get_overlay_plots(config)`
**Get list of plots to overlay**
- **Input**: Configuration dictionary
- **Output**: List of plot keys for overlay plotting
- **Config key**: `OverlayPlots`

### `get_plot_formats(config)`
**Get output file formats**
- **Input**: Configuration dictionary
- **Output**: List of formats (e.g., ['pdf', 'png', 'root'])
- **Config key**: `PlotFormats`

---

## Styling and Color Functions

### `get_color_scheme(scheme_name, color_blind=False)`
**Get color palette for plotting**
- **Available schemes**: `petroff6`, `petroff10`, `viridis`, `traditional`, `default`
- **Color-blind support**: Returns accessible colors when `color_blind=True`
- **Output**: List of ROOT color codes

### `set_cms_style(config=None)`
**Set comprehensive CMS plotting style**
- **Purpose**: Applies official CMS style guidelines
- **Features**: Proper fonts (Helvetica), margins, tick marks, text sizes
- **Canvas margins**: Configurable via config file

### `get_auto_color(color_scheme, index, color_blind=False)`
**Get color by index from scheme**
- **Input**: Color scheme name, index, color-blind flag
- **Output**: ROOT color code
- **Usage**: Automatic color assignment for overlays

### `get_marker_styles()` / `get_auto_marker(index)`
**CMS-compliant marker styles**
- **Available markers**: Full/open circles, squares, triangles, diamonds, stars
- **Usage**: Automatic marker assignment for data points

### `apply_histogram_style(hist, hist_config, color_index, config)`
**Apply styling to histogram**
- **Purpose**: Sets colors, markers, line widths based on configuration
- **Features**: Automatic or manual color/marker assignment

---

## ROOT File Access Functions

### `get_available_jet_dirs(root_file, config)`
**Get available jet collections in ROOT file**
- **Input**: ROOT file object, configuration
- **Output**: List of existing jet directories (e.g., ['AK4Z2', 'AK8Z4'])
- **Config key**: `JetDirectories`

### `get_available_centrality_bins(root_file, jet_dir, config)`
**Get available centrality bins**
- **Input**: ROOT file, jet directory, configuration
- **Output**: List of centrality bins (e.g., ['cent0to60', 'cent60to180'])
- **Config key**: `CentralityBins`

### `get_histogram_from_path(root_file, jet_dir, cent_bin, hist_name, config)`
**Get histogram from nested ROOT structure**
- **Purpose**: Navigate nested ROOT file structure to retrieve histograms
- **Supports**: Both nested (`JetDir/CentBin/HistName`) and flat structures
- **Config key**: `UseNestedStructure`

### `load_histogram_from_file(root_file, hist_path)`
**Load histogram with proper memory management**
- **Purpose**: Clone histogram and detach from file
- **Benefits**: Prevents memory issues when closing files

### `find_histogram_in_structure(root_file, hist_name, jet_dir=None, cent_bin=None)`
**Find histogram path in ROOT structure**
- **Purpose**: Search for histogram in various possible locations
- **Output**: Full path to histogram if found, None otherwise

---

## Plotting Functions

### `plot_histogram_1d(hist, hist_config, config, outdir, formats, jet_dir="", cent_bin="")`
**Plot 1D histograms with full CMS styling**
- **Features**: Log/linear scales, CMS labels, selection text, axis configuration
- **Output**: Saves in multiple formats with proper naming

### `plot_histogram_2d(hist, hist_config, config, outdir, formats, jet_dir="", cent_bin="")`
**Plot 2D histograms with color maps**
- **Features**: Color palettes (viridis, plasma, bird), log z-scale, proper margins
- **Config options**: `ColorMap`, `LogZ`, `DrawOption2D`

### `plot_overlay_advanced(hists, labels, config, outdir, name, formats, hist_config=None, jet_dir="", cent_bin="")`
**Advanced overlay plotting with multiple histograms**
- **Features**: Automatic color/marker assignment, configurable legends, scaling
- **Legend positions**: `left`, `right`, `bcenter`, or custom coordinates
- **Config keys**: `Overlay.LegendPosition`, `Legend.Position`

### `plot_centrality_overlay(root_file, jet_dir, hist_name, config, outdir, formats)`
**Create centrality comparison overlays**
- **Purpose**: Compare same histogram across different centrality bins
- **Config key**: `OverlayCentralityBins`

### `plot_jet_collection_overlay(root_file, cent_bin, hist_name, config, outdir, formats)`
**Create jet collection comparison overlays**
- **Purpose**: Compare same histogram across different jet collections
- **Config key**: `OverlayJetCollections`

---

## Data-MC Comparison Functions

### `plot_datamc_comparison(data_file, mc_file, hist_name, config, outdir, formats, jet_dir="", cent_bin="")`
**Create Data-MC comparison with ratio plot**
- **Features**: 
  - Two-panel plot (main comparison + ratio)
  - Configurable styling for data and MC
  - Automatic normalization option
  - Reference line at ratio = 1
- **Config keys**: All `DataMC.*` parameters

### `plot_datamc_overlay_multiple(data_file, mc_file, hist_configs, config, outdir, formats, jet_dir="", cent_bin="")`
**Multiple histogram Data-MC overlay**
- **Purpose**: Compare multiple histogram types between Data and MC in single plot
- **Usage**: For systematic studies

---

## Utility Functions

### `draw_cms_label(canvas, config, selection_text="")`
**Draw official CMS label and experiment info**
- **Features**: CMS logo, preliminary/simulation text, energy, luminosity
- **Config keys**: `CMSLabel`, `CMSEnergyText`, `CMSLuminosity`, `CMSExtraText`

### `get_cms_text(config)`
**Generate CMS text label from configuration**
- **Purpose**: Format CMS label text for display
- **Output**: Formatted string like "CMS Preliminary" or "CMS Simulation"

### `create_output_dirs(base_dir, config, jet_dirs=None)`
**Create organized output directory structure**
- **Structure**: `base/1D/`, `base/2D/`, `base/overlays/`, etc.
- **Jet-specific**: Creates subdirectories for each jet collection

### `get_selection_text(config, jet_dir="", cent_bin="")`
**Generate selection criteria text for plots**
- **Sources**: Configuration parameters or custom text
- **Config keys**: `SelectionText`, `SelectionPhotonPtMin`, `SelectionJetPtMin`, etc.

### `save_canvas(canvas, outdir, name, formats, jet_dir="", cent_bin="")`
**Save canvas in multiple formats**
- **Features**: Automatic filename generation with jet/centrality info
- **Formats**: PDF, PNG, ROOT, etc.

### `get_centrality_label(cent_bin, config=None)` / `get_jet_label(jet_dir)`
**Format display labels**
- **Purpose**: Convert internal names to display-ready labels
- **Examples**: `cent0to60` → `"Centrality 0-30%"`, `AK4Z2` → `"AK R=0.4, Z_cut=0.2"`

### `parse_overlay_jets(config)` / `parse_overlay_centrality(config)`
**Parse overlay configuration**
- **Purpose**: Determine which collections/bins to overlay
- **Config keys**: `OverlayJetCollections`, `OverlayCentralityBins`

---

## Main Function

### `main()`
**Enhanced main function with full Data-MC support**
- **Modes**: 
  - Single file plotting
  - Data-MC comparison mode
  - Test mode (limited plots)
- **Command line options**:
  ```bash
  -r/--rootfile: Input ROOT file (Data file for Data-MC)
  -c/--config: Configuration file
  -o/--outdir: Output directory
  --mc-file: MC file for Data-MC comparison
  --datamc-mode: Enable Data-MC mode
  --datamc-plots: Specific plots to compare
  --batch: Batch mode (no GUI)
  --verbose: Verbose output
  --test: Test mode
  --jet-dir: Specific jet collection
  --cent-bin: Specific centrality bin
  ```

---

## Available Config Options

### Basic Settings
```
EnablePlotting: 1
SavePlots: 1
PlotFormats: pdf,png,root
UseNestedStructure: 1
PlotOutputDir: /path/to/output
```

### CMS Labeling
```
UseCMSStyle: 1
CMSLabel: Preliminary
CMSEnergyText: 5.36 TeV PbPb
CMSLuminosity: 1.72 nb^{-1}
CMSExtraText: Simulation
```

### Colors and Styling
```
ColorScheme: petroff6
UseColorBlind: 1
CanvasWidth: 800
CanvasHeight: 600
CanvasMarginLeft: 0.15
CanvasMarginRight: 0.05
CanvasMarginTop: 0.08
CanvasMarginBottom: 0.12
```

### Jet Collections and Centrality
```
JetDirectories: AK2Z1,AK2Z2,AK3Z1,AK4Z2,AK8Z4
CentralityBins: 0,60,180
```

### Selection Criteria
```
SelectionPhotonPtMin: 60
SelectionPhotonEtaMax: 1.44
SelectionJetPtMin: 40
SelectionJetEtaMax: 2.0
SelectionDeltaPhiMin: 2π/3
SelectionText: Custom selection text
```

### Data-MC Comparison
```
DataMC.DataLabel: Data
DataMC.MCLabel: PYTHIA8
DataMC.DataColor: 1
DataMC.MCColor: 2
DataMC.RatioTitle: Data/MC
DataMC.RatioYMin: 0.5
DataMC.RatioYMax: 1.5
DataMC.Normalize: 1
```

### Histogram Definitions
```
Histogram.JetPt.Name: hJetPt
Histogram.JetPt.Title: Jet p_{T};p_{T} [GeV/c];Entries
Histogram.JetPt.Bins: 100
Histogram.JetPt.XMin: 0
Histogram.JetPt.XMax: 200
Histogram.JetPt.LogY: 1
Histogram.JetPt.PlotType: 1D
Histogram.JetPt.Color: auto
Histogram.JetPt.DrawOption: hist
```

### Overlay Settings
```
OverlayPlots: JetPt,JetEta,DeltaPhi
OverlayJetCollections: AK4Z2,AK8Z4
OverlayCentralityBins: all
OverlayByCentrality: 1
OverlayByJetCollection: 1
```

---

## Usage Examples

### Basic Data-MC Comparison
```bash
python plotGammaJet.py \
  -r /path/to/data.root \
  --mc-file /path/to/mc.root \
  -c DataMC_overlay.config \
  -o /path/to/output \
  --datamc-mode \
  --batch
```

### Test Specific Plots
```bash
python plotGammaJet.py \
  -r /path/to/data.root \
  --mc-file /path/to/mc.root \
  -c DataMC_overlay.config \
  --datamc-plots JetPt,JetEta,DeltaPhi \
  --test \
  --verbose
```

### Single File Plotting
```bash
python plotGammaJet.py \
  -r /path/to/file.root \
  -c PlotJetSub_2023_PbPb_Data.config \
  -o /path/to/output \
  --jet-dir AK4Z2 \
  --cent-bin cent0to60
```

---

## Available Histograms in Config

Based on the `DataMC_overlay.config`, the following histograms are available:

1. **JetPt**: Jet transverse momentum
2. **JetEta**: Jet pseudorapidity  
3. **JetPhi**: Jet azimuthal angle
4. **DeltaPhi**: Angular correlation between photon and jet
5. **Xj**: Momentum balance (p_T^jet / E_T^photon)
6. **PhotonEt**: Photon transverse energy
7. **PhotonEta**: Photon pseudorapidity
8. **JetMass**: Jet mass

Each histogram has full configuration including binning, axis ranges, log scales, draw options, and styling parameters.

---

## File Structure and Paths

The script is designed to work with:
- **Data file**: `/eos/user/b/bharikri/www/Run3GammaJet/2025_05_24/2023_PbPb/Data/2025_05_24_HiRawPrime0_part_output.root`
- **MC file**: `/eos/user/b/bharikri/www/Run3GammaJet/2025_05_24/2023_PbPb/MC/2025_05_24_QCDPhoton30_output.root`
- **Output**: `/eos/user/b/bharikri/www/Run3GammaJet/2025_05_24/2023_PbPb/DataMC_comparison`

The script automatically creates organized subdirectories for different plot types and jet collections.

For a quick start, see `README.md`. For implementation details, see `gammaJetAnalyzer.cpp` and `plotGammaJet.py`.

---

# Adding New Variables to the Analysis Framework

This section provides a comprehensive step-by-step procedure for adding new jet substructure variables (or any analysis variables) to the gamma-jet analysis framework. Follow this procedure to ensure consistent integration across all components.

### Prerequisites
- Understanding of C++ and ROOT framework
- Familiarity with the existing codebase structure
- Access to jet substructure variable definitions in HiForest files

### Step-by-Step Procedure

#### **Phase 1: JetCollectionManager Integration**

**1.1 Add Getter Method Declaration in JetCollectionManager.h**
```cpp
// Add to public section of JetCollectionManager class
float getJetNewVariable(const std::string& collection, int index) const;
float getRefJetNewVariable(const std::string& collection, int index) const; // For MC
```

**1.2 (Optional)Implement Getter Methods in JetCollectionManager.h**
```cpp
float JetCollectionManager::getJetNewVariable(const std::string& collection, int index) const {
    auto it = jetData.find(collection);
    if (it == jetData.end() || !it->second.jetNewVariable || 
        index < 0 || index >= static_cast<int>(it->second.jetNewVariable->size())) {
        return -999; // Default invalid value
    }
    return it->second.jetNewVariable->at(index);
}
```
Add the variable to the JetProperty enum instead and implement the getter method to call the generic `getJetProperty` method if you want to avoid adding a new getter method for every variable.

**1.3 Add Branch Variables to BranchInfo Structure**


#### **Phase 2: gammaJetAnalyzer.cpp Integration**

**2.1 Add Output Variable Maps**
```cpp
// Add to variable declarations in processEvents function
std::map<std::string, float> selectedJetNewVariables;
std::map<std::string, float> selectedRefJetNewVariables; // For MC
```

**2.2 Initialize and Setup Branches**
```cpp
// Add to initialization loop for each jet collection
selectedJetNewVariables[collection] = 0;
selectedRefJetNewVariables[collection] = 0;

// Add branch creation
outTree->Branch(("jetNewVariable_" + collection).c_str(), &selectedJetNewVariables[collection]);
outTree->Branch(("refJetNewVariable_" + collection).c_str(), &selectedRefJetNewVariables[collection]);
```

**2.3 Reset Variables in Event Loop**
```cpp
// Add to reset section for each collection
selectedJetNewVariables[collection] = -999;
selectedRefJetNewVariables[collection] = -999;
```

**2.4 Fill Variables After Jet Selection**
```cpp
// Add to jet selection section after bestJetIndex is found
if (bestJetIndex >= 0) {
    // ...existing code...
    selectedJetNewVariables[collection] = jetManager.getJetNewVariable(collection, bestJetIndex);
    selectedRefJetNewVariables[collection] = jetManager.getRefJetNewVariable(collection, bestJetIndex);
}
```

**2.5 Add Histogram Creation**
```cpp
// Add to createHistograms function
TH1F* hNewVariable = createHist1D("hNewVariable", "New Variable;NewVar;Entries", 50, min_val, max_val);
TH1F* hRefNewVariable = createHist1D("hRefNewVariable", "Ref New Variable;NewVar;Entries", 50, min_val, max_val);
```

**2.6 Add Histogram Filling**
```cpp
// Add to histogram filling section in event loop
TH1F* hNewVariable = (TH1F*)gDirectory->Get("hNewVariable");
if (hNewVariable) hNewVariable->Fill(selectedJetNewVariables[collection], eventWeight);

// For MC-matched jets
if (isMC && selectedRefJetNewVariables[collection] > -900) {
    TH1F* hRefNewVariable = (TH1F*)gDirectory->Get("hRefNewVariable");
    if (hRefNewVariable) hRefNewVariable->Fill(selectedRefJetNewVariables[collection], eventWeight);
}
```

#### **Phase 3: Configuration File Updates**

**3.1 Data Configuration (PlotJetSub_2023_PbPb_Data.config)**
```
Histogram.NewVariable.Name: hNewVariable
Histogram.NewVariable.Title: New Variable Description;NewVar;Entries
Histogram.NewVariable.Bins: 50
Histogram.NewVariable.XMin: min_value
Histogram.NewVariable.XMax: max_value
Histogram.NewVariable.LogY: 0
Histogram.NewVariable.PlotType: 1D
Histogram.NewVariable.Color: auto
Histogram.NewVariable.LineWidth: 2
Histogram.NewVariable.DrawOption: E1][P0
Histogram.NewVariable.MarkerStyle: auto
Histogram.NewVariable.MarkerSize: 0.8
```

**3.2 MC Configuration (PlotJetSub_2023_PbPb_MC.config)**
```
# Add same histogram definition as data config, plus:

Histogram.RefNewVariable.Name: hRefNewVariable
Histogram.RefNewVariable.Title: MC matched New Variable;NewVar;Entries
Histogram.RefNewVariable.Bins: 50
Histogram.RefNewVariable.XMin: min_value
Histogram.RefNewVariable.XMax: max_value
Histogram.RefNewVariable.LogY: 0
Histogram.RefNewVariable.PlotType: 1D
Histogram.RefNewVariable.Color: auto
Histogram.RefNewVariable.LineWidth: 2
Histogram.RefNewVariable.DrawOption: E1][P0
Histogram.RefNewVariable.MarkerStyle: auto
Histogram.RefNewVariable.MarkerSize: 0.8
```

**3.3 Update Overlay Settings in All Config Files**
```
# Update OverlayPlots line to include new variable
OverlayPlots: JetPt,JetEta,DeltaPhi,Xj,DynSplit,JetMass,NewVariable
```

**3.4 Update DataMC_overlay.config**
```
# Add NewVariable to overlay list
OverlayPlots: JetPt,PhotonEt,DeltaPhi,Xj,JetMass,DynSplit,DynKt,DynZ,Girth,Thrust,NewVariable
```

#### **Phase 4: Testing and Validation**

**4.1 Compile and Test**
```bash
make clean && make
./gammaJetAnalyzer config_file.config input_file.root output_file.root
```

**4.2 Verify Output**
- Check that new branches exist in output ROOT file
- Verify histogram creation and filling
- Validate that MC and data configs work correctly

**4.3 Test Plotting**
```bash
python plotGammaJet.py --config plotting_config.config --input analysis_output.root --output plots/
```

### Important Notes and Best Practices

#### **Variable Naming Conventions**
- Use descriptive names that match physics meaning
- Follow existing camelCase pattern for C++ variables
- Use consistent prefixes: `jet` for reco, `refJet` for MC-matched
- Branch names should match HiForest conventions

#### **Default Values and Error Handling**
- Use `-999` as default for invalid/missing values
- Always check array bounds in getter methods
- Handle missing branches gracefully with nullptr checks

#### **Configuration Guidelines**
- Choose appropriate bin numbers and ranges based on variable physics
- Use consistent plot styling across similar variables
- Add variables to overlay lists only if meaningful comparisons exist

#### **Common Pitfalls to Avoid**
1. **Branch Name Mismatches:** Ensure branch names match exactly with HiForest
2. **Missing MC Branches:** Not all variables may exist in both reco and ref collections
3. **Uninitialized Pointers:** Always initialize vector pointers to nullptr
4. **Index Out of Bounds:** Always validate array indices before access
5. **Memory Leaks:** ROOT manages histogram memory, but be careful with custom objects

#### **Example: Real Implementation (JetArea, DynDeltaR, IntJetMulti)**
See the recent implementation of `JetArea`, `DynDeltaR`, and `IntJetMulti` variables as reference examples following this exact procedure. These variables demonstrate:
- Proper getter method implementation
- Correct output variable handling
- Appropriate histogram configuration
- Full integration across all config files

### Development Workflow Summary
1. **Plan:** Identify variable source in HiForest and physics requirements
2. **Implement:** Follow Phase 1-2 for code integration  
3. **Configure:** Update all relevant config files (Phase 3)
4. **Test:** Compile, run analysis, and verify outputs (Phase 4)
5. **Validate:** Check physics distributions and plot quality
6. **Document:** Update this guide with any new insights or edge cases

This procedure ensures consistent, maintainable code that integrates seamlessly with the existing analysis framework while following CMS software best practices.

## Analysis Framework Architecture

## Core Analysis Files

### Essential Components
1. **`gammaJetAnalyzer.cpp`** - Main analysis executable (C++17)
2. **`plotGammaJet.py`** - Advanced plotting framework (Python 3)
3. **`Makefile`** - Production build system
4. **`compile.sh`** - Development compilation script

### Supporting Files
- **`test_datamc_config.py`** - Configuration validation utility
- **`scripts/tdrstyle.C`** - CMS plotting style definitions
- **`run_plotting.sh`** - Automated plotting execution

### Development Templates
- **`scripts/photonJet.C`** - Legacy ROOT macro template
- **`scripts/generateTemplates.C`** - Code generation utilities
- **`scripts/testHeaderGeneration.C`** - Header file testing

**Note:** Template files in `scripts/` contain features to be implemented in future versions and serve as development references.

### Data Flow
```
Skimmed HiForest → gammaJetAnalyzer → Analysis ROOT Files → plotGammaJet → Publication Plots
(Flat TTree)            ↑                      ↑                     ↑
                  .config files        Helper Scripts        CMS Style Guide
```

## gammaJetAnalyzer.cpp: Analysis Engine

### Core Functionality
- **Input Processing:** Reads skimmed HiForest files with flat TTree structure (output from Step 1)
- **Event Selection:** Photon isolation, jet quality cuts, kinematic selections
- **Multi-Collection Support:** Handles different jet algorithms (AK4, AK8) and Z-cut values
- **Centrality Binning:** Configurable centrality selections for heavy-ion analysis
- **Output Management:** Produces both flat and nested ROOT file structures
- **Memory Optimization:** Efficient ROOT tree processing with selective branch reading

### Key Classes and Methods
- **`JetCollectionManager`:** Manages multiple jet algorithms and configurations
- **`CentralityManager`:** Handles centrality binning and event classification  
- **`PhotonSelector`:** Implements photon identification and isolation cuts
- **`JetAnalyzer`:** Core jet analysis algorithms including substructure
- **`HistogramManager`:** Centralized histogram booking and filling

### Configuration Parameters
```cpp
// Example config entries for gammaJetAnalyzer
PhotonPtMin: 60.0
PhotonEtaMax: 1.44
JetPtMin: 40.0
JetEtaMax: 2.0
DeltaPhiMin: 2.094  // 2π/3
CentralityBins: 0,30,60,90,180
JetCollections: AK4Z1,AK4Z2,AK8Z2,AK8Z4
UseNestedOutput: 1
MaxEvents: -1  // Process all events
```

### Performance Optimizations
- **Selective Branch Reading:** Only loads required branches from HiForest
- **Memory Pool Management:** Efficient histogram and tree memory handling
- **Parallel Processing Ready:** Thread-safe design for future parallelization
- **Batch Processing:** Optimized for HTCondor job submission

### Event Selection Criteria
1. **Photon Selection:**
   - Isolated photons with ET > 60 GeV
   - |η| < 1.44 (barrel region)
   - Shower shape and isolation requirements
   
2. **Jet Selection:**
   - Anti-kT jets with pT > 40 GeV  
   - |η| < 2.0 for tracking efficiency
   - Jet quality cuts and pile-up mitigation
   
3. **Correlation Requirements:**
   - Δφ(γ,jet) > 2π/3 for back-to-back topology
   - Leading jet matching to photon

### plotGammaJet.py: Advanced Plotting Framework

### Design Philosophy
- **Configuration-Driven:** All plot parameters controlled via `.config` files
- **CMS Compliance:** Official CMS style guidelines and formatting
- **Modular Design:** Separate functions for different plot types and overlays
- **Publication Ready:** High-quality output in multiple formats (PDF, PNG, ROOT)

### Advanced Features
- **Data-MC Comparison:** Ratio plots with statistical uncertainty bands
- **Multi-Dimensional Overlays:** Compare across centrality bins and jet collections
- **Color-Blind Support:** Accessible color schemes and marker combinations
- **Batch Processing:** Command-line interface for automated plot production
- **Smart File Handling:** Automatic detection of ROOT file structure (nested vs flat)

### Configuration System Architecture

### Design Rationale
The framework uses simple key-value `.config` files compatible with both C++ (TEnv) and Python parsing. This approach provides:
- **Legacy Compatibility:** Works with existing CMS/ROOT workflows
- **Simplicity:** Easy editing without specialized tools
- **Bi-directional Use:** Same files work for both analysis and plotting
- **Version Control Friendly:** Plain text format for easy diff tracking

### Configuration Categories
1. **Analysis Parameters:** Physics cuts, selections, algorithms
2. **I/O Settings:** File paths, output formats, directory structures  
3. **Plotting Options:** Colors, styles, CMS labeling, overlays
4. **Batch Settings:** Job submission, resource requirements, output management

### Build System and Compilation

### Compilation Methods
1. **Makefile:** Production compilation with optimization
   ```bash
   make clean && make
   ./gammaJetAnalyzer configs/2023_PbPb_Data.config
   ```

2. **compile.sh:** Development compilation script
   ```bash
   ./compile.sh
   ./gammaJetAnalyzer configs/2023_PbPb_Data.config
   ```
   
### Compiler Optimizations
- **Release Mode:** `-O3` optimization for production
- **Debug Mode:** `-g` symbols for development
- **C++17 Features:** Modern C++ features and STL algorithms
- **ROOT Integration:** Optimized ROOT library linking

## Batch Processing and Job Management

### Available Scripts
The framework includes basic job management capabilities in development:

- **`submit_production.sh`:** Job submission script for production analysis
- **`monitor_jobs.sh`:** Basic job monitoring and status checking
- **`run_plotting.sh`:** Automated plotting script execution

### Current Status
- **Batch System:** HTCondor integration under development
- **Job Templates:** Located in `batch/` directory (currently empty - templates to be added)
- **Resource Management:** Basic resource allocation and monitoring

## Advanced Analysis Features

### Jet Substructure Analysis
- **Grooming Algorithms:** Soft Drop, Z-cut grooming for jet substructure
- **Substructure Variables:** Mass, groomed momentum fraction, splitting scales
- **Comparative Studies:** Multiple grooming parameters in parallel

### Systematic Studies
- **Uncertainty Propagation:** JEC, JER, photon energy scale uncertainties
- **Background Estimation:** Data-driven methods for photon purity
- **Centrality Dependencies:** Medium effects across different collision geometries

### Data Quality and Validation
- **Automated Checks:** Event counts, histogram sanity checks, file integrity
- **Cross-Validation:** Data vs MC comparisons at multiple analysis stages
- **Performance Monitoring:** Processing time and memory usage tracking

## Integration with CMS Software

### CMSSW Compatibility
- **Framework Version:** CMSSW_13_2_13 for Run 3 data processing
- **Data Formats:** HiForest ntuples from centralized production
- **Calibrations:** Latest JEC/JER corrections and photon energy scales

### CMS Data Management
- **Input Data:** Centralized HiForest production on `/eos/cms/`
- **Output Storage:** User space on `/eos/user/` with organized directory structure
- **Backup Strategy:** Critical analysis outputs backed up to CMS DAS

## Development Timeline & Milestones
- **May 2025:** Entire framework (C++ analysis, Python plotting, and all configs) designed, implemented, and documented in a single rapid development sprint. All major features, including batch processing, config-driven analysis, multi-jet/centrality support, overlays, ratio plots, and CMS-style outputs, were completed within this period.

## Major Issues & Lessons Learned
- **ROOT file structure drift:** Early in the sprint, standardization was enforced via config files to ensure compatibility.
- **Batch processing bugs:** Addressed by modularizing job submission and output handling.
- **Documentation drift:** Multiple outdated .md files consolidated into this single source.
- **User feedback:** Led to improved error handling, clearer config options, and better quick start guides.
- **Memory Management:** Careful ROOT object handling to prevent memory leaks in long batch jobs
- **Configuration Complexity:** Balance between flexibility and usability in config file design

## Future Development Plans
- **CMS Integration:** Incorporate into official CMS Heavy Ion analysis workflows
- **Machine Learning:** Jet substructure analysis using deep learning techniques  
- **Real-time Analysis:** Integration with CMS online selection and monitoring
- **Systematic Automation:** Automated systematic uncertainty evaluation
- **Performance Optimization:** GPU acceleration for computationally intensive tasks
- **Analysis Preservation:** Integration with CMS Analysis Preservation service

---

# MEMORY_PLOTTING.md

# CMS Gamma-Jet Analysis Plotting Functions Documentation

This document provides comprehensive documentation of all functions available in the `plotGammaJet.py` script for CMS Heavy Ion gamma-jet analysis plotting.

## Table of Contents

1. [Configuration and Data Classes](#configuration-and-data-classes)
2. [Core Configuration Functions](#core-configuration-functions)
3. [Styling and Color Functions](#styling-and-color-functions)
4. [ROOT File Access Functions](#root-file-access-functions)
5. [Plotting Functions](#plotting-functions)
6. [Data-MC Comparison Functions](#data-mc-comparison-functions)
7. [Utility Functions](#utility-functions)
8. [Main Function](#main-function)
9. [Available Config Options](#available-config-options)
10. [Usage Examples](#usage-examples)

---

## Configuration and Data Classes

### `HistogramConfig`
**Class for histogram configuration management**
- **Purpose**: Holds all styling and plotting parameters for histograms
- **Key attributes**: `name`, `title`, `bins`, `x_min`, `x_max`, `log_y`, `plot_type`, `color`, `line_width`, `draw_option`, `marker_style`, `marker_size`
- **Supports**: Both 1D and 2D histograms with separate configurations

### `ProfileConfig`
**Class for ROOT profile histogram configuration**
- **Purpose**: Configuration for TProfile plotting
- **Key attributes**: `name`, `title`, `x_bins`, `x_min`, `x_max`

### `DataMCConfig`
**Class for Data-MC comparison styling**
- **Purpose**: Manages styling for data vs MC comparison plots
- **Key attributes**: Data styling, MC styling, ratio plot settings, canvas dimensions, legend positioning

---

## Core Configuration Functions

### `parse_config(config_path)`
**Parse configuration file into dictionary**
- **Input**: Path to config file
- **Output**: Dictionary of configuration key-value pairs
- **Format**: Supports `key: value` format with `#` comments

### `get_histogram_configs(config)`
**Extract histogram configurations from config**
- **Input**: Configuration dictionary
- **Output**: Dict of `{plot_key: HistogramConfig}` for all defined histograms
- **Usage**: Primary method to get all available histogram definitions

### `get_profile_configs(config)`
**Extract profile configurations from config**
- **Input**: Configuration dictionary  
- **Output**: Dict of `{plot_key: ProfileConfig}` for all defined profiles

### `get_datamc_config(config)`
**Get Data-MC comparison configuration**
- **Input**: Configuration dictionary
- **Output**: `DataMCConfig` object with all Data-MC styling parameters

### `get_overlay_plots(config)`
**Get list of plots to overlay**
- **Input**: Configuration dictionary
- **Output**: List of plot keys for overlay plotting
- **Config key**: `OverlayPlots`

### `get_plot_formats(config)`
**Get output file formats**
- **Input**: Configuration dictionary
- **Output**: List of formats (e.g., ['pdf', 'png', 'root'])
- **Config key**: `PlotFormats`

---

## Styling and Color Functions

### `get_color_scheme(scheme_name, color_blind=False)`
**Get color palette for plotting**
- **Available schemes**: `petroff6`, `petroff10`, `viridis`, `traditional`, `default`
- **Color-blind support**: Returns accessible colors when `color_blind=True`
- **Output**: List of ROOT color codes

### `set_cms_style(config=None)`
**Set comprehensive CMS plotting style**
- **Purpose**: Applies official CMS style guidelines
- **Features**: Proper fonts (Helvetica), margins, tick marks, text sizes
- **Canvas margins**: Configurable via config file

### `get_auto_color(color_scheme, index, color_blind=False)`
**Get color by index from scheme**
- **Input**: Color scheme name, index, color-blind flag
- **Output**: ROOT color code
- **Usage**: Automatic color assignment for overlays

### `get_marker_styles()` / `get_auto_marker(index)`
**CMS-compliant marker styles**
- **Available markers**: Full/open circles, squares, triangles, diamonds, stars
- **Usage**: Automatic marker assignment for data points

### `apply_histogram_style(hist, hist_config, color_index, config)`
**Apply styling to histogram**
- **Purpose**: Sets colors, markers, line widths based on configuration
- **Features**: Automatic or manual color/marker assignment

---

## ROOT File Access Functions

### `get_available_jet_dirs(root_file, config)`
**Get available jet collections in ROOT file**
- **Input**: ROOT file object, configuration
- **Output**: List of existing jet directories (e.g., ['AK4Z2', 'AK8Z4'])
- **Config key**: `JetDirectories`

### `get_available_centrality_bins(root_file, jet_dir, config)`
**Get available centrality bins**
- **Input**: ROOT file, jet directory, configuration
- **Output**: List of centrality bins (e.g., ['cent0to60', 'cent60to180'])
- **Config key**: `CentralityBins`

### `get_histogram_from_path(root_file, jet_dir, cent_bin, hist_name, config)`
**Get histogram from nested ROOT structure**
- **Purpose**: Navigate nested ROOT file structure to retrieve histograms
- **Supports**: Both nested (`JetDir/CentBin/HistName`) and flat structures
- **Config key**: `UseNestedStructure`

### `load_histogram_from_file(root_file, hist_path)`
**Load histogram with proper memory management**
- **Purpose**: Clone histogram and detach from file
- **Benefits**: Prevents memory issues when closing files

### `find_histogram_in_structure(root_file, hist_name, jet_dir=None, cent_bin=None)`
**Find histogram path in ROOT structure**
- **Purpose**: Search for histogram in various possible locations
- **Output**: Full path to histogram if found, None otherwise

---

## Plotting Functions

### `plot_histogram_1d(hist, hist_config, config, outdir, formats, jet_dir="", cent_bin="")`
**Plot 1D histograms with full CMS styling**
- **Features**: Log/linear scales, CMS labels, selection text, axis configuration
- **Output**: Saves in multiple formats with proper naming

### `plot_histogram_2d(hist, hist_config, config, outdir, formats, jet_dir="", cent_bin="")`
**Plot 2D histograms with color maps**
- **Features**: Color palettes (viridis, plasma, bird), log z-scale, proper margins
- **Config options**: `ColorMap`, `LogZ`, `DrawOption2D`

### `plot_overlay_advanced(hists, labels, config, outdir, name, formats, hist_config=None, jet_dir="", cent_bin="")`
**Advanced overlay plotting with multiple histograms**
- **Features**: Automatic color/marker assignment, configurable legends, scaling
- **Legend positions**: `left`, `right`, `bcenter`, or custom coordinates
- **Config keys**: `Overlay.LegendPosition`, `Legend.Position`

### `plot_centrality_overlay(root_file, jet_dir, hist_name, config, outdir, formats)`
**Create centrality comparison overlays**
- **Purpose**: Compare same histogram across different centrality bins
- **Config key**: `OverlayCentralityBins`

### `plot_jet_collection_overlay(root_file, cent_bin, hist_name, config, outdir, formats)`
**Create jet collection comparison overlays**
- **Purpose**: Compare same histogram across different jet collections
- **Config key**: `OverlayJetCollections`

---

## Data-MC Comparison Functions

### `plot_datamc_comparison(data_file, mc_file, hist_name, config, outdir, formats, jet_dir="", cent_bin="")`
**Create Data-MC comparison with ratio plot**
- **Features**: 
  - Two-panel plot (main comparison + ratio)
  - Configurable styling for data and MC
  - Automatic normalization option
  - Reference line at ratio = 1
- **Config keys**: All `DataMC.*` parameters

### `plot_datamc_overlay_multiple(data_file, mc_file, hist_configs, config, outdir, formats, jet_dir="", cent_bin="")`
**Multiple histogram Data-MC overlay**
- **Purpose**: Compare multiple histogram types between Data and MC in single plot
- **Usage**: For systematic studies

---

## Utility Functions

### `draw_cms_label(canvas, config, selection_text="")`
**Draw official CMS label and experiment info**
- **Features**: CMS logo, preliminary/simulation text, energy, luminosity
- **Config keys**: `CMSLabel`, `CMSEnergyText`, `CMSLuminosity`, `CMSExtraText`

### `get_cms_text(config)`
**Generate CMS text label from configuration**
- **Purpose**: Format CMS label text for display
- **Output**: Formatted string like "CMS Preliminary" or "CMS Simulation"

### `create_output_dirs(base_dir, config, jet_dirs=None)`
**Create organized output directory structure**
- **Structure**: `base/1D/`, `base/2D/`, `base/overlays/`, etc.
- **Jet-specific**: Creates subdirectories for each jet collection

### `get_selection_text(config, jet_dir="", cent_bin="")`
**Generate selection criteria text for plots**
- **Sources**: Configuration parameters or custom text
- **Config keys**: `SelectionText`, `SelectionPhotonPtMin`, `SelectionJetPtMin`, etc.

### `save_canvas(canvas, outdir, name, formats, jet_dir="", cent_bin="")`
**Save canvas in multiple formats**
- **Features**: Automatic filename generation with jet/centrality info
- **Formats**: PDF, PNG, ROOT, etc.

### `get_centrality_label(cent_bin, config=None)` / `get_jet_label(jet_dir)`
**Format display labels**
- **Purpose**: Convert internal names to display-ready labels
- **Examples**: `cent0to60` → `"Centrality 0-30%"`, `AK4Z2` → `"AK R=0.4, Z_cut=0.2"`

### `parse_overlay_jets(config)` / `parse_overlay_centrality(config)`
**Parse overlay configuration**
- **Purpose**: Determine which collections/bins to overlay
- **Config keys**: `OverlayJetCollections`, `OverlayCentralityBins`

---

## Main Function

### `main()`
**Enhanced main function with full Data-MC support**
- **Modes**: 
  - Single file plotting
  - Data-MC comparison mode
  - Test mode (limited plots)
- **Command line options**:
  ```bash
  -r/--rootfile: Input ROOT file (Data file for Data-MC)
  -c/--config: Configuration file
  -o/--outdir: Output directory
  --mc-file: MC file for Data-MC comparison
  --datamc-mode: Enable Data-MC mode
  --datamc-plots: Specific plots to compare
  --batch: Batch mode (no GUI)
  --verbose: Verbose output
  --test: Test mode
  --jet-dir: Specific jet collection
  --cent-bin: Specific centrality bin
  ```

---

## Available Config Options

### Basic Settings
```
EnablePlotting: 1
SavePlots: 1
PlotFormats: pdf,png,root
UseNestedStructure: 1
PlotOutputDir: /path/to/output
```

### CMS Labeling
```
UseCMSStyle: 1
CMSLabel: Preliminary
CMSEnergyText: 5.36 TeV PbPb
CMSLuminosity: 1.72 nb^{-1}
CMSExtraText: Simulation
```

### Colors and Styling
```
ColorScheme: petroff6
UseColorBlind: 1
CanvasWidth: 800
CanvasHeight: 600
CanvasMarginLeft: 0.15
CanvasMarginRight: 0.05
CanvasMarginTop: 0.08
CanvasMarginBottom: 0.12
```

### Jet Collections and Centrality
```
JetDirectories: AK2Z1,AK2Z2,AK3Z1,AK4Z2,AK8Z4
CentralityBins: 0,60,180
```

### Selection Criteria
```
SelectionPhotonPtMin: 60
SelectionPhotonEtaMax: 1.44
SelectionJetPtMin: 40
SelectionJetEtaMax: 2.0
SelectionDeltaPhiMin: 2π/3
SelectionText: Custom selection text
```

### Data-MC Comparison
```
DataMC.DataLabel: Data
DataMC.MCLabel: PYTHIA8
DataMC.DataColor: 1
DataMC.MCColor: 2
DataMC.RatioTitle: Data/MC
DataMC.RatioYMin: 0.5
DataMC.RatioYMax: 1.5
DataMC.Normalize: 1
```

### Histogram Definitions
```
Histogram.JetPt.Name: hJetPt
Histogram.JetPt.Title: Jet p_{T};p_{T} [GeV/c];Entries
Histogram.JetPt.Bins: 100
Histogram.JetPt.XMin: 0
Histogram.JetPt.XMax: 200
Histogram.JetPt.LogY: 1
Histogram.JetPt.PlotType: 1D
Histogram.JetPt.Color: auto
Histogram.JetPt.DrawOption: hist
```

### Overlay Settings
```
OverlayPlots: JetPt,JetEta,DeltaPhi
OverlayJetCollections: AK4Z2,AK8Z4
OverlayCentralityBins: all
OverlayByCentrality: 1
OverlayByJetCollection: 1
```

---

## Usage Examples

### Basic Data-MC Comparison
```bash
python plotGammaJet.py \
  -r /path/to/data.root \
  --mc-file /path/to/mc.root \
  -c DataMC_overlay.config \
  -o /path/to/output \
  --datamc-mode \
  --batch
```

### Test Specific Plots
```bash
python plotGammaJet.py \
  -r /path/to/data.root \
  --mc-file /path/to/mc.root \
  -c DataMC_overlay.config \
  --datamc-plots JetPt,JetEta,DeltaPhi \
  --test \
  --verbose
```

### Single File Plotting
```bash
python plotGammaJet.py \
  -r /path/to/file.root \
  -c PlotJetSub_2023_PbPb_Data.config \
  -o /path/to/output \
  --jet-dir AK4Z2 \
  --cent-bin cent0to60
```

---

## Available Histograms in Config

Based on the `DataMC_overlay.config`, the following histograms are available:

1. **JetPt**: Jet transverse momentum
2. **JetEta**: Jet pseudorapidity  
3. **JetPhi**: Jet azimuthal angle
4. **DeltaPhi**: Angular correlation between photon and jet
5. **Xj**: Momentum balance (p_T^jet / E_T^photon)
6. **PhotonEt**: Photon transverse energy
7. **PhotonEta**: Photon pseudorapidity
8. **JetMass**: Jet mass

Each histogram has full configuration including binning, axis ranges, log scales, draw options, and styling parameters.

---

## File Structure and Paths

The script is designed to work with:
- **Data file**: `/eos/user/b/bharikri/www/Run3GammaJet/2025_05_24/2023_PbPb/Data/2025_05_24_HiRawPrime0_part_output.root`
- **MC file**: `/eos/user/b/bharikri/www/Run3GammaJet/2025_05_24/2023_PbPb/MC/2025_05_24_QCDPhoton30_output.root`
- **Output**: `/eos/user/b/bharikri/www/Run3GammaJet/2025_05_24/2023_PbPb/DataMC_comparison`

The script automatically creates organized subdirectories for different plot types and jet collections.

For a quick start, see `README.md`. For implementation details, see `gammaJetAnalyzer.cpp` and `plotGammaJet.py`.
