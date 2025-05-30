#!/bin/bash

# Simplified production batch submission script for gammaJetAnalyzer
# Focuses on file-based batch processing with HTCondor

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

show_help() {
    echo "Simplified batch submission for gammaJetAnalyzer analysis"
    echo ""
    echo "Usage: $0 [OPTIONS] ANALYSIS_CONFIG PLOT_CONFIG"
    echo ""
    echo "Arguments:"
    echo "  ANALYSIS_CONFIG         Analysis configuration file"
    echo "  PLOT_CONFIG            Plot configuration file"
    echo ""
    echo "Options:"
    echo "  -h, --help              Show this help message"
    echo "  -j, --max-jobs N        Maximum number of jobs to submit (default: all)"
    echo "  -n, --files-per-job N   Number of input files per job (default: 5)"
    echo "  -f, --flavour FLAVOUR   Job flavour: espresso, microcentury, longlunch, workday, tomorrow (default: longlunch)"
    echo "  -d, --dry-run           Show what would be submitted without submitting"
    echo "  -v, --verbose           Enable verbose output (level 2)"
    echo "  --verbosity LEVEL       Set verbosity level: 0=minimal, 1=normal, 2=debug, 3=trace (default: 1)"
    echo "  --job-name NAME         Custom job name prefix (default: auto-generated)"
    echo ""
    echo "Examples:"
    echo "  $0 ../configs/JetSub_2023_PbPb_MC.config ../configs/PlotJetSub_2023_PbPb_MC.config"
    echo "  $0 -j 20 -n 3 analysis.config plot.config"
    echo "  $0 -f workday --dry-run analysis.config plot.config"
    echo "  $0 --verbosity 2 analysis.config plot.config"
}

# Default parameters
MAX_JOBS=0  # 0 means process all jobs
FILES_PER_JOB=5
JOB_FLAVOUR="longlunch"
DRY_RUN=false
VERBOSITY=1    # Default verbosity level: 0=minimal, 1=normal, 2=debug, 3=trace
CUSTOM_JOB_NAME=""
ANALYSIS_CONFIG=""
PLOT_CONFIG=""

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

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -j|--max-jobs)
            MAX_JOBS="$2"
            shift 2
            ;;
        -n|--files-per-job)
            FILES_PER_JOB="$2"
            shift 2
            ;;
        --files-per-job)
            FILES_PER_JOB="$2"
            shift 2
            ;;
        -f|--flavour)
            JOB_FLAVOUR="$2"
            shift 2
            ;;
        -d|--dry-run)
            DRY_RUN=true
            shift
            ;;
        -v|--verbose)
            VERBOSITY=2
            shift
            ;;
        --verbosity)
            VERBOSITY="$2"
            shift 2
            ;;
        --job-name)
            CUSTOM_JOB_NAME="$2"
            shift 2
            ;;
        -*|--*)
            log 0 "Unknown option: $1"
            show_help
            exit 1
            ;;
        *)
            # First positional argument is analysis config
            if [[ -z "$ANALYSIS_CONFIG" ]]; then
                ANALYSIS_CONFIG="$1"
            # Second positional argument is plot config
            elif [[ -z "$PLOT_CONFIG" ]]; then
                PLOT_CONFIG="$1"
            else
                log 0 "Too many arguments provided"
                exit 1
            fi
            shift
            ;;
    esac
done

# Validate inputs
if [[ -z "$ANALYSIS_CONFIG" || -z "$PLOT_CONFIG" ]]; then
    log 0 "Both analysis config and plot config files must be specified"
    show_help
    exit 1
fi

# Check if config files exist
if [ ! -f "$ANALYSIS_CONFIG" ]; then
    log 0 "Analysis configuration file not found: $ANALYSIS_CONFIG"
    exit 1
fi

if [ ! -f "$PLOT_CONFIG" ]; then
    log 0 "Plot configuration file not found: $PLOT_CONFIG"
    exit 1
fi

# Get input files from analysis config
get_input_files() {
    local analysis_config=$1
    local input_dir=$(grep "^InputDir" "$analysis_config" | cut -d' ' -f2)
    if [ -z "$input_dir" ]; then
        log 0 "InputDir not found in $analysis_config"
        return 1
    fi
    find "$input_dir" -name "*.root" 2>/dev/null | sort
}

