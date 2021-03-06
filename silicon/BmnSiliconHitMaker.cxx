#include "BmnSiliconHitMaker.h"

#include "BmnSiliconModule.h"

#include "TClonesArray.h"
#include "TSystem.h"

static Float_t workTime = 0.0;

BmnSiliconHitMaker::BmnSiliconHitMaker()
: fHitMatching(kTRUE) {

    fInputPointsBranchName = "SiliconPoint";
    fInputDigitsBranchName = "BmnSiliconDigit";
    fInputDigitMatchesBranchName = "BmnSiliconDigitMatch";

    fOutputHitsBranchName = "BmnSiliconHit";
    fOutputHitMatchesBranchName = "BmnSiliconHitMatch";

    fVerbose = 1;

    StationSet = NULL;
}

BmnSiliconHitMaker::BmnSiliconHitMaker(Bool_t isExp)
: fHitMatching(kTRUE) {

    fInputPointsBranchName = "SiliconPoint";
    fInputDigitsBranchName = (!isExp) ? "BmnSiliconDigit" : "SILICON";
    fInputDigitMatchesBranchName = "BmnSiliconDigitMatch";

    fOutputHitsBranchName = "BmnSiliconHit";
    fOutputHitMatchesBranchName = "BmnSiliconHitMatch";

    fVerbose = 1;

    StationSet = NULL;
}

BmnSiliconHitMaker::~BmnSiliconHitMaker() {

}

InitStatus BmnSiliconHitMaker::Init() {

    if (fVerbose) cout << "\nBmnSiliconHitMaker::Init()\n ";

    FairRootManager* ioman = FairRootManager::Instance();

    fBmnSiliconDigitsArray = (TClonesArray*) ioman->GetObject(fInputDigitsBranchName);
    if (!fBmnSiliconDigitsArray)
    {
      cout<<"BmnSiliconHitMaker::Init(): branch "<<fInputDigitsBranchName<<" not found! Task will be deactivated"<<endl;
      SetActive(kFALSE);
      return kERROR;
    }
    fBmnSiliconDigitMatchesArray = (TClonesArray*) ioman->GetObject(fInputDigitMatchesBranchName);

    if (fVerbose) {
        if (fBmnSiliconDigitMatchesArray) cout << "  Strip matching information exists!\n";
        else cout << "  Strip matching information doesn`t exist!\n";
    }

    fBmnSiliconHitsArray = new TClonesArray(fOutputHitsBranchName);
    ioman->Register(fOutputHitsBranchName, "SILICON", fBmnSiliconHitsArray, kTRUE);

    if (fHitMatching && fBmnSiliconDigitMatchesArray) {
        fBmnSiliconHitMatchesArray = new TClonesArray("BmnMatch");
        ioman->Register(fOutputHitMatchesBranchName, "SILICON", fBmnSiliconHitMatchesArray, kTRUE);
    } else {
        fBmnSiliconHitMatchesArray = 0;
    }

    TString gPathSiliconConfig = gSystem->Getenv("VMCWORKDIR");
        gPathSiliconConfig += "/silicon/XMLConfigs/";

    //Create Silicon detector --------------------------------------------------

    StationSet = new BmnSiliconStationSet(gPathSiliconConfig + "SiliconRunSpring2017.xml");
    cout << "   Current Configuration : RunSpring2017" << "\n";

    //--------------------------------------------------------------------------

    if (fVerbose) cout << "BmnGemStripHitMaker::Init() finished\n";

    return kSUCCESS;
}

void BmnSiliconHitMaker::Exec(Option_t* opt) {
    if (!IsActive())
        return;
    clock_t tStart = clock();

    fBmnSiliconHitsArray->Delete();

    if (fHitMatching && fBmnSiliconHitMatchesArray) {
        fBmnSiliconHitMatchesArray->Delete();
    }

    if (fVerbose) cout << " BmnSiliconHitMaker::Exec(), Number of BmnSiliconDigits = " << fBmnSiliconDigitsArray->GetEntriesFast() << "\n";

    ProcessDigits();

    if (fVerbose) cout << " BmnSiliconHitMaker::Exec() finished\n";
    clock_t tFinish = clock();
    workTime += ((Float_t) (tFinish - tStart)) / CLOCKS_PER_SEC;
}

