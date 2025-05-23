# HiForest Skimming Documentation (AI generated)

## Overview

The skimming system processes HiForest files using ROOT's RDataFrame to create smaller, analysis-ready files. It is implemented as a standalone C++ executable for improved reliability and performance with batch processing support via HTCondor.

### Key Components

1. **Executable Structure**
   - Standalone C++ program with main()
   - No dependency on ROOT's interpreter
   - Pre-compiled for consistent behavior

2. **Build System**
   - Simple Makefile for compilation
   - Produces optimized executable
   - Proper library linkage

3. **Execution Modes**
   - Direct local execution
   - HTCondor batch submission
   - Configurable EL8/EL9 OS version selection

### Usage

1. **Building**
```bash
# Build the executable
make clean && make

# Just rebuild without cleaning
make
```

2. **Local Execution**
```bash
# Run with a config file
./SkimHiForest configs/your_config.config

# Run a specific batch
./SkimHiForest configs/your_config.config 3
```

3. **Batch Submission**
```bash
# Submit jobs to HTCondor
./submit_condor_jobs.sh -c configs/your_config.config

# Submit with pre-compilation
./submit_condor_jobs.sh -c configs/your_config.config -p

# Submit with verbose debugging
./submit_condor_jobs.sh -c configs/your_config.config -v

# Submit a limited number of batches
./submit_condor_jobs.sh -c configs/your_config.config -n 5

# Remove existing jobs before submitting
./submit_condor_jobs.sh -c configs/your_config.config -r

# Submit jobs specifying OS version
./submit_condor_jobs.sh -c configs/your_config.config --os-version el8
```

### Cross-Architecture Support

The system automatically handles EL8/EL9 differences:

- **EL9 Systems**: Direct execution
- **EL8 Systems**: Automatic container usage via singularity/apptainer

Specify the OS version in the job submission script to ensure compatibility.

## Debugging Lessons

1. **Branch Name Resolution**
- Base tree branches have no prefix in RDataFrame
- Friend tree branches follow pattern: alias.branch
- Must check both formats when searching columns

2. **Tree Configuration**
- Base tree must be HiTree for correct event structure
- Friend tree order affects branch naming
- Tree aliases must match config file exactly

3. **Data Processing**
- RDataFrame Snapshot preserves original branch structure
- Friend tree relationships maintained through processing
- Column name mapping handled automatically by ROOT

## TODO List

1. **Performance Optimization**
- [x] Evaluate RDF column filtering performance
- [x] Implement batch processing
- [ ] Profile memory usage with large files
- [x] Fix file size limitations (100GB issue)

2. **Error Handling**
- [x] Add validation for config file format
- [x] Improve missing column reporting
- [ ] Add checks for tree/branch existence
- [x] Fix BatchMode parameter inheritance in condor jobs

3. **Features**
- [ ] Support regex in branch selection
- [ ] Add event filtering options
- [x] Implement progress reporting
- [x] Add verbosity levels for debugging

4. **Documentation**
- [x] Add examples for common use cases
- [x] Document config file format thoroughly
- [x] Create troubleshooting guide
- [ ] Document EOS output file handling best practices

## Best Practices

1. **Development**
- Use verbose debug output during development
- Test with small file samples first
- Verify column names in RDataFrame immediately

2. **Configuration**
- Keep base tree (HiTree) first in config
- Use consistent alias naming
- Document branch selection patterns

3. **Validation**
- Check output file size and content
- Verify branch name preservation
- Ensure event counts match expectations

## Common Issues

1. **Missing Branches**
- Check tree/alias names match exactly
- Verify branch exists in input file
- Confirm correct prefix usage

2. **Tree Loading**
- HiTree must be base tree
- Friend trees need proper paths
- Chain addition order matters

3. **Output Format**
- Branch names follow RDataFrame conventions
- Friend tree prefixes preserved
- Original data types maintained

## CERN HTCondor Specifics

The skimming system is optimized for running on CERN's HTCondor batch system:

### Operating System Requirements

- **Operating System Selection**: Jobs can now specifically request either AlmaLinux 8 or 9 worker nodes
- **Condor Requirements Setting**:
  ```
  # When requesting a specific OS version:
  MY.WantOS = "el8"  # For AlmaLinux 8
  # or
  MY.WantOS = "el9"  # For AlmaLinux 9
  ```
- **Default Behavior**: If no OS version is specified, HTCondor will choose a suitable worker node

### Job Flavor Settings

The system uses the "workday" flavor for jobs (up to 8 hours runtime):
```
+JobFlavour = "workday"
```

