#!/bin/bash
# merge_batch_outputs.sh
# This script merges the output files from batch processing into a single file

# Default parameters
OUTPUT_DIR=""
OUTPUT_NAME=""
CONFIG_FILE="../configs/2023_PbPb_QCDPhoton.config"

# Parse command-line arguments
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        -o|--output)
            OUTPUT_NAME="$2"
            shift
            shift
            ;;
        -d|--dir)
            OUTPUT_DIR="$2"
            shift
            shift
            ;;
        -c|--config)
            CONFIG_FILE="$2"
            shift
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [options]"
            echo "Options:"
            echo "  -o, --output OUTPUT_NAME   Name of the merged output file"
            echo "  -d, --dir OUTPUT_DIR       Directory containing batch outputs"
            echo "  -c, --config CONFIG_FILE   Configuration file path (default: ../configs/2023_PbPb_QCDPhoton.config)"
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

# Read output directory and name from config if not specified
if [ -z "$OUTPUT_DIR" ] || [ -z "$OUTPUT_NAME" ]; then
    echo "Reading from config file: $CONFIG_FILE"
    if [ ! -f "$CONFIG_FILE" ]; then
        echo "Error: Config file not found: $CONFIG_FILE"
        exit 1
    fi
    
    if [ -z "$OUTPUT_DIR" ]; then
        OUTPUT_DIR=$(grep "OutputDir" "$CONFIG_FILE" | tail -1 | sed 's/.*OutputDir\s*//' | tr -d ' ')
        echo "Using output directory from config: $OUTPUT_DIR"
    fi
    
    if [ -z "$OUTPUT_NAME" ]; then
        OUTPUT_NAME=$(grep "OutName" "$CONFIG_FILE" | tail -1 | sed 's/.*OutName\s*//' | tr -d ' ')
        echo "Using output name from config: $OUTPUT_NAME"
        # Add "_merged" to avoid overwriting batch files
        OUTPUT_NAME="${OUTPUT_NAME}_merged"
    fi
fi

if [ -z "$OUTPUT_DIR" ] || [ -z "$OUTPUT_NAME" ]; then
    echo "Error: Output directory and name must be specified either via command line or config file"
    exit 1
fi

echo "Merging batch outputs from: $OUTPUT_DIR"
echo "Merged output file will be: $OUTPUT_DIR/$OUTPUT_NAME.root"

# Create a temporary ROOT macro to merge the files
TEMP_MACRO=$(mktemp /tmp/merge_batches_XXXXX.C)
cat > $TEMP_MACRO << 'EOL'
void merge_batches(const char* outputDir, const char* outputName) {
    TSystemDirectory dir(outputDir, outputDir);
    TList* files = dir.GetListOfFiles();
    
    std::vector<std::string> batchFiles;
    TString outNamePattern = TString::Format("%s_batch", outputName);
    
    if (files) {
        TIter next(files);
        TObject* obj;
        
        while ((obj = next())) {
            TString filename = obj->GetName();
            if (!obj->IsFolder() && filename.EndsWith(".root") && filename.Contains(outNamePattern)) {
                TString fullPath = TString::Format("%s/%s", outputDir, filename.Data());
                batchFiles.push_back(fullPath.Data());
                std::cout << "Found batch file: " << fullPath << std::endl;
            }
        }
    }
    
    if (batchFiles.empty()) {
        std::cerr << "No batch files found matching pattern: " << outNamePattern << std::endl;
        return;
    }
    
    std::cout << "Found " << batchFiles.size() << " batch files to merge" << std::endl;
    
    // Create the target file list for hadd
    std::string mergedFile = std::string(outputDir) + "/" + std::string(outputName) + ".root";
    std::cout << "Merging into: " << mergedFile << std::endl;
    
    // Use TFileMerger for efficiency
    TFileMerger merger;
    merger.OutputFile(mergedFile.c_str());
    
    for (const auto& file : batchFiles) {
        merger.AddFile(file.c_str());
    }
    
    bool success = merger.Merge();
    if (success) {
        std::cout << "Successfully merged " << batchFiles.size() << " files into " << mergedFile << std::endl;
    } else {
        std::cerr << "Failed to merge files" << std::endl;
    }
}
EOL

# Run the ROOT macro to merge the files
echo "Starting merge process..."
root -l -b -q "$TEMP_MACRO(\"$OUTPUT_DIR\", \"$OUTPUT_NAME\")"
RESULT=$?

# Clean up
rm $TEMP_MACRO

if [ $RESULT -eq 0 ]; then
    echo "Merge completed successfully"
else
    echo "Merge failed with exit code: $RESULT"
fi

exit $RESULT
