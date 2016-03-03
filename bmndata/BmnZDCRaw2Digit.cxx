#include "TMath.h"
#include "TSystem.h"
#include "TBox.h"
#include "TText.h"
#include "TCanvas.h"
#include "TRandom.h"
#include "BmnZDCRaw2Digit.h"

static void fcn1(Int_t& npar, Double_t *gin, Double_t& f, Double_t *par, Int_t iflag);

BmnZDCRaw2Digit::BmnZDCRaw2Digit(){
  n_rec=0;
}
BmnZDCRaw2Digit::BmnZDCRaw2Digit(TString mappingFile, TString RunFile) {
    n_rec=0;
    TString dummy;
    ifstream in;

    TString dir = getenv("VMCWORKDIR");
    TString path = dir + "/input/";
    in.open((path + mappingFile).Data());
    in >> dummy >> dummy >> dummy >> dummy >> dummy >> dummy>> dummy >> dummy >> dummy >> dummy;
    maxchan = 0;
    int ixmin = -1, ixmax = -1, iymin = -1, iymax = -1;
    int xmin = 10000., xmax = -10000., ymin = 10000., ymax = -10000.;
    while (!in.eof()) {
        int id,chan,front_chan,size,ix,iy,used;
	float x,y;
        in >>std::hex >> id >>std::dec >> chan >> front_chan>>size>>ix>>iy>>x>>y>>used;
        if (!in.good()) break;
//	printf("%0x %d %d %d %d %d %f %f\n",id,chan,front_chan,size,ix,iy,x,y);
	if (size > 2) continue;
	if (chan <= 0) continue;
	if (front_chan <= 0) continue;
        zdc_map_element[n_rec].id=id;
        zdc_map_element[n_rec].adc_chan=chan-1;
	if (front_chan > maxchan) maxchan = front_chan;
        zdc_map_element[n_rec].chan=front_chan-1;
        zdc_map_element[n_rec].size=size-1;
        zdc_map_element[n_rec].ix=ix;
        zdc_map_element[n_rec].iy=iy;
        zdc_map_element[n_rec].x=x;
        zdc_map_element[n_rec].y=y;
        zdc_map_element[n_rec].used=used;
	if (x < xmin)
	{
	    xmin = x;
	    ixmin = n_rec;
	}
	if (x > xmax)
	{
	    xmax = x;
	    ixmax = n_rec;
	}
	if (y < ymin)
	{
	    ymin = y;
	    iymin = n_rec;
	}
	if (y > ymax)
	{
	    ymax = y;
	    iymax = n_rec;
	}
	n_rec++;
    }
    in.close();
    cell_size[0] = 75.;
    cell_size[1] = 150.;
    if (ixmin >= 0)
    {
	x_min = zdc_map_element[ixmin].x - cell_size[zdc_map_element[ixmin].size];
    }
    else
    {
	x_min = -1000.;
    }
    if (ixmax >= 0)
    {
	x_max = zdc_map_element[ixmax].x + cell_size[zdc_map_element[ixmin].size];
    }
    else
    {
	x_max = +1000.;
    }
    if (iymin >= 0)
    {
	y_min = zdc_map_element[iymin].y - cell_size[zdc_map_element[ixmin].size];
    }
    else
    {
	y_min = -1000.;
    }
    if (iymax >= 0)
    {
	y_max = zdc_map_element[iymax].y + cell_size[zdc_map_element[ixmin].size];
    }
    else
    {
	y_max = +1000.;
    }
//-------------------------------
    int RUN;
    const char *fname = RunFile.Data();
    char *delim = NULL;
    sscanf(&fname[strlen(fname) - 8], "%d", &RUN);
    strcpy(filname_base, gSystem->BaseName(fname));
    if ((delim = strrchr(filname_base, (int)'.'))) *delim = '\0';
    FILE *fin = 0;
    char filn[128], tit1[32] = {"Channel"}, tit2[32] = {"Calibration"}, tit3[32] = {"Error"};
    TString path1 = dir + "/parameters/zdc/";
    sprintf(filn, "%s%s_calibration.txt", path1.Data(), filname_base);
    fin = fopen(filn,"r");
    for (int i=0; i<maxchan; i++)
    {
	cal[i] = 1.;
	cale[i] = 0.;
	cal_out[i] = 1.;
	cale_out[i] = 0.;
    }
    if (!fin)
    {
	printf("Can't open calibration file %s, use default calibration coefficients 1.\n", filn);
    }
    else
    {
	Int_t ch;
	Float_t ca = 1., cae = 0.;
	fscanf(fin, "%s %s %s\n", tit1, tit2, tit3);
	while (fscanf(fin, "%d %f %f\n", &ch, &ca, &cae) == 3)
	{
	    if (ch < maxchan)
	    {
		cal[ch] = ca;
		cale[ch] = cae;
	    }
	};
	fclose(fin);
    }
    printf("%s\t%s\t%s\n", tit1, tit2, tit3);
    for (int i=0; i<maxchan; i++)
    {
	printf("%d\t%f\t%f\n", i, cal[i], cale[i]);
    }
//----------------------------------
    nevents = 0;
    use_log_function = 0;
    thres = 6.;
    wave2amp_flag = 1;
    min_samples = 10;
    ped_samples = 10;
    use_meanxy = 0;
    sigma_amp = 10.;
    shower_energy = 48.;
    shower_norm = 6.461048; // 7.5 cm modules
//    shower_norm = 13.923893; // 15 cm modules
    x_beam = 0.;
    y_beam = 0.;
    for (int j=0; j<MAX_CHANNELS; j++)
    {
	number[j] = 0;
	index[j] = 0;
	channel0[j] = 0;
	channel1[j] = 0;
	SampleProf[j] = NULL;
	for (int i=0; i<MAX_EVENTS; i++)
	{
	    amp_array[i][j] = 0.;
	    pedestals[i][j] = 0.;
	    profile_amp[i][j] = 0.;
	    profile_err[i][j] = 0.;
	}
    }
    hsum_sim = new TH1F("hsumsim","Sum of theoretical amplitudes", 1000, 0., 100.);
    hsum_raw = new TH1F("hsumraw","Sum of raw amplitudes", 2000, 0., 20000.);
    hsum     = new TH1F("hsumcal","Sum of calibrated amplitudes", 1000, 0., 200.);

    hxmean   = new TH1F("hxmean","Shower mean X", 1000, -500., +500.);
    hymean   = new TH1F("hymean","Shower mean Y", 1000, -500., +500.);

    SampleProf[0]   = new TProfile("samprof0","Average sampling wave, module 0", 200, 0., 200., -100000., +100000.,"s");
    SampleProf[1]   = new TProfile("samprof1","Average sampling wave, module 1", 200, 0., 200., -100000., +100000.,"s");
    SampleProf[2]   = new TProfile("samprof2","Average sampling wave, module 2", 200, 0., 200., -100000., +100000.,"s");
    SampleProf[3]   = new TProfile("samprof3","Average sampling wave, module 3", 200, 0., 200., -100000., +100000.,"s");
    SampleProf[4]   = new TProfile("samprof4","Average sampling wave, module 4", 200, 0., 200., -100000., +100000.,"s");
    SampleProf[5]   = new TProfile("samprof5","Average sampling wave, module 5", 200, 0., 200., -100000., +100000.,"s");
    SampleProf[6]   = new TProfile("samprof6","Average sampling wave, module 6", 200, 0., 200., -100000., +100000.,"s");
    SampleProf[7]   = new TProfile("samprof7","Average sampling wave, module 7", 200, 0., 200., -100000., +100000.,"s");
    SampleProf[8]   = new TProfile("samprof8","Average sampling wave, module 8", 200, 0., 200., -100000., +100000.,"s");
}


