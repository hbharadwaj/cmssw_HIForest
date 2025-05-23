#!/bin/bash
# run_batch.sh - Executes a single batch of the HiForest skimming with the compiled executable

# Logging function with verbosity levels
# Usage: log <level> <message>
# Levels: 0=ERROR, 1=INFO, 2=DEBUG, 3=TRACE
log() {
    local level=$1
    local msg=$2
    local timestamp=$(date "+%Y-%m-%d %H:%M:%S")
    
    # Define log level prefixes with colors
    local ERROR="\033[1;31m[ERROR]\033[0m"
    local INFO="\033[1;32m[INFO]\033[0m"
    local DEBUG="\033[1;34m[DEBUG]\033[0m"
    local TRACE="\033[1;35m[TRACE]\033[0m"
    
    # Only show messages at or below the current verbosity level
    if [[ $level -le $VERBOSITY ]]; then
        case $level in
            0) echo -e "$timestamp $ERROR $msg" ;;
            1) echo -e "$timestamp $INFO $msg" ;;
            2) echo -e "$timestamp $DEBUG $msg" ;;
            3) echo -e "$timestamp $TRACE $msg" ;;
            *) echo -e "$timestamp $msg" ;;
        esac
    fi
}

# Set default verbosity level
VERBOSITY=1

# Check command line arguments
if [ $# -lt 3 ] || [ $# -gt 4 ]; then
    log 0 "Usage: $0 CONFIG_FILE BATCH_ID WORKING_DIR [USE_CMSSW]"
    log 0 "  USE_CMSSW: Set to 0 to use current ROOT setup instead of CMSSW environment (default: 1)"
    exit 1
fi

CONFIG_FILE=$1
BATCH_ID=$2
WORKING_DIR=$3
USE_CMSSW=${4:-1}  # Default to using CMSSW environment if not specified

# Print environment info
log 1 "Running on host: $(hostname)"
log 1 "Date: $(date)"
log 1 "Working directory: $WORKING_DIR"
log 1 "Config file: $CONFIG_FILE"
log 1 "Batch ID: $BATCH_ID"
log 1 "USE_CMSSW: $USE_CMSSW"

# Detect OS version
OS_VERSION=""
if [ -f /etc/os-release ]; then
    source /etc/os-release
    OS_VERSION=$ID$VERSION_ID
    log 1 "Detected OS: $OS_VERSION"
else
    log 1 "Cannot detect OS version, will try to continue anyway"
fi

# Set up environment based on OS version
if [[ "$OS_VERSION" == *"9"* || "$OS_VERSION" == *"el9"* || "$OS_VERSION" == *"centos9"* || "$OS_VERSION" == *"rocky9"* ]]; then
    export SCRAM_ARCH=el9_amd64_gcc12
else
    export SCRAM_ARCH=el8_amd64_gcc11
fi
log 1 "Using SCRAM_ARCH: $SCRAM_ARCH"

# Check if we need to use singularity for EL8 hosts
if [[ "$OS_VERSION" == *"8"* || "$OS_VERSION" == *"el8"* || "$OS_VERSION" == *"centos8"* || "$OS_VERSION" == *"rocky8"* ]]; then
    log 1 "Detected EL8 host but code was compiled for EL9. Will try to use singularity."
    
    # Check if singularity/apptainer is available
    if command -v singularity &> /dev/null; then
        CONTAINER_CMD="singularity"
    elif command -v apptainer &> /dev/null; then
        CONTAINER_CMD="apptainer"
    else
        log 1 "WARNING: Neither singularity nor apptainer found. Will continue without container, which might cause issues."
        CONTAINER_CMD=""
    fi
    
    if [ -n "$CONTAINER_CMD" ]; then
        log 1 "Will use $CONTAINER_CMD to run in EL9 container"
        CONTAINER_IMAGE="/cvmfs/unpacked.cern.ch/registry.hub.docker.com/cmssw/el9:amd64"
        
        # Check if container image exists
        if [ ! -d "$CONTAINER_IMAGE" ]; then
            log 1 "ERROR: Container image not found at $CONTAINER_IMAGE"
            log 1 "Will try to continue without container, which might cause issues."
        else
            # Use singularity to run the executable
            exec $CONTAINER_CMD run -B /cvmfs -B /afs "$CONTAINER_IMAGE" \
                "./SkimHiForest" "$CONFIG_FILE" "$BATCH_ID"
            exit $?
        fi
    fi
fi

# Run the executable directly if not using container
exec "./SkimHiForest" "$CONFIG_FILE" "$BATCH_ID"
