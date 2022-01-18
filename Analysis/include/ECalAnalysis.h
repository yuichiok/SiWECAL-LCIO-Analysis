#ifndef __ECAL_ANALYSIS__
#define __ECAL_ANALYSIS___

#include <marlin/Processor.h>

#include <TFile.h>
#include <TH1F.h>

class ECalAnalysisProc: public marlin::Processor
{
public:

  Processor*  newProcessor() { return new ECalAnalysisProc; }

  ECalAnalysisProc();
  ~ECalAnalysisProc() {};

  void init();
  void processRunHeader( LCRunHeader * runH);
  void processEvent(LCEvent * evtP);
  void end();
  
private:

  void writeHistograms();

  std::vector<std::string> _inputCollections;

  int currRun = -1;
  
  TFile* logROOT = nullptr;

  TH1F* nHitHist;

};

#endif