void BmnZDCRaw2Digit::print() {
     printf("id#\tchan\t\tf_chan\tsize\tix\tiy\tx\ty\tused\n");
     for(int i=0;i<n_rec;i++)
     printf("0x%06lX\t%d\t%d\t%d\t%d\t%d\t%d\t%g\t%g\n",
         zdc_map_element[i].id,zdc_map_element[i].chan+1,zdc_map_element[i].adc_chan+1,zdc_map_element[i].size+1,
         zdc_map_element[i].ix,zdc_map_element[i].iy,zdc_map_element[i].used,zdc_map_element[i].x,zdc_map_element[i].y); 
   
}


void BmnZDCRaw2Digit::fillSampleProfiles(TClonesArray *data, Float_t x, Float_t y, Float_t e, Int_t clsize) {
    Float_t amp = 0;
    double r = 0., dx = 0., dy = 0.;
    if (nevents == 0)
    {
	ncells = 0;
	x_beam = x;
	y_beam = y;
	shower_energy = e;
        for(int i=0;i<clsize*clsize;i++)
	{
	    channel1[i] = -1;
	}
	float xnear[MAX_CHANNELS], ynear[MAX_CHANNELS];
	int ixhalf = -1, iyhalf = -1, i0 = -1, imin = -1;
	float rmin = 10000.;
        for(int ind=0;ind<n_rec;ind++)
	{
	    number[ind] = -1;
	    channel0[ind] = -1;
	    dx = x - zdc_map_element[ind].x;
	    dy = y - zdc_map_element[ind].y;
	    r = TMath::Sqrt(dx*dx + dy*dy);
	    if (r < rmin)
	    {
		rmin = r;
		imin = ind;
	    }
	    if (x >= zdc_map_element[ind].x - cell_size[zdc_map_element[ind].size]/2. && \
	        x <  zdc_map_element[ind].x + cell_size[zdc_map_element[ind].size]/2. && \
	        y >= zdc_map_element[ind].y - cell_size[zdc_map_element[ind].size]/2. && \
	        y <  zdc_map_element[ind].y + cell_size[zdc_map_element[ind].size]/2 )
	    {
		if (x >= zdc_map_element[ind].x) ixhalf = 1;
		if (y >= zdc_map_element[ind].y) iyhalf = 1;
		number[ind] = ncells;
		channel0[ind] = zdc_map_element[ind].chan;
		channel1[ncells] = zdc_map_element[ind].chan;
		index[ncells] = ind;
		xnear[0] = zdc_map_element[index[ncells]].x;
		ynear[0] = zdc_map_element[index[ncells]].y;
		ncells++;
		i0 = ind;
//		printf(" ncells %d cell %d\n", ncells, zdc_map_element[ind].chan+1);
//		break;
	    }
	}
	if (i0 == -1)
	{
	    if (x<x_min || x>x_max || y<y_min || y>y_max)
	    {
		printf("Beam entry point (%f,%f) outside ZDC area!\n", x, y);
		return;
	    }
	    else if (imin >= 0)
	    {
		if (x >= zdc_map_element[imin].x) ixhalf = 1;
		if (y >= zdc_map_element[imin].y) iyhalf = 1;
		number[imin] = ncells;
		channel0[imin] = zdc_map_element[imin].chan;
		channel1[ncells] = zdc_map_element[imin].chan;
		index[ncells] = imin;
		xnear[0] = zdc_map_element[index[ncells]].x;
		ynear[0] = zdc_map_element[index[ncells]].y;
		ncells++;
	    }
	    else
	    {
		printf("Invalid beam entry point (%f,%f)!\n", x, y);
		return;
	    }
	}
	for(int i=1; i<clsize; i++)
	{
	    xnear[i] = zdc_map_element[index[0]].x + ixhalf*cell_size[zdc_map_element[index[0]].size];
	    ixhalf = -ixhalf;
	}
	for(int i=1; i<clsize; i++)
	{
	    ynear[i] = zdc_map_element[index[0]].y + iyhalf*cell_size[zdc_map_element[index[0]].size];
	    iyhalf = -iyhalf;
	}

	for(int i=0; i<clsize; i++)
	{
	    for(int j=0; j<clsize; j++)
	    {
    		for(int ind=0;ind<n_rec;ind++)
		{
		    if (ind == index[0]) continue;
		    if (xnear[i] >= zdc_map_element[ind].x - cell_size[zdc_map_element[ind].size]/2. && \
	    		xnear[i] <  zdc_map_element[ind].x + cell_size[zdc_map_element[ind].size]/2. && \
	    		ynear[j] >= zdc_map_element[ind].y - cell_size[zdc_map_element[ind].size]/2. && \
	    		ynear[j] <  zdc_map_element[ind].y + cell_size[zdc_map_element[ind].size]/2 )
		    {
			number[ind] = ncells;
			channel0[ind] = zdc_map_element[ind].chan;
			channel1[ncells] = zdc_map_element[ind].chan;
			index[ncells] = ind;
			ncells++;
//			printf(" ncells %d cell %d\n", ncells, zdc_map_element[ind].chan+1);
		    }
		}
	    }
	}
    }
    float ped = 0., xm = 0., ym = 0., s = 0.;
    if (data != NULL)
    {
     for (int i = 0; i < data->GetEntriesFast(); i++) {
       BmnADCDigit *digit = (BmnADCDigit*) data->At(i);
       int ind, num; 
       for(ind=0;ind<n_rec;ind++) if(digit->GetSerial()==zdc_map_element[ind].id && digit->GetChannel()==(zdc_map_element[ind].adc_chan)) break;
       if(ind==n_rec) continue; 
       if(zdc_map_element[ind].used==0) continue;
       if((num=number[ind])<0) continue;
       UShort_t *sam = digit->GetValue();
       for (int j = 0;j<digit->GetSamples(); j++)
       {
	    if (SampleProf[num]) SampleProf[num]->Fill(j,sam[j]>>4);
       }
       if ((amp = wave2amp(digit->GetSamples(),digit->GetValue(), &ped)) >= 0.)
       {
//	    printf("chan %d amp %f coef %f\n", zdc_map_element[ind].chan, amp, cal[zdc_map_element[ind].chan]);
	    xm += amp*cal[zdc_map_element[ind].chan]*zdc_map_element[ind].x;
	    ym += amp*cal[zdc_map_element[ind].chan]*zdc_map_element[ind].y;
	    s += amp*cal[zdc_map_element[ind].chan];
       }
     }
    }
    if (s > 0.) hxmean->Fill(xm/s);
    if (s > 0.) hymean->Fill(ym/s);
    nevents++;
}

