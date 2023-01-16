#ifndef __CLEANUPANDBEAMSELECTOR__
#define __CLEANUPANDBEAMSELECTOR__

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

class CleanupAndBeamSelector: public marlin::Processor
{
public:

  Processor*  newProcessor() { return new CleanupAndBeamSelector; }

  CleanupAndBeamSelector();

  ~CleanupAndBeamSelector() {};

  void init();

  void processRunHeader( LCRunHeader * runHd);
  void processEvent(LCEvent * evtP);
  
  void end();
  
private:

  void writeHistograms();

  // ---- Input variables ----
  
  std::vector<std::string> _ecalCollections;
  std::string _outFileName;
  std::string _outColName;
  std::string _logRootName;
  int _noisyChipCut, _lastScanLayer, _nLayersScanCut;
  
  // ---- Output variables ----

  LCWriter* _lcWriter;
  
  TFile* _outputFile;

  std::map<std::string, TH1F*> histograms1D;
  std::map<std::string, TH2F*> histograms2D;
  
  // ---- Counters ----

  int evtnum, runNumber = -1;
  int nBeamParticles;
  
};

#endif


