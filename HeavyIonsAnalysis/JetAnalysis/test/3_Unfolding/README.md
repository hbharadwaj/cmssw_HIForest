# Step 3: Jet Substructure Unfolding

This directory implements **multi-dimensional unfolding** of jet substructure observables for CMS Heavy Ion gamma-jet analysis using **TUnfold**.

## Quick Start

```bash
# Setup environment (CRITICAL - for ROOT 6.26.11 with TUnfold)
cmsenv

# Run unfolding (1D, 2D, and 3D)
./run_tunfold.sh
```

## Features Implemented

✅ **1D Unfolding**: Jet pT  
✅ **2D Unfolding**: Jet pT vs Jet Girth  
✅ **3D Unfolding**: Photon ET vs Jet pT vs Jet Girth  
✅ **Complete Pipeline**: Data processing → Response matrices → Unfolding → Output  
✅ **Production Ready**: Tested with 2023 PbPb data/MC  

## Key Files

- `TUnfoldJetSubstructure.C` - Main unfolding implementation
- `run_tunfold.sh` - Execution script  
- `DOCUMENTATION.md` - **Complete technical documentation**
- `../configs/UnfoldJetSub_xj_test.config` - Configuration file

## Critical Dependencies

⚠️ **ROOT Version**: TUnfold **removed in ROOT 6.30+**  
✅ **Solution**: Use `cmsenv` to get ROOT 6.26.11

## Results

Output saved to: `/eos/user/b/bharikri/www/Run3GammaJet/2025_06_15/2023_PbPb/Unfolding/`

Latest successful run:
- **1D**: 10,406 unfolded events (Jet pT)
- **2D**: 10,394 unfolded events (Jet pT vs Girth) 
- **3D**: 10,394 unfolded events (Photon ET vs Jet pT vs Girth)

---

📖 **For complete documentation, implementation details, and future plans, see [`DOCUMENTATION.md`](DOCUMENTATION.md)**
