#include "../include/helpers.h"
#include "../include/TemplateGenerator.h"

// Helper function to generate custom file output
void writeCustomBase(const char* filename) {
    std::ofstream out(filename);
    out << "#ifndef GammaJetAnalysis_h\n"
        << "#define GammaJetAnalysis_h\n\n"
        << "#include <TROOT.h>\n"
        << "#include <TChain.h>\n"
        << "#include <TFile.h>\n"
        << "#include \"ROOT/RVec.hxx\"\n\n"
        << "class GammaJetAnalysis {\n"
        << "protected:\n"
        << "    TTree* fChain = nullptr;   //!pointer to the analyzed TTree or TChain\n"
        << "    Int_t fCurrent = -1;       //!current Tree number in a TChain\n\n"
        << "    // Common branches for all systems\n"
        << "    Int_t ggHi_nPho;\n"
        << "    UInt_t run;\n"
        << "    ULong64_t evt;\n"
        << "    UInt_t lumi;\n\n"
        << "public:\n"
        << "    GammaJetAnalysis(TTree* tree = nullptr) : fChain(nullptr) {}\n"
        << "    virtual ~GammaJetAnalysis() {\n"
        << "        if (fChain) delete fChain->GetCurrentFile();\n"
        << "    }\n\n"
        << "    // Virtual interface\n"
        << "    virtual void Init(TTree* tree) = 0;\n"
        << "    virtual Bool_t Notify() = 0;\n"
        << "    virtual void Loop() = 0;\n"
        << "    virtual Int_t GetEntry(Long64_t entry) = 0;\n"
        << "    virtual Long64_t LoadTree(Long64_t entry) = 0;\n"
        << "};\n\n"
        << "#endif\n";
    out.close();
}

void generateTemplates(const char* configPath = "../../configs/jetSubstructure.config") {
    // Load configuration
    Config cfg;
    if (!loadConfig(cfg, configPath)) {
        std::cerr << "Failed to load config from " << configPath << std::endl;
        return;
    }

    // First, generate base class
    writeCustomBase("../include/GammaJetAnalysis.h");

    // Generate templates using TemplateGenerator
    if (!TemplateGenerator::generateTemplates(cfg)) {
        std::cerr << "Template generation failed" << std::endl;
        return;
    }
    
    std::cout << "Templates generated successfully for:" << std::endl;
    std::cout << "  System: " << cfg.system << std::endl;
    std::cout << "  Type: " << cfg.dataType << std::endl;
    std::cout << "  Input: " << cfg.inputDir << std::endl;
}
