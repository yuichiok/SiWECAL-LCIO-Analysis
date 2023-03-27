#include "ECal_EventBuilding.h"

#include <IMPL/LCRunHeaderImpl.h>
#include <IMPL/LCEventImpl.h>
#include <IMPL/LCCollectionVec.h>
#include <IMPL/CalorimeterHitImpl.h>
#include <UTIL/CellIDEncoder.h>

#include <iostream>

EventBuilder::EventBuilder() {

  // --- Initializing input to the default values
  _inputTreeName = _inputTreeNameDefault;
  _outputFileName = _outputFileNameDefault;
  _outputColName = _outputColNameDefault;
  _configFile = _configFileDefault;
  _mappingFile = _mappingFileDefault;
  _mappingFileCob = _mappingFileCobDefault;
  _pedestalsFile = _pedestalsFileDefault;
  _mipCalibrationFile = _mipCalibrationFileDefault;
  _maskedFile = _maskedFileDefault;
  _commissioningFolder = _commissioningFolderDefault;

}


// Check if the arguments are correct
void EventBuilder::Check() {

  //Input Tree Name
  if(_inputTreeName == "") {
    std::cout << "Input TTree name empty. Using default name: " << _inputTreeNameDefault + std::to_string(_runNumber) << std::endl;
    _inputTreeName = _inputTreeNameDefault + std::to_string(_runNumber);
  }

  //Output File Name
  if(_outputFileName == "") {
    std::cout << "Output file name empty. Using default name: " << _outputFileNameDefault << std::endl;
    _outputFileName = _outputFileNameDefault;
  }
  
  //Output Collection Name
  if(_outputColName == "") {
    std::cout << "Output collection name empty. Using default name: " << _outputColNameDefault << std::endl;
    _outputColName = _outputColNameDefault;  
  }

  //Configuration File Name
  if(_configFile == "") {
    std::cout << "Configuration file name empty. Using default name: " << _configFileDefault << std::endl;
    _configFile = _configFileDefault;  
  }

  //Mapping File Name
  if(_mappingFile == "") {
    std::cout << "Mapping file name empty. Using default name: " << _mappingFileDefault << std::endl;
    _mappingFile = _mappingFileDefault;  
  }

  //Cob Mapping File Name
  if(_mappingFileCob == "") {
    std::cout << "Cob mapping file name empty. Using default name: " << _mappingFileCobDefault << std::endl;
    _mappingFileCob = _mappingFileCobDefault;  
  }
  
  //Pedestals File Name
  if(_pedestalsFile == "") {
    std::cout << "Pedestals file name empty. Using default name: " << _pedestalsFileDefault << std::endl;
    _pedestalsFile = _pedestalsFileDefault;  
  }
  
  //MIP Calibration File Name
  if(_mipCalibrationFile == "") {
    std::cout << "MIP calibration file name empty. Using default name: " << _mipCalibrationFileDefault << std::endl;
    _mipCalibrationFile = _mipCalibrationFileDefault;  
  }
  
  //Masked File Name
  if(_maskedFile == "") {
    std::cout << "Masked file name empty. Using default name: " << _maskedFileDefault << std::endl;
    _maskedFile = _maskedFileDefault;  
  }

  //Checking run mode
  
  if(_excMode == "setup")  _setup = true;
  else if(_excMode == "debug") _debug = true;
  else if(_excMode != "default") { 
    std::cout << "Empty or wrong Run Mode: " << _maskedFileDefault << ". Setting to default" << std::endl;
    _excMode = "default";
  }

  
  
}


