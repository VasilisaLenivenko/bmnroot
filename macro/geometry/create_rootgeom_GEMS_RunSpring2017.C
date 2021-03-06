//---------------------------
#include "TGeoManager.h"
#include "TFile.h"
#include "TString.h"
#include "TMath.h"
#include "TGeoShape.h"
#include "TGeoBBox.h"
#include "../../gem/BmnGemStripConfiguration.h"

using namespace TMath;

//GeoManager
TGeoManager* gGeoMan = NULL;

//Set Parameters of GEMS
const Int_t nStations = BmnGemStationPositions_RunSpring2017::NStations;      //stations in the detector
const Int_t nMaxModules = BmnGemStationPositions_RunSpring2017::NMaxModules;    //max. number of modules in a station

//Positions of stations in the detector
Double_t XStationPositions[nStations];
Double_t YStationPositions[nStations];
Double_t ZStationPositions[nStations];

//Shifts of modules in the stations
Double_t XModuleShifts[nStations][nMaxModules];
Double_t YModuleShifts[nStations][nMaxModules];
Double_t ZModuleShifts[nStations][nMaxModules];

//GEM plane sizes (66x41 type) -------------------------------------------------
const Double_t XModuleSize_Station66x41 = 66.0;
const Double_t YModuleSize_Station66x41 = 41.0;

const Double_t XHotZoneSize_Station66x41 = 15.0;
const Double_t YHotZoneSize_Station66x41 = 10.0;

    //Module Parameters (66x41 type)
    const Double_t dXOuterFrame_Station66x41 = 5.0;
    const Double_t dXInnerFrame_Station66x41 = 1.7;
    const Double_t dYOuterFrame_Station66x41 = 3.75;
    const Double_t dYInnerFrame_Station66x41 = 1.7;
    const Double_t dZSensitiveVolume_Station66x41 = 0.9;
    const Double_t ZModuleSize_Station66x41 = 2.3;

//------------------------------------------------------------------------------

//GEM plane sizes (163x45 type) ------------------------------------------------
const Double_t OuterZoneSizes_Station163x45[2] = {163.2*0.5, 45.0}; // {xsize, ysize}
const Double_t HotZoneSizes_rectangle_Station163x45[2] = {40.0, 15.0}; // {xsize, ysize}
const Double_t HotZoneSizes_slope_Station163x45[3] = {37.99, 42.01, 15.0}; // {xmin_size, xmax_size, ysize}
const Double_t HoleRadius_Station163x45 = 4.0; //cm (real hole without the circle frame)

    //Module Parameters (163x45 type)
    const Double_t dXOuterFrame_Station163x45 = 5.0;
    const Double_t dXInnerFrame_Station163x45 = 1.7;
    const Double_t dYOuterFrame_Station163x45 = 3.75;
    const Double_t dYInnerFrame_Station163x45 = 1.7;
    const Double_t dZSensitiveVolume_Station163x45 = 0.9;
    const Double_t ZModuleSize_Station163x45 = 3.9;

//------------------------------------------------------------------------------

//media
TGeoMedium *pMedAir = 0;
TGeoMedium *pMedCarbon = 0;
TGeoMedium *pMedFiberGlass = 0;
TGeoMedium *pMedCopper = 0;
TGeoMedium *pMedArCO27030 = 0;

class FairGeoMedia;
class FairGeoBuilder;

void SetStationPositionsAndModuleShifts() {

    cout << "Positions of stations and shifts of modules (cm):\n";

    for(Int_t iStation = 0; iStation < nStations; ++iStation) {
        XStationPositions[iStation] = BmnGemStationPositions_RunSpring2017::XStationPositions[iStation];
        YStationPositions[iStation] = BmnGemStationPositions_RunSpring2017::YStationPositions[iStation];
        ZStationPositions[iStation] = BmnGemStationPositions_RunSpring2017::ZStationPositions[iStation];

        cout << "XStationPosition[" << iStation << "] = " << XStationPositions[iStation] << "\n";
        cout << "YStationPosition[" << iStation << "] = " << YStationPositions[iStation] << "\n";
        cout << "ZStationPosition[" << iStation << "] = " << ZStationPositions[iStation] << "\n";

        for(Int_t iModule = 0; iModule < nMaxModules; ++iModule) {
            XModuleShifts[iStation][iModule] = BmnGemStationPositions_RunSpring2017::XModuleShifts[iStation][iModule];
            YModuleShifts[iStation][iModule] = BmnGemStationPositions_RunSpring2017::YModuleShifts[iStation][iModule];
            ZModuleShifts[iStation][iModule] = BmnGemStationPositions_RunSpring2017::ZModuleShifts[iStation][iModule];

            cout << "   XModuleShifts[" << iStation << "][" << iModule << "] = " << XModuleShifts[iStation][iModule] << "\n";
            cout << "   YModuleShifts[" << iStation << "][" << iModule << "] = " << YModuleShifts[iStation][iModule] << "\n";
            cout << "   ZModuleShifts[" << iStation << "][" << iModule << "] = " << ZModuleShifts[iStation][iModule] << "\n";
        }
    }
    cout << "\n";
}

