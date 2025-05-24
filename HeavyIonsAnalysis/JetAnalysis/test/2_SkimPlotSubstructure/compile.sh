#!/bin/bash

# Compilation script for photonJet.C analysis
# Supports both local compilation and batch system submission
# Author: bharikri
# Created: May 2025

# Default values
MODE="local"
CONFIG=""
VERBOSE=false
PRODUCTION=false
MAX_EVENTS=10000
CLEAN=false
BATCH_SYSTEM="condor"
QUEUE=""
OUTPUT_DIR=""
OS_VERSION="el9"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to show usage
show_usage() {
    cat << EOF
Usage: $0 [OPTIONS]

This script compiles and runs the photonJet.C analysis with proper ROOT and CMSSW dependencies.

OPTIONS:
    -m, --mode MODE         Compilation mode: local, batch (default: local)
    -c, --config FILE       Configuration file path (default: ../configs/photon_only.config)
    -v, --verbose           Enable verbose output
    -p, --production        Run in production mode (all events)
    -t, --test [N]          Run in test mode with N events (default: 10000)
    --clean                 Clean previous compilation artifacts
    --batch-system SYS      Batch system: condor, lsf, slurm (default: condor)
    --queue QUEUE           Queue name for batch submission
    --output-dir DIR        Output directory for batch jobs
    --os-version VER        OS version for Condor: el8, el9 (default: el9)
    -h, --help              Show this help message

EXAMPLES:
    # Compile and run locally in test mode
    $0 --mode local --test 5000

    # Compile and run in production mode
    $0 --mode local --production

    # Submit batch job to Condor (default)
    $0 --mode batch --output-dir /eos/user/b/bharikri/batch_output

    # Submit batch job to Condor with specific OS
    $0 --mode batch --os-version el8 --output-dir /eos/user/b/bharikri/batch_output

    # Submit batch job to LSF
    $0 --mode batch --batch-system lsf --queue 8nh --output-dir /eos/user/b/bharikri/batch_output

    # Clean compilation artifacts
    $0 --clean

    # Use custom config file
    $0 --config /path/to/my/config.config --test 1000

ENVIRONMENT:
    The script automatically detects and sets up:
    - CMSSW environment (if available)
    - ROOT environment
    - Proper include paths and library linking

BATCH SYSTEMS:
    - condor: HTCondor batch system (default, recommended at CERN)
    - lsf: LSF batch system
    - slurm: SLURM batch system

CONDOR OS SELECTION:
    Use --os-version to specify the operating system for Condor jobs:
    - el8: Enterprise Linux 8 (CentOS 8, RHEL 8)
    - el9: Enterprise Linux 9 (CentOS 9, RHEL 9, default)
EOF
}

# Function to check if we're in a CMSSW environment
check_cmssw() {
    if [[ -n "$CMSSW_BASE" ]]; then
        print_info "CMSSW environment detected: $CMSSW_BASE"
        return 0
    else
        print_warning "No CMSSW environment detected"
        return 1
    fi
}

# Function to check ROOT environment
check_root() {
    if command -v root-config &> /dev/null; then
        ROOT_VERSION=$(root-config --version)
        print_info "ROOT version detected: $ROOT_VERSION"
        return 0
    else
        print_error "ROOT not found! Please set up ROOT environment."
        return 1
    fi
}

# Function to clean compilation artifacts
clean_artifacts() {
    print_info "Cleaning compilation artifacts..."
    rm -f photonJet
    rm -f photonJet.exe
    rm -f *.o
    rm -f *.so
    rm -f *.d
    rm -rf AutoDict_*
    print_success "Cleanup completed"
}