void BmnZDCRaw2Digit::fillEvent(TClonesArray *data, TClonesArray *zdcdigit) {
    Float_t amp = 0., ped = 0.;
    for (int i = 0; i < data->GetEntriesFast(); i++) {
       BmnADCDigit *digit = (BmnADCDigit*) data->At(i);
       int ind; 
       for(ind=0;ind<n_rec;ind++) if(digit->GetSerial()==zdc_map_element[ind].id && digit->GetChannel()==(zdc_map_element[ind].adc_chan)) break;
       if(ind==n_rec) continue; 
       if(zdc_map_element[ind].used==0) continue;
       TClonesArray &ar_zdc = *zdcdigit;
       if ((amp = wave2amp(digit->GetSamples(),digit->GetValue(), &ped)) >= 0.)
       {
           new(ar_zdc[zdcdigit->GetEntriesFast()]) BmnZDCDigit(zdc_map_element[ind].ix,zdc_map_element[ind].iy,zdc_map_element[ind].x,zdc_map_element[ind].y,zdc_map_element[ind].size+1,
           zdc_map_element[ind].chan+1,amp);  
       }
    }
}

int BmnZDCRaw2Digit::fillCalibrateCluster(TClonesArray *data, Float_t x, Float_t y, Float_t e, Int_t clsize) {
    Float_t amp = 0;
    double r = 0., dx = 0., dy = 0.;
    static double coef[MAX_CHANNELS] = {1.};
    if (nevents >= MAX_EVENTS) return 1;
    if (nevents == 0)
    {
	ncells = 0;
	x_beam = x;
	y_beam = y;
	shower_energy = e;
        for(int i=0;i<clsize*clsize;i++)
	{
	    channel1[i] = -1;
	}
	float xnear[MAX_CHANNELS], ynear[MAX_CHANNELS];
	int ixhalf = -1, iyhalf = -1, i0 = -1, imin = -1;
	float rmin = 10000.;
        for(int ind=0;ind<n_rec;ind++)
	{
	    number[ind] = -1;
	    channel0[ind] = -1;
	    dx = x - zdc_map_element[ind].x;
	    dy = y - zdc_map_element[ind].y;
	    r = TMath::Sqrt(dx*dx + dy*dy);
	    if (r < rmin)
	    {
		rmin = r;
		imin = ind;
	    }
	    if (x >= zdc_map_element[ind].x - cell_size[zdc_map_element[ind].size]/2. && \
	        x <  zdc_map_element[ind].x + cell_size[zdc_map_element[ind].size]/2. && \
	        y >= zdc_map_element[ind].y - cell_size[zdc_map_element[ind].size]/2. && \
	        y <  zdc_map_element[ind].y + cell_size[zdc_map_element[ind].size]/2 )
	    {
		if (x >= zdc_map_element[ind].x) ixhalf = 1;
		if (y >= zdc_map_element[ind].y) iyhalf = 1;
		number[ind] = ncells;
		channel0[ind] = zdc_map_element[ind].chan;
		channel1[ncells] = zdc_map_element[ind].chan;
		index[ncells] = ind;
		xnear[0] = zdc_map_element[index[ncells]].x;
		ynear[0] = zdc_map_element[index[ncells]].y;
		ncells++;
		i0 = ind;
//		printf(" ncells %d cell %d\n", ncells, zdc_map_element[ind].chan+1);
//		break;
	    }
	}
	if (i0 == -1)
	{
	    if (x<x_min || x>x_max || y<y_min || y>y_max)
	    {
		printf("Beam entry point (%f,%f) outside ZDC area!\n", x, y);
		return 1;
	    }
	    else if (imin >= 0)
	    {
		if (x >= zdc_map_element[imin].x) ixhalf = 1;
		if (y >= zdc_map_element[imin].y) iyhalf = 1;
		number[imin] = ncells;
		channel0[imin] = zdc_map_element[imin].chan;
		channel1[ncells] = zdc_map_element[imin].chan;
		index[ncells] = imin;
		xnear[0] = zdc_map_element[index[ncells]].x;
		ynear[0] = zdc_map_element[index[ncells]].y;
		ncells++;
	    }
	    else
	    {
		printf("Invalid beam entry point (%f,%f)!\n", x, y);
		return 1;
	    }
	}
	for(int i=1; i<clsize; i++)
	{
	    xnear[i] = zdc_map_element[index[0]].x + ixhalf*cell_size[zdc_map_element[index[0]].size];
	    ixhalf = -ixhalf;
	}
	for(int i=1; i<clsize; i++)
	{
	    ynear[i] = zdc_map_element[index[0]].y + iyhalf*cell_size[zdc_map_element[index[0]].size];
	    iyhalf = -iyhalf;
	}

	for(int i=0; i<clsize; i++)
	{
	    for(int j=0; j<clsize; j++)
	    {
    		for(int ind=0;ind<n_rec;ind++)
		{
		    if (ind == i0) continue;
		    if (xnear[i] >= zdc_map_element[ind].x - cell_size[zdc_map_element[ind].size]/2. && \
	    		xnear[i] <  zdc_map_element[ind].x + cell_size[zdc_map_element[ind].size]/2. && \
	    		ynear[j] >= zdc_map_element[ind].y - cell_size[zdc_map_element[ind].size]/2. && \
	    		ynear[j] <  zdc_map_element[ind].y + cell_size[zdc_map_element[ind].size]/2 )
		    {
			number[ind] = ncells;
			channel0[ind] = zdc_map_element[ind].chan;
			channel1[ncells] = zdc_map_element[ind].chan;
			index[ncells] = ind;
			ncells++;
//			printf(" ncells %d cell %d\n", ncells, zdc_map_element[ind].chan+1);
		    }
		}
	    }
	}

	for(int j=0; j<MAX_EVENTS; j++)
	{
    	    for(int i=0;i<ncells;i++)
	    {
		amp_array[j][i] = 0.;
		profile_amp[j][i] = 0.;
		profile_err[j][i] = 1.;
	    }
	}
//	drawzdc();
//	hsum_raw = new TH1F("hsumraw","Sum of raw amplitudes", 1000, 0., 100000.);
//	hsum     = new TH1F("hsumcal","Sum of calibrated amplitudes", 1000, 0., 100000.);
//	hsum->Draw();
	if (data == NULL)
	{
	    for(int i=0; i<ncells; i++)
	    {
		coef[i] = (0.5*((i+1)%4) + 2.4)/10.;
	    }
	}
    }
    float ped = 0.;
    float sum_raw =0., sum_sim = 0.;
    double el = 0.;
    if (data != NULL)
    {
     for (int i = 0; i < data->GetEntriesFast(); i++) {
       BmnADCDigit *digit = (BmnADCDigit*) data->At(i);
       int ind, num; 
       for(ind=0;ind<n_rec;ind++) if(digit->GetSerial()==zdc_map_element[ind].id && digit->GetChannel()==(zdc_map_element[ind].adc_chan)) break;
       if(ind==n_rec) continue; 
       if(zdc_map_element[ind].used==0) continue;
       if((num=number[ind])<0) continue;
       if ((amp = wave2amp(digit->GetSamples(),digit->GetValue(), &ped)) >= 0.)
       {
//	    printf("chan %d amp %f coef %f\n", zdc_map_element[ind].chan, amp, cal[zdc_map_element[ind].chan]);
	    amp_array[nevents][num] = amp*cal[zdc_map_element[ind].chan];
	    pedestals[nevents][num] = ped;
	    sum_raw += amp_array[nevents][num];
       }
     }
     for (int i = 0; i < ncells; i++) {
	    dx = x - zdc_map_element[index[i]].x;
	    dy = y - zdc_map_element[index[i]].y;
	    r = TMath::Sqrt(dx*dx + dy*dy);
	    el = shower(r, cell_size[zdc_map_element[index[i]].size]);
	    sum_sim += el;
     }
    }
    else
    {
     for (int i = 0; i < ncells; i++) {
	    dx = x - zdc_map_element[index[i]].x;
	    dy = y - zdc_map_element[index[i]].y;
	    r = TMath::Sqrt(dx*dx + dy*dy);
	    el = shower(r, cell_size[zdc_map_element[index[i]].size]);
	    sum_sim += el;
	    el /= coef[i];
	    amp = el+gRandom->Gaus()*sigma_amp;
	    amp_array[nevents][i] = amp*cal[zdc_map_element[index[i]].chan];
	    pedestals[nevents][i] = ped;
	    sum_raw += amp_array[nevents][i];
     }
    }
    hsum_raw->Fill(sum_raw);
    hsum_sim->Fill(sum_sim);
    cellWeight(nevents);
    nevents++;
    return 0;
}

