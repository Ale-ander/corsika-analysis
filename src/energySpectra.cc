#include <crsRead/MCorsikaReader.h>

#include <crs/TSubBlock.h>
#include <crs/MRunHeader.h>
#include <crs/MEventHeader.h>
#include <crs/MEventEnd.h>
#include <crs/MParticleBlock.h>
#include <crs/MLongitudinalBlock.h>
#include <crs/MParticle.h>

#include <TCanvas.h>
#include <TH1D.h>
#include <TH2D.h>
#include <iostream>
#include <string>


std::string usage()
{
  std::string usg = "usage: ./energySpectra <corsika file name>";
  return usg;
}


int main( int argc, char **argv )
{
  if(argc < 2)
  {
    std::cout << usage() << std::endl;
    return 1;
  }

  // E in [1, 100] TeV => log(E[GeV]) in [3,5]
  TH1D* hEnergy = new TH1D("hEnergy", "energy spectra", 250, 100, 10000);


  /*
   * -----------------------------------------------
   * Reading the file
   * -----------------------------------------------
   */

  std::string fname(argv[1]);

  crsRead::MCorsikaReader cr(fname, 3);
  crs::MRunHeader run;

  
  while( cr.GetRun( run ) ) // Loop over the runs. Usually one
  {
    crs::MEventHeader shower;
    while( cr.GetShower( shower ) ) // Loop over the shower in a run. 1000 in our case
    {
      //std::cout << "shower energy: " << log10(shower.GetEnergy()) << std::endl;
      hEnergy->Fill(shower.GetEnergy());
    }
  }
  
  TCanvas *cEnergy = new TCanvas("cEnergy", "energy spectra", 1200, 1200*9/16);

  hEnergy->SetFillColor(kBlue);
  hEnergy->SetFillStyle(3005);
  hEnergy->Draw();

  hEnergy->GetXaxis()->SetTitle("Energy [GeV]");

  cEnergy->Update();
  cEnergy->SaveAs("./plots/energySpectra.png");
  
  return 0;
}