# Split files into batches
split_files_into_batches() {
    local -n files_ref=$1
    local files_per_job=$2
    local max_jobs=$3
    local -n batches_ref=$4
    
    local total_files=${#files_ref[@]}
    local calculated_jobs=$(( (total_files + files_per_job - 1) / files_per_job ))
    
    # If max_jobs is 0, use all calculated jobs; otherwise limit to max_jobs
    if [ $max_jobs -eq 0 ]; then
        # Use all calculated jobs
        max_jobs=$calculated_jobs
    elif [ $calculated_jobs -gt $max_jobs ]; then
        calculated_jobs=$max_jobs
        log 1 "Limiting to $max_jobs jobs (would need $(( (total_files + files_per_job - 1) / files_per_job )) for all files)"
    fi
    
    log 2 "Splitting $total_files files into $calculated_jobs jobs ($files_per_job files per job)"
    
    for ((job_id=0; job_id<calculated_jobs; job_id++)); do
        local start_idx=$((job_id * files_per_job))
        local end_idx=$((start_idx + files_per_job - 1))
        local job_files=()
        
        for ((i=start_idx; i<=end_idx && i<total_files; i++)); do
            job_files+=("${files_ref[i]}")
        done
        
        if [ ${#job_files[@]} -gt 0 ]; then
            # Join files with comma
            local file_list=$(IFS=','; echo "${job_files[*]}")
            batches_ref+=("$file_list")
            log 3 "Job $job_id: ${#job_files[@]} files"
        fi
    done
}

# Main execution
log 1 "Starting simplified batch submission for gammaJetAnalyzer"
log 1 "Analysis config: $ANALYSIS_CONFIG"
log 1 "Plot config: $PLOT_CONFIG"
if [ $MAX_JOBS -eq 0 ]; then
    log 1 "Max jobs: all available"
else
    log 1 "Max jobs: $MAX_JOBS"
fi
log 1 "Files per job: $FILES_PER_JOB"
log 1 "Job flavour: $JOB_FLAVOUR"
if [ -n "$CUSTOM_JOB_NAME" ]; then
    log 1 "Custom job name: $CUSTOM_JOB_NAME"
fi
log 1 "Dry run: $DRY_RUN"

# Get input files
log 2 "Discovering input files..."
all_files=( $(get_input_files "$ANALYSIS_CONFIG") )
total_files=${#all_files[@]}

if [ $total_files -eq 0 ]; then
    log 0 "No input files found"
    exit 1
fi

log 1 "Found $total_files input files"

# Split files into batches
log 2 "Creating file batches..."
file_batches=()
split_files_into_batches all_files "$FILES_PER_JOB" "$MAX_JOBS" file_batches

num_jobs=${#file_batches[@]}
if [ $num_jobs -eq 0 ]; then
    log 0 "No jobs created"
    exit 1
fi

log 1 "Will submit $num_jobs jobs"

# Create batch directory
config_name=$(basename "$ANALYSIS_CONFIG" .config)
timestamp=$(date '+%Y%m%d_%H%M%S')
if [ -n "$CUSTOM_JOB_NAME" ]; then
    job_name="$CUSTOM_JOB_NAME"
else
    job_name="${config_name}_${timestamp}"
fi
batch_dir="batch/job_${job_name}"

if [ "$DRY_RUN" = false ]; then
    mkdir -p "$batch_dir"
    log 2 "Created batch directory: $batch_dir"
    
    # Copy necessary files
    log 2 "Copying files to batch directory..."
    cp gammaJetAnalyzer.cpp "$batch_dir/" 2>/dev/null || log 1 "Warning: gammaJetAnalyzer.cpp not found"
    cp -r include "$batch_dir/" 2>/dev/null || log 1 "Warning: include directory not found"
    cp Makefile "$batch_dir/" 2>/dev/null || log 1 "Warning: Makefile not found"
    cp "$ANALYSIS_CONFIG" "$batch_dir/" 2>/dev/null || log 1 "Warning: analysis config not found"
    cp "$PLOT_CONFIG" "$batch_dir/" 2>/dev/null || log 1 "Warning: plot config not found"
fi

analysis_config_basename=$(basename "$ANALYSIS_CONFIG")
plot_config_basename=$(basename "$PLOT_CONFIG")

# Create job script
job_script_content="#!/bin/bash
echo \"=== gammaJetAnalyzer Job Start ===\"
echo \"Job started: \$(date) on \$(hostname)\"
echo \"Working directory: \$(pwd)\"
echo \"HTCondor Process: \${_CONDOR_PROCNO:-0}\"
echo \"\"

# Get job files from environment
FILES_LIST=\${JOB_FILES:-\"\"}
echo \"Files for this job: \$FILES_LIST\"
echo \"\"

echo \"Available files in job directory:\"
ls -la
echo \"\"

echo \"=== Compiling gammaJetAnalyzer ===\"
make -j || exit 1
echo \"Compilation successful\"
echo \"\"

echo \"=== Running Analysis ===\"
if [ -n \"\$FILES_LIST\" ]; then
    echo \"Running with files: \$FILES_LIST\"
    ./gammaJetAnalyzer -c $analysis_config_basename -p $plot_config_basename --files \"\$FILES_LIST\"
else
    echo \"Error: No files specified\"
    exit 1
fi
exit_code=\$?

echo \"\"
echo \"=== Job Completion ===\"
echo \"Analysis exit code: \$exit_code\"
echo \"Job completed: \$(date)\"
echo \"Output files:\"
ls -la *.root 2>/dev/null || echo \"No ROOT files produced\"

exit \$exit_code"

# Create HTCondor submit file
submit_file_content="# HTCondor submit file for gammaJetAnalyzer
universe = vanilla
executable = run_job.sh
output = gammaJetAnalyzer_\$(ClusterId).\$(ProcId).out
error = gammaJetAnalyzer_\$(ClusterId).\$(ProcId).err
log = gammaJetAnalyzer_\$(ClusterId).\$(ProcId).log

# File transfer settings
should_transfer_files = YES
when_to_transfer_output = ON_EXIT

# Job requirements
+JobFlavour = \"$JOB_FLAVOUR\"
RequestCpus = 1
RequestMemory = 2GB

# Job identification
JobBatchName = gammaJetAnalyzer_$job_name
"

# Add queue entries for each job
for ((job_id=0; job_id<num_jobs; job_id++)); do
    submit_file_content+="\nenvironment = \"JOB_FILES=${file_batches[job_id]}\""
    submit_file_content+="\nqueue 1"
done

if [ "$DRY_RUN" = true ]; then
    log 1 "=== DRY RUN OUTPUT ==="
    log 1 "Would create batch directory: $batch_dir"
    log 1 "Would create $num_jobs jobs with the following file assignments:"
    for ((job_id=0; job_id<num_jobs; job_id++)); do
        log 1 "  Job $job_id: ${file_batches[job_id]}"
    done
    log 2 "Job script content:"
    log 2 "-------------------"
    echo "$job_script_content"
    log 2 "-------------------"
    log 2 "HTCondor submit file content:"
    log 2 "----------------------------"
    echo "$submit_file_content"
    log 2 "----------------------------"
    log 1 "Would execute: cd $batch_dir && condor_submit submit.sub"
else
    # Write files and submit
    echo "$job_script_content" > "$batch_dir/run_job.sh"
    chmod +x "$batch_dir/run_job.sh"
    
    echo "$submit_file_content" > "$batch_dir/submit.sub"
    
    log 1 "=== Submitting Jobs ==="
    cd "$batch_dir"
    if condor_submit submit.sub; then
        log 1 "✅ Successfully submitted $num_jobs jobs!"
        log 1 "Job name: $job_name"
        log 1 "Job directory: $batch_dir"
        log 1 "Monitor with: condor_q"
        log 1 "Check logs: ls -la $batch_dir/gammaJetAnalyzer_*.{out,err,log}"
    else
        log 0 "❌ Failed to submit jobs"
        exit 1
    fi
fi

log 1 ""
log 1 "=== Summary ==="
log 1 "Jobs: $num_jobs"
log 1 "Files per job: $FILES_PER_JOB"
log 1 "Total files to process: $((num_jobs * FILES_PER_JOB))"
log 1 "Job flavour: $JOB_FLAVOUR"