int BmnZDCRaw2Digit::fillCalibrateNumbers(TClonesArray *data, Float_t x, Float_t y, Float_t e, Int_t nchan, Int_t *cells) {
    Float_t amp = 0;
    double r = 0., dx = 0., dy = 0.;
    if (nevents >= MAX_EVENTS) return 1;
    if (nevents == 0)
    {
	ncells = nchan;
	x_beam = x;
	y_beam = y;
	int i0 = -1;
        for(int ind=0;ind<n_rec;ind++)
	{
	    if (x >= zdc_map_element[ind].x - cell_size[zdc_map_element[ind].size]/2. && \
	        x <  zdc_map_element[ind].x + cell_size[zdc_map_element[ind].size]/2. && \
	        y >= zdc_map_element[ind].y - cell_size[zdc_map_element[ind].size]/2. && \
	        y <  zdc_map_element[ind].y + cell_size[zdc_map_element[ind].size]/2 )
	    {
		i0 = ind;
		break;
	    }
	}
	if (i0 == -1)
	{
	    if (x<x_min || x>x_max || y<y_min || y>y_max)
	    {
		printf("Beam entry point (%f,%f) outside ZDC area!\n", x, y);
		return 1;
	    }
	}
	shower_energy = e;
        for(int i=0;i<ncells;i++)
	{
	    channel1[i] = -1;
	}
        for(int ind=0;ind<n_rec;ind++)
	{
	    number[ind] = -1;
	    channel0[ind] = -1;
	    for (int j=0; j<ncells; j++)
	    {
		if ((cells[j]-1) == zdc_map_element[ind].chan)
		{
		    number[ind] = j;
		    channel0[ind] = zdc_map_element[ind].chan;
		    channel1[j] = zdc_map_element[ind].chan;
		    index[j] = ind;
		    break;
		}
	    }
	}
	for(int j=0; j<MAX_EVENTS; j++)
	{
    	    for(int i=0;i<ncells; i++)
	    {
		amp_array[j][i] = 0.;
		profile_amp[j][i] = 0.;
		profile_err[j][i] = 1.;
	    }
	}
    }
    float ped = 0.;
    float sum_raw =0., sum_sim = 0.;
    double el = 0.;
    for (int i = 0; i < data->GetEntriesFast(); i++) {
       BmnADCDigit *digit = (BmnADCDigit*) data->At(i);
       int ind, num; 
       for(ind=0;ind<n_rec;ind++) if(digit->GetSerial()==zdc_map_element[ind].id && digit->GetChannel()==(zdc_map_element[ind].adc_chan)) break;
       if(ind==n_rec) continue; 
       if(zdc_map_element[ind].used==0) continue;
       if((num=number[ind])<0) continue;
       if ((amp = wave2amp(digit->GetSamples(),digit->GetValue(), &ped)) >= 0.)
       {
	    amp_array[nevents][num] = amp*cal[zdc_map_element[ind].chan];
	    pedestals[nevents][num] = ped;
	    sum_raw += amp_array[nevents][num];
       }
    }
    for (int i = 0; i < ncells; i++) {
	    dx = x - zdc_map_element[index[i]].x;
	    dy = y - zdc_map_element[index[i]].y;
	    r = TMath::Sqrt(dx*dx + dy*dy);
	    el = shower(r, cell_size[zdc_map_element[index[i]].size]);
	    sum_sim += el;
    }
    hsum_raw->Fill(sum_raw);
    hsum_sim->Fill(sum_sim);
    cellWeight(nevents);
    nevents++;
    return 0;
}

