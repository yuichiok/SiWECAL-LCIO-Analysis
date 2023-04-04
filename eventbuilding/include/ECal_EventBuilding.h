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

  float energy, energyE;
  float position[3];
  int I,J,K,CHP,CHN,SCA, bcid, badbcid;
};

class EventBuilder {

public:
  
  EventBuilder();
  ~EventBuilder() {};
    
  Long64_t _maxEntries = -1; // If <= 0 it is ignored
  int _runNumber = -1;
  int _inType = -1; // Types: 0 = Start from binary (full process - read binary, event building and conversion to LCIO) ; 1 = Start from RawROOT (event building and conversion to LCIO) ; 2 = Start from BuiltROOT (conversion to LCIO)
  bool _debug = false; // Turn on/off debug mode 
  bool _setup = false; // Mode to only open and display the input files (mapping, calib, ped, etc.)
  
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
  std::string _pedestalsFileHG;
  std::string _pedestalsFileLG;
  std::string _mipCalibrationFileHG;
  std::string _mipCalibrationFileLG;
  std::string _commissioningFolder;
  std::string _excMode;
  
private:

  // I/O variables
  
  TFile* inputFile = nullptr;
  TTree* inputTree = nullptr;

  Branches* inEvent = nullptr;

  // Functions to treat the event in each case

  void processRawROOTEvent();
  void processBuiltEvent();
  
  lcio::LCWriter* outputWriter = nullptr;

  ECalTools* tools;
  std::map<int, Chip>* mapping;
  std::map<int, Chip>* cobMapping;
  std::map<int,Slot>* ecalConfig;
  std::map<int,std::map<int,std::map<int,std::vector<Sca>>>> *pedestalsMapHG, *pedestalsMapLG;
  std::map<int,std::map<int,std::map<int,Chan>>> *calibrationMapHG, *calibrationMapLG;  
  
  // Header variables that will be saved in the LCIO file
  std::string detectorName = "ECAL_15Slabs_March2022";
 
  // Counters

  int evtNr, readoutNr;
  
  // Program options defaults and constants
  const std::string _inputTreeNameDefault = "ecal";
  const std::string _outputFileNameDefault = "SiWEcal_TB2022_";
  const std::string _outputColNameDefault = "ECalEvents";
  const std::string _configFileDefault = "config/March2022/ecalConfiguration_March2022_2.txt";
  const std::string _mappingFileDefault = "mapping/IJmapping_type_fev10_flipx0_flipy0.txt";
  const std::string _mappingFileCobDefault = "mapping/IJmapping_type_fev11_cob_rotate_flipx0_flipy0.txt";
  const std::string _pedestalsFileHGDefault = "pedestals/pedestal_PROTO15_HG_dummy.txt";
  const std::string _pedestalsFileLGDefault = "pedestals/pedestal_PROTO15_LG_dummy.txt";
  const std::string _mipCalibrationFileHGDefault = "mip_calib/MIP_PROTO15_HG_dummy.txt";
  const std::string _mipCalibrationFileLGDefault = "mip_calib/MIP_PROTO15_LG_dummy.txt";
  const std::string _commissioningFolderDefault = "";
  const std::string _executionModeDefault = "default";
  
  //const int noisy_acquisition_start = 50;
  const int bcid_merge_delta = 3;
  const int bcid_too_many_hits = 8000;

  const float mip_cutoff = 0.5;

 
};

#endif
