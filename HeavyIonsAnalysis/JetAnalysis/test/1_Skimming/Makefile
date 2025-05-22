# Makefile for HiForest Skimming with Batch Processing

# Compiler settings
CXX = g++
CXXFLAGS = -O2 -g -Wall -fPIC
ROOTFLAGS = $(shell root-config --cflags)
ROOTLIBS = $(shell root-config --libs)
ROOTGLIBS = $(shell root-config --glibs)
INCLUDES = -I$(shell root-config --incdir)

# Source files
SOURCES = SkimHiForest.C
EXECUTABLE = SkimHiForest

# Default target
all: $(EXECUTABLE)

# Build the executable
$(EXECUTABLE): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(ROOTFLAGS) $(INCLUDES) -o $@ $< $(ROOTLIBS) $(ROOTGLIBS)

# Build the shared library for ROOT
lib: $(SOURCES)
	$(CXX) $(CXXFLAGS) $(ROOTFLAGS) $(INCLUDES) -shared -o lib$(EXECUTABLE).so $< $(ROOTLIBS) $(ROOTGLIBS)

# Clean the build
clean:
	rm -f $(EXECUTABLE) lib$(EXECUTABLE).so *.o *.d

# Run with default config
run:
	./$(EXECUTABLE) ../configs/batch_processing_sample.config

# Run with specific config
run-%:
	./$(EXECUTABLE) ../configs/$*.config

# Submit to condor
submit:
	./submit_condor_jobs.sh -c ../configs/batch_processing_sample.config

# Submit with specific config
submit-%:
	./submit_condor_jobs.sh -c ../configs/$*.config

# Merge batch outputs
merge:
	./merge_batch_outputs.sh -c ../configs/batch_processing_sample.config

# Merge outputs from specific config
merge-%:
	./merge_batch_outputs.sh -c ../configs/$*.config

# Help target
help:
	@echo "Makefile for HiForest Skimming with Batch Processing"
	@echo ""
	@echo "Targets:"
	@echo "  all                Build the executable"
	@echo "  lib                Build a shared library for ROOT"
	@echo "  clean              Clean build artifacts"
	@echo "  run                Run with default config"
	@echo "  run-CONFIG         Run with specified CONFIG file"
	@echo "  submit             Submit condor jobs with default config"
	@echo "  submit-CONFIG      Submit condor jobs with specified CONFIG file"
	@echo "  merge              Merge batch outputs with default config"
	@echo "  merge-CONFIG       Merge batch outputs with specified CONFIG file"
	@echo ""
	@echo "Examples:"
	@echo "  make run-2023_PbPb_QCDPhoton"
	@echo "  make submit-batch_processing_sample"
	@echo "  make merge-batch_processing_sample"

.PHONY: all lib clean run submit merge help