int BmnZDCRaw2Digit::fillCalibrateAll(TClonesArray *data, Float_t x, Float_t y, Float_t e) {
    Float_t amp = 0;
    Int_t j = 0;
    if (nevents >= MAX_EVENTS) return 1;
    if (nevents == 0)
    {
	x_beam = x;
	y_beam = y;
	int i0 = -1;
        for(int ind=0;ind<n_rec;ind++)
	{
	    if (x >= zdc_map_element[ind].x - cell_size[zdc_map_element[ind].size]/2. && \
	        x <  zdc_map_element[ind].x + cell_size[zdc_map_element[ind].size]/2. && \
	        y >= zdc_map_element[ind].y - cell_size[zdc_map_element[ind].size]/2. && \
	        y <  zdc_map_element[ind].y + cell_size[zdc_map_element[ind].size]/2 )
	    {
		i0 = ind;
		break;
	    }
	}
	if (i0 == -1)
	{
	    printf("Beam entry point (%f,%f) outside ZDC area!\n", x, y);
	    return 1;
	}
	ncells = n_rec;
	shower_energy = e;
        for(int i=0;i<n_rec;i++)
	{
	    channel1[i] = -1;
	}
        for(int ind=0;ind<n_rec;ind++)
	{
		    number[ind] = j;
		    channel0[ind] = zdc_map_element[ind].chan;
		    channel1[j] = zdc_map_element[ind].chan;
		    j++;
	}
	for(int j=0; j<MAX_EVENTS; j++)
	{
    	    for(int i=0;i<n_rec; i++)
	    {
		amp_array[j][i] = 0.;
		profile_amp[j][i] = 0.;
		profile_err[j][i] = 1.;
	    }
	}
    }
    float ped = 0.;
    for (int i = 0; i < data->GetEntriesFast(); i++) {
       BmnADCDigit *digit = (BmnADCDigit*) data->At(i);
       int ind, num; 
       for(ind=0;ind<n_rec;ind++) if(digit->GetSerial()==zdc_map_element[ind].id && digit->GetChannel()==(zdc_map_element[ind].adc_chan)) break;
       if(ind==n_rec) continue; 
       if(zdc_map_element[ind].used==0) continue;
       if((num=number[ind])<0) continue;
       if ((amp = wave2amp(digit->GetSamples(),digit->GetValue(), &ped)) >= 0.)
       {
	    amp_array[nevents][num] = amp*cal[zdc_map_element[ind].chan];
	    pedestals[nevents][num] = ped;
       }
    }
    cellWeight(nevents);
    nevents++;
    return 0;
}

