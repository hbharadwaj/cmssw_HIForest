#!/bin/bash
# run_batch.sh - Executes a single batch of the HiForest skimming

if [ $# -lt 3 ] || [ $# -gt 4 ]; then
    echo "Usage: $0 CONFIG_FILE BATCH_ID WORKING_DIR [USE_CMSSW]"
    echo "  USE_CMSSW: Set to 0 to use current ROOT setup instead of CMSSW environment (default: 1)"
    exit 1
fi

CONFIG_FILE=$1
BATCH_ID=$2
WORKING_DIR=$3
USE_CMSSW=${4:-1}  # Default to using CMSSW environment if not specified

# Print environment info
echo "Running on host: $(hostname)"
echo "Date: $(date)"
echo "Working directory: $WORKING_DIR"
echo "Config file: $CONFIG_FILE"
echo "Batch ID: $BATCH_ID"
echo "USE_CMSSW: $USE_CMSSW"

# Detect OS version
OS_VERSION=""
if [ -f /etc/os-release ]; then
    source /etc/os-release
    OS_VERSION=$ID$VERSION_ID
    echo "Detected OS: $OS_VERSION"
else
    echo "Cannot detect OS version, will try to continue anyway"
fi

# Set SCRAM_ARCH based on OS version
if [[ "$OS_VERSION" == *"9"* || "$OS_VERSION" == *"el9"* || "$OS_VERSION" == *"centos9"* || "$OS_VERSION" == *"rocky9"* ]]; then
    export SCRAM_ARCH=el9_amd64_gcc12
else
    export SCRAM_ARCH=el8_amd64_gcc11
fi
echo "Using SCRAM_ARCH: $SCRAM_ARCH"

# Check if we need to use singularity for EL8 hosts
if [[ "$OS_VERSION" == *"8"* || "$OS_VERSION" == *"el8"* || "$OS_VERSION" == *"centos8"* || "$OS_VERSION" == *"rocky8"* ]]; then
    echo "Detected EL8 host but code was compiled for EL9. Will try to use singularity."
    
    # Check if singularity/apptainer is available
    if command -v singularity &> /dev/null; then
        CONTAINER_CMD="singularity"
    elif command -v apptainer &> /dev/null; then
        CONTAINER_CMD="apptainer"
    else
        echo "WARNING: Neither singularity nor apptainer found. Will continue without container, which might cause issues."
        CONTAINER_CMD=""
    fi
    
    if [ -n "$CONTAINER_CMD" ]; then
        echo "Will use $CONTAINER_CMD to run in EL9 container"
        CONTAINER_IMAGE="/cvmfs/unpacked.cern.ch/registry.hub.docker.com/cmssw/el9:amd64"
        
        # Check if container image exists
        if [ ! -d "$CONTAINER_IMAGE" ]; then
            echo "ERROR: Container image not found at $CONTAINER_IMAGE"
            echo "Will try to continue without container, which might cause issues."
        else
            # Create a script to run inside the container
            TEMP_SCRIPT=$(mktemp /tmp/singularity_script_XXXXX.sh)
            cat > $TEMP_SCRIPT << EOSCRIPT
#!/bin/bash
export SCRAM_ARCH=$SCRAM_ARCH
cd $WORKING_DIR
source /cvmfs/cms.cern.ch/cmsset_default.sh

# Find CMSSW path
CURRENT_DIR="$WORKING_DIR"
CMSSW_PATH=""
while [[ "\$CURRENT_DIR" != "/" ]]; do
    if [[ -d "\$CURRENT_DIR/src" && -d "\$CURRENT_DIR/lib" && -f "\$CURRENT_DIR/config/scram_version" ]]; then
        CMSSW_PATH="\$CURRENT_DIR"
        break
    fi
    CURRENT_DIR=\$(dirname "\$CURRENT_DIR")
done

if [[ -n "\$CMSSW_PATH" ]]; then
    echo "Found CMSSW installation at: \$CMSSW_PATH"
    cd "\$CMSSW_PATH"
    eval \`scramv1 runtime -sh\`
    cd "$WORKING_DIR"
    
    # Pre-compile SkimHiForest.C
    echo "Pre-compiling SkimHiForest.C in container..."
    root -l -b -q -e "gSystem->CompileMacro(\\\"SkimHiForest.C\\\", \\\"kf\\\", \\\"libSkimHiForest\\\", \\\"\\\");"
    
    # Create and run the ROOT macro
    TEMP_MACRO=\$(mktemp /tmp/run_batch_XXXXX.C)
    cat > \$TEMP_MACRO << 'EOF'
void run_batch(const char* configFile, int batchID) {
    // Enable batch mode in config
    TEnv env;
    if (env.ReadFile(configFile, kEnvGlobal) < 0) {
        std::cerr << "ERROR: Cannot read config file: " << configFile << std::endl;
        return;
    }
    env.SetValue("BatchMode", "1");
    
    std::string tempConfigFile = "/tmp/temp_config_" + std::to_string(batchID) + ".config";
    env.WriteFile(tempConfigFile.c_str());
    
    // Load the library or source file
    bool loaded = false;
    if (gSystem->AccessPathName("libSkimHiForest.so") == false) {
        std::cout << "Loading pre-compiled library..." << std::endl;
        gSystem->AddDynamicPath(".:$LD_LIBRARY_PATH");
        int loadResult = gSystem->Load("libSkimHiForest.so");
        if (loadResult >= 0) loaded = true;
    }
    
    if (!loaded) {
        std::cout << "Trying direct compilation..." << std::endl;
        gROOT->ProcessLine(".L SkimHiForest.C+");
    }
    
    // Execute the function
    std::cout << "Running SkimHiForest(" << tempConfigFile << ", " << batchID << ")" << std::endl;
    gROOT->ProcessLine(Form("SkimHiForest(\\\"%s\\\", %d)", tempConfigFile.c_str(), batchID));
    
    // Clean up
    gSystem->Exec(("rm " + tempConfigFile).c_str());
}
EOF
    
    WRAPPER_MACRO=\$(mktemp /tmp/wrapper_XXXXX.C)
    cat > \$WRAPPER_MACRO << EOF
{
    gSystem->Load("libCore.so");
    gSystem->Load("libTree.so");
    gSystem->Load("libRIO.so");
    gSystem->Load("libFoam.so");
    
    gROOT->ProcessLine(".L \$TEMP_MACRO");
    run_batch("$CONFIG_FILE", $BATCH_ID);
}
EOF
    
    echo "Starting batch processing in container..."
    root -l -b -q \$WRAPPER_MACRO
    RESULT=\$?
    
    rm \$TEMP_MACRO \$WRAPPER_MACRO
    echo "Batch processing completed with exit code: \$RESULT"
    exit \$RESULT
else
    echo "ERROR: Could not find CMSSW installation directory"
    exit 1
fi
EOSCRIPT
            
            chmod +x $TEMP_SCRIPT
            echo "Executing inside $CONTAINER_CMD container..."
            $CONTAINER_CMD exec -B /cvmfs -B /afs $CONTAINER_IMAGE $TEMP_SCRIPT
            RESULT=$?
            rm $TEMP_SCRIPT
            exit $RESULT
        fi
    fi
fi

# Setup environment for ROOT
if [ "$USE_CMSSW" -eq 1 ] && [ -f /cvmfs/cms.cern.ch/cmsset_default.sh ]; then
    echo "Setting up CMSSW environment..."
    source /cvmfs/cms.cern.ch/cmsset_default.sh
    
    # Find CMSSW release directory (navigate up from $WORKING_DIR until we find src/)
    CURRENT_DIR="$WORKING_DIR"
    CMSSW_PATH=""
    
    while [[ "$CURRENT_DIR" != "/" ]]; do
        if [[ -d "$CURRENT_DIR/src" && -d "$CURRENT_DIR/lib" && -f "$CURRENT_DIR/config/scram_version" ]]; then
            CMSSW_PATH="$CURRENT_DIR"
            break
        fi
        CURRENT_DIR=$(dirname "$CURRENT_DIR")
    done
    
    if [[ -n "$CMSSW_PATH" ]]; then
        echo "Found CMSSW installation at: $CMSSW_PATH"
        cd "$CMSSW_PATH"
        eval `scramv1 runtime -sh`
        cd "$WORKING_DIR"
        echo "CMSSW environment successfully initialized"
        # Print ROOT version to verify
        root-config --version
    else
        echo "WARNING: Could not find CMSSW installation directory"
    fi
elif [ "$USE_CMSSW" -eq 0 ]; then
    echo "Using current ROOT setup as requested (USE_CMSSW=0)"
    # Check if ROOT is available in current environment
    if ! command -v root &> /dev/null || ! command -v root-config &> /dev/null; then
        echo "WARNING: ROOT does not appear to be in the current environment."
        # Try to use a known ROOT installation if available
        if [ -f /cvmfs/sft.cern.ch/lcg/app/releases/ROOT/6.34.08/x86_64-centos8-gcc11-opt/bin/thisroot.sh ]; then
            source /cvmfs/sft.cern.ch/lcg/app/releases/ROOT/6.34.08/x86_64-centos8-gcc11-opt/bin/thisroot.sh
            echo "Using ROOT from LCG release"
        elif [ -f /cvmfs/sft.cern.ch/lcg/app/releases/ROOT/latest/x86_64-centos9-gcc12-opt/bin/thisroot.sh ]; then
            source /cvmfs/sft.cern.ch/lcg/app/releases/ROOT/latest/x86_64-centos9-gcc12-opt/bin/thisroot.sh
            echo "Using latest ROOT from LCG release (EL9 compatible)"
        else
            echo "WARNING: Could not find a ROOT installation to use."
        fi
    else
        echo "Found ROOT in current environment: $(which root)"
        root-config --version
    fi
else
    echo "WARNING: Could not find CMSSET script. Trying direct ROOT setup..."
    # Fallback to direct ROOT setup
    if [ -f /cvmfs/sft.cern.ch/lcg/app/releases/ROOT/6.34.08/x86_64-centos8-gcc11-opt/bin/thisroot.sh ]; then
        source /cvmfs/sft.cern.ch/lcg/app/releases/ROOT/6.34.08/x86_64-centos8-gcc11-opt/bin/thisroot.sh
        echo "Using ROOT from LCG release"
    elif [ -n "$CMSSW_BASE" ] && [ -f ${CMSSW_BASE}/external/${SCRAM_ARCH}/bin/thisroot.sh ]; then
        source ${CMSSW_BASE}/external/${SCRAM_ARCH}/bin/thisroot.sh
        echo "Using ROOT from CMSSW"
    else
        echo "WARNING: Could not find ROOT environment. Job will likely fail."
    fi
fi

# Change to the working directory
cd $WORKING_DIR

# Additional ROOT environment setup
echo "Setting up ROOT environment for library loading..."
export ROOT_INCLUDE_PATH=.:$ROOT_INCLUDE_PATH
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
export ROOT_HIST=0  # Disable graphics

# Check what library files we have in the current directory
echo "Checking for precompiled libraries..."
echo "Directory contents:"
ls -la

# Avoid recompiling if we already have the libraries transferred from the submit host
if [ -f "libSkimHiForest.so" ] || [ -f "SkimHiForest_C.so" ]; then
    echo "Found precompiled libraries transferred from submit host, skipping compilation"
    
    # Check for library loading artifacts
    if [ -f "libSkimHiForest_ACLiC_dict_rdict.pcm" ] || [ -f "SkimHiForest_C_ACLiC_dict_rdict.pcm" ]; then
        echo "Found dictionary files (.pcm) - good"
        
        # Try to preload libraries to ensure they're recognized
        if [ -f "libSkimHiForest.so" ]; then
            echo "Preloading libSkimHiForest.so for ROOT dictionary recognition"
            root -l -b -q -e "gSystem->Load(\"libSkimHiForest.so\"); return 0;" > /dev/null 2>&1
        fi
        
        if [ -f "SkimHiForest_C.so" ]; then
            echo "Preloading SkimHiForest_C.so for ROOT dictionary recognition"
            root -l -b -q -e "gSystem->Load(\"SkimHiForest_C.so\"); return 0;" > /dev/null 2>&1
        fi
    else
        echo "WARNING: No dictionary files (.pcm) found, library loading might fail"
    fi
    
    # Fix permissions just in case
    chmod 644 *.so *.pcm 2>/dev/null || true
else
    # Try compiling if we don't have precompiled libraries
    echo "No precompiled libraries found, attempting compilation in batch node..."
    
    # Use a temporary directory for compilation to avoid space issues
    TMP_COMPILE_DIR=$(mktemp -d /tmp/skim_compile_XXXXX)
    echo "Using temporary directory for compilation: $TMP_COMPILE_DIR"
    cp SkimHiForest.C $TMP_COMPILE_DIR/
    pushd $TMP_COMPILE_DIR > /dev/null
    
    # Try both compilation methods for better compatibility
    echo "Trying CompileMacro method..."
    COMPILE_OUTPUT1=$(root -l -b -q -e "int result = gSystem->CompileMacro(\"SkimHiForest.C\", \"kf\", \"libSkimHiForest\", \"\"); printf(\"COMPILE_RESULT=%d\\n\", result);" 2>&1)
    COMPILE_RESULT1=$(echo "$COMPILE_OUTPUT1" | grep "COMPILE_RESULT" | cut -d= -f2)
    
    echo "Trying .L+ method..."
    COMPILE_OUTPUT2=$(root -l -b -q -e "gROOT->ProcessLine(\".L SkimHiForest.C+\"); printf(\"COMPILE2_RESULT=1\\n\");" 2>&1)
    COMPILE_RESULT2=$(echo "$COMPILE_OUTPUT2" | grep "COMPILE2_RESULT" | cut -d= -f2)
    
    if [[ "$COMPILE_RESULT1" == "1" || "$COMPILE_RESULT2" == "1" ]]; then
        echo "Compilation successful"
        cp -v *.so *_rdict.pcm ../ 2>/dev/null || true
    else
        echo "WARNING: Compilation in batch node failed."
        echo "Will try to use direct loading methods during execution."
    fi
    
    popd > /dev/null
    rm -rf $TMP_COMPILE_DIR
fi

# Create debug info about available libraries
echo "Libraries available before execution:" > library_debug.txt
ls -la *.so *_rdict.pcm 2>/dev/null >> library_debug.txt || echo "No libraries found" >> library_debug.txt
echo "LD_LIBRARY_PATH=$LD_LIBRARY_PATH" >> library_debug.txt

# Create a temporary macro to run the batch
TEMP_MACRO=$(mktemp /tmp/run_batch_XXXXX.C)
cat > $TEMP_MACRO << 'EOF'
#include <iostream>
#include <fstream>
#include <string>
#include <TSystem.h>
#include <TEnv.h>
#include <TROOT.h>
#include <TString.h>

void run_batch(const char* configFile, int batchID) {
    // Check if ROOT is properly configured
    if (!gSystem || !gROOT) {
        std::cerr << "ERROR: ROOT environment not properly initialized!" << std::endl;
        return;
    }
    
    std::cout << "Run batch started with config: " << configFile << ", batch ID: " << batchID << std::endl;
    
    // Update configuration to enable batch mode
    TEnv env;
    if (env.ReadFile(configFile, kEnvGlobal) < 0) {
        std::cerr << "ERROR: Cannot read config file: " << configFile << std::endl;
        return;
    }
    env.SetValue("BatchMode", "1");
    
    // Get the current file name
    std::string tempConfigFile = "/tmp/temp_config_" + std::to_string(batchID) + ".config";
    env.WriteFile(tempConfigFile.c_str());
    
    // Debug: Check what files we have and what our environment looks like
    std::cout << "Creating debug info file..." << std::endl;
    gSystem->Exec("ls -la . > loadinfo.txt");
    gSystem->Exec("echo 'LD_LIBRARY_PATH=' $LD_LIBRARY_PATH >> loadinfo.txt");
    gSystem->Exec("echo 'ROOT_INCLUDE_PATH=' $ROOT_INCLUDE_PATH >> loadinfo.txt");
    gSystem->Exec("echo 'PWD=' $(pwd) >> loadinfo.txt");
    
    // Try different methods to load SkimHiForest
    bool loaded = false;
    
    // Make sure the current directory is in the library path
    gSystem->AddDynamicPath(".:$LD_LIBRARY_PATH");
    gSystem->AddIncludePath("-I.:$ROOT_INCLUDE_PATH");
    
    std::cout << "Trying multiple methods to load SkimHiForest..." << std::endl;
    
    // Method 1: Try loading pre-compiled libSkimHiForest.so if it exists
    if (gSystem->AccessPathName("libSkimHiForest.so") == false) {
        std::cout << "Method 1: Found libSkimHiForest.so in current directory, trying to load it" << std::endl;
        
        int loadResult = gSystem->Load("libSkimHiForest.so");
        if (loadResult >= 0) {
            std::cout << "Successfully loaded pre-compiled libSkimHiForest.so" << std::endl;
            loaded = true;
        } else {
            std::cout << "Failed to load libSkimHiForest.so (error " << loadResult << ")" << std::endl;
        }
    } else {
        std::cout << "No libSkimHiForest.so found in current directory" << std::endl;
    }
    
    // Method 2: Try with ACLiC-generated library (SkimHiForest_C.so) if Method 1 failed
    if (!loaded && gSystem->AccessPathName("SkimHiForest_C.so") == false) {
        std::cout << "Method 2: Found SkimHiForest_C.so, trying to load it" << std::endl;
        
        int loadResult = gSystem->Load("SkimHiForest_C.so");
        if (loadResult >= 0) {
            std::cout << "Successfully loaded SkimHiForest_C.so" << std::endl;
            loaded = true;
        } else {
            std::cout << "Failed to load SkimHiForest_C.so (error " << loadResult << ")" << std::endl;
        }
    } else if (!loaded) {
        std::cout << "No SkimHiForest_C.so found in current directory" << std::endl;
    }
    
    // Method 3: Try with + compilation option if previous methods failed
    if (!loaded) {
        std::cout << "Method 3: Trying to compile and load SkimHiForest.C with .L+" << std::endl;
        // Make sure we can access dictionary files by adding current dir to ROOT_INCLUDE_PATH
        gSystem->Exec("echo 'Trying to compile with .L+' >> loadinfo.txt");
        
        int result = gROOT->ProcessLine(".L SkimHiForest.C+");
        if (result >= 0) {
            std::cout << "Successfully loaded SkimHiForest.C with compilation" << std::endl;
            loaded = true;
        } else {
            std::cout << "Failed to compile SkimHiForest.C with .L+ (error " << result << ")" << std::endl;
            
            // Method 4: Try without compilation
            std::cout << "Method 4: Trying to load SkimHiForest.C without compilation" << std::endl;
            result = gROOT->ProcessLine(".L SkimHiForest.C");
            if (result >= 0) {
                std::cout << "Successfully loaded SkimHiForest.C without compilation" << std::endl;
                loaded = true;
            } else {
                std::cout << "Failed to load SkimHiForest.C (error " << result << ")" << std::endl;
            }
        }
    }
    
    // Last resort method (5): Try directly including the code
    if (!loaded) {
        std::cout << "Method 5: All standard methods failed. Trying direct include..." << std::endl;
        
        // Write a small wrapper that includes the header directly
        std::string includeWrapper = "/tmp/include_wrapper_" + std::to_string(batchID) + ".C";
        std::ofstream wrapperFile(includeWrapper);
        wrapperFile << "#include \"SkimHiForest.C\"\n";
        wrapperFile.close();
        
        int directResult = gROOT->ProcessLine((".L " + includeWrapper).c_str());
        if (directResult >= 0) {
            std::cout << "Successfully included SkimHiForest.C directly" << std::endl;
            loaded = true;
        } else {
            std::cout << "Failed to include SkimHiForest.C directly (error " << directResult << ")" << std::endl;
        }
        
        // Clean up
        gSystem->Unlink(includeWrapper.c_str());
    }
    
    if (!loaded) {
        std::cerr << "ERROR: Failed to load SkimHiForest.C with all methods!" << std::endl;
        std::cerr << "Dumping environment information for debugging:" << std::endl;
        gSystem->Exec("cat loadinfo.txt");
        gSystem->Exec("ls -la >> loadinfo.txt");
        gSystem->Exec("cat loadinfo.txt");
        return;
    }
    
    // Execute SkimHiForest function
    std::cout << "Calling SkimHiForest(" << tempConfigFile << ", " << batchID << ")" << std::endl;
    gROOT->ProcessLine(Form("SkimHiForest(\"%s\", %d)", tempConfigFile.c_str(), batchID));
    
    // Clean up
    gSystem->Exec(("rm " + tempConfigFile).c_str());
}
EOF

# Create a wrapper macro that loads and calls our function
WRAPPER_MACRO=$(mktemp /tmp/wrapper_XXXXX.C)
cat > $WRAPPER_MACRO << EOF
{
    // Load system libraries that might be needed
    gSystem->Load("libCore.so");
    gSystem->Load("libTree.so");
    gSystem->Load("libRIO.so");
    gSystem->Load("libFoam.so");
    
    // Increase verbosity for debugging
    gErrorIgnoreLevel = kInfo;
    
    // Add the current directory to include path
    gROOT->ProcessLine(".include .");
    
    // Load our macro with verbose error reporting
    int loadResult = gROOT->LoadMacro("${TEMP_MACRO}");
    if (loadResult < 0) {
        printf("ERROR: Failed to load run_batch macro (error %d)\n", loadResult);
        gSystem->Exit(1);
    } else {
        printf("Successfully loaded run_batch macro\n");
    }
    
    // Call the function
    run_batch("${CONFIG_FILE}", ${BATCH_ID});
}
EOF

# Run the batch processing
echo "Starting batch processing..."
root -l -b -q ${WRAPPER_MACRO}
RESULT=$?

# Clean up
rm $TEMP_MACRO
rm $WRAPPER_MACRO

echo "Batch processing completed with exit code: $RESULT"
exit $RESULT