void DefineRequiredMedia(FairGeoMedia* geoMedia, FairGeoBuilder* geoBuild) {

    //air medium
    FairGeoMedium* mAir = geoMedia->getMedium("air");
    if ( ! mAir ) Fatal("Main", "FairMedium air not found");
    geoBuild->createMedium(mAir);
    pMedAir = gGeoManager->GetMedium("air");
    if ( ! pMedAir ) Fatal("Main", "Medium air not found");

    //carbon medium
    FairGeoMedium* mCarbon = geoMedia->getMedium("carbon");
    if ( ! mCarbon  ) Fatal("Main", "FairMedium carbon not found");
    geoBuild->createMedium(mCarbon);
    pMedCarbon = gGeoManager->GetMedium("carbon");
    if ( ! pMedCarbon ) Fatal("Main", "Medium carbon not found");

    //fiberglass medium
    FairGeoMedium* mFiberGlass = geoMedia->getMedium("fiberglass");
    if ( ! mFiberGlass  ) Fatal("Main", "FairMedium fiberglass not found");
    geoBuild->createMedium(mFiberGlass);
    pMedFiberGlass = gGeoManager->GetMedium("fiberglass");
    if ( ! pMedFiberGlass  ) Fatal("Main", "Medium fiberglass not found");

    //arco27020 medium
    FairGeoMedium* mArCO27030 = geoMedia->getMedium("arco27030");
    if ( ! mArCO27030  ) Fatal("Main", "FairMedium arco27030 not found");
    geoBuild->createMedium(mArCO27030);
    pMedArCO27030= gGeoManager->GetMedium("arco27030");
    if ( ! pMedArCO27030  ) Fatal("Main", "Medium arco27030 not found");
}

void create_rootgeom_GEMS_RunSpring2017() {

    // Load the necessary FairRoot libraries
    gROOT->LoadMacro("$VMCWORKDIR/macro/run/bmnloadlibs.C");
    bmnloadlibs(); // load libraries

    // ----  set working directory  --------------------------------------------
    TString gPath = gSystem->Getenv("VMCWORKDIR");

    // -------   Geometry file name (output)   ----------------------------------
    const TString geoDetectorName = "GEMS";
    const TString geoDetectorVersion = "RunSpring2017";
    const TString geoFileName = gPath + "/geometry/" + geoDetectorName + "_"+ geoDetectorVersion + ".root";

    // ----  global geometry parameters  ---------------------------------------
    FairGeoLoader*    geoLoad = new FairGeoLoader("TGeo","FairGeoLoader");
    FairGeoInterface* geoFace = geoLoad->getGeoInterface();

    // -------   Load media from media file   ----------------------------------
    TString medFile = gPath + "/geometry/media.geo";
    geoFace->setMediaFile(medFile);
    geoFace->readMedia();
    FairGeoMedia*   geoMedia = geoFace->getMedia();
    FairGeoBuilder* geoBuild = geoLoad->getGeoBuilder();

    DefineRequiredMedia(geoMedia, geoBuild);
    // -------------------------------------------------------------------------

    // --------------   Create geometry and top volume  -------------------------
    gGeoMan = (TGeoManager*)gROOT->FindObject("FAIRGeom");
    gGeoMan->SetName(geoDetectorName + "_geom");
    TGeoVolume* top = new TGeoVolumeAssembly("TOP");
    top->SetMedium(pMedAir);
    gGeoMan->SetTopVolume(top);
    //gGeoMan->SetTopVisible(1);
    // --------------------------------------------------------------------------

    // Define TOP Geometry
    TGeoVolume* GEMS = new TGeoVolumeAssembly(geoDetectorName);
    GEMS->SetMedium(pMedAir);

    //Set positions of GEM stations
    SetStationPositionsAndModuleShifts();

    //station composing
    CreateStation_One66x41Plane(GEMS, "station0",
                                XStationPositions[0], YStationPositions[0], ZStationPositions[0],
                                XModuleShifts[0][0], YModuleShifts[0][0], ZModuleShifts[0][0],
                                kFALSE);

    CreateStation_One66x41Plane(GEMS, "station1",
                                XStationPositions[1], YStationPositions[1], ZStationPositions[1],
                                XModuleShifts[1][0], YModuleShifts[1][0], ZModuleShifts[1][0],
                                kTRUE);

    CreateStation_One66x41Plane(GEMS, "station2",
                                XStationPositions[2], YStationPositions[2], ZStationPositions[2],
                                XModuleShifts[2][0], YModuleShifts[2][0], ZModuleShifts[2][0],
                                kFALSE);

    CreateStation_Two66x41Plane(GEMS, "station3",
                                XStationPositions[3], YStationPositions[3], ZStationPositions[3],
                                XModuleShifts[3][0], YModuleShifts[3][0], ZModuleShifts[3][],
                                XModuleShifts[3][1], YModuleShifts[3][1], ZModuleShifts[3][1]);

    CreateStation_One163x45Plane(GEMS, "station4",
                                 XStationPositions[4], YStationPositions[4], ZStationPositions[4],
                                 XModuleShifts[4][0], YModuleShifts[4][0], ZModuleShifts[4][0],
                                 XModuleShifts[4][1], YModuleShifts[4][1], ZModuleShifts[4][1],
                                 HoleRadius_Station163x45+dXInnerFrame_Station163x45, kFALSE);

    CreateStation_One163x45Plane(GEMS, "station5",
                                 XStationPositions[5], YStationPositions[5], ZStationPositions[5],
                                 XModuleShifts[5][0], YModuleShifts[5][0], ZModuleShifts[5][0],
                                 XModuleShifts[5][1], YModuleShifts[5][1], ZModuleShifts[5][1],
                                 HoleRadius_Station163x45+dXInnerFrame_Station163x45, kTRUE);


    top->AddNode(GEMS, 0);
    top->SetVisContainers(kTRUE);

    // ---------------   Finish   -----------------------------------------------
    gGeoMan->CloseGeometry();
    gGeoMan->CheckOverlaps(0.0001);
    gGeoMan->PrintOverlaps();
    gGeoMan->Test();

    TFile* geoFile = new TFile(geoFileName, "RECREATE");
    top->Write();
    geoFile->Close();
    top->Draw("ogl");
}
//------------------------------------------------------------------------------

