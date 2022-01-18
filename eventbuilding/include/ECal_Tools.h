#ifndef __ECAL_TOOLS__
#define __ECAL_TOOLS__

#include <string>
#include <utility>
#include <vector>
#include <map>

struct Slot{

  int layer, slab, slabID, slabAdd;
  float W, X0, X0Acc;
  std::string glissiere, ASU, wafer;
  
};


struct Chip{

  float X0, Y0;
  std::map<int, std::pair<float, float>> chanMapping = {};

};


struct Sca{
  
  float pedestal, error, width;

};

struct Chan{

  int nentries;
  float mpv, empv, widthmpv, chi2ndf;
  
};

class ECalTools {

public:
  
  ECalTools() {};
  ~ECalTools() {};

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
  
};

#endif