bool EventBuilder::Init() {
  
  if(_setup) tools = new ECalTools();
  else tools = new ECalTools(true, "LogROOT_ECalEventBuilding_" + std::to_string(_runNumber) + ".root",_debug);
  
  // --- Reading the txt files
  mapping = tools->ReadMappingFile(_commissioningFolder + _mappingFile);
  if(mapping == nullptr) {
    std::cout << "Could not open mapping file: " << _commissioningFolder + _mappingFile << std::endl;
    return false;
  }
  
  cobMapping = tools->ReadMappingFile(_commissioningFolder + _mappingFileCob);
  if(cobMapping == nullptr) {
    std::cout << "Could not open cob mapping file: " << _commissioningFolder + _mappingFileCob << std::endl;
    return false;
  }

  ecalConfig = tools->ReadConfigFile(_commissioningFolder + _configFile);
  if(ecalConfig == nullptr) {
    std::cout << "Could not open the configuration file: " << _commissioningFolder + _configFile << std::endl;
    return false;
  }

  pedestalsMap = tools->ReadPedestalsFile(_commissioningFolder + _pedestalsFile);
  if(pedestalsMap == nullptr) {
    std::cout << "Could not open the pedestals file: " << _commissioningFolder + _pedestalsFile << std::endl;
    return false;
  }

  calibrationMap = tools->ReadCalibrationFile(_commissioningFolder + _mipCalibrationFile);
  if(calibrationMap == nullptr) {
    std::cout << "Could not open the calibration file: " << _commissioningFolder + _mipCalibrationFile << std::endl;
    return false;
  }

  maskedMap = tools->ReadMaskedFile(_commissioningFolder + _maskedFile);
  if(maskedMap == nullptr) {
    std::cout << "Could not open the masked file: " << _commissioningFolder + _maskedFile << std::endl;
    return false;
  }

  if(_debug || _setup) {
    std::cout << "--------- Mapping File ---------" << std::endl; 
    tools->DisplayMapping(mapping);
    std::cout << "--------- Cob mapping File ---------" << std::endl; 
    tools->DisplayMapping(cobMapping);
    std::cout << "--------- Configuration File ---------" << std::endl;
    tools->DisplayConfiguration(ecalConfig);
    std::cout << "--------- Pedestals File ---------" << std::endl;
    tools->DisplayPedestals(pedestalsMap);
    std::cout << "--------- Calibration File ---------" << std::endl;
    tools->DisplayCalibration(calibrationMap);
    std::cout << "--------- Masked File ---------" << std::endl;
    tools->DisplayMasked(maskedMap);
  }

  if(_setup) return false;

  // --- Opening input TFile
  inputFile = new TFile(_inputFileName.c_str(), "READ", "InputFile");
  if(!inputFile->IsOpen()) {
    std::cout << "Could not open the input file: " << _inputFileName << std::endl;
    return false;
  }

  // --- Reading the input TTree
  inputTree = inputFile->Get<TTree>(_inputTreeName.c_str());
  if(!inputTree) {
    std::cout << "TTree with name: " << _inputTreeName << " not found in file: " << _inputFileName << std::endl;
    std::cout << "Found items in file: ";
    // for(TIter keyIt = inputFile->GetListOfKeys()->begin(); keyIt != inputFile->GetListOfKeys()->end(); keyIt.Next() ) { std::cout << (dynamic_cast<TKey*>(*keyIt))->GetName() << " "; } 
    std::cout << std::endl;
    return false;
  }

  // --- Setting up the branches from the RawECalEvent scheme
  if(_debug) std::cout << "Setting up branches from input TTree(" << _inputTreeName << ")" << std::endl;
  for(auto branchIt = rawEvent.branchesMap.begin(); branchIt != rawEvent.branchesMap.end(); branchIt++) {
    if(_debug) std::cout << "\t |->" << branchIt->first.c_str() << std::endl; 
    inputTree->SetBranchAddress(branchIt->first.c_str(),branchIt->second);
  }

  // --- Adjusting the maximun number of entries if necessary
  if(_maxEntries > inputTree->GetEntries() || _maxEntries <= 0) _maxEntries = inputTree->GetEntries();
  
  // --- Creating the output File
  outputWriter = LCFactory::getInstance()->createLCWriter();
  outputWriter->setCompressionLevel(0);
  outputWriter->open(_outputFileName.c_str(), LCIO::WRITE_NEW);

  LCRunHeaderImpl* runHeader = new LCRunHeaderImpl();
  runHeader->setRunNumber(_runNumber);
  runHeader->setDetectorName(detectorName);

  outputWriter->writeRunHeader(runHeader);
  delete runHeader;
  
  // --- Debug prints
  if(_debug) {
    std::cout << "Input TTree(" << _inputTreeName << ") found with " << inputTree->GetEntries() << " entries. ";
    std::cout << "Processing a total of " << _maxEntries << "." << std::endl;
  }

  evtNr = readoutNr = 1;

  
  return true;
}


