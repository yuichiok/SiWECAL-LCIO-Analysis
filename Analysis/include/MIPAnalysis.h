#ifndef __MIPANALYSIS__
#define __MIPANALYSIS__

#include <marlin/Processor.h>

#include <EVENT/LCRunHeader.h>
#include <EVENT/LCEvent.h>

#include <TPaveText.h>
#include <TGraph.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>

#include <map>
#include <vector>
#include <string>
#include <utility>
#include <fstream>

class MIPAnalysisProc: public marlin::Processor
{
public:

  Processor*  newProcessor() { return new MIPAnalysisProc; }

  MIPAnalysisProc();

  ~MIPAnalysisProc() {};

  void init();

  void processRunHeader( LCRunHeader * runHd);
  void processEvent(LCEvent * evtP);
  
  void end();
  
private:

  void computeEffAndMult(std::string type);
  void writeHistograms();

  // ---- Input variables ----
  
  std::vector<std::string> _ecalCollections;
  std::string _logRootName, _alignmentFileName, _deadCellsFileName;  
  //std::vector<float> _refAngles;
  int _scan, _MIPsPerRun;
  float kGap;

  
  std::ifstream alignmentFile, deadCellsFile;
  
  // ---- Output variables ----

  TFile* _outputFile;

  std::map<std::string, TGraph*> graphs;
  std::map<std::string, TH1F*> globalHistograms1D;
  std::map<std::string, TPaveText*> statBoxes;
  std::map<std::string, TH1F*> histograms1D;
  std::map<std::string, TH2F*> histograms2D;

  // ---- Run variables ----

  std::map<int,std::pair<int,int>> runHitEff = {};
  std::map<int,int> runMult = {};

  // ---- Detector variables ----

  std::map<int, std::pair<std::pair<float,float>,std::pair<float,float>>> alignmentMap = {};
  std::map<int, std::map<std::pair<int,int>, bool>> deadCellsMap = {};

  std::map<int,std::pair<int,int>> techEff = {};
  std::map<int,std::pair<int,int>> detHitEff = {};
  std::map<int,int> detMult = {};

  // ---- Counters ----

  int evtnum, runNumber = -1;
  int nMIPsTotal, nMIPsRun;

  // ---- Constant values ----

  const int fev13Layers[2] = { 0, 1 };
  const float cobDx = -60.0;
  const float centerDx = 3.8;
  
};

#endif


