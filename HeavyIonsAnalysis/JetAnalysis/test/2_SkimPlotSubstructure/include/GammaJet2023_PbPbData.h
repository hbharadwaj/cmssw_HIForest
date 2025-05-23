//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Fri May 23 16:03:22 2025 by ROOT version 6.30/08
// from TTree jet_tree/jet_tree
// found on file: root://eoscms.cern.ch//eos/cms/store/group/phys_heavyions/bharikri/Run3GammaJet/2023_PbPb/Data/2025_05_22_Data_HiRawPrime0_part_skimmed_batch0_of_85.root
//////////////////////////////////////////////////////////

#ifndef GammaJet2023_PbPbData_h
#define GammaJet2023_PbPbData_h

#include "GammaJetAnalysis.h"
#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include "ROOT/RVec.hxx"
#include "ROOT/RVec.hxx"
#include "ROOT/RVec.hxx"

class GammaJet2023_PbPbData : public GammaJetAnalysis {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   Int_t           AK2Z1_run;
   Int_t           AK2Z1_evt;
   Int_t           AK2Z1_lumi;
   Int_t           AK2Z1_nref;
   Int_t           AK2Z1_ncalo;
   Int_t           nref;
   Float_t         AK2Z1_rawpt[117];   //[nref]
   Float_t         AK2Z1_jtpt[117];   //[nref]
   Float_t         AK2Z1_jteta[117];   //[nref]
   Float_t         AK2Z1_jty[117];   //[nref]
   Float_t         AK2Z1_jtphi[117];   //[nref]
   Float_t         AK2Z1_jtpu[117];   //[nref]
   Float_t         AK2Z1_jtm[117];   //[nref]
   Float_t         AK2Z1_jtarea[117];   //[nref]
   Int_t           AK2Z1_jtdyn_split[117];   //[nref]
   Float_t         AK2Z1_jtdyn_eta[117];   //[nref]
   Float_t         AK2Z1_jtdyn_phi[117];   //[nref]
   Float_t         AK2Z1_jtdyn_deltaR[117];   //[nref]
   Float_t         AK2Z1_jtdyn_kt[117];   //[nref]
   Float_t         AK2Z1_jtdyn_z[117];   //[nref]
   Int_t           AK2Z1_jt_intjet_multi[117];   //[nref]
   Float_t         AK2Z1_jt_girth[117];   //[nref]
   Float_t         AK2Z1_jt_thrust[117];   //[nref]
   Float_t         AK2Z1_jt_LHA[117];   //[nref]
   Float_t         AK2Z1_jt_pTD[117];   //[nref]
   Int_t           AK2Z2_run;
   Int_t           AK2Z2_evt;
   Int_t           AK2Z2_lumi;
   Int_t           AK2Z2_nref;
   Int_t           AK2Z2_ncalo;
   Float_t         AK2Z2_rawpt[117];   //[nref]
   Float_t         AK2Z2_jtpt[117];   //[nref]
   Float_t         AK2Z2_jteta[117];   //[nref]
   Float_t         AK2Z2_jty[117];   //[nref]
   Float_t         AK2Z2_jtphi[117];   //[nref]
   Float_t         AK2Z2_jtpu[117];   //[nref]
   Float_t         AK2Z2_jtm[117];   //[nref]
   Float_t         AK2Z2_jtarea[117];   //[nref]
   Int_t           AK2Z2_jtdyn_split[117];   //[nref]
   Float_t         AK2Z2_jtdyn_eta[117];   //[nref]
   Float_t         AK2Z2_jtdyn_phi[117];   //[nref]
   Float_t         AK2Z2_jtdyn_deltaR[117];   //[nref]
   Float_t         AK2Z2_jtdyn_kt[117];   //[nref]
   Float_t         AK2Z2_jtdyn_z[117];   //[nref]
   Int_t           AK2Z2_jt_intjet_multi[117];   //[nref]
   Float_t         AK2Z2_jt_girth[117];   //[nref]
   Float_t         AK2Z2_jt_thrust[117];   //[nref]
   Float_t         AK2Z2_jt_LHA[117];   //[nref]
   Float_t         AK2Z2_jt_pTD[117];   //[nref]
   Int_t           AK2Z3_run;
   Int_t           AK2Z3_evt;
   Int_t           AK2Z3_lumi;
   Int_t           AK2Z3_nref;
   Int_t           AK2Z3_ncalo;
   Float_t         AK2Z3_rawpt[117];   //[nref]
   Float_t         AK2Z3_jtpt[117];   //[nref]
   Float_t         AK2Z3_jteta[117];   //[nref]
   Float_t         AK2Z3_jty[117];   //[nref]
   Float_t         AK2Z3_jtphi[117];   //[nref]
   Float_t         AK2Z3_jtpu[117];   //[nref]
   Float_t         AK2Z3_jtm[117];   //[nref]
   Float_t         AK2Z3_jtarea[117];   //[nref]
   Int_t           AK2Z3_jtdyn_split[117];   //[nref]
   Float_t         AK2Z3_jtdyn_eta[117];   //[nref]
   Float_t         AK2Z3_jtdyn_phi[117];   //[nref]
   Float_t         AK2Z3_jtdyn_deltaR[117];   //[nref]
   Float_t         AK2Z3_jtdyn_kt[117];   //[nref]
   Float_t         AK2Z3_jtdyn_z[117];   //[nref]
   Int_t           AK2Z3_jt_intjet_multi[117];   //[nref]
   Float_t         AK2Z3_jt_girth[117];   //[nref]
   Float_t         AK2Z3_jt_thrust[117];   //[nref]
   Float_t         AK2Z3_jt_LHA[117];   //[nref]
   Float_t         AK2Z3_jt_pTD[117];   //[nref]
   Int_t           AK2Z4_run;
   Int_t           AK2Z4_evt;
   Int_t           AK2Z4_lumi;
   Int_t           AK2Z4_nref;
   Int_t           AK2Z4_ncalo;
   Float_t         AK2Z4_rawpt[117];   //[nref]
   Float_t         AK2Z4_jtpt[117];   //[nref]
   Float_t         AK2Z4_jteta[117];   //[nref]
   Float_t         AK2Z4_jty[117];   //[nref]
   Float_t         AK2Z4_jtphi[117];   //[nref]
   Float_t         AK2Z4_jtpu[117];   //[nref]
   Float_t         AK2Z4_jtm[117];   //[nref]
   Float_t         AK2Z4_jtarea[117];   //[nref]
   Int_t           AK2Z4_jtdyn_split[117];   //[nref]
   Float_t         AK2Z4_jtdyn_eta[117];   //[nref]
   Float_t         AK2Z4_jtdyn_phi[117];   //[nref]
   Float_t         AK2Z4_jtdyn_deltaR[117];   //[nref]
   Float_t         AK2Z4_jtdyn_kt[117];   //[nref]
   Float_t         AK2Z4_jtdyn_z[117];   //[nref]
   Int_t           AK2Z4_jt_intjet_multi[117];   //[nref]
   Float_t         AK2Z4_jt_girth[117];   //[nref]
   Float_t         AK2Z4_jt_thrust[117];   //[nref]
   Float_t         AK2Z4_jt_LHA[117];   //[nref]
   Float_t         AK2Z4_jt_pTD[117];   //[nref]
   Int_t           AK3Z1_run;
   Int_t           AK3Z1_evt;
   Int_t           AK3Z1_lumi;
   Int_t           AK3Z1_nref;
   Int_t           AK3Z1_ncalo;
   Float_t         AK3Z1_rawpt[117];   //[nref]
   Float_t         AK3Z1_jtpt[117];   //[nref]
   Float_t         AK3Z1_jteta[117];   //[nref]
   Float_t         AK3Z1_jty[117];   //[nref]
   Float_t         AK3Z1_jtphi[117];   //[nref]
   Float_t         AK3Z1_jtpu[117];   //[nref]
   Float_t         AK3Z1_jtm[117];   //[nref]
   Float_t         AK3Z1_jtarea[117];   //[nref]
   Int_t           AK3Z1_jtdyn_split[117];   //[nref]
   Float_t         AK3Z1_jtdyn_eta[117];   //[nref]
   Float_t         AK3Z1_jtdyn_phi[117];   //[nref]
   Float_t         AK3Z1_jtdyn_deltaR[117];   //[nref]
   Float_t         AK3Z1_jtdyn_kt[117];   //[nref]
   Float_t         AK3Z1_jtdyn_z[117];   //[nref]
   Int_t           AK3Z1_jt_intjet_multi[117];   //[nref]
   Float_t         AK3Z1_jt_girth[117];   //[nref]
   Float_t         AK3Z1_jt_thrust[117];   //[nref]
   Float_t         AK3Z1_jt_LHA[117];   //[nref]
   Float_t         AK3Z1_jt_pTD[117];   //[nref]
   Int_t           AK3Z2_run;
   Int_t           AK3Z2_evt;
   Int_t           AK3Z2_lumi;
   Int_t           AK3Z2_nref;
   Int_t           AK3Z2_ncalo;
   Float_t         AK3Z2_rawpt[117];   //[nref]
   Float_t         AK3Z2_jtpt[117];   //[nref]
   Float_t         AK3Z2_jteta[117];   //[nref]
   Float_t         AK3Z2_jty[117];   //[nref]
   Float_t         AK3Z2_jtphi[117];   //[nref]
   Float_t         AK3Z2_jtpu[117];   //[nref]
   Float_t         AK3Z2_jtm[117];   //[nref]
   Float_t         AK3Z2_jtarea[117];   //[nref]
   Int_t           AK3Z2_jtdyn_split[117];   //[nref]
   Float_t         AK3Z2_jtdyn_eta[117];   //[nref]
   Float_t         AK3Z2_jtdyn_phi[117];   //[nref]
   Float_t         AK3Z2_jtdyn_deltaR[117];   //[nref]
   Float_t         AK3Z2_jtdyn_kt[117];   //[nref]
   Float_t         AK3Z2_jtdyn_z[117];   //[nref]
   Int_t           AK3Z2_jt_intjet_multi[117];   //[nref]
   Float_t         AK3Z2_jt_girth[117];   //[nref]
   Float_t         AK3Z2_jt_thrust[117];   //[nref]
   Float_t         AK3Z2_jt_LHA[117];   //[nref]
   Float_t         AK3Z2_jt_pTD[117];   //[nref]
   Int_t           AK3Z3_run;
   Int_t           AK3Z3_evt;
   Int_t           AK3Z3_lumi;
   Int_t           AK3Z3_nref;
   Int_t           AK3Z3_ncalo;
   Float_t         AK3Z3_rawpt[117];   //[nref]
   Float_t         AK3Z3_jtpt[117];   //[nref]
   Float_t         AK3Z3_jteta[117];   //[nref]
   Float_t         AK3Z3_jty[117];   //[nref]
   Float_t         AK3Z3_jtphi[117];   //[nref]
   Float_t         AK3Z3_jtpu[117];   //[nref]
   Float_t         AK3Z3_jtm[117];   //[nref]
   Float_t         AK3Z3_jtarea[117];   //[nref]
   Int_t           AK3Z3_jtdyn_split[117];   //[nref]
   Float_t         AK3Z3_jtdyn_eta[117];   //[nref]
   Float_t         AK3Z3_jtdyn_phi[117];   //[nref]
   Float_t         AK3Z3_jtdyn_deltaR[117];   //[nref]
   Float_t         AK3Z3_jtdyn_kt[117];   //[nref]
   Float_t         AK3Z3_jtdyn_z[117];   //[nref]
   Int_t           AK3Z3_jt_intjet_multi[117];   //[nref]
   Float_t         AK3Z3_jt_girth[117];   //[nref]
   Float_t         AK3Z3_jt_thrust[117];   //[nref]
   Float_t         AK3Z3_jt_LHA[117];   //[nref]
   Float_t         AK3Z3_jt_pTD[117];   //[nref]
   Int_t           AK3Z4_run;
   Int_t           AK3Z4_evt;
   Int_t           AK3Z4_lumi;
   Int_t           AK3Z4_nref;
   Int_t           AK3Z4_ncalo;
   Float_t         AK3Z4_rawpt[117];   //[nref]
   Float_t         AK3Z4_jtpt[117];   //[nref]
   Float_t         AK3Z4_jteta[117];   //[nref]
   Float_t         AK3Z4_jty[117];   //[nref]
   Float_t         AK3Z4_jtphi[117];   //[nref]
   Float_t         AK3Z4_jtpu[117];   //[nref]
   Float_t         AK3Z4_jtm[117];   //[nref]
   Float_t         AK3Z4_jtarea[117];   //[nref]
   Int_t           AK3Z4_jtdyn_split[117];   //[nref]
   Float_t         AK3Z4_jtdyn_eta[117];   //[nref]
   Float_t         AK3Z4_jtdyn_phi[117];   //[nref]
   Float_t         AK3Z4_jtdyn_deltaR[117];   //[nref]
   Float_t         AK3Z4_jtdyn_kt[117];   //[nref]
   Float_t         AK3Z4_jtdyn_z[117];   //[nref]
   Int_t           AK3Z4_jt_intjet_multi[117];   //[nref]
   Float_t         AK3Z4_jt_girth[117];   //[nref]
   Float_t         AK3Z4_jt_thrust[117];   //[nref]
   Float_t         AK3Z4_jt_LHA[117];   //[nref]
   Float_t         AK3Z4_jt_pTD[117];   //[nref]
   Int_t           AK3Z5_run;
   Int_t           AK3Z5_evt;
   Int_t           AK3Z5_lumi;
   Int_t           AK3Z5_nref;
   Int_t           AK3Z5_ncalo;
   Float_t         AK3Z5_rawpt[117];   //[nref]
   Float_t         AK3Z5_jtpt[117];   //[nref]
   Float_t         AK3Z5_jteta[117];   //[nref]
   Float_t         AK3Z5_jty[117];   //[nref]
   Float_t         AK3Z5_jtphi[117];   //[nref]
   Float_t         AK3Z5_jtpu[117];   //[nref]
   Float_t         AK3Z5_jtm[117];   //[nref]
   Float_t         AK3Z5_jtarea[117];   //[nref]
   Int_t           AK3Z5_jtdyn_split[117];   //[nref]
   Float_t         AK3Z5_jtdyn_eta[117];   //[nref]
   Float_t         AK3Z5_jtdyn_phi[117];   //[nref]
   Float_t         AK3Z5_jtdyn_deltaR[117];   //[nref]
   Float_t         AK3Z5_jtdyn_kt[117];   //[nref]
   Float_t         AK3Z5_jtdyn_z[117];   //[nref]
   Int_t           AK3Z5_jt_intjet_multi[117];   //[nref]
   Float_t         AK3Z5_jt_girth[117];   //[nref]
   Float_t         AK3Z5_jt_thrust[117];   //[nref]
   Float_t         AK3Z5_jt_LHA[117];   //[nref]
   Float_t         AK3Z5_jt_pTD[117];   //[nref]
   Int_t           AK4Z1_run;
   Int_t           AK4Z1_evt;
   Int_t           AK4Z1_lumi;
   Int_t           AK4Z1_nref;
   Int_t           AK4Z1_ncalo;
   Float_t         AK4Z1_rawpt[117];   //[nref]
   Float_t         AK4Z1_jtpt[117];   //[nref]
   Float_t         AK4Z1_jteta[117];   //[nref]
   Float_t         AK4Z1_jty[117];   //[nref]
   Float_t         AK4Z1_jtphi[117];   //[nref]
   Float_t         AK4Z1_jtpu[117];   //[nref]
   Float_t         AK4Z1_jtm[117];   //[nref]
   Float_t         AK4Z1_jtarea[117];   //[nref]
   Int_t           AK4Z1_jtdyn_split[117];   //[nref]
   Float_t         AK4Z1_jtdyn_eta[117];   //[nref]
   Float_t         AK4Z1_jtdyn_phi[117];   //[nref]
   Float_t         AK4Z1_jtdyn_deltaR[117];   //[nref]
   Float_t         AK4Z1_jtdyn_kt[117];   //[nref]
   Float_t         AK4Z1_jtdyn_z[117];   //[nref]
   Int_t           AK4Z1_jt_intjet_multi[117];   //[nref]
   Float_t         AK4Z1_jt_girth[117];   //[nref]
   Float_t         AK4Z1_jt_thrust[117];   //[nref]
   Float_t         AK4Z1_jt_LHA[117];   //[nref]
   Float_t         AK4Z1_jt_pTD[117];   //[nref]
   Int_t           AK4Z2_run;
   Int_t           AK4Z2_evt;
   Int_t           AK4Z2_lumi;
   Int_t           AK4Z2_nref;
   Int_t           AK4Z2_ncalo;
   Float_t         AK4Z2_rawpt[117];   //[nref]
   Float_t         AK4Z2_jtpt[117];   //[nref]
   Float_t         AK4Z2_jteta[117];   //[nref]
   Float_t         AK4Z2_jty[117];   //[nref]
   Float_t         AK4Z2_jtphi[117];   //[nref]
   Float_t         AK4Z2_jtpu[117];   //[nref]
   Float_t         AK4Z2_jtm[117];   //[nref]
   Float_t         AK4Z2_jtarea[117];   //[nref]
   Int_t           AK4Z2_jtdyn_split[117];   //[nref]
   Float_t         AK4Z2_jtdyn_eta[117];   //[nref]
   Float_t         AK4Z2_jtdyn_phi[117];   //[nref]
   Float_t         AK4Z2_jtdyn_deltaR[117];   //[nref]
   Float_t         AK4Z2_jtdyn_kt[117];   //[nref]
   Float_t         AK4Z2_jtdyn_z[117];   //[nref]
   Int_t           AK4Z2_jt_intjet_multi[117];   //[nref]
   Float_t         AK4Z2_jt_girth[117];   //[nref]
   Float_t         AK4Z2_jt_thrust[117];   //[nref]
   Float_t         AK4Z2_jt_LHA[117];   //[nref]
   Float_t         AK4Z2_jt_pTD[117];   //[nref]
   Int_t           AK4Z3_run;
   Int_t           AK4Z3_evt;
   Int_t           AK4Z3_lumi;
   Int_t           AK4Z3_nref;
   Int_t           AK4Z3_ncalo;
   Float_t         AK4Z3_rawpt[117];   //[nref]
   Float_t         AK4Z3_jtpt[117];   //[nref]
   Float_t         AK4Z3_jteta[117];   //[nref]
   Float_t         AK4Z3_jty[117];   //[nref]
   Float_t         AK4Z3_jtphi[117];   //[nref]
   Float_t         AK4Z3_jtpu[117];   //[nref]
   Float_t         AK4Z3_jtm[117];   //[nref]
   Float_t         AK4Z3_jtarea[117];   //[nref]
   Int_t           AK4Z3_jtdyn_split[117];   //[nref]
   Float_t         AK4Z3_jtdyn_eta[117];   //[nref]
   Float_t         AK4Z3_jtdyn_phi[117];   //[nref]
   Float_t         AK4Z3_jtdyn_deltaR[117];   //[nref]
   Float_t         AK4Z3_jtdyn_kt[117];   //[nref]
   Float_t         AK4Z3_jtdyn_z[117];   //[nref]
   Int_t           AK4Z3_jt_intjet_multi[117];   //[nref]
   Float_t         AK4Z3_jt_girth[117];   //[nref]
   Float_t         AK4Z3_jt_thrust[117];   //[nref]
   Float_t         AK4Z3_jt_LHA[117];   //[nref]
   Float_t         AK4Z3_jt_pTD[117];   //[nref]
   Int_t           AK4Z4_run;
   Int_t           AK4Z4_evt;
   Int_t           AK4Z4_lumi;
   Int_t           AK4Z4_nref;
   Int_t           AK4Z4_ncalo;
   Float_t         AK4Z4_rawpt[117];   //[nref]
   Float_t         AK4Z4_jtpt[117];   //[nref]
   Float_t         AK4Z4_jteta[117];   //[nref]
   Float_t         AK4Z4_jty[117];   //[nref]
   Float_t         AK4Z4_jtphi[117];   //[nref]
   Float_t         AK4Z4_jtpu[117];   //[nref]
   Float_t         AK4Z4_jtm[117];   //[nref]
   Float_t         AK4Z4_jtarea[117];   //[nref]
   Int_t           AK4Z4_jtdyn_split[117];   //[nref]
   Float_t         AK4Z4_jtdyn_eta[117];   //[nref]
   Float_t         AK4Z4_jtdyn_phi[117];   //[nref]
   Float_t         AK4Z4_jtdyn_deltaR[117];   //[nref]
   Float_t         AK4Z4_jtdyn_kt[117];   //[nref]
   Float_t         AK4Z4_jtdyn_z[117];   //[nref]
   Int_t           AK4Z4_jt_intjet_multi[117];   //[nref]
   Float_t         AK4Z4_jt_girth[117];   //[nref]
   Float_t         AK4Z4_jt_thrust[117];   //[nref]
   Float_t         AK4Z4_jt_LHA[117];   //[nref]
   Float_t         AK4Z4_jt_pTD[117];   //[nref]
   Int_t           AK4Z5_run;
   Int_t           AK4Z5_evt;
   Int_t           AK4Z5_lumi;
   Int_t           AK4Z5_nref;
   Int_t           AK4Z5_ncalo;
   Float_t         AK4Z5_rawpt[117];   //[nref]
   Float_t         AK4Z5_jtpt[117];   //[nref]
   Float_t         AK4Z5_jteta[117];   //[nref]
   Float_t         AK4Z5_jty[117];   //[nref]
   Float_t         AK4Z5_jtphi[117];   //[nref]
   Float_t         AK4Z5_jtpu[117];   //[nref]
   Float_t         AK4Z5_jtm[117];   //[nref]
   Float_t         AK4Z5_jtarea[117];   //[nref]
   Int_t           AK4Z5_jtdyn_split[117];   //[nref]
   Float_t         AK4Z5_jtdyn_eta[117];   //[nref]
   Float_t         AK4Z5_jtdyn_phi[117];   //[nref]
   Float_t         AK4Z5_jtdyn_deltaR[117];   //[nref]
   Float_t         AK4Z5_jtdyn_kt[117];   //[nref]
   Float_t         AK4Z5_jtdyn_z[117];   //[nref]
   Int_t           AK4Z5_jt_intjet_multi[117];   //[nref]
   Float_t         AK4Z5_jt_girth[117];   //[nref]
   Float_t         AK4Z5_jt_thrust[117];   //[nref]
   Float_t         AK4Z5_jt_LHA[117];   //[nref]
   Float_t         AK4Z5_jt_pTD[117];   //[nref]
   Int_t           AK5Z1_run;
   Int_t           AK5Z1_evt;
   Int_t           AK5Z1_lumi;
   Int_t           AK5Z1_nref;
   Int_t           AK5Z1_ncalo;
   Float_t         AK5Z1_rawpt[117];   //[nref]
   Float_t         AK5Z1_jtpt[117];   //[nref]
   Float_t         AK5Z1_jteta[117];   //[nref]
   Float_t         AK5Z1_jty[117];   //[nref]
   Float_t         AK5Z1_jtphi[117];   //[nref]
   Float_t         AK5Z1_jtpu[117];   //[nref]
   Float_t         AK5Z1_jtm[117];   //[nref]
   Float_t         AK5Z1_jtarea[117];   //[nref]
   Int_t           AK5Z1_jtdyn_split[117];   //[nref]
   Float_t         AK5Z1_jtdyn_eta[117];   //[nref]
   Float_t         AK5Z1_jtdyn_phi[117];   //[nref]
   Float_t         AK5Z1_jtdyn_deltaR[117];   //[nref]
   Float_t         AK5Z1_jtdyn_kt[117];   //[nref]
   Float_t         AK5Z1_jtdyn_z[117];   //[nref]
   Int_t           AK5Z1_jt_intjet_multi[117];   //[nref]
   Float_t         AK5Z1_jt_girth[117];   //[nref]
   Float_t         AK5Z1_jt_thrust[117];   //[nref]
   Float_t         AK5Z1_jt_LHA[117];   //[nref]
   Float_t         AK5Z1_jt_pTD[117];   //[nref]
   Int_t           AK5Z2_run;
   Int_t           AK5Z2_evt;
   Int_t           AK5Z2_lumi;
   Int_t           AK5Z2_nref;
   Int_t           AK5Z2_ncalo;
   Float_t         AK5Z2_rawpt[117];   //[nref]
   Float_t         AK5Z2_jtpt[117];   //[nref]
   Float_t         AK5Z2_jteta[117];   //[nref]
   Float_t         AK5Z2_jty[117];   //[nref]
   Float_t         AK5Z2_jtphi[117];   //[nref]
   Float_t         AK5Z2_jtpu[117];   //[nref]
   Float_t         AK5Z2_jtm[117];   //[nref]
   Float_t         AK5Z2_jtarea[117];   //[nref]
   Int_t           AK5Z2_jtdyn_split[117];   //[nref]
   Float_t         AK5Z2_jtdyn_eta[117];   //[nref]
   Float_t         AK5Z2_jtdyn_phi[117];   //[nref]
   Float_t         AK5Z2_jtdyn_deltaR[117];   //[nref]
   Float_t         AK5Z2_jtdyn_kt[117];   //[nref]
   Float_t         AK5Z2_jtdyn_z[117];   //[nref]
   Int_t           AK5Z2_jt_intjet_multi[117];   //[nref]
   Float_t         AK5Z2_jt_girth[117];   //[nref]
   Float_t         AK5Z2_jt_thrust[117];   //[nref]
   Float_t         AK5Z2_jt_LHA[117];   //[nref]
   Float_t         AK5Z2_jt_pTD[117];   //[nref]
   Int_t           AK5Z3_run;
   Int_t           AK5Z3_evt;
   Int_t           AK5Z3_lumi;
   Int_t           AK5Z3_nref;
   Int_t           AK5Z3_ncalo;
   Float_t         AK5Z3_rawpt[117];   //[nref]
   Float_t         AK5Z3_jtpt[117];   //[nref]
   Float_t         AK5Z3_jteta[117];   //[nref]
   Float_t         AK5Z3_jty[117];   //[nref]
   Float_t         AK5Z3_jtphi[117];   //[nref]
   Float_t         AK5Z3_jtpu[117];   //[nref]
   Float_t         AK5Z3_jtm[117];   //[nref]
   Float_t         AK5Z3_jtarea[117];   //[nref]
   Int_t           AK5Z3_jtdyn_split[117];   //[nref]
   Float_t         AK5Z3_jtdyn_eta[117];   //[nref]
   Float_t         AK5Z3_jtdyn_phi[117];   //[nref]
   Float_t         AK5Z3_jtdyn_deltaR[117];   //[nref]
   Float_t         AK5Z3_jtdyn_kt[117];   //[nref]
   Float_t         AK5Z3_jtdyn_z[117];   //[nref]
   Int_t           AK5Z3_jt_intjet_multi[117];   //[nref]
   Float_t         AK5Z3_jt_girth[117];   //[nref]
   Float_t         AK5Z3_jt_thrust[117];   //[nref]
   Float_t         AK5Z3_jt_LHA[117];   //[nref]
   Float_t         AK5Z3_jt_pTD[117];   //[nref]
   Int_t           AK5Z4_run;
   Int_t           AK5Z4_evt;
   Int_t           AK5Z4_lumi;
   Int_t           AK5Z4_nref;
   Int_t           AK5Z4_ncalo;
   Float_t         AK5Z4_rawpt[117];   //[nref]
   Float_t         AK5Z4_jtpt[117];   //[nref]
   Float_t         AK5Z4_jteta[117];   //[nref]
   Float_t         AK5Z4_jty[117];   //[nref]
   Float_t         AK5Z4_jtphi[117];   //[nref]
   Float_t         AK5Z4_jtpu[117];   //[nref]
   Float_t         AK5Z4_jtm[117];   //[nref]
   Float_t         AK5Z4_jtarea[117];   //[nref]
   Int_t           AK5Z4_jtdyn_split[117];   //[nref]
   Float_t         AK5Z4_jtdyn_eta[117];   //[nref]
   Float_t         AK5Z4_jtdyn_phi[117];   //[nref]
   Float_t         AK5Z4_jtdyn_deltaR[117];   //[nref]
   Float_t         AK5Z4_jtdyn_kt[117];   //[nref]
   Float_t         AK5Z4_jtdyn_z[117];   //[nref]
   Int_t           AK5Z4_jt_intjet_multi[117];   //[nref]
   Float_t         AK5Z4_jt_girth[117];   //[nref]
   Float_t         AK5Z4_jt_thrust[117];   //[nref]
   Float_t         AK5Z4_jt_LHA[117];   //[nref]
   Float_t         AK5Z4_jt_pTD[117];   //[nref]
   Int_t           AK5Z5_run;
   Int_t           AK5Z5_evt;
   Int_t           AK5Z5_lumi;
   Int_t           AK5Z5_nref;
   Int_t           AK5Z5_ncalo;
   Float_t         AK5Z5_rawpt[117];   //[nref]
   Float_t         AK5Z5_jtpt[117];   //[nref]
   Float_t         AK5Z5_jteta[117];   //[nref]
   Float_t         AK5Z5_jty[117];   //[nref]
   Float_t         AK5Z5_jtphi[117];   //[nref]
   Float_t         AK5Z5_jtpu[117];   //[nref]
   Float_t         AK5Z5_jtm[117];   //[nref]
   Float_t         AK5Z5_jtarea[117];   //[nref]
   Int_t           AK5Z5_jtdyn_split[117];   //[nref]
   Float_t         AK5Z5_jtdyn_eta[117];   //[nref]
   Float_t         AK5Z5_jtdyn_phi[117];   //[nref]
   Float_t         AK5Z5_jtdyn_deltaR[117];   //[nref]
   Float_t         AK5Z5_jtdyn_kt[117];   //[nref]
   Float_t         AK5Z5_jtdyn_z[117];   //[nref]
   Int_t           AK5Z5_jt_intjet_multi[117];   //[nref]
   Float_t         AK5Z5_jt_girth[117];   //[nref]
   Float_t         AK5Z5_jt_thrust[117];   //[nref]
   Float_t         AK5Z5_jt_LHA[117];   //[nref]
   Float_t         AK5Z5_jt_pTD[117];   //[nref]
   Int_t           AK6Z1_run;
   Int_t           AK6Z1_evt;
   Int_t           AK6Z1_lumi;
   Int_t           AK6Z1_nref;
   Int_t           AK6Z1_ncalo;
   Float_t         AK6Z1_rawpt[117];   //[nref]
   Float_t         AK6Z1_jtpt[117];   //[nref]
   Float_t         AK6Z1_jteta[117];   //[nref]
   Float_t         AK6Z1_jty[117];   //[nref]
   Float_t         AK6Z1_jtphi[117];   //[nref]
   Float_t         AK6Z1_jtpu[117];   //[nref]
   Float_t         AK6Z1_jtm[117];   //[nref]
   Float_t         AK6Z1_jtarea[117];   //[nref]
   Int_t           AK6Z1_jtdyn_split[117];   //[nref]
   Float_t         AK6Z1_jtdyn_eta[117];   //[nref]
   Float_t         AK6Z1_jtdyn_phi[117];   //[nref]
   Float_t         AK6Z1_jtdyn_deltaR[117];   //[nref]
   Float_t         AK6Z1_jtdyn_kt[117];   //[nref]
   Float_t         AK6Z1_jtdyn_z[117];   //[nref]
   Int_t           AK6Z1_jt_intjet_multi[117];   //[nref]
   Float_t         AK6Z1_jt_girth[117];   //[nref]
   Float_t         AK6Z1_jt_thrust[117];   //[nref]
   Float_t         AK6Z1_jt_LHA[117];   //[nref]
   Float_t         AK6Z1_jt_pTD[117];   //[nref]
   Int_t           AK6Z2_run;
   Int_t           AK6Z2_evt;
   Int_t           AK6Z2_lumi;
   Int_t           AK6Z2_nref;
   Int_t           AK6Z2_ncalo;
   Float_t         AK6Z2_rawpt[117];   //[nref]
   Float_t         AK6Z2_jtpt[117];   //[nref]
   Float_t         AK6Z2_jteta[117];   //[nref]
   Float_t         AK6Z2_jty[117];   //[nref]
   Float_t         AK6Z2_jtphi[117];   //[nref]
   Float_t         AK6Z2_jtpu[117];   //[nref]
   Float_t         AK6Z2_jtm[117];   //[nref]
   Float_t         AK6Z2_jtarea[117];   //[nref]
   Int_t           AK6Z2_jtdyn_split[117];   //[nref]
   Float_t         AK6Z2_jtdyn_eta[117];   //[nref]
   Float_t         AK6Z2_jtdyn_phi[117];   //[nref]
   Float_t         AK6Z2_jtdyn_deltaR[117];   //[nref]
   Float_t         AK6Z2_jtdyn_kt[117];   //[nref]
   Float_t         AK6Z2_jtdyn_z[117];   //[nref]
   Int_t           AK6Z2_jt_intjet_multi[117];   //[nref]
   Float_t         AK6Z2_jt_girth[117];   //[nref]
   Float_t         AK6Z2_jt_thrust[117];   //[nref]
   Float_t         AK6Z2_jt_LHA[117];   //[nref]
   Float_t         AK6Z2_jt_pTD[117];   //[nref]
   Int_t           AK6Z3_run;
   Int_t           AK6Z3_evt;
   Int_t           AK6Z3_lumi;
   Int_t           AK6Z3_nref;
   Int_t           AK6Z3_ncalo;
   Float_t         AK6Z3_rawpt[117];   //[nref]
   Float_t         AK6Z3_jtpt[117];   //[nref]
   Float_t         AK6Z3_jteta[117];   //[nref]
   Float_t         AK6Z3_jty[117];   //[nref]
   Float_t         AK6Z3_jtphi[117];   //[nref]
   Float_t         AK6Z3_jtpu[117];   //[nref]
   Float_t         AK6Z3_jtm[117];   //[nref]
   Float_t         AK6Z3_jtarea[117];   //[nref]
   Int_t           AK6Z3_jtdyn_split[117];   //[nref]
   Float_t         AK6Z3_jtdyn_eta[117];   //[nref]
   Float_t         AK6Z3_jtdyn_phi[117];   //[nref]
   Float_t         AK6Z3_jtdyn_deltaR[117];   //[nref]
   Float_t         AK6Z3_jtdyn_kt[117];   //[nref]
   Float_t         AK6Z3_jtdyn_z[117];   //[nref]
   Int_t           AK6Z3_jt_intjet_multi[117];   //[nref]
   Float_t         AK6Z3_jt_girth[117];   //[nref]
   Float_t         AK6Z3_jt_thrust[117];   //[nref]
   Float_t         AK6Z3_jt_LHA[117];   //[nref]
   Float_t         AK6Z3_jt_pTD[117];   //[nref]
   Int_t           AK6Z4_run;
   Int_t           AK6Z4_evt;
   Int_t           AK6Z4_lumi;
   Int_t           AK6Z4_nref;
   Int_t           AK6Z4_ncalo;
   Float_t         AK6Z4_rawpt[117];   //[nref]
   Float_t         AK6Z4_jtpt[117];   //[nref]
   Float_t         AK6Z4_jteta[117];   //[nref]
   Float_t         AK6Z4_jty[117];   //[nref]
   Float_t         AK6Z4_jtphi[117];   //[nref]
   Float_t         AK6Z4_jtpu[117];   //[nref]
   Float_t         AK6Z4_jtm[117];   //[nref]
   Float_t         AK6Z4_jtarea[117];   //[nref]
   Int_t           AK6Z4_jtdyn_split[117];   //[nref]
   Float_t         AK6Z4_jtdyn_eta[117];   //[nref]
   Float_t         AK6Z4_jtdyn_phi[117];   //[nref]
   Float_t         AK6Z4_jtdyn_deltaR[117];   //[nref]
   Float_t         AK6Z4_jtdyn_kt[117];   //[nref]
   Float_t         AK6Z4_jtdyn_z[117];   //[nref]
   Int_t           AK6Z4_jt_intjet_multi[117];   //[nref]
   Float_t         AK6Z4_jt_girth[117];   //[nref]
   Float_t         AK6Z4_jt_thrust[117];   //[nref]
   Float_t         AK6Z4_jt_LHA[117];   //[nref]
   Float_t         AK6Z4_jt_pTD[117];   //[nref]
   Int_t           AK6Z5_run;
   Int_t           AK6Z5_evt;
   Int_t           AK6Z5_lumi;
   Int_t           AK6Z5_nref;
   Int_t           AK6Z5_ncalo;
   Float_t         AK6Z5_rawpt[117];   //[nref]
   Float_t         AK6Z5_jtpt[117];   //[nref]
   Float_t         AK6Z5_jteta[117];   //[nref]
   Float_t         AK6Z5_jty[117];   //[nref]
   Float_t         AK6Z5_jtphi[117];   //[nref]
   Float_t         AK6Z5_jtpu[117];   //[nref]
   Float_t         AK6Z5_jtm[117];   //[nref]
   Float_t         AK6Z5_jtarea[117];   //[nref]
   Int_t           AK6Z5_jtdyn_split[117];   //[nref]
   Float_t         AK6Z5_jtdyn_eta[117];   //[nref]
   Float_t         AK6Z5_jtdyn_phi[117];   //[nref]
   Float_t         AK6Z5_jtdyn_deltaR[117];   //[nref]
   Float_t         AK6Z5_jtdyn_kt[117];   //[nref]
   Float_t         AK6Z5_jtdyn_z[117];   //[nref]
   Int_t           AK6Z5_jt_intjet_multi[117];   //[nref]
   Float_t         AK6Z5_jt_girth[117];   //[nref]
   Float_t         AK6Z5_jt_thrust[117];   //[nref]
   Float_t         AK6Z5_jt_LHA[117];   //[nref]
   Float_t         AK6Z5_jt_pTD[117];   //[nref]
   Int_t           AK8Z1_run;
   Int_t           AK8Z1_evt;
   Int_t           AK8Z1_lumi;
   Int_t           AK8Z1_nref;
   Int_t           AK8Z1_ncalo;
   Float_t         AK8Z1_rawpt[117];   //[nref]
   Float_t         AK8Z1_jtpt[117];   //[nref]
   Float_t         AK8Z1_jteta[117];   //[nref]
   Float_t         AK8Z1_jty[117];   //[nref]
   Float_t         AK8Z1_jtphi[117];   //[nref]
   Float_t         AK8Z1_jtpu[117];   //[nref]
   Float_t         AK8Z1_jtm[117];   //[nref]
   Float_t         AK8Z1_jtarea[117];   //[nref]
   Int_t           AK8Z1_jtdyn_split[117];   //[nref]
   Float_t         AK8Z1_jtdyn_eta[117];   //[nref]
   Float_t         AK8Z1_jtdyn_phi[117];   //[nref]
   Float_t         AK8Z1_jtdyn_deltaR[117];   //[nref]
   Float_t         AK8Z1_jtdyn_kt[117];   //[nref]
   Float_t         AK8Z1_jtdyn_z[117];   //[nref]
   Int_t           AK8Z1_jt_intjet_multi[117];   //[nref]
   Float_t         AK8Z1_jt_girth[117];   //[nref]
   Float_t         AK8Z1_jt_thrust[117];   //[nref]
   Float_t         AK8Z1_jt_LHA[117];   //[nref]
   Float_t         AK8Z1_jt_pTD[117];   //[nref]
   Int_t           AK8Z2_run;
   Int_t           AK8Z2_evt;
   Int_t           AK8Z2_lumi;
   Int_t           AK8Z2_nref;
   Int_t           AK8Z2_ncalo;
   Float_t         AK8Z2_rawpt[117];   //[nref]
   Float_t         AK8Z2_jtpt[117];   //[nref]
   Float_t         AK8Z2_jteta[117];   //[nref]
   Float_t         AK8Z2_jty[117];   //[nref]
   Float_t         AK8Z2_jtphi[117];   //[nref]
   Float_t         AK8Z2_jtpu[117];   //[nref]
   Float_t         AK8Z2_jtm[117];   //[nref]
   Float_t         AK8Z2_jtarea[117];   //[nref]
   Int_t           AK8Z2_jtdyn_split[117];   //[nref]
   Float_t         AK8Z2_jtdyn_eta[117];   //[nref]
   Float_t         AK8Z2_jtdyn_phi[117];   //[nref]
   Float_t         AK8Z2_jtdyn_deltaR[117];   //[nref]
   Float_t         AK8Z2_jtdyn_kt[117];   //[nref]
   Float_t         AK8Z2_jtdyn_z[117];   //[nref]
   Int_t           AK8Z2_jt_intjet_multi[117];   //[nref]
   Float_t         AK8Z2_jt_girth[117];   //[nref]
   Float_t         AK8Z2_jt_thrust[117];   //[nref]
   Float_t         AK8Z2_jt_LHA[117];   //[nref]
   Float_t         AK8Z2_jt_pTD[117];   //[nref]
   Int_t           AK8Z3_run;
   Int_t           AK8Z3_evt;
   Int_t           AK8Z3_lumi;
   Int_t           AK8Z3_nref;
   Int_t           AK8Z3_ncalo;
   Float_t         AK8Z3_rawpt[117];   //[nref]
   Float_t         AK8Z3_jtpt[117];   //[nref]
   Float_t         AK8Z3_jteta[117];   //[nref]
   Float_t         AK8Z3_jty[117];   //[nref]
   Float_t         AK8Z3_jtphi[117];   //[nref]
   Float_t         AK8Z3_jtpu[117];   //[nref]
   Float_t         AK8Z3_jtm[117];   //[nref]
   Float_t         AK8Z3_jtarea[117];   //[nref]
   Int_t           AK8Z3_jtdyn_split[117];   //[nref]
   Float_t         AK8Z3_jtdyn_eta[117];   //[nref]
   Float_t         AK8Z3_jtdyn_phi[117];   //[nref]
   Float_t         AK8Z3_jtdyn_deltaR[117];   //[nref]
   Float_t         AK8Z3_jtdyn_kt[117];   //[nref]
   Float_t         AK8Z3_jtdyn_z[117];   //[nref]
   Int_t           AK8Z3_jt_intjet_multi[117];   //[nref]
   Float_t         AK8Z3_jt_girth[117];   //[nref]
   Float_t         AK8Z3_jt_thrust[117];   //[nref]
   Float_t         AK8Z3_jt_LHA[117];   //[nref]
   Float_t         AK8Z3_jt_pTD[117];   //[nref]
   Int_t           AK8Z4_run;
   Int_t           AK8Z4_evt;
   Int_t           AK8Z4_lumi;
   Int_t           AK8Z4_nref;
   Int_t           AK8Z4_ncalo;
   Float_t         AK8Z4_rawpt[117];   //[nref]
   Float_t         AK8Z4_jtpt[117];   //[nref]
   Float_t         AK8Z4_jteta[117];   //[nref]
   Float_t         AK8Z4_jty[117];   //[nref]
   Float_t         AK8Z4_jtphi[117];   //[nref]
   Float_t         AK8Z4_jtpu[117];   //[nref]
   Float_t         AK8Z4_jtm[117];   //[nref]
   Float_t         AK8Z4_jtarea[117];   //[nref]
   Int_t           AK8Z4_jtdyn_split[117];   //[nref]
   Float_t         AK8Z4_jtdyn_eta[117];   //[nref]
   Float_t         AK8Z4_jtdyn_phi[117];   //[nref]
   Float_t         AK8Z4_jtdyn_deltaR[117];   //[nref]
   Float_t         AK8Z4_jtdyn_kt[117];   //[nref]
   Float_t         AK8Z4_jtdyn_z[117];   //[nref]
   Int_t           AK8Z4_jt_intjet_multi[117];   //[nref]
   Float_t         AK8Z4_jt_girth[117];   //[nref]
   Float_t         AK8Z4_jt_thrust[117];   //[nref]
   Float_t         AK8Z4_jt_LHA[117];   //[nref]
   Float_t         AK8Z4_jt_pTD[117];   //[nref]
   Int_t           AK8Z5_run;
   Int_t           AK8Z5_evt;
   Int_t           AK8Z5_lumi;
   Int_t           AK8Z5_nref;
   Int_t           AK8Z5_ncalo;
   Float_t         AK8Z5_rawpt[117];   //[nref]
   Float_t         AK8Z5_jtpt[117];   //[nref]
   Float_t         AK8Z5_jteta[117];   //[nref]
   Float_t         AK8Z5_jty[117];   //[nref]
   Float_t         AK8Z5_jtphi[117];   //[nref]
   Float_t         AK8Z5_jtpu[117];   //[nref]
   Float_t         AK8Z5_jtm[117];   //[nref]
   Float_t         AK8Z5_jtarea[117];   //[nref]
   Int_t           AK8Z5_jtdyn_split[117];   //[nref]
   Float_t         AK8Z5_jtdyn_eta[117];   //[nref]
   Float_t         AK8Z5_jtdyn_phi[117];   //[nref]
   Float_t         AK8Z5_jtdyn_deltaR[117];   //[nref]
   Float_t         AK8Z5_jtdyn_kt[117];   //[nref]
   Float_t         AK8Z5_jtdyn_z[117];   //[nref]
   Int_t           AK8Z5_jt_intjet_multi[117];   //[nref]
   Float_t         AK8Z5_jt_girth[117];   //[nref]
   Float_t         AK8Z5_jt_thrust[117];   //[nref]
   Float_t         AK8Z5_jt_LHA[117];   //[nref]
   Float_t         AK8Z5_jt_pTD[117];   //[nref]
   UInt_t          ggHi_run;
   ULong64_t       ggHi_event;
   UInt_t          ggHi_lumis;
   Float_t         ggHi_rho;
   Int_t           ggHi_nEle;
   ROOT::VecOps::RVec<float> *ggHi_eleD0;
   ROOT::VecOps::RVec<float> *ggHi_eleDz;
   ROOT::VecOps::RVec<float> *ggHi_eleD0Err;
   ROOT::VecOps::RVec<float> *ggHi_eleDzErr;
   ROOT::VecOps::RVec<float> *ggHi_eleTrkPt;
   ROOT::VecOps::RVec<float> *ggHi_eleTrkEta;
   ROOT::VecOps::RVec<float> *ggHi_eleTrkPhi;
   ROOT::VecOps::RVec<int> *ggHi_eleTrkCharge;
   ROOT::VecOps::RVec<float> *ggHi_eleTrkPtErr;
   ROOT::VecOps::RVec<float> *ggHi_eleTrkChi2;
   ROOT::VecOps::RVec<float> *ggHi_eleTrkNdof;
   ROOT::VecOps::RVec<float> *ggHi_eleTrkNormalizedChi2;
   ROOT::VecOps::RVec<int> *ggHi_eleTrkValidHits;
   ROOT::VecOps::RVec<int> *ggHi_eleTrkLayers;
   ROOT::VecOps::RVec<int> *ggHi_eleMissHits;
   ROOT::VecOps::RVec<float> *ggHi_eleIP3D;
   ROOT::VecOps::RVec<float> *ggHi_eleIP3DErr;
   ROOT::VecOps::RVec<float> *ggHi_elePt;
   ROOT::VecOps::RVec<float> *ggHi_eleEta;
   ROOT::VecOps::RVec<float> *ggHi_elePhi;
   ROOT::VecOps::RVec<int> *ggHi_eleCharge;
   ROOT::VecOps::RVec<float> *ggHi_eleEn;
   ROOT::VecOps::RVec<float> *ggHi_eleSCEn;
   ROOT::VecOps::RVec<float> *ggHi_eleESEn;
   ROOT::VecOps::RVec<float> *ggHi_eleSCEta;
   ROOT::VecOps::RVec<float> *ggHi_eleSCPhi;
   ROOT::VecOps::RVec<float> *ggHi_eleSCRawEn;
   ROOT::VecOps::RVec<float> *ggHi_eleSCEtaWidth;
   ROOT::VecOps::RVec<float> *ggHi_eleSCPhiWidth;
   ROOT::VecOps::RVec<int> *ggHi_eleSCClustersSize;
   ROOT::VecOps::RVec<float> *ggHi_eleSeedEn;
   ROOT::VecOps::RVec<float> *ggHi_eleSeedEta;
   ROOT::VecOps::RVec<float> *ggHi_eleSeedPhi;
   ROOT::VecOps::RVec<float> *ggHi_eleHoverE;
   ROOT::VecOps::RVec<float> *ggHi_eleHoverEBc;
   ROOT::VecOps::RVec<float> *ggHi_eleEoverP;
   ROOT::VecOps::RVec<float> *ggHi_eleEoverPInv;
   ROOT::VecOps::RVec<float> *ggHi_eleEcalE;
   ROOT::VecOps::RVec<float> *ggHi_elePAtVtx;
   ROOT::VecOps::RVec<float> *ggHi_elePAtSC;
   ROOT::VecOps::RVec<float> *ggHi_elePAtCluster;
   ROOT::VecOps::RVec<float> *ggHi_elePAtSeed;
   ROOT::VecOps::RVec<float> *ggHi_eledEtaAtVtx;
   ROOT::VecOps::RVec<float> *ggHi_eledPhiAtVtx;
   ROOT::VecOps::RVec<float> *ggHi_eledEtaSeedAtVtx;
   ROOT::VecOps::RVec<float> *ggHi_eleSigmaIEtaIEta;
   ROOT::VecOps::RVec<float> *ggHi_eleSigmaIPhiIPhi;
   ROOT::VecOps::RVec<float> *ggHi_eleBrem;
   ROOT::VecOps::RVec<int> *ggHi_eleConvVeto;
   ROOT::VecOps::RVec<float> *ggHi_eleR9;
   ROOT::VecOps::RVec<float> *ggHi_eleE3x3;
   ROOT::VecOps::RVec<float> *ggHi_eleE5x5;
   ROOT::VecOps::RVec<float> *ggHi_eleR9Full5x5;
   ROOT::VecOps::RVec<float> *ggHi_eleE3x3Full5x5;
   ROOT::VecOps::RVec<float> *ggHi_eleE5x5Full5x5;
   ROOT::VecOps::RVec<float> *ggHi_eleSigmaIEtaIEta_2012;
   ROOT::VecOps::RVec<float> *ggHi_elePFChIso;
   ROOT::VecOps::RVec<float> *ggHi_elePFPhoIso;
   ROOT::VecOps::RVec<float> *ggHi_elePFNeuIso;
   ROOT::VecOps::RVec<float> *ggHi_elePFPUIso;
   ROOT::VecOps::RVec<float> *ggHi_elePFRelIsoWithEA;
   ROOT::VecOps::RVec<float> *ggHi_elePFRelIsoWithDBeta;
   ROOT::VecOps::RVec<float> *ggHi_eleEffAreaTimesRho;
   ROOT::VecOps::RVec<float> *ggHi_elePFChIso03;
   ROOT::VecOps::RVec<float> *ggHi_elePFPhoIso03;
   ROOT::VecOps::RVec<float> *ggHi_elePFNeuIso03;
   ROOT::VecOps::RVec<float> *ggHi_elePFChIso04;
   ROOT::VecOps::RVec<float> *ggHi_elePFPhoIso04;
   ROOT::VecOps::RVec<float> *ggHi_elePFNeuIso04;
   ROOT::VecOps::RVec<float> *ggHi_eleSeedCryEta;
   ROOT::VecOps::RVec<float> *ggHi_eleSeedCryPhi;
   ROOT::VecOps::RVec<float> *ggHi_eleSeedCryIeta;
   ROOT::VecOps::RVec<float> *ggHi_eleSeedCryIphi;
   Int_t           ggHi_nPho;
   ROOT::VecOps::RVec<float> *ggHi_phoE;
   ROOT::VecOps::RVec<float> *ggHi_phoEt;
   ROOT::VecOps::RVec<float> *ggHi_phoEta;
   ROOT::VecOps::RVec<float> *ggHi_phoPhi;
   ROOT::VecOps::RVec<float> *ggHi_phoEcorrStdEcal;
   ROOT::VecOps::RVec<float> *ggHi_phoEcorrPhoEcal;
   ROOT::VecOps::RVec<float> *ggHi_phoEcorrRegr1;
   ROOT::VecOps::RVec<float> *ggHi_phoEcorrRegr2;
   ROOT::VecOps::RVec<float> *ggHi_phoEcorrErrStdEcal;
   ROOT::VecOps::RVec<float> *ggHi_phoEcorrErrPhoEcal;
   ROOT::VecOps::RVec<float> *ggHi_phoEcorrErrRegr1;
   ROOT::VecOps::RVec<float> *ggHi_phoEcorrErrRegr2;
   ROOT::VecOps::RVec<float> *ggHi_phoSCE;
   ROOT::VecOps::RVec<float> *ggHi_phoSCRawE;
   ROOT::VecOps::RVec<float> *ggHi_phoSCEta;
   ROOT::VecOps::RVec<float> *ggHi_phoSCPhi;
   ROOT::VecOps::RVec<float> *ggHi_phoSCEtaWidth;
   ROOT::VecOps::RVec<float> *ggHi_phoSCPhiWidth;
   ROOT::VecOps::RVec<float> *ggHi_phoSCBrem;
   ROOT::VecOps::RVec<int> *ggHi_phoSCnHits;
   ROOT::VecOps::RVec<unsigned int> *ggHi_phoSCflags;
   ROOT::VecOps::RVec<int> *ggHi_phoSCinClean;
   ROOT::VecOps::RVec<int> *ggHi_phoSCinUnClean;
   ROOT::VecOps::RVec<int> *ggHi_phoSCnBC;
   ROOT::VecOps::RVec<float> *ggHi_phoESEn;
   ROOT::VecOps::RVec<int> *ggHi_phoIsPFPhoton;
   ROOT::VecOps::RVec<int> *ggHi_phoIsStandardPhoton;
   ROOT::VecOps::RVec<int> *ggHi_phoHasPixelSeed;
   ROOT::VecOps::RVec<int> *ggHi_phoHasConversionTracks;
   ROOT::VecOps::RVec<float> *ggHi_phoHadTowerOverEm;
   ROOT::VecOps::RVec<float> *ggHi_phoHoverE;
   ROOT::VecOps::RVec<int> *ggHi_phoHoverEValid;
   ROOT::VecOps::RVec<float> *ggHi_phoSigmaIEtaIEta;
   ROOT::VecOps::RVec<float> *ggHi_phoR9;
   ROOT::VecOps::RVec<float> *ggHi_phoE1x5;
   ROOT::VecOps::RVec<float> *ggHi_phoE2x5;
   ROOT::VecOps::RVec<float> *ggHi_phoE3x3;
   ROOT::VecOps::RVec<float> *ggHi_phoE5x5;
   ROOT::VecOps::RVec<float> *ggHi_phoMaxEnergyXtal;
   ROOT::VecOps::RVec<float> *ggHi_phoSigmaEtaEta;
   ROOT::VecOps::RVec<float> *ggHi_phoSigmaIEtaIEta_2012;
   ROOT::VecOps::RVec<float> *ggHi_phoR9_2012;
   ROOT::VecOps::RVec<float> *ggHi_phoE1x5_2012;
   ROOT::VecOps::RVec<float> *ggHi_phoE2x5_2012;
   ROOT::VecOps::RVec<float> *ggHi_phoE3x3_2012;
   ROOT::VecOps::RVec<float> *ggHi_phoE5x5_2012;
   ROOT::VecOps::RVec<float> *ggHi_phoMaxEnergyXtal_2012;
   ROOT::VecOps::RVec<float> *ggHi_phoSigmaEtaEta_2012;
   ROOT::VecOps::RVec<float> *ggHi_phoHadTowerOverEm1;
   ROOT::VecOps::RVec<float> *ggHi_phoHadTowerOverEm2;
   ROOT::VecOps::RVec<float> *ggHi_phoHoverE1;
   ROOT::VecOps::RVec<float> *ggHi_phoHoverE2;
   ROOT::VecOps::RVec<float> *ggHi_phoSigmaIEtaIPhi;
   ROOT::VecOps::RVec<float> *ggHi_phoSigmaIPhiIPhi;
   ROOT::VecOps::RVec<float> *ggHi_phoR1x5;
   ROOT::VecOps::RVec<float> *ggHi_phoR2x5;
   ROOT::VecOps::RVec<float> *ggHi_phoE2nd;
   ROOT::VecOps::RVec<float> *ggHi_phoETop;
   ROOT::VecOps::RVec<float> *ggHi_phoEBottom;
   ROOT::VecOps::RVec<float> *ggHi_phoELeft;
   ROOT::VecOps::RVec<float> *ggHi_phoERight;
   ROOT::VecOps::RVec<float> *ggHi_phoE1x3;
   ROOT::VecOps::RVec<float> *ggHi_phoE2x2;
   ROOT::VecOps::RVec<float> *ggHi_phoE2x5Max;
   ROOT::VecOps::RVec<float> *ggHi_phoE2x5Top;
   ROOT::VecOps::RVec<float> *ggHi_phoE2x5Bottom;
   ROOT::VecOps::RVec<float> *ggHi_phoE2x5Left;
   ROOT::VecOps::RVec<float> *ggHi_phoE2x5Right;
   ROOT::VecOps::RVec<float> *ggHi_phoSigmaIEtaIPhi_2012;
   ROOT::VecOps::RVec<float> *ggHi_phoSigmaIPhiIPhi_2012;
   ROOT::VecOps::RVec<float> *ggHi_phoR1x5_2012;
   ROOT::VecOps::RVec<float> *ggHi_phoR2x5_2012;
   ROOT::VecOps::RVec<float> *ggHi_phoE2nd_2012;
   ROOT::VecOps::RVec<float> *ggHi_phoETop_2012;
   ROOT::VecOps::RVec<float> *ggHi_phoEBottom_2012;
   ROOT::VecOps::RVec<float> *ggHi_phoELeft_2012;
   ROOT::VecOps::RVec<float> *ggHi_phoERight_2012;
   ROOT::VecOps::RVec<float> *ggHi_phoE1x3_2012;
   ROOT::VecOps::RVec<float> *ggHi_phoE2x2_2012;
   ROOT::VecOps::RVec<float> *ggHi_phoE2x5Max_2012;
   ROOT::VecOps::RVec<float> *ggHi_phoE2x5Top_2012;
   ROOT::VecOps::RVec<float> *ggHi_phoE2x5Bottom_2012;
   ROOT::VecOps::RVec<float> *ggHi_phoE2x5Left_2012;
   ROOT::VecOps::RVec<float> *ggHi_phoE2x5Right_2012;
   ROOT::VecOps::RVec<float> *ggHi_phoBC1E;
   ROOT::VecOps::RVec<float> *ggHi_phoBC1Ecorr;
   ROOT::VecOps::RVec<float> *ggHi_phoBC1Eta;
   ROOT::VecOps::RVec<float> *ggHi_phoBC1Phi;
   ROOT::VecOps::RVec<int> *ggHi_phoBC1size;
   ROOT::VecOps::RVec<unsigned int> *ggHi_phoBC1flags;
   ROOT::VecOps::RVec<int> *ggHi_phoBC1inClean;
   ROOT::VecOps::RVec<int> *ggHi_phoBC1inUnClean;
   ROOT::VecOps::RVec<unsigned int> *ggHi_phoBC1rawID;
   ROOT::VecOps::RVec<float> *ggHi_pho_ecalClusterIsoR1;
   ROOT::VecOps::RVec<float> *ggHi_pho_ecalClusterIsoR2;
   ROOT::VecOps::RVec<float> *ggHi_pho_ecalClusterIsoR3;
   ROOT::VecOps::RVec<float> *ggHi_pho_ecalClusterIsoR4;
   ROOT::VecOps::RVec<float> *ggHi_pho_ecalClusterIsoR5;
   ROOT::VecOps::RVec<float> *ggHi_pho_hcalRechitIsoR1;
   ROOT::VecOps::RVec<float> *ggHi_pho_hcalRechitIsoR2;
   ROOT::VecOps::RVec<float> *ggHi_pho_hcalRechitIsoR3;
   ROOT::VecOps::RVec<float> *ggHi_pho_hcalRechitIsoR4;
   ROOT::VecOps::RVec<float> *ggHi_pho_hcalRechitIsoR5;
   ROOT::VecOps::RVec<float> *ggHi_pho_trackIsoR1PtCut20;
   ROOT::VecOps::RVec<float> *ggHi_pho_trackIsoR2PtCut20;
   ROOT::VecOps::RVec<float> *ggHi_pho_trackIsoR3PtCut20;
   ROOT::VecOps::RVec<float> *ggHi_pho_trackIsoR4PtCut20;
   ROOT::VecOps::RVec<float> *ggHi_pho_trackIsoR5PtCut20;
   ROOT::VecOps::RVec<float> *ggHi_pho_swissCrx;
   ROOT::VecOps::RVec<float> *ggHi_pho_seedTime;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso1;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso2;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso3;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso4;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso5;
   ROOT::VecOps::RVec<float> *ggHi_pfpIso1;
   ROOT::VecOps::RVec<float> *ggHi_pfpIso2;
   ROOT::VecOps::RVec<float> *ggHi_pfpIso3;
   ROOT::VecOps::RVec<float> *ggHi_pfpIso4;
   ROOT::VecOps::RVec<float> *ggHi_pfpIso5;
   ROOT::VecOps::RVec<float> *ggHi_pfnIso1;
   ROOT::VecOps::RVec<float> *ggHi_pfnIso2;
   ROOT::VecOps::RVec<float> *ggHi_pfnIso3;
   ROOT::VecOps::RVec<float> *ggHi_pfnIso4;
   ROOT::VecOps::RVec<float> *ggHi_pfnIso5;
   ROOT::VecOps::RVec<float> *ggHi_pfpIso1subSC;
   ROOT::VecOps::RVec<float> *ggHi_pfpIso2subSC;
   ROOT::VecOps::RVec<float> *ggHi_pfpIso3subSC;
   ROOT::VecOps::RVec<float> *ggHi_pfpIso4subSC;
   ROOT::VecOps::RVec<float> *ggHi_pfpIso5subSC;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso1subUE;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso2subUE;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso3subUE;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso4subUE;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso5subUE;
   ROOT::VecOps::RVec<float> *ggHi_pfpIso1subUE;
   ROOT::VecOps::RVec<float> *ggHi_pfpIso2subUE;
   ROOT::VecOps::RVec<float> *ggHi_pfpIso3subUE;
   ROOT::VecOps::RVec<float> *ggHi_pfpIso4subUE;
   ROOT::VecOps::RVec<float> *ggHi_pfpIso5subUE;
   ROOT::VecOps::RVec<float> *ggHi_pfnIso1subUE;
   ROOT::VecOps::RVec<float> *ggHi_pfnIso2subUE;
   ROOT::VecOps::RVec<float> *ggHi_pfnIso3subUE;
   ROOT::VecOps::RVec<float> *ggHi_pfnIso4subUE;
   ROOT::VecOps::RVec<float> *ggHi_pfnIso5subUE;
   ROOT::VecOps::RVec<float> *ggHi_pfpIso1subSCsubUE;
   ROOT::VecOps::RVec<float> *ggHi_pfpIso2subSCsubUE;
   ROOT::VecOps::RVec<float> *ggHi_pfpIso3subSCsubUE;
   ROOT::VecOps::RVec<float> *ggHi_pfpIso4subSCsubUE;
   ROOT::VecOps::RVec<float> *ggHi_pfpIso5subSCsubUE;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso1pTgt1p0subUE;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso2pTgt1p0subUE;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso3pTgt1p0subUE;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso4pTgt1p0subUE;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso5pTgt1p0subUE;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso1pTgt2p0subUE;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso2pTgt2p0subUE;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso3pTgt2p0subUE;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso4pTgt2p0subUE;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso5pTgt2p0subUE;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso1pTgt3p0subUE;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso2pTgt3p0subUE;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso3pTgt3p0subUE;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso4pTgt3p0subUE;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso5pTgt3p0subUE;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso2subUEec;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso3subUEec;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso4subUEec;
   ROOT::VecOps::RVec<float> *ggHi_pfpIso2subUEec;
   ROOT::VecOps::RVec<float> *ggHi_pfpIso3subUEec;
   ROOT::VecOps::RVec<float> *ggHi_pfpIso4subUEec;
   ROOT::VecOps::RVec<float> *ggHi_pfnIso2subUEec;
   ROOT::VecOps::RVec<float> *ggHi_pfnIso3subUEec;
   ROOT::VecOps::RVec<float> *ggHi_pfnIso4subUEec;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso2pTgt2p0subUEec;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso3pTgt2p0subUEec;
   ROOT::VecOps::RVec<float> *ggHi_pfcIso4pTgt2p0subUEec;
   UInt_t          run;
   ULong64_t       evt;
   UInt_t          lumi;
   Float_t         vx;
   Float_t         vy;
   Float_t         vz;
   Int_t           hiBin;
   Float_t         hiHF;
   Int_t           hlt_HLT_HIGEDPhoton10_v10;
   Int_t           hlt_HLT_HIGEDPhoton20_v10;
   Int_t           hlt_HLT_HIGEDPhoton30_v10;
   Int_t           hlt_HLT_HIGEDPhoton40_v10;
   Int_t           hlt_HLT_HIGEDPhoton50_v10;
   Int_t           hlt_HLT_HIGEDPhoton60_v10;
   Int_t           hlt_L1_SingleEG12_BptxAND;
   Int_t           hlt_L1_SingleEG15_BptxAND;
   Int_t           hlt_L1_SingleEG21_BptxAND;
   Int_t           hlt_L1_SingleEG30_BptxAND;
   Int_t           skim_pclusterCompatibilityFilter;
   Int_t           skim_pprimaryVertexFilter;
   Int_t           skim_pphfCoincFilter4Th2;
   Int_t           skim_pphfCoincFilter1Th3;
   Int_t           skim_pphfCoincFilter2Th3;
   Int_t           skim_pphfCoincFilter3Th3;
   Int_t           skim_pphfCoincFilter4Th3;
   Int_t           skim_pphfCoincFilter5Th3;
   Int_t           skim_pphfCoincFilter1Th4;
   Int_t           skim_pphfCoincFilter2Th4;
   Int_t           skim_pphfCoincFilter3Th4;
   Int_t           skim_pphfCoincFilter4Th4;
   Int_t           skim_pphfCoincFilter5Th4;
   Int_t           skim_pphfCoincFilter1Th5;
   Int_t           skim_pphfCoincFilter2Th5;
   Int_t           skim_pphfCoincFilter3Th5;
   Int_t           skim_pphfCoincFilter4Th5;
   Int_t           skim_pphfCoincFilter5Th5;
   Int_t           skim_pphfCoincFilter1Th6;
   Int_t           skim_pphfCoincFilter2Th6;
   Int_t           skim_pphfCoincFilter3Th6;
   Int_t           skim_pphfCoincFilter4Th6;
   Int_t           skim_pphfCoincFilter5Th6;

