#include "dummyExperiment.h"

#include <math.h>
#include <iostream>
#include <sstream>


#include <TCanvas.h>
#include <TGaxis.h>
#include <TAttMarker.h>
#include <TMarker.h>
#include <TPaveText.h>

dummyExperiment::dummyExperiment(double experimentSize,
				 double tankSize,
				 double tankSpacing)  
{
  fEsize = experimentSize;
  fTsize = tankSize;
  fTspacing = tankSpacing;

  fNtank = floor( fEsize/(fTsize+fTspacing) );
  fSignals = new double[fNtank*fNtank];
  fTimes = new double[fNtank*fNtank];

  
  Reset();
}

dummyExperiment::~dummyExperiment()
{
  delete fSignals;
  delete fTimes;
}

void
dummyExperiment::Reset()
{
  for(int j=0; j<fNtank; j++)
    for(int i=0; i<fNtank; i++)
    {
      fSignals[i+j*fNtank] = 0.0;
      fTimes[i+j*fNtank] = 0.0;
    }

  fFirstTime = 0.0;
  fLastTime = 0.0;
  fMaxSign = 0.0;
  fTriggerS = 4.0;
  fMuonWeight = 2.0;
  fEmWeight = 2.0;

}

void
dummyExperiment::findFirst()
{
  fFirstTime = 1000000.0;
  for(int i=0; i<fNtank*fNtank; i++)
  {
    if( fTimes[i] < 0.00001 || fSignals[i] < fTriggerS )
      continue;
    if(fTimes[i] < fFirstTime)
      fFirstTime=fTimes[i];
    //std::cout << "Time: " << fFirstTime << std::endl;
  }
  if(abs(fFirstTime - 1000000.0)<0.00001)
    fFirstTime = 0.0;
}

void
dummyExperiment::findLast()
{
  
  for(int i=0; i<fNtank*fNtank; i++)
  {
    if( fTimes[i] < 0.00001 || fSignals[i] < fTriggerS )
      continue;
    if(fTimes[i] > fLastTime)
      fLastTime=fTimes[i];
    //std::cout << "Time: " << fFirstTime << std::endl;
  }
}


void
dummyExperiment::findHigher()
{
  for(int i=0; i<fNtank*fNtank; i++)
  {
    if(fSignals[i] > fMaxSign)
      fMaxSign=fSignals[i];
    //std::cout << "Signal: " << fMaxSign << std::endl;
  }
  
}

void
dummyExperiment::Fill( double x, double y, double time, int pID )
{
  int i = floor((x + (fEsize/2))/(fTsize+fTspacing));
  int j = floor((y + (fEsize/2))/(fTsize+fTspacing));
  if( i >= fNtank || i < 0 || j >= fNtank || j <0)
    return;
  //std::cout << "Filling tank: (" << i << "," << j << ")" << std::endl;
  double tX = (i+0.5)*(fTsize+fTspacing)-(fEsize/2);
  double tY = (j+0.5)*(fTsize+fTspacing)-(fEsize/2);

  double distFromTankCenter = sqrt(pow(x-tX,2)+pow(y-tY,2));
  
  if (distFromTankCenter < (fTsize/2))
  {
    if( pID == 5 || pID == 6)
      fSignals[i+j*fNtank] += fMuonWeight;
    else
      fSignals[i+j*fNtank] += fEmWeight;

    if(time < fTimes[i+j*fNtank] || fTimes[i+j*fNtank] < 0.000001)
      fTimes[i+j*fNtank] = time;
  }
  
}



int
dummyExperiment::GetCol( double time, double firstT, double lastT )
{

  double tBinSize = (lastT - firstT)/10;
  
  double dT = (time - firstT)/tBinSize;

  if(dT<0.0)
    return 1;

  //std::cout << "Time: " << firstT
  //          << " " << lastT
  //	    << " " << tBinSize << std::endl;
  //std::cout << "-- Time bin: " << dT << std::endl;
  
  if(dT<0.2)
    return 632;

  if(dT<0.5)
    return 807;

  if(dT<1)
    return 798;

  if(dT<2)
    return 409;

  if(dT<3)
    return 413;

  if(dT<5)
    return 418;

  if(dT<7)
    return 862;

  if(dT<9)
    return 601;

  if(dT<10)
    return 874;

  return 1;
}

void
dummyExperiment::Draw(std::string outFile)
{
  double canvasDim = 1500;
  TCanvas cExp("cExp", "dummy experiment", canvasDim*1.3, canvasDim);

  double cSize = 11.0;
  cExp.Range(-cSize, -cSize, cSize*1.3, cSize);

  double wSize = 0.80*cSize;
  TGaxis xAxis(-wSize, -wSize, wSize, -wSize, -fEsize/2, fEsize/2, 510, "");
  xAxis.SetName("xAxis");
  xAxis.SetTitle("[m]");
  TGaxis yAxis(-wSize, -wSize, -wSize, wSize, -fEsize/2, fEsize/2, 510, "");
  yAxis.SetName("yAxis");
  yAxis.SetTitle("[m]");

  TPaveText pave(wSize, 0, 1.25*cSize, wSize, "bl");
  pave.SetBorderSize(1);
  pave.SetFillColor(kWhite);
  pave.SetTextFont(42);
  pave.SetTextAlign(13);
  xAxis.Draw();
  yAxis.Draw();

  findFirst();
  findLast();
  findHigher();
  
  TMarker* stMarker[fNtank*fNtank];
  
  
  for(int j=0; j<fNtank; j++)
    for(int i=0; i<fNtank; i++)
    {
      int id = i+j*fNtank;

      stMarker[id] = new TMarker();
 
      double tX = (i+0.5)*(fTsize+fTspacing)-(fEsize/2);
      double tY = (j+0.5)*(fTsize+fTspacing)-(fEsize/2);

      stMarker[id]->SetX(tX*wSize/(fEsize/2)); 
      stMarker[id]->SetY(tY*wSize/(fEsize/2));

      if( fSignals[id] > fTriggerS )
      {
	int col = GetCol(fTimes[id], fFirstTime, fLastTime);
	double scale = (fSignals[id]*8.0)/fMaxSign;
	if(scale>1)
	  stMarker[id]->SetMarkerSize(scale);

	stMarker[id]->SetMarkerStyle(20);
	stMarker[id]->SetMarkerColor(col);
	  
      }
      else
	stMarker[id]->SetMarkerStyle(7);
      stMarker[id]->Draw();
    }

  std::ostringstream outstr;
  
  pave.AddText("Shower INFO:");

  outstr.str(""); outstr << "ID: " << fShowerID;
  pave.AddText(outstr.str().c_str());

  outstr.str(""); outstr << "Energy MC: " << fEnergyGen << " GeV";
  pave.AddText(outstr.str().c_str());

  outstr.str(""); outstr << "Obs. Level: " << fObsLev << " m";
  pave.AddText(outstr.str().c_str());

  pave.AddText("------------------");

  outstr.str(""); outstr << "Hottest station: " << fMaxSign << " DU";
  pave.AddText(outstr.str().c_str());

  outstr.str(""); outstr << "Event start at: " << fFirstTime << " ns";
  pave.AddText(outstr.str().c_str());

  outstr.str(""); outstr << "Duration: " << fLastTime - fFirstTime << " ns";
  pave.AddText(outstr.str().c_str());

  pave.Draw();
  
  
  cExp.SaveAs(outFile.c_str());
  delete *stMarker;
}

    
void
dummyExperiment::SetShowerInfo(double shEnergy, double obsLev, int shID)
{
  fEnergyGen = shEnergy;
  fObsLev = obsLev;
  fShowerID = shID;
}