void BmnZDCRaw2Digit::cellWeight(int ievent)
{
    int n;
    double s = 0., e = 0., r = 0., x0 = 0., y0 = 0.;
    if (use_meanxy)
    {
	for (int i=0; i<ncells; i++)
	{
	    x0 += zdc_map_element[index[i]].x*amp_array[ievent][i];
	    y0 += zdc_map_element[index[i]].y*amp_array[ievent][i];
	    s += amp_array[ievent][i];
	}
	x0 /= s;
	y0 /= s;
    }
    else
    {
	x0 = x_beam;
	y0 = y_beam;
    }
    for (int i=0; i<ncells; i++)
    {
	r = TMath::Sqrt((zdc_map_element[index[i]].x-x0)*(zdc_map_element[index[i]].x-x0)+(zdc_map_element[index[i]].y-y0)*(zdc_map_element[index[i]].y-y0));
	e = shower(r, cell_size[zdc_map_element[index[i]].size]);
	profile_amp[ievent][i] = e;
	profile_err[ievent][i] = sigma_amp*sigma_amp;
    }
}

void BmnZDCRaw2Digit::calibrate() {
   // initialize TMinuit with a maximum of 5 params

   Double_t vstart[MAX_CHANNELS], step[MAX_CHANNELS], vmin = 0.00001, vmax = 100000.;

//   printf(" ncells = %d\n", ncells);
   TMinuit *gMinuit = new TMinuit(ncells+1);  
   gMinuit->SetFCN(fcn1);
 
   Double_t arglist[MAX_CHANNELS];
   Int_t ierflg = 0;
 
   arglist[0] = 1;
   gMinuit->mnexcm("SET ERR", arglist ,1,ierflg);
 
// Set starting values and step sizes for parameters
   char name[32] = {""};
//   printf(" ncells1 = %d\n", ncells);
   for (int i=0; i<ncells; i++)
   {
    vstart[i] = pstart;
    step[i] = pstep;
    sprintf(name, "Coeff_%03d", channel1[i]+1);
    gMinuit->mnparm(i, name, vstart[i], step[i], vmin, vmax, ierflg);
//    printf("%d name %s min %f max %f start %f step %f\n",i,name,vmin,vmax,vstart[i],step[i]);
   } 
// Now ready for minimization step
   arglist[0] = 50000;
   arglist[1] = 0.1;
   gMinuit->mnexcm("MIGRAD", arglist ,2, ierflg);
 
// Print results
   Double_t amin,edm,errdef;
   Int_t nvpar,nparx,icstat;
   gMinuit->mnstat(amin,edm,errdef,nvpar,nparx,icstat);
   gMinuit->mnprin(3,amin);

//----- write results

    for (int i=0; i<maxchan; i++)
    {
	cal_out[i] = cal[i];
	cale_out[i] = cale[i];
    }
    Double_t par[MAX_CHANNELS], epar[MAX_CHANNELS];
    for (int i=0; i<ncells; i++)
    {
	gMinuit->GetParameter(i,par[i],epar[i]);
	cal_out[channel1[i]] = cal[channel1[i]]*par[i];
	cale_out[channel1[i]] = cal[channel1[i]]*epar[i];
//	printf("   %d par %f epar %f\n",i,par[i],epar[i]);
    }
    TString dir = getenv("VMCWORKDIR");
    TString path = dir + "/parameters/zdc/";
    FILE *fout = 0;
    char filn[128], tit1[32] = {"Channel"}, tit2[32] = {"Calibration"}, tit3[32] = {"Error"};
    sprintf(filn, "%s%s_calibration_out.txt", path.Data(), filname_base);
    fout = fopen(filn,"w");
    if (!fout)
    {
	printf("Can't open output calibration file %s\n", filn);
    }
    else
    {
	fprintf(fout, "%s\t%s\t%s\n", tit1, tit2, tit3);
	for (int i=0; i<maxchan; i++)
	{
	    fprintf(fout, "%3d\t%f\t%f\n", i+1, cal_out[i], cale_out[i]);
	}
	fclose(fout);
    }
    printf("%s\t%s\t%s\n", tit1, tit2, tit3);
    for (int i=0; i<maxchan; i++)
    {
	printf("%3d\t%f\t%f\n", i+1, cal_out[i], cale_out[i]);
    }
//-------
    float sum = 0.;
    for (int j=0; j<nevents; j++)
    {
	sum = 0.;
	for (int i=0; i<ncells; i++)
	{
//	    sum += amp_array[j][i]*cal_out[channel1[i]];
	    sum += amp_array[j][i]*par[i];
	}
	hsum->Fill(sum);
    }
//    hsum->Draw();
//    drawzdc();
}

