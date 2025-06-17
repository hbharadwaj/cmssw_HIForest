#!/bin/bash

# Script to run TUnfold-based jet substructure unfolding
# This script runs the ROOT macro without external dependencies

echo "=== TUnfold Jet Substructure Unfolding ==="
echo "Running TUnfold macro..."

# Run ROOT with the macro
root -l << 'EOF'
.x TUnfoldJetSubstructure.C("../configs/UnfoldJetSub_xj_test.config")
.q
EOF

echo ""
echo "=== TUnfold unfolding completed ==="

# Check if output file was created
OUTPUT_FILE="/eos/user/b/bharikri/www/Run3GammaJet/2025_06_15/2023_PbPb/Unfolding/2025_06_15_Unfolded_tunfold.root"
if [ -f "$OUTPUT_FILE" ]; then
    echo "Output file created successfully: $OUTPUT_FILE"
    echo "Checking contents:"
    root -l -b -q -e "TFile *f = TFile::Open(\"$OUTPUT_FILE\"); f->ls(); f->cd(\"Unfolding1D\"); gDirectory->ls(); f->cd(\"../Unfolding2D\"); gDirectory->ls(); f->cd(\"../Unfolding3D\"); gDirectory->ls();"
else
    echo "Warning: Output file not found at $OUTPUT_FILE"
fi
