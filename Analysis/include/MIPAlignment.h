#ifndef __MIPALIGNMENT__
#define __MIPALIGNMENT__

#include <marlin/Processor.h>

#include <EVENT/LCRunHeader.h>
#include <EVENT/LCEvent.h>

#include <TFile.h>
#include <TH1F.h>

#include <map>
#include <vector>
#include <string>

class MIPAlignmentProc: public marlin::Processor
{
public:

  Processor*  newProcessor() { return new MIPAlignmentProc; }

  MIPAlignmentProc();

  ~MIPAlignmentProc() {};

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

  // ---- Counters ----

  int evtnum, runNumber = -1;

};

#endif


