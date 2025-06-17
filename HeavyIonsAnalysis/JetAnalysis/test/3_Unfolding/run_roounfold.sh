#!/bin/bash

# Script to run RooUnfold-based jet substructure unfolding as a ROOT macro
# This script runs the ROOT macro in the correct CMSSW environment

echo "=== RooUnfold Jet Substructure Unfolding ==="
echo "Running RooUnfold macro..."

# Run ROOT with the macro
root -l << 'EOF'
gSystem->Load("./RooUnfold/libRooUnfold.so");
.x RooUnfoldJetSubstructure.C("../configs/UnfoldJetSub_xj_test.config")
.q
EOF

echo ""
echo "=== RooUnfold unfolding completed ==="

# Check if output file was created
OUTPUT_FILE="/eos/user/b/bharikri/www/Run3GammaJet/2025_06_15/2023_PbPb/Unfolding/2025_06_15_Unfolded_roounfold.root"
if [ -f "$OUTPUT_FILE" ]; then
    echo "Output file created successfully: $OUTPUT_FILE"
    echo "Checking contents:"
    root -l -b -q -e "TFile *f = TFile::Open(\"$OUTPUT_FILE\"); f->ls(); f->cd(\"Unfolding1D\"); gDirectory->ls();"
else
    echo "Warning: Output file not found at $OUTPUT_FILE"
fi