# Function to compile the analysis
compile_analysis() {
    print_info "Starting compilation..."
    
    # Get ROOT flags
    ROOT_CFLAGS=$(root-config --cflags)
    ROOT_LIBS=$(root-config --libs)
    
    # Additional ROOT libraries needed
    EXTRA_ROOT_LIBS="-lTree -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lMathCore -lPhysics"
    
    # CMSSW flags (if available)
    CMSSW_FLAGS=""
    if check_cmssw; then
        CMSSW_FLAGS="-I$CMSSW_BASE/src -I$CMSSW_RELEASE_BASE/src"
    fi
    
    # Compiler settings
    CXX="g++"
    CXXFLAGS="-std=c++17 -O2 -Wall -Wextra"
    
    # Include paths
    INCLUDES="-I. -I./include -I../include $ROOT_CFLAGS $CMSSW_FLAGS"
    
    # Libraries
    LIBS="$ROOT_LIBS $EXTRA_ROOT_LIBS"
    
    # Define macros
    DEFINES="-DSTANDALONE_COMPILE"
    
    # Source files
    SOURCES="scripts/photonJet.C"
    
    # Output executable
    OUTPUT="photonJet"
    
    # Compilation command
    COMPILE_CMD="$CXX $CXXFLAGS $INCLUDES $DEFINES $SOURCES $LIBS -o $OUTPUT"
    
    if [[ "$VERBOSE" == true ]]; then
        print_info "Compilation command:"
        echo "$COMPILE_CMD"
        echo ""
    fi
    
    # Execute compilation
    if eval $COMPILE_CMD; then
        print_success "Compilation successful! Executable: $OUTPUT"
        return 0
    else
        print_error "Compilation failed!"
        return 1
    fi
}