   // List of branches
   TBranch        *b_AK2Z1_run;   //!
   TBranch        *b_AK2Z1_evt;   //!
   TBranch        *b_AK2Z1_lumi;   //!
   TBranch        *b_AK2Z1_nref;   //!
   TBranch        *b_AK2Z1_ncalo;   //!
   TBranch        *b_nref;   //!
   TBranch        *b_AK2Z1_rawpt;   //!
   TBranch        *b_AK2Z1_jtpt;   //!
   TBranch        *b_AK2Z1_jteta;   //!
   TBranch        *b_AK2Z1_jty;   //!
   TBranch        *b_AK2Z1_jtphi;   //!
   TBranch        *b_AK2Z1_jtpu;   //!
   TBranch        *b_AK2Z1_jtm;   //!
   TBranch        *b_AK2Z1_jtarea;   //!
   TBranch        *b_AK2Z1_jtdyn_split;   //!
   TBranch        *b_AK2Z1_jtdyn_eta;   //!
   TBranch        *b_AK2Z1_jtdyn_phi;   //!
   TBranch        *b_AK2Z1_jtdyn_deltaR;   //!
   TBranch        *b_AK2Z1_jtdyn_kt;   //!
   TBranch        *b_AK2Z1_jtdyn_z;   //!
   TBranch        *b_AK2Z1_jt_intjet_multi;   //!
   TBranch        *b_AK2Z1_jt_girth;   //!
   TBranch        *b_AK2Z1_jt_thrust;   //!
   TBranch        *b_AK2Z1_jt_LHA;   //!
   TBranch        *b_AK2Z1_jt_pTD;   //!
   TBranch        *b_AK2Z2_run;   //!
   TBranch        *b_AK2Z2_evt;   //!
   TBranch        *b_AK2Z2_lumi;   //!
   TBranch        *b_AK2Z2_nref;   //!
   TBranch        *b_AK2Z2_ncalo;   //!
   TBranch        *b_AK2Z2_rawpt;   //!
   TBranch        *b_AK2Z2_jtpt;   //!
   TBranch        *b_AK2Z2_jteta;   //!
   TBranch        *b_AK2Z2_jty;   //!
   TBranch        *b_AK2Z2_jtphi;   //!
   TBranch        *b_AK2Z2_jtpu;   //!
   TBranch        *b_AK2Z2_jtm;   //!
   TBranch        *b_AK2Z2_jtarea;   //!
   TBranch        *b_AK2Z2_jtdyn_split;   //!
   TBranch        *b_AK2Z2_jtdyn_eta;   //!
   TBranch        *b_AK2Z2_jtdyn_phi;   //!
   TBranch        *b_AK2Z2_jtdyn_deltaR;   //!
   TBranch        *b_AK2Z2_jtdyn_kt;   //!
   TBranch        *b_AK2Z2_jtdyn_z;   //!
   TBranch        *b_AK2Z2_jt_intjet_multi;   //!
   TBranch        *b_AK2Z2_jt_girth;   //!
   TBranch        *b_AK2Z2_jt_thrust;   //!
   TBranch        *b_AK2Z2_jt_LHA;   //!
   TBranch        *b_AK2Z2_jt_pTD;   //!
   TBranch        *b_AK2Z3_run;   //!
   TBranch        *b_AK2Z3_evt;   //!
   TBranch        *b_AK2Z3_lumi;   //!
   TBranch        *b_AK2Z3_nref;   //!
   TBranch        *b_AK2Z3_ncalo;   //!
   TBranch        *b_AK2Z3_rawpt;   //!
   TBranch        *b_AK2Z3_jtpt;   //!
   TBranch        *b_AK2Z3_jteta;   //!
   TBranch        *b_AK2Z3_jty;   //!
   TBranch        *b_AK2Z3_jtphi;   //!
   TBranch        *b_AK2Z3_jtpu;   //!
   TBranch        *b_AK2Z3_jtm;   //!
   TBranch        *b_AK2Z3_jtarea;   //!
   TBranch        *b_AK2Z3_jtdyn_split;   //!
   TBranch        *b_AK2Z3_jtdyn_eta;   //!
   TBranch        *b_AK2Z3_jtdyn_phi;   //!
   TBranch        *b_AK2Z3_jtdyn_deltaR;   //!
   TBranch        *b_AK2Z3_jtdyn_kt;   //!
   TBranch        *b_AK2Z3_jtdyn_z;   //!
   TBranch        *b_AK2Z3_jt_intjet_multi;   //!
   TBranch        *b_AK2Z3_jt_girth;   //!
   TBranch        *b_AK2Z3_jt_thrust;   //!
   TBranch        *b_AK2Z3_jt_LHA;   //!
   TBranch        *b_AK2Z3_jt_pTD;   //!
   TBranch        *b_AK2Z4_run;   //!
   TBranch        *b_AK2Z4_evt;   //!
   TBranch        *b_AK2Z4_lumi;   //!
   TBranch        *b_AK2Z4_nref;   //!
   TBranch        *b_AK2Z4_ncalo;   //!
   TBranch        *b_AK2Z4_rawpt;   //!
   TBranch        *b_AK2Z4_jtpt;   //!
   TBranch        *b_AK2Z4_jteta;   //!
   TBranch        *b_AK2Z4_jty;   //!
   TBranch        *b_AK2Z4_jtphi;   //!
   TBranch        *b_AK2Z4_jtpu;   //!
   TBranch        *b_AK2Z4_jtm;   //!
   TBranch        *b_AK2Z4_jtarea;   //!
   TBranch        *b_AK2Z4_jtdyn_split;   //!
   TBranch        *b_AK2Z4_jtdyn_eta;   //!
   TBranch        *b_AK2Z4_jtdyn_phi;   //!
   TBranch        *b_AK2Z4_jtdyn_deltaR;   //!
   TBranch        *b_AK2Z4_jtdyn_kt;   //!
   TBranch        *b_AK2Z4_jtdyn_z;   //!
   TBranch        *b_AK2Z4_jt_intjet_multi;   //!
   TBranch        *b_AK2Z4_jt_girth;   //!
   TBranch        *b_AK2Z4_jt_thrust;   //!
   TBranch        *b_AK2Z4_jt_LHA;   //!
   TBranch        *b_AK2Z4_jt_pTD;   //!
   TBranch        *b_AK3Z1_run;   //!
   TBranch        *b_AK3Z1_evt;   //!
   TBranch        *b_AK3Z1_lumi;   //!
   TBranch        *b_AK3Z1_nref;   //!
   TBranch        *b_AK3Z1_ncalo;   //!
   TBranch        *b_AK3Z1_rawpt;   //!
   TBranch        *b_AK3Z1_jtpt;   //!
   TBranch        *b_AK3Z1_jteta;   //!
   TBranch        *b_AK3Z1_jty;   //!
   TBranch        *b_AK3Z1_jtphi;   //!
   TBranch        *b_AK3Z1_jtpu;   //!
   TBranch        *b_AK3Z1_jtm;   //!
   TBranch        *b_AK3Z1_jtarea;   //!
   TBranch        *b_AK3Z1_jtdyn_split;   //!
   TBranch        *b_AK3Z1_jtdyn_eta;   //!
   TBranch        *b_AK3Z1_jtdyn_phi;   //!
   TBranch        *b_AK3Z1_jtdyn_deltaR;   //!
   TBranch        *b_AK3Z1_jtdyn_kt;   //!
   TBranch        *b_AK3Z1_jtdyn_z;   //!
   TBranch        *b_AK3Z1_jt_intjet_multi;   //!
   TBranch        *b_AK3Z1_jt_girth;   //!
   TBranch        *b_AK3Z1_jt_thrust;   //!
   TBranch        *b_AK3Z1_jt_LHA;   //!
   TBranch        *b_AK3Z1_jt_pTD;   //!
   TBranch        *b_AK3Z2_run;   //!
   TBranch        *b_AK3Z2_evt;   //!
   TBranch        *b_AK3Z2_lumi;   //!
   TBranch        *b_AK3Z2_nref;   //!
   TBranch        *b_AK3Z2_ncalo;   //!
   TBranch        *b_AK3Z2_rawpt;   //!
   TBranch        *b_AK3Z2_jtpt;   //!
   TBranch        *b_AK3Z2_jteta;   //!
   TBranch        *b_AK3Z2_jty;   //!
   TBranch        *b_AK3Z2_jtphi;   //!
   TBranch        *b_AK3Z2_jtpu;   //!
   TBranch        *b_AK3Z2_jtm;   //!
   TBranch        *b_AK3Z2_jtarea;   //!
   TBranch        *b_AK3Z2_jtdyn_split;   //!
   TBranch        *b_AK3Z2_jtdyn_eta;   //!
   TBranch        *b_AK3Z2_jtdyn_phi;   //!
   TBranch        *b_AK3Z2_jtdyn_deltaR;   //!
   TBranch        *b_AK3Z2_jtdyn_kt;   //!
   TBranch        *b_AK3Z2_jtdyn_z;   //!
   TBranch        *b_AK3Z2_jt_intjet_multi;   //!
   TBranch        *b_AK3Z2_jt_girth;   //!
   TBranch        *b_AK3Z2_jt_thrust;   //!
   TBranch        *b_AK3Z2_jt_LHA;   //!
   TBranch        *b_AK3Z2_jt_pTD;   //!
   TBranch        *b_AK3Z3_run;   //!
   TBranch        *b_AK3Z3_evt;   //!
   TBranch        *b_AK3Z3_lumi;   //!
   TBranch        *b_AK3Z3_nref;   //!
   TBranch        *b_AK3Z3_ncalo;   //!
   TBranch        *b_AK3Z3_rawpt;   //!
   TBranch        *b_AK3Z3_jtpt;   //!
   TBranch        *b_AK3Z3_jteta;   //!
   TBranch        *b_AK3Z3_jty;   //!
   TBranch        *b_AK3Z3_jtphi;   //!
   TBranch        *b_AK3Z3_jtpu;   //!
   TBranch        *b_AK3Z3_jtm;   //!
   TBranch        *b_AK3Z3_jtarea;   //!
   TBranch        *b_AK3Z3_jtdyn_split;   //!
   TBranch        *b_AK3Z3_jtdyn_eta;   //!
   TBranch        *b_AK3Z3_jtdyn_phi;   //!
   TBranch        *b_AK3Z3_jtdyn_deltaR;   //!
   TBranch        *b_AK3Z3_jtdyn_kt;   //!
   TBranch        *b_AK3Z3_jtdyn_z;   //!
   TBranch        *b_AK3Z3_jt_intjet_multi;   //!
   TBranch        *b_AK3Z3_jt_girth;   //!
   TBranch        *b_AK3Z3_jt_thrust;   //!
   TBranch        *b_AK3Z3_jt_LHA;   //!
   TBranch        *b_AK3Z3_jt_pTD;   //!
   TBranch        *b_AK3Z4_run;   //!
   TBranch        *b_AK3Z4_evt;   //!
   TBranch        *b_AK3Z4_lumi;   //!
   TBranch        *b_AK3Z4_nref;   //!
   TBranch        *b_AK3Z4_ncalo;   //!
   TBranch        *b_AK3Z4_rawpt;   //!
   TBranch        *b_AK3Z4_jtpt;   //!
   TBranch        *b_AK3Z4_jteta;   //!
   TBranch        *b_AK3Z4_jty;   //!
   TBranch        *b_AK3Z4_jtphi;   //!
   TBranch        *b_AK3Z4_jtpu;   //!
   TBranch        *b_AK3Z4_jtm;   //!
   TBranch        *b_AK3Z4_jtarea;   //!
   TBranch        *b_AK3Z4_jtdyn_split;   //!
   TBranch        *b_AK3Z4_jtdyn_eta;   //!
   TBranch        *b_AK3Z4_jtdyn_phi;   //!
   TBranch        *b_AK3Z4_jtdyn_deltaR;   //!
   TBranch        *b_AK3Z4_jtdyn_kt;   //!
   TBranch        *b_AK3Z4_jtdyn_z;   //!
   TBranch        *b_AK3Z4_jt_intjet_multi;   //!
   TBranch        *b_AK3Z4_jt_girth;   //!
   TBranch        *b_AK3Z4_jt_thrust;   //!
   TBranch        *b_AK3Z4_jt_LHA;   //!
   TBranch        *b_AK3Z4_jt_pTD;   //!
   TBranch        *b_AK3Z5_run;   //!
   TBranch        *b_AK3Z5_evt;   //!
   TBranch        *b_AK3Z5_lumi;   //!
   TBranch        *b_AK3Z5_nref;   //!
   TBranch        *b_AK3Z5_ncalo;   //!
   TBranch        *b_AK3Z5_rawpt;   //!
   TBranch        *b_AK3Z5_jtpt;   //!
   TBranch        *b_AK3Z5_jteta;   //!
   TBranch        *b_AK3Z5_jty;   //!
   TBranch        *b_AK3Z5_jtphi;   //!
   TBranch        *b_AK3Z5_jtpu;   //!
   TBranch        *b_AK3Z5_jtm;   //!
   TBranch        *b_AK3Z5_jtarea;   //!
   TBranch        *b_AK3Z5_jtdyn_split;   //!
   TBranch        *b_AK3Z5_jtdyn_eta;   //!
   TBranch        *b_AK3Z5_jtdyn_phi;   //!
   TBranch        *b_AK3Z5_jtdyn_deltaR;   //!
   TBranch        *b_AK3Z5_jtdyn_kt;   //!
   TBranch        *b_AK3Z5_jtdyn_z;   //!
   TBranch        *b_AK3Z5_jt_intjet_multi;   //!
   TBranch        *b_AK3Z5_jt_girth;   //!
   TBranch        *b_AK3Z5_jt_thrust;   //!
   TBranch        *b_AK3Z5_jt_LHA;   //!
   TBranch        *b_AK3Z5_jt_pTD;   //!
   TBranch        *b_AK4Z1_run;   //!
   TBranch        *b_AK4Z1_evt;   //!
   TBranch        *b_AK4Z1_lumi;   //!
   TBranch        *b_AK4Z1_nref;   //!
   TBranch        *b_AK4Z1_ncalo;   //!
   TBranch        *b_AK4Z1_rawpt;   //!
   TBranch        *b_AK4Z1_jtpt;   //!
   TBranch        *b_AK4Z1_jteta;   //!
   TBranch        *b_AK4Z1_jty;   //!
   TBranch        *b_AK4Z1_jtphi;   //!
   TBranch        *b_AK4Z1_jtpu;   //!
   TBranch        *b_AK4Z1_jtm;   //!
   TBranch        *b_AK4Z1_jtarea;   //!
   TBranch        *b_AK4Z1_jtdyn_split;   //!
   TBranch        *b_AK4Z1_jtdyn_eta;   //!
   TBranch        *b_AK4Z1_jtdyn_phi;   //!
   TBranch        *b_AK4Z1_jtdyn_deltaR;   //!
   TBranch        *b_AK4Z1_jtdyn_kt;   //!
   TBranch        *b_AK4Z1_jtdyn_z;   //!
   TBranch        *b_AK4Z1_jt_intjet_multi;   //!
   TBranch        *b_AK4Z1_jt_girth;   //!
   TBranch        *b_AK4Z1_jt_thrust;   //!
   TBranch        *b_AK4Z1_jt_LHA;   //!
   TBranch        *b_AK4Z1_jt_pTD;   //!
   TBranch        *b_AK4Z2_run;   //!
   TBranch        *b_AK4Z2_evt;   //!
   TBranch        *b_AK4Z2_lumi;   //!
   TBranch        *b_AK4Z2_nref;   //!
   TBranch        *b_AK4Z2_ncalo;   //!
   TBranch        *b_AK4Z2_rawpt;   //!
   TBranch        *b_AK4Z2_jtpt;   //!
   TBranch        *b_AK4Z2_jteta;   //!
   TBranch        *b_AK4Z2_jty;   //!
   TBranch        *b_AK4Z2_jtphi;   //!
   TBranch        *b_AK4Z2_jtpu;   //!
   TBranch        *b_AK4Z2_jtm;   //!
   TBranch        *b_AK4Z2_jtarea;   //!
   TBranch        *b_AK4Z2_jtdyn_split;   //!
   TBranch        *b_AK4Z2_jtdyn_eta;   //!
   TBranch        *b_AK4Z2_jtdyn_phi;   //!
   TBranch        *b_AK4Z2_jtdyn_deltaR;   //!
   TBranch        *b_AK4Z2_jtdyn_kt;   //!
   TBranch        *b_AK4Z2_jtdyn_z;   //!
   TBranch        *b_AK4Z2_jt_intjet_multi;   //!
   TBranch        *b_AK4Z2_jt_girth;   //!
   TBranch        *b_AK4Z2_jt_thrust;   //!
   TBranch        *b_AK4Z2_jt_LHA;   //!
   TBranch        *b_AK4Z2_jt_pTD;   //!
   TBranch        *b_AK4Z3_run;   //!
   TBranch        *b_AK4Z3_evt;   //!
   TBranch        *b_AK4Z3_lumi;   //!
   TBranch        *b_AK4Z3_nref;   //!
   TBranch        *b_AK4Z3_ncalo;   //!
   TBranch        *b_AK4Z3_rawpt;   //!
   TBranch        *b_AK4Z3_jtpt;   //!
   TBranch        *b_AK4Z3_jteta;   //!
   TBranch        *b_AK4Z3_jty;   //!
   TBranch        *b_AK4Z3_jtphi;   //!
   TBranch        *b_AK4Z3_jtpu;   //!
   TBranch        *b_AK4Z3_jtm;   //!
   TBranch        *b_AK4Z3_jtarea;   //!
   TBranch        *b_AK4Z3_jtdyn_split;   //!
   TBranch        *b_AK4Z3_jtdyn_eta;   //!
   TBranch        *b_AK4Z3_jtdyn_phi;   //!
   TBranch        *b_AK4Z3_jtdyn_deltaR;   //!
   TBranch        *b_AK4Z3_jtdyn_kt;   //!
   TBranch        *b_AK4Z3_jtdyn_z;   //!
   TBranch        *b_AK4Z3_jt_intjet_multi;   //!
   TBranch        *b_AK4Z3_jt_girth;   //!
   TBranch        *b_AK4Z3_jt_thrust;   //!
   TBranch        *b_AK4Z3_jt_LHA;   //!
   TBranch        *b_AK4Z3_jt_pTD;   //!
   TBranch        *b_AK4Z4_run;   //!
   TBranch        *b_AK4Z4_evt;   //!
   TBranch        *b_AK4Z4_lumi;   //!
   TBranch        *b_AK4Z4_nref;   //!
   TBranch        *b_AK4Z4_ncalo;   //!
   TBranch        *b_AK4Z4_rawpt;   //!
   TBranch        *b_AK4Z4_jtpt;   //!
   TBranch        *b_AK4Z4_jteta;   //!
   TBranch        *b_AK4Z4_jty;   //!
   TBranch        *b_AK4Z4_jtphi;   //!
   TBranch        *b_AK4Z4_jtpu;   //!
   TBranch        *b_AK4Z4_jtm;   //!
   TBranch        *b_AK4Z4_jtarea;   //!
   TBranch        *b_AK4Z4_jtdyn_split;   //!
   TBranch        *b_AK4Z4_jtdyn_eta;   //!
   TBranch        *b_AK4Z4_jtdyn_phi;   //!
   TBranch        *b_AK4Z4_jtdyn_deltaR;   //!
   TBranch        *b_AK4Z4_jtdyn_kt;   //!
   TBranch        *b_AK4Z4_jtdyn_z;   //!
   TBranch        *b_AK4Z4_jt_intjet_multi;   //!
   TBranch        *b_AK4Z4_jt_girth;   //!
   TBranch        *b_AK4Z4_jt_thrust;   //!
   TBranch        *b_AK4Z4_jt_LHA;   //!
   TBranch        *b_AK4Z4_jt_pTD;   //!
   TBranch        *b_AK4Z5_run;   //!
   TBranch        *b_AK4Z5_evt;   //!
   TBranch        *b_AK4Z5_lumi;   //!
   TBranch        *b_AK4Z5_nref;   //!
   TBranch        *b_AK4Z5_ncalo;   //!
   TBranch        *b_AK4Z5_rawpt;   //!
   TBranch        *b_AK4Z5_jtpt;   //!
   TBranch        *b_AK4Z5_jteta;   //!
   TBranch        *b_AK4Z5_jty;   //!
   TBranch        *b_AK4Z5_jtphi;   //!
   TBranch        *b_AK4Z5_jtpu;   //!
   TBranch        *b_AK4Z5_jtm;   //!
   TBranch        *b_AK4Z5_jtarea;   //!
   TBranch        *b_AK4Z5_jtdyn_split;   //!
   TBranch        *b_AK4Z5_jtdyn_eta;   //!
   TBranch        *b_AK4Z5_jtdyn_phi;   //!
   TBranch        *b_AK4Z5_jtdyn_deltaR;   //!
   TBranch        *b_AK4Z5_jtdyn_kt;   //!
   TBranch        *b_AK4Z5_jtdyn_z;   //!
   TBranch        *b_AK4Z5_jt_intjet_multi;   //!
   TBranch        *b_AK4Z5_jt_girth;   //!
   TBranch        *b_AK4Z5_jt_thrust;   //!
   TBranch        *b_AK4Z5_jt_LHA;   //!
   TBranch        *b_AK4Z5_jt_pTD;   //!
   TBranch        *b_AK5Z1_run;   //!
   TBranch        *b_AK5Z1_evt;   //!
   TBranch        *b_AK5Z1_lumi;   //!
   TBranch        *b_AK5Z1_nref;   //!
   TBranch        *b_AK5Z1_ncalo;   //!
   TBranch        *b_AK5Z1_rawpt;   //!
   TBranch        *b_AK5Z1_jtpt;   //!
   TBranch        *b_AK5Z1_jteta;   //!
   TBranch        *b_AK5Z1_jty;   //!
   TBranch        *b_AK5Z1_jtphi;   //!
   TBranch        *b_AK5Z1_jtpu;   //!
   TBranch        *b_AK5Z1_jtm;   //!
   TBranch        *b_AK5Z1_jtarea;   //!
   TBranch        *b_AK5Z1_jtdyn_split;   //!
   TBranch        *b_AK5Z1_jtdyn_eta;   //!
   TBranch        *b_AK5Z1_jtdyn_phi;   //!
   TBranch        *b_AK5Z1_jtdyn_deltaR;   //!
   TBranch        *b_AK5Z1_jtdyn_kt;   //!
   TBranch        *b_AK5Z1_jtdyn_z;   //!
   TBranch        *b_AK5Z1_jt_intjet_multi;   //!
   TBranch        *b_AK5Z1_jt_girth;   //!
   TBranch        *b_AK5Z1_jt_thrust;   //!
   TBranch        *b_AK5Z1_jt_LHA;   //!
   TBranch        *b_AK5Z1_jt_pTD;   //!
   TBranch        *b_AK5Z2_run;   //!
   TBranch        *b_AK5Z2_evt;   //!
   TBranch        *b_AK5Z2_lumi;   //!
   TBranch        *b_AK5Z2_nref;   //!
   TBranch        *b_AK5Z2_ncalo;   //!
   TBranch        *b_AK5Z2_rawpt;   //!
   TBranch        *b_AK5Z2_jtpt;   //!
   TBranch        *b_AK5Z2_jteta;   //!
   TBranch        *b_AK5Z2_jty;   //!
   TBranch        *b_AK5Z2_jtphi;   //!
   TBranch        *b_AK5Z2_jtpu;   //!
   TBranch        *b_AK5Z2_jtm;   //!
   TBranch        *b_AK5Z2_jtarea;   //!
   TBranch        *b_AK5Z2_jtdyn_split;   //!
   TBranch        *b_AK5Z2_jtdyn_eta;   //!
   TBranch        *b_AK5Z2_jtdyn_phi;   //!
   TBranch        *b_AK5Z2_jtdyn_deltaR;   //!
   TBranch        *b_AK5Z2_jtdyn_kt;   //!
   TBranch        *b_AK5Z2_jtdyn_z;   //!
   TBranch        *b_AK5Z2_jt_intjet_multi;   //!
   TBranch        *b_AK5Z2_jt_girth;   //!
   TBranch        *b_AK5Z2_jt_thrust;   //!
   TBranch        *b_AK5Z2_jt_LHA;   //!
   TBranch        *b_AK5Z2_jt_pTD;   //!
   TBranch        *b_AK5Z3_run;   //!
   TBranch        *b_AK5Z3_evt;   //!
   TBranch        *b_AK5Z3_lumi;   //!
   TBranch        *b_AK5Z3_nref;   //!
   TBranch        *b_AK5Z3_ncalo;   //!
   TBranch        *b_AK5Z3_rawpt;   //!
   TBranch        *b_AK5Z3_jtpt;   //!
   TBranch        *b_AK5Z3_jteta;   //!
   TBranch        *b_AK5Z3_jty;   //!
   TBranch        *b_AK5Z3_jtphi;   //!
   TBranch        *b_AK5Z3_jtpu;   //!
   TBranch        *b_AK5Z3_jtm;   //!
   TBranch        *b_AK5Z3_jtarea;   //!
   TBranch        *b_AK5Z3_jtdyn_split;   //!
   TBranch        *b_AK5Z3_jtdyn_eta;   //!
   TBranch        *b_AK5Z3_jtdyn_phi;   //!
   TBranch        *b_AK5Z3_jtdyn_deltaR;   //!
   TBranch        *b_AK5Z3_jtdyn_kt;   //!
   TBranch        *b_AK5Z3_jtdyn_z;   //!
   TBranch        *b_AK5Z3_jt_intjet_multi;   //!
   TBranch        *b_AK5Z3_jt_girth;   //!
   TBranch        *b_AK5Z3_jt_thrust;   //!
   TBranch        *b_AK5Z3_jt_LHA;   //!
   TBranch        *b_AK5Z3_jt_pTD;   //!
   TBranch        *b_AK5Z4_run;   //!
   TBranch        *b_AK5Z4_evt;   //!
   TBranch        *b_AK5Z4_lumi;   //!
   TBranch        *b_AK5Z4_nref;   //!
   TBranch        *b_AK5Z4_ncalo;   //!
   TBranch        *b_AK5Z4_rawpt;   //!
   TBranch        *b_AK5Z4_jtpt;   //!
   TBranch        *b_AK5Z4_jteta;   //!
   TBranch        *b_AK5Z4_jty;   //!
   TBranch        *b_AK5Z4_jtphi;   //!
   TBranch        *b_AK5Z4_jtpu;   //!
   TBranch        *b_AK5Z4_jtm;   //!
   TBranch        *b_AK5Z4_jtarea;   //!
   TBranch        *b_AK5Z4_jtdyn_split;   //!
   TBranch        *b_AK5Z4_jtdyn_eta;   //!
   TBranch        *b_AK5Z4_jtdyn_phi;   //!
   TBranch        *b_AK5Z4_jtdyn_deltaR;   //!
   TBranch        *b_AK5Z4_jtdyn_kt;   //!
   TBranch        *b_AK5Z4_jtdyn_z;   //!
   TBranch        *b_AK5Z4_jt_intjet_multi;   //!
   TBranch        *b_AK5Z4_jt_girth;   //!
   TBranch        *b_AK5Z4_jt_thrust;   //!
   TBranch        *b_AK5Z4_jt_LHA;   //!
   TBranch        *b_AK5Z4_jt_pTD;   //!
   TBranch        *b_AK5Z5_run;   //!
   TBranch        *b_AK5Z5_evt;   //!
   TBranch        *b_AK5Z5_lumi;   //!
   TBranch        *b_AK5Z5_nref;   //!
   TBranch        *b_AK5Z5_ncalo;   //!
   TBranch        *b_AK5Z5_rawpt;   //!
   TBranch        *b_AK5Z5_jtpt;   //!
   TBranch        *b_AK5Z5_jteta;   //!
   TBranch        *b_AK5Z5_jty;   //!
   TBranch        *b_AK5Z5_jtphi;   //!
   TBranch        *b_AK5Z5_jtpu;   //!
   TBranch        *b_AK5Z5_jtm;   //!
   TBranch        *b_AK5Z5_jtarea;   //!
   TBranch        *b_AK5Z5_jtdyn_split;   //!
   TBranch        *b_AK5Z5_jtdyn_eta;   //!
   TBranch        *b_AK5Z5_jtdyn_phi;   //!
   TBranch        *b_AK5Z5_jtdyn_deltaR;   //!
   TBranch        *b_AK5Z5_jtdyn_kt;   //!
   TBranch        *b_AK5Z5_jtdyn_z;   //!
   TBranch        *b_AK5Z5_jt_intjet_multi;   //!
   TBranch        *b_AK5Z5_jt_girth;   //!
   TBranch        *b_AK5Z5_jt_thrust;   //!
   TBranch        *b_AK5Z5_jt_LHA;   //!
   TBranch        *b_AK5Z5_jt_pTD;   //!
   TBranch        *b_AK6Z1_run;   //!
   TBranch        *b_AK6Z1_evt;   //!
   TBranch        *b_AK6Z1_lumi;   //!
   TBranch        *b_AK6Z1_nref;   //!
   TBranch        *b_AK6Z1_ncalo;   //!
   TBranch        *b_AK6Z1_rawpt;   //!
   TBranch        *b_AK6Z1_jtpt;   //!
   TBranch        *b_AK6Z1_jteta;   //!
   TBranch        *b_AK6Z1_jty;   //!
   TBranch        *b_AK6Z1_jtphi;   //!
   TBranch        *b_AK6Z1_jtpu;   //!
   TBranch        *b_AK6Z1_jtm;   //!
   TBranch        *b_AK6Z1_jtarea;   //!
   TBranch        *b_AK6Z1_jtdyn_split;   //!
   TBranch        *b_AK6Z1_jtdyn_eta;   //!
   TBranch        *b_AK6Z1_jtdyn_phi;   //!
   TBranch        *b_AK6Z1_jtdyn_deltaR;   //!
   TBranch        *b_AK6Z1_jtdyn_kt;   //!
   TBranch        *b_AK6Z1_jtdyn_z;   //!
   TBranch        *b_AK6Z1_jt_intjet_multi;   //!
   TBranch        *b_AK6Z1_jt_girth;   //!
   TBranch        *b_AK6Z1_jt_thrust;   //!
   TBranch        *b_AK6Z1_jt_LHA;   //!
   TBranch        *b_AK6Z1_jt_pTD;   //!
   TBranch        *b_AK6Z2_run;   //!
   TBranch        *b_AK6Z2_evt;   //!
   TBranch        *b_AK6Z2_lumi;   //!
   TBranch        *b_AK6Z2_nref;   //!
   TBranch        *b_AK6Z2_ncalo;   //!
   TBranch        *b_AK6Z2_rawpt;   //!
   TBranch        *b_AK6Z2_jtpt;   //!
   TBranch        *b_AK6Z2_jteta;   //!
   TBranch        *b_AK6Z2_jty;   //!
   TBranch        *b_AK6Z2_jtphi;   //!
   TBranch        *b_AK6Z2_jtpu;   //!
   TBranch        *b_AK6Z2_jtm;   //!
   TBranch        *b_AK6Z2_jtarea;   //!
   TBranch        *b_AK6Z2_jtdyn_split;   //!
   TBranch        *b_AK6Z2_jtdyn_eta;   //!
   TBranch        *b_AK6Z2_jtdyn_phi;   //!
   TBranch        *b_AK6Z2_jtdyn_deltaR;   //!
   TBranch        *b_AK6Z2_jtdyn_kt;   //!
   TBranch        *b_AK6Z2_jtdyn_z;   //!
   TBranch        *b_AK6Z2_jt_intjet_multi;   //!
   TBranch        *b_AK6Z2_jt_girth;   //!
   TBranch        *b_AK6Z2_jt_thrust;   //!
   TBranch        *b_AK6Z2_jt_LHA;   //!
   TBranch        *b_AK6Z2_jt_pTD;   //!
   TBranch        *b_AK6Z3_run;   //!
   TBranch        *b_AK6Z3_evt;   //!
   TBranch        *b_AK6Z3_lumi;   //!
   TBranch        *b_AK6Z3_nref;   //!
   TBranch        *b_AK6Z3_ncalo;   //!
   TBranch        *b_AK6Z3_rawpt;   //!
   TBranch        *b_AK6Z3_jtpt;   //!
   TBranch        *b_AK6Z3_jteta;   //!
   TBranch        *b_AK6Z3_jty;   //!
   TBranch        *b_AK6Z3_jtphi;   //!
   TBranch        *b_AK6Z3_jtpu;   //!
   TBranch        *b_AK6Z3_jtm;   //!
   TBranch        *b_AK6Z3_jtarea;   //!
   TBranch        *b_AK6Z3_jtdyn_split;   //!
   TBranch        *b_AK6Z3_jtdyn_eta;   //!
   TBranch        *b_AK6Z3_jtdyn_phi;   //!
   TBranch        *b_AK6Z3_jtdyn_deltaR;   //!
   TBranch        *b_AK6Z3_jtdyn_kt;   //!
   TBranch        *b_AK6Z3_jtdyn_z;   //!
   TBranch        *b_AK6Z3_jt_intjet_multi;   //!
   TBranch        *b_AK6Z3_jt_girth;   //!
   TBranch        *b_AK6Z3_jt_thrust;   //!
   TBranch        *b_AK6Z3_jt_LHA;   //!
   TBranch        *b_AK6Z3_jt_pTD;   //!
   TBranch        *b_AK6Z4_run;   //!
   TBranch        *b_AK6Z4_evt;   //!
   TBranch        *b_AK6Z4_lumi;   //!
   TBranch        *b_AK6Z4_nref;   //!
   TBranch        *b_AK6Z4_ncalo;   //!
   TBranch        *b_AK6Z4_rawpt;   //!
   TBranch        *b_AK6Z4_jtpt;   //!
   TBranch        *b_AK6Z4_jteta;   //!
   TBranch        *b_AK6Z4_jty;   //!
   TBranch        *b_AK6Z4_jtphi;   //!
   TBranch        *b_AK6Z4_jtpu;   //!
   TBranch        *b_AK6Z4_jtm;   //!
   TBranch        *b_AK6Z4_jtarea;   //!
   TBranch        *b_AK6Z4_jtdyn_split;   //!
   TBranch        *b_AK6Z4_jtdyn_eta;   //!
   TBranch        *b_AK6Z4_jtdyn_phi;   //!
   TBranch        *b_AK6Z4_jtdyn_deltaR;   //!
   TBranch        *b_AK6Z4_jtdyn_kt;   //!
   TBranch        *b_AK6Z4_jtdyn_z;   //!
   TBranch        *b_AK6Z4_jt_intjet_multi;   //!
   TBranch        *b_AK6Z4_jt_girth;   //!
   TBranch        *b_AK6Z4_jt_thrust;   //!
   TBranch        *b_AK6Z4_jt_LHA;   //!
   TBranch        *b_AK6Z4_jt_pTD;   //!
   TBranch        *b_AK6Z5_run;   //!
   TBranch        *b_AK6Z5_evt;   //!
   TBranch        *b_AK6Z5_lumi;   //!
   TBranch        *b_AK6Z5_nref;   //!
   TBranch        *b_AK6Z5_ncalo;   //!
   TBranch        *b_AK6Z5_rawpt;   //!
   TBranch        *b_AK6Z5_jtpt;   //!
   TBranch        *b_AK6Z5_jteta;   //!
   TBranch        *b_AK6Z5_jty;   //!
   TBranch        *b_AK6Z5_jtphi;   //!
   TBranch        *b_AK6Z5_jtpu;   //!
   TBranch        *b_AK6Z5_jtm;   //!
   TBranch        *b_AK6Z5_jtarea;   //!
   TBranch        *b_AK6Z5_jtdyn_split;   //!
   TBranch        *b_AK6Z5_jtdyn_eta;   //!
   TBranch        *b_AK6Z5_jtdyn_phi;   //!
   TBranch        *b_AK6Z5_jtdyn_deltaR;   //!
   TBranch        *b_AK6Z5_jtdyn_kt;   //!
   TBranch        *b_AK6Z5_jtdyn_z;   //!
   TBranch        *b_AK6Z5_jt_intjet_multi;   //!
   TBranch        *b_AK6Z5_jt_girth;   //!
   TBranch        *b_AK6Z5_jt_thrust;   //!
   TBranch        *b_AK6Z5_jt_LHA;   //!
   TBranch        *b_AK6Z5_jt_pTD;   //!
   TBranch        *b_AK8Z1_run;   //!
   TBranch        *b_AK8Z1_evt;   //!
   TBranch        *b_AK8Z1_lumi;   //!
   TBranch        *b_AK8Z1_nref;   //!
   TBranch        *b_AK8Z1_ncalo;   //!
   TBranch        *b_AK8Z1_rawpt;   //!
   TBranch        *b_AK8Z1_jtpt;   //!
   TBranch        *b_AK8Z1_jteta;   //!
   TBranch        *b_AK8Z1_jty;   //!
   TBranch        *b_AK8Z1_jtphi;   //!
   TBranch        *b_AK8Z1_jtpu;   //!
   TBranch        *b_AK8Z1_jtm;   //!
   TBranch        *b_AK8Z1_jtarea;   //!
   TBranch        *b_AK8Z1_jtdyn_split;   //!
   TBranch        *b_AK8Z1_jtdyn_eta;   //!
   TBranch        *b_AK8Z1_jtdyn_phi;   //!
   TBranch        *b_AK8Z1_jtdyn_deltaR;   //!
   TBranch        *b_AK8Z1_jtdyn_kt;   //!
   TBranch        *b_AK8Z1_jtdyn_z;   //!
   TBranch        *b_AK8Z1_jt_intjet_multi;   //!
   TBranch        *b_AK8Z1_jt_girth;   //!
   TBranch        *b_AK8Z1_jt_thrust;   //!
   TBranch        *b_AK8Z1_jt_LHA;   //!
   TBranch        *b_AK8Z1_jt_pTD;   //!
   TBranch        *b_AK8Z2_run;   //!
   TBranch        *b_AK8Z2_evt;   //!
   TBranch        *b_AK8Z2_lumi;   //!
   TBranch        *b_AK8Z2_nref;   //!
   TBranch        *b_AK8Z2_ncalo;   //!
   TBranch        *b_AK8Z2_rawpt;   //!
   TBranch        *b_AK8Z2_jtpt;   //!
   TBranch        *b_AK8Z2_jteta;   //!
   TBranch        *b_AK8Z2_jty;   //!
   TBranch        *b_AK8Z2_jtphi;   //!
   TBranch        *b_AK8Z2_jtpu;   //!
   TBranch        *b_AK8Z2_jtm;   //!
   TBranch        *b_AK8Z2_jtarea;   //!
   TBranch        *b_AK8Z2_jtdyn_split;   //!
   TBranch        *b_AK8Z2_jtdyn_eta;   //!
   TBranch        *b_AK8Z2_jtdyn_phi;   //!
   TBranch        *b_AK8Z2_jtdyn_deltaR;   //!
   TBranch        *b_AK8Z2_jtdyn_kt;   //!
   TBranch        *b_AK8Z2_jtdyn_z;   //!
   TBranch        *b_AK8Z2_jt_intjet_multi;   //!
   TBranch        *b_AK8Z2_jt_girth;   //!
   TBranch        *b_AK8Z2_jt_thrust;   //!
   TBranch        *b_AK8Z2_jt_LHA;   //!
   TBranch        *b_AK8Z2_jt_pTD;   //!
   TBranch        *b_AK8Z3_run;   //!
   TBranch        *b_AK8Z3_evt;   //!
   TBranch        *b_AK8Z3_lumi;   //!
   TBranch        *b_AK8Z3_nref;   //!
   TBranch        *b_AK8Z3_ncalo;   //!
   TBranch        *b_AK8Z3_rawpt;   //!
   TBranch        *b_AK8Z3_jtpt;   //!
   TBranch        *b_AK8Z3_jteta;   //!
   TBranch        *b_AK8Z3_jty;   //!
   TBranch        *b_AK8Z3_jtphi;   //!
   TBranch        *b_AK8Z3_jtpu;   //!
   TBranch        *b_AK8Z3_jtm;   //!
   TBranch        *b_AK8Z3_jtarea;   //!
   TBranch        *b_AK8Z3_jtdyn_split;   //!
   TBranch        *b_AK8Z3_jtdyn_eta;   //!
   TBranch        *b_AK8Z3_jtdyn_phi;   //!
   TBranch        *b_AK8Z3_jtdyn_deltaR;   //!
   TBranch        *b_AK8Z3_jtdyn_kt;   //!
   TBranch        *b_AK8Z3_jtdyn_z;   //!
   TBranch        *b_AK8Z3_jt_intjet_multi;   //!
   TBranch        *b_AK8Z3_jt_girth;   //!
   TBranch        *b_AK8Z3_jt_thrust;   //!
   TBranch        *b_AK8Z3_jt_LHA;   //!
   TBranch        *b_AK8Z3_jt_pTD;   //!
   TBranch        *b_AK8Z4_run;   //!
   TBranch        *b_AK8Z4_evt;   //!
   TBranch        *b_AK8Z4_lumi;   //!
   TBranch        *b_AK8Z4_nref;   //!
   TBranch        *b_AK8Z4_ncalo;   //!
   TBranch        *b_AK8Z4_rawpt;   //!
   TBranch        *b_AK8Z4_jtpt;   //!
   TBranch        *b_AK8Z4_jteta;   //!
   TBranch        *b_AK8Z4_jty;   //!
   TBranch        *b_AK8Z4_jtphi;   //!
   TBranch        *b_AK8Z4_jtpu;   //!
   TBranch        *b_AK8Z4_jtm;   //!
   TBranch        *b_AK8Z4_jtarea;   //!
   TBranch        *b_AK8Z4_jtdyn_split;   //!
   TBranch        *b_AK8Z4_jtdyn_eta;   //!
   TBranch        *b_AK8Z4_jtdyn_phi;   //!
   TBranch        *b_AK8Z4_jtdyn_deltaR;   //!
   TBranch        *b_AK8Z4_jtdyn_kt;   //!
   TBranch        *b_AK8Z4_jtdyn_z;   //!
   TBranch        *b_AK8Z4_jt_intjet_multi;   //!
   TBranch        *b_AK8Z4_jt_girth;   //!
   TBranch        *b_AK8Z4_jt_thrust;   //!
   TBranch        *b_AK8Z4_jt_LHA;   //!
   TBranch        *b_AK8Z4_jt_pTD;   //!
   TBranch        *b_AK8Z5_run;   //!
   TBranch        *b_AK8Z5_evt;   //!
   TBranch        *b_AK8Z5_lumi;   //!
   TBranch        *b_AK8Z5_nref;   //!
   TBranch        *b_AK8Z5_ncalo;   //!
   TBranch        *b_AK8Z5_rawpt;   //!
   TBranch        *b_AK8Z5_jtpt;   //!
   TBranch        *b_AK8Z5_jteta;   //!
   TBranch        *b_AK8Z5_jty;   //!
   TBranch        *b_AK8Z5_jtphi;   //!
   TBranch        *b_AK8Z5_jtpu;   //!
   TBranch        *b_AK8Z5_jtm;   //!
   TBranch        *b_AK8Z5_jtarea;   //!
   TBranch        *b_AK8Z5_jtdyn_split;   //!
   TBranch        *b_AK8Z5_jtdyn_eta;   //!
   TBranch        *b_AK8Z5_jtdyn_phi;   //!
   TBranch        *b_AK8Z5_jtdyn_deltaR;   //!
   TBranch        *b_AK8Z5_jtdyn_kt;   //!
   TBranch        *b_AK8Z5_jtdyn_z;   //!
   TBranch        *b_AK8Z5_jt_intjet_multi;   //!
   TBranch        *b_AK8Z5_jt_girth;   //!
   TBranch        *b_AK8Z5_jt_thrust;   //!
   TBranch        *b_AK8Z5_jt_LHA;   //!
   TBranch        *b_AK8Z5_jt_pTD;   //!
   TBranch        *b_ggHi_run;   //!
   TBranch        *b_ggHi_event;   //!
   TBranch        *b_ggHi_lumis;   //!
   TBranch        *b_ggHi_rho;   //!
   TBranch        *b_ggHi_nEle;   //!
   TBranch        *b_ggHi_eleD0;   //!
   TBranch        *b_ggHi_eleDz;   //!
   TBranch        *b_ggHi_eleD0Err;   //!
   TBranch        *b_ggHi_eleDzErr;   //!
   TBranch        *b_ggHi_eleTrkPt;   //!
   TBranch        *b_ggHi_eleTrkEta;   //!
   TBranch        *b_ggHi_eleTrkPhi;   //!
   TBranch        *b_ggHi_eleTrkCharge;   //!
   TBranch        *b_ggHi_eleTrkPtErr;   //!
   TBranch        *b_ggHi_eleTrkChi2;   //!
   TBranch        *b_ggHi_eleTrkNdof;   //!
   TBranch        *b_ggHi_eleTrkNormalizedChi2;   //!
   TBranch        *b_ggHi_eleTrkValidHits;   //!
   TBranch        *b_ggHi_eleTrkLayers;   //!
   TBranch        *b_ggHi_eleMissHits;   //!
   TBranch        *b_ggHi_eleIP3D;   //!
   TBranch        *b_ggHi_eleIP3DErr;   //!
   TBranch        *b_ggHi_elePt;   //!
   TBranch        *b_ggHi_eleEta;   //!
   TBranch        *b_ggHi_elePhi;   //!
   TBranch        *b_ggHi_eleCharge;   //!
   TBranch        *b_ggHi_eleEn;   //!
   TBranch        *b_ggHi_eleSCEn;   //!
   TBranch        *b_ggHi_eleESEn;   //!
   TBranch        *b_ggHi_eleSCEta;   //!
   TBranch        *b_ggHi_eleSCPhi;   //!
   TBranch        *b_ggHi_eleSCRawEn;   //!
   TBranch        *b_ggHi_eleSCEtaWidth;   //!
   TBranch        *b_ggHi_eleSCPhiWidth;   //!
   TBranch        *b_ggHi_eleSCClustersSize;   //!
   TBranch        *b_ggHi_eleSeedEn;   //!
   TBranch        *b_ggHi_eleSeedEta;   //!
   TBranch        *b_ggHi_eleSeedPhi;   //!
   TBranch        *b_ggHi_eleHoverE;   //!
   TBranch        *b_ggHi_eleHoverEBc;   //!
   TBranch        *b_ggHi_eleEoverP;   //!
   TBranch        *b_ggHi_eleEoverPInv;   //!
   TBranch        *b_ggHi_eleEcalE;   //!
   TBranch        *b_ggHi_elePAtVtx;   //!
   TBranch        *b_ggHi_elePAtSC;   //!
   TBranch        *b_ggHi_elePAtCluster;   //!
   TBranch        *b_ggHi_elePAtSeed;   //!
   TBranch        *b_ggHi_eledEtaAtVtx;   //!
   TBranch        *b_ggHi_eledPhiAtVtx;   //!
   TBranch        *b_ggHi_eledEtaSeedAtVtx;   //!
   TBranch        *b_ggHi_eleSigmaIEtaIEta;   //!
   TBranch        *b_ggHi_eleSigmaIPhiIPhi;   //!
   TBranch        *b_ggHi_eleBrem;   //!
   TBranch        *b_ggHi_eleConvVeto;   //!
   TBranch        *b_ggHi_eleR9;   //!
   TBranch        *b_ggHi_eleE3x3;   //!
   TBranch        *b_ggHi_eleE5x5;   //!
   TBranch        *b_ggHi_eleR9Full5x5;   //!
   TBranch        *b_ggHi_eleE3x3Full5x5;   //!
   TBranch        *b_ggHi_eleE5x5Full5x5;   //!
   TBranch        *b_ggHi_eleSigmaIEtaIEta_2012;   //!
   TBranch        *b_ggHi_elePFChIso;   //!
   TBranch        *b_ggHi_elePFPhoIso;   //!
   TBranch        *b_ggHi_elePFNeuIso;   //!
   TBranch        *b_ggHi_elePFPUIso;   //!
   TBranch        *b_ggHi_elePFRelIsoWithEA;   //!
   TBranch        *b_ggHi_elePFRelIsoWithDBeta;   //!
   TBranch        *b_ggHi_eleEffAreaTimesRho;   //!
   TBranch        *b_ggHi_elePFChIso03;   //!
   TBranch        *b_ggHi_elePFPhoIso03;   //!
   TBranch        *b_ggHi_elePFNeuIso03;   //!
   TBranch        *b_ggHi_elePFChIso04;   //!
   TBranch        *b_ggHi_elePFPhoIso04;   //!
   TBranch        *b_ggHi_elePFNeuIso04;   //!
   TBranch        *b_ggHi_eleSeedCryEta;   //!
   TBranch        *b_ggHi_eleSeedCryPhi;   //!
   TBranch        *b_ggHi_eleSeedCryIeta;   //!
   TBranch        *b_ggHi_eleSeedCryIphi;   //!
   TBranch        *b_ggHi_nPho;   //!
   TBranch        *b_ggHi_phoE;   //!
   TBranch        *b_ggHi_phoEt;   //!
   TBranch        *b_ggHi_phoEta;   //!
   TBranch        *b_ggHi_phoPhi;   //!
   TBranch        *b_ggHi_phoEcorrStdEcal;   //!
   TBranch        *b_ggHi_phoEcorrPhoEcal;   //!
   TBranch        *b_ggHi_phoEcorrRegr1;   //!
   TBranch        *b_ggHi_phoEcorrRegr2;   //!
   TBranch        *b_ggHi_phoEcorrErrStdEcal;   //!
   TBranch        *b_ggHi_phoEcorrErrPhoEcal;   //!
   TBranch        *b_ggHi_phoEcorrErrRegr1;   //!
   TBranch        *b_ggHi_phoEcorrErrRegr2;   //!
   TBranch        *b_ggHi_phoSCE;   //!
   TBranch        *b_ggHi_phoSCRawE;   //!
   TBranch        *b_ggHi_phoSCEta;   //!
   TBranch        *b_ggHi_phoSCPhi;   //!
   TBranch        *b_ggHi_phoSCEtaWidth;   //!
   TBranch        *b_ggHi_phoSCPhiWidth;   //!
   TBranch        *b_ggHi_phoSCBrem;   //!
   TBranch        *b_ggHi_phoSCnHits;   //!
   TBranch        *b_ggHi_phoSCflags;   //!
   TBranch        *b_ggHi_phoSCinClean;   //!
   TBranch        *b_ggHi_phoSCinUnClean;   //!
   TBranch        *b_ggHi_phoSCnBC;   //!
   TBranch        *b_ggHi_phoESEn;   //!
   TBranch        *b_ggHi_phoIsPFPhoton;   //!
   TBranch        *b_ggHi_phoIsStandardPhoton;   //!
   TBranch        *b_ggHi_phoHasPixelSeed;   //!
   TBranch        *b_ggHi_phoHasConversionTracks;   //!
   TBranch        *b_ggHi_phoHadTowerOverEm;   //!
   TBranch        *b_ggHi_phoHoverE;   //!
   TBranch        *b_ggHi_phoHoverEValid;   //!
   TBranch        *b_ggHi_phoSigmaIEtaIEta;   //!
   TBranch        *b_ggHi_phoR9;   //!
   TBranch        *b_ggHi_phoE1x5;   //!
   TBranch        *b_ggHi_phoE2x5;   //!
   TBranch        *b_ggHi_phoE3x3;   //!
   TBranch        *b_ggHi_phoE5x5;   //!
   TBranch        *b_ggHi_phoMaxEnergyXtal;   //!
   TBranch        *b_ggHi_phoSigmaEtaEta;   //!
   TBranch        *b_ggHi_phoSigmaIEtaIEta_2012;   //!
   TBranch        *b_ggHi_phoR9_2012;   //!
   TBranch        *b_ggHi_phoE1x5_2012;   //!
   TBranch        *b_ggHi_phoE2x5_2012;   //!
   TBranch        *b_ggHi_phoE3x3_2012;   //!
   TBranch        *b_ggHi_phoE5x5_2012;   //!
   TBranch        *b_ggHi_phoMaxEnergyXtal_2012;   //!
   TBranch        *b_ggHi_phoSigmaEtaEta_2012;   //!
   TBranch        *b_ggHi_phoHadTowerOverEm1;   //!
   TBranch        *b_ggHi_phoHadTowerOverEm2;   //!
   TBranch        *b_ggHi_phoHoverE1;   //!
   TBranch        *b_ggHi_phoHoverE2;   //!
   TBranch        *b_ggHi_phoSigmaIEtaIPhi;   //!
   TBranch        *b_ggHi_phoSigmaIPhiIPhi;   //!
   TBranch        *b_ggHi_phoR1x5;   //!
   TBranch        *b_ggHi_phoR2x5;   //!
   TBranch        *b_ggHi_phoE2nd;   //!
   TBranch        *b_ggHi_phoETop;   //!
   TBranch        *b_ggHi_phoEBottom;   //!
   TBranch        *b_ggHi_phoELeft;   //!
   TBranch        *b_ggHi_phoERight;   //!
   TBranch        *b_ggHi_phoE1x3;   //!
   TBranch        *b_ggHi_phoE2x2;   //!
   TBranch        *b_ggHi_phoE2x5Max;   //!
   TBranch        *b_ggHi_phoE2x5Top;   //!
   TBranch        *b_ggHi_phoE2x5Bottom;   //!
   TBranch        *b_ggHi_phoE2x5Left;   //!
   TBranch        *b_ggHi_phoE2x5Right;   //!
   TBranch        *b_ggHi_phoSigmaIEtaIPhi_2012;   //!
   TBranch        *b_ggHi_phoSigmaIPhiIPhi_2012;   //!
   TBranch        *b_ggHi_phoR1x5_2012;   //!
   TBranch        *b_ggHi_phoR2x5_2012;   //!
   TBranch        *b_ggHi_phoE2nd_2012;   //!
   TBranch        *b_ggHi_phoETop_2012;   //!
   TBranch        *b_ggHi_phoEBottom_2012;   //!
   TBranch        *b_ggHi_phoELeft_2012;   //!
   TBranch        *b_ggHi_phoERight_2012;   //!
   TBranch        *b_ggHi_phoE1x3_2012;   //!
   TBranch        *b_ggHi_phoE2x2_2012;   //!
   TBranch        *b_ggHi_phoE2x5Max_2012;   //!
   TBranch        *b_ggHi_phoE2x5Top_2012;   //!
   TBranch        *b_ggHi_phoE2x5Bottom_2012;   //!
   TBranch        *b_ggHi_phoE2x5Left_2012;   //!
   TBranch        *b_ggHi_phoE2x5Right_2012;   //!
   TBranch        *b_ggHi_phoBC1E;   //!
   TBranch        *b_ggHi_phoBC1Ecorr;   //!
   TBranch        *b_ggHi_phoBC1Eta;   //!
   TBranch        *b_ggHi_phoBC1Phi;   //!
   TBranch        *b_ggHi_phoBC1size;   //!
   TBranch        *b_ggHi_phoBC1flags;   //!
   TBranch        *b_ggHi_phoBC1inClean;   //!
   TBranch        *b_ggHi_phoBC1inUnClean;   //!
   TBranch        *b_ggHi_phoBC1rawID;   //!
   TBranch        *b_ggHi_pho_ecalClusterIsoR1;   //!
   TBranch        *b_ggHi_pho_ecalClusterIsoR2;   //!
   TBranch        *b_ggHi_pho_ecalClusterIsoR3;   //!
   TBranch        *b_ggHi_pho_ecalClusterIsoR4;   //!
   TBranch        *b_ggHi_pho_ecalClusterIsoR5;   //!
   TBranch        *b_ggHi_pho_hcalRechitIsoR1;   //!
   TBranch        *b_ggHi_pho_hcalRechitIsoR2;   //!
   TBranch        *b_ggHi_pho_hcalRechitIsoR3;   //!
   TBranch        *b_ggHi_pho_hcalRechitIsoR4;   //!
   TBranch        *b_ggHi_pho_hcalRechitIsoR5;   //!
   TBranch        *b_ggHi_pho_trackIsoR1PtCut20;   //!
   TBranch        *b_ggHi_pho_trackIsoR2PtCut20;   //!
   TBranch        *b_ggHi_pho_trackIsoR3PtCut20;   //!
   TBranch        *b_ggHi_pho_trackIsoR4PtCut20;   //!
   TBranch        *b_ggHi_pho_trackIsoR5PtCut20;   //!
   TBranch        *b_ggHi_pho_swissCrx;   //!
   TBranch        *b_ggHi_pho_seedTime;   //!
   TBranch        *b_ggHi_pfcIso1;   //!
   TBranch        *b_ggHi_pfcIso2;   //!
   TBranch        *b_ggHi_pfcIso3;   //!
   TBranch        *b_ggHi_pfcIso4;   //!
   TBranch        *b_ggHi_pfcIso5;   //!
   TBranch        *b_ggHi_pfpIso1;   //!
   TBranch        *b_ggHi_pfpIso2;   //!
   TBranch        *b_ggHi_pfpIso3;   //!
   TBranch        *b_ggHi_pfpIso4;   //!
   TBranch        *b_ggHi_pfpIso5;   //!
   TBranch        *b_ggHi_pfnIso1;   //!
   TBranch        *b_ggHi_pfnIso2;   //!
   TBranch        *b_ggHi_pfnIso3;   //!
   TBranch        *b_ggHi_pfnIso4;   //!
   TBranch        *b_ggHi_pfnIso5;   //!
   TBranch        *b_ggHi_pfpIso1subSC;   //!
   TBranch        *b_ggHi_pfpIso2subSC;   //!
   TBranch        *b_ggHi_pfpIso3subSC;   //!
   TBranch        *b_ggHi_pfpIso4subSC;   //!
   TBranch        *b_ggHi_pfpIso5subSC;   //!
   TBranch        *b_ggHi_pfcIso1subUE;   //!
   TBranch        *b_ggHi_pfcIso2subUE;   //!
   TBranch        *b_ggHi_pfcIso3subUE;   //!
   TBranch        *b_ggHi_pfcIso4subUE;   //!
   TBranch        *b_ggHi_pfcIso5subUE;   //!
   TBranch        *b_ggHi_pfpIso1subUE;   //!
   TBranch        *b_ggHi_pfpIso2subUE;   //!
   TBranch        *b_ggHi_pfpIso3subUE;   //!
   TBranch        *b_ggHi_pfpIso4subUE;   //!
   TBranch        *b_ggHi_pfpIso5subUE;   //!
   TBranch        *b_ggHi_pfnIso1subUE;   //!
   TBranch        *b_ggHi_pfnIso2subUE;   //!
   TBranch        *b_ggHi_pfnIso3subUE;   //!
   TBranch        *b_ggHi_pfnIso4subUE;   //!
   TBranch        *b_ggHi_pfnIso5subUE;   //!
   TBranch        *b_ggHi_pfpIso1subSCsubUE;   //!
   TBranch        *b_ggHi_pfpIso2subSCsubUE;   //!
   TBranch        *b_ggHi_pfpIso3subSCsubUE;   //!
   TBranch        *b_ggHi_pfpIso4subSCsubUE;   //!
   TBranch        *b_ggHi_pfpIso5subSCsubUE;   //!
   TBranch        *b_ggHi_pfcIso1pTgt1p0subUE;   //!
   TBranch        *b_ggHi_pfcIso2pTgt1p0subUE;   //!
   TBranch        *b_ggHi_pfcIso3pTgt1p0subUE;   //!
   TBranch        *b_ggHi_pfcIso4pTgt1p0subUE;   //!
   TBranch        *b_ggHi_pfcIso5pTgt1p0subUE;   //!
   TBranch        *b_ggHi_pfcIso1pTgt2p0subUE;   //!
   TBranch        *b_ggHi_pfcIso2pTgt2p0subUE;   //!
   TBranch        *b_ggHi_pfcIso3pTgt2p0subUE;   //!
   TBranch        *b_ggHi_pfcIso4pTgt2p0subUE;   //!
   TBranch        *b_ggHi_pfcIso5pTgt2p0subUE;   //!
   TBranch        *b_ggHi_pfcIso1pTgt3p0subUE;   //!
   TBranch        *b_ggHi_pfcIso2pTgt3p0subUE;   //!
   TBranch        *b_ggHi_pfcIso3pTgt3p0subUE;   //!
   TBranch        *b_ggHi_pfcIso4pTgt3p0subUE;   //!
   TBranch        *b_ggHi_pfcIso5pTgt3p0subUE;   //!
   TBranch        *b_ggHi_pfcIso2subUEec;   //!
   TBranch        *b_ggHi_pfcIso3subUEec;   //!
   TBranch        *b_ggHi_pfcIso4subUEec;   //!
   TBranch        *b_ggHi_pfpIso2subUEec;   //!
   TBranch        *b_ggHi_pfpIso3subUEec;   //!
   TBranch        *b_ggHi_pfpIso4subUEec;   //!
   TBranch        *b_ggHi_pfnIso2subUEec;   //!
   TBranch        *b_ggHi_pfnIso3subUEec;   //!
   TBranch        *b_ggHi_pfnIso4subUEec;   //!
   TBranch        *b_ggHi_pfcIso2pTgt2p0subUEec;   //!
   TBranch        *b_ggHi_pfcIso3pTgt2p0subUEec;   //!
   TBranch        *b_ggHi_pfcIso4pTgt2p0subUEec;   //!
   TBranch        *b_run;   //!
   TBranch        *b_evt;   //!
   TBranch        *b_lumi;   //!
   TBranch        *b_vx;   //!
   TBranch        *b_vy;   //!
   TBranch        *b_vz;   //!
   TBranch        *b_hiBin;   //!
   TBranch        *b_hiHF;   //!
   TBranch        *b_hlt_HLT_HIGEDPhoton10_v10;   //!
   TBranch        *b_hlt_HLT_HIGEDPhoton20_v10;   //!
   TBranch        *b_hlt_HLT_HIGEDPhoton30_v10;   //!
   TBranch        *b_hlt_HLT_HIGEDPhoton40_v10;   //!
   TBranch        *b_hlt_HLT_HIGEDPhoton50_v10;   //!
   TBranch        *b_hlt_HLT_HIGEDPhoton60_v10;   //!
   TBranch        *b_hlt_L1_SingleEG12_BptxAND;   //!
   TBranch        *b_hlt_L1_SingleEG15_BptxAND;   //!
   TBranch        *b_hlt_L1_SingleEG21_BptxAND;   //!
   TBranch        *b_hlt_L1_SingleEG30_BptxAND;   //!
   TBranch        *b_skim_pclusterCompatibilityFilter;   //!
   TBranch        *b_skim_pprimaryVertexFilter;   //!
   TBranch        *b_skim_pphfCoincFilter4Th2;   //!
   TBranch        *b_skim_pphfCoincFilter1Th3;   //!
   TBranch        *b_skim_pphfCoincFilter2Th3;   //!
   TBranch        *b_skim_pphfCoincFilter3Th3;   //!
   TBranch        *b_skim_pphfCoincFilter4Th3;   //!
   TBranch        *b_skim_pphfCoincFilter5Th3;   //!
   TBranch        *b_skim_pphfCoincFilter1Th4;   //!
   TBranch        *b_skim_pphfCoincFilter2Th4;   //!
   TBranch        *b_skim_pphfCoincFilter3Th4;   //!
   TBranch        *b_skim_pphfCoincFilter4Th4;   //!
   TBranch        *b_skim_pphfCoincFilter5Th4;   //!
   TBranch        *b_skim_pphfCoincFilter1Th5;   //!
   TBranch        *b_skim_pphfCoincFilter2Th5;   //!
   TBranch        *b_skim_pphfCoincFilter3Th5;   //!
   TBranch        *b_skim_pphfCoincFilter4Th5;   //!
   TBranch        *b_skim_pphfCoincFilter5Th5;   //!
   TBranch        *b_skim_pphfCoincFilter1Th6;   //!
   TBranch        *b_skim_pphfCoincFilter2Th6;   //!
   TBranch        *b_skim_pphfCoincFilter3Th6;   //!
   TBranch        *b_skim_pphfCoincFilter4Th6;   //!
   TBranch        *b_skim_pphfCoincFilter5Th6;   //!

