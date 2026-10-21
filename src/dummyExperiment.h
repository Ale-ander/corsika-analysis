#ifndef __DUMMY_EXPERIMENT_H__
#define __DUMMY_EXPERIMENT_H__

#include <string>

class dummyExperiment
{
public:
  dummyExperiment(double experimentSize,
		  double tankSize,
		  double tankSpacing);
  
  ~dummyExperiment();
  void Fill( double x, double y, double time, int pID);
  void Draw(std::string outfile);
  void Reset();
  void SetShowerInfo(double shEnergy, double obsLev, int shID);
  void SetMinimumSignal(const double triggerS) { fTriggerS = triggerS; }
  void SetMuonWeight(const double w) { fMuonWeight = w; }
  void SetEmWeight(const double w) { fEmWeight = w; }
  
private:
  int GetCol( double signal, double firstT, double lastT );
  void findFirst();
  void findLast();
  void findHigher();
  double fEsize;
  double fTsize;
  double fTspacing;
  int fNtank;
  double* fSignals;
  double* fTimes;
  double fFirstTime;
  double fLastTime;
  double fMaxSign;

  double fEnergyGen;
  double fObsLev;
  int fShowerID;

  double fTriggerS;
  double fMuonWeight;
  double fEmWeight;
};


#endif
