#!/bin/bash

# Monitor batch jobs for photonJet analysis
# Usage: ./monitor_jobs.sh [job_id] [--continuous]

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BATCH_DIR="${SCRIPT_DIR}/batch"

show_help() {
    echo "Monitor batch jobs for photonJet analysis"
    echo ""
    echo "Usage: $0 [OPTIONS] [JOB_ID]"
    echo ""
    echo "Options:"
    echo "  -h, --help        Show this help message"
    echo "  -c, --continuous  Monitor continuously (refresh every 30s)"
    echo "  -a, --all         Show all user jobs"
    echo "  -l, --logs        Show recent log output"
    echo "  -s, --summary     Show job summary only"
    echo ""
    echo "Examples:"
    echo "  $0                     # Show all running jobs"
    echo "  $0 5385690.0          # Monitor specific job"
    echo "  $0 -c 5385690.0       # Monitor job continuously"
    echo "  $0 -l 5385690.0       # Show log output for job"
}

get_job_directory() {
    local job_id=$1
    # Try to find job directory by looking for recent submissions
    if [ -d "$BATCH_DIR" ]; then
        find "$BATCH_DIR" -name "*.sub" -newer "$BATCH_DIR" 2>/dev/null | head -1 | xargs dirname 2>/dev/null
    fi
}

show_job_status() {
    local job_id=$1
    echo "=== Job Status for $job_id ==="
    condor_q "$job_id" 2>/dev/null || echo "Job not found in queue (may have completed)"
    
    echo ""
    echo "=== Job Details ==="
    condor_q -l "$job_id" 2>/dev/null | grep -E "(JobStatus|RemoteHost|JobStartDate|QDate|ExitCode)" | while read line; do
        if [[ $line == *"JobStartDate"* ]]; then
            timestamp=$(echo $line | cut -d'=' -f2 | tr -d ' ')
            echo "$line ($(date -d @$timestamp 2>/dev/null || echo 'Invalid timestamp'))"
        elif [[ $line == *"QDate"* ]]; then
            timestamp=$(echo $line | cut -d'=' -f2 | tr -d ' ')
            echo "$line ($(date -d @$timestamp 2>/dev/null || echo 'Invalid timestamp'))"
        else
            echo "$line"
        fi
    done
    
    # Check if job has finished
    echo ""
    echo "=== Job History (if completed) ==="
    condor_history -l "$job_id" 2>/dev/null | grep -E "(ExitCode|JobStatus|RemoteWallClockTime)" | head -3
}

show_job_logs() {
    local job_id=$1
    local job_dir=$(get_job_directory "$job_id")
    
    if [ -z "$job_dir" ]; then
        echo "Could not find job directory for $job_id"
        return 1
    fi
    
    echo "=== Job Directory: $job_dir ==="
    if [ -d "$job_dir/logs" ]; then
        echo "=== Available Log Files ==="
        ls -la "$job_dir/logs/"
        
        echo ""
        echo "=== Recent Output (last 20 lines) ==="
        for logfile in "$job_dir/logs"/*.out; do
            if [ -f "$logfile" ]; then
                echo "--- $logfile ---"
                tail -20 "$logfile" 2>/dev/null || echo "Could not read $logfile"
                echo ""
            fi
        done
        
        echo "=== Recent Errors (last 10 lines) ==="
        for errfile in "$job_dir/logs"/*.err; do
            if [ -f "$errfile" ]; then
                echo "--- $errfile ---"
                tail -10 "$errfile" 2>/dev/null || echo "Could not read $errfile"
                echo ""
            fi
        done
    else
        echo "No logs directory found in $job_dir"
    fi
}

# Parse command line arguments
CONTINUOUS=false
SHOW_ALL=false
SHOW_LOGS=false
SUMMARY_ONLY=false
JOB_ID=""

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -c|--continuous)
            CONTINUOUS=true
            shift
            ;;
        -a|--all)
            SHOW_ALL=true
            shift
            ;;
        -l|--logs)
            SHOW_LOGS=true
            shift
            ;;
        -s|--summary)
            SUMMARY_ONLY=true
            shift
            ;;
        *)
            if [[ $1 =~ ^[0-9]+\.[0-9]+$ ]]; then
                JOB_ID=$1
            else
                echo "Unknown option: $1"
                show_help
                exit 1
            fi
            shift
            ;;
    esac
done

# Main monitoring logic
if [ "$CONTINUOUS" = true ]; then
    echo "Starting continuous monitoring (press Ctrl+C to stop)..."
    while true; do
        clear
        echo "=== Batch Job Monitor - $(date) ==="
        echo ""
        
        if [ -n "$JOB_ID" ]; then
            show_job_status "$JOB_ID"
            if [ "$SHOW_LOGS" = true ]; then
                echo ""
                show_job_logs "$JOB_ID"
            fi
        else
            echo "=== All User Jobs ==="
            condor_q bharikri
        fi
        
        echo ""
        echo "Refreshing in 30 seconds... (Ctrl+C to stop)"
        sleep 30
    done
else
    if [ -n "$JOB_ID" ]; then
        if [ "$SHOW_LOGS" = true ]; then
            show_job_logs "$JOB_ID"
        else
            show_job_status "$JOB_ID"
        fi
    elif [ "$SHOW_ALL" = true ]; then
        echo "=== All User Jobs ==="
        condor_q bharikri
    else
        echo "=== Current Jobs ==="
        condor_q bharikri
        
        echo ""
        echo "=== Recent Job Directories ==="
        if [ -d "$BATCH_DIR" ]; then
            ls -lt "$BATCH_DIR" | head -5
        fi
    fi
fi
