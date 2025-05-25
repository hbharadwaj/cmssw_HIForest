#!/bin/bash
# compile.sh
#
# Compile the gamma jet analyzer with ROOT ACLiC
#
# Usage: ./compile.sh

echo "Compiling gammaJetAnalyzer with dependencies..."
echo "Using ROOT ACLiC to compile the main analyzer with all dependencies..."
root -l -b -q 'gammaJetAnalyzer.C+g'

if [ $? -eq 0 ]; then
    echo "Compilation successful!"
    echo ""
    echo "To run the analyzer:"
    echo "  ./gammaJetAnalyzer -c /path/to/config.config"
    echo "  ./gammaJetAnalyzer -c /path/to/config.config -t 1000 (test mode)"
else
    echo "Compilation failed!"
fi
