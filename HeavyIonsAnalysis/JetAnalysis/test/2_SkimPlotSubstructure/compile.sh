#!/bin/bash
# PhotonJet Analysis Compilation Script - Streamlined version
# Supports local compilation and Condor batch submission

# Default values and colors
MODE="local"; CONFIG=""; VERBOSE=false; PRODUCTION=false; MAX_EVENTS=10000; CLEAN=false; DRY_RUN=false
OS_VERSION="$(grep -q "VERSION_ID.*8" /etc/os-release 2>/dev/null && echo el8 || echo el9)"
RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'; BLUE='\033[0;34m'; NC='\033[0m'

# Print functions
print_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
print_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
print_warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }
print_error() { echo -e "${RED}[ERROR]${NC} $1"; }

# Usage and help
show_usage() {
    cat << EOF
Usage: $0 [OPTIONS]
OPTIONS:
    -m, --mode MODE     local or batch (default: local)
    -c, --config FILE   Configuration file path
    -v, --verbose       Enable verbose output
    -p, --production    Run in production mode (all events)
    -t, --test [N]      Run in test mode with N events (default: 10000)
    --clean             Clean compilation artifacts
    --dry-run           Show what would be executed
    --os-version VER    OS version for batch: el8, el9 (default: auto-detect)
    -h, --help          Show this help

EXAMPLES:
    $0 --mode local --test 5000
    $0 --mode batch --production
    $0 --clean
EOF
}

# Setup environment and compile
setup_and_compile() {
    print_info "Setting up environment and compiling..."
    
    # Setup environment
    if [[ -n "$CMSSW_BASE" ]]; then
        print_info "Using CMSSW environment: $CMSSW_BASE"
    else
        print_info "Setting up LCG environment"
        local lcg_path="/cvmfs/sft.cern.ch/lcg/views/LCG_104/x86_64-${OS_VERSION}-gcc11-opt/setup.sh"
        [[ -f "$lcg_path" ]] && source "$lcg_path"
    fi
    
    # Verify ROOT
    if ! command -v root-config &>/dev/null; then
        print_error "ROOT not found"; return 1
    fi
    print_success "ROOT available - Version: $(root-config --version)"
    
    # Compilation
    local cxx_flags="-std=c++17 -O2 -Wall"
    local includes="-I. -I./include -I../include $(root-config --cflags)"
    local libs="$(root-config --libs) -lTree -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lMathCore -lPhysics"
    [[ -n "$CMSSW_BASE" ]] && includes="$includes -I$CMSSW_BASE/src -I$CMSSW_RELEASE_BASE/src"
    
    local compile_cmd="g++ $cxx_flags $includes -DSTANDALONE_COMPILE scripts/photonJet.C $libs -o photonJet"
    
    [[ "$VERBOSE" == true || "$DRY_RUN" == true ]] && print_info "Command: $compile_cmd"
    [[ "$DRY_RUN" == true ]] && { print_info "[DRY RUN] Would compile"; return 0; }
    
    if eval $compile_cmd; then
        print_success "Compilation successful! Executable: photonJet"
    else
        print_error "Compilation failed!"; return 1
    fi
}

# Run analysis locally
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
    
    [[ -n "$config_file" ]] && { run_args="$run_args --config $config_file"; print_info "Using config: $config_file"; }
    
    print_info "Starting analysis..."
    [[ "$DRY_RUN" == true ]] && { print_info "[DRY RUN] Would execute: ./photonJet $run_args"; return 0; }
    
    if ./photonJet $run_args; then
        print_success "Analysis completed successfully!"
    else
        print_error "Analysis failed!"; return 1
    fi
}