   GammaJet2023_PbPbData(TTree *tree=0);
   virtual ~GammaJet2023_PbPbData();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef GammaJet2023_PbPbData_cxx
GammaJet2023_PbPbData::GammaJet2023_PbPbData(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("root://eoscms.cern.ch//eos/cms/store/group/phys_heavyions/bharikri/Run3GammaJet/2023_PbPb/Data/2025_05_22_Data_HiRawPrime0_part_skimmed_batch0_of_85.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("root://eoscms.cern.ch//eos/cms/store/group/phys_heavyions/bharikri/Run3GammaJet/2023_PbPb/Data/2025_05_22_Data_HiRawPrime0_part_skimmed_batch0_of_85.root");
      }
      f->GetObject("jet_tree",tree);

   }
   Init(tree);
}

GammaJet2023_PbPbData::~GammaJet2023_PbPbData()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t GammaJet2023_PbPbData::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t GammaJet2023_PbPbData::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void GammaJet2023_PbPbData::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set object pointer
   ggHi_eleD0 = 0;
   ggHi_eleDz = 0;
   ggHi_eleD0Err = 0;
   ggHi_eleDzErr = 0;
   ggHi_eleTrkPt = 0;
   ggHi_eleTrkEta = 0;
   ggHi_eleTrkPhi = 0;
   ggHi_eleTrkCharge = 0;
   ggHi_eleTrkPtErr = 0;
   ggHi_eleTrkChi2 = 0;
   ggHi_eleTrkNdof = 0;
   ggHi_eleTrkNormalizedChi2 = 0;
   ggHi_eleTrkValidHits = 0;
   ggHi_eleTrkLayers = 0;
   ggHi_eleMissHits = 0;
   ggHi_eleIP3D = 0;
   ggHi_eleIP3DErr = 0;
   ggHi_elePt = 0;
   ggHi_eleEta = 0;
   ggHi_elePhi = 0;
   ggHi_eleCharge = 0;
   ggHi_eleEn = 0;
   ggHi_eleSCEn = 0;
   ggHi_eleESEn = 0;
   ggHi_eleSCEta = 0;
   ggHi_eleSCPhi = 0;
   ggHi_eleSCRawEn = 0;
   ggHi_eleSCEtaWidth = 0;
   ggHi_eleSCPhiWidth = 0;
   ggHi_eleSCClustersSize = 0;
   ggHi_eleSeedEn = 0;
   ggHi_eleSeedEta = 0;
   ggHi_eleSeedPhi = 0;
   ggHi_eleHoverE = 0;
   ggHi_eleHoverEBc = 0;
   ggHi_eleEoverP = 0;
   ggHi_eleEoverPInv = 0;
   ggHi_eleEcalE = 0;
   ggHi_elePAtVtx = 0;
   ggHi_elePAtSC = 0;
   ggHi_elePAtCluster = 0;
   ggHi_elePAtSeed = 0;
   ggHi_eledEtaAtVtx = 0;
   ggHi_eledPhiAtVtx = 0;
   ggHi_eledEtaSeedAtVtx = 0;
   ggHi_eleSigmaIEtaIEta = 0;
   ggHi_eleSigmaIPhiIPhi = 0;
   ggHi_eleBrem = 0;
   ggHi_eleConvVeto = 0;
   ggHi_eleR9 = 0;
   ggHi_eleE3x3 = 0;
   ggHi_eleE5x5 = 0;
   ggHi_eleR9Full5x5 = 0;
   ggHi_eleE3x3Full5x5 = 0;
   ggHi_eleE5x5Full5x5 = 0;
   ggHi_eleSigmaIEtaIEta_2012 = 0;
   ggHi_elePFChIso = 0;
   ggHi_elePFPhoIso = 0;
   ggHi_elePFNeuIso = 0;
   ggHi_elePFPUIso = 0;
   ggHi_elePFRelIsoWithEA = 0;
   ggHi_elePFRelIsoWithDBeta = 0;
   ggHi_eleEffAreaTimesRho = 0;
   ggHi_elePFChIso03 = 0;
   ggHi_elePFPhoIso03 = 0;
   ggHi_elePFNeuIso03 = 0;
   ggHi_elePFChIso04 = 0;
   ggHi_elePFPhoIso04 = 0;
   ggHi_elePFNeuIso04 = 0;
   ggHi_eleSeedCryEta = 0;
   ggHi_eleSeedCryPhi = 0;
   ggHi_eleSeedCryIeta = 0;
   ggHi_eleSeedCryIphi = 0;
   ggHi_phoE = 0;
   ggHi_phoEt = 0;
   ggHi_phoEta = 0;
   ggHi_phoPhi = 0;
   ggHi_phoEcorrStdEcal = 0;
   ggHi_phoEcorrPhoEcal = 0;
   ggHi_phoEcorrRegr1 = 0;
   ggHi_phoEcorrRegr2 = 0;
   ggHi_phoEcorrErrStdEcal = 0;
   ggHi_phoEcorrErrPhoEcal = 0;
   ggHi_phoEcorrErrRegr1 = 0;
   ggHi_phoEcorrErrRegr2 = 0;
   ggHi_phoSCE = 0;
   ggHi_phoSCRawE = 0;
   ggHi_phoSCEta = 0;
   ggHi_phoSCPhi = 0;
   ggHi_phoSCEtaWidth = 0;
   ggHi_phoSCPhiWidth = 0;
   ggHi_phoSCBrem = 0;
   ggHi_phoSCnHits = 0;
   ggHi_phoSCflags = 0;
   ggHi_phoSCinClean = 0;
   ggHi_phoSCinUnClean = 0;
   ggHi_phoSCnBC = 0;
   ggHi_phoESEn = 0;
   ggHi_phoIsPFPhoton = 0;
   ggHi_phoIsStandardPhoton = 0;
   ggHi_phoHasPixelSeed = 0;
   ggHi_phoHasConversionTracks = 0;
   ggHi_phoHadTowerOverEm = 0;
   ggHi_phoHoverE = 0;
   ggHi_phoHoverEValid = 0;
   ggHi_phoSigmaIEtaIEta = 0;
   ggHi_phoR9 = 0;
   ggHi_phoE1x5 = 0;
   ggHi_phoE2x5 = 0;
   ggHi_phoE3x3 = 0;
   ggHi_phoE5x5 = 0;
   ggHi_phoMaxEnergyXtal = 0;
   ggHi_phoSigmaEtaEta = 0;
   ggHi_phoSigmaIEtaIEta_2012 = 0;
   ggHi_phoR9_2012 = 0;
   ggHi_phoE1x5_2012 = 0;
   ggHi_phoE2x5_2012 = 0;
   ggHi_phoE3x3_2012 = 0;
   ggHi_phoE5x5_2012 = 0;
   ggHi_phoMaxEnergyXtal_2012 = 0;
   ggHi_phoSigmaEtaEta_2012 = 0;
   ggHi_phoHadTowerOverEm1 = 0;
   ggHi_phoHadTowerOverEm2 = 0;
   ggHi_phoHoverE1 = 0;
   ggHi_phoHoverE2 = 0;
   ggHi_phoSigmaIEtaIPhi = 0;
   ggHi_phoSigmaIPhiIPhi = 0;
   ggHi_phoR1x5 = 0;
   ggHi_phoR2x5 = 0;
   ggHi_phoE2nd = 0;
   ggHi_phoETop = 0;
   ggHi_phoEBottom = 0;
   ggHi_phoELeft = 0;
   ggHi_phoERight = 0;
   ggHi_phoE1x3 = 0;
   ggHi_phoE2x2 = 0;
   ggHi_phoE2x5Max = 0;
   ggHi_phoE2x5Top = 0;
   ggHi_phoE2x5Bottom = 0;
   ggHi_phoE2x5Left = 0;
   ggHi_phoE2x5Right = 0;
   ggHi_phoSigmaIEtaIPhi_2012 = 0;
   ggHi_phoSigmaIPhiIPhi_2012 = 0;
   ggHi_phoR1x5_2012 = 0;
   ggHi_phoR2x5_2012 = 0;
   ggHi_phoE2nd_2012 = 0;
   ggHi_phoETop_2012 = 0;
   ggHi_phoEBottom_2012 = 0;
   ggHi_phoELeft_2012 = 0;
   ggHi_phoERight_2012 = 0;
   ggHi_phoE1x3_2012 = 0;
   ggHi_phoE2x2_2012 = 0;
   ggHi_phoE2x5Max_2012 = 0;
   ggHi_phoE2x5Top_2012 = 0;
   ggHi_phoE2x5Bottom_2012 = 0;
   ggHi_phoE2x5Left_2012 = 0;
   ggHi_phoE2x5Right_2012 = 0;
   ggHi_phoBC1E = 0;
   ggHi_phoBC1Ecorr = 0;
   ggHi_phoBC1Eta = 0;
   ggHi_phoBC1Phi = 0;
   ggHi_phoBC1size = 0;
   ggHi_phoBC1flags = 0;
   ggHi_phoBC1inClean = 0;
   ggHi_phoBC1inUnClean = 0;
   ggHi_phoBC1rawID = 0;
   ggHi_pho_ecalClusterIsoR1 = 0;
   ggHi_pho_ecalClusterIsoR2 = 0;
   ggHi_pho_ecalClusterIsoR3 = 0;
   ggHi_pho_ecalClusterIsoR4 = 0;
   ggHi_pho_ecalClusterIsoR5 = 0;
   ggHi_pho_hcalRechitIsoR1 = 0;
   ggHi_pho_hcalRechitIsoR2 = 0;
   ggHi_pho_hcalRechitIsoR3 = 0;
   ggHi_pho_hcalRechitIsoR4 = 0;
   ggHi_pho_hcalRechitIsoR5 = 0;
   ggHi_pho_trackIsoR1PtCut20 = 0;
   ggHi_pho_trackIsoR2PtCut20 = 0;
   ggHi_pho_trackIsoR3PtCut20 = 0;
   ggHi_pho_trackIsoR4PtCut20 = 0;
   ggHi_pho_trackIsoR5PtCut20 = 0;
   ggHi_pho_swissCrx = 0;
   ggHi_pho_seedTime = 0;
   ggHi_pfcIso1 = 0;
   ggHi_pfcIso2 = 0;
   ggHi_pfcIso3 = 0;
   ggHi_pfcIso4 = 0;
   ggHi_pfcIso5 = 0;
   ggHi_pfpIso1 = 0;
   ggHi_pfpIso2 = 0;
   ggHi_pfpIso3 = 0;
   ggHi_pfpIso4 = 0;
   ggHi_pfpIso5 = 0;
   ggHi_pfnIso1 = 0;
   ggHi_pfnIso2 = 0;
   ggHi_pfnIso3 = 0;
   ggHi_pfnIso4 = 0;
   ggHi_pfnIso5 = 0;
   ggHi_pfpIso1subSC = 0;
   ggHi_pfpIso2subSC = 0;
   ggHi_pfpIso3subSC = 0;
   ggHi_pfpIso4subSC = 0;
   ggHi_pfpIso5subSC = 0;
   ggHi_pfcIso1subUE = 0;
   ggHi_pfcIso2subUE = 0;
   ggHi_pfcIso3subUE = 0;
   ggHi_pfcIso4subUE = 0;
   ggHi_pfcIso5subUE = 0;
   ggHi_pfpIso1subUE = 0;
   ggHi_pfpIso2subUE = 0;
   ggHi_pfpIso3subUE = 0;
   ggHi_pfpIso4subUE = 0;
   ggHi_pfpIso5subUE = 0;
   ggHi_pfnIso1subUE = 0;
   ggHi_pfnIso2subUE = 0;
   ggHi_pfnIso3subUE = 0;
   ggHi_pfnIso4subUE = 0;
   ggHi_pfnIso5subUE = 0;
   ggHi_pfpIso1subSCsubUE = 0;
   ggHi_pfpIso2subSCsubUE = 0;
   ggHi_pfpIso3subSCsubUE = 0;
   ggHi_pfpIso4subSCsubUE = 0;
   ggHi_pfpIso5subSCsubUE = 0;
   ggHi_pfcIso1pTgt1p0subUE = 0;
   ggHi_pfcIso2pTgt1p0subUE = 0;
   ggHi_pfcIso3pTgt1p0subUE = 0;
   ggHi_pfcIso4pTgt1p0subUE = 0;
   ggHi_pfcIso5pTgt1p0subUE = 0;
   ggHi_pfcIso1pTgt2p0subUE = 0;
   ggHi_pfcIso2pTgt2p0subUE = 0;
   ggHi_pfcIso3pTgt2p0subUE = 0;
   ggHi_pfcIso4pTgt2p0subUE = 0;
   ggHi_pfcIso5pTgt2p0subUE = 0;
   ggHi_pfcIso1pTgt3p0subUE = 0;
   ggHi_pfcIso2pTgt3p0subUE = 0;
   ggHi_pfcIso3pTgt3p0subUE = 0;
   ggHi_pfcIso4pTgt3p0subUE = 0;
   ggHi_pfcIso5pTgt3p0subUE = 0;
   ggHi_pfcIso2subUEec = 0;
   ggHi_pfcIso3subUEec = 0;
   ggHi_pfcIso4subUEec = 0;
   ggHi_pfpIso2subUEec = 0;
   ggHi_pfpIso3subUEec = 0;
   ggHi_pfpIso4subUEec = 0;
   ggHi_pfnIso2subUEec = 0;
   ggHi_pfnIso3subUEec = 0;
   ggHi_pfnIso4subUEec = 0;
   ggHi_pfcIso2pTgt2p0subUEec = 0;
   ggHi_pfcIso3pTgt2p0subUEec = 0;
   ggHi_pfcIso4pTgt2p0subUEec = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("AK2Z1_run", &AK2Z1_run, &b_AK2Z1_run);
   fChain->SetBranchAddress("AK2Z1_evt", &AK2Z1_evt, &b_AK2Z1_evt);
   fChain->SetBranchAddress("AK2Z1_lumi", &AK2Z1_lumi, &b_AK2Z1_lumi);
   fChain->SetBranchAddress("AK2Z1_nref", &AK2Z1_nref, &b_AK2Z1_nref);
   fChain->SetBranchAddress("AK2Z1_ncalo", &AK2Z1_ncalo, &b_AK2Z1_ncalo);
   fChain->SetBranchAddress("nref", &nref, &b_nref);
   fChain->SetBranchAddress("AK2Z1_rawpt", AK2Z1_rawpt, &b_AK2Z1_rawpt);
   fChain->SetBranchAddress("AK2Z1_jtpt", AK2Z1_jtpt, &b_AK2Z1_jtpt);
   fChain->SetBranchAddress("AK2Z1_jteta", AK2Z1_jteta, &b_AK2Z1_jteta);
   fChain->SetBranchAddress("AK2Z1_jty", AK2Z1_jty, &b_AK2Z1_jty);
   fChain->SetBranchAddress("AK2Z1_jtphi", AK2Z1_jtphi, &b_AK2Z1_jtphi);
   fChain->SetBranchAddress("AK2Z1_jtpu", AK2Z1_jtpu, &b_AK2Z1_jtpu);
   fChain->SetBranchAddress("AK2Z1_jtm", AK2Z1_jtm, &b_AK2Z1_jtm);
   fChain->SetBranchAddress("AK2Z1_jtarea", AK2Z1_jtarea, &b_AK2Z1_jtarea);
   fChain->SetBranchAddress("AK2Z1_jtdyn_split", AK2Z1_jtdyn_split, &b_AK2Z1_jtdyn_split);
   fChain->SetBranchAddress("AK2Z1_jtdyn_eta", AK2Z1_jtdyn_eta, &b_AK2Z1_jtdyn_eta);
   fChain->SetBranchAddress("AK2Z1_jtdyn_phi", AK2Z1_jtdyn_phi, &b_AK2Z1_jtdyn_phi);
   fChain->SetBranchAddress("AK2Z1_jtdyn_deltaR", AK2Z1_jtdyn_deltaR, &b_AK2Z1_jtdyn_deltaR);
   fChain->SetBranchAddress("AK2Z1_jtdyn_kt", AK2Z1_jtdyn_kt, &b_AK2Z1_jtdyn_kt);
   fChain->SetBranchAddress("AK2Z1_jtdyn_z", AK2Z1_jtdyn_z, &b_AK2Z1_jtdyn_z);
   fChain->SetBranchAddress("AK2Z1_jt_intjet_multi", AK2Z1_jt_intjet_multi, &b_AK2Z1_jt_intjet_multi);
   fChain->SetBranchAddress("AK2Z1_jt_girth", AK2Z1_jt_girth, &b_AK2Z1_jt_girth);
   fChain->SetBranchAddress("AK2Z1_jt_thrust", AK2Z1_jt_thrust, &b_AK2Z1_jt_thrust);
   fChain->SetBranchAddress("AK2Z1_jt_LHA", AK2Z1_jt_LHA, &b_AK2Z1_jt_LHA);
   fChain->SetBranchAddress("AK2Z1_jt_pTD", AK2Z1_jt_pTD, &b_AK2Z1_jt_pTD);
   fChain->SetBranchAddress("AK2Z2_run", &AK2Z2_run, &b_AK2Z2_run);
   fChain->SetBranchAddress("AK2Z2_evt", &AK2Z2_evt, &b_AK2Z2_evt);
   fChain->SetBranchAddress("AK2Z2_lumi", &AK2Z2_lumi, &b_AK2Z2_lumi);
   fChain->SetBranchAddress("AK2Z2_nref", &AK2Z2_nref, &b_AK2Z2_nref);
   fChain->SetBranchAddress("AK2Z2_ncalo", &AK2Z2_ncalo, &b_AK2Z2_ncalo);
   fChain->SetBranchAddress("AK2Z2_rawpt", AK2Z2_rawpt, &b_AK2Z2_rawpt);
   fChain->SetBranchAddress("AK2Z2_jtpt", AK2Z2_jtpt, &b_AK2Z2_jtpt);
   fChain->SetBranchAddress("AK2Z2_jteta", AK2Z2_jteta, &b_AK2Z2_jteta);
   fChain->SetBranchAddress("AK2Z2_jty", AK2Z2_jty, &b_AK2Z2_jty);
   fChain->SetBranchAddress("AK2Z2_jtphi", AK2Z2_jtphi, &b_AK2Z2_jtphi);
   fChain->SetBranchAddress("AK2Z2_jtpu", AK2Z2_jtpu, &b_AK2Z2_jtpu);
   fChain->SetBranchAddress("AK2Z2_jtm", AK2Z2_jtm, &b_AK2Z2_jtm);
   fChain->SetBranchAddress("AK2Z2_jtarea", AK2Z2_jtarea, &b_AK2Z2_jtarea);
   fChain->SetBranchAddress("AK2Z2_jtdyn_split", AK2Z2_jtdyn_split, &b_AK2Z2_jtdyn_split);
   fChain->SetBranchAddress("AK2Z2_jtdyn_eta", AK2Z2_jtdyn_eta, &b_AK2Z2_jtdyn_eta);
   fChain->SetBranchAddress("AK2Z2_jtdyn_phi", AK2Z2_jtdyn_phi, &b_AK2Z2_jtdyn_phi);
   fChain->SetBranchAddress("AK2Z2_jtdyn_deltaR", AK2Z2_jtdyn_deltaR, &b_AK2Z2_jtdyn_deltaR);
   fChain->SetBranchAddress("AK2Z2_jtdyn_kt", AK2Z2_jtdyn_kt, &b_AK2Z2_jtdyn_kt);
   fChain->SetBranchAddress("AK2Z2_jtdyn_z", AK2Z2_jtdyn_z, &b_AK2Z2_jtdyn_z);
   fChain->SetBranchAddress("AK2Z2_jt_intjet_multi", AK2Z2_jt_intjet_multi, &b_AK2Z2_jt_intjet_multi);
   fChain->SetBranchAddress("AK2Z2_jt_girth", AK2Z2_jt_girth, &b_AK2Z2_jt_girth);
   fChain->SetBranchAddress("AK2Z2_jt_thrust", AK2Z2_jt_thrust, &b_AK2Z2_jt_thrust);
   fChain->SetBranchAddress("AK2Z2_jt_LHA", AK2Z2_jt_LHA, &b_AK2Z2_jt_LHA);
   fChain->SetBranchAddress("AK2Z2_jt_pTD", AK2Z2_jt_pTD, &b_AK2Z2_jt_pTD);
   fChain->SetBranchAddress("AK2Z3_run", &AK2Z3_run, &b_AK2Z3_run);
   fChain->SetBranchAddress("AK2Z3_evt", &AK2Z3_evt, &b_AK2Z3_evt);
   fChain->SetBranchAddress("AK2Z3_lumi", &AK2Z3_lumi, &b_AK2Z3_lumi);
   fChain->SetBranchAddress("AK2Z3_nref", &AK2Z3_nref, &b_AK2Z3_nref);
   fChain->SetBranchAddress("AK2Z3_ncalo", &AK2Z3_ncalo, &b_AK2Z3_ncalo);
   fChain->SetBranchAddress("AK2Z3_rawpt", AK2Z3_rawpt, &b_AK2Z3_rawpt);
   fChain->SetBranchAddress("AK2Z3_jtpt", AK2Z3_jtpt, &b_AK2Z3_jtpt);
   fChain->SetBranchAddress("AK2Z3_jteta", AK2Z3_jteta, &b_AK2Z3_jteta);
   fChain->SetBranchAddress("AK2Z3_jty", AK2Z3_jty, &b_AK2Z3_jty);
   fChain->SetBranchAddress("AK2Z3_jtphi", AK2Z3_jtphi, &b_AK2Z3_jtphi);
   fChain->SetBranchAddress("AK2Z3_jtpu", AK2Z3_jtpu, &b_AK2Z3_jtpu);
   fChain->SetBranchAddress("AK2Z3_jtm", AK2Z3_jtm, &b_AK2Z3_jtm);
   fChain->SetBranchAddress("AK2Z3_jtarea", AK2Z3_jtarea, &b_AK2Z3_jtarea);
   fChain->SetBranchAddress("AK2Z3_jtdyn_split", AK2Z3_jtdyn_split, &b_AK2Z3_jtdyn_split);
   fChain->SetBranchAddress("AK2Z3_jtdyn_eta", AK2Z3_jtdyn_eta, &b_AK2Z3_jtdyn_eta);
   fChain->SetBranchAddress("AK2Z3_jtdyn_phi", AK2Z3_jtdyn_phi, &b_AK2Z3_jtdyn_phi);
   fChain->SetBranchAddress("AK2Z3_jtdyn_deltaR", AK2Z3_jtdyn_deltaR, &b_AK2Z3_jtdyn_deltaR);
   fChain->SetBranchAddress("AK2Z3_jtdyn_kt", AK2Z3_jtdyn_kt, &b_AK2Z3_jtdyn_kt);
   fChain->SetBranchAddress("AK2Z3_jtdyn_z", AK2Z3_jtdyn_z, &b_AK2Z3_jtdyn_z);
   fChain->SetBranchAddress("AK2Z3_jt_intjet_multi", AK2Z3_jt_intjet_multi, &b_AK2Z3_jt_intjet_multi);
   fChain->SetBranchAddress("AK2Z3_jt_girth", AK2Z3_jt_girth, &b_AK2Z3_jt_girth);
   fChain->SetBranchAddress("AK2Z3_jt_thrust", AK2Z3_jt_thrust, &b_AK2Z3_jt_thrust);
   fChain->SetBranchAddress("AK2Z3_jt_LHA", AK2Z3_jt_LHA, &b_AK2Z3_jt_LHA);
   fChain->SetBranchAddress("AK2Z3_jt_pTD", AK2Z3_jt_pTD, &b_AK2Z3_jt_pTD);
   fChain->SetBranchAddress("AK2Z4_run", &AK2Z4_run, &b_AK2Z4_run);
   fChain->SetBranchAddress("AK2Z4_evt", &AK2Z4_evt, &b_AK2Z4_evt);
   fChain->SetBranchAddress("AK2Z4_lumi", &AK2Z4_lumi, &b_AK2Z4_lumi);
   fChain->SetBranchAddress("AK2Z4_nref", &AK2Z4_nref, &b_AK2Z4_nref);
   fChain->SetBranchAddress("AK2Z4_ncalo", &AK2Z4_ncalo, &b_AK2Z4_ncalo);
   fChain->SetBranchAddress("AK2Z4_rawpt", AK2Z4_rawpt, &b_AK2Z4_rawpt);
   fChain->SetBranchAddress("AK2Z4_jtpt", AK2Z4_jtpt, &b_AK2Z4_jtpt);
   fChain->SetBranchAddress("AK2Z4_jteta", AK2Z4_jteta, &b_AK2Z4_jteta);
   fChain->SetBranchAddress("AK2Z4_jty", AK2Z4_jty, &b_AK2Z4_jty);
   fChain->SetBranchAddress("AK2Z4_jtphi", AK2Z4_jtphi, &b_AK2Z4_jtphi);
   fChain->SetBranchAddress("AK2Z4_jtpu", AK2Z4_jtpu, &b_AK2Z4_jtpu);
   fChain->SetBranchAddress("AK2Z4_jtm", AK2Z4_jtm, &b_AK2Z4_jtm);
   fChain->SetBranchAddress("AK2Z4_jtarea", AK2Z4_jtarea, &b_AK2Z4_jtarea);
   fChain->SetBranchAddress("AK2Z4_jtdyn_split", AK2Z4_jtdyn_split, &b_AK2Z4_jtdyn_split);
   fChain->SetBranchAddress("AK2Z4_jtdyn_eta", AK2Z4_jtdyn_eta, &b_AK2Z4_jtdyn_eta);
   fChain->SetBranchAddress("AK2Z4_jtdyn_phi", AK2Z4_jtdyn_phi, &b_AK2Z4_jtdyn_phi);
   fChain->SetBranchAddress("AK2Z4_jtdyn_deltaR", AK2Z4_jtdyn_deltaR, &b_AK2Z4_jtdyn_deltaR);
   fChain->SetBranchAddress("AK2Z4_jtdyn_kt", AK2Z4_jtdyn_kt, &b_AK2Z4_jtdyn_kt);
   fChain->SetBranchAddress("AK2Z4_jtdyn_z", AK2Z4_jtdyn_z, &b_AK2Z4_jtdyn_z);
   fChain->SetBranchAddress("AK2Z4_jt_intjet_multi", AK2Z4_jt_intjet_multi, &b_AK2Z4_jt_intjet_multi);
   fChain->SetBranchAddress("AK2Z4_jt_girth", AK2Z4_jt_girth, &b_AK2Z4_jt_girth);
   fChain->SetBranchAddress("AK2Z4_jt_thrust", AK2Z4_jt_thrust, &b_AK2Z4_jt_thrust);
   fChain->SetBranchAddress("AK2Z4_jt_LHA", AK2Z4_jt_LHA, &b_AK2Z4_jt_LHA);
   fChain->SetBranchAddress("AK2Z4_jt_pTD", AK2Z4_jt_pTD, &b_AK2Z4_jt_pTD);
   fChain->SetBranchAddress("AK3Z1_run", &AK3Z1_run, &b_AK3Z1_run);
   fChain->SetBranchAddress("AK3Z1_evt", &AK3Z1_evt, &b_AK3Z1_evt);
   fChain->SetBranchAddress("AK3Z1_lumi", &AK3Z1_lumi, &b_AK3Z1_lumi);
   fChain->SetBranchAddress("AK3Z1_nref", &AK3Z1_nref, &b_AK3Z1_nref);
   fChain->SetBranchAddress("AK3Z1_ncalo", &AK3Z1_ncalo, &b_AK3Z1_ncalo);
   fChain->SetBranchAddress("AK3Z1_rawpt", AK3Z1_rawpt, &b_AK3Z1_rawpt);
   fChain->SetBranchAddress("AK3Z1_jtpt", AK3Z1_jtpt, &b_AK3Z1_jtpt);
   fChain->SetBranchAddress("AK3Z1_jteta", AK3Z1_jteta, &b_AK3Z1_jteta);
   fChain->SetBranchAddress("AK3Z1_jty", AK3Z1_jty, &b_AK3Z1_jty);
   fChain->SetBranchAddress("AK3Z1_jtphi", AK3Z1_jtphi, &b_AK3Z1_jtphi);
   fChain->SetBranchAddress("AK3Z1_jtpu", AK3Z1_jtpu, &b_AK3Z1_jtpu);
   fChain->SetBranchAddress("AK3Z1_jtm", AK3Z1_jtm, &b_AK3Z1_jtm);
   fChain->SetBranchAddress("AK3Z1_jtarea", AK3Z1_jtarea, &b_AK3Z1_jtarea);
   fChain->SetBranchAddress("AK3Z1_jtdyn_split", AK3Z1_jtdyn_split, &b_AK3Z1_jtdyn_split);
   fChain->SetBranchAddress("AK3Z1_jtdyn_eta", AK3Z1_jtdyn_eta, &b_AK3Z1_jtdyn_eta);
   fChain->SetBranchAddress("AK3Z1_jtdyn_phi", AK3Z1_jtdyn_phi, &b_AK3Z1_jtdyn_phi);
   fChain->SetBranchAddress("AK3Z1_jtdyn_deltaR", AK3Z1_jtdyn_deltaR, &b_AK3Z1_jtdyn_deltaR);
   fChain->SetBranchAddress("AK3Z1_jtdyn_kt", AK3Z1_jtdyn_kt, &b_AK3Z1_jtdyn_kt);
   fChain->SetBranchAddress("AK3Z1_jtdyn_z", AK3Z1_jtdyn_z, &b_AK3Z1_jtdyn_z);
   fChain->SetBranchAddress("AK3Z1_jt_intjet_multi", AK3Z1_jt_intjet_multi, &b_AK3Z1_jt_intjet_multi);
   fChain->SetBranchAddress("AK3Z1_jt_girth", AK3Z1_jt_girth, &b_AK3Z1_jt_girth);
   fChain->SetBranchAddress("AK3Z1_jt_thrust", AK3Z1_jt_thrust, &b_AK3Z1_jt_thrust);
   fChain->SetBranchAddress("AK3Z1_jt_LHA", AK3Z1_jt_LHA, &b_AK3Z1_jt_LHA);
   fChain->SetBranchAddress("AK3Z1_jt_pTD", AK3Z1_jt_pTD, &b_AK3Z1_jt_pTD);
   fChain->SetBranchAddress("AK3Z2_run", &AK3Z2_run, &b_AK3Z2_run);
   fChain->SetBranchAddress("AK3Z2_evt", &AK3Z2_evt, &b_AK3Z2_evt);
   fChain->SetBranchAddress("AK3Z2_lumi", &AK3Z2_lumi, &b_AK3Z2_lumi);
   fChain->SetBranchAddress("AK3Z2_nref", &AK3Z2_nref, &b_AK3Z2_nref);
   fChain->SetBranchAddress("AK3Z2_ncalo", &AK3Z2_ncalo, &b_AK3Z2_ncalo);
   fChain->SetBranchAddress("AK3Z2_rawpt", AK3Z2_rawpt, &b_AK3Z2_rawpt);
   fChain->SetBranchAddress("AK3Z2_jtpt", AK3Z2_jtpt, &b_AK3Z2_jtpt);
   fChain->SetBranchAddress("AK3Z2_jteta", AK3Z2_jteta, &b_AK3Z2_jteta);
   fChain->SetBranchAddress("AK3Z2_jty", AK3Z2_jty, &b_AK3Z2_jty);
   fChain->SetBranchAddress("AK3Z2_jtphi", AK3Z2_jtphi, &b_AK3Z2_jtphi);
   fChain->SetBranchAddress("AK3Z2_jtpu", AK3Z2_jtpu, &b_AK3Z2_jtpu);
   fChain->SetBranchAddress("AK3Z2_jtm", AK3Z2_jtm, &b_AK3Z2_jtm);
   fChain->SetBranchAddress("AK3Z2_jtarea", AK3Z2_jtarea, &b_AK3Z2_jtarea);
   fChain->SetBranchAddress("AK3Z2_jtdyn_split", AK3Z2_jtdyn_split, &b_AK3Z2_jtdyn_split);
   fChain->SetBranchAddress("AK3Z2_jtdyn_eta", AK3Z2_jtdyn_eta, &b_AK3Z2_jtdyn_eta);
   fChain->SetBranchAddress("AK3Z2_jtdyn_phi", AK3Z2_jtdyn_phi, &b_AK3Z2_jtdyn_phi);
   fChain->SetBranchAddress("AK3Z2_jtdyn_deltaR", AK3Z2_jtdyn_deltaR, &b_AK3Z2_jtdyn_deltaR);
   fChain->SetBranchAddress("AK3Z2_jtdyn_kt", AK3Z2_jtdyn_kt, &b_AK3Z2_jtdyn_kt);
   fChain->SetBranchAddress("AK3Z2_jtdyn_z", AK3Z2_jtdyn_z, &b_AK3Z2_jtdyn_z);
   fChain->SetBranchAddress("AK3Z2_jt_intjet_multi", AK3Z2_jt_intjet_multi, &b_AK3Z2_jt_intjet_multi);
   fChain->SetBranchAddress("AK3Z2_jt_girth", AK3Z2_jt_girth, &b_AK3Z2_jt_girth);
   fChain->SetBranchAddress("AK3Z2_jt_thrust", AK3Z2_jt_thrust, &b_AK3Z2_jt_thrust);
   fChain->SetBranchAddress("AK3Z2_jt_LHA", AK3Z2_jt_LHA, &b_AK3Z2_jt_LHA);
   fChain->SetBranchAddress("AK3Z2_jt_pTD", AK3Z2_jt_pTD, &b_AK3Z2_jt_pTD);
   fChain->SetBranchAddress("AK3Z3_run", &AK3Z3_run, &b_AK3Z3_run);
   fChain->SetBranchAddress("AK3Z3_evt", &AK3Z3_evt, &b_AK3Z3_evt);
   fChain->SetBranchAddress("AK3Z3_lumi", &AK3Z3_lumi, &b_AK3Z3_lumi);
   fChain->SetBranchAddress("AK3Z3_nref", &AK3Z3_nref, &b_AK3Z3_nref);
   fChain->SetBranchAddress("AK3Z3_ncalo", &AK3Z3_ncalo, &b_AK3Z3_ncalo);
   fChain->SetBranchAddress("AK3Z3_rawpt", AK3Z3_rawpt, &b_AK3Z3_rawpt);
   fChain->SetBranchAddress("AK3Z3_jtpt", AK3Z3_jtpt, &b_AK3Z3_jtpt);
   fChain->SetBranchAddress("AK3Z3_jteta", AK3Z3_jteta, &b_AK3Z3_jteta);
   fChain->SetBranchAddress("AK3Z3_jty", AK3Z3_jty, &b_AK3Z3_jty);
   fChain->SetBranchAddress("AK3Z3_jtphi", AK3Z3_jtphi, &b_AK3Z3_jtphi);
   fChain->SetBranchAddress("AK3Z3_jtpu", AK3Z3_jtpu, &b_AK3Z3_jtpu);
   fChain->SetBranchAddress("AK3Z3_jtm", AK3Z3_jtm, &b_AK3Z3_jtm);
   fChain->SetBranchAddress("AK3Z3_jtarea", AK3Z3_jtarea, &b_AK3Z3_jtarea);
   fChain->SetBranchAddress("AK3Z3_jtdyn_split", AK3Z3_jtdyn_split, &b_AK3Z3_jtdyn_split);
   fChain->SetBranchAddress("AK3Z3_jtdyn_eta", AK3Z3_jtdyn_eta, &b_AK3Z3_jtdyn_eta);
   fChain->SetBranchAddress("AK3Z3_jtdyn_phi", AK3Z3_jtdyn_phi, &b_AK3Z3_jtdyn_phi);
   fChain->SetBranchAddress("AK3Z3_jtdyn_deltaR", AK3Z3_jtdyn_deltaR, &b_AK3Z3_jtdyn_deltaR);
   fChain->SetBranchAddress("AK3Z3_jtdyn_kt", AK3Z3_jtdyn_kt, &b_AK3Z3_jtdyn_kt);
   fChain->SetBranchAddress("AK3Z3_jtdyn_z", AK3Z3_jtdyn_z, &b_AK3Z3_jtdyn_z);
   fChain->SetBranchAddress("AK3Z3_jt_intjet_multi", AK3Z3_jt_intjet_multi, &b_AK3Z3_jt_intjet_multi);
   fChain->SetBranchAddress("AK3Z3_jt_girth", AK3Z3_jt_girth, &b_AK3Z3_jt_girth);
   fChain->SetBranchAddress("AK3Z3_jt_thrust", AK3Z3_jt_thrust, &b_AK3Z3_jt_thrust);
   fChain->SetBranchAddress("AK3Z3_jt_LHA", AK3Z3_jt_LHA, &b_AK3Z3_jt_LHA);
   fChain->SetBranchAddress("AK3Z3_jt_pTD", AK3Z3_jt_pTD, &b_AK3Z3_jt_pTD);
   fChain->SetBranchAddress("AK3Z4_run", &AK3Z4_run, &b_AK3Z4_run);
   fChain->SetBranchAddress("AK3Z4_evt", &AK3Z4_evt, &b_AK3Z4_evt);
   fChain->SetBranchAddress("AK3Z4_lumi", &AK3Z4_lumi, &b_AK3Z4_lumi);
   fChain->SetBranchAddress("AK3Z4_nref", &AK3Z4_nref, &b_AK3Z4_nref);
   fChain->SetBranchAddress("AK3Z4_ncalo", &AK3Z4_ncalo, &b_AK3Z4_ncalo);
   fChain->SetBranchAddress("AK3Z4_rawpt", AK3Z4_rawpt, &b_AK3Z4_rawpt);
   fChain->SetBranchAddress("AK3Z4_jtpt", AK3Z4_jtpt, &b_AK3Z4_jtpt);
   fChain->SetBranchAddress("AK3Z4_jteta", AK3Z4_jteta, &b_AK3Z4_jteta);
   fChain->SetBranchAddress("AK3Z4_jty", AK3Z4_jty, &b_AK3Z4_jty);
   fChain->SetBranchAddress("AK3Z4_jtphi", AK3Z4_jtphi, &b_AK3Z4_jtphi);
   fChain->SetBranchAddress("AK3Z4_jtpu", AK3Z4_jtpu, &b_AK3Z4_jtpu);
   fChain->SetBranchAddress("AK3Z4_jtm", AK3Z4_jtm, &b_AK3Z4_jtm);
   fChain->SetBranchAddress("AK3Z4_jtarea", AK3Z4_jtarea, &b_AK3Z4_jtarea);
   fChain->SetBranchAddress("AK3Z4_jtdyn_split", AK3Z4_jtdyn_split, &b_AK3Z4_jtdyn_split);
   fChain->SetBranchAddress("AK3Z4_jtdyn_eta", AK3Z4_jtdyn_eta, &b_AK3Z4_jtdyn_eta);
   fChain->SetBranchAddress("AK3Z4_jtdyn_phi", AK3Z4_jtdyn_phi, &b_AK3Z4_jtdyn_phi);
   fChain->SetBranchAddress("AK3Z4_jtdyn_deltaR", AK3Z4_jtdyn_deltaR, &b_AK3Z4_jtdyn_deltaR);
   fChain->SetBranchAddress("AK3Z4_jtdyn_kt", AK3Z4_jtdyn_kt, &b_AK3Z4_jtdyn_kt);
   fChain->SetBranchAddress("AK3Z4_jtdyn_z", AK3Z4_jtdyn_z, &b_AK3Z4_jtdyn_z);
   fChain->SetBranchAddress("AK3Z4_jt_intjet_multi", AK3Z4_jt_intjet_multi, &b_AK3Z4_jt_intjet_multi);
   fChain->SetBranchAddress("AK3Z4_jt_girth", AK3Z4_jt_girth, &b_AK3Z4_jt_girth);
   fChain->SetBranchAddress("AK3Z4_jt_thrust", AK3Z4_jt_thrust, &b_AK3Z4_jt_thrust);
   fChain->SetBranchAddress("AK3Z4_jt_LHA", AK3Z4_jt_LHA, &b_AK3Z4_jt_LHA);
   fChain->SetBranchAddress("AK3Z4_jt_pTD", AK3Z4_jt_pTD, &b_AK3Z4_jt_pTD);
   fChain->SetBranchAddress("AK3Z5_run", &AK3Z5_run, &b_AK3Z5_run);
   fChain->SetBranchAddress("AK3Z5_evt", &AK3Z5_evt, &b_AK3Z5_evt);
   fChain->SetBranchAddress("AK3Z5_lumi", &AK3Z5_lumi, &b_AK3Z5_lumi);
   fChain->SetBranchAddress("AK3Z5_nref", &AK3Z5_nref, &b_AK3Z5_nref);
   fChain->SetBranchAddress("AK3Z5_ncalo", &AK3Z5_ncalo, &b_AK3Z5_ncalo);
   fChain->SetBranchAddress("AK3Z5_rawpt", AK3Z5_rawpt, &b_AK3Z5_rawpt);
   fChain->SetBranchAddress("AK3Z5_jtpt", AK3Z5_jtpt, &b_AK3Z5_jtpt);
   fChain->SetBranchAddress("AK3Z5_jteta", AK3Z5_jteta, &b_AK3Z5_jteta);
   fChain->SetBranchAddress("AK3Z5_jty", AK3Z5_jty, &b_AK3Z5_jty);
   fChain->SetBranchAddress("AK3Z5_jtphi", AK3Z5_jtphi, &b_AK3Z5_jtphi);
   fChain->SetBranchAddress("AK3Z5_jtpu", AK3Z5_jtpu, &b_AK3Z5_jtpu);
   fChain->SetBranchAddress("AK3Z5_jtm", AK3Z5_jtm, &b_AK3Z5_jtm);
   fChain->SetBranchAddress("AK3Z5_jtarea", AK3Z5_jtarea, &b_AK3Z5_jtarea);
   fChain->SetBranchAddress("AK3Z5_jtdyn_split", AK3Z5_jtdyn_split, &b_AK3Z5_jtdyn_split);
   fChain->SetBranchAddress("AK3Z5_jtdyn_eta", AK3Z5_jtdyn_eta, &b_AK3Z5_jtdyn_eta);
   fChain->SetBranchAddress("AK3Z5_jtdyn_phi", AK3Z5_jtdyn_phi, &b_AK3Z5_jtdyn_phi);
   fChain->SetBranchAddress("AK3Z5_jtdyn_deltaR", AK3Z5_jtdyn_deltaR, &b_AK3Z5_jtdyn_deltaR);
   fChain->SetBranchAddress("AK3Z5_jtdyn_kt", AK3Z5_jtdyn_kt, &b_AK3Z5_jtdyn_kt);
   fChain->SetBranchAddress("AK3Z5_jtdyn_z", AK3Z5_jtdyn_z, &b_AK3Z5_jtdyn_z);
   fChain->SetBranchAddress("AK3Z5_jt_intjet_multi", AK3Z5_jt_intjet_multi, &b_AK3Z5_jt_intjet_multi);
   fChain->SetBranchAddress("AK3Z5_jt_girth", AK3Z5_jt_girth, &b_AK3Z5_jt_girth);
   fChain->SetBranchAddress("AK3Z5_jt_thrust", AK3Z5_jt_thrust, &b_AK3Z5_jt_thrust);
   fChain->SetBranchAddress("AK3Z5_jt_LHA", AK3Z5_jt_LHA, &b_AK3Z5_jt_LHA);
   fChain->SetBranchAddress("AK3Z5_jt_pTD", AK3Z5_jt_pTD, &b_AK3Z5_jt_pTD);
   fChain->SetBranchAddress("AK4Z1_run", &AK4Z1_run, &b_AK4Z1_run);
   fChain->SetBranchAddress("AK4Z1_evt", &AK4Z1_evt, &b_AK4Z1_evt);
   fChain->SetBranchAddress("AK4Z1_lumi", &AK4Z1_lumi, &b_AK4Z1_lumi);
   fChain->SetBranchAddress("AK4Z1_nref", &AK4Z1_nref, &b_AK4Z1_nref);
   fChain->SetBranchAddress("AK4Z1_ncalo", &AK4Z1_ncalo, &b_AK4Z1_ncalo);
   fChain->SetBranchAddress("AK4Z1_rawpt", AK4Z1_rawpt, &b_AK4Z1_rawpt);
   fChain->SetBranchAddress("AK4Z1_jtpt", AK4Z1_jtpt, &b_AK4Z1_jtpt);
   fChain->SetBranchAddress("AK4Z1_jteta", AK4Z1_jteta, &b_AK4Z1_jteta);
   fChain->SetBranchAddress("AK4Z1_jty", AK4Z1_jty, &b_AK4Z1_jty);
   fChain->SetBranchAddress("AK4Z1_jtphi", AK4Z1_jtphi, &b_AK4Z1_jtphi);
   fChain->SetBranchAddress("AK4Z1_jtpu", AK4Z1_jtpu, &b_AK4Z1_jtpu);
   fChain->SetBranchAddress("AK4Z1_jtm", AK4Z1_jtm, &b_AK4Z1_jtm);
   fChain->SetBranchAddress("AK4Z1_jtarea", AK4Z1_jtarea, &b_AK4Z1_jtarea);
   fChain->SetBranchAddress("AK4Z1_jtdyn_split", AK4Z1_jtdyn_split, &b_AK4Z1_jtdyn_split);
   fChain->SetBranchAddress("AK4Z1_jtdyn_eta", AK4Z1_jtdyn_eta, &b_AK4Z1_jtdyn_eta);
   fChain->SetBranchAddress("AK4Z1_jtdyn_phi", AK4Z1_jtdyn_phi, &b_AK4Z1_jtdyn_phi);
   fChain->SetBranchAddress("AK4Z1_jtdyn_deltaR", AK4Z1_jtdyn_deltaR, &b_AK4Z1_jtdyn_deltaR);
   fChain->SetBranchAddress("AK4Z1_jtdyn_kt", AK4Z1_jtdyn_kt, &b_AK4Z1_jtdyn_kt);
   fChain->SetBranchAddress("AK4Z1_jtdyn_z", AK4Z1_jtdyn_z, &b_AK4Z1_jtdyn_z);
   fChain->SetBranchAddress("AK4Z1_jt_intjet_multi", AK4Z1_jt_intjet_multi, &b_AK4Z1_jt_intjet_multi);
   fChain->SetBranchAddress("AK4Z1_jt_girth", AK4Z1_jt_girth, &b_AK4Z1_jt_girth);
   fChain->SetBranchAddress("AK4Z1_jt_thrust", AK4Z1_jt_thrust, &b_AK4Z1_jt_thrust);
   fChain->SetBranchAddress("AK4Z1_jt_LHA", AK4Z1_jt_LHA, &b_AK4Z1_jt_LHA);
   fChain->SetBranchAddress("AK4Z1_jt_pTD", AK4Z1_jt_pTD, &b_AK4Z1_jt_pTD);
   fChain->SetBranchAddress("AK4Z2_run", &AK4Z2_run, &b_AK4Z2_run);
   fChain->SetBranchAddress("AK4Z2_evt", &AK4Z2_evt, &b_AK4Z2_evt);
   fChain->SetBranchAddress("AK4Z2_lumi", &AK4Z2_lumi, &b_AK4Z2_lumi);
   fChain->SetBranchAddress("AK4Z2_nref", &AK4Z2_nref, &b_AK4Z2_nref);
   fChain->SetBranchAddress("AK4Z2_ncalo", &AK4Z2_ncalo, &b_AK4Z2_ncalo);
   fChain->SetBranchAddress("AK4Z2_rawpt", AK4Z2_rawpt, &b_AK4Z2_rawpt);
   fChain->SetBranchAddress("AK4Z2_jtpt", AK4Z2_jtpt, &b_AK4Z2_jtpt);
   fChain->SetBranchAddress("AK4Z2_jteta", AK4Z2_jteta, &b_AK4Z2_jteta);
   fChain->SetBranchAddress("AK4Z2_jty", AK4Z2_jty, &b_AK4Z2_jty);
   fChain->SetBranchAddress("AK4Z2_jtphi", AK4Z2_jtphi, &b_AK4Z2_jtphi);
   fChain->SetBranchAddress("AK4Z2_jtpu", AK4Z2_jtpu, &b_AK4Z2_jtpu);
   fChain->SetBranchAddress("AK4Z2_jtm", AK4Z2_jtm, &b_AK4Z2_jtm);
   fChain->SetBranchAddress("AK4Z2_jtarea", AK4Z2_jtarea, &b_AK4Z2_jtarea);
   fChain->SetBranchAddress("AK4Z2_jtdyn_split", AK4Z2_jtdyn_split, &b_AK4Z2_jtdyn_split);
   fChain->SetBranchAddress("AK4Z2_jtdyn_eta", AK4Z2_jtdyn_eta, &b_AK4Z2_jtdyn_eta);
   fChain->SetBranchAddress("AK4Z2_jtdyn_phi", AK4Z2_jtdyn_phi, &b_AK4Z2_jtdyn_phi);
   fChain->SetBranchAddress("AK4Z2_jtdyn_deltaR", AK4Z2_jtdyn_deltaR, &b_AK4Z2_jtdyn_deltaR);
   fChain->SetBranchAddress("AK4Z2_jtdyn_kt", AK4Z2_jtdyn_kt, &b_AK4Z2_jtdyn_kt);
   fChain->SetBranchAddress("AK4Z2_jtdyn_z", AK4Z2_jtdyn_z, &b_AK4Z2_jtdyn_z);
   fChain->SetBranchAddress("AK4Z2_jt_intjet_multi", AK4Z2_jt_intjet_multi, &b_AK4Z2_jt_intjet_multi);
   fChain->SetBranchAddress("AK4Z2_jt_girth", AK4Z2_jt_girth, &b_AK4Z2_jt_girth);
   fChain->SetBranchAddress("AK4Z2_jt_thrust", AK4Z2_jt_thrust, &b_AK4Z2_jt_thrust);
   fChain->SetBranchAddress("AK4Z2_jt_LHA", AK4Z2_jt_LHA, &b_AK4Z2_jt_LHA);
   fChain->SetBranchAddress("AK4Z2_jt_pTD", AK4Z2_jt_pTD, &b_AK4Z2_jt_pTD);
   fChain->SetBranchAddress("AK4Z3_run", &AK4Z3_run, &b_AK4Z3_run);
   fChain->SetBranchAddress("AK4Z3_evt", &AK4Z3_evt, &b_AK4Z3_evt);
   fChain->SetBranchAddress("AK4Z3_lumi", &AK4Z3_lumi, &b_AK4Z3_lumi);
   fChain->SetBranchAddress("AK4Z3_nref", &AK4Z3_nref, &b_AK4Z3_nref);
   fChain->SetBranchAddress("AK4Z3_ncalo", &AK4Z3_ncalo, &b_AK4Z3_ncalo);
   fChain->SetBranchAddress("AK4Z3_rawpt", AK4Z3_rawpt, &b_AK4Z3_rawpt);
   fChain->SetBranchAddress("AK4Z3_jtpt", AK4Z3_jtpt, &b_AK4Z3_jtpt);
   fChain->SetBranchAddress("AK4Z3_jteta", AK4Z3_jteta, &b_AK4Z3_jteta);
   fChain->SetBranchAddress("AK4Z3_jty", AK4Z3_jty, &b_AK4Z3_jty);
   fChain->SetBranchAddress("AK4Z3_jtphi", AK4Z3_jtphi, &b_AK4Z3_jtphi);
   fChain->SetBranchAddress("AK4Z3_jtpu", AK4Z3_jtpu, &b_AK4Z3_jtpu);
   fChain->SetBranchAddress("AK4Z3_jtm", AK4Z3_jtm, &b_AK4Z3_jtm);
   fChain->SetBranchAddress("AK4Z3_jtarea", AK4Z3_jtarea, &b_AK4Z3_jtarea);
   fChain->SetBranchAddress("AK4Z3_jtdyn_split", AK4Z3_jtdyn_split, &b_AK4Z3_jtdyn_split);
   fChain->SetBranchAddress("AK4Z3_jtdyn_eta", AK4Z3_jtdyn_eta, &b_AK4Z3_jtdyn_eta);
   fChain->SetBranchAddress("AK4Z3_jtdyn_phi", AK4Z3_jtdyn_phi, &b_AK4Z3_jtdyn_phi);
   fChain->SetBranchAddress("AK4Z3_jtdyn_deltaR", AK4Z3_jtdyn_deltaR, &b_AK4Z3_jtdyn_deltaR);
   fChain->SetBranchAddress("AK4Z3_jtdyn_kt", AK4Z3_jtdyn_kt, &b_AK4Z3_jtdyn_kt);
   fChain->SetBranchAddress("AK4Z3_jtdyn_z", AK4Z3_jtdyn_z, &b_AK4Z3_jtdyn_z);
   fChain->SetBranchAddress("AK4Z3_jt_intjet_multi", AK4Z3_jt_intjet_multi, &b_AK4Z3_jt_intjet_multi);
   fChain->SetBranchAddress("AK4Z3_jt_girth", AK4Z3_jt_girth, &b_AK4Z3_jt_girth);
   fChain->SetBranchAddress("AK4Z3_jt_thrust", AK4Z3_jt_thrust, &b_AK4Z3_jt_thrust);
   fChain->SetBranchAddress("AK4Z3_jt_LHA", AK4Z3_jt_LHA, &b_AK4Z3_jt_LHA);
   fChain->SetBranchAddress("AK4Z3_jt_pTD", AK4Z3_jt_pTD, &b_AK4Z3_jt_pTD);
   fChain->SetBranchAddress("AK4Z4_run", &AK4Z4_run, &b_AK4Z4_run);
   fChain->SetBranchAddress("AK4Z4_evt", &AK4Z4_evt, &b_AK4Z4_evt);
   fChain->SetBranchAddress("AK4Z4_lumi", &AK4Z4_lumi, &b_AK4Z4_lumi);
   fChain->SetBranchAddress("AK4Z4_nref", &AK4Z4_nref, &b_AK4Z4_nref);
   fChain->SetBranchAddress("AK4Z4_ncalo", &AK4Z4_ncalo, &b_AK4Z4_ncalo);
   fChain->SetBranchAddress("AK4Z4_rawpt", AK4Z4_rawpt, &b_AK4Z4_rawpt);
   fChain->SetBranchAddress("AK4Z4_jtpt", AK4Z4_jtpt, &b_AK4Z4_jtpt);
   fChain->SetBranchAddress("AK4Z4_jteta", AK4Z4_jteta, &b_AK4Z4_jteta);
   fChain->SetBranchAddress("AK4Z4_jty", AK4Z4_jty, &b_AK4Z4_jty);
   fChain->SetBranchAddress("AK4Z4_jtphi", AK4Z4_jtphi, &b_AK4Z4_jtphi);
   fChain->SetBranchAddress("AK4Z4_jtpu", AK4Z4_jtpu, &b_AK4Z4_jtpu);
   fChain->SetBranchAddress("AK4Z4_jtm", AK4Z4_jtm, &b_AK4Z4_jtm);
   fChain->SetBranchAddress("AK4Z4_jtarea", AK4Z4_jtarea, &b_AK4Z4_jtarea);
   fChain->SetBranchAddress("AK4Z4_jtdyn_split", AK4Z4_jtdyn_split, &b_AK4Z4_jtdyn_split);
   fChain->SetBranchAddress("AK4Z4_jtdyn_eta", AK4Z4_jtdyn_eta, &b_AK4Z4_jtdyn_eta);
   fChain->SetBranchAddress("AK4Z4_jtdyn_phi", AK4Z4_jtdyn_phi, &b_AK4Z4_jtdyn_phi);
   fChain->SetBranchAddress("AK4Z4_jtdyn_deltaR", AK4Z4_jtdyn_deltaR, &b_AK4Z4_jtdyn_deltaR);
   fChain->SetBranchAddress("AK4Z4_jtdyn_kt", AK4Z4_jtdyn_kt, &b_AK4Z4_jtdyn_kt);
   fChain->SetBranchAddress("AK4Z4_jtdyn_z", AK4Z4_jtdyn_z, &b_AK4Z4_jtdyn_z);
   fChain->SetBranchAddress("AK4Z4_jt_intjet_multi", AK4Z4_jt_intjet_multi, &b_AK4Z4_jt_intjet_multi);
   fChain->SetBranchAddress("AK4Z4_jt_girth", AK4Z4_jt_girth, &b_AK4Z4_jt_girth);
   fChain->SetBranchAddress("AK4Z4_jt_thrust", AK4Z4_jt_thrust, &b_AK4Z4_jt_thrust);
   fChain->SetBranchAddress("AK4Z4_jt_LHA", AK4Z4_jt_LHA, &b_AK4Z4_jt_LHA);
   fChain->SetBranchAddress("AK4Z4_jt_pTD", AK4Z4_jt_pTD, &b_AK4Z4_jt_pTD);
   fChain->SetBranchAddress("AK4Z5_run", &AK4Z5_run, &b_AK4Z5_run);
   fChain->SetBranchAddress("AK4Z5_evt", &AK4Z5_evt, &b_AK4Z5_evt);
   fChain->SetBranchAddress("AK4Z5_lumi", &AK4Z5_lumi, &b_AK4Z5_lumi);
   fChain->SetBranchAddress("AK4Z5_nref", &AK4Z5_nref, &b_AK4Z5_nref);
   fChain->SetBranchAddress("AK4Z5_ncalo", &AK4Z5_ncalo, &b_AK4Z5_ncalo);
   fChain->SetBranchAddress("AK4Z5_rawpt", AK4Z5_rawpt, &b_AK4Z5_rawpt);
   fChain->SetBranchAddress("AK4Z5_jtpt", AK4Z5_jtpt, &b_AK4Z5_jtpt);
   fChain->SetBranchAddress("AK4Z5_jteta", AK4Z5_jteta, &b_AK4Z5_jteta);
   fChain->SetBranchAddress("AK4Z5_jty", AK4Z5_jty, &b_AK4Z5_jty);
   fChain->SetBranchAddress("AK4Z5_jtphi", AK4Z5_jtphi, &b_AK4Z5_jtphi);
   fChain->SetBranchAddress("AK4Z5_jtpu", AK4Z5_jtpu, &b_AK4Z5_jtpu);
   fChain->SetBranchAddress("AK4Z5_jtm", AK4Z5_jtm, &b_AK4Z5_jtm);
   fChain->SetBranchAddress("AK4Z5_jtarea", AK4Z5_jtarea, &b_AK4Z5_jtarea);
   fChain->SetBranchAddress("AK4Z5_jtdyn_split", AK4Z5_jtdyn_split, &b_AK4Z5_jtdyn_split);
   fChain->SetBranchAddress("AK4Z5_jtdyn_eta", AK4Z5_jtdyn_eta, &b_AK4Z5_jtdyn_eta);
   fChain->SetBranchAddress("AK4Z5_jtdyn_phi", AK4Z5_jtdyn_phi, &b_AK4Z5_jtdyn_phi);
   fChain->SetBranchAddress("AK4Z5_jtdyn_deltaR", AK4Z5_jtdyn_deltaR, &b_AK4Z5_jtdyn_deltaR);
   fChain->SetBranchAddress("AK4Z5_jtdyn_kt", AK4Z5_jtdyn_kt, &b_AK4Z5_jtdyn_kt);
   fChain->SetBranchAddress("AK4Z5_jtdyn_z", AK4Z5_jtdyn_z, &b_AK4Z5_jtdyn_z);
   fChain->SetBranchAddress("AK4Z5_jt_intjet_multi", AK4Z5_jt_intjet_multi, &b_AK4Z5_jt_intjet_multi);
   fChain->SetBranchAddress("AK4Z5_jt_girth", AK4Z5_jt_girth, &b_AK4Z5_jt_girth);
   fChain->SetBranchAddress("AK4Z5_jt_thrust", AK4Z5_jt_thrust, &b_AK4Z5_jt_thrust);
   fChain->SetBranchAddress("AK4Z5_jt_LHA", AK4Z5_jt_LHA, &b_AK4Z5_jt_LHA);
   fChain->SetBranchAddress("AK4Z5_jt_pTD", AK4Z5_jt_pTD, &b_AK4Z5_jt_pTD);
   fChain->SetBranchAddress("AK5Z1_run", &AK5Z1_run, &b_AK5Z1_run);
   fChain->SetBranchAddress("AK5Z1_evt", &AK5Z1_evt, &b_AK5Z1_evt);
   fChain->SetBranchAddress("AK5Z1_lumi", &AK5Z1_lumi, &b_AK5Z1_lumi);
   fChain->SetBranchAddress("AK5Z1_nref", &AK5Z1_nref, &b_AK5Z1_nref);
   fChain->SetBranchAddress("AK5Z1_ncalo", &AK5Z1_ncalo, &b_AK5Z1_ncalo);
   fChain->SetBranchAddress("AK5Z1_rawpt", AK5Z1_rawpt, &b_AK5Z1_rawpt);
   fChain->SetBranchAddress("AK5Z1_jtpt", AK5Z1_jtpt, &b_AK5Z1_jtpt);
   fChain->SetBranchAddress("AK5Z1_jteta", AK5Z1_jteta, &b_AK5Z1_jteta);
   fChain->SetBranchAddress("AK5Z1_jty", AK5Z1_jty, &b_AK5Z1_jty);
   fChain->SetBranchAddress("AK5Z1_jtphi", AK5Z1_jtphi, &b_AK5Z1_jtphi);
   fChain->SetBranchAddress("AK5Z1_jtpu", AK5Z1_jtpu, &b_AK5Z1_jtpu);
   fChain->SetBranchAddress("AK5Z1_jtm", AK5Z1_jtm, &b_AK5Z1_jtm);
   fChain->SetBranchAddress("AK5Z1_jtarea", AK5Z1_jtarea, &b_AK5Z1_jtarea);
   fChain->SetBranchAddress("AK5Z1_jtdyn_split", AK5Z1_jtdyn_split, &b_AK5Z1_jtdyn_split);
   fChain->SetBranchAddress("AK5Z1_jtdyn_eta", AK5Z1_jtdyn_eta, &b_AK5Z1_jtdyn_eta);
   fChain->SetBranchAddress("AK5Z1_jtdyn_phi", AK5Z1_jtdyn_phi, &b_AK5Z1_jtdyn_phi);
   fChain->SetBranchAddress("AK5Z1_jtdyn_deltaR", AK5Z1_jtdyn_deltaR, &b_AK5Z1_jtdyn_deltaR);
   fChain->SetBranchAddress("AK5Z1_jtdyn_kt", AK5Z1_jtdyn_kt, &b_AK5Z1_jtdyn_kt);
   fChain->SetBranchAddress("AK5Z1_jtdyn_z", AK5Z1_jtdyn_z, &b_AK5Z1_jtdyn_z);
   fChain->SetBranchAddress("AK5Z1_jt_intjet_multi", AK5Z1_jt_intjet_multi, &b_AK5Z1_jt_intjet_multi);
   fChain->SetBranchAddress("AK5Z1_jt_girth", AK5Z1_jt_girth, &b_AK5Z1_jt_girth);
   fChain->SetBranchAddress("AK5Z1_jt_thrust", AK5Z1_jt_thrust, &b_AK5Z1_jt_thrust);
   fChain->SetBranchAddress("AK5Z1_jt_LHA", AK5Z1_jt_LHA, &b_AK5Z1_jt_LHA);
   fChain->SetBranchAddress("AK5Z1_jt_pTD", AK5Z1_jt_pTD, &b_AK5Z1_jt_pTD);
   fChain->SetBranchAddress("AK5Z2_run", &AK5Z2_run, &b_AK5Z2_run);
   fChain->SetBranchAddress("AK5Z2_evt", &AK5Z2_evt, &b_AK5Z2_evt);
   fChain->SetBranchAddress("AK5Z2_lumi", &AK5Z2_lumi, &b_AK5Z2_lumi);
   fChain->SetBranchAddress("AK5Z2_nref", &AK5Z2_nref, &b_AK5Z2_nref);
   fChain->SetBranchAddress("AK5Z2_ncalo", &AK5Z2_ncalo, &b_AK5Z2_ncalo);
   fChain->SetBranchAddress("AK5Z2_rawpt", AK5Z2_rawpt, &b_AK5Z2_rawpt);
   fChain->SetBranchAddress("AK5Z2_jtpt", AK5Z2_jtpt, &b_AK5Z2_jtpt);
   fChain->SetBranchAddress("AK5Z2_jteta", AK5Z2_jteta, &b_AK5Z2_jteta);
   fChain->SetBranchAddress("AK5Z2_jty", AK5Z2_jty, &b_AK5Z2_jty);
   fChain->SetBranchAddress("AK5Z2_jtphi", AK5Z2_jtphi, &b_AK5Z2_jtphi);
   fChain->SetBranchAddress("AK5Z2_jtpu", AK5Z2_jtpu, &b_AK5Z2_jtpu);
   fChain->SetBranchAddress("AK5Z2_jtm", AK5Z2_jtm, &b_AK5Z2_jtm);
   fChain->SetBranchAddress("AK5Z2_jtarea", AK5Z2_jtarea, &b_AK5Z2_jtarea);
   fChain->SetBranchAddress("AK5Z2_jtdyn_split", AK5Z2_jtdyn_split, &b_AK5Z2_jtdyn_split);
   fChain->SetBranchAddress("AK5Z2_jtdyn_eta", AK5Z2_jtdyn_eta, &b_AK5Z2_jtdyn_eta);
   fChain->SetBranchAddress("AK5Z2_jtdyn_phi", AK5Z2_jtdyn_phi, &b_AK5Z2_jtdyn_phi);
   fChain->SetBranchAddress("AK5Z2_jtdyn_deltaR", AK5Z2_jtdyn_deltaR, &b_AK5Z2_jtdyn_deltaR);
   fChain->SetBranchAddress("AK5Z2_jtdyn_kt", AK5Z2_jtdyn_kt, &b_AK5Z2_jtdyn_kt);
   fChain->SetBranchAddress("AK5Z2_jtdyn_z", AK5Z2_jtdyn_z, &b_AK5Z2_jtdyn_z);
   fChain->SetBranchAddress("AK5Z2_jt_intjet_multi", AK5Z2_jt_intjet_multi, &b_AK5Z2_jt_intjet_multi);
   fChain->SetBranchAddress("AK5Z2_jt_girth", AK5Z2_jt_girth, &b_AK5Z2_jt_girth);
   fChain->SetBranchAddress("AK5Z2_jt_thrust", AK5Z2_jt_thrust, &b_AK5Z2_jt_thrust);
   fChain->SetBranchAddress("AK5Z2_jt_LHA", AK5Z2_jt_LHA, &b_AK5Z2_jt_LHA);
   fChain->SetBranchAddress("AK5Z2_jt_pTD", AK5Z2_jt_pTD, &b_AK5Z2_jt_pTD);
   fChain->SetBranchAddress("AK5Z3_run", &AK5Z3_run, &b_AK5Z3_run);
   fChain->SetBranchAddress("AK5Z3_evt", &AK5Z3_evt, &b_AK5Z3_evt);
   fChain->SetBranchAddress("AK5Z3_lumi", &AK5Z3_lumi, &b_AK5Z3_lumi);
   fChain->SetBranchAddress("AK5Z3_nref", &AK5Z3_nref, &b_AK5Z3_nref);
   fChain->SetBranchAddress("AK5Z3_ncalo", &AK5Z3_ncalo, &b_AK5Z3_ncalo);
   fChain->SetBranchAddress("AK5Z3_rawpt", AK5Z3_rawpt, &b_AK5Z3_rawpt);
   fChain->SetBranchAddress("AK5Z3_jtpt", AK5Z3_jtpt, &b_AK5Z3_jtpt);
   fChain->SetBranchAddress("AK5Z3_jteta", AK5Z3_jteta, &b_AK5Z3_jteta);
   fChain->SetBranchAddress("AK5Z3_jty", AK5Z3_jty, &b_AK5Z3_jty);
   fChain->SetBranchAddress("AK5Z3_jtphi", AK5Z3_jtphi, &b_AK5Z3_jtphi);
   fChain->SetBranchAddress("AK5Z3_jtpu", AK5Z3_jtpu, &b_AK5Z3_jtpu);
   fChain->SetBranchAddress("AK5Z3_jtm", AK5Z3_jtm, &b_AK5Z3_jtm);
   fChain->SetBranchAddress("AK5Z3_jtarea", AK5Z3_jtarea, &b_AK5Z3_jtarea);
   fChain->SetBranchAddress("AK5Z3_jtdyn_split", AK5Z3_jtdyn_split, &b_AK5Z3_jtdyn_split);
   fChain->SetBranchAddress("AK5Z3_jtdyn_eta", AK5Z3_jtdyn_eta, &b_AK5Z3_jtdyn_eta);
   fChain->SetBranchAddress("AK5Z3_jtdyn_phi", AK5Z3_jtdyn_phi, &b_AK5Z3_jtdyn_phi);
   fChain->SetBranchAddress("AK5Z3_jtdyn_deltaR", AK5Z3_jtdyn_deltaR, &b_AK5Z3_jtdyn_deltaR);
   fChain->SetBranchAddress("AK5Z3_jtdyn_kt", AK5Z3_jtdyn_kt, &b_AK5Z3_jtdyn_kt);
   fChain->SetBranchAddress("AK5Z3_jtdyn_z", AK5Z3_jtdyn_z, &b_AK5Z3_jtdyn_z);
   fChain->SetBranchAddress("AK5Z3_jt_intjet_multi", AK5Z3_jt_intjet_multi, &b_AK5Z3_jt_intjet_multi);
   fChain->SetBranchAddress("AK5Z3_jt_girth", AK5Z3_jt_girth, &b_AK5Z3_jt_girth);
   fChain->SetBranchAddress("AK5Z3_jt_thrust", AK5Z3_jt_thrust, &b_AK5Z3_jt_thrust);
   fChain->SetBranchAddress("AK5Z3_jt_LHA", AK5Z3_jt_LHA, &b_AK5Z3_jt_LHA);
   fChain->SetBranchAddress("AK5Z3_jt_pTD", AK5Z3_jt_pTD, &b_AK5Z3_jt_pTD);
   fChain->SetBranchAddress("AK5Z4_run", &AK5Z4_run, &b_AK5Z4_run);
   fChain->SetBranchAddress("AK5Z4_evt", &AK5Z4_evt, &b_AK5Z4_evt);
   fChain->SetBranchAddress("AK5Z4_lumi", &AK5Z4_lumi, &b_AK5Z4_lumi);
   fChain->SetBranchAddress("AK5Z4_nref", &AK5Z4_nref, &b_AK5Z4_nref);
   fChain->SetBranchAddress("AK5Z4_ncalo", &AK5Z4_ncalo, &b_AK5Z4_ncalo);
   fChain->SetBranchAddress("AK5Z4_rawpt", AK5Z4_rawpt, &b_AK5Z4_rawpt);
   fChain->SetBranchAddress("AK5Z4_jtpt", AK5Z4_jtpt, &b_AK5Z4_jtpt);
   fChain->SetBranchAddress("AK5Z4_jteta", AK5Z4_jteta, &b_AK5Z4_jteta);
   fChain->SetBranchAddress("AK5Z4_jty", AK5Z4_jty, &b_AK5Z4_jty);
   fChain->SetBranchAddress("AK5Z4_jtphi", AK5Z4_jtphi, &b_AK5Z4_jtphi);
   fChain->SetBranchAddress("AK5Z4_jtpu", AK5Z4_jtpu, &b_AK5Z4_jtpu);
   fChain->SetBranchAddress("AK5Z4_jtm", AK5Z4_jtm, &b_AK5Z4_jtm);
   fChain->SetBranchAddress("AK5Z4_jtarea", AK5Z4_jtarea, &b_AK5Z4_jtarea);
   fChain->SetBranchAddress("AK5Z4_jtdyn_split", AK5Z4_jtdyn_split, &b_AK5Z4_jtdyn_split);
   fChain->SetBranchAddress("AK5Z4_jtdyn_eta", AK5Z4_jtdyn_eta, &b_AK5Z4_jtdyn_eta);
   fChain->SetBranchAddress("AK5Z4_jtdyn_phi", AK5Z4_jtdyn_phi, &b_AK5Z4_jtdyn_phi);
   fChain->SetBranchAddress("AK5Z4_jtdyn_deltaR", AK5Z4_jtdyn_deltaR, &b_AK5Z4_jtdyn_deltaR);
   fChain->SetBranchAddress("AK5Z4_jtdyn_kt", AK5Z4_jtdyn_kt, &b_AK5Z4_jtdyn_kt);
   fChain->SetBranchAddress("AK5Z4_jtdyn_z", AK5Z4_jtdyn_z, &b_AK5Z4_jtdyn_z);
   fChain->SetBranchAddress("AK5Z4_jt_intjet_multi", AK5Z4_jt_intjet_multi, &b_AK5Z4_jt_intjet_multi);
   fChain->SetBranchAddress("AK5Z4_jt_girth", AK5Z4_jt_girth, &b_AK5Z4_jt_girth);
   fChain->SetBranchAddress("AK5Z4_jt_thrust", AK5Z4_jt_thrust, &b_AK5Z4_jt_thrust);
   fChain->SetBranchAddress("AK5Z4_jt_LHA", AK5Z4_jt_LHA, &b_AK5Z4_jt_LHA);
   fChain->SetBranchAddress("AK5Z4_jt_pTD", AK5Z4_jt_pTD, &b_AK5Z4_jt_pTD);
   fChain->SetBranchAddress("AK5Z5_run", &AK5Z5_run, &b_AK5Z5_run);
   fChain->SetBranchAddress("AK5Z5_evt", &AK5Z5_evt, &b_AK5Z5_evt);
   fChain->SetBranchAddress("AK5Z5_lumi", &AK5Z5_lumi, &b_AK5Z5_lumi);
   fChain->SetBranchAddress("AK5Z5_nref", &AK5Z5_nref, &b_AK5Z5_nref);
   fChain->SetBranchAddress("AK5Z5_ncalo", &AK5Z5_ncalo, &b_AK5Z5_ncalo);
   fChain->SetBranchAddress("AK5Z5_rawpt", AK5Z5_rawpt, &b_AK5Z5_rawpt);
   fChain->SetBranchAddress("AK5Z5_jtpt", AK5Z5_jtpt, &b_AK5Z5_jtpt);
   fChain->SetBranchAddress("AK5Z5_jteta", AK5Z5_jteta, &b_AK5Z5_jteta);
   fChain->SetBranchAddress("AK5Z5_jty", AK5Z5_jty, &b_AK5Z5_jty);
   fChain->SetBranchAddress("AK5Z5_jtphi", AK5Z5_jtphi, &b_AK5Z5_jtphi);
   fChain->SetBranchAddress("AK5Z5_jtpu", AK5Z5_jtpu, &b_AK5Z5_jtpu);
   fChain->SetBranchAddress("AK5Z5_jtm", AK5Z5_jtm, &b_AK5Z5_jtm);
   fChain->SetBranchAddress("AK5Z5_jtarea", AK5Z5_jtarea, &b_AK5Z5_jtarea);
   fChain->SetBranchAddress("AK5Z5_jtdyn_split", AK5Z5_jtdyn_split, &b_AK5Z5_jtdyn_split);
   fChain->SetBranchAddress("AK5Z5_jtdyn_eta", AK5Z5_jtdyn_eta, &b_AK5Z5_jtdyn_eta);
   fChain->SetBranchAddress("AK5Z5_jtdyn_phi", AK5Z5_jtdyn_phi, &b_AK5Z5_jtdyn_phi);
   fChain->SetBranchAddress("AK5Z5_jtdyn_deltaR", AK5Z5_jtdyn_deltaR, &b_AK5Z5_jtdyn_deltaR);
   fChain->SetBranchAddress("AK5Z5_jtdyn_kt", AK5Z5_jtdyn_kt, &b_AK5Z5_jtdyn_kt);
   fChain->SetBranchAddress("AK5Z5_jtdyn_z", AK5Z5_jtdyn_z, &b_AK5Z5_jtdyn_z);
   fChain->SetBranchAddress("AK5Z5_jt_intjet_multi", AK5Z5_jt_intjet_multi, &b_AK5Z5_jt_intjet_multi);
   fChain->SetBranchAddress("AK5Z5_jt_girth", AK5Z5_jt_girth, &b_AK5Z5_jt_girth);
   fChain->SetBranchAddress("AK5Z5_jt_thrust", AK5Z5_jt_thrust, &b_AK5Z5_jt_thrust);
   fChain->SetBranchAddress("AK5Z5_jt_LHA", AK5Z5_jt_LHA, &b_AK5Z5_jt_LHA);
   fChain->SetBranchAddress("AK5Z5_jt_pTD", AK5Z5_jt_pTD, &b_AK5Z5_jt_pTD);
   fChain->SetBranchAddress("AK6Z1_run", &AK6Z1_run, &b_AK6Z1_run);
   fChain->SetBranchAddress("AK6Z1_evt", &AK6Z1_evt, &b_AK6Z1_evt);
   fChain->SetBranchAddress("AK6Z1_lumi", &AK6Z1_lumi, &b_AK6Z1_lumi);
   fChain->SetBranchAddress("AK6Z1_nref", &AK6Z1_nref, &b_AK6Z1_nref);
   fChain->SetBranchAddress("AK6Z1_ncalo", &AK6Z1_ncalo, &b_AK6Z1_ncalo);
   fChain->SetBranchAddress("AK6Z1_rawpt", AK6Z1_rawpt, &b_AK6Z1_rawpt);
   fChain->SetBranchAddress("AK6Z1_jtpt", AK6Z1_jtpt, &b_AK6Z1_jtpt);
   fChain->SetBranchAddress("AK6Z1_jteta", AK6Z1_jteta, &b_AK6Z1_jteta);
   fChain->SetBranchAddress("AK6Z1_jty", AK6Z1_jty, &b_AK6Z1_jty);
   fChain->SetBranchAddress("AK6Z1_jtphi", AK6Z1_jtphi, &b_AK6Z1_jtphi);
   fChain->SetBranchAddress("AK6Z1_jtpu", AK6Z1_jtpu, &b_AK6Z1_jtpu);
   fChain->SetBranchAddress("AK6Z1_jtm", AK6Z1_jtm, &b_AK6Z1_jtm);
   fChain->SetBranchAddress("AK6Z1_jtarea", AK6Z1_jtarea, &b_AK6Z1_jtarea);
   fChain->SetBranchAddress("AK6Z1_jtdyn_split", AK6Z1_jtdyn_split, &b_AK6Z1_jtdyn_split);
   fChain->SetBranchAddress("AK6Z1_jtdyn_eta", AK6Z1_jtdyn_eta, &b_AK6Z1_jtdyn_eta);
   fChain->SetBranchAddress("AK6Z1_jtdyn_phi", AK6Z1_jtdyn_phi, &b_AK6Z1_jtdyn_phi);
   fChain->SetBranchAddress("AK6Z1_jtdyn_deltaR", AK6Z1_jtdyn_deltaR, &b_AK6Z1_jtdyn_deltaR);
   fChain->SetBranchAddress("AK6Z1_jtdyn_kt", AK6Z1_jtdyn_kt, &b_AK6Z1_jtdyn_kt);
   fChain->SetBranchAddress("AK6Z1_jtdyn_z", AK6Z1_jtdyn_z, &b_AK6Z1_jtdyn_z);
   fChain->SetBranchAddress("AK6Z1_jt_intjet_multi", AK6Z1_jt_intjet_multi, &b_AK6Z1_jt_intjet_multi);
   fChain->SetBranchAddress("AK6Z1_jt_girth", AK6Z1_jt_girth, &b_AK6Z1_jt_girth);
   fChain->SetBranchAddress("AK6Z1_jt_thrust", AK6Z1_jt_thrust, &b_AK6Z1_jt_thrust);
   fChain->SetBranchAddress("AK6Z1_jt_LHA", AK6Z1_jt_LHA, &b_AK6Z1_jt_LHA);
   fChain->SetBranchAddress("AK6Z1_jt_pTD", AK6Z1_jt_pTD, &b_AK6Z1_jt_pTD);
   fChain->SetBranchAddress("AK6Z2_run", &AK6Z2_run, &b_AK6Z2_run);
   fChain->SetBranchAddress("AK6Z2_evt", &AK6Z2_evt, &b_AK6Z2_evt);
   fChain->SetBranchAddress("AK6Z2_lumi", &AK6Z2_lumi, &b_AK6Z2_lumi);
   fChain->SetBranchAddress("AK6Z2_nref", &AK6Z2_nref, &b_AK6Z2_nref);
   fChain->SetBranchAddress("AK6Z2_ncalo", &AK6Z2_ncalo, &b_AK6Z2_ncalo);
   fChain->SetBranchAddress("AK6Z2_rawpt", AK6Z2_rawpt, &b_AK6Z2_rawpt);
   fChain->SetBranchAddress("AK6Z2_jtpt", AK6Z2_jtpt, &b_AK6Z2_jtpt);
   fChain->SetBranchAddress("AK6Z2_jteta", AK6Z2_jteta, &b_AK6Z2_jteta);
   fChain->SetBranchAddress("AK6Z2_jty", AK6Z2_jty, &b_AK6Z2_jty);
   fChain->SetBranchAddress("AK6Z2_jtphi", AK6Z2_jtphi, &b_AK6Z2_jtphi);
   fChain->SetBranchAddress("AK6Z2_jtpu", AK6Z2_jtpu, &b_AK6Z2_jtpu);
   fChain->SetBranchAddress("AK6Z2_jtm", AK6Z2_jtm, &b_AK6Z2_jtm);
   fChain->SetBranchAddress("AK6Z2_jtarea", AK6Z2_jtarea, &b_AK6Z2_jtarea);
   fChain->SetBranchAddress("AK6Z2_jtdyn_split", AK6Z2_jtdyn_split, &b_AK6Z2_jtdyn_split);
   fChain->SetBranchAddress("AK6Z2_jtdyn_eta", AK6Z2_jtdyn_eta, &b_AK6Z2_jtdyn_eta);
   fChain->SetBranchAddress("AK6Z2_jtdyn_phi", AK6Z2_jtdyn_phi, &b_AK6Z2_jtdyn_phi);
   fChain->SetBranchAddress("AK6Z2_jtdyn_deltaR", AK6Z2_jtdyn_deltaR, &b_AK6Z2_jtdyn_deltaR);
   fChain->SetBranchAddress("AK6Z2_jtdyn_kt", AK6Z2_jtdyn_kt, &b_AK6Z2_jtdyn_kt);
   fChain->SetBranchAddress("AK6Z2_jtdyn_z", AK6Z2_jtdyn_z, &b_AK6Z2_jtdyn_z);
   fChain->SetBranchAddress("AK6Z2_jt_intjet_multi", AK6Z2_jt_intjet_multi, &b_AK6Z2_jt_intjet_multi);
   fChain->SetBranchAddress("AK6Z2_jt_girth", AK6Z2_jt_girth, &b_AK6Z2_jt_girth);
   fChain->SetBranchAddress("AK6Z2_jt_thrust", AK6Z2_jt_thrust, &b_AK6Z2_jt_thrust);
   fChain->SetBranchAddress("AK6Z2_jt_LHA", AK6Z2_jt_LHA, &b_AK6Z2_jt_LHA);
   fChain->SetBranchAddress("AK6Z2_jt_pTD", AK6Z2_jt_pTD, &b_AK6Z2_jt_pTD);
   fChain->SetBranchAddress("AK6Z3_run", &AK6Z3_run, &b_AK6Z3_run);
   fChain->SetBranchAddress("AK6Z3_evt", &AK6Z3_evt, &b_AK6Z3_evt);
   fChain->SetBranchAddress("AK6Z3_lumi", &AK6Z3_lumi, &b_AK6Z3_lumi);
   fChain->SetBranchAddress("AK6Z3_nref", &AK6Z3_nref, &b_AK6Z3_nref);
   fChain->SetBranchAddress("AK6Z3_ncalo", &AK6Z3_ncalo, &b_AK6Z3_ncalo);
   fChain->SetBranchAddress("AK6Z3_rawpt", AK6Z3_rawpt, &b_AK6Z3_rawpt);
   fChain->SetBranchAddress("AK6Z3_jtpt", AK6Z3_jtpt, &b_AK6Z3_jtpt);
   fChain->SetBranchAddress("AK6Z3_jteta", AK6Z3_jteta, &b_AK6Z3_jteta);
   fChain->SetBranchAddress("AK6Z3_jty", AK6Z3_jty, &b_AK6Z3_jty);
   fChain->SetBranchAddress("AK6Z3_jtphi", AK6Z3_jtphi, &b_AK6Z3_jtphi);
   fChain->SetBranchAddress("AK6Z3_jtpu", AK6Z3_jtpu, &b_AK6Z3_jtpu);
   fChain->SetBranchAddress("AK6Z3_jtm", AK6Z3_jtm, &b_AK6Z3_jtm);
   fChain->SetBranchAddress("AK6Z3_jtarea", AK6Z3_jtarea, &b_AK6Z3_jtarea);
   fChain->SetBranchAddress("AK6Z3_jtdyn_split", AK6Z3_jtdyn_split, &b_AK6Z3_jtdyn_split);
   fChain->SetBranchAddress("AK6Z3_jtdyn_eta", AK6Z3_jtdyn_eta, &b_AK6Z3_jtdyn_eta);
   fChain->SetBranchAddress("AK6Z3_jtdyn_phi", AK6Z3_jtdyn_phi, &b_AK6Z3_jtdyn_phi);
   fChain->SetBranchAddress("AK6Z3_jtdyn_deltaR", AK6Z3_jtdyn_deltaR, &b_AK6Z3_jtdyn_deltaR);
   fChain->SetBranchAddress("AK6Z3_jtdyn_kt", AK6Z3_jtdyn_kt, &b_AK6Z3_jtdyn_kt);
   fChain->SetBranchAddress("AK6Z3_jtdyn_z", AK6Z3_jtdyn_z, &b_AK6Z3_jtdyn_z);
   fChain->SetBranchAddress("AK6Z3_jt_intjet_multi", AK6Z3_jt_intjet_multi, &b_AK6Z3_jt_intjet_multi);
   fChain->SetBranchAddress("AK6Z3_jt_girth", AK6Z3_jt_girth, &b_AK6Z3_jt_girth);
   fChain->SetBranchAddress("AK6Z3_jt_thrust", AK6Z3_jt_thrust, &b_AK6Z3_jt_thrust);
   fChain->SetBranchAddress("AK6Z3_jt_LHA", AK6Z3_jt_LHA, &b_AK6Z3_jt_LHA);
   fChain->SetBranchAddress("AK6Z3_jt_pTD", AK6Z3_jt_pTD, &b_AK6Z3_jt_pTD);
   fChain->SetBranchAddress("AK6Z4_run", &AK6Z4_run, &b_AK6Z4_run);
   fChain->SetBranchAddress("AK6Z4_evt", &AK6Z4_evt, &b_AK6Z4_evt);
   fChain->SetBranchAddress("AK6Z4_lumi", &AK6Z4_lumi, &b_AK6Z4_lumi);
   fChain->SetBranchAddress("AK6Z4_nref", &AK6Z4_nref, &b_AK6Z4_nref);
   fChain->SetBranchAddress("AK6Z4_ncalo", &AK6Z4_ncalo, &b_AK6Z4_ncalo);
   fChain->SetBranchAddress("AK6Z4_rawpt", AK6Z4_rawpt, &b_AK6Z4_rawpt);
   fChain->SetBranchAddress("AK6Z4_jtpt", AK6Z4_jtpt, &b_AK6Z4_jtpt);
   fChain->SetBranchAddress("AK6Z4_jteta", AK6Z4_jteta, &b_AK6Z4_jteta);
   fChain->SetBranchAddress("AK6Z4_jty", AK6Z4_jty, &b_AK6Z4_jty);
   fChain->SetBranchAddress("AK6Z4_jtphi", AK6Z4_jtphi, &b_AK6Z4_jtphi);
   fChain->SetBranchAddress("AK6Z4_jtpu", AK6Z4_jtpu, &b_AK6Z4_jtpu);
   fChain->SetBranchAddress("AK6Z4_jtm", AK6Z4_jtm, &b_AK6Z4_jtm);
   fChain->SetBranchAddress("AK6Z4_jtarea", AK6Z4_jtarea, &b_AK6Z4_jtarea);
   fChain->SetBranchAddress("AK6Z4_jtdyn_split", AK6Z4_jtdyn_split, &b_AK6Z4_jtdyn_split);
   fChain->SetBranchAddress("AK6Z4_jtdyn_eta", AK6Z4_jtdyn_eta, &b_AK6Z4_jtdyn_eta);
   fChain->SetBranchAddress("AK6Z4_jtdyn_phi", AK6Z4_jtdyn_phi, &b_AK6Z4_jtdyn_phi);
   fChain->SetBranchAddress("AK6Z4_jtdyn_deltaR", AK6Z4_jtdyn_deltaR, &b_AK6Z4_jtdyn_deltaR);
   fChain->SetBranchAddress("AK6Z4_jtdyn_kt", AK6Z4_jtdyn_kt, &b_AK6Z4_jtdyn_kt);
   fChain->SetBranchAddress("AK6Z4_jtdyn_z", AK6Z4_jtdyn_z, &b_AK6Z4_jtdyn_z);
   fChain->SetBranchAddress("AK6Z4_jt_intjet_multi", AK6Z4_jt_intjet_multi, &b_AK6Z4_jt_intjet_multi);
   fChain->SetBranchAddress("AK6Z4_jt_girth", AK6Z4_jt_girth, &b_AK6Z4_jt_girth);
   fChain->SetBranchAddress("AK6Z4_jt_thrust", AK6Z4_jt_thrust, &b_AK6Z4_jt_thrust);
   fChain->SetBranchAddress("AK6Z4_jt_LHA", AK6Z4_jt_LHA, &b_AK6Z4_jt_LHA);
   fChain->SetBranchAddress("AK6Z4_jt_pTD", AK6Z4_jt_pTD, &b_AK6Z4_jt_pTD);
   fChain->SetBranchAddress("AK6Z5_run", &AK6Z5_run, &b_AK6Z5_run);
   fChain->SetBranchAddress("AK6Z5_evt", &AK6Z5_evt, &b_AK6Z5_evt);
   fChain->SetBranchAddress("AK6Z5_lumi", &AK6Z5_lumi, &b_AK6Z5_lumi);
   fChain->SetBranchAddress("AK6Z5_nref", &AK6Z5_nref, &b_AK6Z5_nref);
   fChain->SetBranchAddress("AK6Z5_ncalo", &AK6Z5_ncalo, &b_AK6Z5_ncalo);
   fChain->SetBranchAddress("AK6Z5_rawpt", AK6Z5_rawpt, &b_AK6Z5_rawpt);
   fChain->SetBranchAddress("AK6Z5_jtpt", AK6Z5_jtpt, &b_AK6Z5_jtpt);
   fChain->SetBranchAddress("AK6Z5_jteta", AK6Z5_jteta, &b_AK6Z5_jteta);
   fChain->SetBranchAddress("AK6Z5_jty", AK6Z5_jty, &b_AK6Z5_jty);
   fChain->SetBranchAddress("AK6Z5_jtphi", AK6Z5_jtphi, &b_AK6Z5_jtphi);
   fChain->SetBranchAddress("AK6Z5_jtpu", AK6Z5_jtpu, &b_AK6Z5_jtpu);
   fChain->SetBranchAddress("AK6Z5_jtm", AK6Z5_jtm, &b_AK6Z5_jtm);
   fChain->SetBranchAddress("AK6Z5_jtarea", AK6Z5_jtarea, &b_AK6Z5_jtarea);
   fChain->SetBranchAddress("AK6Z5_jtdyn_split", AK6Z5_jtdyn_split, &b_AK6Z5_jtdyn_split);
   fChain->SetBranchAddress("AK6Z5_jtdyn_eta", AK6Z5_jtdyn_eta, &b_AK6Z5_jtdyn_eta);
   fChain->SetBranchAddress("AK6Z5_jtdyn_phi", AK6Z5_jtdyn_phi, &b_AK6Z5_jtdyn_phi);
   fChain->SetBranchAddress("AK6Z5_jtdyn_deltaR", AK6Z5_jtdyn_deltaR, &b_AK6Z5_jtdyn_deltaR);
   fChain->SetBranchAddress("AK6Z5_jtdyn_kt", AK6Z5_jtdyn_kt, &b_AK6Z5_jtdyn_kt);
   fChain->SetBranchAddress("AK6Z5_jtdyn_z", AK6Z5_jtdyn_z, &b_AK6Z5_jtdyn_z);
   fChain->SetBranchAddress("AK6Z5_jt_intjet_multi", AK6Z5_jt_intjet_multi, &b_AK6Z5_jt_intjet_multi);
   fChain->SetBranchAddress("AK6Z5_jt_girth", AK6Z5_jt_girth, &b_AK6Z5_jt_girth);
   fChain->SetBranchAddress("AK6Z5_jt_thrust", AK6Z5_jt_thrust, &b_AK6Z5_jt_thrust);
   fChain->SetBranchAddress("AK6Z5_jt_LHA", AK6Z5_jt_LHA, &b_AK6Z5_jt_LHA);
   fChain->SetBranchAddress("AK6Z5_jt_pTD", AK6Z5_jt_pTD, &b_AK6Z5_jt_pTD);
   fChain->SetBranchAddress("AK8Z1_run", &AK8Z1_run, &b_AK8Z1_run);
   fChain->SetBranchAddress("AK8Z1_evt", &AK8Z1_evt, &b_AK8Z1_evt);
   fChain->SetBranchAddress("AK8Z1_lumi", &AK8Z1_lumi, &b_AK8Z1_lumi);
   fChain->SetBranchAddress("AK8Z1_nref", &AK8Z1_nref, &b_AK8Z1_nref);
   fChain->SetBranchAddress("AK8Z1_ncalo", &AK8Z1_ncalo, &b_AK8Z1_ncalo);
   fChain->SetBranchAddress("AK8Z1_rawpt", AK8Z1_rawpt, &b_AK8Z1_rawpt);
   fChain->SetBranchAddress("AK8Z1_jtpt", AK8Z1_jtpt, &b_AK8Z1_jtpt);
   fChain->SetBranchAddress("AK8Z1_jteta", AK8Z1_jteta, &b_AK8Z1_jteta);
   fChain->SetBranchAddress("AK8Z1_jty", AK8Z1_jty, &b_AK8Z1_jty);
   fChain->SetBranchAddress("AK8Z1_jtphi", AK8Z1_jtphi, &b_AK8Z1_jtphi);
   fChain->SetBranchAddress("AK8Z1_jtpu", AK8Z1_jtpu, &b_AK8Z1_jtpu);
   fChain->SetBranchAddress("AK8Z1_jtm", AK8Z1_jtm, &b_AK8Z1_jtm);
   fChain->SetBranchAddress("AK8Z1_jtarea", AK8Z1_jtarea, &b_AK8Z1_jtarea);
   fChain->SetBranchAddress("AK8Z1_jtdyn_split", AK8Z1_jtdyn_split, &b_AK8Z1_jtdyn_split);
   fChain->SetBranchAddress("AK8Z1_jtdyn_eta", AK8Z1_jtdyn_eta, &b_AK8Z1_jtdyn_eta);
   fChain->SetBranchAddress("AK8Z1_jtdyn_phi", AK8Z1_jtdyn_phi, &b_AK8Z1_jtdyn_phi);
   fChain->SetBranchAddress("AK8Z1_jtdyn_deltaR", AK8Z1_jtdyn_deltaR, &b_AK8Z1_jtdyn_deltaR);
   fChain->SetBranchAddress("AK8Z1_jtdyn_kt", AK8Z1_jtdyn_kt, &b_AK8Z1_jtdyn_kt);
   fChain->SetBranchAddress("AK8Z1_jtdyn_z", AK8Z1_jtdyn_z, &b_AK8Z1_jtdyn_z);
   fChain->SetBranchAddress("AK8Z1_jt_intjet_multi", AK8Z1_jt_intjet_multi, &b_AK8Z1_jt_intjet_multi);
   fChain->SetBranchAddress("AK8Z1_jt_girth", AK8Z1_jt_girth, &b_AK8Z1_jt_girth);
   fChain->SetBranchAddress("AK8Z1_jt_thrust", AK8Z1_jt_thrust, &b_AK8Z1_jt_thrust);
   fChain->SetBranchAddress("AK8Z1_jt_LHA", AK8Z1_jt_LHA, &b_AK8Z1_jt_LHA);
   fChain->SetBranchAddress("AK8Z1_jt_pTD", AK8Z1_jt_pTD, &b_AK8Z1_jt_pTD);
   fChain->SetBranchAddress("AK8Z2_run", &AK8Z2_run, &b_AK8Z2_run);
   fChain->SetBranchAddress("AK8Z2_evt", &AK8Z2_evt, &b_AK8Z2_evt);
   fChain->SetBranchAddress("AK8Z2_lumi", &AK8Z2_lumi, &b_AK8Z2_lumi);
   fChain->SetBranchAddress("AK8Z2_nref", &AK8Z2_nref, &b_AK8Z2_nref);
   fChain->SetBranchAddress("AK8Z2_ncalo", &AK8Z2_ncalo, &b_AK8Z2_ncalo);
   fChain->SetBranchAddress("AK8Z2_rawpt", AK8Z2_rawpt, &b_AK8Z2_rawpt);
   fChain->SetBranchAddress("AK8Z2_jtpt", AK8Z2_jtpt, &b_AK8Z2_jtpt);
   fChain->SetBranchAddress("AK8Z2_jteta", AK8Z2_jteta, &b_AK8Z2_jteta);
   fChain->SetBranchAddress("AK8Z2_jty", AK8Z2_jty, &b_AK8Z2_jty);
   fChain->SetBranchAddress("AK8Z2_jtphi", AK8Z2_jtphi, &b_AK8Z2_jtphi);
   fChain->SetBranchAddress("AK8Z2_jtpu", AK8Z2_jtpu, &b_AK8Z2_jtpu);
   fChain->SetBranchAddress("AK8Z2_jtm", AK8Z2_jtm, &b_AK8Z2_jtm);
   fChain->SetBranchAddress("AK8Z2_jtarea", AK8Z2_jtarea, &b_AK8Z2_jtarea);
   fChain->SetBranchAddress("AK8Z2_jtdyn_split", AK8Z2_jtdyn_split, &b_AK8Z2_jtdyn_split);
   fChain->SetBranchAddress("AK8Z2_jtdyn_eta", AK8Z2_jtdyn_eta, &b_AK8Z2_jtdyn_eta);
   fChain->SetBranchAddress("AK8Z2_jtdyn_phi", AK8Z2_jtdyn_phi, &b_AK8Z2_jtdyn_phi);
   fChain->SetBranchAddress("AK8Z2_jtdyn_deltaR", AK8Z2_jtdyn_deltaR, &b_AK8Z2_jtdyn_deltaR);
   fChain->SetBranchAddress("AK8Z2_jtdyn_kt", AK8Z2_jtdyn_kt, &b_AK8Z2_jtdyn_kt);
   fChain->SetBranchAddress("AK8Z2_jtdyn_z", AK8Z2_jtdyn_z, &b_AK8Z2_jtdyn_z);
   fChain->SetBranchAddress("AK8Z2_jt_intjet_multi", AK8Z2_jt_intjet_multi, &b_AK8Z2_jt_intjet_multi);
   fChain->SetBranchAddress("AK8Z2_jt_girth", AK8Z2_jt_girth, &b_AK8Z2_jt_girth);
   fChain->SetBranchAddress("AK8Z2_jt_thrust", AK8Z2_jt_thrust, &b_AK8Z2_jt_thrust);
   fChain->SetBranchAddress("AK8Z2_jt_LHA", AK8Z2_jt_LHA, &b_AK8Z2_jt_LHA);
   fChain->SetBranchAddress("AK8Z2_jt_pTD", AK8Z2_jt_pTD, &b_AK8Z2_jt_pTD);
   fChain->SetBranchAddress("AK8Z3_run", &AK8Z3_run, &b_AK8Z3_run);
   fChain->SetBranchAddress("AK8Z3_evt", &AK8Z3_evt, &b_AK8Z3_evt);
   fChain->SetBranchAddress("AK8Z3_lumi", &AK8Z3_lumi, &b_AK8Z3_lumi);
   fChain->SetBranchAddress("AK8Z3_nref", &AK8Z3_nref, &b_AK8Z3_nref);
   fChain->SetBranchAddress("AK8Z3_ncalo", &AK8Z3_ncalo, &b_AK8Z3_ncalo);
   fChain->SetBranchAddress("AK8Z3_rawpt", AK8Z3_rawpt, &b_AK8Z3_rawpt);
   fChain->SetBranchAddress("AK8Z3_jtpt", AK8Z3_jtpt, &b_AK8Z3_jtpt);
   fChain->SetBranchAddress("AK8Z3_jteta", AK8Z3_jteta, &b_AK8Z3_jteta);
   fChain->SetBranchAddress("AK8Z3_jty", AK8Z3_jty, &b_AK8Z3_jty);
   fChain->SetBranchAddress("AK8Z3_jtphi", AK8Z3_jtphi, &b_AK8Z3_jtphi);
   fChain->SetBranchAddress("AK8Z3_jtpu", AK8Z3_jtpu, &b_AK8Z3_jtpu);
   fChain->SetBranchAddress("AK8Z3_jtm", AK8Z3_jtm, &b_AK8Z3_jtm);
   fChain->SetBranchAddress("AK8Z3_jtarea", AK8Z3_jtarea, &b_AK8Z3_jtarea);
   fChain->SetBranchAddress("AK8Z3_jtdyn_split", AK8Z3_jtdyn_split, &b_AK8Z3_jtdyn_split);
   fChain->SetBranchAddress("AK8Z3_jtdyn_eta", AK8Z3_jtdyn_eta, &b_AK8Z3_jtdyn_eta);
   fChain->SetBranchAddress("AK8Z3_jtdyn_phi", AK8Z3_jtdyn_phi, &b_AK8Z3_jtdyn_phi);
   fChain->SetBranchAddress("AK8Z3_jtdyn_deltaR", AK8Z3_jtdyn_deltaR, &b_AK8Z3_jtdyn_deltaR);
   fChain->SetBranchAddress("AK8Z3_jtdyn_kt", AK8Z3_jtdyn_kt, &b_AK8Z3_jtdyn_kt);
   fChain->SetBranchAddress("AK8Z3_jtdyn_z", AK8Z3_jtdyn_z, &b_AK8Z3_jtdyn_z);
   fChain->SetBranchAddress("AK8Z3_jt_intjet_multi", AK8Z3_jt_intjet_multi, &b_AK8Z3_jt_intjet_multi);
   fChain->SetBranchAddress("AK8Z3_jt_girth", AK8Z3_jt_girth, &b_AK8Z3_jt_girth);
   fChain->SetBranchAddress("AK8Z3_jt_thrust", AK8Z3_jt_thrust, &b_AK8Z3_jt_thrust);
   fChain->SetBranchAddress("AK8Z3_jt_LHA", AK8Z3_jt_LHA, &b_AK8Z3_jt_LHA);
   fChain->SetBranchAddress("AK8Z3_jt_pTD", AK8Z3_jt_pTD, &b_AK8Z3_jt_pTD);
   fChain->SetBranchAddress("AK8Z4_run", &AK8Z4_run, &b_AK8Z4_run);
   fChain->SetBranchAddress("AK8Z4_evt", &AK8Z4_evt, &b_AK8Z4_evt);
   fChain->SetBranchAddress("AK8Z4_lumi", &AK8Z4_lumi, &b_AK8Z4_lumi);
   fChain->SetBranchAddress("AK8Z4_nref", &AK8Z4_nref, &b_AK8Z4_nref);
   fChain->SetBranchAddress("AK8Z4_ncalo", &AK8Z4_ncalo, &b_AK8Z4_ncalo);
   fChain->SetBranchAddress("AK8Z4_rawpt", AK8Z4_rawpt, &b_AK8Z4_rawpt);
   fChain->SetBranchAddress("AK8Z4_jtpt", AK8Z4_jtpt, &b_AK8Z4_jtpt);
   fChain->SetBranchAddress("AK8Z4_jteta", AK8Z4_jteta, &b_AK8Z4_jteta);
   fChain->SetBranchAddress("AK8Z4_jty", AK8Z4_jty, &b_AK8Z4_jty);
   fChain->SetBranchAddress("AK8Z4_jtphi", AK8Z4_jtphi, &b_AK8Z4_jtphi);
   fChain->SetBranchAddress("AK8Z4_jtpu", AK8Z4_jtpu, &b_AK8Z4_jtpu);
   fChain->SetBranchAddress("AK8Z4_jtm", AK8Z4_jtm, &b_AK8Z4_jtm);
   fChain->SetBranchAddress("AK8Z4_jtarea", AK8Z4_jtarea, &b_AK8Z4_jtarea);
   fChain->SetBranchAddress("AK8Z4_jtdyn_split", AK8Z4_jtdyn_split, &b_AK8Z4_jtdyn_split);
   fChain->SetBranchAddress("AK8Z4_jtdyn_eta", AK8Z4_jtdyn_eta, &b_AK8Z4_jtdyn_eta);
   fChain->SetBranchAddress("AK8Z4_jtdyn_phi", AK8Z4_jtdyn_phi, &b_AK8Z4_jtdyn_phi);
   fChain->SetBranchAddress("AK8Z4_jtdyn_deltaR", AK8Z4_jtdyn_deltaR, &b_AK8Z4_jtdyn_deltaR);
   fChain->SetBranchAddress("AK8Z4_jtdyn_kt", AK8Z4_jtdyn_kt, &b_AK8Z4_jtdyn_kt);
   fChain->SetBranchAddress("AK8Z4_jtdyn_z", AK8Z4_jtdyn_z, &b_AK8Z4_jtdyn_z);
   fChain->SetBranchAddress("AK8Z4_jt_intjet_multi", AK8Z4_jt_intjet_multi, &b_AK8Z4_jt_intjet_multi);
   fChain->SetBranchAddress("AK8Z4_jt_girth", AK8Z4_jt_girth, &b_AK8Z4_jt_girth);
   fChain->SetBranchAddress("AK8Z4_jt_thrust", AK8Z4_jt_thrust, &b_AK8Z4_jt_thrust);
   fChain->SetBranchAddress("AK8Z4_jt_LHA", AK8Z4_jt_LHA, &b_AK8Z4_jt_LHA);
   fChain->SetBranchAddress("AK8Z4_jt_pTD", AK8Z4_jt_pTD, &b_AK8Z4_jt_pTD);
   fChain->SetBranchAddress("AK8Z5_run", &AK8Z5_run, &b_AK8Z5_run);
   fChain->SetBranchAddress("AK8Z5_evt", &AK8Z5_evt, &b_AK8Z5_evt);
   fChain->SetBranchAddress("AK8Z5_lumi", &AK8Z5_lumi, &b_AK8Z5_lumi);
   fChain->SetBranchAddress("AK8Z5_nref", &AK8Z5_nref, &b_AK8Z5_nref);
   fChain->SetBranchAddress("AK8Z5_ncalo", &AK8Z5_ncalo, &b_AK8Z5_ncalo);
   fChain->SetBranchAddress("AK8Z5_rawpt", AK8Z5_rawpt, &b_AK8Z5_rawpt);
   fChain->SetBranchAddress("AK8Z5_jtpt", AK8Z5_jtpt, &b_AK8Z5_jtpt);
   fChain->SetBranchAddress("AK8Z5_jteta", AK8Z5_jteta, &b_AK8Z5_jteta);
   fChain->SetBranchAddress("AK8Z5_jty", AK8Z5_jty, &b_AK8Z5_jty);
   fChain->SetBranchAddress("AK8Z5_jtphi", AK8Z5_jtphi, &b_AK8Z5_jtphi);
   fChain->SetBranchAddress("AK8Z5_jtpu", AK8Z5_jtpu, &b_AK8Z5_jtpu);
   fChain->SetBranchAddress("AK8Z5_jtm", AK8Z5_jtm, &b_AK8Z5_jtm);
   fChain->SetBranchAddress("AK8Z5_jtarea", AK8Z5_jtarea, &b_AK8Z5_jtarea);
   fChain->SetBranchAddress("AK8Z5_jtdyn_split", AK8Z5_jtdyn_split, &b_AK8Z5_jtdyn_split);
   fChain->SetBranchAddress("AK8Z5_jtdyn_eta", AK8Z5_jtdyn_eta, &b_AK8Z5_jtdyn_eta);
   fChain->SetBranchAddress("AK8Z5_jtdyn_phi", AK8Z5_jtdyn_phi, &b_AK8Z5_jtdyn_phi);
   fChain->SetBranchAddress("AK8Z5_jtdyn_deltaR", AK8Z5_jtdyn_deltaR, &b_AK8Z5_jtdyn_deltaR);
   fChain->SetBranchAddress("AK8Z5_jtdyn_kt", AK8Z5_jtdyn_kt, &b_AK8Z5_jtdyn_kt);
   fChain->SetBranchAddress("AK8Z5_jtdyn_z", AK8Z5_jtdyn_z, &b_AK8Z5_jtdyn_z);
   fChain->SetBranchAddress("AK8Z5_jt_intjet_multi", AK8Z5_jt_intjet_multi, &b_AK8Z5_jt_intjet_multi);
   fChain->SetBranchAddress("AK8Z5_jt_girth", AK8Z5_jt_girth, &b_AK8Z5_jt_girth);
   fChain->SetBranchAddress("AK8Z5_jt_thrust", AK8Z5_jt_thrust, &b_AK8Z5_jt_thrust);
   fChain->SetBranchAddress("AK8Z5_jt_LHA", AK8Z5_jt_LHA, &b_AK8Z5_jt_LHA);
   fChain->SetBranchAddress("AK8Z5_jt_pTD", AK8Z5_jt_pTD, &b_AK8Z5_jt_pTD);
   fChain->SetBranchAddress("ggHi_run", &ggHi_run, &b_ggHi_run);
   fChain->SetBranchAddress("ggHi_event", &ggHi_event, &b_ggHi_event);
   fChain->SetBranchAddress("ggHi_lumis", &ggHi_lumis, &b_ggHi_lumis);
   fChain->SetBranchAddress("ggHi_rho", &ggHi_rho, &b_ggHi_rho);
   fChain->SetBranchAddress("ggHi_nEle", &ggHi_nEle, &b_ggHi_nEle);
   fChain->SetBranchAddress("ggHi_eleD0", &ggHi_eleD0, &b_ggHi_eleD0);
   fChain->SetBranchAddress("ggHi_eleDz", &ggHi_eleDz, &b_ggHi_eleDz);
   fChain->SetBranchAddress("ggHi_eleD0Err", &ggHi_eleD0Err, &b_ggHi_eleD0Err);
   fChain->SetBranchAddress("ggHi_eleDzErr", &ggHi_eleDzErr, &b_ggHi_eleDzErr);
   fChain->SetBranchAddress("ggHi_eleTrkPt", &ggHi_eleTrkPt, &b_ggHi_eleTrkPt);
   fChain->SetBranchAddress("ggHi_eleTrkEta", &ggHi_eleTrkEta, &b_ggHi_eleTrkEta);
   fChain->SetBranchAddress("ggHi_eleTrkPhi", &ggHi_eleTrkPhi, &b_ggHi_eleTrkPhi);
   fChain->SetBranchAddress("ggHi_eleTrkCharge", &ggHi_eleTrkCharge, &b_ggHi_eleTrkCharge);
   fChain->SetBranchAddress("ggHi_eleTrkPtErr", &ggHi_eleTrkPtErr, &b_ggHi_eleTrkPtErr);
   fChain->SetBranchAddress("ggHi_eleTrkChi2", &ggHi_eleTrkChi2, &b_ggHi_eleTrkChi2);
   fChain->SetBranchAddress("ggHi_eleTrkNdof", &ggHi_eleTrkNdof, &b_ggHi_eleTrkNdof);
   fChain->SetBranchAddress("ggHi_eleTrkNormalizedChi2", &ggHi_eleTrkNormalizedChi2, &b_ggHi_eleTrkNormalizedChi2);
   fChain->SetBranchAddress("ggHi_eleTrkValidHits", &ggHi_eleTrkValidHits, &b_ggHi_eleTrkValidHits);
   fChain->SetBranchAddress("ggHi_eleTrkLayers", &ggHi_eleTrkLayers, &b_ggHi_eleTrkLayers);
   fChain->SetBranchAddress("ggHi_eleMissHits", &ggHi_eleMissHits, &b_ggHi_eleMissHits);
   fChain->SetBranchAddress("ggHi_eleIP3D", &ggHi_eleIP3D, &b_ggHi_eleIP3D);
   fChain->SetBranchAddress("ggHi_eleIP3DErr", &ggHi_eleIP3DErr, &b_ggHi_eleIP3DErr);
   fChain->SetBranchAddress("ggHi_elePt", &ggHi_elePt, &b_ggHi_elePt);
   fChain->SetBranchAddress("ggHi_eleEta", &ggHi_eleEta, &b_ggHi_eleEta);
   fChain->SetBranchAddress("ggHi_elePhi", &ggHi_elePhi, &b_ggHi_elePhi);
   fChain->SetBranchAddress("ggHi_eleCharge", &ggHi_eleCharge, &b_ggHi_eleCharge);
   fChain->SetBranchAddress("ggHi_eleEn", &ggHi_eleEn, &b_ggHi_eleEn);
   fChain->SetBranchAddress("ggHi_eleSCEn", &ggHi_eleSCEn, &b_ggHi_eleSCEn);
   fChain->SetBranchAddress("ggHi_eleESEn", &ggHi_eleESEn, &b_ggHi_eleESEn);
   fChain->SetBranchAddress("ggHi_eleSCEta", &ggHi_eleSCEta, &b_ggHi_eleSCEta);
   fChain->SetBranchAddress("ggHi_eleSCPhi", &ggHi_eleSCPhi, &b_ggHi_eleSCPhi);
   fChain->SetBranchAddress("ggHi_eleSCRawEn", &ggHi_eleSCRawEn, &b_ggHi_eleSCRawEn);
   fChain->SetBranchAddress("ggHi_eleSCEtaWidth", &ggHi_eleSCEtaWidth, &b_ggHi_eleSCEtaWidth);
   fChain->SetBranchAddress("ggHi_eleSCPhiWidth", &ggHi_eleSCPhiWidth, &b_ggHi_eleSCPhiWidth);
   fChain->SetBranchAddress("ggHi_eleSCClustersSize", &ggHi_eleSCClustersSize, &b_ggHi_eleSCClustersSize);
   fChain->SetBranchAddress("ggHi_eleSeedEn", &ggHi_eleSeedEn, &b_ggHi_eleSeedEn);
   fChain->SetBranchAddress("ggHi_eleSeedEta", &ggHi_eleSeedEta, &b_ggHi_eleSeedEta);
   fChain->SetBranchAddress("ggHi_eleSeedPhi", &ggHi_eleSeedPhi, &b_ggHi_eleSeedPhi);
   fChain->SetBranchAddress("ggHi_eleHoverE", &ggHi_eleHoverE, &b_ggHi_eleHoverE);
   fChain->SetBranchAddress("ggHi_eleHoverEBc", &ggHi_eleHoverEBc, &b_ggHi_eleHoverEBc);
   fChain->SetBranchAddress("ggHi_eleEoverP", &ggHi_eleEoverP, &b_ggHi_eleEoverP);
   fChain->SetBranchAddress("ggHi_eleEoverPInv", &ggHi_eleEoverPInv, &b_ggHi_eleEoverPInv);
   fChain->SetBranchAddress("ggHi_eleEcalE", &ggHi_eleEcalE, &b_ggHi_eleEcalE);
   fChain->SetBranchAddress("ggHi_elePAtVtx", &ggHi_elePAtVtx, &b_ggHi_elePAtVtx);
   fChain->SetBranchAddress("ggHi_elePAtSC", &ggHi_elePAtSC, &b_ggHi_elePAtSC);
   fChain->SetBranchAddress("ggHi_elePAtCluster", &ggHi_elePAtCluster, &b_ggHi_elePAtCluster);
   fChain->SetBranchAddress("ggHi_elePAtSeed", &ggHi_elePAtSeed, &b_ggHi_elePAtSeed);
   fChain->SetBranchAddress("ggHi_eledEtaAtVtx", &ggHi_eledEtaAtVtx, &b_ggHi_eledEtaAtVtx);
   fChain->SetBranchAddress("ggHi_eledPhiAtVtx", &ggHi_eledPhiAtVtx, &b_ggHi_eledPhiAtVtx);
   fChain->SetBranchAddress("ggHi_eledEtaSeedAtVtx", &ggHi_eledEtaSeedAtVtx, &b_ggHi_eledEtaSeedAtVtx);
   fChain->SetBranchAddress("ggHi_eleSigmaIEtaIEta", &ggHi_eleSigmaIEtaIEta, &b_ggHi_eleSigmaIEtaIEta);
   fChain->SetBranchAddress("ggHi_eleSigmaIPhiIPhi", &ggHi_eleSigmaIPhiIPhi, &b_ggHi_eleSigmaIPhiIPhi);
   fChain->SetBranchAddress("ggHi_eleBrem", &ggHi_eleBrem, &b_ggHi_eleBrem);
   fChain->SetBranchAddress("ggHi_eleConvVeto", &ggHi_eleConvVeto, &b_ggHi_eleConvVeto);
   fChain->SetBranchAddress("ggHi_eleR9", &ggHi_eleR9, &b_ggHi_eleR9);
   fChain->SetBranchAddress("ggHi_eleE3x3", &ggHi_eleE3x3, &b_ggHi_eleE3x3);
   fChain->SetBranchAddress("ggHi_eleE5x5", &ggHi_eleE5x5, &b_ggHi_eleE5x5);
   fChain->SetBranchAddress("ggHi_eleR9Full5x5", &ggHi_eleR9Full5x5, &b_ggHi_eleR9Full5x5);
   fChain->SetBranchAddress("ggHi_eleE3x3Full5x5", &ggHi_eleE3x3Full5x5, &b_ggHi_eleE3x3Full5x5);
   fChain->SetBranchAddress("ggHi_eleE5x5Full5x5", &ggHi_eleE5x5Full5x5, &b_ggHi_eleE5x5Full5x5);
   fChain->SetBranchAddress("ggHi_eleSigmaIEtaIEta_2012", &ggHi_eleSigmaIEtaIEta_2012, &b_ggHi_eleSigmaIEtaIEta_2012);
   fChain->SetBranchAddress("ggHi_elePFChIso", &ggHi_elePFChIso, &b_ggHi_elePFChIso);
   fChain->SetBranchAddress("ggHi_elePFPhoIso", &ggHi_elePFPhoIso, &b_ggHi_elePFPhoIso);
   fChain->SetBranchAddress("ggHi_elePFNeuIso", &ggHi_elePFNeuIso, &b_ggHi_elePFNeuIso);
   fChain->SetBranchAddress("ggHi_elePFPUIso", &ggHi_elePFPUIso, &b_ggHi_elePFPUIso);
   fChain->SetBranchAddress("ggHi_elePFRelIsoWithEA", &ggHi_elePFRelIsoWithEA, &b_ggHi_elePFRelIsoWithEA);
   fChain->SetBranchAddress("ggHi_elePFRelIsoWithDBeta", &ggHi_elePFRelIsoWithDBeta, &b_ggHi_elePFRelIsoWithDBeta);
   fChain->SetBranchAddress("ggHi_eleEffAreaTimesRho", &ggHi_eleEffAreaTimesRho, &b_ggHi_eleEffAreaTimesRho);
   fChain->SetBranchAddress("ggHi_elePFChIso03", &ggHi_elePFChIso03, &b_ggHi_elePFChIso03);
   fChain->SetBranchAddress("ggHi_elePFPhoIso03", &ggHi_elePFPhoIso03, &b_ggHi_elePFPhoIso03);
   fChain->SetBranchAddress("ggHi_elePFNeuIso03", &ggHi_elePFNeuIso03, &b_ggHi_elePFNeuIso03);
   fChain->SetBranchAddress("ggHi_elePFChIso04", &ggHi_elePFChIso04, &b_ggHi_elePFChIso04);
   fChain->SetBranchAddress("ggHi_elePFPhoIso04", &ggHi_elePFPhoIso04, &b_ggHi_elePFPhoIso04);
   fChain->SetBranchAddress("ggHi_elePFNeuIso04", &ggHi_elePFNeuIso04, &b_ggHi_elePFNeuIso04);
   fChain->SetBranchAddress("ggHi_eleSeedCryEta", &ggHi_eleSeedCryEta, &b_ggHi_eleSeedCryEta);
   fChain->SetBranchAddress("ggHi_eleSeedCryPhi", &ggHi_eleSeedCryPhi, &b_ggHi_eleSeedCryPhi);
   fChain->SetBranchAddress("ggHi_eleSeedCryIeta", &ggHi_eleSeedCryIeta, &b_ggHi_eleSeedCryIeta);
   fChain->SetBranchAddress("ggHi_eleSeedCryIphi", &ggHi_eleSeedCryIphi, &b_ggHi_eleSeedCryIphi);
   fChain->SetBranchAddress("ggHi_nPho", &ggHi_nPho, &b_ggHi_nPho);
   fChain->SetBranchAddress("ggHi_phoE", &ggHi_phoE, &b_ggHi_phoE);
   fChain->SetBranchAddress("ggHi_phoEt", &ggHi_phoEt, &b_ggHi_phoEt);
   fChain->SetBranchAddress("ggHi_phoEta", &ggHi_phoEta, &b_ggHi_phoEta);
   fChain->SetBranchAddress("ggHi_phoPhi", &ggHi_phoPhi, &b_ggHi_phoPhi);
   fChain->SetBranchAddress("ggHi_phoEcorrStdEcal", &ggHi_phoEcorrStdEcal, &b_ggHi_phoEcorrStdEcal);
   fChain->SetBranchAddress("ggHi_phoEcorrPhoEcal", &ggHi_phoEcorrPhoEcal, &b_ggHi_phoEcorrPhoEcal);
   fChain->SetBranchAddress("ggHi_phoEcorrRegr1", &ggHi_phoEcorrRegr1, &b_ggHi_phoEcorrRegr1);
   fChain->SetBranchAddress("ggHi_phoEcorrRegr2", &ggHi_phoEcorrRegr2, &b_ggHi_phoEcorrRegr2);
   fChain->SetBranchAddress("ggHi_phoEcorrErrStdEcal", &ggHi_phoEcorrErrStdEcal, &b_ggHi_phoEcorrErrStdEcal);
   fChain->SetBranchAddress("ggHi_phoEcorrErrPhoEcal", &ggHi_phoEcorrErrPhoEcal, &b_ggHi_phoEcorrErrPhoEcal);
   fChain->SetBranchAddress("ggHi_phoEcorrErrRegr1", &ggHi_phoEcorrErrRegr1, &b_ggHi_phoEcorrErrRegr1);
   fChain->SetBranchAddress("ggHi_phoEcorrErrRegr2", &ggHi_phoEcorrErrRegr2, &b_ggHi_phoEcorrErrRegr2);
   fChain->SetBranchAddress("ggHi_phoSCE", &ggHi_phoSCE, &b_ggHi_phoSCE);
   fChain->SetBranchAddress("ggHi_phoSCRawE", &ggHi_phoSCRawE, &b_ggHi_phoSCRawE);
   fChain->SetBranchAddress("ggHi_phoSCEta", &ggHi_phoSCEta, &b_ggHi_phoSCEta);
   fChain->SetBranchAddress("ggHi_phoSCPhi", &ggHi_phoSCPhi, &b_ggHi_phoSCPhi);
   fChain->SetBranchAddress("ggHi_phoSCEtaWidth", &ggHi_phoSCEtaWidth, &b_ggHi_phoSCEtaWidth);
   fChain->SetBranchAddress("ggHi_phoSCPhiWidth", &ggHi_phoSCPhiWidth, &b_ggHi_phoSCPhiWidth);
   fChain->SetBranchAddress("ggHi_phoSCBrem", &ggHi_phoSCBrem, &b_ggHi_phoSCBrem);
   fChain->SetBranchAddress("ggHi_phoSCnHits", &ggHi_phoSCnHits, &b_ggHi_phoSCnHits);
   fChain->SetBranchAddress("ggHi_phoSCflags", &ggHi_phoSCflags, &b_ggHi_phoSCflags);
   fChain->SetBranchAddress("ggHi_phoSCinClean", &ggHi_phoSCinClean, &b_ggHi_phoSCinClean);
   fChain->SetBranchAddress("ggHi_phoSCinUnClean", &ggHi_phoSCinUnClean, &b_ggHi_phoSCinUnClean);
   fChain->SetBranchAddress("ggHi_phoSCnBC", &ggHi_phoSCnBC, &b_ggHi_phoSCnBC);
   fChain->SetBranchAddress("ggHi_phoESEn", &ggHi_phoESEn, &b_ggHi_phoESEn);
   fChain->SetBranchAddress("ggHi_phoIsPFPhoton", &ggHi_phoIsPFPhoton, &b_ggHi_phoIsPFPhoton);
   fChain->SetBranchAddress("ggHi_phoIsStandardPhoton", &ggHi_phoIsStandardPhoton, &b_ggHi_phoIsStandardPhoton);
   fChain->SetBranchAddress("ggHi_phoHasPixelSeed", &ggHi_phoHasPixelSeed, &b_ggHi_phoHasPixelSeed);
   fChain->SetBranchAddress("ggHi_phoHasConversionTracks", &ggHi_phoHasConversionTracks, &b_ggHi_phoHasConversionTracks);
   fChain->SetBranchAddress("ggHi_phoHadTowerOverEm", &ggHi_phoHadTowerOverEm, &b_ggHi_phoHadTowerOverEm);
   fChain->SetBranchAddress("ggHi_phoHoverE", &ggHi_phoHoverE, &b_ggHi_phoHoverE);
   fChain->SetBranchAddress("ggHi_phoHoverEValid", &ggHi_phoHoverEValid, &b_ggHi_phoHoverEValid);
   fChain->SetBranchAddress("ggHi_phoSigmaIEtaIEta", &ggHi_phoSigmaIEtaIEta, &b_ggHi_phoSigmaIEtaIEta);
   fChain->SetBranchAddress("ggHi_phoR9", &ggHi_phoR9, &b_ggHi_phoR9);
   fChain->SetBranchAddress("ggHi_phoE1x5", &ggHi_phoE1x5, &b_ggHi_phoE1x5);
   fChain->SetBranchAddress("ggHi_phoE2x5", &ggHi_phoE2x5, &b_ggHi_phoE2x5);
   fChain->SetBranchAddress("ggHi_phoE3x3", &ggHi_phoE3x3, &b_ggHi_phoE3x3);
   fChain->SetBranchAddress("ggHi_phoE5x5", &ggHi_phoE5x5, &b_ggHi_phoE5x5);
   fChain->SetBranchAddress("ggHi_phoMaxEnergyXtal", &ggHi_phoMaxEnergyXtal, &b_ggHi_phoMaxEnergyXtal);
   fChain->SetBranchAddress("ggHi_phoSigmaEtaEta", &ggHi_phoSigmaEtaEta, &b_ggHi_phoSigmaEtaEta);
   fChain->SetBranchAddress("ggHi_phoSigmaIEtaIEta_2012", &ggHi_phoSigmaIEtaIEta_2012, &b_ggHi_phoSigmaIEtaIEta_2012);
   fChain->SetBranchAddress("ggHi_phoR9_2012", &ggHi_phoR9_2012, &b_ggHi_phoR9_2012);
   fChain->SetBranchAddress("ggHi_phoE1x5_2012", &ggHi_phoE1x5_2012, &b_ggHi_phoE1x5_2012);
   fChain->SetBranchAddress("ggHi_phoE2x5_2012", &ggHi_phoE2x5_2012, &b_ggHi_phoE2x5_2012);
   fChain->SetBranchAddress("ggHi_phoE3x3_2012", &ggHi_phoE3x3_2012, &b_ggHi_phoE3x3_2012);
   fChain->SetBranchAddress("ggHi_phoE5x5_2012", &ggHi_phoE5x5_2012, &b_ggHi_phoE5x5_2012);
   fChain->SetBranchAddress("ggHi_phoMaxEnergyXtal_2012", &ggHi_phoMaxEnergyXtal_2012, &b_ggHi_phoMaxEnergyXtal_2012);
   fChain->SetBranchAddress("ggHi_phoSigmaEtaEta_2012", &ggHi_phoSigmaEtaEta_2012, &b_ggHi_phoSigmaEtaEta_2012);
   fChain->SetBranchAddress("ggHi_phoHadTowerOverEm1", &ggHi_phoHadTowerOverEm1, &b_ggHi_phoHadTowerOverEm1);
   fChain->SetBranchAddress("ggHi_phoHadTowerOverEm2", &ggHi_phoHadTowerOverEm2, &b_ggHi_phoHadTowerOverEm2);
   fChain->SetBranchAddress("ggHi_phoHoverE1", &ggHi_phoHoverE1, &b_ggHi_phoHoverE1);
   fChain->SetBranchAddress("ggHi_phoHoverE2", &ggHi_phoHoverE2, &b_ggHi_phoHoverE2);
   fChain->SetBranchAddress("ggHi_phoSigmaIEtaIPhi", &ggHi_phoSigmaIEtaIPhi, &b_ggHi_phoSigmaIEtaIPhi);
   fChain->SetBranchAddress("ggHi_phoSigmaIPhiIPhi", &ggHi_phoSigmaIPhiIPhi, &b_ggHi_phoSigmaIPhiIPhi);
   fChain->SetBranchAddress("ggHi_phoR1x5", &ggHi_phoR1x5, &b_ggHi_phoR1x5);
   fChain->SetBranchAddress("ggHi_phoR2x5", &ggHi_phoR2x5, &b_ggHi_phoR2x5);
   fChain->SetBranchAddress("ggHi_phoE2nd", &ggHi_phoE2nd, &b_ggHi_phoE2nd);
   fChain->SetBranchAddress("ggHi_phoETop", &ggHi_phoETop, &b_ggHi_phoETop);
   fChain->SetBranchAddress("ggHi_phoEBottom", &ggHi_phoEBottom, &b_ggHi_phoEBottom);
   fChain->SetBranchAddress("ggHi_phoELeft", &ggHi_phoELeft, &b_ggHi_phoELeft);
   fChain->SetBranchAddress("ggHi_phoERight", &ggHi_phoERight, &b_ggHi_phoERight);
   fChain->SetBranchAddress("ggHi_phoE1x3", &ggHi_phoE1x3, &b_ggHi_phoE1x3);
   fChain->SetBranchAddress("ggHi_phoE2x2", &ggHi_phoE2x2, &b_ggHi_phoE2x2);
   fChain->SetBranchAddress("ggHi_phoE2x5Max", &ggHi_phoE2x5Max, &b_ggHi_phoE2x5Max);
   fChain->SetBranchAddress("ggHi_phoE2x5Top", &ggHi_phoE2x5Top, &b_ggHi_phoE2x5Top);
   fChain->SetBranchAddress("ggHi_phoE2x5Bottom", &ggHi_phoE2x5Bottom, &b_ggHi_phoE2x5Bottom);
   fChain->SetBranchAddress("ggHi_phoE2x5Left", &ggHi_phoE2x5Left, &b_ggHi_phoE2x5Left);
   fChain->SetBranchAddress("ggHi_phoE2x5Right", &ggHi_phoE2x5Right, &b_ggHi_phoE2x5Right);
   fChain->SetBranchAddress("ggHi_phoSigmaIEtaIPhi_2012", &ggHi_phoSigmaIEtaIPhi_2012, &b_ggHi_phoSigmaIEtaIPhi_2012);
   fChain->SetBranchAddress("ggHi_phoSigmaIPhiIPhi_2012", &ggHi_phoSigmaIPhiIPhi_2012, &b_ggHi_phoSigmaIPhiIPhi_2012);
   fChain->SetBranchAddress("ggHi_phoR1x5_2012", &ggHi_phoR1x5_2012, &b_ggHi_phoR1x5_2012);
   fChain->SetBranchAddress("ggHi_phoR2x5_2012", &ggHi_phoR2x5_2012, &b_ggHi_phoR2x5_2012);
   fChain->SetBranchAddress("ggHi_phoE2nd_2012", &ggHi_phoE2nd_2012, &b_ggHi_phoE2nd_2012);
   fChain->SetBranchAddress("ggHi_phoETop_2012", &ggHi_phoETop_2012, &b_ggHi_phoETop_2012);
   fChain->SetBranchAddress("ggHi_phoEBottom_2012", &ggHi_phoEBottom_2012, &b_ggHi_phoEBottom_2012);
   fChain->SetBranchAddress("ggHi_phoELeft_2012", &ggHi_phoELeft_2012, &b_ggHi_phoELeft_2012);
   fChain->SetBranchAddress("ggHi_phoERight_2012", &ggHi_phoERight_2012, &b_ggHi_phoERight_2012);
   fChain->SetBranchAddress("ggHi_phoE1x3_2012", &ggHi_phoE1x3_2012, &b_ggHi_phoE1x3_2012);
   fChain->SetBranchAddress("ggHi_phoE2x2_2012", &ggHi_phoE2x2_2012, &b_ggHi_phoE2x2_2012);
   fChain->SetBranchAddress("ggHi_phoE2x5Max_2012", &ggHi_phoE2x5Max_2012, &b_ggHi_phoE2x5Max_2012);
   fChain->SetBranchAddress("ggHi_phoE2x5Top_2012", &ggHi_phoE2x5Top_2012, &b_ggHi_phoE2x5Top_2012);
   fChain->SetBranchAddress("ggHi_phoE2x5Bottom_2012", &ggHi_phoE2x5Bottom_2012, &b_ggHi_phoE2x5Bottom_2012);
   fChain->SetBranchAddress("ggHi_phoE2x5Left_2012", &ggHi_phoE2x5Left_2012, &b_ggHi_phoE2x5Left_2012);
   fChain->SetBranchAddress("ggHi_phoE2x5Right_2012", &ggHi_phoE2x5Right_2012, &b_ggHi_phoE2x5Right_2012);
   fChain->SetBranchAddress("ggHi_phoBC1E", &ggHi_phoBC1E, &b_ggHi_phoBC1E);
   fChain->SetBranchAddress("ggHi_phoBC1Ecorr", &ggHi_phoBC1Ecorr, &b_ggHi_phoBC1Ecorr);
   fChain->SetBranchAddress("ggHi_phoBC1Eta", &ggHi_phoBC1Eta, &b_ggHi_phoBC1Eta);
   fChain->SetBranchAddress("ggHi_phoBC1Phi", &ggHi_phoBC1Phi, &b_ggHi_phoBC1Phi);
   fChain->SetBranchAddress("ggHi_phoBC1size", &ggHi_phoBC1size, &b_ggHi_phoBC1size);
   fChain->SetBranchAddress("ggHi_phoBC1flags", &ggHi_phoBC1flags, &b_ggHi_phoBC1flags);
   fChain->SetBranchAddress("ggHi_phoBC1inClean", &ggHi_phoBC1inClean, &b_ggHi_phoBC1inClean);
   fChain->SetBranchAddress("ggHi_phoBC1inUnClean", &ggHi_phoBC1inUnClean, &b_ggHi_phoBC1inUnClean);
   fChain->SetBranchAddress("ggHi_phoBC1rawID", &ggHi_phoBC1rawID, &b_ggHi_phoBC1rawID);
   fChain->SetBranchAddress("ggHi_pho_ecalClusterIsoR1", &ggHi_pho_ecalClusterIsoR1, &b_ggHi_pho_ecalClusterIsoR1);
   fChain->SetBranchAddress("ggHi_pho_ecalClusterIsoR2", &ggHi_pho_ecalClusterIsoR2, &b_ggHi_pho_ecalClusterIsoR2);
   fChain->SetBranchAddress("ggHi_pho_ecalClusterIsoR3", &ggHi_pho_ecalClusterIsoR3, &b_ggHi_pho_ecalClusterIsoR3);
   fChain->SetBranchAddress("ggHi_pho_ecalClusterIsoR4", &ggHi_pho_ecalClusterIsoR4, &b_ggHi_pho_ecalClusterIsoR4);
   fChain->SetBranchAddress("ggHi_pho_ecalClusterIsoR5", &ggHi_pho_ecalClusterIsoR5, &b_ggHi_pho_ecalClusterIsoR5);
   fChain->SetBranchAddress("ggHi_pho_hcalRechitIsoR1", &ggHi_pho_hcalRechitIsoR1, &b_ggHi_pho_hcalRechitIsoR1);
   fChain->SetBranchAddress("ggHi_pho_hcalRechitIsoR2", &ggHi_pho_hcalRechitIsoR2, &b_ggHi_pho_hcalRechitIsoR2);
   fChain->SetBranchAddress("ggHi_pho_hcalRechitIsoR3", &ggHi_pho_hcalRechitIsoR3, &b_ggHi_pho_hcalRechitIsoR3);
   fChain->SetBranchAddress("ggHi_pho_hcalRechitIsoR4", &ggHi_pho_hcalRechitIsoR4, &b_ggHi_pho_hcalRechitIsoR4);
   fChain->SetBranchAddress("ggHi_pho_hcalRechitIsoR5", &ggHi_pho_hcalRechitIsoR5, &b_ggHi_pho_hcalRechitIsoR5);
   fChain->SetBranchAddress("ggHi_pho_trackIsoR1PtCut20", &ggHi_pho_trackIsoR1PtCut20, &b_ggHi_pho_trackIsoR1PtCut20);
   fChain->SetBranchAddress("ggHi_pho_trackIsoR2PtCut20", &ggHi_pho_trackIsoR2PtCut20, &b_ggHi_pho_trackIsoR2PtCut20);
   fChain->SetBranchAddress("ggHi_pho_trackIsoR3PtCut20", &ggHi_pho_trackIsoR3PtCut20, &b_ggHi_pho_trackIsoR3PtCut20);
   fChain->SetBranchAddress("ggHi_pho_trackIsoR4PtCut20", &ggHi_pho_trackIsoR4PtCut20, &b_ggHi_pho_trackIsoR4PtCut20);
   fChain->SetBranchAddress("ggHi_pho_trackIsoR5PtCut20", &ggHi_pho_trackIsoR5PtCut20, &b_ggHi_pho_trackIsoR5PtCut20);
   fChain->SetBranchAddress("ggHi_pho_swissCrx", &ggHi_pho_swissCrx, &b_ggHi_pho_swissCrx);
   fChain->SetBranchAddress("ggHi_pho_seedTime", &ggHi_pho_seedTime, &b_ggHi_pho_seedTime);
   fChain->SetBranchAddress("ggHi_pfcIso1", &ggHi_pfcIso1, &b_ggHi_pfcIso1);
   fChain->SetBranchAddress("ggHi_pfcIso2", &ggHi_pfcIso2, &b_ggHi_pfcIso2);
   fChain->SetBranchAddress("ggHi_pfcIso3", &ggHi_pfcIso3, &b_ggHi_pfcIso3);
   fChain->SetBranchAddress("ggHi_pfcIso4", &ggHi_pfcIso4, &b_ggHi_pfcIso4);
   fChain->SetBranchAddress("ggHi_pfcIso5", &ggHi_pfcIso5, &b_ggHi_pfcIso5);
   fChain->SetBranchAddress("ggHi_pfpIso1", &ggHi_pfpIso1, &b_ggHi_pfpIso1);
   fChain->SetBranchAddress("ggHi_pfpIso2", &ggHi_pfpIso2, &b_ggHi_pfpIso2);
   fChain->SetBranchAddress("ggHi_pfpIso3", &ggHi_pfpIso3, &b_ggHi_pfpIso3);
   fChain->SetBranchAddress("ggHi_pfpIso4", &ggHi_pfpIso4, &b_ggHi_pfpIso4);
   fChain->SetBranchAddress("ggHi_pfpIso5", &ggHi_pfpIso5, &b_ggHi_pfpIso5);
   fChain->SetBranchAddress("ggHi_pfnIso1", &ggHi_pfnIso1, &b_ggHi_pfnIso1);
   fChain->SetBranchAddress("ggHi_pfnIso2", &ggHi_pfnIso2, &b_ggHi_pfnIso2);
   fChain->SetBranchAddress("ggHi_pfnIso3", &ggHi_pfnIso3, &b_ggHi_pfnIso3);
   fChain->SetBranchAddress("ggHi_pfnIso4", &ggHi_pfnIso4, &b_ggHi_pfnIso4);
   fChain->SetBranchAddress("ggHi_pfnIso5", &ggHi_pfnIso5, &b_ggHi_pfnIso5);
   fChain->SetBranchAddress("ggHi_pfpIso1subSC", &ggHi_pfpIso1subSC, &b_ggHi_pfpIso1subSC);
   fChain->SetBranchAddress("ggHi_pfpIso2subSC", &ggHi_pfpIso2subSC, &b_ggHi_pfpIso2subSC);
   fChain->SetBranchAddress("ggHi_pfpIso3subSC", &ggHi_pfpIso3subSC, &b_ggHi_pfpIso3subSC);
   fChain->SetBranchAddress("ggHi_pfpIso4subSC", &ggHi_pfpIso4subSC, &b_ggHi_pfpIso4subSC);
   fChain->SetBranchAddress("ggHi_pfpIso5subSC", &ggHi_pfpIso5subSC, &b_ggHi_pfpIso5subSC);
   fChain->SetBranchAddress("ggHi_pfcIso1subUE", &ggHi_pfcIso1subUE, &b_ggHi_pfcIso1subUE);
   fChain->SetBranchAddress("ggHi_pfcIso2subUE", &ggHi_pfcIso2subUE, &b_ggHi_pfcIso2subUE);
   fChain->SetBranchAddress("ggHi_pfcIso3subUE", &ggHi_pfcIso3subUE, &b_ggHi_pfcIso3subUE);
   fChain->SetBranchAddress("ggHi_pfcIso4subUE", &ggHi_pfcIso4subUE, &b_ggHi_pfcIso4subUE);
   fChain->SetBranchAddress("ggHi_pfcIso5subUE", &ggHi_pfcIso5subUE, &b_ggHi_pfcIso5subUE);
   fChain->SetBranchAddress("ggHi_pfpIso1subUE", &ggHi_pfpIso1subUE, &b_ggHi_pfpIso1subUE);
   fChain->SetBranchAddress("ggHi_pfpIso2subUE", &ggHi_pfpIso2subUE, &b_ggHi_pfpIso2subUE);
   fChain->SetBranchAddress("ggHi_pfpIso3subUE", &ggHi_pfpIso3subUE, &b_ggHi_pfpIso3subUE);
   fChain->SetBranchAddress("ggHi_pfpIso4subUE", &ggHi_pfpIso4subUE, &b_ggHi_pfpIso4subUE);
   fChain->SetBranchAddress("ggHi_pfpIso5subUE", &ggHi_pfpIso5subUE, &b_ggHi_pfpIso5subUE);
   fChain->SetBranchAddress("ggHi_pfnIso1subUE", &ggHi_pfnIso1subUE, &b_ggHi_pfnIso1subUE);
   fChain->SetBranchAddress("ggHi_pfnIso2subUE", &ggHi_pfnIso2subUE, &b_ggHi_pfnIso2subUE);
   fChain->SetBranchAddress("ggHi_pfnIso3subUE", &ggHi_pfnIso3subUE, &b_ggHi_pfnIso3subUE);
   fChain->SetBranchAddress("ggHi_pfnIso4subUE", &ggHi_pfnIso4subUE, &b_ggHi_pfnIso4subUE);
   fChain->SetBranchAddress("ggHi_pfnIso5subUE", &ggHi_pfnIso5subUE, &b_ggHi_pfnIso5subUE);
   fChain->SetBranchAddress("ggHi_pfpIso1subSCsubUE", &ggHi_pfpIso1subSCsubUE, &b_ggHi_pfpIso1subSCsubUE);
   fChain->SetBranchAddress("ggHi_pfpIso2subSCsubUE", &ggHi_pfpIso2subSCsubUE, &b_ggHi_pfpIso2subSCsubUE);
   fChain->SetBranchAddress("ggHi_pfpIso3subSCsubUE", &ggHi_pfpIso3subSCsubUE, &b_ggHi_pfpIso3subSCsubUE);
   fChain->SetBranchAddress("ggHi_pfpIso4subSCsubUE", &ggHi_pfpIso4subSCsubUE, &b_ggHi_pfpIso4subSCsubUE);
   fChain->SetBranchAddress("ggHi_pfpIso5subSCsubUE", &ggHi_pfpIso5subSCsubUE, &b_ggHi_pfpIso5subSCsubUE);
   fChain->SetBranchAddress("ggHi_pfcIso1pTgt1p0subUE", &ggHi_pfcIso1pTgt1p0subUE, &b_ggHi_pfcIso1pTgt1p0subUE);
   fChain->SetBranchAddress("ggHi_pfcIso2pTgt1p0subUE", &ggHi_pfcIso2pTgt1p0subUE, &b_ggHi_pfcIso2pTgt1p0subUE);
   fChain->SetBranchAddress("ggHi_pfcIso3pTgt1p0subUE", &ggHi_pfcIso3pTgt1p0subUE, &b_ggHi_pfcIso3pTgt1p0subUE);
   fChain->SetBranchAddress("ggHi_pfcIso4pTgt1p0subUE", &ggHi_pfcIso4pTgt1p0subUE, &b_ggHi_pfcIso4pTgt1p0subUE);
   fChain->SetBranchAddress("ggHi_pfcIso5pTgt1p0subUE", &ggHi_pfcIso5pTgt1p0subUE, &b_ggHi_pfcIso5pTgt1p0subUE);
   fChain->SetBranchAddress("ggHi_pfcIso1pTgt2p0subUE", &ggHi_pfcIso1pTgt2p0subUE, &b_ggHi_pfcIso1pTgt2p0subUE);
   fChain->SetBranchAddress("ggHi_pfcIso2pTgt2p0subUE", &ggHi_pfcIso2pTgt2p0subUE, &b_ggHi_pfcIso2pTgt2p0subUE);
   fChain->SetBranchAddress("ggHi_pfcIso3pTgt2p0subUE", &ggHi_pfcIso3pTgt2p0subUE, &b_ggHi_pfcIso3pTgt2p0subUE);
   fChain->SetBranchAddress("ggHi_pfcIso4pTgt2p0subUE", &ggHi_pfcIso4pTgt2p0subUE, &b_ggHi_pfcIso4pTgt2p0subUE);
   fChain->SetBranchAddress("ggHi_pfcIso5pTgt2p0subUE", &ggHi_pfcIso5pTgt2p0subUE, &b_ggHi_pfcIso5pTgt2p0subUE);
   fChain->SetBranchAddress("ggHi_pfcIso1pTgt3p0subUE", &ggHi_pfcIso1pTgt3p0subUE, &b_ggHi_pfcIso1pTgt3p0subUE);
   fChain->SetBranchAddress("ggHi_pfcIso2pTgt3p0subUE", &ggHi_pfcIso2pTgt3p0subUE, &b_ggHi_pfcIso2pTgt3p0subUE);
   fChain->SetBranchAddress("ggHi_pfcIso3pTgt3p0subUE", &ggHi_pfcIso3pTgt3p0subUE, &b_ggHi_pfcIso3pTgt3p0subUE);
   fChain->SetBranchAddress("ggHi_pfcIso4pTgt3p0subUE", &ggHi_pfcIso4pTgt3p0subUE, &b_ggHi_pfcIso4pTgt3p0subUE);
   fChain->SetBranchAddress("ggHi_pfcIso5pTgt3p0subUE", &ggHi_pfcIso5pTgt3p0subUE, &b_ggHi_pfcIso5pTgt3p0subUE);
   fChain->SetBranchAddress("ggHi_pfcIso2subUEec", &ggHi_pfcIso2subUEec, &b_ggHi_pfcIso2subUEec);
   fChain->SetBranchAddress("ggHi_pfcIso3subUEec", &ggHi_pfcIso3subUEec, &b_ggHi_pfcIso3subUEec);
   fChain->SetBranchAddress("ggHi_pfcIso4subUEec", &ggHi_pfcIso4subUEec, &b_ggHi_pfcIso4subUEec);
   fChain->SetBranchAddress("ggHi_pfpIso2subUEec", &ggHi_pfpIso2subUEec, &b_ggHi_pfpIso2subUEec);
   fChain->SetBranchAddress("ggHi_pfpIso3subUEec", &ggHi_pfpIso3subUEec, &b_ggHi_pfpIso3subUEec);
   fChain->SetBranchAddress("ggHi_pfpIso4subUEec", &ggHi_pfpIso4subUEec, &b_ggHi_pfpIso4subUEec);
   fChain->SetBranchAddress("ggHi_pfnIso2subUEec", &ggHi_pfnIso2subUEec, &b_ggHi_pfnIso2subUEec);
   fChain->SetBranchAddress("ggHi_pfnIso3subUEec", &ggHi_pfnIso3subUEec, &b_ggHi_pfnIso3subUEec);
   fChain->SetBranchAddress("ggHi_pfnIso4subUEec", &ggHi_pfnIso4subUEec, &b_ggHi_pfnIso4subUEec);
   fChain->SetBranchAddress("ggHi_pfcIso2pTgt2p0subUEec", &ggHi_pfcIso2pTgt2p0subUEec, &b_ggHi_pfcIso2pTgt2p0subUEec);
   fChain->SetBranchAddress("ggHi_pfcIso3pTgt2p0subUEec", &ggHi_pfcIso3pTgt2p0subUEec, &b_ggHi_pfcIso3pTgt2p0subUEec);
   fChain->SetBranchAddress("ggHi_pfcIso4pTgt2p0subUEec", &ggHi_pfcIso4pTgt2p0subUEec, &b_ggHi_pfcIso4pTgt2p0subUEec);
   fChain->SetBranchAddress("run", &run, &b_run);
   fChain->SetBranchAddress("evt", &evt, &b_evt);
   fChain->SetBranchAddress("lumi", &lumi, &b_lumi);
   fChain->SetBranchAddress("vx", &vx, &b_vx);
   fChain->SetBranchAddress("vy", &vy, &b_vy);
   fChain->SetBranchAddress("vz", &vz, &b_vz);
   fChain->SetBranchAddress("hiBin", &hiBin, &b_hiBin);
   fChain->SetBranchAddress("hiHF", &hiHF, &b_hiHF);
   fChain->SetBranchAddress("hlt_HLT_HIGEDPhoton10_v10", &hlt_HLT_HIGEDPhoton10_v10, &b_hlt_HLT_HIGEDPhoton10_v10);
   fChain->SetBranchAddress("hlt_HLT_HIGEDPhoton20_v10", &hlt_HLT_HIGEDPhoton20_v10, &b_hlt_HLT_HIGEDPhoton20_v10);
   fChain->SetBranchAddress("hlt_HLT_HIGEDPhoton30_v10", &hlt_HLT_HIGEDPhoton30_v10, &b_hlt_HLT_HIGEDPhoton30_v10);
   fChain->SetBranchAddress("hlt_HLT_HIGEDPhoton40_v10", &hlt_HLT_HIGEDPhoton40_v10, &b_hlt_HLT_HIGEDPhoton40_v10);
   fChain->SetBranchAddress("hlt_HLT_HIGEDPhoton50_v10", &hlt_HLT_HIGEDPhoton50_v10, &b_hlt_HLT_HIGEDPhoton50_v10);
   fChain->SetBranchAddress("hlt_HLT_HIGEDPhoton60_v10", &hlt_HLT_HIGEDPhoton60_v10, &b_hlt_HLT_HIGEDPhoton60_v10);
   fChain->SetBranchAddress("hlt_L1_SingleEG12_BptxAND", &hlt_L1_SingleEG12_BptxAND, &b_hlt_L1_SingleEG12_BptxAND);
   fChain->SetBranchAddress("hlt_L1_SingleEG15_BptxAND", &hlt_L1_SingleEG15_BptxAND, &b_hlt_L1_SingleEG15_BptxAND);
   fChain->SetBranchAddress("hlt_L1_SingleEG21_BptxAND", &hlt_L1_SingleEG21_BptxAND, &b_hlt_L1_SingleEG21_BptxAND);
   fChain->SetBranchAddress("hlt_L1_SingleEG30_BptxAND", &hlt_L1_SingleEG30_BptxAND, &b_hlt_L1_SingleEG30_BptxAND);
   fChain->SetBranchAddress("skim_pclusterCompatibilityFilter", &skim_pclusterCompatibilityFilter, &b_skim_pclusterCompatibilityFilter);
   fChain->SetBranchAddress("skim_pprimaryVertexFilter", &skim_pprimaryVertexFilter, &b_skim_pprimaryVertexFilter);
   fChain->SetBranchAddress("skim_pphfCoincFilter4Th2", &skim_pphfCoincFilter4Th2, &b_skim_pphfCoincFilter4Th2);
   fChain->SetBranchAddress("skim_pphfCoincFilter1Th3", &skim_pphfCoincFilter1Th3, &b_skim_pphfCoincFilter1Th3);
   fChain->SetBranchAddress("skim_pphfCoincFilter2Th3", &skim_pphfCoincFilter2Th3, &b_skim_pphfCoincFilter2Th3);
   fChain->SetBranchAddress("skim_pphfCoincFilter3Th3", &skim_pphfCoincFilter3Th3, &b_skim_pphfCoincFilter3Th3);
   fChain->SetBranchAddress("skim_pphfCoincFilter4Th3", &skim_pphfCoincFilter4Th3, &b_skim_pphfCoincFilter4Th3);
   fChain->SetBranchAddress("skim_pphfCoincFilter5Th3", &skim_pphfCoincFilter5Th3, &b_skim_pphfCoincFilter5Th3);
   fChain->SetBranchAddress("skim_pphfCoincFilter1Th4", &skim_pphfCoincFilter1Th4, &b_skim_pphfCoincFilter1Th4);
   fChain->SetBranchAddress("skim_pphfCoincFilter2Th4", &skim_pphfCoincFilter2Th4, &b_skim_pphfCoincFilter2Th4);
   fChain->SetBranchAddress("skim_pphfCoincFilter3Th4", &skim_pphfCoincFilter3Th4, &b_skim_pphfCoincFilter3Th4);
   fChain->SetBranchAddress("skim_pphfCoincFilter4Th4", &skim_pphfCoincFilter4Th4, &b_skim_pphfCoincFilter4Th4);
   fChain->SetBranchAddress("skim_pphfCoincFilter5Th4", &skim_pphfCoincFilter5Th4, &b_skim_pphfCoincFilter5Th4);
   fChain->SetBranchAddress("skim_pphfCoincFilter1Th5", &skim_pphfCoincFilter1Th5, &b_skim_pphfCoincFilter1Th5);
   fChain->SetBranchAddress("skim_pphfCoincFilter2Th5", &skim_pphfCoincFilter2Th5, &b_skim_pphfCoincFilter2Th5);
   fChain->SetBranchAddress("skim_pphfCoincFilter3Th5", &skim_pphfCoincFilter3Th5, &b_skim_pphfCoincFilter3Th5);
   fChain->SetBranchAddress("skim_pphfCoincFilter4Th5", &skim_pphfCoincFilter4Th5, &b_skim_pphfCoincFilter4Th5);
   fChain->SetBranchAddress("skim_pphfCoincFilter5Th5", &skim_pphfCoincFilter5Th5, &b_skim_pphfCoincFilter5Th5);
   fChain->SetBranchAddress("skim_pphfCoincFilter1Th6", &skim_pphfCoincFilter1Th6, &b_skim_pphfCoincFilter1Th6);
   fChain->SetBranchAddress("skim_pphfCoincFilter2Th6", &skim_pphfCoincFilter2Th6, &b_skim_pphfCoincFilter2Th6);
   fChain->SetBranchAddress("skim_pphfCoincFilter3Th6", &skim_pphfCoincFilter3Th6, &b_skim_pphfCoincFilter3Th6);
   fChain->SetBranchAddress("skim_pphfCoincFilter4Th6", &skim_pphfCoincFilter4Th6, &b_skim_pphfCoincFilter4Th6);
   fChain->SetBranchAddress("skim_pphfCoincFilter5Th6", &skim_pphfCoincFilter5Th6, &b_skim_pphfCoincFilter5Th6);
   Notify();
}

Bool_t GammaJet2023_PbPbData::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void GammaJet2023_PbPbData::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t GammaJet2023_PbPbData::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef GammaJet2023_PbPbData_cxx
