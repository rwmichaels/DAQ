//    Strip Chart for Charge Asymmetry.  R. Michaels  9/04
//
//    To show the charge asymmetry online
//    Run this as follows from the root prompt:
//        .x qasy_strip.C
//    and don't forget the dot(.) before the x


#include "qasystrip.h"

THaStripChart::THaStripChart(int upd) :
  scaler(NULL), ntup(NULL), farray_ntup(NULL), canv(NULL),
  ntsize(0), updtime(upd), iclear(0), bcmasysum(0),
  bcmasysq(0), xcnt(0), bcmped(0)
{
  // constructor
}


THaStripChart::~THaStripChart() 
{
  if (scaler) delete scaler;
  if (ntup) delete ntup;
  if (farray_ntup) delete farray_ntup;
  if (canv) delete canv;
  if (bcmasysum) delete bcyasysum;
  if (bcmasysq) delete bcyasysq;
  if (xcnt) delete xcnt;
  if (bcmped) delete bcmped;
}

THaStripChart::Init() 
{
  iloop = 0;
  gSystem->Load("libdc.so");
  gSystem->Load("libscaler.so");
  scaler = new THaScaler("Left");
  if (scaler->Init() == -1) {
    cout << "Error initializing scalers !!"<<endl;
    exit(0);
  }
  // Get an initial reading, then delay.
  if (scaler->LoadDataOnline() == -1) {
     cout << "Failed to obtain data from VME crate.  Stopping"<<endl;
     exit(0);
  }
  Int_t dum = 0;
  for (Int_t i = 0; i < 2000000; i++) dum++;  // delay
  char string_ntup[]="time:bcm_u3:bcm_u10:trig1:trig2:trig3:trig4:trig5:qasy_bcm_u3:qasy_bcm_u10:CrossSection_T1:CrossSection_T3";
  ntsize = 12;  // this must match the length of string_ntup;
  ntup = new TNtuple("ascal","Scaler Data",string_ntup);
  farray_ntup = new Float_t[ntsize];
  bcmasysum = new Float_t[NBCMS];
  bcmasysq = new Float_t[NBCMS];
  xcnt = new Float_t[NBCMS];
  bcmped = new Float_t[NBCMS];
  BCM_CUT = 3000;      // cut on bcm_u3
  // Left-HRS BCM pedestals u1,u3,u10,d1,d3,d10
  Float_t bcmpedL[NBCMS] = { 52.5, 44.1, 110.7, 0., 94.2, 227. };
  for (int i = 0; i < NBCMS; i++) bcmped[i] = bcmpedL[i];
  Clear();
  InitPlots();
}

THaStripChart::InitPlots()
{
  char comment[80];
  sprintf(comment,"SCALER STRIPCHART  (Recent time at right, X axis is time in seconds)",updtime);
  canv = new TCanvas("c1",comment,200,10,1000,600);
  canv->Divide(2,2);
  Update();
}

THaStripChart::Update()
{
   if (!scaler) return;
// No, don't do this.   if (CheckClear()) Clear();
   if (scaler->LoadDataOnline() == -1) {
     cout << "Failed to obtain data from VME crate.  Stopping"<<endl;
     cout << "This probably means TS1 crate is down or was rebooted."<<endl;
     cout << "Try again:  'root .x qasy_strip.C'"<<endl;
     delete canv;
     exit(0);
   }
   iloop++;
   UpdateNtuple(); 
}

Int_t THaStripChart::CheckClear()
{
  FILE *fd;
  char file[]="clear.ctl";
  fd = fopen(file,"r");
  char clearstat[10];
  if (fd == NULL) return 0;
  fgets(clearstat,10,fd);
  iclear = atoi(clearstat);
  strcpy(clearstat,"0");
  fclose(fd);
  fd = fopen(file,"w");
  if (fd == NULL) return iclear;
  fputs(clearstat,fd);
  fclose(fd);
  return iclear;
}

THaStripChart::Clear() 
{
  ntup->Reset();
  memset(farray_ntup,0,ntsize);
  for (int i = 0; i < NBCMS; i++) {
    bcmasysum[i] = 0;
    bcmasysq[i] = 0;
    xcnt[i] = 0;
  }
  iloop = 0;
}

THaStripChart::ReStartAvg() 
{
  for (int i = 0; i < NBCMS; i++) {
    bcmasysum[i] = 0;
    bcmasysq[i] = 0;
    xcnt[i] = 0;
  }
  if (iloop > 9999999) iloop = 0;
}