Available flavors at CERN:
- `espresso`: 20 minutes
- `microcentury`: 1 hour
- `longlunch`: 2 hours
- `workday`: 8 hours
- `tomorrow`: 1 day
- `testmatch`: 3 days
- `nextweek`: 1 week

### Resource Allocation

Default resource requests:
```
request_memory = 4GB
request_disk = 2GB
request_cpus = 1
```

### Wrapper Script

A wrapper script (`run_skimming.sh`) is created for each job to:
- Set up the execution environment
- Print diagnostic information
- Verify key parameters (BatchMode, Verbose level)
- Execute the SkimHiForest binary with proper arguments

### Best Practices for CERN Batch System

1. **Token Handling**:
   - When writing to EOS, ensure your grid tokens are valid
   - For long jobs, consider using `transfer_output_files` instead of direct EOS writing
   Refer to https://batchdocs.web.cern.ch/local/file_xfer_plugin.html

2. **Job Monitoring**:
   - Use `condor_q -better-analyze <job_id>` for detailed job status
   - Monitor system resource usage via `condor_q -l <job_id> | grep _usage`

3. **EOS Considerations**:
   - Avoid 100GB file size limit by using smaller batches
   - Consider using local disk for output, then copying to EOS after completion

## Batch Processing

The skimming system supports batch processing to efficiently handle large datasets. This feature:
- Divides input files into manageable batches
- Processes each batch individually to reduce memory usage
- Supports HTCondor job submission for parallel processing
- Creates separate output files for each batch with unique names

### Configuration

To enable batch processing, add the following parameters to your config file:

```
# Batch processing parameters
BatchMode 1            # Enable batch processing (0 = disabled, 1 = enabled)
FilesPerOutput 5       # Number of files per batch
```

> **Important**: The `BatchMode` parameter must be set to 1 in the config file used by the condor jobs. The `submit_condor_jobs.sh` script now automatically ensures this parameter is set correctly in the copied config file that gets transferred to the worker nodes.

### Usage Modes

#### 1. Local Batch Processing

Process all batches sequentially on the local machine:

```bash
# Using the compiled executable directly
./SkimHiForest configs/your_config.config
```

