#ifndef __ECAL_EVENTBUILDING__
#define __ECAL_EVENTBUILDING__

#include "ECal_EventScheme.h"
#include "ECal_Tools.h"

#include <vector>
#include <string>
#include <map>

#include "lcio.h"

#include <TFile.h>
#include <TTree.h>

using namespace lcio ;

struct ECalHit {

  float energy;
  float position[3];
  int I,J,K,SLB,CHP,CHN,SCA;
  
};

class EventBuilder {

public:
  
  EventBuilder();
  ~EventBuilder() {};
    
  Long64_t _maxEntries = -1;
  int _wConfig = -1;
  bool _debug = false;

  void Check();
  bool Init();
  void Mapping() {};
  void BuildEvents();
  void End();
  
  std::string _inputFileName;
  std::string _inputTreeName;
  std::string _outputFileName;
  std::string _outputColName;
  std::string _configFile;
  std::string _mappingFile;
  std::string _mappingFileCob;
  std::string _pedestalsFile;
  std::string _mipCalibrationFile;
  std::string _maskedFile;
  std::string _commissioningFolder;

private:

  // I/O variables
  
  TFile* inputFile = nullptr;
  TTree* inputTree = nullptr;
  RawECALEvent rawEvent;

  lcio::LCWriter* outputWriter = nullptr;

  ECalTools tools;
  std::map<int, Chip>* mapping;
  std::map<int, Chip>* cobMapping;
  std::map<int,Slot>* ecalConfig;
  std::map<int,std::map<int,std::map<int,std::vector<Sca>>>>* pedestalsMap;
  std::map<int,std::map<int,std::map<int,Chan>>>* calibrationMap;  
  std::map<int,std::map<int,std::map<int,bool>>>* maskedMap;  

  // Header variables that will be saved in the LCIO file
  int runNumber = -1;
  std::string detectorName = "ECAL_15Slabs_2021";
  
  // Event variables that will be saved in the LCIO file

  //Program options defaults and constants
  const std::string _inputTreeNameDefault = "siwecaldecoded";
  const std::string _outputFileNameDefault = "SiWEcal_TB2021";
  const std::string _outputColNameDefault = "ECalEvents";
  const std::string _configFileDefault = "config/ecalConfiguration_Nov2021.txt";
  const std::string _mappingFileDefault = "mapping/fev10_chip_channel_x_y_mapping.txt";
  const std::string _mappingFileCobDefault = "mapping/fev11_cob_chip_channel_x_y_mapping.txt";
  const std::string _pedestalsFileDefault = "pedestals/pedestal_PROTO15_dummy.txt";
  const std::string _mipCalibrationFileDefault = "mip_calib/MIP_PROTO15_dummy.txt";
  const std::string _maskedFileDefault = "masked/masked_PROTO15_dummy.txt";
  const std::string _commissioningFolderDefault = "";
  
  const int noisy_acquisition_start = 50;
  const int bcid_merge_delta = 3;
  const int bcid_too_many_hits = 8000;
  
  const int pedestal_min_average = 200;
  const int pedestal_min_scas = 3;
  const int pedestal_min_value = 10;

  const float mip_cutoff = 0.5;

};

#endif
