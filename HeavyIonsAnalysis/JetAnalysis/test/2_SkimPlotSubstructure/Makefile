# Makefile for BranchManager testing and Gamma Jet Analysis
# CMS Heavy Ion Analysis Framework
# May 24, 2025

# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -g

# ROOT configuration
ROOTCFLAGS = $(shell root-config --cflags)
ROOTLIBS = $(shell root-config --libs)

# Include directories
INCLUDES = -I. -I$(ROOTSYS)/include

# Target executables
TARGETS = gammaJetAnalyzer

# Source files
SOURCES = include/BranchManager.cpp include/DataFrameAnalyzer.cpp
HEADERS = include/BranchManager.h include/DataFrameAnalyzer.h
GAMMA_JET_SOURCES = UnifiedDataReader.cpp
GAMMA_JET_HEADERS = include/FormatDetector.h include/BranchMapper.h include/UnifiedDataReader.h

# Default target
all: $(TARGETS)

# # Test executable
# test_BranchManager: test_BranchManager.cpp $(SOURCES) $(HEADERS)
# 	$(CXX) $(CXXFLAGS) $(INCLUDES) $(ROOTCFLAGS) -o $@ test_BranchManager.cpp include/BranchManager.cpp $(ROOTLIBS)

# # Simple DataFrameAnalyzer test executable  
# test_DataFrameAnalyzer_simple: test_DataFrameAnalyzer_simple.cpp $(SOURCES) $(HEADERS)
# 	$(CXX) $(CXXFLAGS) $(INCLUDES) $(ROOTCFLAGS) -o $@ test_DataFrameAnalyzer_simple.cpp include/BranchManager.cpp $(ROOTLIBS)

# # Full DataFrameAnalyzer test executable
# test_DataFrameAnalyzer: test_DataFrameAnalyzer.cpp $(SOURCES) $(HEADERS)
# 	$(CXX) $(CXXFLAGS) $(INCLUDES) $(ROOTCFLAGS) -o $@ test_DataFrameAnalyzer.cpp $(SOURCES) $(ROOTLIBS)

# Gamma Jet Analyzer executable (standalone C++ executable)
gammaJetAnalyzer: gammaJetAnalyzer.cpp $(GAMMA_JET_SOURCES) $(GAMMA_JET_HEADERS)
	@echo "Compiling gammaJetAnalyzer as standalone C++ executable..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(ROOTCFLAGS) -o $@ gammaJetAnalyzer.cpp $(GAMMA_JET_SOURCES) $(ROOTLIBS)
	@echo "gammaJetAnalyzer compilation successful!"

# Clean target
clean:
	rm -f $(TARGETS) *.o
	rm -f *_C.d *_C.so *_C_ACLiC_dict_rdict.pcm
	rm -f include/*_cpp.d include/*_cpp.so include/*_cpp_ACLiC_dict_rdict.pcm

# Run tests
test: test_BranchManager test_DataFrameAnalyzer_simple
	@echo "Running BranchManager tests..."
	./test_BranchManager
	@echo ""
	@echo "Running Simple DataFrameAnalyzer integration tests..."
	./test_DataFrameAnalyzer_simple

# Test gamma jet analyzer compilation
test-gamma-jet: gammaJetAnalyzer
	@echo "Gamma jet analyzer compilation test completed successfully!"
	@echo "To run the analyzer:"
	@echo "  ./gammaJetAnalyzer -c ../configs/JetSub_2023_PbPb_MC.config -p ../configs/plot.config -t 1000"
	@echo "  ./gammaJetAnalyzer --config ../configs/JetSub_2023_PbPb_MC.config --plot-config ../configs/plot.config --test 1000"
	@echo "  ./gammaJetAnalyzer -c ../configs/analysis.config -P  # Production mode"
	@echo "  ./gammaJetAnalyzer --help  # Show usage information"

# Run gamma jet analyzer with default config (if available)
run-gamma-jet: gammaJetAnalyzer
	@if [ -f ../configs/JetSub_2023_PbPb_MC.config ]; then \
		echo "Running gammaJetAnalyzer with default config..."; \
		./gammaJetAnalyzer -c ../configs/JetSub_2023_PbPb_MC.config -t 1000; \
	else \
		echo "Default config file not found. Please specify config:"; \
		echo "  ./gammaJetAnalyzer -c path/to/config.config -t 1000"; \
	fi

# Debug build
debug: CXXFLAGS += -DDEBUG -O0
debug: all

# Help target
help:
	@echo "Available targets:"
	@echo "  all          - Build all executables"
	@echo "  test         - Build and run tests"
	@echo "  test-gamma-jet - Test gamma jet analyzer compilation"
	@echo "  run-gamma-jet  - Build and run gamma jet analyzer with default config"
	@echo "  gammaJetAnalyzer - Build gamma jet analyzer"
	@echo "  clean        - Remove built files"
	@echo "  debug        - Build with debug flags"
	@echo "  help         - Show this help"

.PHONY: all clean test test-gamma-jet run-gamma-jet debug help