Process a specific batch (e.g., batch #3):

```bash
# Using the compiled executable directly (batch ID as second argument)
./SkimHiForest configs/your_config.config 3
```

#### 2. HTCondor Job Submission

The provided script `submit_condor_jobs.sh` automates the process of submitting jobs to HTCondor:

```bash
./submit_condor_jobs.sh -c configs/your_config.config
```

Options:
- `-c, --config CONFIG_FILE`: Path to the configuration file
- `-d, --dir WORKING_DIR`: Working directory (default: current directory)
- `-n, --max-batches NUM`: Maximum number of batches to process
- `-p, --precompile`: Compile SkimHiForest executable before submitting jobs
- `-r, --remove-jobs`: Remove existing condor jobs before submitting new ones
- `-v, --verbose`: Enable detailed debug output (level 2)
- `--trace`: Enable trace-level output (level 3, very verbose)
- `-q, --quiet`: Show only error messages (level 0)
- `--os-version VERSION`: Specify OS version to use (e.g., 'el8' or 'el9')
- `--verbosity LEVEL`: Set verbosity level manually (0-3)
- `--cleanup`: Clean up old compilation artifacts (default: preserve them)

After preparing the submission files, the script will prompt you to confirm before actually submitting the jobs.

#### 3. Manual Job Management

After jobs are submitted, you can monitor and manage them using standard HTCondor commands:

```bash
# Check the status of your jobs
condor_q -submitter $USER

# Remove all your jobs
condor_rm $USER

# Get details about a specific job
condor_q -better-analyze <job_id>
```

The job logs are stored in the `logs` directory within the batch job directory (e.g., `batch/job_20250523_093715_2023_PbPb_QCDPhoton/logs/`).

### Output Files

Batch processing creates output files with the following naming convention:
```
<OutName>_batch<BatchID>_of_<TotalBatches>.root
```

For example:
```
2025_05_22_QCDPhoton30_skimmed_batch0_of_10.root
2025_05_22_QCDPhoton30_skimmed_batch1_of_10.root
...
```

These files are written directly to the output directory specified in your configuration file. 
For direct writing to EOS storage, ensure you have proper permissions and tokens:

```
# EOS output setup example
OutputDir /eos/cms/store/group/phys_heavyions/bharikri/Run3GammaJet/2023_PbPb/MC/
```

> **Note about EOS**: When writing large files (approaching 100GB) directly to EOS, you may encounter problems. Consider using a smaller value for `FilesPerOutput` to keep individual output files smaller.

### Tips for Efficient Processing

1. **Determine optimal batch size**:
   - Too small: Excessive overhead from job startup and file I/O
   - Too large: High memory usage and longer per-job processing time
   - Recommended: 5-10 files per batch, depending on file size
   - When writing to EOS: Keep FilesPerOutput small enough to avoid large file issues

2. **Monitor job progress**:
   - For HTCondor: `condor_q -submitter <username>`
   - Check logs in the `logs` directory
   - Examine .out and .err files for detailed job information

3. **Handle failed jobs**:
   - Resubmit individual failed batches using the specific batch ID
   - Example: `./SkimHiForest configs/your_config.config 7`
   - Use the `-v` flag with submit_condor_jobs.sh for more detailed error information

4. **Resource allocation**:
   - The current defaults are appropriate for most workflows:
   - 4GB memory, 2GB disk space
   - "workday" job flavor (up to 8 hours runtime)
   - If processing takes longer, consider using "tomorrow" flavor instead

### Common Batch Processing Issues

1. **Memory usage too high**:
   - Decrease the number of files per batch
   - Use the `FileLimit` parameter to process a subset during development
   - Increase the memory allocation in the condor submission file

2. **Missing output files**:
   - Check the logs for errors
   - Verify that the output directory is writable
   - Ensure you have proper permissions for EOS directories

3. **TFile Merger errors or 100GB limit exceeded**:
   - Decrease the `FilesPerOutput` parameter to reduce output file size
   - Consider writing to local storage first, then copying to EOS
   - Ensure BatchMode is set to 1 to process files in batches

4. **Incomplete or corrupted outputs**:
   - Check for failed jobs in the logs
   - Examine the .err files for detailed error messages
   - Use the wrapper script debugging output to verify config settings

5. **ROOT version or environment issues**:
   - Use the `--precompile` option to ensure consistent executable
   - The script now has a parameter to request a specific OS version
   - Check in the .out file that the correct OS is being used

### Manual Environment Testing

If you're experiencing persistent issues, you can manually check your environment:

```bash
# Check OS version
cat /etc/os-release

# Check SCRAM_ARCH
echo $SCRAM_ARCH

# Test container access
singularity exec /cvmfs/unpacked.cern.ch/registry.hub.docker.com/cmssw/el9:amd64 cat /etc/os-release

# Test ROOT availability 
root-config --version
```

## Future Plans

1. **Automation**
- Automated testing suite
- Config file validation
- Branch existence checking

2. **Extensions**
- Support for more complex event selection
- Dynamic tree structure handling
- Improved progress monitoring

3. **Integration**
- Better CMSSW integration
- Support for different forest formats
- Handling of calibration data

## Log File Organization

The skimming system now creates a unique log directory for each condor submission to prevent interference between concurrent batch jobs.

### Features

1. **Unique Log Directories**
   - Each submission gets its own timestamp-based log directory
   - Format: `logs_<config-name>_<timestamp>` (e.g., `logs_2023_PbPb_QCDPhoton_20250522_134527`)
   - All output, error, and log files for a submission are contained in this directory

2. **Symbolic Link for Easy Access**
   - A symbolic link named `logs_latest` always points to the most recently created log directory
   - Use `tail -f logs_latest/skim_*.out` to monitor the latest submission

3. **Unique Condor Files**
   - Each condor submission file is named uniquely: `submit_<config-name>_<timestamp>.condor`
   - This preserves a record of all submissions and their parameters

### Running Multiple Samples Concurrently

The new log directory structure allows running multiple submissions with different configurations simultaneously:

```bash
# First submission (MC sample)
./submit_condor_jobs.sh -c ../configs/2023_PbPb_QCDPhoton.config -p

# Second submission (Data sample) - can run concurrently
./submit_condor_jobs.sh -c ../configs/2023_PbPb_Data_HirawPrime0_part.config
# Note: By default, the script preserves existing compilation artifacts

# Third submission (Different MC sample) - can run concurrently
./submit_condor_jobs.sh -c ../configs/some_other_sample.config
```

Each submission will use its own log directory, preventing log file conflicts and allowing easier debugging.

## Compilation Management

The skimming system handles ROOT compilation artifacts in a way that optimizes for both safety and performance.

### Compilation Behavior

1. **Default Behavior**
   - By default, the system preserves existing compilation artifacts (`libSkimHiForest.so` and related files)
   - This allows successive submissions to reuse the same compilation, saving time

2. **Precompilation**
   - When using the `-p` or `--precompile` flag, old compilation artifacts are automatically removed first
   - This ensures a clean compilation environment
   - Example: `./submit_condor_jobs.sh -p` will clean up old artifacts and compile freshly

3. **Manual Control**
   - `--cleanup`: Explicitly request removal of old compilation artifacts
   - By default, the system preserves existing compilation artifacts without needing an explicit flag
   - Example: `./submit_condor_jobs.sh --cleanup` will remove old artifacts without precompiling

### Typical Usage Scenarios

1. **First submission:**
   ```bash
   ./submit_condor_jobs.sh -p -c ../configs/2023_PbPb_QCDPhoton.config
   ```
   This will clean up any old artifacts, precompile, and submit jobs.

2. **Follow-up submission with the same code:**
   ```bash
   ./submit_condor_jobs.sh -c ../configs/2023_PbPb_Data_HirawPrime0_part.config
   ```
   This will reuse the previous compilation for faster submission.

3. **Submission after code changes:**
   ```bash
   ./submit_condor_jobs.sh -p -c ../configs/2023_PbPb_QCDPhoton.config
   ```
   This will clean up old artifacts and recompile when the code has changed.

4. **Submission specifying an OS version:**
   ```bash
   ./submit_condor_jobs.sh -c ../configs/2023_PbPb_QCDPhoton.config --os-version el8
   ```
   This will submit jobs that specifically request AlmaLinux 8 worker nodes.

5. **Submission specifying a different OS version:**
   ```bash
   ./submit_condor_jobs.sh -c ../configs/2023_PbPb_QCDPhoton.config --os-version el9
   ```
   This will submit jobs that specifically request AlmaLinux 9 worker nodes.

## Troubleshooting Large Output Files

### ROOT TFile Size Limitations

ROOT has a 100GB file size limitation that may cause failures when processing large datasets. The primary error message indicates this issue:

```
Fatal in <TFileMerger::RecursiveRemove>: Output file of the TFile Merger (targeting root://eoscms.cern.ch//eos/cms/store/group/phys_heavyions/bharikri/Run3GammaJet/2023_PbPb/MC/2025_05_22_QCDPhoton30_skimmed.root) has been deleted (likely due to a TTree larger than 100Gb)
```

### Recommended Solutions

1. **Ensure BatchMode is Properly Set**
   - Most critically, verify that `BatchMode 1` is set in the config file
   - The updated submit_condor_jobs.sh script now ensures this parameter is correctly set in the copied config file
   - Each job must process only its assigned batch of files, not the entire input dataset

2. **Adjust FilesPerOutput**
   - Decrease the `FilesPerOutput` parameter in your config:
   ```
   FilesPerOutput 5  # Try lower values like 3 or 2 if still hitting limits
   ```

3. **Write to Local Disk First**
   - For very large outputs, consider writing to local disk on the worker node first:
   ```
   OutputDir ./local_output
   ```
   - Then use condor's `transfer_output_files` mechanism to copy files back:
   ```
   transfer_output_files = local_output/*.root
   ```

4. **Check EOS Permissions**
   - Ensure you have proper write permissions to the EOS output directory
   - Check your grid tokens with `voms-proxy-info`
   - If needed, refresh your token: `voms-proxy-init --voms cms`

### Diagnostic Tips

1. **Check Verbose Output**
   - Run with increased verbosity (`Verbose 2` in config)
   - Look for messages about tree sizes and file operations

2. **Examine Condor Logs**
   - Check .err files for detailed error messages
   - Look for resource usage information with `condor_q -l <job_id> | grep _usage`

3. **ROOT File Validation**
   - Validate successful output files with `root -l outfile.root`
   - Look for branch structures with `TBrowser`
   - Check file sizes with `ls -lh`

## Verbosity Levels

The skimming system now supports multiple verbosity levels that can be set in the config file or via the submit_condor_jobs.sh script:

### Available Levels

- **0 (Quiet)**: Only error messages
- **1 (Normal)**: Standard information (default)
- **2 (Debug)**: Detailed debugging information
- **3 (Trace)**: Extremely verbose output for deep debugging

### Setting Verbosity

1. **In Config File**:
   ```
   Verbose 2  # Set debug level
   ```

2. **Via submit_condor_jobs.sh**:
   ```bash
   # Debug mode
   ./submit_condor_jobs.sh -v -c configs/your_config.config
   
   # Trace mode (very verbose)
   ./submit_condor_jobs.sh --trace -c configs/your_config.config
   
   # Quiet mode
   ./submit_condor_jobs.sh -q -c configs/your_config.config
   
   # Custom level
   ./submit_condor_jobs.sh --verbosity 2 -c configs/your_config.config
   ```

The script will automatically update the verbosity in the copied config file and pass it to all jobs. This is particularly helpful for diagnosing issues across all condor batch jobs.

## Job Submission Process

The job submission process has been significantly improved to ensure proper environment setup and parameter inheritance:

### Key Improvements

1. **Config File Handling**:
   - The original config file is preserved unchanged
   - A job-specific copy is created in the batch directory
   - Essential parameters (BatchMode, Verbose) are ensured in the copy
   - The copied config is used by the job

2. **Parameter Validation**:
   - Wrapper script double-checks BatchMode is enabled
   - Config file settings are printed for verification
   - Job logs show which parameters were actually used

3. **Diagnostic Output**:
   - The wrapper script prints detailed system information
   - Config file contents are displayed in job logs
   - Permissions and environment are verified

## Configuration Examples

Below are examples of optimal configuration settings for various use cases:

### 1. Standard Production Config

```
# Processing Control
Verbose 1
BatchMode 1
FileLimit 99999
FilesPerOutput 5

# I/O Configuration
InputDir /eos/cms/store/group/phys_heavyions/yourpath/inputdata/
OutputDir /eos/cms/store/group/phys_heavyions/yourpath/skimmed/
OutName 2025_05_23_dataset_skimmed

# Trees and Collections (abbreviated)
Trees hiEvtAnalyzer/HiTree:hiEvt ggHiNtuplizer/EventTree:ggHi skimanalysis/HltTree:skim
```

### 2. Development/Testing Config

```
# Processing Control with limited scope
Verbose 2
BatchMode 1
FileLimit 20         # Limit files for quick testing
FilesPerOutput 2     # Small batches for faster feedback

# I/O Configuration
InputDir /eos/cms/store/group/phys_heavyions/yourpath/testdata/
OutputDir ./test_output
OutName test_skim

# Trees and Collections (subset for testing)
Trees hiEvtAnalyzer/HiTree:hiEvt ggHiNtuplizer/EventTree:ggHi
```

### 3. Large Dataset Processing

```
# Processing Control optimized for large datasets
Verbose 1
BatchMode 1
FilesPerOutput 3     # Smaller batches to avoid 100GB limit

# I/O Configuration
InputDir /eos/cms/store/group/phys_heavyions/yourpath/bigdataset/
OutputDir /eos/cms/store/group/phys_heavyions/yourpath/skimmed/
OutName 2025_05_23_large_dataset_skimmed

# Resource tuning in submit_condor_jobs.sh:
# request_memory = 12GB
# request_disk = 8GB
# +JobFlavour = "tomorrow"
```

## Migration From ROOT Macros

For users migrating from the old ROOT macro-based system to the new standalone executable approach:

### Key Differences

1. **Execution Method**:
   - Old: `root -l -b -q 'SkimHiForest.C("config.config")'`
   - New: `./SkimHiForest config.config`

2. **BatchMode Parameter**:
   - Required now for proper batch processing
   - Add `BatchMode 1` to your config files

3. **Condor Integration**:
   - Old: Custom scripts with varying approaches
   - New: Standard submit_condor_jobs.sh with consistent behavior
   
4. **File Handling**:
   - Old: JSON-like file lists
   - New: Direct directory scanning based on InputDir

### Migration Steps

1. Update your config files:
   - Add `BatchMode 1` if missing
   - Update paths to reflect current storage locations

2. Test locally before submitting:
   ```bash
   ./SkimHiForest configs/your_config.config 0
   ```

3. Submit using the new script:
   ```bash
   ./submit_condor_jobs.sh -p -v -c configs/your_config.config
   ```

4. Check output organization:
   - Files now follow a consistent naming pattern
   - Logs are in structured directories with timestamps

## Summary of Latest Improvements

The skimming system has undergone significant improvements in May 2025:

### Core Improvements

1. **Standalone Execution**:
   - Fully compiled C++ executable
   - No dependency on ROOT interpreter
   - Improved reliability and performance

2. **Batch Processing**:
   - Proper BatchMode enforcement
   - Fixed 100GB file size limitation
   - Improved resource utilization

3. **Configuration Handling**:
   - Job-specific configs created automatically
   - Parameter validation and inheritance
   - Diagnostic output for troubleshooting

4. **Logging and Diagnostics**:
   - Multiple verbosity levels
   - Structured log directories
   - Detailed diagnostic output

### Future Roadmap

1. **Q2 2025**:
   - Add regex support for branch selection
   - Implement proper tree existence validation
   - Enhance documentation with additional examples

2. **Q3 2025**:
   - Explore distributed processing approaches
   - Implement automated testing framework
   - Add support for additional analysis-specific filters
