#ifndef MNguyen_HiInclusiveJetAnalyzer_inclusiveJetAnalyzer_
#define MNguyen_HiInclusiveJetAnalyzer_inclusiveJetAnalyzer_

// system include files
#include <memory>
#include <string>
#include <iostream>

// ROOT headers
#include "TTree.h"

// user include files
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "fastjet/contrib/Njettiness.hh"
#include "DataFormats/JetMatching/interface/JetFlavourInfo.h"
#include "DataFormats/JetMatching/interface/JetFlavourInfoMatching.h"
//

/**\class HiInclusiveJetAnalyzer

   \author Matt Nguyen
   \date   November 2010
   Modified by Bharadwaj Harikrishnan, October 2024
*/

class HiInclusiveJetAnalyzer : public edm::one::EDAnalyzer<edm::one::WatchRuns> {
public:
  explicit HiInclusiveJetAnalyzer(const edm::ParameterSet&);

  ~HiInclusiveJetAnalyzer() override;

  void analyze(const edm::Event&, const edm::EventSetup&) override;

  void beginRun(const edm::Run& run, const edm::EventSetup& es) override;
  void endRun(const edm::Run& run, const edm::EventSetup& es) override;

  void beginJob() override;

  enum JetType {
    kAllGen = 0,    // All gen jets
    kMatchGen = 1,  // Only matched gen jets
    kReco = 2       // Reco jets
  };

private:
  // for reWTA reclustering-----------------------
  bool doWTARecluster_ = false;
  fastjet::JetDefinition WTAjtDef =
      fastjet::JetDefinition(fastjet::JetAlgorithm::antikt_algorithm, 2, fastjet::WTA_pt_scheme);
  //--------------------------------------------

  //int getPFJetMuon(const pat::Jet& pfJet, const reco::PFCandidateCollection *pfCandidateColl);
  int getPFJetMuon(const pat::Jet& pfJet, const edm::View<pat::PackedCandidate>* pfCandidateColl);

  //double getPtRel(const reco::PFCandidate& lep, const pat::Jet& jet );
  double getPtRel(const pat::PackedCandidate& lep, const pat::Jet& jet);

  void analyzeSubjets(const reco::Jet& jet);
  int getGroomedGenJetIndex(const reco::GenJet& jet) const;
  void analyzeRefSubjets(const reco::GenJet& jet);
  void analyzeGenSubjets(const reco::GenJet& jet);

  int TaggedJet(pat::Jet patjet, edm::Handle<reco::JetTagCollection> jetTags);

  template <typename T>
  void IterativeDeclustering(int flagGen, const T& jet, fastjet::PseudoJet* sub1, fastjet::PseudoJet* sub2);
  void countJetSplits(const fastjet::PseudoJet& node, bool primaryOnly, int& nSD, std::vector<int>& nKT, std::vector<float>* angles = nullptr);

  edm::InputTag jetTagLabel_;
  edm::EDGetTokenT<pat::JetCollection> jetTag_;
  edm::EDGetTokenT<reco::CaloJetCollection> caloJetTag_;
  edm::EDGetTokenT<pat::JetCollection> matchTag_;
  edm::EDGetTokenT<edm::View<pat::PackedCandidate>> pfCandidateLabel_;
  edm::EDGetTokenT<reco::GenParticleCollection> genParticleSrc_;
  edm::EDGetTokenT<edm::View<reco::GenJet>> genjetTag_;
  edm::EDGetTokenT<edm::HepMCProduct> eventInfoTag_;
  edm::EDGetTokenT<GenEventInfoProduct> eventGenInfoTag_;
  // b and c hadrons
  edm::EDGetTokenT<reco::JetFlavourInfoMatchingCollection> jetFlavourInfosToken_;

