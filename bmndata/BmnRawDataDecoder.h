#include "TString.h"
#include "BmnEnums.h"
#include "BmnTTBDigit.h"
#include "BmnTDCDigit.h"
#include "BmnADCDigit.h"
#include "BmnADC32Digit.h"
#include "BmnSyncDigit.h"
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"
#include <iostream>
#include <vector>
#include <fstream>
#include "BmnGemRaw2Digit.h"
#include "BmnGemStripDigit.h"
#include "BmnDchRaw2Digit.h"
#include "BmnTof1Raw2Digit.h"
#include "BmnTrigRaw2Digit.h"
#include "BmnEventHeader.h"
#include <bitset>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <cstdio>
#include <list>
#include <map>
#include <deque>

// wait limit for input data
#define WAIT_LIMIT 3000000
using namespace std;

struct DigiArrays {
    TClonesArray *gem;
    TClonesArray *tof400;
    TClonesArray *tof700;
    TClonesArray *dch;
    TClonesArray *t0;
    TClonesArray *bc1;
    TClonesArray *bc2;
    TClonesArray *veto;
    //header array
    TClonesArray *header;
};

class BmnRawDataDecoder {
public:
    BmnRawDataDecoder(TString file, ULong_t nEvents = 0);
    BmnRawDataDecoder();
    virtual ~BmnRawDataDecoder();

    BmnStatus ConvertRawToRoot();
    BmnStatus ConvertRawToRootIterate();
    BmnStatus ConvertRawToRootIterateFile();
    BmnStatus DecodeDataToDigi();
    BmnStatus DecodeDataToDigiIterate();
    BmnStatus CalcGemPedestals();
    BmnStatus InitConverter();
    BmnStatus InitConverter(deque<UInt_t> *dq);
    BmnStatus DisposeConverter();
    BmnStatus InitDecoder();
    void ResetDecoder(TString file);
    BmnStatus DisposeDecoder();
    BmnStatus wait_stream(deque<UInt_t> *que, Int_t len);

    void SetQue(deque<UInt_t> *v) {
        fDataQueue = v;
    }

    deque<UInt_t> *GetQue() {
        return fDataQueue;
    }

    struct DigiArrays GetDigiArrays() {
        struct DigiArrays d;
        d.gem = gem;
        d.tof400 = tof400;
        d.tof700 = tof700;
        d.dch = dch;
        d.t0 = t0;
        d.bc1 = bc1;
        d.bc2 = bc2;
        d.veto = veto;
        d.header = header;
        return d;
    }

    TTree* GetDigiTree() {
        return fDigiTree;
    }

    void *GetQueMutex() {
        return fDataMutex;
    }

    UInt_t GetRunId() {
        return fRunId;
    }

    void SetQueMutex(void *v) {
        fDataMutex = v;
    }

    void SetRunId(UInt_t v) {
        fRunId = v;
    }

    map<UInt_t, Long64_t> GetTimeShifts() {
        return fTimeShifts;
    }

    UInt_t GetRunId() const {
        return fRunId;
    }

    UInt_t GetNevents() const {
        return fNevents;
    }

    void SetTrigMapping(TString map) {
        fTrigMapFileName = map;
    }

    void SetDchMapping(TString map) {
        fDchMapFileName = map;
    }

    void SetGemMapping(TString map) {
        fGemMapFileName = map;
    }

    void SetTof400Mapping(TString map) {
        fTof400MapFileName = map;
    }

    void SetTof700Mapping(TString map) {
        fTof700MapFileName = map;
    }

    void SetPedestalRun(Bool_t ped) {
        fPedestalRun = ped;
    }


private:

    UInt_t fRunId;
    UInt_t fEventId;
    UInt_t fNevents;
    Long64_t fTime_s;
    Long64_t fTime_ns;

    Long64_t fLengthRawFile;
    Long64_t fCurentPositionRawFile;

    TTree *fRawTree;
    TTree *fDigiTree;
    TString fRootFileName;
    TString fRawFileName;
    TString fDigiFileName;
    TString fDchMapFileName;
    TString fGemMapFileName;
    TString fTof400MapFileName;
    TString fTof700MapFileName;
    TString fTrigMapFileName;

    ifstream fDchMapFile;
    ifstream fGemMapFile;
    ifstream fTof400MapFile;
    ifstream fTof700MapFile;
    ifstream fTrigMapFile;

    TFile *fRootFileIn;
    TFile *fRootFileOut;
    TFile *fDigiFileOut;
    FILE *fRawFileIn;

    //DAQ arrays
    TClonesArray *sync;
    TClonesArray *adc;
    TClonesArray *tdc;
    TClonesArray *msc;
    TClonesArray *headerDAQ;
    TClonesArray *pedestalAdc;

    //Digi arrays
    TClonesArray *gem;
    TClonesArray *tof400;
    TClonesArray *tof700;
    TClonesArray *dch;
    TClonesArray *t0;
    TClonesArray *bc1;
    TClonesArray *bc2;
    TClonesArray *veto;
    //header array
    TClonesArray *header;

    UInt_t data[1000000];
    ULong_t fMaxEvent;

    UInt_t dat = 0;
    BmnGemRaw2Digit *gemMapper = NULL;
    BmnDchRaw2Digit *dchMapper = NULL;
    BmnTrigRaw2Digit *trigMapper = NULL;
    BmnTof1Raw2Digit *tof400Mapper = NULL;
    deque<UInt_t> *fDataQueue = NULL;
    void *fDataMutex; // actually std::mutex

    Bool_t fPedestalRun;

    //Map to store pairs <Crate serial> - <crate time - T0 time>
    map<UInt_t, Long64_t> fTimeShifts;
    Double_t fT0Time; //ns

    BmnStatus ProcessEvent(UInt_t *data, UInt_t len);
    BmnStatus Process_ADC64VE(UInt_t *data, UInt_t len, UInt_t serial);
    BmnStatus Process_FVME(UInt_t *data, UInt_t len, UInt_t serial, BmnEventType &ped);
    BmnStatus FillTDC(UInt_t *d, UInt_t serial, UInt_t slot, UInt_t modId, UInt_t &idx);
    BmnStatus FillSYNC(UInt_t *d, UInt_t serial, UInt_t &idx);
    BmnStatus FillMSC(UInt_t *d, UInt_t serial, UInt_t &idx);
    BmnStatus FillTimeShiftsMap();
};
