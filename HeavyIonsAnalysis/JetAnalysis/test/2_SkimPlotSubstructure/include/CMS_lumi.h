#include "TPad.h"
#include "TLatex.h"
#include "TLine.h"
#include "TString.h"
#include "TBox.h"

//
// Global variables
//
TString cmsText     = "CMS";
float cmsTextFont   = 61;  // default is helvetic-bold

bool writeExtraText = true;
TString extraText   = "Heavy Ion Preliminary"; // "Preliminary" or "Simulation"
float extraTextFont = 52;  // default is helvetica-italics

// text sizes and text offsets with respect to the top frame
// in unit of the top margin size
float lumiTextSize     = 0.6;
float lumiTextOffset   = 0.2;
float cmsTextSize      = 0.75;
float cmsTextOffset    = 0.1;  // in case of ticks

float relPosX    = 0.045;
float relPosY    = 0.035;
float relExtraDY = 1.2;

// ratio of "CMS" and extra text size
float extraOverCmsTextSize  = 0.76;

TString lumi_sqrtS = "";
TString lumi_13TeV = "PbPb 5.36 TeV";
TString lumi_8TeV  = "";
TString lumi_7TeV  = "";
TString lumi_5TeV  = "";

bool drawLogo      = false;

void CMS_lumi(TPad* pad, int iPeriod=3, int iPosX=10);

void CMS_lumi(TPad* pad, int iPeriod, int iPosX) {
    bool outOfFrame = false;
    if (iPosX/10==0) outOfFrame = true;

    int alignY_=3;
    int alignX_=2;
    if (iPosX/10==0) alignX_=1;
    if (iPosX==0)    alignY_=1;
    if (iPosX/10==1) alignX_=1;
    if (iPosX/10==2) alignX_=2;
    if (iPosX/10==3) alignX_=3;
    int align_ = 10*alignX_ + alignY_;

    float H = pad->GetWh();
    float W = pad->GetWw();
    float l = pad->GetLeftMargin();
    float t = pad->GetTopMargin();
    float r = pad->GetRightMargin();
    float b = pad->GetBottomMargin();

    pad->cd();

    TString lumiText;
    if (iPeriod==3) lumiText = lumi_13TeV;

    TLatex latex;
    latex.SetNDC();
    latex.SetTextAngle(0);
    latex.SetTextColor(kBlack);    

    float extraTextSize = extraOverCmsTextSize*cmsTextSize;

    latex.SetTextFont(42);
    latex.SetTextAlign(31); 
    latex.SetTextSize(lumiTextSize*t);    
    latex.DrawLatex(1-r,1-t+lumiTextOffset*t,lumiText);

    if (outOfFrame) {
        latex.SetTextFont(cmsTextFont);
        latex.SetTextAlign(11); 
        latex.SetTextSize(cmsTextSize*t);    
        latex.DrawLatex(l,1-t+lumiTextOffset*t,cmsText);
    }

    pad->cd();

    float posX_ = 0;
    if (iPosX%10<=1) {
        posX_ =   l + relPosX*(1-l-r);
    } else if (iPosX%10==2) {
        posX_ =  l + 0.5*(1-l-r);
    } else if (iPosX%10==3) {
        posX_ =  1-r - relPosX*(1-l-r);
    }

    float posY_ = 1-t - relPosY*(1-t-b);
    if (!outOfFrame) {
        latex.SetTextFont(cmsTextFont);
        latex.SetTextSize(cmsTextSize*t);
        latex.SetTextAlign(align_);
        latex.DrawLatex(posX_, posY_, cmsText);

        if (writeExtraText) {
            latex.SetTextFont(extraTextFont);
            latex.SetTextAlign(align_);
            latex.SetTextSize(extraTextSize*t);
            latex.DrawLatex(posX_, posY_- relExtraDY*cmsTextSize*t, extraText);
        }
    }
}
