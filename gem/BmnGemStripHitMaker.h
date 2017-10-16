#ifndef BMNGEMSTRIPHITMAKER_H
#define BMNGEMSTRIPHITMAKER_H 1

#include <iostream>
#include <fstream>
#include <sstream>

#include "Rtypes.h"
#include "TClonesArray.h"
#include "TRegexp.h"
#include "TString.h"

#include "FairTask.h"
#include "FairMCPoint.h"

#include <UniDbDetectorParameter.h>
#include <UniDbRun.h>
#include "FairField.h"
#include "BmnGemStripDigit.h"
#include "BmnGemStripHit.h"
#include "BmnGemStripStationSet.h"
#include "BmnGemStripConfiguration.h"
#include "BmnGemAlignmentCorrections.h"
#include "BmnGemAlignCorrections.h"

using namespace std;

class BmnGemStripHitMaker : public FairTask {
public:

    BmnGemStripHitMaker();
    BmnGemStripHitMaker(Bool_t);

    virtual ~BmnGemStripHitMaker();

    //void SetVerbosity(Bool_t verbose);

    virtual InitStatus Init();

    virtual void Exec(Option_t* opt);

    virtual void Finish();

    void ProcessDigits();

    void SetHitMatching(Bool_t opt = kTRUE) {
        fHitMatching = opt;
    }

    void SetCurrentConfig(BmnGemStripConfiguration::GEM_CONFIG config) {
        fCurrentConfig = config;
    }

    void SetAlignmentCorrectionsFileName(TString filename) {
        fAlignCorrFileName = filename;
    }

    void SetAlignmentCorrectionsFileName(Int_t run_period, Int_t file_number) {
        if (run_period == 5)
            fAlignCorrFileName = "$VMCWORKDIR/input/alignCorrsLocal_GEM.root";
        else if (run_period == 6) {
            fAlignCorrFileName = "alignment_GEM.root";
            UniDbDetectorParameter::ReadRootFile(run_period, file_number, "BM@N", "alignment", (Char_t*) fAlignCorrFileName.Data());
        }
        else
            fAlignCorrFileName = "";
    }

    Double_t GetLorentzByField(Double_t By, Int_t station) { //By in kGs
        const Int_t nStation = 6;

        //parabolic coefficients are gotten from fits indepedend for each station.
        //shifts for fit gotten from IterateLorentz.C macro
        Double_t p0[nStation] = {-0.142402, 0.0785986, 0.0591568, -0.0137718, 0.0712004, 0.093149};
        Double_t p1[nStation] = {0.090507, -0.00764583, 0.0070895, 0.0343746, -0.00760665, -0.00253474};
        Double_t p2[nStation] = {-0.0058462, 0.00274913, 0.00195576, -0.000538933, 0.00249631, 0.00221692};
        return p0[station] + p1[station] * By + p2[station] * By * By;
//        return 0;
    }

private:

    TString fInputPointsBranchName;
    TString fInputDigitsBranchName;
    TString fInputDigitMatchesBranchName;
    TString fBmnEventHeaderBranchName;
    
    TString fT0Branch;
    TString fVetoBranch;
    TString fBC2Branch;
    TString fBDBranch;

    TString fOutputHitsBranchName;
    TString fOutputHitMatchesBranchName;

    /** Input array of Gem Points **/
    TClonesArray* fBmnGemStripPointsArray;
    TClonesArray* fBmnGemStripDigitsArray;
    TClonesArray* fBmnGemStripDigitMatchesArray;
    TClonesArray* fBmnEventHeader;
    
    TClonesArray* fT0Array;
    TClonesArray* fVetoArray;
    TClonesArray* fBC2Array;
    TClonesArray* fBDArray;

    /** Output array of Gem Hits **/
    TClonesArray* fBmnGemStripHitsArray;

    /** Output array of GEM Hit Matches **/
    TClonesArray* fBmnGemStripHitMatchesArray;

    Bool_t fHitMatching;
    Bool_t fIsExp; // Specify type of input data (MC or real data)

    BmnGemStripConfiguration::GEM_CONFIG fCurrentConfig;

    BmnGemStripStationSet *StationSet; //Entire GEM detector

    TString fAlignCorrFileName; // a file with geometry corrections
    void ReadAlignCorrFile(TString, Double_t***); // read corrections from the file
    Double_t*** corr; // array to store the corrections

    FairField* fField;

    ClassDef(BmnGemStripHitMaker, 1);
};


#endif
