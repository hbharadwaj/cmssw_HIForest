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
USE_CMSSW=0    # By default, don't use CMSSW environment (use current ROOT setup)
VERBOSITY=1    # Default verbosity level: 0=minimal, 1=normal, 2=debug, 3=trace

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
            shift
            ;;
        -r|--remove-jobs)
            REMOVE_JOBS=1
            shift
            ;;
        --use-cmssw)
            USE_CMSSW=1
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
            echo "  -p, --precompile           Pre-compile SkimHiForest.C before submitting jobs (implies --cleanup)"
            echo "  -r, --remove-jobs          Remove existing condor jobs before submitting new ones"
            echo "  --use-cmssw                Use CMSSW environment instead of current ROOT setup"
            echo "  --cleanup                  Clean up old compilation artifacts (default: off)"
            echo "  -v, --verbose              Enable detailed debug output (level 2)"
            echo "  --trace                    Enable trace-level output (level 3, very verbose)"
            echo "  -q, --quiet                Show only error messages (level 0)"
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

# Check and remove existing condor jobs if requested
if [[ $REMOVE_JOBS -eq 1 ]]; then
    log 1 "Checking for existing condor jobs..."
    EXISTING_JOBS=$(condor_q -submitter $(whoami) -format "%d." ClusterId -format "%d " ProcId -format "%s\n" Cmd | grep run_batch.sh | wc -l)
    
    if [[ $EXISTING_JOBS -gt 0 ]]; then
        log 1 "Found $EXISTING_JOBS running jobs. Removing them..."
        log 2 "Running: condor_q -submitter $(whoami) -format \"%d.\" ClusterId -format \"%d \" ProcId -format \"%s\\n\" Cmd | grep run_batch.sh | awk '{print \$1}' | xargs condor_rm"
        condor_q -submitter $(whoami) -format "%d." ClusterId -format "%d " ProcId -format "%s\n" Cmd | grep run_batch.sh | awk '{print $1}' | xargs condor_rm
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

if [[ $USE_CMSSW -eq 1 ]]; then
    log 1 "  Environment: Using CMSSW environment"
else
    log 1 "  Environment: Using current ROOT setup (no CMSSW)"
fi
if [[ $MAX_BATCHES -gt 0 ]]; then
    log 1 "  Max batches: $MAX_BATCHES"
else
    log 1 "  Processing all batches"
fi

# Cleanup compilation artifacts if explicitly requested (but not by default)
if [[ $CLEANUP -eq 1 ]]; then
    log 1 "Cleaning up compilation artifacts..."
    rm -f SkimHiForest_C_ACLiC_dict_rdict.pcm SkimHiForest_C.d SkimHiForest_C.so libSkimHiForest.so libSkimHiForest_ACLiC_dict_rdict.pcm
    log 2 "Removed files: SkimHiForest_C_ACLiC_dict_rdict.pcm SkimHiForest_C.d SkimHiForest_C.so libSkimHiForest.so libSkimHiForest_ACLiC_dict_rdict.pcm"
else
    log 1 "Reusing existing compilation artifacts if present..."
fi

# Create a unique batch directory for this submission
CONFIG_BASENAME=$(basename "$CONFIG_FILE" .config)
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
BATCH_DIR="$WORKING_DIR/batch/job_${TIMESTAMP}_${CONFIG_BASENAME}"
log 2 "Creating batch directory: $BATCH_DIR"
mkdir -p "$BATCH_DIR"

