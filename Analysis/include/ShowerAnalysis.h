#ifndef __SHOWERANALYSIS__
#define __SHOWERANALYSIS__

#include <marlin/Processor.h>

#include <EVENT/LCRunHeader.h>
#include <EVENT/LCEvent.h>

#include <IO/LCWriter.h>

#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>

#include <map>
#include <vector>
#include <string>

class ShowerAnalysisProc: public marlin::Processor
{
public:

  Processor*  newProcessor() { return new ShowerAnalysisProc; }

  ShowerAnalysisProc();

  ~ShowerAnalysisProc() {};

  void init();

  void processRunHeader( LCRunHeader * runHd);
  void processEvent(LCEvent * evtP);
  
  void end();
  
private:

  void writeHistograms();

  // ---- Input variables ----
  
  std::vector<std::string> _ecalCollections;
  std::string _logRootName;
  
  // ---- Output variables ----

  TFile* _outputFile;

  std::map<std::string, TH1F*> histograms1D;
  std::map<std::string, TH2F*> histograms2D;
  
  // ---- Counters ----

  int evtnum, runNumber = -1;

};

#endif