void EventBuilder::BuildEvents() {

  if(_debug) std::cout << "Starting the loop over the input" << std::endl;

  for(Long64_t iEntry = 0; iEntry < _maxEntries; iEntry++) {
    inputTree->GetEntry(iEntry);
    
    std::map<int, std::vector<ECalHit>> bcidMap = {};
    int nHitsReadout = 0;
    
    for(int iSlab = 0; iSlab < SLBDEPTH; iSlab++) {
      if(rawEvent.slot[iSlab] == -1) continue;
      for(int iChip = 0; iChip < NCHIP; iChip++) {
	if(rawEvent.chipid[iSlab][iChip] == -1) continue;
	for(int iMem = 0; iMem < MEMDEPTH; iMem++){
	  //if(rawEvent.corrected_bcid[iSlab][iChip][iMem] < noisy_acquisition_start || rawEvent.badbcid[iSlab][iChip][iMem] != 0) continue;
	  if(rawEvent.badbcid[iSlab][iChip][iMem] != 0) continue;
	  
	  for(int iChan = 0; iChan < NCHANNELS; iChan++) {
        
	    if(rawEvent.gain_hit_high[iSlab][iChip][iMem][iChan] <= 0) continue;

            if((*maskedMap)[iSlab][iChip][iChan]) continue;
	    
	    if(_debug) std::cout << "Pedestal: " << (*pedestalsMap)[iSlab][iChip][iMem][iChan].pedestal << std::endl;
	    float hg =  (float)rawEvent.highGain[iSlab][iChip][iMem][iChan] - (*pedestalsMap)[iSlab][iChip][iMem][iChan].pedestal;

	    if(_debug) std::cout << "MIP Calibration: " << (*calibrationMap)[iSlab][iChip][iChan].mpv << std::endl;	    
	    if((*calibrationMap)[iSlab][iChip][iChan].mpv < mip_cutoff || (*calibrationMap)[iSlab][iChip][iChan].empv < 0) continue;

	    if(!tools->readoutDone) {
	      tools->histograms1D.at("Readout")->Fill(rawEvent.bcid[iSlab][iChip][iMem]);
	      tools->histograms1D.at("CorrectedReadout")->Fill(rawEvent.corrected_bcid[iSlab][iChip][iMem]);
	    }

	    if(_debug) std::cout << "Creating new ECal hit" << std::endl;	    
        
	    ECalHit newHit;
	    newHit.energy = hg/(*calibrationMap)[iSlab][iChip][iChan].mpv;

	    
	    newHit.position[0] = (*mapping)[iChip].chanMapping[iChan].first;
	    newHit.position[1] = (*mapping)[iChip].chanMapping[iChan].second;
	    newHit.position[2] = iSlab*15;

	    if(newHit.position[0] < 0) newHit.I = (int)((newHit.position[0] + 91.7)/5.5); 
	    else newHit.I = (int)(newHit.position[0]/5.5) + 16; 
	    if(newHit.position[1] < 0) newHit.J = (int)((newHit.position[1] + 91.7)/5.5);
	    else newHit.J = (int)(newHit.position[1]/5.5) + 16;
	    newHit.K = iSlab;
	    newHit.CHP = iChip;
	    newHit.CHN = iChan;
	    newHit.SCA = iMem;
	    //newHit.prevBCID = prevBCID;
	    //if(prevBCID > rawEvent.bcid[iSlab][iChip][iMem]) newHit.needsBCIDCorrection = true;

	    tools->histograms1D.at("CorrectedBCID")->Fill(rawEvent.corrected_bcid[iSlab][iChip][iMem]);
	    
	    bcidMap[rawEvent.corrected_bcid[iSlab][iChip][iMem]].push_back(newHit);
	    
	  } // End of Channel loop

	  nHitsReadout++;
	  //prevBCID = rawEvent.bcid[iSlab][iChip][iMem];
	} // End of Sca loop
      } // End of Chip loop
    } // End of Layer loop

    tools->readoutDone = true;
    tools->histograms1D.at("NHitsReadout")->Fill(nHitsReadout);

    if(_debug) std::cout << "Starting BCID Merging" << std::endl;	    
    
    auto bcidIt = bcidMap.begin();
    while(bcidIt != bcidMap.end()) {
      int totalHits = bcidIt->second.size();
      int prevScanBCID = bcidIt->first;
      auto scanIt = bcidIt;
      auto maxIt = bcidIt;
      scanIt++;
      while(scanIt != bcidMap.end()) {
	if(scanIt->first - prevScanBCID > bcid_merge_delta) break;
	if(maxIt->second.size() < scanIt->second.size()) maxIt = scanIt;
	prevScanBCID = scanIt->first;
	totalHits += scanIt->second.size();
	scanIt++;
      }

      if(totalHits >= bcid_too_many_hits) {
	bcidIt = scanIt;
	continue;
      }

      LCEventImpl* evt = new LCEventImpl();
      evt->setEventNumber(evtNr);
      evt->setTimeStamp(maxIt->first);

      LCCollectionVec* outCol = new LCCollectionVec(LCIO::CALORIMETERHIT);
      outCol->setFlag(0|(1 << LCIO::RCHBIT_LONG));

      CellIDEncoder<CalorimeterHitImpl> cd("I:5,J:5,K:4,CHP:4,CHN:6,SCA:4,",outCol);
      
      std::map<int,std::map<int,int>> hitMap = {};
      
      float sumEnergy = 0.;

      int nHits = 0;
      while(bcidIt != scanIt) {
	for(auto hitIt = bcidIt->second.begin(); hitIt != bcidIt->second.end(); hitIt++) {

	  if(_debug) std::cout << "Creating new CalorimeterHit" << std::endl; 

	  CalorimeterHitImpl* lcioHit = new CalorimeterHitImpl();
	  lcioHit->setEnergy(hitIt->energy);
	  sumEnergy += hitIt->energy;

	  lcioHit->setPosition(hitIt->position);
	  lcioHit->setTime(bcidIt->first);

	  cd["CHP"] = hitIt->CHP;
	  cd["CHN"] = hitIt->CHN;
	  cd["SCA"] = hitIt->SCA;
	  cd["I"] = hitIt->I;
	  cd["J"] = hitIt->J;
	  cd["K"] = hitIt->K;
	  cd.setCellID(lcioHit);

	  hitMap[hitIt->K][hitIt->CHP]++;
	  outCol->addElement(lcioHit);
	  nHits++;
	  
	  tools->histograms1D.at("I")->Fill(hitIt->I);
	  tools->histograms1D.at("J")->Fill(hitIt->J);
	  tools->histograms1D.at("K")->Fill(hitIt->K);

	  /*if(layers.find(hitIt->K) == layers.end()) {
	    if(_debug) std::cout << "Found new layer: " << hitIt->K << std::endl; 
	    nLayers++;
	    nChips++;
	  }
	  else if(chips.at(hitIt->K).find(hitIt->CHP) == chips.at(hitIt->K).end()) {
	    if(_debug) std::cout << "Found new Chip: " << hitIt->CHP << std::endl;
	    nChips++;
	    nScas++;
	    }*/

	}
	bcidIt++;
      } // End of scan loop
      
      int nLayers, nChips;
      nLayers = nChips = 0;

      for(auto layerIt = hitMap.begin(); layerIt != hitMap.end(); layerIt++) {
	int nHitsInLayer = 0;
	for(auto chipIt = layerIt->second.begin(); chipIt != layerIt->second.end(); chipIt++) {
	  tools->histograms1D.at("NHitsChip")->Fill(chipIt->second);
	  nHitsInLayer += chipIt->second;
	  nChips++;
	}
	tools->histograms1D.at("NHitsLayer")->Fill(nHitsInLayer);
	nLayers++;
      }
      
      tools->histograms1D.at("NHits")->Fill(nHits);
      tools->histograms1D.at("NChips")->Fill(nChips);
      tools->histograms1D.at("NLayers")->Fill(nLayers);
      
      outCol->parameters().setValue("NLayers", nLayers);
      outCol->parameters().setValue("NChips", nChips);
      outCol->parameters().setValue("SumEnergy", sumEnergy);

      evt->addCollection(outCol, _outputColName.c_str());
      outputWriter->writeEvent(evt);
      delete evt;

      evtNr++;
    } // End of loop over BICD Map
    
  }
  
}


void EventBuilder::End() {
  
  //Closing files and freeing memory

  delete tools;
  
  if(mapping != nullptr) delete mapping;
  if(cobMapping != nullptr) delete cobMapping;
  if(ecalConfig != nullptr) delete ecalConfig;
  if(pedestalsMap != nullptr) delete pedestalsMap;
  if(calibrationMap != nullptr) delete calibrationMap;
  if(maskedMap != nullptr) delete maskedMap;
  
  if(outputWriter != nullptr) {
    outputWriter->close();
    delete outputWriter;
  }

  if(inputTree != nullptr) delete inputTree;

  if(inputFile != nullptr) {
    inputFile->Close();
    delete inputFile;
  }
  
}
