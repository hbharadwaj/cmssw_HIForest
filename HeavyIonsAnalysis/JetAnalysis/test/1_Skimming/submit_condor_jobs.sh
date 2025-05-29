#!/bin/bash
# submit_condor_jobs.sh
# This script prepares and submits condor jobs for batch processing of HiForest files

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

# Default parameters
CONFIG_FILE="../configs/2023_PbPb_QCDPhoton.config"
WORKING_DIR=$(pwd)
MAX_BATCHES=0  # 0 means process all batches
CLEANUP=0      # By default, don't cleanup old compilation artifacts (reuse them)
REMOVE_JOBS=0  # By default, don't remove existing jobs
VERBOSITY=1    # Default verbosity level: 0=minimal, 1=normal, 2=debug, 3=trace
EXECUTABLE="SkimHiForest"
OS_VERSION=""   # Default to not specifying OS version (let HTCondor choose)

# Function to create a unique logs directory for each submission
create_logs_directory() {
    local config_file="$1"
    local config_basename=$(basename "$config_file" .config)
    local timestamp=$(date +"%Y%m%d_%H%M%S")
    local logs_dir="$WORKING_DIR/logs_${config_basename}_${timestamp}"
    
    mkdir -p "$logs_dir"
    echo "$logs_dir"
}

# Function removed - No longer using job type classification

# Parse command-line arguments
PRECOMPILE=0
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        -c|--config)
            CONFIG_FILE="$2"
            shift
            shift
            ;;
        -d|--dir)
            WORKING_DIR="$2"
            shift
            shift
            ;;
        -n|--max-batches)
            MAX_BATCHES="$2"
            shift
            shift
            ;;
        -p|--precompile)
            PRECOMPILE=1
            CLEANUP=1
            shift
            ;;
        -r|--remove-jobs)
            REMOVE_JOBS=1
            shift
            ;;
        --cleanup)
            CLEANUP=1
            shift
            ;;
        -v|--verbose)
            VERBOSITY=2  # Set debug level
            echo "Debug mode enabled (verbosity level 2)"
            shift
            ;;
        --trace)
            VERBOSITY=3  # Set trace level (most verbose)
            echo "Trace mode enabled (verbosity level 3)"
            shift
            ;;
        -q|--quiet)
            VERBOSITY=0  # Only errors
            echo "Quiet mode enabled (verbosity level 0)"
            shift
            ;;
        --os-version)
            OS_VERSION="$2"  # Set OS version (el8 or el9)
            echo "Using OS version: $OS_VERSION"
            shift
            shift
            ;;
        --verbosity)
            VERBOSITY="$2"  # Set custom level
            echo "Custom verbosity level set: $VERBOSITY"
            shift
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [options]"
            echo "Options:"
            echo "  -c, --config CONFIG_FILE   Configuration file path (default: ../configs/2023_PbPb_QCDPhoton.config)"
            echo "  -d, --dir WORKING_DIR      Working directory (default: current directory)"
            echo "  -n, --max-batches NUM      Maximum number of batches to process (default: process all)"
            echo "  -p, --precompile           Compile SkimHiForest executable before submitting jobs"
            echo "  -r, --remove-jobs          Remove existing condor jobs before submitting new ones"
            echo "  --cleanup                  Clean up old compilation artifacts (default: off)"
            echo "  -v, --verbose              Enable detailed debug output (level 2)"
            echo "  --trace                    Enable trace-level output (level 3, very verbose)"
            echo "  -q, --quiet                Show only error messages (level 0)"
            echo "  --os-version VERSION       Specify OS version to use (e.g., 'el8' or 'el9')"
            echo "  --verbosity LEVEL          Set verbosity level manually (0-3)"
            echo "  -h, --help                 Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $key"
            echo "Use -h or --help for usage information"
            exit 1
            ;;
    esac
done

echo "Verbosity level : "$VERBOSITY

# Check and remove existing condor jobs if requested
if [[ $REMOVE_JOBS -eq 1 ]]; then
    log 1 "Checking for existing condor jobs..."
    EXISTING_JOBS=$(condor_q -format "%d." ClusterId -format "%d " ProcId -format "%s\n" Cmd | grep "$EXECUTABLE" | wc -l)
    
    if [[ $EXISTING_JOBS -gt 0 ]]; then
        log 1 "Found $EXISTING_JOBS running jobs. Removing them..."
        log 2 "Running: condor_q -format \"%d.\" ClusterId -format \"%d \" ProcId -format \"%s\\n\" Cmd | grep $EXECUTABLE | awk '{print \$1}' | xargs condor_rm"
        condor_q -format "%d." ClusterId -format "%d " ProcId -format "%s\n" Cmd | grep "$EXECUTABLE" | awk '{print $1}' | xargs condor_rm
        log 1 "Waiting for jobs to finish terminating..."
        sleep 5
        log 1 "Done."
    else
        log 1 "No existing jobs found."
    fi
