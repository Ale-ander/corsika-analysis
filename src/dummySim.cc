#include <iostream>
#include <crsRead/MCorsikaReader.h>

#include <crs/TSubBlock.h>
#include <crs/MRunHeader.h>
#include <crs/MEventHeader.h>
#include <crs/MEventEnd.h>
#include <crs/MParticleBlock.h>
#include <crs/MLongitudinalBlock.h>
#include <crs/MParticle.h>

#include <sstream>
#include <string>
#include <cstdlib>
#include <unistd.h>

#include "dummyExperiment.h"


std::string usage();
int getOptions(int argc, char** argv, int& n);


int main(int argc, char** argv)
{
  int shIndex = 1;
  const int nOptions = getOptions(argc, argv, shIndex);

  if(nOptions < 0 || argc-nOptions < 1 )
  {
    std::cout << usage() << std::endl;
    return 1;
  }

  /*
   * -----------------------------------------------
   * Reading the file
   * -----------------------------------------------
   */

  std::string fname(argv[nOptions]);

  crsRead::MCorsikaReader cr(fname, 1);
  crs::MRunHeader run;
  
  dummyExperiment dExp(250, 7, 0);

  double logE = 0.0;
  double obsLev = 0.0;
  int i=0;

  dExp.SetMinimumSignal(1.0);
  dExp.SetMuonWeight(1.0);
  dExp.SetEmWeight(0.01);
  if( shIndex == 0 )
  {
    std::cout << "ERROR: ID should start from 1" << std::endl;
    return -1;
  }
  while( cr.GetRun( run ) ) // Loop over the runs. Usually one
  {
    crs::MEventHeader shower;
    while( cr.GetShower( shower ) )
    {
      i++;
      if(i != shIndex)
	continue;

      std::cout << "-----------------------------" << std::endl;
      std::cout << "plot of the shower nr: " << i << std::endl;
      logE = log10( shower.GetEnergy() );
      obsLev = shower.GetObservationHeight( shower.GetNObservationLevels()-1 )/100.0;
      std::cout << "shower obs level: " << obsLev << " m" << std::endl;

      dExp.SetShowerInfo(shower.GetEnergy(), obsLev, shIndex);
      
      crs::TSubBlock sub_block;
      while( cr.GetData( sub_block ) )
      {
	//std::cout << "Block type: " << data.GetBlockTypeName() << std::endl;

	crs::MParticleBlock data(sub_block);	
	
	if( data.GetBlockType() == crs::TSubBlock::ePARTDATA )
	{
	  for( crs::MParticleBlock::ParticleListConstIterator p_it = data.FirstParticle();
	       p_it != data.LastParticle();
	       ++p_it )
	  {
	    crs::MParticle part(*p_it);

	  int pID = part.GetParticleID();
	  
	  dExp.Fill(part.GetX()/100.0, part.GetY()/100.0, part.GetTime(), pID);	  
	  }
	}
	
	
      }
      break;
    }
  }
  
  if ( i < shIndex )
  {
    std::cout << "ERROR: The file contains " << i << " showers" << std::endl;
    return -1;
  }
  std::cout << "-----------------------------" << std::endl;
  std::ostringstream outFname;
  outFname << "dummySim_" << fname << "_" << shIndex << ".png";
  dExp.Draw(outFname.str());
}





std::string usage()
{
  std::string usg = "usage: ./energySpectra <corsika file name>";
  return usg;
}


int getOptions( int argc, char** argv, int &n )
{

  int c;
  while ( (c = getopt(argc, argv, "n:h")) != -1 )
  {
    switch(c)
    {
    case 'n':
      n = std::atoi(optarg);
      break;
      
    case 'h':
      return -2;

    default:
      return -2;
    }
  }
  return optind;
  
}
