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
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <set>
#include <cstdlib>
#include <unistd.h>

std::string usage();
int getOptions(int argc, char ** argv, std::set<int> & showers);
std::string baseName(const std::string & path);
int processFile(const std::string & fname, const std::set<int> & showers);

int main(int argc, char ** argv)
{
  std::set<int> showers;
  const int nOptions = getOptions(argc, argv, showers);
  if (nOptions < 0 || argc - nOptions < 1)
  {
    std::cout << usage() << std::endl;
    return 1;
  }

  int status = 0;
  for (int f = nOptions; f < argc; ++f)
  {
    if (processFile(argv[f], showers) != 0)
      status = -1;
  }
  return status;
}

int processFile(const std::string & fname, const std::set<int> & showers)
{
  std::cout << "=============================" << std::endl;
  std::cout << "file: " << fname << std::endl;

  const bool selectAll = showers.empty();

  TH1D * hZenith = new TH1D("hZenith", "angular distribution", 360, 0, 90);

  crsRead::MCorsikaReader cr(fname, 3);
  crs::MRunHeader run;
  int i = 0;
  int nFilled = 0;

  while (cr.GetRun(run))
  {
    crs::MEventHeader shower;
    while (cr.GetShower(shower))
    {
      i++;
      if (!selectAll && !showers.count(i))
        continue;

      hZenith->Fill(shower.GetTheta() * 180.0 / M_PI);
      nFilled++;
    }
  }

  int status = 0;
  for (std::set<int>::const_iterator it = showers.begin(); it != showers.end(); ++it)
  {
    if (*it > i)
    {
      std::cout << "ERROR: shower " << *it << " not found (file contains "
                << i << " showers)" << std::endl;
      status = -1;
    }
  }

  std::cout << "histogram filled with " << nFilled << " of " << i << " showers" << std::endl;

  TCanvas * cZenith = new TCanvas("cZenith", "angular distribution", 1200, 1200 * 9 / 16);

  hZenith->SetFillColor(kBlue);
  hZenith->SetFillStyle(3005);
  hZenith->Draw();

  hZenith->GetXaxis()->SetTitle("#phi [degree]");

  cZenith->Update();

  std::ostringstream outFname;
  outFname << "./plots/angularDistributionZenith_" << baseName(fname) << ".png";
  cZenith->SaveAs(outFname.str().c_str());

  delete cZenith;
  delete hZenith;

  return status;
}

std::string baseName(const std::string & path)
{
  size_t pos = path.find_last_of('/');
  return (pos == std::string::npos) ? path : path.substr(pos + 1);
}

std::string usage()
{
  return "usage: ./angularDistributionZenith [-n <id>[,<id>...]]... <corsika file> [<corsika file> ...]\n"
         "  without -n all showers are used; -n can be repeated and/or take a comma-separated list\n"
         "  e.g. ./angularDistributionZenith -n 3,5,7 file1.dat file2.dat";
}

int getOptions(int argc, char ** argv, std::set<int> & showers)
{
  int c;
  while ((c = getopt(argc, argv, "n:h")) != -1)
  {
    switch (c)
    {
    case 'n':
    {
      std::stringstream ss(optarg);
      std::string tok;
      while (std::getline(ss, tok, ','))
      {
        if (tok.empty()) continue;
        int v = std::atoi(tok.c_str());
        if (v < 1)
        {
          std::cerr << "invalid shower id: " << tok << std::endl;
          return -2;
        }
        showers.insert(v);
      }
      break;
    }
    case 'h':
      return -2;
    default:
      return -2;
    }
  }
  return optind;
}