void BmnSiliconHitMaker::ProcessDigits() {

    FairMCPoint* MCPoint;
    BmnSiliconDigit* digit;
    BmnMatch *strip_match;

    BmnSiliconStation* station;
    BmnSiliconModule* module;

    //Loading digits ---------------------------------------------------------------
    Int_t AddedDigits = 0;
    Int_t AddedStripDigitMatches = 0;

    for (UInt_t idigit = 0; idigit < fBmnSiliconDigitsArray->GetEntriesFast(); idigit++) {
        digit = (BmnSiliconDigit*) fBmnSiliconDigitsArray->At(idigit);
        station = StationSet->GetSiliconStation(digit->GetStation());
        module = station->GetModule(digit->GetModule());

        if (module->SetStripSignalInLayerByZoneId(digit->GetStripLayer(), digit->GetStripNumber(), digit->GetStripSignal())) AddedDigits++;

        if (fBmnSiliconDigitMatchesArray) {
            strip_match = (BmnMatch*) fBmnSiliconDigitMatchesArray->At(idigit);
            if (module->SetStripMatchInLayerByZoneId(digit->GetStripLayer(), digit->GetStripNumber(), *strip_match)) AddedStripDigitMatches++;
        }
    }

    if (fVerbose) cout << "   Processed strip digits  : " << AddedDigits << "\n";
    if (fVerbose && fBmnSiliconDigitMatchesArray) cout << "   Added strip digit matches  : " << AddedStripDigitMatches << "\n";
    //------------------------------------------------------------------------------

    //Processing digits
    StationSet->ProcessPointsInDetector();

    Int_t NCalculatedPoints = StationSet->CountNProcessedPointsInDetector();
    if (fVerbose) cout << "   Calculated points  : " << NCalculatedPoints << "\n";

    Int_t clear_matched_points_cnt = 0; // points with the only one match-indexes

    for (Int_t iStation = 0; iStation < StationSet->GetNStations(); ++iStation) {
        station = StationSet->GetSiliconStation(iStation);

        for (Int_t iModule = 0; iModule < station->GetNModules(); ++iModule) {
            module = station->GetModule(iModule);
            Double_t z = module->GetZPositionRegistered();

            Int_t NIntersectionPointsInModule = module->GetNIntersectionPoints();

            for (Int_t iPoint = 0; iPoint < NIntersectionPointsInModule; ++iPoint) {
                Double_t x = module->GetIntersectionPointX(iPoint);
                Double_t y = module->GetIntersectionPointY(iPoint);

                Double_t x_err = module->GetIntersectionPointXError(iPoint);
                Double_t y_err = module->GetIntersectionPointYError(iPoint);
                Double_t z_err = 0.0;

                Int_t RefMCIndex = 0;

                //hit matching (define RefMCIndex)) ----------------------------
                BmnMatch match = module->GetIntersectionPointMatch(iPoint);

                Int_t most_probably_index = -1;
                Double_t max_weight = 0;

                Int_t n_links = match.GetNofLinks();
                if (n_links == 1) clear_matched_points_cnt++;
                for (Int_t ilink = 0; ilink < n_links; ilink++) {
                    Int_t index = match.GetLink(ilink).GetIndex();
                    Double_t weight = match.GetLink(ilink).GetWeight();
                    if (weight > max_weight) {
                        max_weight = weight;
                        most_probably_index = index;
                    }
                }

                RefMCIndex = most_probably_index;
                //--------------------------------------------------------------

                //Add hit ------------------------------------------------------
                x *= -1; // invert to global X

                new ((*fBmnSiliconHitsArray)[fBmnSiliconHitsArray->GetEntriesFast()])
                        BmnSiliconHit(0, TVector3(x, y, z), TVector3(x_err, y_err, z_err), RefMCIndex);

                BmnSiliconHit* hit = (BmnSiliconHit*) fBmnSiliconHitsArray->At(fBmnSiliconHitsArray->GetEntriesFast() - 1);
                hit->SetStation(iStation);
                hit->SetModule(iModule);
                hit->SetIndex(fBmnSiliconHitsArray->GetEntriesFast() - 1);
                hit->SetClusterSizeInLowerLayer(module->GetIntersectionPoint_LowerLayerClusterSize(iPoint)); //cluster size (lower layer |||)
                hit->SetClusterSizeInUpperLayer(module->GetIntersectionPoint_UpperLayerClusterSize(iPoint)); //cluster size (upper layer ///or\\\)
                hit->SetStripPositionInLowerLayer(module->GetIntersectionPoint_LowerLayerSripPosition(iPoint)); //strip position (lower layer |||)
                hit->SetStripPositionInUpperLayer(module->GetIntersectionPoint_UpperLayerSripPosition(iPoint)); //strip position (upper layer ///or\\\)
                //--------------------------------------------------------------

                //hit matching -------------------------------------------------
                if (fHitMatching && fBmnSiliconHitMatchesArray) {
                    new ((*fBmnSiliconHitMatchesArray)[fBmnSiliconHitMatchesArray->GetEntriesFast()])
                            BmnMatch(module->GetIntersectionPointMatch(iPoint));
                }
                //--------------------------------------------------------------
            }
        }
    }
    if (fVerbose) cout << "   N clear matches with MC-points = " << clear_matched_points_cnt << "\n";
    //------------------------------------------------------------------------------
    StationSet->Reset();
}

void BmnSiliconHitMaker::Finish() {
    if (StationSet) {
        delete StationSet;
        StationSet = NULL;
    }
    cout << "Work time of the Silicon hit maker: " << workTime << endl;
}

ClassImp(BmnSiliconHitMaker)