  std::string jetName_;  //used as prefix for jet structures
  edm::EDGetTokenT<edm::View<reco::Jet>> subjetGenTag_;
  edm::Handle<reco::JetView> gensubjets_;
  edm::EDGetTokenT<edm::ValueMap<float>> tokenGenTau1_;
  edm::EDGetTokenT<edm::ValueMap<float>> tokenGenTau2_;
  edm::EDGetTokenT<edm::ValueMap<float>> tokenGenTau3_;
  edm::EDGetTokenT<edm::ValueMap<float>> tokenGenSym_;
  edm::Handle<edm::ValueMap<float>> genSymVM_;
  edm::EDGetTokenT<edm::ValueMap<int>> tokenGenDroppedBranches_;
  edm::Handle<edm::ValueMap<int>> genDroppedBranchesVM_;

  bool doMatch_;
  bool useVtx_;
  bool useRawPt_;
  bool isMC_;
  bool useHepMC_;
  bool fillGenJets_;
  bool fillDetailBranches_;
  bool useQuality_;
  std::string trackQuality_;

  bool doBtagging_;
  bool doGenTaus_;
  bool doGenSym_;
  bool doSubJets_;
  bool doJetConstituents_;
  bool doGenSubJets_;
  bool doCaloJets_;

  bool doHiJetID_;
  bool doStandardJetID_;

  double rParam_;
  double hardPtMin_;
  double jetPtMin_;
  double jetAbsEtaMax_;

  bool doSubstructure_;
  double sdZcut_;
  double sdBeta_;
  double sdDynktcut_;
  double groomType_;
  double groomCombine_;
  bool doChargedConstOnly_;
  std::vector<double> ktThresholds_;

  bool doLegacyBtagging_;
  bool doCandidateBtagging_;
  bool useNewBtaggers_;

  bool doSubEvent_;
  double genPtMin_;
  bool doPFCandEnergySmear_;
  double pfChargedCandidateEnergyScale_;
  double pfNeutralCandidateEnergyScale_;
  double pfGammaCandidateEnergyScale_;

  TTree* t;
  edm::Service<TFileService> fs1;

  std::string bTagJetName_;
  std::string particleTransformerJetTags_;

  edm::EDGetTokenT<reco::JetTagCollection> particleTransformerJetTagsTkn_, particleTransformerJetTagsBBTkn_,
      particleTransformerJetTagsLepBTkn_;
  std::map<std::string, std::map<std::string, edm::EDGetTokenT<reco::JetTagCollection>>> jetTaggers_;

  // Grooming type enum for readability (config still uses double for backward compatibility)
  enum GroomType { kLateSD = 0, kStandardSD = 1 };
  enum GroomCombine { kNoRequirement = 0, kRequireSDandKT = 1 };

  // Use constexpr to avoid linker issues with static const in templates
  static constexpr int MAXNKT = 15;
  static constexpr int MAXJETS = 500;
  static constexpr int MAXTRACKS = 5000;
  static constexpr int MAXCALO = 100;

  struct JRA {
    int nref = 0;
    int run = 0;
    int evt = 0;
    int lumi = 0;
    int ncalo = 0;

    float rawpt[MAXJETS] = {0};
    float rawE[MAXJETS] = {0};
    float jtpt[MAXJETS] = {0};
    float jteta[MAXJETS] = {0};
    float jtphi[MAXJETS] = {0};

    //reWTA reclusted jet axis
    float WTAeta[MAXJETS] = {0};
    float WTAphi[MAXJETS] = {0};
    float WTAgeneta[MAXJETS] = {0};
    float WTAgenphi[MAXJETS] = {0};

    float jty[MAXJETS] = {0};
    float jtpu[MAXJETS] = {0};
    float jtm[MAXJETS] = {0};
    float jtarea[MAXJETS] = {0};

    float jtPfCHF[MAXJETS] = {0};
    float jtPfNHF[MAXJETS] = {0};
    float jtPfCEF[MAXJETS] = {0};
    float jtPfNEF[MAXJETS] = {0};
    float jtPfMUF[MAXJETS] = {0};

    int jtPfCHM[MAXJETS] = {0};
    int jtPfNHM[MAXJETS] = {0};
    int jtPfCEM[MAXJETS] = {0};
    int jtPfNEM[MAXJETS] = {0};
    int jtPfMUM[MAXJETS] = {0};

    float jttau1[MAXJETS] = {0};
    float jttau2[MAXJETS] = {0};
    float jttau3[MAXJETS] = {0};

