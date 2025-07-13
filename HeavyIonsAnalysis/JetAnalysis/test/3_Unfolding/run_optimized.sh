#!/bin/bash

# Test script for optimized RooUnfold implementation
# Ensures proper RooUnfold library loading
#
# Usage:
#   ./run_optimized.sh [config_file]
#   Optionally set ROOUNFOLD_VERSION to 2_0_0 or 3_0_0 (default: 2_0_0)
#   Example: ROOUNFOLD_VERSION=3_0_0 ./run_optimized.sh ../configs/UnfoldJetSub_xj_test.config

CONFIG_FILE="${1:-../configs/UnfoldJetSub_xj_test.config}"
if [ ! -f "$CONFIG_FILE" ]; then
    echo "❌ Config file '$CONFIG_FILE' not found. Please provide a valid config file as the first argument."
    exit 1
fi

ROOUNFOLD_VERSION="${ROOUNFOLD_VERSION:-2_0_0}"
ROOUNFOLD_DIR="RooUnfold_${ROOUNFOLD_VERSION}"
ROOUNFOLD_SO="libRooUnfold.so"

if [ ! -d "$PWD/$ROOUNFOLD_DIR" ]; then
    echo "❌ RooUnfold directory '$PWD/$ROOUNFOLD_DIR' not found. Please check ROOUNFOLD_VERSION."
    exit 1
fi
if [ ! -f "$PWD/$ROOUNFOLD_DIR/$ROOUNFOLD_SO" ]; then
    echo "❌ RooUnfold library '$PWD/$ROOUNFOLD_DIR/$ROOUNFOLD_SO' not found."
    exit 1
fi

export LD_LIBRARY_PATH=$PWD/$ROOUNFOLD_DIR:$LD_LIBRARY_PATH

echo "=== Testing Optimized RooUnfold Implementation (version: $ROOUNFOLD_VERSION) ==="

echo "Using RooUnfold library: $PWD/$ROOUNFOLD_DIR/$ROOUNFOLD_SO"
echo "Using config file: $CONFIG_FILE"

# Run ROOT with the optimized macro
root -l << EOF
gSystem->Load("./$ROOUNFOLD_DIR/$ROOUNFOLD_SO");
.x RooUnfoldOptimized.C("$CONFIG_FILE")
.q
EOF
ROOT_EXIT_CODE=$?

if [ $ROOT_EXIT_CODE -ne 0 ]; then
    echo "❌ ROOT macro execution failed with exit code $ROOT_EXIT_CODE. Check for errors above."
    exit $ROOT_EXIT_CODE
fi

echo "=== Optimized RooUnfold test completed ==="

# Check if output file was created in the correct output directory
# Extract output directory and filename from config (handle space-separated format)
OUTPUT_DIR=$(grep "^default.OutputDir" "$CONFIG_FILE" | awk '{for(i=2;i<=NF;i++) printf "%s ", $i; print ""}' | sed 's/[[:space:]]*$//')
OUTPUT_PREFIX=$(grep "^default.OutputPrefix" "$CONFIG_FILE" | awk '{for(i=2;i<=NF;i++) printf "%s ", $i; print ""}' | sed 's/[[:space:]]*$//')

# Set defaults if not found
if [ -z "$OUTPUT_DIR" ]; then
    OUTPUT_DIR="./"
fi

if [ -z "$OUTPUT_PREFIX" ]; then
    OUTPUT_PREFIX="roounfold"
fi

# Clean up path separators
if [[ "$OUTPUT_DIR" != */ ]]; then
    OUTPUT_DIR="${OUTPUT_DIR}/"
fi

OUTPUT_FILE="${OUTPUT_DIR}${OUTPUT_PREFIX}_optimized_results.root"

echo "Detected output directory: '$OUTPUT_DIR'"
echo "Detected output prefix: '$OUTPUT_PREFIX'"
echo "Looking for output file: $OUTPUT_FILE"

if [ -f "$OUTPUT_FILE" ]; then
    echo "✅ Output file created successfully in configured directory: $OUTPUT_FILE"
    echo "Checking contents:"
    root -l -b -q << EOF
TFile* f = TFile::Open("$OUTPUT_FILE");
if (f && !f->IsZombie()) {
    f->ls();
    f->Close();
} else {
    std::cout << "Failed to open file" << std::endl;
}
.q
EOF
else
    echo "❌ Output file not found in expected location: $OUTPUT_FILE"
    echo "Checking if files are accessible via different methods..."
    # Try to find files that match the expected pattern
    echo "Files matching pattern in expected directory:"
    find "$OUTPUT_DIR" -name "*_optimized_results.root" 2>/dev/null || echo "Directory not accessible or no matching files"
fi
