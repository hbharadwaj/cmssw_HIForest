#ifndef TEMPLATEGENERATOR_H
#define TEMPLATEGENERATOR_H

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <fstream>
#include <sstream>
#include "helpers.h"

class TemplateGenerator {
public:
    static bool generateTemplates(const Config& cfg);

private:
    static std::string getFirstFile(const std::string& dir);
    static std::string generateSystemClassName(const Config& cfg);
    static bool addInheritance(const std::string& headerFile, const std::string& className);
};

bool TemplateGenerator::generateTemplates(const Config& cfg) {
    try {
        // Get input file
        std::string inputFile = getFirstFile(cfg.inputDir);
        if (inputFile.empty()) {
            std::cerr << "No input files found in " << cfg.inputDir << std::endl;
            return false;
        }

        // Open file and get tree
        TFile *f = TFile::Open(inputFile.c_str());
        if (!f || f->IsZombie()) {
            std::cerr << "Failed to open " << inputFile << std::endl;
            return false;
        }

        // Get tree
        TTree *tree = (TTree*)f->Get("jet_tree");
        if (!tree) {
            std::cerr << "No jet_tree found in " << inputFile << std::endl;
            return false;
        }

        // Generate system class name
        std::string className = generateSystemClassName(cfg);
        
        // Use ROOT's MakeClass to generate the header
        if (tree->MakeClass(className.c_str()) != 0) {
            std::cerr << "Error: MakeClass failed" << std::endl;
            return false;
        }

        // Add inheritance to base class
        if (!addInheritance(className + ".h", className)) {
            std::cerr << "Error: Failed to add inheritance" << std::endl;
            return false;
        }

        // Move files to include directory
        if (gSystem->Exec(("mv " + className + ".h ../include/").c_str()) != 0 ||
            gSystem->Exec(("mv " + className + ".C ../include/").c_str()) != 0) {
            std::cerr << "Error: Failed to move files to include directory" << std::endl;
            return false;
        }

        f->Close();
        delete f;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception in generateTemplates: " << e.what() << std::endl;
        return false;
    }
}

std::string TemplateGenerator::getFirstFile(const std::string& dir) {
    auto files = GetFiles(dir, 1);
    return files.empty() ? "" : files[0];
}

std::string TemplateGenerator::generateSystemClassName(const Config& cfg) {
    return "GammaJet" + cfg.system + (cfg.dataType == "MC" ? "MC" : "Data");
}

bool TemplateGenerator::addInheritance(const std::string& headerFile, const std::string& className) {
    std::ifstream inFile(headerFile);
    if (!inFile.is_open()) {
        std::cerr << "Error: Could not open " << headerFile << " for reading" << std::endl;
        return false;
    }

    std::string content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    inFile.close();

    // Add include for base class
    size_t includePos = content.find("#include");
    if (includePos != std::string::npos) {
        content.insert(includePos, "#include \"GammaJetAnalysis.h\"\n");
    }

    // Add inheritance
    std::string classPattern = "class " + className + " {";
    size_t classPos = content.find(classPattern);
    if (classPos != std::string::npos) {
        content.replace(classPos, classPattern.length(), "class " + className + " : public GammaJetAnalysis {");
    } else {
        std::cerr << "Error: Could not find class declaration in " << headerFile << std::endl;
        return false;
    }

    // Write back to file
    std::ofstream outFile(headerFile);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open " << headerFile << " for writing" << std::endl;
        return false;
    }
    outFile << content;
    outFile.close();

    return true;
}

#endif