    int jtdynsplit[MAXJETS] = {0};
    float jtdyneta[MAXJETS] = {0};
    float jtdynphi[MAXJETS] = {0};
    float jtdyndeltaR[MAXJETS] = {0};
    float jtdynkt[MAXJETS] = {0};
    float jtdynz[MAXJETS] = {0};
    int jtNPrimarySD[MAXJETS] = {0};
    int jtNTotalSD[MAXJETS] = {0};
    int jtNPrimaryKT[MAXJETS][MAXNKT] = {{0}};
    int jtNTotalKT[MAXJETS][MAXNKT] = {{0}};
    int jtintjetmulti[MAXJETS] = {0};
    float jtgirth[MAXJETS] = {0};
    float jtthrust[MAXJETS] = {0};
    float jtLHA[MAXJETS] = {0};
    float jtpTD[MAXJETS] = {0};
    float jtdyntauform[MAXJETS] = {0};
    std::vector<std::vector<float>> jtPLJPkT = {};
    std::vector<std::vector<float>> jtPLJPdR = {};
    std::vector<std::vector<float>> jtPLJPeta = {};
    std::vector<std::vector<float>> jtPLJPphi = {};

    std::vector<std::vector<float>> jtTotalSDAngles = {};
    std::vector<std::vector<float>> jtTotalSDAnglesPljp = {};
    std::vector<float> jtAvgTotalSDAngle = {};
    std::vector<float> jtAvgPrimarySDAngle = {};

    std::vector<std::vector<std::vector<float>>> jtTotalKTAngles = {};
    std::vector<std::vector<std::vector<float>>> jtTotalKTAnglesPljp = {};
    std::vector<std::vector<float>> jtAvgTotalKTAngle = {};
    std::vector<std::vector<float>> jtAvgPrimaryKTAngle = {};

    float jtsym[MAXJETS] = {0};
    int jtdroppedBranches[MAXJETS] = {0};

    std::vector<std::vector<float>> jtSubJetPt = {};
    std::vector<std::vector<float>> jtSubJetEta = {};
    std::vector<std::vector<float>> jtSubJetPhi = {};
    std::vector<std::vector<float>> jtSubJetM = {};

    std::vector<std::vector<int>> jtConstituentsId = {};
    std::vector<std::vector<float>> jtConstituentsE = {};
    std::vector<std::vector<float>> jtConstituentsPt = {};
    std::vector<std::vector<float>> jtConstituentsEta = {};
    std::vector<std::vector<float>> jtConstituentsPhi = {};
    std::vector<std::vector<float>> jtConstituentsM = {};
    std::vector<std::vector<int>> jtSDConstituentsId = {};
    std::vector<std::vector<float>> jtSDConstituentsE = {};
    std::vector<std::vector<float>> jtSDConstituentsPt = {};
    std::vector<std::vector<float>> jtSDConstituentsEta = {};
    std::vector<std::vector<float>> jtSDConstituentsPhi = {};
    std::vector<std::vector<float>> jtSDConstituentsM = {};

    float trackMax[MAXJETS] = {0};
    float trackSum[MAXJETS] = {0};
    int trackN[MAXJETS] = {0};

    float chargedMax[MAXJETS] = {0};
    float chargedSum[MAXJETS] = {0};
    int chargedN[MAXJETS] = {0};

    float photonMax[MAXJETS] = {0};
    float photonSum[MAXJETS] = {0};
    int photonN[MAXJETS] = {0};

    float trackHardSum[MAXJETS] = {0};
    float chargedHardSum[MAXJETS] = {0};
    float photonHardSum[MAXJETS] = {0};

    int trackHardN[MAXJETS] = {0};
    int chargedHardN[MAXJETS] = {0};
    int photonHardN[MAXJETS] = {0};

    float neutralMax[MAXJETS] = {0};
    float neutralSum[MAXJETS] = {0};
    int neutralN[MAXJETS] = {0};

    float eMax[MAXJETS] = {0};
    float eSum[MAXJETS] = {0};
    int eN[MAXJETS] = {0};

