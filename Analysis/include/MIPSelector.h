#ifndef __MIPSELECTOR__
#define __MIPSELECTOR__

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

class MIPSelectorProc: public marlin::Processor
{
public:

  Processor*  newProcessor() { return new MIPSelectorProc; }

  MIPSelectorProc();

  ~MIPSelectorProc() {};

  void init();

  void processRunHeader( LCRunHeader * runHd);
  void processEvent(LCEvent * evtP);
  
  void end();
  
private:

  void writeHistograms();

  // ---- Input variables ----
  
  std::vector<std::string> _ecalCollections;
  std::string _outFileNameMIPs;
  std::string _outColNameMIPs;
  std::string _outFileNameShowers;
  std::string _outColNameShowers;
  std::string _logRootName;
  float _densityCutMIPs;//, _densityCutDiscarded;
  int _secondMaxCutMIPs;//, _secondMaxCutDiscarded;
  
  // ---- Output variables ----

  LCWriter *_lcWriterMIPs, *_lcWriterShowers;

  TFile* _outputFile;

  std::map<std::string, TH1F*> histograms1D;
  std::map<std::string, TH2F*> histograms2D;
  
  // ---- Counters ----

  int evtnum, runNumber = -1;
  int nDiscarded, nMIPs, nShowers;

};

#endif