void CreateStation_One66x41Plane(TGeoVolume* mother_volume, TString station_name,
                                 Double_t x_position, Double_t y_position, Double_t z_position,
                                 Double_t x_shift_mod0, Double_t y_shift_mod0, Double_t z_shift_mod0,
                                 Bool_t IsInverted) {
    TGeoVolume *stationA = new TGeoVolumeAssembly(station_name);
    stationA->SetMedium(pMedAir);

    //sensitive shapes
    TGeoShape *moduleS = new TGeoBBox("moduleS", XModuleSize_Station66x41*0.5, YModuleSize_Station66x41*0.5, dZSensitiveVolume_Station66x41*0.5);
    TGeoShape *hot_zoneS = new TGeoBBox("hot_zoneS", XHotZoneSize_Station66x41*0.5, YHotZoneSize_Station66x41*0.5, dZSensitiveVolume_Station66x41*0.5);

    //frame shapes
    TGeoShape *vertical_outer_frameS = new TGeoBBox("vertical_outer_frameS", dXOuterFrame_Station66x41*0.5, (YModuleSize_Station66x41+dYOuterFrame_Station66x41)*0.5, ZModuleSize_Station66x41*0.5);
    TGeoShape *horizontal_outer_frameS = new TGeoBBox("horizontal_outer_frameS", XModuleSize_Station66x41*0.5, dYOuterFrame_Station66x41*0.5, ZModuleSize_Station66x41*0.5);
    TGeoShape *vertical_inner_frameS = new TGeoBBox("vertical_inner_frameS", dXInnerFrame_Station66x41*0.5, (YModuleSize_Station66x41+dYOuterFrame_Station66x41)*0.5, ZModuleSize_Station66x41*0.5);
    TGeoShape *horizontal_inner_frameS = new TGeoBBox("horizontal_inner_frameS", (XModuleSize_Station66x41+dXInnerFrame_Station66x41+dXOuterFrame_Station66x41)*0.5, dYInnerFrame_Station66x41*0.5, ZModuleSize_Station66x41*0.5);

    //sensitive volumes
    TGeoVolume *moduleV = new TGeoVolume("module_Sensor", moduleS);
    TGeoVolume *hot_zoneV = new TGeoVolume("hot_zone_Sensor", hot_zoneS);
    moduleV->SetLineColor(TColor::GetColor("#ff47ca"));
    hot_zoneV->SetLineColor(TColor::GetColor("#e0b0ff"));

    //frame volumes
    TGeoVolume *vertical_outer_frameV = new TGeoVolume("vertical_outer_frame", vertical_outer_frameS);
    TGeoVolume *horizontal_outer_frameV = new TGeoVolume("horizontal_outer_frame", horizontal_outer_frameS);
    TGeoVolume *vertical_inner_frameV = new TGeoVolume("vertical_inner_frame", vertical_inner_frameS);
    TGeoVolume *horizontal_inner_frameV = new TGeoVolume("horizontal_inner_frame", horizontal_inner_frameS);
    TGeoVolume *composite_frameV = new TGeoVolumeAssembly("frame");
    vertical_outer_frameV->SetLineColor(TColor::GetColor("#9999ff"));
    horizontal_outer_frameV->SetLineColor(TColor::GetColor("#9999ff"));
    vertical_inner_frameV->SetLineColor(TColor::GetColor("#9999ff"));
    horizontal_inner_frameV->SetLineColor(TColor::GetColor("#9999ff"));

    //media for sensitive volumes
    TGeoMedium *sensitive_medium = pMedArCO27030; //set medium
    if(sensitive_medium) {
        moduleV->SetMedium(sensitive_medium);
        hot_zoneV->SetMedium(sensitive_medium);
    }
    else Fatal("Main", "Invalid medium for sensitive modules!");

    //media for frames
    TGeoMedium *frame_medium = pMedCarbon; //set medium
    if(frame_medium) {
        vertical_outer_frameV->SetMedium(frame_medium);
        horizontal_outer_frameV->SetMedium(frame_medium);
        vertical_inner_frameV->SetMedium(frame_medium);
        horizontal_inner_frameV->SetMedium(frame_medium);
        composite_frameV->SetMedium(frame_medium);
    }
    else Fatal("Main", "Invalid medium for frames!");

    moduleV->SetTransparency(20);

    //hot zone position in the module
    TGeoTranslation *hot_zone_trans = new TGeoTranslation(XModuleSize_Station66x41*0.5 - XHotZoneSize_Station66x41*0.5,
                                                          -YModuleSize_Station66x41*0.5 + YHotZoneSize_Station66x41*0.5,
                                                          0.0);

    //module positions in the station
    TGeoCombiTrans *module_position = new TGeoCombiTrans();
    if(IsInverted) {
        module_position->ReflectY(true);
    }
    module_position->SetTranslation(x_shift_mod0, y_shift_mod0, z_shift_mod0);

    //frame part positions in the frame
    TGeoCombiTrans *right_outer_frame_position = new TGeoCombiTrans();
    right_outer_frame_position->SetTranslation(-(XModuleSize_Station66x41+dXOuterFrame_Station66x41)*0.5,
                                               (dYOuterFrame_Station66x41)*0.5,
                                               0.0);

    TGeoCombiTrans *top_outer_frame_position = new TGeoCombiTrans();
    top_outer_frame_position->SetTranslation(0.0,
                                             YModuleSize_Station66x41*0.5+dYOuterFrame_Station66x41*0.5,
                                             0.0);

    TGeoCombiTrans *left_inner_frame_position = new TGeoCombiTrans();
    left_inner_frame_position->SetTranslation(XModuleSize_Station66x41*0.5+dXInnerFrame_Station66x41*0.5,
                                              dYOuterFrame_Station66x41*0.5,
                                              0.0);

    TGeoCombiTrans *bottom_inner_frame_position = new TGeoCombiTrans();
    bottom_inner_frame_position->SetTranslation((dXInnerFrame_Station66x41+dXOuterFrame_Station66x41)*0.5 - dXOuterFrame_Station66x41,
                                                -YModuleSize_Station66x41*0.5-dYInnerFrame_Station66x41*0.5,
                                                0.0);

    //frame positions in the station
    TGeoCombiTrans *comp_frame_center_position = new TGeoCombiTrans();
    if(IsInverted) {
        comp_frame_center_position->ReflectY(true);
    }
    comp_frame_center_position->SetTranslation(module_position->GetTranslation());

    //station assembling
    moduleV->AddNode(hot_zoneV, 0, hot_zone_trans);

    composite_frameV->AddNode(vertical_outer_frameV, 0, right_outer_frame_position);
    composite_frameV->AddNode(horizontal_outer_frameV, 0, top_outer_frame_position);
    //composite_frameV->AddNode(vertical_inner_frameV, 0, left_inner_frame_position);
    //composite_frameV->AddNode(horizontal_inner_frameV, 0, bottom_inner_frame_position);

    stationA->AddNode(moduleV, 0, module_position);

    stationA->AddNode(composite_frameV, 0, comp_frame_center_position);

    //Station position
    Double_t station_shift = dZSensitiveVolume_Station66x41*0.5;
    TGeoTranslation *station_pos = new TGeoTranslation(x_position, y_position, z_position+station_shift);

    mother_volume->AddNode(stationA, 0, station_pos);
}