    float muMax[MAXJETS] = {0};
    float muSum[MAXJETS] = {0};
    int muN[MAXJETS] = {0};

    float genChargedSum[MAXJETS] = {0};
    float genHardSum[MAXJETS] = {0};
    float signalChargedSum[MAXJETS] = {0};
    float signalHardSum[MAXJETS] = {0};

    float fHPD[MAXJETS] = {0};
    float fRBX[MAXJETS] = {0};
    int n90[MAXJETS] = {0};
    float fSubDet1[MAXJETS] = {0};
    float fSubDet2[MAXJETS] = {0};
    float fSubDet3[MAXJETS] = {0};
    float fSubDet4[MAXJETS] = {0};
    float restrictedEMF[MAXJETS] = {0};
    int nHCAL[MAXJETS] = {0};
    int nECAL[MAXJETS] = {0};
    float apprHPD[MAXJETS] = {0};
    float apprRBX[MAXJETS] = {0};

    int n2RPC[MAXJETS] = {0};
    int n3RPC[MAXJETS] = {0};
    int nRPC[MAXJETS] = {0};

    float fEB[MAXJETS] = {0};
    float fEE[MAXJETS] = {0};
    float fHB[MAXJETS] = {0};
    float fHE[MAXJETS] = {0};
    float fHO[MAXJETS] = {0};
    float fLong[MAXJETS] = {0};
    float fShort[MAXJETS] = {0};
    float fLS[MAXJETS] = {0};
    float fHFOOT[MAXJETS] = {0};

    int subid[MAXJETS] = {0};

    float matchedPt[MAXJETS] = {0};
    float matchedRawPt[MAXJETS] = {0};
    float matchedRawE[MAXJETS] = {0};
    float matchedR[MAXJETS] = {0};
    float matchedPu[MAXJETS] = {0};
    int matchedHadronFlavor[MAXJETS] = {0};
    int matchedPartonFlavor[MAXJETS] = {0};

    float discr_csvV2[MAXJETS] = {0};
    float discr_deepCSV[MAXJETS] = {0};
    float discr_pX[MAXJETS] = {0};
    float discr_pfJP[MAXJETS] = {0};
    float discr_muByIp3[MAXJETS] = {0};
    float discr_muByPt[MAXJETS] = {0};
    float discr_prob[MAXJETS] = {0};
    float discr_probb[MAXJETS] = {0};
    float discr_tcHighEff[MAXJETS] = {0};
    float discr_tcHighPur[MAXJETS] = {0};
    float discr_ssvHighEff[MAXJETS] = {0};
    float discr_ssvHighPur[MAXJETS] = {0};

    float ndiscr_ssvHighEff[MAXJETS] = {0};
    float ndiscr_ssvHighPur[MAXJETS] = {0};
    float ndiscr_csvV1[MAXJETS] = {0};
    float ndiscr_csvV2[MAXJETS] = {0};
    float ndiscr_muByPt[MAXJETS] = {0};

    float pdiscr_csvV1[MAXJETS] = {0};
    float pdiscr_csvV2[MAXJETS] = {0};

    int nsvtx[MAXJETS] = {0};
    int svtxntrk[MAXJETS] = {0};
    float svtxdl[MAXJETS] = {0};
    float svtxdls[MAXJETS] = {0};
    float svtxdl2d[MAXJETS] = {0};
    float svtxdls2d[MAXJETS] = {0};
    float svtxm[MAXJETS] = {0};
    float svtxpt[MAXJETS] = {0};
    float svtxmcorr[MAXJETS] = {0};
    float svtxnormchi2[MAXJETS] = {0};
    float svJetDeltaR[MAXJETS] = {0};
    float svtxTrkSumChi2[MAXJETS] = {0};
    int svtxTrkNetCharge[MAXJETS] = {0};
    int svtxNtrkInCone[MAXJETS] = {0};

    float trackPtRel[MAXTRACKS] = {0};
    float trackPtRatio[MAXTRACKS] = {0};
    float trackPPar[MAXTRACKS] = {0};
    float trackPParRatio[MAXTRACKS] = {0};
    float trackDeltaR[MAXTRACKS] = {0};