fi

log 1 "Starting condor job submission with:"
log 1 "  Config file: $CONFIG_FILE"
log 1 "  Working directory: $WORKING_DIR"
log 1 "  Verbosity level: $VERBOSITY"
if [[ -n "$OS_VERSION" ]]; then
    log 1 "  OS version: $OS_VERSION"
fi

log 1 "  Using standalone executable mode"
if [[ $MAX_BATCHES -gt 0 ]]; then
    log 1 "  Max batches: $MAX_BATCHES"
else
    log 1 "  Processing all batches"
fi

# Cleanup compilation artifacts if needed
if [[ $CLEANUP -eq 1 ]]; then
    log 1 "Cleaning up compilation artifacts..."
    rm -f "$EXECUTABLE" lib"$EXECUTABLE".so *_rdict.pcm *.d
    log 2 "Cleaned up old compilation artifacts"
fi

# Compile if requested
if [[ $PRECOMPILE -eq 1 ]]; then
    log 1 "Compiling SkimHiForest..."
    make clean && make || {
        log 0 "ERROR: Compilation failed"
        exit 1
    }
    log 1 "Compilation successful"
fi

# Create batch directory
CONFIG_BASENAME=$(basename "$CONFIG_FILE" .config)
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
BATCH_DIR="$WORKING_DIR/batch/job_${TIMESTAMP}_${CONFIG_BASENAME}"
log 2 "Creating batch directory: $BATCH_DIR"
mkdir -p "$BATCH_DIR"