static void fcn1(Int_t& npar, Double_t *gin, Double_t& f, Double_t *par, Int_t iflag)
{
    double chi = 0., le = 0., laA = 0.;
    for (int j=0; j<nevents; j++)
    {
	for (int i=0; i<npar; i++)
	{
//	    if (j == nevents-1) printf(" nevents %d par %f prof %f eprof %f amp %f\n", nevents, par[i], profile_amp[j][i], profile_err[j][i], amp_array[j][i]);
	    if (use_log_function)
	    {
		le = TMath::Log10(profile_amp[j][i]);
		laA = TMath::Log10(par[i]*amp_array[j][i]);
	    }
	    else
	    {
		le = profile_amp[j][i];
		laA = par[i]*amp_array[j][i];
	    }
	    chi += (le - laA)*(le - laA)/profile_err[j][i];
	}
    }
    f = chi;
    return;
}

double BmnZDCRaw2Digit::ch(double x, double r){ double amp = ( exp(x/r) + exp(-x/r) )/2.; return amp;}
double BmnZDCRaw2Digit::sh(double x, double r){ double amp = ( exp(x/r) - exp(-x/r) )/2.; return amp;}

//-----------------------------------------------PP1 
double BmnZDCRaw2Digit::PP1(double x, double h)
{//Shower profile in cell with size h  when R <= h/2

double a1,a2,b1,b2,B;
double ss1=1.;
double bb2=6.*ss1;
double bb1=1.4*ss1;

double E = shower_energy; // total hadron energy , GeV

a2=0.105 - 0.014*log(E);   //R -dependences
b1=bb1   - 0.120*log(E);   //R -dependences
b2=bb2   - 0.260*log(E);   //R -dependences

a1 = 1. - a2;
B = (a1*b1+a2*b2);
double  fff=0.;
if(x <= h/2) fff=(a1*b1*(1.-exp(-h/(2.*b1))*ch(x,b1)) + a2*b2*(1.-exp(-h/(2.*b2))*ch(x,b2 )))/(h*B);
return fff;
}

//---------------------------------------- PP2 
double BmnZDCRaw2Digit::PP2(double x, double h)
{//Shower profile in cell with size h when R > h/2

double a1,a2,b1,b2,B;
double ss1=1.;
double bb2=6.*ss1;
double bb1=1.4*ss1;

double E = shower_energy; // total hadron energy , GeV

a2 = 0.105 - 0.014*log(E); // R -dependences
b1 = bb1   - 0.120*log(E); // R -dependences
b2 = bb2   - 0.260*log(E); // R -dependences

a1 = 1. - a2;
B = (a1*b1 + a2*b2);
double fff = 0.;
if(x > h/2) fff = (a1*b1*exp(-x/b1)*sh(h,2.*b1) )/(h*B) + (a2*b2*exp(-x/b2)*sh(h,2.*b2 ) )/(h*B);
return fff;
}

double BmnZDCRaw2Digit::shower(double x, double h){double amp = PP1(x/10.,h/10.) + PP2(x/10.,h/10.); return shower_energy*shower_norm*amp;}