THaStripChart::UpdateNtuple()
{
   if (ntsize <= 0 || farray_ntup == NULL) return;

   Float_t bcm_u3 = scaler->GetBcmRate("bcm_u3");
   Float_t bcm_u10 = scaler->GetBcmRate("bcm_u10");
   Float_t t1 = scaler->GetTrigRate(1);
   Float_t t2 = scaler->GetTrigRate(2);
   Float_t t3 = scaler->GetTrigRate(3);
   Float_t t4 = scaler->GetTrigRate(4);
   Float_t t5 = scaler->GetTrigRate(5);
   Float_t crsec1 = 0;
   Float_t crsec3 = 0;

   if (bcm_u3 > 0) {
     crsec1 = t1/bcm_u3;
     crsec3 = t3/bcm_u3;
   }

   int ibcm; 
   for (int i = 0; i < 2; i++) {
       char cbcm[50];
       if (i == 0) {
          ibcm = 1; // pick u3
          sprintf(cbcm,"bcm_u3");
       }
       if (i == 1) {
          ibcm = 2; // pick u10
          sprintf(cbcm,"bcm_u10");
       }
       Float_t sum = scaler->GetBcmRate(1,cbcm) - bcmped[ibcm]
                  + scaler->GetBcmRate(-1,cbcm) - bcmped[ibcm];
       Float_t asy = -999;
       if (sum != 0 && bcm_u3 > BCM_CUT) {
	 asy = (scaler->GetBcmRate(1,cbcm) - 
                scaler->GetBcmRate(-1,cbcm)) / sum;
         bcmasysum[ibcm] += asy;
         bcmasysq[ibcm] += asy*asy;
         xcnt[ibcm] += 1;
       }
   }

   Float_t bcmasy = 0;
   Float_t bcmerr = -9999;
   Float_t asy3 = -1;
   Float_t asy10 = -1;
   Float_t xct = 0;
   Float_t bcmasyavg = 0;
   Float_t bcmasyerr = 0;
// The BCMs are not indpendent.  Take the average, and
// use the maximum error as the error.
   for (ibcm = 0; ibcm < NBCMS; ibcm++) {
     if (xcnt[ibcm] <= 0) continue;
     bcmasyavg = bcmasysum[ibcm]/xcnt[ibcm];
     if (ibcm == 1) asy3 = bcmasyavg;
     if (ibcm == 2) asy10 = bcmasyavg;
     bcmasy += bcmasyavg;
     xct += 1;
     Float_t diff = bcmasysq[ibcm]/xcnt[ibcm] - bcmasyavg*bcmasyavg;
     if (diff < 0.0000001) {  // avoid "nan"
       bcmasyerr = 0;
     } else {
       bcmasyerr = sqrt(diff)/sqrt(xcnt[ibcm]);
     }
     if (bcmasyerr > bcmerr) bcmerr = bcmasyerr;
   }
   if (xct > 0) bcmasy = bcmasy/xct;

   if (iloop == 1 || iloop%3 == 0) {

     cout << "\nHeartbeat :  " << updtime*iloop;
     cout << "  seconds" << endl;
     if (bcmerr != -9999) {
       cout << "Charge asymmetry = "<< bcmasy<<"  +/- "<<bcmerr<<endl;
     } else {
       cout << "No update on asymmetry (no beam ?)"<<endl;
     }

     farray_ntup[0] = (float)updtime * iloop;
     farray_ntup[1] = bcm_u3;
     farray_ntup[2] = bcm_u10;
     farray_ntup[3] = t1;
     farray_ntup[4] = t2;
     farray_ntup[5] = t3;
     farray_ntup[6] = t4;
     farray_ntup[7] = t5;
     farray_ntup[8] = asy3;
     farray_ntup[9] = asy10;
     farray_ntup[10] = crsec1;
     farray_ntup[11] = crsec3;

     if (ntup) ntup->Fill(farray_ntup);

     if (iloop > 1) ReStartAvg();

     UpdatePlots();

   }

}

THaStripChart::UpdatePlots()
{
  if (!ntup || !canv) return;  

  char cutcomm[50];

  // to turn on/off the X axis label "SEC"
  Int_t drawx = 1;

  ntup->SetMarkerColor(4);
  ntup->SetMarkerStyle(21);

  gROOT->Reset();
  gStyle->SetOptStat(0);
  gStyle->SetLabelSize(0.06,"x");
  gStyle->SetLabelSize(0.06,"y");
  gStyle->SetPadBottomMargin(0.16);
  gStyle->SetPadTopMargin(0.1);
  gStyle->SetPadLeftMargin(0.16);
  gStyle->SetPadRightMargin(0.1);
  gStyle->SetTitleH(0.10);
  gStyle->SetTitleW(0.80);
  gStyle->SetTitleOffset(0.4);
  gStyle->SetTitleSize(0.10);

  canv->cd(1);
  // cut on time (e.g. 1800 = 30 minutes)
  sprintf(cutcomm,"%d-time<1800",updtime*iloop);  
  ntup->Draw("bcm_u3 : time  ",cutcomm);
  if (drawx) htemp->GetXaxis()->SetTitle(" sec");

  canv->cd(2);
  sprintf(cutcomm,"%d-time<1800",updtime*iloop); 
  ntup->Draw("trig5 : time  ",cutcomm);
  if (drawx) htemp->GetXaxis()->SetTitle(" sec");

  canv->cd(3);
  sprintf(cutcomm,"qasy_bcm_u3!=-1&&%d-time<1800",updtime*iloop); 
  ntup->Draw("qasy_bcm_u3 : time  ",cutcomm);
  //  if (drawx) htemp->GetXaxis()->SetTitle(" sec");

  canv->cd(4);
  sprintf(cutcomm,"qasy_bcm_u10!=-1&&%d-time<1800",updtime*iloop); 
  ntup->Draw("qasy_bcm_u10 : time  ",cutcomm);
  //  if (drawx) htemp->GetXaxis()->SetTitle(" sec");

  canv->Update();
}


void qasy_strip()
{  
  // Invoke this script with "root .x qasy_strip.C"  

  Int_t update_time = 10;  // update time (sec)
  TTimer *timer = new TTimer(update_time*1000); 
  THaStripChart *strip = new THaStripChart(update_time);
  timer->Connect("Timeout()", "THaStripChart", strip, "Update()");
  strip->Init();
  timer->TurnOn();
}

