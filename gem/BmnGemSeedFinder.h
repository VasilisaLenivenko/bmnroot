
// Base Class Headers ----------------

#include "FairTask.h"
#include "TClonesArray.h"
#include "TString.h"
#include "BmnGemTrack.h"
#include <iostream>
#include <vector>
#include "TMath.h"
#include "TVector3.h"
#include "BmnGemSeedFinderQA.h"
#include "BmnGemStripHit.h"
#include "FairMCPoint.h"
#include "CbmMCTrack.h"
#include "CbmStsPoint.h"
#include "BmnEnums.h"
#include "FairRunAna.h"
#include "FairField.h"

using namespace std;

class BmnGemSeedFinder : public FairTask {
public:

    // Constructors/Destructors ---------
    BmnGemSeedFinder();
    virtual ~BmnGemSeedFinder();

    BmnStatus FindStrightSeeds(); // for runs w/o magnetic field
    
    BmnStatus FindYZSeeds();
    void FindSeeds(Int_t station, Int_t gate, Bool_t isIdeal, TClonesArray* arr);
    UInt_t SearchTrackCandidates(Int_t startStation, Int_t gate, Bool_t isIdeal, Bool_t isLeft, TClonesArray* arr);
    void SearchTrackCandInLine(const Int_t i, const Int_t y, BmnGemTrack* tr, Int_t* hitCntr, Int_t* maxDist, Int_t* dist, Int_t* startBin, Int_t* prevStation, Int_t gate, Bool_t isIdeal);
    Bool_t CalculateTrackParams(BmnGemTrack* tr, TVector3* circPar, TVector3* linePar, Short_t q);
    Bool_t CalculateTrackParamsSpiral(BmnGemTrack* tr, TVector3* spirPar, TVector3* linePar, Short_t q);
    Bool_t CalculateTrackParamsParabolicSpiral(BmnGemTrack* tr, TLorentzVector* spirPar, TVector3* linePar, Short_t q);
    BmnStatus DoSeeding(Int_t min, Int_t max, TClonesArray* arr);
    TVector3 CircleFit(BmnGemTrack* track);
    BmnStatus FillTgStation(Short_t st, vector<BmnGemTrack>& tg);
    BmnStatus CombineTg(vector<BmnGemTrack> tg1, vector<BmnGemTrack> tg2, vector<BmnGemTrack>& tg);
    BmnStatus CollectSeeds(vector<BmnGemTrack>& tg);

    
    void SetHitsUnused(BmnGemTrack* tr);
        
    Float_t NewtonSolver(Float_t A0, Float_t A1, Float_t A2, Float_t A22);
    
    void FillAddr();
    void FillAddrWithLorentz(Float_t sigma_x, Float_t yStep, Float_t trs);
    
    void SetUseLorentz(Bool_t use) {
        fUseLorentz = use;
    }

    //some useful functions
    Float_t Dist(Float_t x1, Float_t y1, Float_t x2, Float_t y2);
    Float_t Sqr(Float_t x);
    BmnGemStripHit* GetHit(Int_t i);
    
    void SetTrs(Float_t trs) {kTRS = trs;}
    void SetYstep(Float_t stp) {kY_STEP = stp;}
    void SetSigX(Float_t sig) {kSIG_X = sig;}
    void SetNbins(Int_t n) {fNBins = n;}
    void SetField(Bool_t f) {fIsField = f;}

    virtual InitStatus Init();
    virtual void Exec(Option_t* opt);
    virtual void Finish();

private:

    // Private Data Members ------------
    TString fHitsBranchName;
    TString fSeedsBranchName;

    Bool_t fUseLorentz; //flag for using Lorentz filtration
    
    Float_t kSIG_X;
    UInt_t kY_STEP;
    Float_t kTRS;
    UInt_t kNHITSFORFIT;
    
    UInt_t fNBins; // number of bins in histogram  
    Float_t fMin;
    Float_t fMax;
    Float_t fWidth;

    UInt_t fEventNo; // event counter

    TClonesArray* fGemHitsArray;
    TClonesArray* fGemSeedsArray;
    TClonesArray* fMCTracksArray;
    TClonesArray* fMCPointsArray;

    FairField* fField;
    
    Bool_t fIsField; // is mag. field or not

    ClassDef(BmnGemSeedFinder, 1);
};
