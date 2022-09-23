#ifndef __ECAL_TOOLS__
#define __ECAL_TOOLS__

#include <string>
#include <utility>
#include <vector>
#include <map>

#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"

struct Slot{

  int layer, slab, slabID, slabAdd;
  float W, X0, X0Acc, deltaX;
  std::string glissiere, ASU, wafer;
  
};


struct Chip{

  std::map<int, std::pair<int, int>> chanMapping = {};

};


struct Sca{
  
  float pedestal, error, noiseInCoherent, noiseCoherent1, noiseCoherent2;

};

struct Chan{

  int nentries;
  float mpv, empv, widthmpv, chi2ndf;
  
};

class ECalTools {

public:
  
  ECalTools(bool createLogFile = false, std::string logFileName = "LogROOT_ECalEventBuilding.root", bool debug = false);
  ~ECalTools();

  bool hasLogFile;
  bool debugMode = false;
  
  // --- Tools to read the different files
  std::map<int,Slot>* ReadConfigFile(std::string configFileName);
  std::map<int,Chip>* ReadMappingFile(std::string mapFileName);
  std::map<int,std::map<int,std::map<int,std::vector<Sca>>>>* ReadPedestalsFile(std::string pedestalsFileName);
  std::map<int,std::map<int,std::map<int,Chan>>>* ReadCalibrationFile(std::string calibrationFileName);
  std::map<int,std::map<int, std::map<int,bool>>>* ReadMaskedFile(std::string maskedFileName);
  
  // --- Tools to display the different maps
  void DisplayMapping(std::map<int,Chip>* mapping);
  void DisplayConfiguration(std::map<int,Slot>* config);
  void DisplayPedestals(std::map<int,std::map<int,std::map<int,std::vector<Sca>>>>* pedestals);
  void DisplayCalibration(std::map<int,std::map<int,std::map<int,Chan>>>* calibration);
  void DisplayMasked(std::map<int,std::map<int, std::map<int,bool>>>* masked);

  // Log file and histograms

  void InitFileAndHistograms(std::string logFileName);
  void WriteAndClose();
  
  TFile* logFile;
  std::map<std::string,TH1F*> histograms1D = {};
  std::map<std::string,TH2F*> histograms2D = {};
  bool readoutDone = false;
  
};

#endif