void CreateStation_Two66x41Plane(TGeoVolume* mother_volume, TString station_name,
                                 Double_t x_position, Double_t y_position, Double_t z_position,
                                 Double_t x_shift_mod0, Double_t y_shift_mod0, Double_t z_shift_mod0,
                                 Double_t x_shift_mod1, Double_t y_shift_mod1, Double_t z_shift_mod1) {

    TGeoVolume *stationA = new TGeoVolumeAssembly(station_name);
    stationA->SetMedium(pMedAir);

    //sensitive shapes
    TGeoShape *moduleS = new TGeoBBox("moduleS", XModuleSize_Station66x41*0.5, YModuleSize_Station66x41*0.5, dZSensitiveVolume_Station66x41*0.5);
    TGeoShape *hot_zoneS = new TGeoBBox("hot_zoneS", XHotZoneSize_Station66x41*0.5, YHotZoneSize_Station66x41*0.5, dZSensitiveVolume_Station66x41*0.5);

    //frame shapes
    TGeoShape *vertical_outer_frameS = new TGeoBBox("vertical_outer_frameS", dXOuterFrame_Station66x41*0.5, (YModuleSize_Station66x41+dYOuterFrame_Station66x41)*0.5, ZModuleSize_Station66x41*0.5);
    TGeoShape *horizontal_outer_frameS = new TGeoBBox("horizontal_outer_frameS", XModuleSize_Station66x41*0.5, dYOuterFrame_Station66x41*0.5, ZModuleSize_Station66x41*0.5);
    TGeoShape *vertical_inner_frameS = new TGeoBBox("vertical_inner_frameS", dXInnerFrame_Station66x41*0.5, (YModuleSize_Station66x41+dYOuterFrame_Station66x41)*0.5, ZModuleSize_Station66x41*0.5);
    TGeoShape *horizontal_inner_frameS = new TGeoBBox("horizontal_inner_frameS", (XModuleSize_Station66x41+dXInnerFrame_Station66x41+dXOuterFrame_Station66x41)*0.5, dYInnerFrame_Station66x41*0.5, ZModuleSize_Station66x41*0.5);

    //sensitive volumes
    TGeoVolume *moduleV = new TGeoVolume("module_Sensor", moduleS);
    TGeoVolume *hot_zoneV = new TGeoVolume("hot_zone_Sensor", hot_zoneS);
    moduleV->SetLineColor(TColor::GetColor("#ff47ca"));
    hot_zoneV->SetLineColor(TColor::GetColor("#e0b0ff"));

    //frame volumes
    TGeoVolume *vertical_outer_frameV = new TGeoVolume("vertical_outer_frame", vertical_outer_frameS);
    TGeoVolume *horizontal_outer_frameV = new TGeoVolume("horizontal_outer_frame", horizontal_outer_frameS);
    TGeoVolume *vertical_inner_frameV = new TGeoVolume("vertical_inner_frame", vertical_inner_frameS);
    TGeoVolume *horizontal_inner_frameV = new TGeoVolume("horizontal_inner_frame", horizontal_inner_frameS);
    TGeoVolume *composite_frameV = new TGeoVolumeAssembly("frame");
    vertical_outer_frameV->SetLineColor(TColor::GetColor("#9999ff"));
    horizontal_outer_frameV->SetLineColor(TColor::GetColor("#9999ff"));
    vertical_inner_frameV->SetLineColor(TColor::GetColor("#9999ff"));
    horizontal_inner_frameV->SetLineColor(TColor::GetColor("#9999ff"));

    //media for sensitive volumes
    TGeoMedium *sensitive_medium = pMedArCO27030; //set medium
    if(sensitive_medium) {
        moduleV->SetMedium(sensitive_medium);
        hot_zoneV->SetMedium(sensitive_medium);
    }
    else Fatal("Main", "Invalid medium for sensitive modules!");

    //media for frames
    TGeoMedium *frame_medium = pMedCarbon; //set medium
    if(frame_medium) {
        vertical_outer_frameV->SetMedium(frame_medium);
        horizontal_outer_frameV->SetMedium(frame_medium);
        vertical_inner_frameV->SetMedium(frame_medium);
        horizontal_inner_frameV->SetMedium(frame_medium);
        composite_frameV->SetMedium(frame_medium);
    }
    else Fatal("Main", "Invalid medium for frames!");

    moduleV->SetTransparency(20);

    //hot zone position in the module
    TGeoTranslation *hot_zone_trans = new TGeoTranslation(XModuleSize_Station66x41*0.5 - XHotZoneSize_Station66x41*0.5,
                                                          -YModuleSize_Station66x41*0.5 + YHotZoneSize_Station66x41*0.5,
                                                          0.0);

    //module positions in the station
    TGeoCombiTrans *module_position_left = new TGeoCombiTrans();
    module_position_left->ReflectX(true);
    module_position_left->SetTranslation(XModuleSize_Station66x41*0.5 + x_shift_mod0, 0.0 + y_shift_mod0, 0.0 + z_shift_mod0);

    TGeoCombiTrans *module_position_right = new TGeoCombiTrans();
    module_position_right->ReflectY(true);
    module_position_right->SetTranslation(-XModuleSize_Station66x41*0.5 + x_shift_mod1, 0.0 + y_shift_mod1, 0.0 + z_shift_mod1);

    //frame part positions in the frame
    TGeoCombiTrans *right_outer_frame_position = new TGeoCombiTrans();
    right_outer_frame_position->SetTranslation(-(XModuleSize_Station66x41+dXOuterFrame_Station66x41)*0.5,
                                               (dYOuterFrame_Station66x41)*0.5,
                                               0.0);

    TGeoCombiTrans *top_outer_frame_position = new TGeoCombiTrans();
    top_outer_frame_position->SetTranslation(0.0,
                                             YModuleSize_Station66x41*0.5+dYOuterFrame_Station66x41*0.5,
                                             0.0);

    TGeoCombiTrans *left_inner_frame_position = new TGeoCombiTrans();
    left_inner_frame_position->SetTranslation(XModuleSize_Station66x41*0.5+dXInnerFrame_Station66x41*0.5,
                                              dYOuterFrame_Station66x41*0.5,
                                              0.0);

    TGeoCombiTrans *bottom_inner_frame_position = new TGeoCombiTrans();
    bottom_inner_frame_position->SetTranslation((dXInnerFrame_Station66x41+dXOuterFrame_Station66x41)*0.5 - dXOuterFrame_Station66x41,
                                                -YModuleSize_Station66x41*0.5-dYInnerFrame_Station66x41*0.5,
                                                0.0);

    //frame positions in the station
    TGeoCombiTrans *comp_frame_position_left = new TGeoCombiTrans();
    comp_frame_position_left->ReflectX(true);
    comp_frame_position_left->SetTranslation(module_position_left->GetTranslation());


    TGeoCombiTrans *comp_frame_position_right = new TGeoCombiTrans();
    comp_frame_position_right->ReflectY(true);
    comp_frame_position_right->SetTranslation(module_position_right->GetTranslation());

    //station assembling
    moduleV->AddNode(hot_zoneV, 0, hot_zone_trans);

    composite_frameV->AddNode(vertical_outer_frameV, 0, right_outer_frame_position);
    composite_frameV->AddNode(horizontal_outer_frameV, 0, top_outer_frame_position);
    //composite_frameV->AddNode(vertical_inner_frameV, 0, left_inner_frame_position);
    //composite_frameV->AddNode(horizontal_inner_frameV, 0, bottom_inner_frame_position);

    stationA->AddNode(moduleV, 0, module_position_left);
    stationA->AddNode(moduleV, 1, module_position_right);

    stationA->AddNode(composite_frameV, 0, comp_frame_position_left);
    stationA->AddNode(composite_frameV, 1, comp_frame_position_right);

    //Station position
    Double_t station_shift = dZSensitiveVolume_Station66x41*0.5;
    //station_shift = 0.0;
    TGeoTranslation *station_pos = new TGeoTranslation(x_position, y_position, z_position+station_shift);

    mother_volume->AddNode(stationA, 0, station_pos);
}

