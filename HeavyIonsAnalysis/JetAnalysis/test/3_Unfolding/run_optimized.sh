#!/bin/bash

# Test script for optimized RooUnfold implementation
# Ensures proper RooUnfold library loading

echo "=== Testing Optimized RooUnfold Implementation ==="

# Add RooUnfold library path
export LD_LIBRARY_PATH=$PWD/RooUnfold:$LD_LIBRARY_PATH

# Run ROOT with the optimized macro
root -l << 'EOF'
gSystem->Load("./RooUnfold_2_0_0/libRooUnfold.so");
.x RooUnfoldOptimized.C("../configs/UnfoldJetSub_xj_test.config")
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
OUTPUT_DIR=$(grep "^default.OutputDir" ../configs/UnfoldJetSub_xj_test.config | awk '{for(i=2;i<=NF;i++) printf "%s ", $i; print ""}' | sed 's/[[:space:]]*$//')
OUTPUT_PREFIX=$(grep "^default.OutputPrefix" ../configs/UnfoldJetSub_xj_test.config | awk '{for(i=2;i<=NF;i++) printf "%s ", $i; print ""}' | sed 's/[[:space:]]*$//')

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