# Make CONFIG_FILE absolute path if it's relative
if [[ "$CONFIG_FILE" != /* ]]; then
    ABS_CONFIG_FILE=$(cd "$WORKING_DIR" && realpath -e "$CONFIG_FILE" 2>/dev/null)
    if [[ -n "$ABS_CONFIG_FILE" && -f "$ABS_CONFIG_FILE" ]]; then
        CONFIG_FILE="$ABS_CONFIG_FILE"
        log 1 "Using absolute config path: $CONFIG_FILE"
    else
        log 0 "WARNING: Config file not found: $CONFIG_FILE"
        SCRIPT_DIR="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
        POSSIBLE_CONFIG="$SCRIPT_DIR/$CONFIG_FILE"
        if [[ -f "$POSSIBLE_CONFIG" ]]; then
            CONFIG_FILE="$POSSIBLE_CONFIG"
            log 1 "Found config file at: $CONFIG_FILE"
        fi
    fi
fi

# Count input files to determine number of batches
log 1 "Analyzing config file to determine number of batches..."
if [[ -f "$CONFIG_FILE" ]]; then
    
    
    # Get InputDir and FilesPerOutput
    INPUT_DIR=$(awk '/^InputDir / {
        sub(/^InputDir[ \t]+/, "")
        sub(/#.*$/, "")
        gsub(/^["'\'']+|["'\'']+$/, "")
        gsub(/^[ \t]+|[ \t]+$/, "")
        print
    }' "$CONFIG_FILE")
    
    FILES_PER_BATCH=$(awk '/^FilesPerOutput / {
        sub(/^FilesPerOutput[ \t]+/, "")
        sub(/#.*$/, "")
        gsub(/^[ \t]+|[ \t]+$/, "")
        print
    }' "$CONFIG_FILE")
    
    if [[ -z "$FILES_PER_BATCH" ]]; then
        FILES_PER_BATCH=1
    fi
    
    if [[ -d "$INPUT_DIR" ]]; then
        FILE_COUNT=$(find "$INPUT_DIR" -type f -name "*.root" | wc -l)
        log 1 "[INFO] Found $FILE_COUNT total input files"
        log 1 "[INFO] Processing $FILES_PER_BATCH files per batch"
        if [[ $FILE_COUNT -gt 0 ]]; then
            ESTIMATED_BATCHES=$((FILE_COUNT / FILES_PER_BATCH))
            if [[ $((FILE_COUNT % FILES_PER_BATCH)) -gt 0 ]]; then
                ESTIMATED_BATCHES=$((ESTIMATED_BATCHES + 1))
            fi
            
            if [[ $MAX_BATCHES -gt 0 && $MAX_BATCHES -lt $ESTIMATED_BATCHES ]]; then
                TOTAL_BATCHES=$MAX_BATCHES
            else
                TOTAL_BATCHES=$ESTIMATED_BATCHES
            fi
        else
            TOTAL_BATCHES=$MAX_BATCHES
        fi
    else
        TOTAL_BATCHES=$MAX_BATCHES
    fi
fi

if [[ $TOTAL_BATCHES -le 0 ]]; then
    log 0 "ERROR: Invalid number of batches: $TOTAL_BATCHES"
    TOTAL_BATCHES=1
fi

# Prepare logs directory
LOGS_DIR="$BATCH_DIR/logs"
mkdir -p "$LOGS_DIR"

# Copy executable and files to batch directory
log 1 "Copying files to batch directory..."
cp -p "$EXECUTABLE" "$BATCH_DIR/"
chmod 755 "$BATCH_DIR/$EXECUTABLE"
ls -la "$BATCH_DIR/$EXECUTABLE"
log 1 "Executable copied with permissions: $(stat -c '%a' "$BATCH_DIR/$EXECUTABLE")"

# Copy source files
log 1 "Copying source files..."
cp "SkimHiForest.C" "$BATCH_DIR/"

# Create a modified copy of the config file specifically for the batch job
log 1 "Creating batch-specific config file..."
CONFIG_BASENAME=$(basename "$CONFIG_FILE")
BATCH_CONFIG="$BATCH_DIR/$CONFIG_BASENAME"

# Copy the original config file
cp "$CONFIG_FILE" "$BATCH_CONFIG"

# Update parameters in the batch config
log 1 "Setting BatchMode=1 and Verbose=$VERBOSITY in batch config file..."
if grep -q "^BatchMode" "$BATCH_CONFIG"; then
    sed -i "s/^BatchMode[ \t]\+[0-9]\+/BatchMode 1/" "$BATCH_CONFIG"
else
    # Insert BatchMode right after any comment lines at the top
    sed -i '/^#/,$!i\BatchMode 1' "$BATCH_CONFIG"
    # If no comment lines exist, add it to the beginning
    if ! grep -q "^BatchMode" "$BATCH_CONFIG"; then
        sed -i "1i\\BatchMode 1" "$BATCH_CONFIG"
    fi
fi

if grep -q "^Verbose" "$BATCH_CONFIG"; then
    sed -i "s/^Verbose[ \t]\+[0-9]\+/Verbose $VERBOSITY/" "$BATCH_CONFIG"
else
    sed -i "1i\\Verbose $VERBOSITY" "$BATCH_CONFIG"
fi

log 2 "Copied SkimHiForest.C and created modified $(basename "$CONFIG_FILE") in $BATCH_DIR"

# List files to be transferred
log 2 "Files to be transferred to condor job:"
log 2 "- $EXECUTABLE"
log 2 "- $(basename "$CONFIG_FILE")"
log 2 "- run_skimming.sh"

# Create a simple wrapper script without any external dependencies
WRAPPER_SCRIPT="$BATCH_DIR/run_skimming.sh"
log 1 "Creating wrapper script: $WRAPPER_SCRIPT"

cat > "$WRAPPER_SCRIPT" << 'WRAPPER_EOF'
#!/bin/bash

CONFIG_FILE=$1
BATCH_ID=$2

# Print basic information
echo "Running SkimHiForest on $(hostname)"
echo "OS: $(cat /etc/redhat-release 2>/dev/null || echo 'Unknown OS')"
echo "Working directory: $(pwd)"

# Double-check BatchMode is set in the config file
if ! grep -q "^BatchMode 1" "$CONFIG_FILE"; then
    echo "WARNING: BatchMode 1 not found in config, adding it..."
    echo "BatchMode 1" >> "$CONFIG_FILE"
fi

# Debug: Print the config file contents
echo "================ CONFIG FILE CONTENTS ================"
grep -v "^#" "$CONFIG_FILE" | grep -v "^$" 
echo "==================================================="
echo "BatchMode setting: $(grep 'BatchMode' "$CONFIG_FILE" || echo 'BatchMode not found')"
echo "Verbose setting: $(grep 'Verbose' "$CONFIG_FILE" || echo 'Verbose not found')"
echo "FilesPerOutput setting: $(grep 'FilesPerOutput' "$CONFIG_FILE" || echo 'FilesPerOutput not found')"

# Make sure executable has correct permissions
chmod +x ./SkimHiForest

# Run the executable directly
echo "Starting SkimHiForest with arguments: $CONFIG_FILE $BATCH_ID"
./SkimHiForest "$CONFIG_FILE" "$BATCH_ID"
WRAPPER_EOF

chmod +x "$WRAPPER_SCRIPT"

# Create condor submission file
CONDOR_FILE="$BATCH_DIR/submit_${CONFIG_BASENAME}_${TIMESTAMP}.condor"
log 1 "Creating condor submission file: $CONDOR_FILE"

cat > "$CONDOR_FILE" << 'EOT'
# Condor submission file for HiForest skimming
universe = vanilla
EOT

# Append to the condor file safely (avoiding variable expansion issues)
cat >> "$CONDOR_FILE" << EOF
executable = $BATCH_DIR/run_skimming.sh
arguments = $(basename "$CONFIG_FILE") \$(Process)
output = $LOGS_DIR/skim_\$(Process).out
error = $LOGS_DIR/skim_\$(Process).err
log = $LOGS_DIR/skim_\$(Process).log

# Transfer all necessary files
transfer_input_files = $BATCH_DIR/$EXECUTABLE,$BATCH_DIR/$(basename "$CONFIG_FILE")
should_transfer_files = YES
when_to_transfer_output = ON_EXIT

# Resource requests
request_memory = 4GB
request_disk = 2GB
request_cpus = 1

# CERN-specific settings
# Choice of version for the job el8 or el9
EOF

# Conditionally add OS version requirement
if [[ -n "$OS_VERSION" ]]; then
    echo "MY.WantOS = \"$OS_VERSION\"" >> "$CONDOR_FILE"
    log 2 "Requesting OS version: $OS_VERSION"
fi

cat >> "$CONDOR_FILE" << EOF
max_retries = 3
# Use "workday" flavor for jobs up to 8 hours
+JobFlavour = "workday"
queue $TOTAL_BATCHES
EOF

log 1 "==================== SUBMISSION SUMMARY ===================="
log 1 "Condor submission files prepared:"
log 1 "  Submission file: $CONDOR_FILE"
log 1 "  Executable: $BATCH_DIR/$EXECUTABLE"
log 1 "  Logs directory: $LOGS_DIR"
log 1 "  Batch working directory: $BATCH_DIR"
log 1 "  Total jobs: $TOTAL_BATCHES"
log 1 ""

# Print detailed information in debug mode
if [[ $VERBOSITY -ge 2 ]]; then
    log 2 "============== ENVIRONMENT INFORMATION ================"
    log 2 "Config file: $CONFIG_FILE"
    log 2 "Input directory: $INPUT_DIR"
    log 2 "Files per batch: $FILES_PER_BATCH"
    log 2 "Total input files: $FILE_COUNT"
    log 2 "========================================================="
fi

log 1 "To submit the jobs, run:"
log 1 "  condor_submit $CONDOR_FILE"
log 1 ""
log 1 "To monitor logs from this submission:"
log 1 "  tail -f $LOGS_DIR/skim_*.out"
log 1 "=========================================================="

# Optionally, submit the jobs automatically
read -p "Submit jobs now? (y/n) " SUBMIT
if [[ $SUBMIT == "y" || $SUBMIT == "Y" ]]; then
    log 1 "Submitting jobs to condor..."
    log 2 "Running: condor_submit $CONDOR_FILE"
    SUBMIT_OUTPUT=$(condor_submit "$CONDOR_FILE")
    SUBMIT_STATUS=$?
    
    if [[ $VERBOSITY -ge 2 ]]; then
        echo "$SUBMIT_OUTPUT"
    else
        echo "$SUBMIT_OUTPUT" | grep "job(s) submitted"
    fi
    
    if [[ $SUBMIT_STATUS -eq 0 ]]; then
        log 1 "Jobs submitted successfully."
        log 1 "============= JOB MONITORING =============="
        log 1 "Monitor job status with:"
        log 1 "  condor_q "
        log 1 "  # or check logs with"
        log 1 "  tail -f $LOGS_DIR/skim_*.out"
        log 1 "=========================================="
    else
        log 0 "ERROR: Job submission failed with status $SUBMIT_STATUS"
        log 0 "Run with --verbose for more details"
    fi
else
    log 1 "Jobs not submitted. You can submit them later with:"
    log 1 "  condor_submit $CONDOR_FILE"
fi