void CreateStation_One163x45Plane(TGeoVolume* mother_volume, TString station_name,
                                  Double_t x_position, Double_t y_position, Double_t z_position,
                                  Double_t x_shift_mod0, Double_t y_shift_mod0, Double_t z_shift_mod0,
                                  Double_t x_shift_mod1, Double_t y_shift_mod1, Double_t z_shift_mod1,
                                  Double_t hole_radius_with_frame, Bool_t IsInverted) {

    TGeoVolume *stationA = new TGeoVolumeAssembly(station_name);
    stationA->SetMedium(pMedAir);

    Double_t XOuterZoneSize = OuterZoneSizes_Station163x45[0];
    Double_t YOuterZoneSize = OuterZoneSizes_Station163x45[1];

    Double_t XRectangleZoneSize = HotZoneSizes_rectangle_Station163x45[0];
    Double_t YRectangleZoneSize = HotZoneSizes_rectangle_Station163x45[1];

    Double_t XMinSlopeZoneSize = HotZoneSizes_slope_Station163x45[0];
    Double_t XMaxSlopeZoneSize = HotZoneSizes_slope_Station163x45[1];
    Double_t YSlopeZoneSize = HotZoneSizes_slope_Station163x45[2];

    //module shape
    TString part_module_name = "part_moduleS_" + station_name;
    TGeoShape *part_moduleS = new TGeoBBox(part_module_name, XOuterZoneSize*0.5, YOuterZoneSize*0.5, dZSensitiveVolume_Station163x45*0.5);

    //part hot zone shapes (without the hole)
    TString part_rectangle_inner_zone_name = "part_rectangle_inner_zoneS_" + station_name;
    TGeoShape *part_rectangle_inner_zoneS = new TGeoBBox(part_rectangle_inner_zone_name, XRectangleZoneSize*0.5, YRectangleZoneSize*0.5, dZSensitiveVolume_Station163x45*0.5);

    TString part_slope_inner_zone_name = "part_slope_inner_zoneS_" + station_name;
    TGeoXtru *part_slope_inner_zoneS = new TGeoXtru(2); //origin centered
    part_slope_inner_zoneS->SetName(part_slope_inner_zone_name);
        Double_t x_pos[4] = {XMaxSlopeZoneSize*0.5, XMaxSlopeZoneSize*0.5 - (XMaxSlopeZoneSize-XMinSlopeZoneSize), -XMaxSlopeZoneSize*0.5, -XMaxSlopeZoneSize*0.5};
        Double_t y_pos[4] = {-YSlopeZoneSize*0.5, YSlopeZoneSize*0.5, YSlopeZoneSize*0.5, -YSlopeZoneSize*0.5};
    part_slope_inner_zoneS->DefinePolygon(4, x_pos, y_pos);
    part_slope_inner_zoneS->DefineSection(0, -dZSensitiveVolume_Station163x45*0.5);
    part_slope_inner_zoneS->DefineSection(1, dZSensitiveVolume_Station163x45*0.5);

    //hole shape
    TString hole_name = "holeS_" + station_name;
    //TGeoShape *holeS = new TGeoTubeSeg(hole_name, 0.0, hole_radius_with_frame, dZSensitiveVolume_Station163x45*0.5 + 0.1, -1.0, 91.0);
    TGeoShape *holeS = new TGeoTube(hole_name, 0.0, hole_radius_with_frame, dZSensitiveVolume_Station163x45*0.5+0.01);

    //hole positions in the module and zones to extract the hole
    TString hole_module_trans_name = "hole_module_trans_name_" + station_name;
    TGeoTranslation *hole_module_trans = new TGeoTranslation(hole_module_trans_name, -XOuterZoneSize*0.5, -YOuterZoneSize*0.5, 0);
    hole_module_trans->RegisterYourself();

    TString hole_rectangle_inner_zone_trans_name = "hole_rectangle_inner_zone_trans_name_" + station_name;
    TGeoTranslation *hole_rectangle_inner_zone_trans = new TGeoTranslation(hole_rectangle_inner_zone_trans_name, -XRectangleZoneSize*0.5, -YRectangleZoneSize*0.5, 0);
    hole_rectangle_inner_zone_trans->RegisterYourself();

    TString hole_slope_inner_zone_trans_name = "hole_slope_inner_zone_trans_name_" + station_name;
    TGeoTranslation *hole_slope_inner_zone_trans = new TGeoTranslation(hole_slope_inner_zone_trans_name, -XMaxSlopeZoneSize*0.5, -YSlopeZoneSize*0.5, 0);
    hole_slope_inner_zone_trans->RegisterYourself();

    //module and hot zone shapes
    TString bool_expression_module = part_module_name + " - " + hole_name + ":" + hole_module_trans_name;
    TGeoCompositeShape *moduleS = new TGeoCompositeShape(bool_expression_module);

    TString bool_expression_rectangle_inner_zone = part_rectangle_inner_zone_name + " - " + hole_name + ":" + hole_rectangle_inner_zone_trans_name;
    TGeoCompositeShape *rectangle_inner_zoneS = new TGeoCompositeShape(bool_expression_rectangle_inner_zone);

    TString bool_expression_slope_inner_zone = part_slope_inner_zone_name + " - " + hole_name + ":" + hole_slope_inner_zone_trans_name;
    TGeoCompositeShape *slope_inner_zoneS = new TGeoCompositeShape(bool_expression_slope_inner_zone);

    //frame shapes
    TGeoShape *vertical_frameS = new TGeoBBox("vertical_frameS", dXOuterFrame_Station163x45*0.5, (YOuterZoneSize+dYInnerFrame_Station163x45+dYOuterFrame_Station163x45)*0.5, ZModuleSize_Station163x45*0.5);
    TGeoShape *horizontal_long_frameS = new TGeoBBox("horizontal_long_frameS", XOuterZoneSize, dYOuterFrame_Station163x45*0.5, ZModuleSize_Station163x45*0.5);
    TGeoShape *horizontal_short_frameS = new TGeoBBox("horizontal_short_frameS", (XOuterZoneSize-hole_radius_with_frame+dXInnerFrame_Station163x45)*0.5, dYInnerFrame_Station163x45*0.5, ZModuleSize_Station163x45*0.5);
    TGeoShape *circle_inner_frameS = new TGeoTubeSeg("circle_inner_frameS", hole_radius_with_frame-dXInnerFrame_Station163x45, hole_radius_with_frame, ZModuleSize_Station163x45*0.5, 0., -180.0);

    //Volumes
    TGeoVolume *module_with_slope_inner_zoneV = new TGeoVolume("module_Sensor", moduleS);
    TGeoVolume *module_with_rect_inner_zoneV = new TGeoVolume("module_Sensor", moduleS);

    TGeoVolume *rectangle_inner_zoneV = new TGeoVolume("inner_zone_Sensor", rectangle_inner_zoneS);
    TGeoVolume *slope_inner_zoneV = new TGeoVolume("inner_zone_Sensor", slope_inner_zoneS);

    TGeoVolume *vertical_frameV = new TGeoVolume("vertical_frame", vertical_frameS);
    TGeoVolume *horizontal_long_frameV = new TGeoVolume("horizontal_long_frame", horizontal_long_frameS);
    TGeoVolume *horizontal_short_frameV = new TGeoVolume("horizontal_short_frame", horizontal_short_frameS);
    TGeoVolume *circle_inner_frameV = new TGeoVolume("circle_inner_frame", circle_inner_frameS);
    TGeoVolume *composite_frameV = new TGeoVolumeAssembly("frame");

    //media for sensitive volumes
    TGeoMedium *sensitive_medium = pMedArCO27030; //set medium
    if(sensitive_medium) {
        module_with_slope_inner_zoneV->SetMedium(sensitive_medium);
        module_with_rect_inner_zoneV->SetMedium(sensitive_medium);
        rectangle_inner_zoneV->SetMedium(sensitive_medium);
        slope_inner_zoneV->SetMedium(sensitive_medium);
    }
    else Fatal("Main", "Invalid medium for sensitive modules!");

    //media for frames
    TGeoMedium *frame_medium = pMedCarbon; //set medium
    if(frame_medium) {
        vertical_frameV->SetMedium(frame_medium);
        horizontal_long_frameV->SetMedium(frame_medium);
        horizontal_short_frameV->SetMedium(frame_medium);
        circle_inner_frameV->SetMedium(frame_medium);
        composite_frameV->SetMedium(frame_medium);
    }
    else Fatal("Main", "Invalid medium for frames!");

    //volume visual properties
    module_with_slope_inner_zoneV->SetTransparency(20);
    module_with_rect_inner_zoneV->SetTransparency(20);
    module_with_slope_inner_zoneV->SetLineColor(TColor::GetColor("#ff47ca"));
    module_with_rect_inner_zoneV->SetLineColor(TColor::GetColor("#ff47ca"));
    rectangle_inner_zoneV->SetLineColor(TColor::GetColor("#e0b0ff"));
    slope_inner_zoneV->SetLineColor(TColor::GetColor("#e0b0ff"));
    vertical_frameV->SetLineColor(TColor::GetColor("#9999ff"));
    horizontal_long_frameV->SetLineColor(TColor::GetColor("#9999ff"));
    horizontal_short_frameV->SetLineColor(TColor::GetColor("#9999ff"));
    circle_inner_frameV->SetLineColor(TColor::GetColor("#9999ff"));

    //hot zone positions in modules
    TGeoTranslation *slope_inner_zone_position = new TGeoTranslation(-XOuterZoneSize*0.5+XMaxSlopeZoneSize*0.5, -YOuterZoneSize*0.5 + YSlopeZoneSize*0.5, 0);
    TGeoTranslation *rect_inner_zone_position = new TGeoTranslation(-XOuterZoneSize*0.5+XRectangleZoneSize*0.5, -YOuterZoneSize*0.5 + YRectangleZoneSize*0.5, 0);

    //module positions in the station
    TGeoCombiTrans *left_module_position = new TGeoCombiTrans();
    left_module_position->SetTranslation(XOuterZoneSize*0.5 + x_shift_mod0, 0.0 + y_shift_mod0, 0.0 + z_shift_mod0);

    TGeoCombiTrans *right_module_position = new TGeoCombiTrans();
    right_module_position->RotateY(180.0);
    right_module_position->SetTranslation(-XOuterZoneSize*0.5 + x_shift_mod1, 0.0 + y_shift_mod1, 0.0 + z_shift_mod1);


    //frame part positions in the frame
    TGeoTranslation *left_frame_position = new TGeoTranslation(XOuterZoneSize+dXOuterFrame_Station163x45*0.5,
                                                               (dYOuterFrame_Station163x45-dYInnerFrame_Station163x45)*0.5,
                                                               0.0);

    TGeoTranslation *right_frame_position = new TGeoTranslation(-(XOuterZoneSize+dXOuterFrame_Station163x45*0.5),
                                                                (dYOuterFrame_Station163x45-dYInnerFrame_Station163x45)*0.5,
                                                                0.0);

    TGeoTranslation *top_long_frame_position = new TGeoTranslation(0.0,
                                                                   YOuterZoneSize*0.5+dYOuterFrame_Station163x45*0.5,
                                                                   0.0);

    TGeoTranslation *right_bottom_short_frame_position = new TGeoTranslation(-((XOuterZoneSize-hole_radius_with_frame-dXInnerFrame_Station163x45)*0.5 + hole_radius_with_frame),
                                                                             -dYInnerFrame_Station163x45*0.5-YOuterZoneSize*0.5,
                                                                             0.0);

    TGeoTranslation *left_bottom_short_frame_position = new TGeoTranslation(((XOuterZoneSize-hole_radius_with_frame-dXInnerFrame_Station163x45)*0.5 + hole_radius_with_frame),
                                                                            -dYInnerFrame_Station163x45*0.5-YOuterZoneSize*0.5,
                                                                            0.0);

    //frame positions in the station
    TGeoCombiTrans *upper_frame_trans = new TGeoCombiTrans();
    upper_frame_trans->SetTranslation(0.0, 0.0, 0.0);

    //station assembling
    module_with_slope_inner_zoneV->AddNode(slope_inner_zoneV, 0, slope_inner_zone_position);
    module_with_rect_inner_zoneV->AddNode(rectangle_inner_zoneV, 0, rect_inner_zone_position);

    if(IsInverted) {
        stationA->AddNode(module_with_rect_inner_zoneV, 0, left_module_position);
        stationA->AddNode(module_with_slope_inner_zoneV, 1, right_module_position);
    }
    else {
        stationA->AddNode(module_with_slope_inner_zoneV, 0, left_module_position);
        stationA->AddNode(module_with_rect_inner_zoneV, 1, right_module_position);
    }

    //frames
    composite_frameV->AddNode(vertical_frameV, 0, left_frame_position);
    composite_frameV->AddNode(vertical_frameV, 1, right_frame_position);
    composite_frameV->AddNode(horizontal_long_frameV, 0, top_long_frame_position);
    composite_frameV->AddNode(horizontal_short_frameV, 0, left_bottom_short_frame_position);
    composite_frameV->AddNode(horizontal_short_frameV, 1, right_bottom_short_frame_position);
    composite_frameV->AddNode(circle_inner_frameV, 0, new TGeoTranslation(0, -YOuterZoneSize*0.5, 0.0));

    stationA->AddNode(composite_frameV, 0, upper_frame_trans);

    //Station position
    Double_t station_shift = 0.0;
    station_shift = dZSensitiveVolume_Station163x45*0.5;
    TGeoTranslation *station_pos = new TGeoTranslation(x_position, y_position, z_position+station_shift);

    mother_volume->AddNode(stationA, 0, station_pos);
}
//------------------------------------------------------------------------------