# Function to run analysis locally
run_local() {
    local config_file="$1"
    local run_args=""
    
    if [[ "$PRODUCTION" == true ]]; then
        run_args="--production"
        print_info "Running in production mode (all events)"
    else
        run_args="--test $MAX_EVENTS"
        print_info "Running in test mode with $MAX_EVENTS events"
    fi
    
    if [[ -n "$config_file" ]]; then
        # Convert to absolute path if it's not already
        if [[ ! "$config_file" = /* ]]; then
            config_file="$(cd "$(dirname "$config_file")" && pwd)/$(basename "$config_file")"
        fi
        run_args="$run_args --config $config_file"
        print_info "Using config file: $config_file"
    fi
    
    print_info "Starting analysis..."
    print_info "Running command: ./photonJet $run_args"
    ./photonJet $run_args
    
    if [[ $? -eq 0 ]]; then
        print_success "Analysis completed successfully!"
    else
        print_error "Analysis failed!"
        return 1
    fi
}

# Function to create isolated job directory with all required files
create_job_directory() {
    local config_file="$1"
    local job_timestamp="$2"
    local config_name=""
    
    # Extract config name from path
    if [[ -n "$config_file" ]]; then
        config_name=$(basename "$config_file" .config)
    else
        config_name="default"
    fi
    
    # Create job directory structure
    local job_dir="batch/job_${job_timestamp}_${config_name}"
    mkdir -p "$job_dir"
    mkdir -p "$job_dir/include"
    mkdir -p "$job_dir/scripts"
    mkdir -p "$job_dir/logs"
    
    print_info "Creating isolated job directory: $job_dir"
    
    # Copy executable
    if [[ -f "./photonJet" ]]; then
        cp "./photonJet" "$job_dir/"
        print_info "Copied executable to job directory"
    else
        print_error "Executable photonJet not found!"
        return 1
    fi
    
    # Copy config file if specified
    if [[ -n "$config_file" && -f "$config_file" ]]; then
        cp "$config_file" "$job_dir/$(basename "$config_file")"
        print_info "Copied config file: $(basename "$config_file")"
    fi
    
    # Copy essential header files
    local header_files=(
        "./include/helpers.h"
        "./include/GammaJetAnalysis.h"
        "./include/JetSubstructure.h"
        "./include/photonJet.h"
    )
    
    if [[ "$config_file" == *"MC"* ]]; then
        header_files+=("./include/GammaJet2023_PbPbMC.h")
        header_files+=("./include/GammaJet2023_PbPbMC.C")
    else
        header_files+=("./include/GammaJet2023_PbPbData.h")
        header_files+=("./include/GammaJet2023_PbPbData.C")
    fi
    
    for header in "${header_files[@]}"; do
        if [[ -f "$header" ]]; then
            cp "$header" "$job_dir/include/"
            print_info "Copied header: $(basename "$header")"
        fi
    done
    
    # Copy the original source file for reference
    if [[ -f "./scripts/photonJet.C" ]]; then
        cp "./scripts/photonJet.C" "$job_dir/scripts/"
        print_info "Copied source file for reference"
    fi
    
    echo "$job_dir"
}

# Function to create batch submission script
create_batch_script() {
    local job_dir="$1"
    local config_file="$2"
    local batch_script="$job_dir/run_photonJet_batch.sh"
    
    cat > $batch_script << EOF
#!/bin/bash

# Batch job script for photonJet analysis
# Auto-generated by compile.sh
# Date: $(date)

echo "======================================"
echo "PhotonJet Analysis Batch Job"
echo "======================================"
echo "Starting batch job at: \$(date)"
echo "Hostname: \$(hostname)"
echo "Working directory: \$(pwd)"
echo "User: \$(whoami)"
echo "Available memory: \$(free -h | grep '^Mem')"
echo "Available disk space: \$(df -h . | tail -1)"

# Print environment info
echo ""
echo "Environment Information:"
echo "CONDOR_CLUSTER_ID: \$CONDOR_CLUSTER_ID"
echo "CONDOR_PROCESS_ID: \$CONDOR_PROCESS_ID"
echo "HTCondor slot: \$_CONDOR_SLOT"

# Set up CMSSW environment if available
if [[ -n "\$CMSSW_BASE" ]]; then
    echo ""
    echo "Setting up CMSSW environment..."
    echo "CMSSW_BASE: \$CMSSW_BASE"
    echo "CMSSW_VERSION: \$CMSSW_VERSION"
    cd \$CMSSW_BASE/src
    eval \`scramv1 runtime -sh\`
    cd -
    echo "CMSSW environment setup completed"
else
    echo ""
    echo "No CMSSW environment found, continuing with system ROOT..."
fi

# Check if ROOT is available
if command -v root-config &> /dev/null; then
    echo "ROOT version: \$(root-config --version)"
else
    echo "ERROR: ROOT not found in PATH"
    exit 1
fi

# Make executable if it exists
if [[ -f "./photonJet" ]]; then
    chmod +x ./photonJet
    echo "PhotonJet executable found and made executable"
else
    echo "ERROR: PhotonJet executable not found"
    ls -la
    exit 1
fi

echo ""
echo "Starting photonJet analysis..."
echo "======================================"
EOF

    # Add the actual run command
    if [[ "$PRODUCTION" == true ]]; then
        echo "./photonJet --production" >> $batch_script
    else
        echo "./photonJet --test $MAX_EVENTS" >> $batch_script
    fi
    
    if [[ -n "$config_file" ]]; then
        echo "# Config file: $(basename "$config_file")" >> $batch_script
        # If config file is specified, add it to the command using just the basename
        sed -i "s|./photonJet|./photonJet --config $(basename "$config_file")|" $batch_script
    fi
    
    cat >> $batch_script << EOF

echo ""
echo "======================================"
echo "Analysis completed with exit code: \$?"
echo "Job finished at: \$(date)"
echo "Final working directory contents:"
ls -la
echo "======================================"
EOF

    chmod +x $batch_script
    print_success "Created batch script: $batch_script"
}

# Function to submit to LSF
submit_lsf() {
    local config_file="$1"
    local job_timestamp="$(date +%Y%m%d_%H%M%S)"
    
    # Create isolated job directory
    local job_dir=$(create_job_directory "$config_file" "$job_timestamp")
    if [[ $? -ne 0 ]]; then
        print_error "Failed to create job directory"
        return 1
    fi
    
    # Create batch script in the job directory
    create_batch_script "$job_dir" "$config_file"
    
    local job_name="photonJet_${job_timestamp}"
    local log_dir="$job_dir/logs"
    
    # Change to job directory for submission
    pushd "$job_dir" > /dev/null
    
    local bsub_cmd="bsub -J $job_name -q ${QUEUE:-8nh} -o logs/${job_name}.out -e logs/${job_name}.err ./run_photonJet_batch.sh"
    
    print_info "Submitting to LSF queue: ${QUEUE:-8nh}"
    print_info "Job name: $job_name"
    print_info "Job directory: $job_dir"
    print_info "Logs will be in: $log_dir"
    
    if [[ "$VERBOSE" == true ]]; then
        print_info "LSF command: $bsub_cmd"
    fi
    
    eval $bsub_cmd
    local submit_status=$?
    
    popd > /dev/null
    
    if [[ $submit_status -eq 0 ]]; then
        print_success "LSF job submitted successfully"
        print_info "Job directory preserved at: $job_dir"
    else
        print_error "LSF job submission failed"
        return 1
    fi
}

# Function to submit to Condor
submit_condor() {
    local config_file="$1"
    local job_timestamp="$(date +%Y%m%d_%H%M%S)"
    
    # Create isolated job directory
    local job_dir=$(create_job_directory "$config_file" "$job_timestamp")
    if [[ $? -ne 0 ]]; then
        print_error "Failed to create job directory"
        return 1
    fi
    
    # Create batch script in the job directory
    create_batch_script "$job_dir" "$config_file"
    
    local job_name="photonJet_${job_timestamp}"
    local log_dir="$job_dir/logs"
    
    # Change to job directory for submission
    pushd "$job_dir" > /dev/null
    
    # Create Condor submit file following CERN batch documentation
    local condor_file="${job_name}.sub"
    cat > $condor_file << EOF
# Condor submit file for photonJet analysis
# Following CERN batch system guidelines: https://batchdocs.web.cern.ch/local/submit.html

universe = vanilla
executable = run_photonJet_batch.sh
output = logs/${job_name}.out
error = logs/${job_name}.err
log = logs/${job_name}.log

# OS requirements
OpSysAndVer = "$OS_VERSION"
MY.WantOS = "$OS_VERSION"

# Job requirements
RequestCpus = 1
RequestMemory = 2000
RequestDisk = 1000000

# File transfer
should_transfer_files = YES
when_to_transfer_output = ON_EXIT
EOF

    # Build transfer input files list - all files are now local in job directory
    local transfer_files="photonJet, run_photonJet_batch.sh, include/"
    if [[ -n "$config_file" && -f "$(basename "$config_file")" ]]; then
        transfer_files="$transfer_files, $(basename "$config_file")"
        print_info "Including config file in transfer: $(basename "$config_file")"
    fi
    
    # Add scripts directory if it exists
    if [[ -d "scripts" ]]; then
        transfer_files="$transfer_files, scripts/"
    fi
    
    echo "transfer_input_files = $transfer_files" >> $condor_file
    
    cat >> $condor_file << EOF

# CERN-specific settings
+AccountingGroup = "group_u_CMS.CAF.ALCA"
EOF

    # Add queue specification if provided
    if [[ -n "$QUEUE" ]]; then
        echo "+JobFlavour = \"$QUEUE\"" >> $condor_file
    else
        echo "+JobFlavour = \"espresso\"" >> $condor_file
    fi
    
    echo "queue" >> $condor_file
    
    print_info "Submitting to Condor"
    print_info "Job name: $job_name"
    print_info "Job directory: $job_dir"
    print_info "Submit file: $condor_file"
    print_info "OS version: $OS_VERSION"
    print_info "Job flavour: ${QUEUE:-espresso}"
    print_info "Logs will be in: $log_dir"
    
    if [[ "$VERBOSE" == true ]]; then
        print_info "Condor submit file contents:"
        cat $condor_file
        print_info "Job directory contents:"
        ls -la
    fi
    
    condor_submit $condor_file
    local submit_status=$?
    
    # Return to original directory
    popd > /dev/null
    
    if [[ $submit_status -eq 0 ]]; then
        print_success "Job submitted successfully"
        print_info "Monitor job with: condor_q"
        print_info "Job directory: $job_dir"
    else
        print_error "Job submission failed"
        return 1
    fi
}

# Function to submit to SLURM
submit_slurm() {
    local config_file="$1"
    local job_timestamp="$(date +%Y%m%d_%H%M%S)"
    
    # Create isolated job directory
    local job_dir=$(create_job_directory "$config_file" "$job_timestamp")
    if [[ $? -ne 0 ]]; then
        print_error "Failed to create job directory"
        return 1
    fi
    
    # Create batch script in the job directory
    create_batch_script "$job_dir" "$config_file"
    
    local job_name="photonJet_${job_timestamp}"
    local log_dir="$job_dir/logs"
    
    # Change to job directory for submission
    pushd "$job_dir" > /dev/null
    
    local sbatch_cmd="sbatch --job-name=$job_name --partition=${QUEUE:-batch} --output=logs/${job_name}.out --error=logs/${job_name}.err ./run_photonJet_batch.sh"
    
    print_info "Submitting to SLURM partition: ${QUEUE:-batch}"
    print_info "Job name: $job_name"
    print_info "Job directory: $job_dir"
    print_info "Logs will be in: $log_dir"
    
    if [[ "$VERBOSE" == true ]]; then
        print_info "SLURM command: $sbatch_cmd"
        print_info "Job directory contents:"
        ls -la
    fi
    
    eval $sbatch_cmd
    local submit_status=$?
    
    # Return to original directory
    popd > /dev/null
    
    if [[ $submit_status -eq 0 ]]; then
        print_success "Job submitted successfully"
        print_info "Monitor job with: squeue -u $(whoami)"
        print_info "Job directory: $job_dir"
    else
        print_error "Job submission failed"
        return 1
    fi
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -m|--mode)
            MODE="$2"
            shift 2
            ;;
        -c|--config)
            CONFIG="$2"
            shift 2
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -p|--production)
            PRODUCTION=true
            shift
            ;;
        -t|--test)
            PRODUCTION=false
            if [[ $2 =~ ^[0-9]+$ ]]; then
                MAX_EVENTS="$2"
                shift 2
            else
                MAX_EVENTS=10000
                shift
            fi
            ;;
        --clean)
            CLEAN=true
            shift
            ;;
        --batch-system)
            BATCH_SYSTEM="$2"
            shift 2
            ;;
        --queue)
            QUEUE="$2"
            shift 2
            ;;
        --output-dir)
            OUTPUT_DIR="$2"
            shift 2
            ;;
        --os-version)
            OS_VERSION="$2"
            shift 2
            ;;
        -h|--help)
            show_usage
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

# Main execution
print_info "PhotonJet Analysis Compilation Script"
print_info "======================================"

# Clean if requested
if [[ "$CLEAN" == true ]]; then
    clean_artifacts
    exit 0
fi

# Check environments
if ! check_root; then
    exit 1
fi

check_cmssw

# Set default config if not provided
if [[ -z "$CONFIG" ]]; then
    # Get the directory where this script is located
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    
    # Try multiple possible config locations with absolute paths
    CONFIG_CANDIDATES=(
        "$SCRIPT_DIR/../configs/photon_only.config"
        "$SCRIPT_DIR/configs/photon_only.config"
        "$SCRIPT_DIR/../../configs/photon_only.config"
        "/afs/cern.ch/user/b/bharikri/private/HeavyIon/run3_gamma_jet/CMSSW_13_2_13/src/HeavyIonsAnalysis/JetAnalysis/test/configs/photon_only.config"
    )
    
    for config_candidate in "${CONFIG_CANDIDATES[@]}"; do
        if [[ -f "$config_candidate" ]]; then
            CONFIG="$config_candidate"
            print_info "Found config file: $CONFIG"
            break
        fi
    done
    
    if [[ -z "$CONFIG" ]]; then
        print_warning "No default config file found. Tried:"
        for config_candidate in "${CONFIG_CANDIDATES[@]}"; do
            print_warning "  $config_candidate"
        done
        print_warning "Analysis will run with default parameters"
    fi
fi

# Validate config file exists if specified
if [[ -n "$CONFIG" ]] && [[ ! -f "$CONFIG" ]]; then
    print_error "Config file not found: $CONFIG"
    exit 1
fi

# Validate mode
case $MODE in
    local)
        print_info "Mode: Local compilation and execution"
        ;;
    batch)
        print_info "Mode: Batch submission ($BATCH_SYSTEM)"
        print_info "OS version: $OS_VERSION"
        
        # Validate OS version for Condor
        if [[ "$BATCH_SYSTEM" == "condor" ]]; then
            case $OS_VERSION in
                el8|el9)
                    print_info "Using OS version: $OS_VERSION"
                    ;;
                *)
                    print_error "Invalid OS version: $OS_VERSION. Supported: el8, el9"
                    exit 1
                    ;;
            esac
        fi
        ;;
    *)
        print_error "Invalid mode: $MODE"
        exit 1
        ;;
esac

# Compile the analysis
if ! compile_analysis; then
    exit 1
fi

# Execute based on mode
case $MODE in
    local)
        run_local "$CONFIG"
        ;;
    batch)
        case $BATCH_SYSTEM in
            lsf)
                submit_lsf "$CONFIG"
                ;;
            condor)
                submit_condor "$CONFIG"
                ;;
            slurm)
                submit_slurm "$CONFIG"
                ;;
            *)
                print_error "Unsupported batch system: $BATCH_SYSTEM"
                exit 1
                ;;
        esac
        ;;
esac

print_success "Script completed successfully!"