    float mue[MAXJETS] = {0};
    float mupt[MAXJETS] = {0};
    float mueta[MAXJETS] = {0};
    float muphi[MAXJETS] = {0};
    float mudr[MAXJETS] = {0};
    float muptrel[MAXJETS] = {0};
    int muchg[MAXJETS] = {0};

    float refpt[MAXJETS] = {0};
    float refeta[MAXJETS] = {0};
    float refphi[MAXJETS] = {0};
    float refm[MAXJETS] = {0};
    float refarea[MAXJETS] = {0};
    float refy[MAXJETS] = {0};
    float reftau1[MAXJETS] = {0};
    float reftau2[MAXJETS] = {0};
    float reftau3[MAXJETS] = {0};
    float refsym[MAXJETS] = {0};
    int refdroppedBranches[MAXJETS] = {0};
    float refdphijt[MAXJETS] = {0};
    float refdrjt[MAXJETS] = {0};
    float refparton_pt[MAXJETS] = {0};
    int refparton_flavor[MAXJETS] = {0};
    int refparton_flavorForB[MAXJETS] = {0};

    int refdynsplit[MAXJETS] = {0};
    float refdyneta[MAXJETS] = {0};
    float refdynphi[MAXJETS] = {0};
    float refdyndeltaR[MAXJETS] = {0};
    float refdynkt[MAXJETS] = {0};
    float refdynz[MAXJETS] = {0};
    int refNPrimarySD[MAXJETS] = {0};
    int refNTotalSD[MAXJETS] = {0};
    int refNPrimaryKT[MAXJETS][MAXNKT] = {{0}};
    int refNTotalKT[MAXJETS][MAXNKT] = {{0}};
    int refintjetmulti[MAXJETS] = {0};
    float refgirth[MAXJETS] = {0};
    float refthrust[MAXJETS] = {0};
    float refLHA[MAXJETS] = {0};
    float refpTD[MAXJETS] = {0};
    float refdyntauform[MAXJETS] = {0};
    std::vector<std::vector<float>> refPLJPkT = {};
    std::vector<std::vector<float>> refPLJPdR = {};
    std::vector<std::vector<float>> refPLJPeta = {};
    std::vector<std::vector<float>> refPLJPphi = {};

    std::vector<std::vector<float>> refTotalSDAngles = {};
    std::vector<std::vector<float>> refTotalSDAnglesPljp = {};
    std::vector<float> refAvgTotalSDAngle = {};
    std::vector<float> refAvgPrimarySDAngle = {};

    std::vector<std::vector<std::vector<float>>> refTotalKTAngles = {};
    std::vector<std::vector<std::vector<float>>> refTotalKTAnglesPljp = {};
    std::vector<std::vector<float>> refAvgTotalKTAngle = {};
    std::vector<std::vector<float>> refAvgPrimaryKTAngle = {};

    float refptG[MAXJETS] = {0};
    float refetaG[MAXJETS] = {0};
    float refphiG[MAXJETS] = {0};
    float refmG[MAXJETS] = {0};
    std::vector<std::vector<float>> refSubJetPt = {};
    std::vector<std::vector<float>> refSubJetEta = {};
    std::vector<std::vector<float>> refSubJetPhi = {};
    std::vector<std::vector<float>> refSubJetM = {};

    std::vector<std::vector<int>> refConstituentsId = {};
    std::vector<std::vector<float>> refConstituentsE = {};
    std::vector<std::vector<float>> refConstituentsPt = {};
    std::vector<std::vector<float>> refConstituentsEta = {};
    std::vector<std::vector<float>> refConstituentsPhi = {};
    std::vector<std::vector<float>> refConstituentsM = {};
    std::vector<std::vector<int>> refSDConstituentsId = {};
    std::vector<std::vector<float>> refSDConstituentsE = {};
    std::vector<std::vector<float>> refSDConstituentsPt = {};
    std::vector<std::vector<float>> refSDConstituentsEta = {};
    std::vector<std::vector<float>> refSDConstituentsPhi = {};
    std::vector<std::vector<float>> refSDConstituentsM = {};

