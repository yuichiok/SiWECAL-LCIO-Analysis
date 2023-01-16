#ifndef __ECAL_EVENTDISPLAY__
#define __ECAL_EVENTDISPLAY__

#include "lcio.h"
#include <EVENT/LCEvent.h>

#include <TFile.h>

#include <vector>
#include <string>
#include <map>

using namespace lcio ;

class EventDisplay {

public:
  
  EventDisplay();
  ~EventDisplay() {};
    
  void Check();
  bool Init();
  void Process();
  void DisplayEvent(LCEvent* evt);
  void End();
  
  std::string _inputFileName;
  std::string _inputColName;
  std::string _inputEventsFileName;
  std::string _outputFileName;
  
private:

  // I/O variables

  LCReader* inputReader = nullptr;
  int _nEntries, _runNr;
  
  TFile* outputFile = nullptr;

  // Event variables

  std::vector<int> evtsMap;
  
  // Program options defaults and constants
  const std::string _inputColNameDefault = "ECalEvents";
  const std::string _inputEventsFileNameDefault = "EventsToDisplay.txt";
  const std::string _outputFileNameDefault = "ECalDisplays.root";

 
};

#endif