# Make CONFIG_FILE absolute path if it's relative
if [[ "$CONFIG_FILE" != /* ]]; then
    log 2 "Converting relative config path to absolute path..."
    log 3 "Current config path: $CONFIG_FILE"
    log 3 "Working directory: $WORKING_DIR"
    
    ABS_CONFIG_FILE=$(cd "$WORKING_DIR" && realpath -e "$CONFIG_FILE" 2>/dev/null)
    if [[ -n "$ABS_CONFIG_FILE" && -f "$ABS_CONFIG_FILE" ]]; then
        CONFIG_FILE="$ABS_CONFIG_FILE"
        log 1 "Using absolute config path: $CONFIG_FILE"
    else
        log 0 "WARNING: Config file not found: $CONFIG_FILE"
        log 0 "Working directory: $WORKING_DIR"
        # Try to find the config file relative to the script location
        SCRIPT_DIR="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
        log 2 "Script directory: $SCRIPT_DIR"
        POSSIBLE_CONFIG="$SCRIPT_DIR/$CONFIG_FILE"
        if [[ -f "$POSSIBLE_CONFIG" ]]; then
            CONFIG_FILE="$POSSIBLE_CONFIG"
            log 1 "Found config file at: $CONFIG_FILE"
        else
            log 3 "Tried paths:"
            log 3 "  - $WORKING_DIR/$CONFIG_FILE"
            log 3 "  - $SCRIPT_DIR/$CONFIG_FILE"
        fi
    fi
fi

# Move to the batch directory for all further operations
log 2 "Changing to batch directory: $BATCH_DIR"
cd "$BATCH_DIR"

# Find SkimHiForest.C path (from current working directory or full path)
log 2 "Looking for SkimHiForest.C..."
SKIMHIFOREST_PATH="$WORKING_DIR/SkimHiForest.C"
log 3 "Checking path: $SKIMHIFOREST_PATH"

if [[ ! -f "$SKIMHIFOREST_PATH" ]]; then
    log 3 "Not found at $SKIMHIFOREST_PATH"
    SKIMHIFOREST_PATH="$WORKING_DIR/CMSSW_13_2_13/src/HeavyIonsAnalysis/JetAnalysis/test/1_Skimming/SkimHiForest.C"
    log 3 "Checking path: $SKIMHIFOREST_PATH"
    
    if [[ ! -f "$SKIMHIFOREST_PATH" ]]; then
        log 3 "Not found at $SKIMHIFOREST_PATH"
        SCRIPT_DIR="$(dirname "$(readlink -f "${BASH_SOURCE[0]}")")"
        SKIMHIFOREST_PATH="$SCRIPT_DIR/SkimHiForest.C"
        log 3 "Checking path: $SKIMHIFOREST_PATH"
        
        if [[ ! -f "$SKIMHIFOREST_PATH" ]]; then
            log 0 "ERROR: Could not find SkimHiForest.C at expected locations:"
            log 0 "  - $WORKING_DIR/SkimHiForest.C"
            log 0 "  - $WORKING_DIR/CMSSW_13_2_13/src/HeavyIonsAnalysis/JetAnalysis/test/1_Skimming/SkimHiForest.C"
            log 0 "  - $SCRIPT_DIR/SkimHiForest.C"
            exit 1
        else
            log 2 "Found SkimHiForest.C at: $SKIMHIFOREST_PATH"
        fi
    else
        log 2 "Found SkimHiForest.C at: $SKIMHIFOREST_PATH"
    fi
else
    log 2 "Found SkimHiForest.C at: $SKIMHIFOREST_PATH"
fi

# Count input files to determine number of batches
# Parse the config to get input directory
CONFIG_FULL_PATH="$CONFIG_FILE"
if [[ ! -f "$CONFIG_FULL_PATH" ]]; then
    log 0 "WARNING: Could not find config file: $CONFIG_FILE"
    log 0 "Will default to MAX_BATCHES=$MAX_BATCHES for total batches"
    TOTAL_BATCHES=$MAX_BATCHES
else
    log 1 "Analyzing config file to determine number of batches..."

    # Update the Verbose parameter in the config file to match command-line verbosity
    if grep -q "^Verbose" "$CONFIG_FULL_PATH"; then
        # Replace existing Verbose line
        log 2 "Updating Verbose parameter in config file to match command-line verbosity level: $VERBOSITY"
        sed -i "s/^Verbose[ \t]\+[0-9]\+/Verbose $VERBOSITY/" "$CONFIG_FULL_PATH"
    else
        # Add Verbose parameter if it doesn't exist
        log 2 "Adding Verbose parameter to config file with value: $VERBOSITY"
        sed -i "1i\\Verbose $VERBOSITY" "$CONFIG_FULL_PATH"
    fi
    
    # Print the config file format for debugging
    log 2 "First 10 lines of config file:"
    CONFIG_PREVIEW=$(head -n 10 "$CONFIG_FULL_PATH")
    if [[ $VERBOSITY -ge 2 ]]; then
        echo "$CONFIG_PREVIEW"
    fi
    log 3 "Full config file path: $CONFIG_FULL_PATH"
    
    # Parse InputDir more carefully, considering comments and whitespace
    INPUT_DIR=$(awk '/^InputDir / {
        # Remove leading "InputDir" and any whitespace
        sub(/^InputDir[ \t]+/, "")
        # Remove any trailing comments
        sub(/#.*$/, "")
        # Remove any quotes
        gsub(/^["'\'']+|["'\'']+$/, "")
        # Trim whitespace
        gsub(/^[ \t]+|[ \t]+$/, "")
        print
    }' "$CONFIG_FULL_PATH")
    
    FILES_PER_BATCH=$(awk '/^FilesPerOutput / {
        sub(/^FilesPerOutput[ \t]+/, "")
        sub(/#.*$/, "")
        gsub(/^[ \t]+|[ \t]+$/, "")
        print
    }' "$CONFIG_FULL_PATH")
    
    log 1 "Config InputDir: $INPUT_DIR"
    
    if [[ -z "$FILES_PER_BATCH" ]]; then
        log 2 "FilesPerOutput not found in config, defaulting to 1"
        FILES_PER_BATCH=1
    else
        log 2 "Files per batch: $FILES_PER_BATCH"
    fi
    
    if [[ -n "$INPUT_DIR" ]]; then
        # Check if directory exists
        if [[ ! -d "$INPUT_DIR" ]]; then
            log 0 "ERROR: Input directory does not exist: $INPUT_DIR"
            log 0 "Will default to MAX_BATCHES=$MAX_BATCHES for total batches"
            TOTAL_BATCHES=$MAX_BATCHES
        else
            # Debug: Print find command
            FIND_CMD="find \"$INPUT_DIR\" -type f -name \"*.root\""
            log 1 "Counting input files..."
            log 2 "Executing: $FIND_CMD"
            
            # Count .root files recursively
            START_TIME=$(date +%s)
            FILE_COUNT=$(find "$INPUT_DIR" -type f -name "*.root" | wc -l)
            FIND_EXIT=$?
            END_TIME=$(date +%s)
            ELAPSED=$((END_TIME - START_TIME))
            
            log 3 "find command exit code: $FIND_EXIT"
            log 3 "File counting took $ELAPSED seconds"
            
            if [[ $FILE_COUNT -gt 0 ]]; then
                ESTIMATED_BATCHES=$((FILE_COUNT / FILES_PER_BATCH))
                if [[ $((FILE_COUNT % FILES_PER_BATCH)) -gt 0 ]]; then
                    ESTIMATED_BATCHES=$((ESTIMATED_BATCHES + 1))
                fi
                
                log 1 "Found $FILE_COUNT input files, estimated $ESTIMATED_BATCHES batches"
                log 3 "Calculation: $FILE_COUNT files / $FILES_PER_BATCH files per batch = $ESTIMATED_BATCHES batches"
                
                if [[ $MAX_BATCHES -gt 0 && $MAX_BATCHES -lt $ESTIMATED_BATCHES ]]; then
                    log 1 "Limiting to $MAX_BATCHES batches as requested"
                    TOTAL_BATCHES=$MAX_BATCHES
                else
                    TOTAL_BATCHES=$ESTIMATED_BATCHES
                fi
            else
                log 0 "WARNING: No .root files found in input directory: $INPUT_DIR"
                log 0 "Will default to MAX_BATCHES=$MAX_BATCHES for total batches"
                TOTAL_BATCHES=$MAX_BATCHES
            fi
        fi
    else
        log 0 "WARNING: Could not determine InputDir from config"
        log 0 "Will default to MAX_BATCHES=$MAX_BATCHES for total batches"
        TOTAL_BATCHES=$MAX_BATCHES
    fi
fi

if [[ $TOTAL_BATCHES -le 0 ]]; then
    log 0 "ERROR: Invalid number of batches: $TOTAL_BATCHES"
    log 0 "Setting to default of 1 batch"
    TOTAL_BATCHES=1
fi

# Pre-compile SkimHiForest.C if requested (using /tmp to avoid AFS space issues)
if [[ $PRECOMPILE -eq 1 ]]; then
    log 1 "Cleaning up old compilation artifacts..."
    log 3 "Removing files: SkimHiForest_C_ACLiC_dict_rdict.pcm SkimHiForest_C.d SkimHiForest_C.so libSkimHiForest.so libSkimHiForest_ACLiC_dict_rdict.pcm"
    rm -f SkimHiForest_C_ACLiC_dict_rdict.pcm SkimHiForest_C.d SkimHiForest_C.so libSkimHiForest.so libSkimHiForest_ACLiC_dict_rdict.pcm
    
    # Create a temporary directory to avoid AFS space issues
    TMP_COMPILE_DIR=$(mktemp -d /tmp/skim_compile_XXXXX)
    log 2 "Using temporary directory for compilation: $TMP_COMPILE_DIR"
    log 2 "Disk space available in /tmp: $(df -h /tmp | grep '/tmp' | awk '{print $4}')"
    
    # Copy SkimHiForest.C to temporary directory and batch directory
    log 2 "Copying $SKIMHIFOREST_PATH to temporary directory and batch directory"
    cp "$SKIMHIFOREST_PATH" $TMP_COMPILE_DIR/
    cp "$SKIMHIFOREST_PATH" $BATCH_DIR/
    
    # Change to temporary directory for compilation
    pushd $TMP_COMPILE_DIR > /dev/null
    
    log 1 "Pre-compiling SkimHiForest.C in temporary directory..."
    log 3 "Running ROOT compilation command..."
    
    # Try both compilation methods to ensure all needed files are generated
    COMPILATION_SUCCESS=0
    
    # Method 1: Compile with kf option for libSkimHiForest.so
    log 2 "Method 1: Compiling with CompileMacro and kf option..."
    COMPILE_CMD1="root -l -b -q -e \"int result = gSystem->CompileMacro(\\\"SkimHiForest.C\\\", \\\"kf\\\", \\\"libSkimHiForest\\\", \\\"\\\"); printf(\\\"COMPILE_RESULT=%d\\\\n\\\", result);\""
    log 3 "Command 1: $COMPILE_CMD1"
    COMPILE_OUTPUT1=$(eval $COMPILE_CMD1 2>&1)
    COMPILE_RESULT1=$(echo "$COMPILE_OUTPUT1" | grep "COMPILE_RESULT" | cut -d= -f2)
    
    # Log the results
    if [[ "$COMPILE_RESULT1" == "1" ]]; then
        log 1 "Method 1 compilation successful"
        COMPILATION_SUCCESS=1
        ls -la libSkimHiForest* >> compile_artifacts.log
    else
        log 0 "Method 1 compilation failed. See output:"
        echo "$COMPILE_OUTPUT1"
    fi
    
    # Method 2: Compile with .L+ for compatibility with .L+ loading
    log 2 "Method 2: Compiling with .L+ method..."
    COMPILE_CMD2="root -l -b -q -e \"gROOT->ProcessLine(\\\".L SkimHiForest.C+\\\"); printf(\\\"COMPILE2_RESULT=1\\\\n\\\");\""
    log 3 "Command 2: $COMPILE_CMD2"
    COMPILE_OUTPUT2=$(eval $COMPILE_CMD2 2>&1)
    COMPILE_RESULT2=$(echo "$COMPILE_OUTPUT2" | grep "COMPILE2_RESULT" | cut -d= -f2)
    
    # Log the results
    if [[ "$COMPILE_RESULT2" == "1" ]]; then
        log 1 "Method 2 compilation successful"
        COMPILATION_SUCCESS=1
        ls -la SkimHiForest_C* >> compile_artifacts.log
    else
        log 0 "Method 2 compilation failed. See output:"
        echo "$COMPILE_OUTPUT2"
    fi
    
    # Show all compilation outputs in debug mode
    if [[ $VERBOSITY -ge 2 ]]; then
        echo "Compilation output 1:"
        echo "$COMPILE_OUTPUT1"
        echo "Compilation output 2:"
        echo "$COMPILE_OUTPUT2"
        echo "Generated files:"
        ls -la
    fi
    
    if [[ $COMPILATION_SUCCESS -eq 1 ]]; then
        log 1 "Successfully pre-compiled SkimHiForest.C using at least one method"
        
        # List all generated files and their details
        log 2 "Listing all generated compilation files:"
        ls -la
        
        # Copy ALL possible compilation artifacts to the batch directory
        log 2 "Copying all compilation artifacts to batch directory..."
        mkdir -p $BATCH_DIR/lib
        
        # Copy everything that might be relevant
        cp -v SkimHiForest_C*.so SkimHiForest_C*.d SkimHiForest_C_ACLiC_* $BATCH_DIR/ 2>/dev/null || true
        cp -v libSkimHiForest*.so libSkimHiForest_ACLiC_* $BATCH_DIR/ 2>/dev/null || true
        cp -v *_rdict.pcm *_ACLiC_dict_rdict.pcm $BATCH_DIR/ 2>/dev/null || true
        
        # Go to batch dir and check what was copied
        pushd $BATCH_DIR > /dev/null
        
        # Collect information about all library files
        LIB_FILES=$(find . -maxdepth 1 -type f -name "*.so" -o -name "*_rdict.pcm" -o -name "*.d" -o -name "*_dict_*" | grep -v "^\.\/\.")
        
        if [[ -n "$LIB_FILES" ]]; then
            log 1 "Found compiled libraries and dictionaries in batch dir:"
            echo "COMPILATION ARTIFACTS:" > $BATCH_DIR/library_info.txt
            for file in $LIB_FILES; do
                # Make sure files are readable by all
                chmod 644 "$file"
                
                # Get file info for debugging
                FILE_SIZE=$(stat -c %s "$file")
                FILE_PERMS=$(stat -c %a "$file")
                echo "  $file: $FILE_SIZE bytes, permissions $FILE_PERMS" >> $BATCH_DIR/library_info.txt
                log 2 "  $file: $FILE_SIZE bytes, permissions $FILE_PERMS"
            done
        else
            log 0 "WARNING: Compilation reported success but no library files found in batch directory."
            log 0 "This might cause problems during execution."
        fi
        
        # Return to original directory
        popd > /dev/null
    else
        log 0 "ERROR: All pre-compilation methods failed. Aborting submission."
        log 0 "Please run with --verbose for more details"
        popd > /dev/null
        rm -rf $TMP_COMPILE_DIR
        exit 1
    fi
    
    # Return to original directory and clean up
    popd > /dev/null
    log 3 "Cleaning up temporary directory..."
    rm -rf $TMP_COMPILE_DIR
else
    # If not precompiling, copy the .so if it exists in the main dir
    if [[ -f "$WORKING_DIR/libSkimHiForest.so" ]]; then
        log 1 "Copying existing libSkimHiForest.so from working directory"
        cp "$WORKING_DIR/libSkimHiForest.so" .
        log 3 "Library size: $(stat -c %s "libSkimHiForest.so") bytes"
        log 3 "Library timestamp: $(stat -c %y "libSkimHiForest.so")"
    else
        log 2 "No existing libSkimHiForest.so found in working directory"
    fi
fi

# Prepare logs directory inside batch dir
LOGS_DIR="$BATCH_DIR/logs"
log 2 "Creating logs directory: $LOGS_DIR"
mkdir -p "$LOGS_DIR"

# Find CMSSW base directory
log 2 "Finding CMSSW base directory..."
CMSSW_BASE_DIR=$(cd "$WORKING_DIR"; while [[ "$PWD" != "/" ]]; do if [[ -f "$PWD/config/scram_version" ]]; then echo "$PWD"; break; fi; cd ..; done)

if [[ -n "$CMSSW_BASE_DIR" ]]; then
    log 2 "Found CMSSW base directory: $CMSSW_BASE_DIR"
else
    log 2 "No CMSSW installation found"
fi

# Prepare the transfer_input_files value (relative to batch dir)
log 2 "Preparing transfer input files list..."
TRANSFER_FILES="$BATCH_DIR/SkimHiForest.C"

# Add the run_batch.sh script to the batch directory if not there
if [[ ! -f "$BATCH_DIR/run_batch.sh" ]]; then
    log 2 "Copying run_batch.sh to batch directory"
    cp "$WORKING_DIR/run_batch.sh" "$BATCH_DIR/"
    chmod +x "$BATCH_DIR/run_batch.sh"
fi

# Check for all possible library and dictionary files to transfer
# Expanded to include all possible ROOT compilation artifacts
ROOT_FILES=(
    # Main libraries
    "libSkimHiForest.so" 
    "SkimHiForest_C.so"
    
    # Dictionary files
    "libSkimHiForest_ACLiC_dict_rdict.pcm" 
    "SkimHiForest_C_ACLiC_dict_rdict.pcm"
    "libSkimHiForest_rdict.pcm"
    "SkimHiForest_C_rdict.pcm"
    
    # Definition files
    "libSkimHiForest.d"
    "SkimHiForest_C.d"
    
    # Other potential artifacts
    "libSkimHiForest_ACLiC_dict.cxx"
    "SkimHiForest_C_ACLiC_dict.cxx"
)

# Create a file with transfer file information for debugging
TRANSFER_INFO_FILE="$BATCH_DIR/transfer_files_info.txt"
echo "TRANSFER FILES INFORMATION - $(date)" > $TRANSFER_INFO_FILE
echo "----------------------------------------" >> $TRANSFER_INFO_FILE

# Start with the required SkimHiForest.C file
echo "SkimHiForest.C: $(stat -c %s "$BATCH_DIR/SkimHiForest.C") bytes, $(stat -c %A "$BATCH_DIR/SkimHiForest.C") permissions" >> $TRANSFER_INFO_FILE

# Add all generated files from batch directory
for file in "${ROOT_FILES[@]}"; do
    if [[ -f "$BATCH_DIR/$file" ]]; then
        log 2 "Adding $file to transfer list"
        TRANSFER_FILES="$TRANSFER_FILES,$BATCH_DIR/$file"
        
        # Make sure permissions are correct (readable by all)
        chmod 644 "$BATCH_DIR/$file"
        
        # List file details for debugging
        FILE_SIZE=$(stat -c %s "$BATCH_DIR/$file")
        FILE_PERMS=$(stat -c %A "$BATCH_DIR/$file")
        log 3 "File: $BATCH_DIR/$file"
        log 3 "Size: $FILE_SIZE bytes"
        log 3 "Permissions: $FILE_PERMS"
        
        # Add to transfer info file
        echo "$file: $FILE_SIZE bytes, $FILE_PERMS permissions" >> $TRANSFER_INFO_FILE
    else
        log 3 "File not found: $BATCH_DIR/$file"
    fi
done

# Search for any other potential compilation artifacts we might have missed
OTHER_ARTIFACTS=$(find "$BATCH_DIR" -maxdepth 1 -type f \( -name "*.so" -o -name "*_rdict.pcm" -o -name "*.d" -o -name "*_dict.cxx" \) | grep -v "run_batch.sh" | grep -v "SkimHiForest.C")

for artifact in $OTHER_ARTIFACTS; do
    FILENAME=$(basename "$artifact")
    if [[ ! $TRANSFER_FILES == *"$FILENAME"* ]]; then
        log 2 "Found additional artifact to transfer: $FILENAME"
        TRANSFER_FILES="$TRANSFER_FILES,$artifact"
        
        # Make sure permissions are correct
        chmod 644 "$artifact"
        
        # Add to transfer info file
        echo "$FILENAME: $(stat -c %s "$artifact") bytes, $(stat -c %A "$artifact") permissions (additional)" >> $TRANSFER_INFO_FILE
    fi
done

# Add the transfer info file itself
echo "transfer_files_info.txt: Information file about transferred files" >> $TRANSFER_INFO_FILE
TRANSFER_FILES="$TRANSFER_FILES,$TRANSFER_INFO_FILE"

log 1 "Total files to transfer: $(echo $TRANSFER_FILES | tr ',' '\n' | wc -l)"
log 2 "Transfer files: $TRANSFER_FILES"

# Create a condor submission file in the batch dir
log 1 "Creating condor submission file..."
CONDOR_FILE="$BATCH_DIR/submit_${CONFIG_BASENAME}_${TIMESTAMP}.condor"
log 2 "Condor file: $CONDOR_FILE"

log 3 "Creating condor submission template..."
cat > $CONDOR_FILE << 'CONDOR_EOL'
# Condor submission file for HiForest skimming
# Config: CONFIG_FILE_PLACEHOLDER
# Created: CREATION_DATE_PLACEHOLDER
universe = vanilla
executable = EXECUTABLE_PLACEHOLDER
arguments = CONFIG_FILE_PLACEHOLDER $(Process) BATCH_DIR_PLACEHOLDER USE_CMSSW_PLACEHOLDER
output = LOGS_DIR_PLACEHOLDER/skim_$(Process).out
error = LOGS_DIR_PLACEHOLDER/skim_$(Process).err
log = LOGS_DIR_PLACEHOLDER/skim_$(Process).log
request_memory = 4GB
request_disk = 2GB
should_transfer_files = YES
when_to_transfer_output = ON_EXIT
transfer_input_files = TRANSFER_FILES_PLACEHOLDER
environment = "CMSSW_BASE=CMSSW_BASE_PLACEHOLDER SCRAM_ARCH=el8_amd64_gcc11 PATH=PATH_PLACEHOLDER LD_LIBRARY_PATH=LD_LIBRARY_PATH_PLACEHOLDER ROOT_INCLUDE_PATH=."
requirements = (OpSysAndVer =?= "CentOS7" || OpSysAndVer =?= "CentOS8" || OpSysAndVer =?= "AlmaLinux9")
max_retries = 2
+JobFlavour = "espresso"
CONDOR_EOL

# Now substitute our values properly
log 2 "Substituting template variables with actual values..."
log 3 "Substituting CONFIG_FILE_PLACEHOLDER with: $CONFIG_FILE"
sed -i "s|CONFIG_FILE_PLACEHOLDER|${CONFIG_FILE}|g" $CONDOR_FILE

log 3 "Substituting CREATION_DATE_PLACEHOLDER"
sed -i "s|CREATION_DATE_PLACEHOLDER|$(date)|g" $CONDOR_FILE

log 3 "Substituting EXECUTABLE_PLACEHOLDER with: ${BATCH_DIR}/run_batch.sh"
sed -i "s|EXECUTABLE_PLACEHOLDER|${BATCH_DIR}/run_batch.sh|g" $CONDOR_FILE

log 3 "Substituting other placeholders..."
sed -i "s|BATCH_DIR_PLACEHOLDER|${BATCH_DIR}|g" $CONDOR_FILE
sed -i "s|USE_CMSSW_PLACEHOLDER|${USE_CMSSW}|g" $CONDOR_FILE
sed -i "s|LOGS_DIR_PLACEHOLDER|${LOGS_DIR}|g" $CONDOR_FILE
sed -i "s|CMSSW_BASE_PLACEHOLDER|${CMSSW_BASE_DIR}|g" $CONDOR_FILE
sed -i "s|PATH_PLACEHOLDER|${PATH}|g" $CONDOR_FILE
sed -i "s|LD_LIBRARY_PATH_PLACEHOLDER|${LD_LIBRARY_PATH}|g" $CONDOR_FILE

# Substitute the correct value for transfer_input_files (executable is now set directly)
log 3 "Substituting TRANSFER_FILES_PLACEHOLDER with: $TRANSFER_FILES"
sed -i "s|TRANSFER_FILES_PLACEHOLDER|$TRANSFER_FILES|g" $CONDOR_FILE

# Set appropriate number of batches to process
log 2 "Finalizing batch count..."
if [[ -z "$TOTAL_BATCHES" || "$TOTAL_BATCHES" -le 0 ]]; then
    if [[ $MAX_BATCHES -gt 0 ]]; then
        TOTAL_BATCHES=$MAX_BATCHES
        log 2 "Using MAX_BATCHES value: $TOTAL_BATCHES"
    else
        TOTAL_BATCHES=1
        log 0 "WARNING: No batch count determined, defaulting to 1 batch"
    fi
fi
log 1 "Will submit $TOTAL_BATCHES job(s)"
log 3 "Adding 'queue $TOTAL_BATCHES' to condor file"
echo "queue $TOTAL_BATCHES" >> $CONDOR_FILE

# Copy the batch script to the batch dir
log 2 "Copying batch script to batch directory..."
if [[ ! -f "$WORKING_DIR/run_batch.sh" ]]; then
    log 0 "ERROR: Batch script not found: $WORKING_DIR/run_batch.sh"
    exit 1
fi
cp "$WORKING_DIR/run_batch.sh" "$BATCH_DIR/run_batch.sh"
chmod +x "$BATCH_DIR/run_batch.sh"
log 3 "Batch script copied and made executable"

# Print summary
log 2 "Returning to working directory"
cd "$WORKING_DIR"

log 1 "==================== SUBMISSION SUMMARY ===================="
log 1 "Condor submission files prepared:"
log 1 "  Submission file: $CONDOR_FILE"
log 1 "  Batch script: $BATCH_DIR/run_batch.sh"
log 1 "  Logs directory: $LOGS_DIR"
log 1 "  Batch working directory: $BATCH_DIR"
log 1 "  Total jobs: $TOTAL_BATCHES"
log 1 ""

# Print detailed information in debug mode
if [[ $VERBOSITY -ge 2 ]]; then
    log 2 "============== ENVIRONMENT INFORMATION ================"
    log 2 "CMSSW_BASE: $CMSSW_BASE_DIR"
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
    SUBMIT_OUTPUT=$(condor_submit $CONDOR_FILE)
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
        log 1 "  condor_q -submitter $(whoami)"
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