    float pthat = 0;
    int beamId1 = 0;
    int beamId2 = 0;
    int ngen = 0;
    int genmatchindex[MAXJETS] = {0};
    float genpt[MAXJETS] = {0};
    float geneta[MAXJETS] = {0};
    float genphi[MAXJETS] = {0};
    float genm[MAXJETS] = {0};
    float geny[MAXJETS] = {0};
    float gentau1[MAXJETS] = {0};
    float gentau2[MAXJETS] = {0};
    float gentau3[MAXJETS] = {0};
    float gendphijt[MAXJETS] = {0};
    float gendrjt[MAXJETS] = {0};
    int gensubid[MAXJETS] = {0};

    int gendynsplit[MAXJETS] = {0};
    float gendyneta[MAXJETS] = {0};
    float gendynphi[MAXJETS] = {0};
    float gendyndeltaR[MAXJETS] = {0};
    float gendynkt[MAXJETS] = {0};
    float gendynz[MAXJETS] = {0};
    int genNPrimarySD[MAXJETS] = {0};
    int genNTotalSD[MAXJETS] = {0};
    int genNPrimaryKT[MAXJETS][MAXNKT] = {{0}};
    int genNTotalKT[MAXJETS][MAXNKT] = {{0}};
    int genintjetmulti[MAXJETS] = {0};
    float gengirth[MAXJETS] = {0};
    float genthrust[MAXJETS] = {0};
    float genLHA[MAXJETS] = {0};
    float genpTD[MAXJETS] = {0};
    float gendyntauform[MAXJETS] = {0};
    std::vector<std::vector<float>> genPLJPkT = {};
    std::vector<std::vector<float>> genPLJPdR = {};
    std::vector<std::vector<float>> genPLJPeta = {};
    std::vector<std::vector<float>> genPLJPphi = {};

    std::vector<std::vector<float>> genTotalSDAngles = {};
    std::vector<std::vector<float>> genTotalSDAnglesPljp = {};
    std::vector<float> genAvgTotalSDAngle = {};
    std::vector<float> genAvgPrimarySDAngle = {};

    std::vector<std::vector<std::vector<float>>> genTotalKTAngles = {};
    std::vector<std::vector<std::vector<float>>> genTotalKTAnglesPljp = {};
    std::vector<std::vector<float>> genAvgTotalKTAngle = {};
    std::vector<std::vector<float>> genAvgPrimaryKTAngle = {};

    float genptG[MAXJETS] = {0};
    float genetaG[MAXJETS] = {0};
    float genphiG[MAXJETS] = {0};
    float genmG[MAXJETS] = {0};
    std::vector<std::vector<float>> genSubJetPt = {};
    std::vector<std::vector<float>> genSubJetEta = {};
    std::vector<std::vector<float>> genSubJetPhi = {};
    std::vector<std::vector<float>> genSubJetM = {};
    std::vector<std::vector<float>> genSubJetArea = {};
    float gensym[MAXJETS] = {0};
    int gendroppedBranches[MAXJETS] = {0};

    std::vector<std::vector<int>> genConstituentsId = {};
    std::vector<std::vector<float>> genConstituentsE = {};
    std::vector<std::vector<float>> genConstituentsPt = {};
    std::vector<std::vector<float>> genConstituentsEta = {};
    std::vector<std::vector<float>> genConstituentsPhi = {};
    std::vector<std::vector<float>> genConstituentsM = {};
    std::vector<std::vector<int>> genSDConstituentsId = {};
    std::vector<std::vector<float>> genSDConstituentsE = {};
    std::vector<std::vector<float>> genSDConstituentsPt = {};
    std::vector<std::vector<float>> genSDConstituentsEta = {};
    std::vector<std::vector<float>> genSDConstituentsPhi = {};
    std::vector<std::vector<float>> genSDConstituentsM = {};

    float calopt[MAXCALO] = {0};
    float caloeta[MAXCALO] = {0};
    float calophi[MAXCALO] = {0};
  };

  JRA jets_;
  std::map<std::string, std::map<std::string, std::array<float, MAXJETS>>> jets_discr_;
};

#endif