float BmnZDCRaw2Digit::wave2amp(UChar_t ns, UShort_t *s, Float_t *pedestal)
{
	    float pedest = 0., ampl = 0., signal = 0., signal_max = 0.;
	    int nsignal = 0, nsignal_max = 0, ismax = -1;
	    float ampl_max = 0.;
	    if (ns > 0)
	    {
		for (int m = 0; m < ns; m++)
		{
		    if (m < ped_samples)
		    {
			pedest += (s[m]>>4);
			if (m == (ped_samples-1))
			{
			    pedest /= ped_samples;
			}
			continue;
		    }
		    else
		    {
			ampl = -(float)(s[m]>>4) + pedest;
			if (ampl > thres)
			{
			    signal += ampl;
			    nsignal++;
			    if (ampl > ampl_max)
			    {
				ampl_max = ampl;
				ismax = m;
			    }
			}
			else
			{
			    if (nsignal < min_samples)
			    {
				signal = 0.;
				nsignal = 0;
			    }
			    else if (nsignal > nsignal_max)
			    {
				signal_max = signal;
				nsignal_max = nsignal;
			    }
			}
		    }
		} // loop over samples
//		printf("Chan %d Amplmax %f Ped %f imax %d nsam %d\n",l,ampl_max,pedest[l],ismax,nsamples[l]);
		if (nsignal_max > 0 || ampl_max > 0.)
		{
		  if (wave2amp_flag) signal_max = ampl_max;
		}
		else
		{
		  signal_max = 0.;
		}
	    } // if samples exist
	    else
	    {
		signal_max = -1.;
	    }
	    *pedestal = pedest;
	    return signal_max;
}

void BmnZDCRaw2Digit::drawzdc(int nohist)
{
  TCanvas *callbe0 = new TCanvas("callbe0", "ZDC mapping", 800,800);
  gPad->Range(x_min-10.,y_min-10.,x_max+10.,y_max+10.);
  int i;
  char text[16];
  double x1,x2,y1,y2;
  TBox *b[MAX_CHANNELS] = {0};
  TText *t[MAX_CHANNELS] = {0};
  TText *tc[MAX_CHANNELS] = {0};
  callbe0->cd();
  for (i=0; i<n_rec; i++)
    {
      x1 = zdc_map_element[i].x - cell_size[zdc_map_element[i].size]/2.;
      x2 = zdc_map_element[i].x + cell_size[zdc_map_element[i].size]/2.;
      y1 = zdc_map_element[i].y - cell_size[zdc_map_element[i].size]/2.;
      y2 = zdc_map_element[i].y + cell_size[zdc_map_element[i].size]/2.;
//      printf("%f %f %f %f\n",x1,y1,x2,y2);
      b[i] = new TBox(x1,y1,x2,y2);
      b[i]->Draw();
      if (ncells > 0 && number[i] >= 0) b[i]->SetLineColor(kRed);
      b[i]->SetFillStyle(0);
      b[i]->SetLineStyle(0);
      sprintf(text,"%d",zdc_map_element[i].chan+1);
      t[i] = new TText(zdc_map_element[i].x,zdc_map_element[i].y,text);
      t[i]->Draw();
      if (ncells > 0 && number[i] >= 0)
      {
	t[i]->SetTextColor(kRed);
	t[i]->SetTextAlign(21);
      }
      else
      {
	t[i]->SetTextAlign(21);
      }
      t[i]->SetTextSize(0.02);
//      if (ncells > 0 && number[i] >= 0)
//      {
//        sprintf(text,"%6.3f",cal_out[channel0[i]]);
        sprintf(text,"%6.3f",cal_out[zdc_map_element[i].chan]);
        tc[i] = new TText(zdc_map_element[i].x,zdc_map_element[i].y,text);
        tc[i]->Draw();
        if (ncells > 0 && number[i] >= 0) tc[i]->SetTextColor(kRed);
        tc[i]->SetTextAlign(23);
        tc[i]->SetTextSize(0.02);
//      }
    }

  if ((hsum_raw == NULL && hsum == NULL)||nohist) return;

  TCanvas *calres = new TCanvas("calres", "ZDC calibration", 800,800);
  calres->cd();
  calres->Divide(1,2);
  calres->cd(1);
  if (hsum_raw) hsum_raw->Draw();
  gPad->AddExec("exselt","select_hist()");
  calres->cd(2);
  if (hsum) hsum->Draw();
  gPad->AddExec("exselt","select_hist()");
//  calres->cd(3);
//  if (hsum_sim) hsum_sim->Draw();
//  gPad->AddExec("exselt","select_hist()");
  return;
}

void BmnZDCRaw2Digit::drawprof()
{
  TCanvas *csampro = new TCanvas("csampro", "ZDC sample profiles", 800,800);
  csampro->cd();
  if (ncells <= 4) csampro->Divide(2,2);
  else if (ncells <= 9) csampro->Divide(3,3);
  else csampro->Divide(3,3);
  for (int i=0; i<(ncells<9?ncells:9); i++)
  {
    csampro->cd(i+1);
    if (SampleProf[i]) SampleProf[i]->Draw();
    gPad->AddExec("exselt","select_hist()");
  }

  TCanvas *calres = new TCanvas("calres", "ZDC mean X and Y", 800,800);
  calres->cd();
  calres->Divide(1,2);
  calres->cd(1);
  if (hxmean) hxmean->Draw();
  gPad->AddExec("exselt","select_hist()");
  calres->cd(2);
  if (hymean) hymean->Draw();
  gPad->AddExec("exselt","select_hist()");
//  hsum_raw->Print();
  return;
}


ClassImp(BmnZDCRaw2Digit)