# Submit batch job to Condor
submit_batch() {
    local config_file="$1"
    local job_timestamp="$(date +%Y%m%d_%H%M%S)"
    local config_name="default"
    [[ -n "$config_file" ]] && config_name=$(basename "$config_file" .config)
    local job_dir="batch/job_${job_timestamp}_${config_name}"
    
    [[ "$DRY_RUN" == true ]] && {
        print_info "[DRY RUN] Would create: $job_dir"
        print_info "[DRY RUN] Would submit to Condor with OS: $OS_VERSION"
        return 0
    }
    
    mkdir -p "$job_dir/logs"
    print_info "Creating job directory: $job_dir"
    
    # Copy files
    cp scripts/photonJet.C "$job_dir/"
    cp include/*.h "$job_dir/" 2>/dev/null || true
    [[ -n "$config_file" && -f "$config_file" ]] && cp "$config_file" "$job_dir/$(basename "$config_file")"
    
    # Create job script
    cat > "$job_dir/run_job.sh" << EOF
#!/bin/bash
echo "Starting photonJet analysis: \$(date) on \$(hostname)"
[[ -n "\$CMSSW_BASE" ]] && echo "CMSSW: \$CMSSW_BASE" || {
    echo "Setting up LCG environment"
    source /cvmfs/sft.cern.ch/lcg/views/LCG_104/x86_64-${OS_VERSION}-gcc11-opt/setup.sh
}
echo "Compiling..."
g++ -std=c++17 -O2 -I. \$(root-config --cflags) -DSTANDALONE_COMPILE \\
    photonJet.C \$(root-config --libs) -lTree -lRIO -lNet -lHist \\
    -lGraf -lGraf3d -lGpad -lMathCore -lPhysics -o photonJet || exit 1
chmod +x photonJet
echo "Running analysis..."
EOF
    
    # Add run command
    if [[ "$PRODUCTION" == true ]]; then
        echo "./photonJet --production" >> "$job_dir/run_job.sh"
    else
        echo "./photonJet --test $MAX_EVENTS" >> "$job_dir/run_job.sh"
    fi
    [[ -n "$config_file" ]] && echo " --config $(basename "$config_file")" >> "$job_dir/run_job.sh"
    echo 'echo "Completed: $(date)"' >> "$job_dir/run_job.sh"
    chmod +x "$job_dir/run_job.sh"
    
    # Create Condor submit file
    local input_files="photonJet.C"
    for f in include/*.h; do
        [[ -f "$f" ]] && input_files="$input_files, $(basename "$f")"
    done
    [[ -n "$config_file" ]] && input_files="$input_files, $(basename "$config_file")"
    
    cat > "$job_dir/submit.sub" << EOF
universe = vanilla
executable = run_job.sh
output = logs/\$(Process).out
error = logs/\$(Process).err
log = logs/\$(Process).log
MY.WantOS = "$OS_VERSION"
RequestCpus = 1
RequestMemory = 2000
should_transfer_files = YES
when_to_transfer_output = ON_EXIT
transfer_input_files = $input_files
+JobFlavour = "workday"
queue
EOF
    
    # Submit
    print_info "Submitting job with OS: $OS_VERSION"
    pushd "$job_dir" >/dev/null
    if condor_submit submit.sub; then
        print_success "Job submitted successfully - Directory: $job_dir"
        print_info "Monitor with: condor_q"
    else
        print_error "Job submission failed"; popd >/dev/null; return 1
    fi
    popd >/dev/null
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -m|--mode) MODE="$2"; shift 2 ;;
        -c|--config) 
            CONFIG="$2"
            CONFIG_ABS=$(realpath "$CONFIG" 2>/dev/null)
            if [[ $? -eq 0 && -f "$CONFIG_ABS" ]]; then
                CONFIG="$CONFIG_ABS"
                print_info "Using config file: $CONFIG"
            else
                print_error "Config file not found: $CONFIG"; exit 1
            fi
            shift 2 ;;
        -v|--verbose) VERBOSE=true; shift ;;
        -p|--production) PRODUCTION=true; shift ;;
        -t|--test) 
            PRODUCTION=false
            if [[ $2 =~ ^[0-9]+$ ]]; then MAX_EVENTS="$2"; shift 2; else MAX_EVENTS=10000; shift; fi ;;
        --clean) CLEAN=true; shift ;;
        --dry-run) DRY_RUN=true; shift ;;
        --os-version) OS_VERSION="$2"; shift 2 ;;
        -h|--help) show_usage; exit 0 ;;
        *) print_error "Unknown option: $1"; show_usage; exit 1 ;;
    esac
done

# Clean and exit if requested
if [[ "$CLEAN" == true ]]; then
    print_info "Cleaning compilation artifacts..."
    rm -f photonJet photonJet.exe *.o *.so *.d
    rm -rf AutoDict_* batch/
    print_success "Cleanup completed"; exit 0
fi

# Main execution
print_info "PhotonJet Analysis Compilation Script"
[[ "$DRY_RUN" == true ]] && print_warning "DRY RUN MODE - No commands will be executed"

# Set default config if not provided
if [[ -z "$CONFIG" ]]; then
    CONFIG="../configs/photon_only.config"
    CONFIG_ABS=$(realpath "$CONFIG" 2>/dev/null)
    if [[ $? -eq 0 && -f "$CONFIG_ABS" ]]; then
        CONFIG="$CONFIG_ABS"
        print_info "Using default config file: $CONFIG"
    else
        print_error "Default config file not found: $CONFIG"; exit 1
    fi
fi

# Validate config and mode
[[ -n "$CONFIG" && ! -f "$CONFIG" ]] && { print_error "Config file not found: $CONFIG"; exit 1; }
case $MODE in
    local) print_info "Mode: Local compilation and execution" ;;
    batch) print_info "Mode: Batch submission (Condor) - OS: $OS_VERSION" ;;
    *) print_error "Invalid mode: $MODE"; exit 1 ;;
esac

# Execute based on mode
case $MODE in
    local) setup_and_compile && run_local "$CONFIG" ;;
    batch) submit_batch "$CONFIG" ;;
esac

print_success "Script completed successfully!"
