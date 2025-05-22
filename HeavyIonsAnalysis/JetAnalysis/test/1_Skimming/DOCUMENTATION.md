# HiForest Skimming Documentation

## Overview

The skimming system processes HiForest files using ROOT's RDataFrame to create smaller, analysis-ready files. 

### Key Components

1. **Config File Structure**
- Tree definitions with aliases (e.g., hiEvtAnalyzer/HiTree:hiEvt)
- Branch selections per tree
- I/O paths and processing parameters

2. **Tree Organization**
- Base tree: hiEvtAnalyzer/HiTree (critical for proper branch handling)
- Friend trees with aliases (ggHi, skim, hlt, AK2Z1, etc.)
- Branch name formats vary by tree type

3. **RDataFrame Column Handling**
- Base tree: branches appear without prefix
- Friend trees: branches appear as alias.branch
- Column selection must account for both formats

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
- [ ] Evaluate RDF column filtering performance
- [x] Implement batch processing
- [ ] Profile memory usage with large files

2. **Error Handling**
- [ ] Add validation for config file format
- [ ] Improve missing column reporting
- [ ] Add checks for tree/branch existence

3. **Features**
- [ ] Support regex in branch selection
- [ ] Add event filtering options
- [ ] Implement progress reporting

4. **Documentation**
- [ ] Add examples for common use cases
- [ ] Document config file format thoroughly
- [ ] Create troubleshooting guide

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

## Cross-Architecture Compatibility (EL8/EL9)

The skimming system now supports job submissions in both EL8 and EL9 environments:

### Architecture Handling

- **EL9 Systems (Default)**: Jobs run natively without containers
- **EL8 Systems**: Jobs use a Singularity/Apptainer container with an EL9 environment

The system automatically detects the host OS version and makes the appropriate adjustments:

1. **OS Detection**: Reads `/etc/os-release` to identify the operating system version
2. **SCRAM_ARCH Selection**: 
   - EL9: Uses `el9_amd64_gcc12`
   - EL8: Uses `el8_amd64_gcc11` with Singularity container

### Container Usage

For EL8 hosts, the system uses Singularity/Apptainer to run in an EL9 container:

```bash
# Container image path
/cvmfs/unpacked.cern.ch/registry.hub.docker.com/cmssw/el9:amd64
```

The container setup automatically:
- Mounts /cvmfs and /afs paths
- Sets up the appropriate CMSSW environment 
- Handles ROOT library initialization

### CMSSW Environment Control

You can choose whether to use the CMSSW environment or the current ROOT setup:

```bash
# Use current ROOT setup instead of CMSSW
./submit_condor_jobs.sh --no-cmssw -c configs/your_config.config
```

This option is useful when:
- Working with standalone ROOT analyses
- Testing custom ROOT installations
- Avoiding version conflicts with CMSSW

## Batch Processing

The skimming system supports batch processing to efficiently handle large datasets. This feature:
- Divides input files into manageable batches
- Processes each batch individually to reduce memory usage
- Supports HTCondor job submission for parallel processing
- Includes tools to merge batch outputs into a single file

### Configuration

To enable batch processing, add the following parameters to your config file:

```
# Batch processing parameters
BatchMode 1            # Enable batch processing (0 = disabled, 1 = enabled)
FilesPerOutput 5       # Number of files per batch
```

> **Note**: The `DoCondor` parameter appears in some config files but is not actually used by the codebase. Condor submission is handled entirely by the `submit_condor_jobs.sh` script, regardless of this parameter's value.

### Usage Modes

#### 1. Local Batch Processing

Process all batches sequentially on the local machine:

```bash
root -l -b -q 'SkimHiForest.C("configs/your_config.config")'
```

Process a specific batch (e.g., batch #3):

```bash
root -l -b -q 'SkimHiForest.C("configs/your_config.config", 3)'
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
- `-p, --precompile`: Pre-compile SkimHiForest.C before submitting jobs (implies cleanup)
- `-r, --remove-jobs`: Remove existing condor jobs before submitting new ones
- `--use-cmssw`: Use CMSSW environment (default: use current ROOT setup)
- `--cleanup`: Clean up old compilation artifacts (default: preserve them)

#### 3. Merging Batch Outputs

After processing all batches, you can merge the outputs into a single file using:

```bash
./merge_batch_outputs.sh -c configs/your_config.config
```

Options:
- `-c, --config CONFIG_FILE`: Path to the configuration file
- `-d, --dir OUTPUT_DIR`: Directory containing batch outputs
- `-o, --output OUTPUT_NAME`: Name of the merged output file

### Output Files

Batch processing creates output files with the following naming convention:
```
<OutName>_batch<BatchID>_of_<TotalBatches>.root
```

For example:
```
2025_05_21_QCDPhoton30_skimmed_batch_batch0_of_10.root
2025_05_21_QCDPhoton30_skimmed_batch_batch1_of_10.root
...
```

When merged, the final output is named:
```
<OutName>_merged.root
```

### Tips for Efficient Processing

1. **Determine optimal batch size**:
   - Too small: Excessive overhead from job startup and file I/O
   - Too large: High memory usage and longer per-job processing time
   - Recommended: 5-20 files per batch, depending on file size

2. **Monitor job progress**:
   - For HTCondor: `condor_q -submitter <username>`
   - Check logs in the `logs` directory

3. **Handle failed jobs**:
   - Resubmit individual failed batches using the specific batch ID
   - Example: `root -l -b -q 'SkimHiForest.C("configs/your_config.config", 7)'`

4. **Resource allocation**:
   - Adjust memory and disk requests in `submit_condor_jobs.sh` based on your dataset size
   - Default: 4GB memory, 2GB disk space

### Common Batch Processing Issues

1. **Memory usage too high**:
   - Decrease the number of files per batch
   - Use the `FileLimit` parameter to process a subset during development

2. **Missing output files**:
   - Check the logs for errors
   - Verify that the output directory is writable

3. **Slow processing**:
   - Increase the number of concurrent jobs
   - Consider using a higher value for `FilesPerOutput` to reduce overhead

4. **Incomplete or corrupted outputs**:
   - Check for failed jobs in the logs
   - Verify that the output files have the expected size and content

5. **ROOT version mismatches**:
   - Use the `--precompile` option to ensure consistent library usage
   - Check for ROOT version compatibility between submission host and worker nodes

## Cross-Architecture Troubleshooting

### Common EL8/EL9 Issues

1. **Singularity/Apptainer Not Available**:
   - Error message: `Neither singularity nor apptainer found`
   - Solution: Load the singularity module before job submission:
     ```bash
     module load singularity
     ```

2. **Container Image Not Found**:
   - Error message: `Container image not found at /cvmfs/unpacked.cern.ch/registry.hub.docker.com/cmssw/el9:amd64`
   - Solution: Check CVMFS connectivity and mount points:
     ```bash
     ls -la /cvmfs/unpacked.cern.ch/registry.hub.docker.com/cmssw/
     ```

3. **Library Loading Issues**:
   - Error: `error while loading shared libraries: libXXX.so: cannot open shared object file`
   - Solution: Make sure the proper library paths are available in the container:
     ```bash
     singularity exec /cvmfs/unpacked.cern.ch/registry.hub.docker.com/cmssw/el9:amd64 env | grep LD_LIBRARY_PATH
     ```

4. **CMSSW Environment Errors**:
   - Issues related to SCRAM setup or missing environment
   - Solution: Try using the `--no-cmssw` option to use the current ROOT setup:
     ```bash
     ./submit_condor_jobs.sh --no-cmssw -c configs/your_config.config
     ```

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
