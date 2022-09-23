#include "ECal_EventBuilding.h"

#include <IMPL/LCRunHeaderImpl.h>
#include <IMPL/LCEventImpl.h>
#include <IMPL/LCCollectionVec.h>
#include <IMPL/CalorimeterHitImpl.h>
#include <UTIL/CellIDEncoder.h>

#include <iostream>
#include <cmath>

EventBuilder::EventBuilder() {

  // --- Initializing input to the default values
  _inputTreeName = _inputTreeNameDefault;
  _outputFileName = _outputFileNameDefault;
  _outputColName = _outputColNameDefault;
  _configFile = _configFileDefault;
  _mappingFile = _mappingFileDefault;
  _mappingFileCob = _mappingFileCobDefault;
  _pedestalsFileHG = _pedestalsFileHGDefault;
  _pedestalsFileLG = _pedestalsFileLGDefault;
  _mipCalibrationFileHG = _mipCalibrationFileHGDefault;
  _mipCalibrationFileLG = _mipCalibrationFileLGDefault;
  _commissioningFolder = _commissioningFolderDefault;
  _excMode = _executionModeDefault;
  
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

  //Check input type
  if(_inType == -1) {
    std::cout << "Input type not especified. Using default mode of binary building" << std::endl;
    _inType = 0;
  }
  
  //Configuration File Name
  if(_configFile == "") {
    std::cout << "Configuration file name empty. Using default name: " << _configFileDefault << std::endl;
    _configFile = _configFileDefault;  
  }

  //Mapping File Name
  if(_mappingFile == "" && _inType != 2) {
    std::cout << "Mapping file name empty. Using default name: " << _mappingFileDefault << std::endl;
    _mappingFile = _mappingFileDefault;  
  }

  //Cob Mapping File Name
  if(_mappingFileCob == "" && _inType != 2) {
    std::cout << "Cob mapping file name empty. Using default name: " << _mappingFileCobDefault << std::endl;
    _mappingFileCob = _mappingFileCobDefault;  
  }
  
  //Pedestals File Name HG
  if(_pedestalsFileHG == "" && _inType != 2) {
    std::cout << "HG Pedestals file name empty. Using default name: " << _pedestalsFileHGDefault << std::endl;
    _pedestalsFileHG = _pedestalsFileHGDefault;  
  }

  //Pedestals File Name LG
  if(_pedestalsFileLG == "" && _inType != 2) {
    std::cout << "LG Pedestals file name empty. Using default name: " << _pedestalsFileLGDefault << std::endl;
    _pedestalsFileLG = _pedestalsFileLGDefault;  
  }
  
  //MIP Calibration File Name HG
  if(_mipCalibrationFileHG == "" && _inType != 2) {
    std::cout << "HG MIP calibration file name empty. Using default name: " << _mipCalibrationFileHGDefault << std::endl;
    _mipCalibrationFileHG = _mipCalibrationFileHGDefault;  
  }
  
  //MIP Calibration File Name LG
  if(_mipCalibrationFileLG == "" && _inType != 2) {
    std::cout << "LG MIP calibration file name empty. Using default name: " << _mipCalibrationFileLGDefault << std::endl;
    _mipCalibrationFileLG = _mipCalibrationFileLGDefault;  
  }
  
  //Checking run mode
  
  if(_excMode == "setup")  _setup = true;
  else if(_excMode == "debug") _debug = true;
  else if(_excMode != "default") { 
    std::cout << "Empty or wrong Run Mode: " << _excMode << ". Setting to default" << std::endl;
    _excMode = "default";
  }
  
}


bool EventBuilder::Init() {
  
    if(_setup) tools = new ECalTools();
    else tools = new ECalTools(true, "LogROOT_ECalEventBuilding_" + std::to_string(_runNumber) + ".root",_debug);

    ecalConfig = tools->ReadConfigFile(_commissioningFolder + _configFile);
    if(ecalConfig == nullptr) {
      std::cout << "Could not open the configuration file: " << _commissioningFolder + _configFile << std::endl;
      return false;
    }
  
  if(_inType != 2) {
    
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
    
    pedestalsMapHG = tools->ReadPedestalsFile(_commissioningFolder + _pedestalsFileHG);
    if(pedestalsMapHG == nullptr) {
      std::cout << "Could not open the pedestals file HG: " << _commissioningFolder + _pedestalsFileHG << std::endl;
      return false;
    }

    pedestalsMapLG = tools->ReadPedestalsFile(_commissioningFolder + _pedestalsFileLG);
    if(pedestalsMapLG == nullptr) {
      std::cout << "Could not open the pedestals file LG: " << _commissioningFolder + _pedestalsFileLG << std::endl;
      return false;
    }
    
    calibrationMapHG = tools->ReadCalibrationFile(_commissioningFolder + _mipCalibrationFileHG);
    if(calibrationMapHG == nullptr) {
      std::cout << "Could not open the calibration file HG: " << _commissioningFolder + _mipCalibrationFileHG << std::endl;
      return false;
    }

    calibrationMapLG = tools->ReadCalibrationFile(_commissioningFolder + _mipCalibrationFileLG);
    if(calibrationMapLG == nullptr) {
      std::cout << "Could not open the calibration file LG: " << _commissioningFolder + _mipCalibrationFileLG << std::endl;
      return false;
    }
    
  }
  
  if(_debug || _setup) {
    std::cout << "--------- Configuration File ---------" << std::endl;
    tools->DisplayConfiguration(ecalConfig);
    if(_inType != 2) {
      std::cout << "--------- Mapping File ---------" << std::endl; 
      tools->DisplayMapping(mapping);
      std::cout << "--------- Cob mapping File ---------" << std::endl; 
      tools->DisplayMapping(cobMapping);
      std::cout << "--------- Pedestals File HG ---------" << std::endl;
      tools->DisplayPedestals(pedestalsMapHG);
      std::cout << "--------- Pedestals File LG ---------" << std::endl;
      tools->DisplayPedestals(pedestalsMapLG);
      std::cout << "--------- Calibration File HG ---------" << std::endl;
      tools->DisplayCalibration(calibrationMapHG);
      std::cout << "--------- Calibration File LG ---------" << std::endl;
      tools->DisplayCalibration(calibrationMapLG);
    }
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
    for(TIter keyIt = inputFile->GetListOfKeys()->begin(); keyIt != inputFile->GetListOfKeys()->end(); keyIt.Next() ) { std::cout << (dynamic_cast<TKey*>(*keyIt))->GetName() << " "; } 
    std::cout << std::endl;
    return false;
  }

  switch(_inType) {
  case 0:
    break;
  case 1:
    inEvent = new RawECALEvent(); break;
  case 2:
    inEvent = new BuiltROOTEvent(); break;
  };
  
  // --- Setting up the branches from the corresponding scheme
  if(_debug) std::cout << "Setting up branches from input TTree(" << _inputTreeName << ")" << std::endl;
  for(auto branchIt = inEvent->branchesMap.begin(); branchIt != inEvent->branchesMap.end(); branchIt++) {
    if(_debug) std::cout << "\t |->" << branchIt->first.c_str() << std::endl; 
    inputTree->SetBranchAddress(branchIt->first.c_str(),branchIt->second);
  }

  // --- Adjusting the maximun number of entries if necessary
  if(_maxEntries > inputTree->GetEntries() || _maxEntries <= 0) _maxEntries = inputTree->GetEntries();
  
  // --- Creating the output File
  outputWriter = LCFactory::getInstance()->createLCWriter();
  outputWriter->setCompressionLevel(0);
  outputWriter->open((_outputFileName + std::to_string(_runNumber)).c_str(), LCIO::WRITE_NEW);

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

    switch(_inType) {
    case 0:
      break; // TO DO
    case 1:
      processRawROOTEvent();
      break;
    case 2:
      processBuiltEvent();
      break;
    };
    
  }
  
}


void EventBuilder::End() {
  
  //Closing files and freeing memory

  delete tools;
  
  if(mapping != nullptr) delete mapping;
  if(cobMapping != nullptr) delete cobMapping;
  if(ecalConfig != nullptr) delete ecalConfig;
  if(pedestalsMapHG != nullptr) delete pedestalsMapHG;
  if(pedestalsMapLG != nullptr) delete pedestalsMapLG;
  if(calibrationMapHG != nullptr) delete calibrationMapHG;
  if(calibrationMapLG != nullptr) delete calibrationMapLG;
 
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

/*
  ###### Note about the removed channels ######
  These are the conditions in which a hit is removed:

  -> Wrong ids: slboard_id < 0 || chipid < 0
  -> badbcid != 0
  -> Not a hit: HitBit_LOW && HitBit_High == 0
  -> Pedestals not computed or fitting error: pedestal <= 0 || error <= 0
  -> Calibration below threshold or fitting error: mpv < mip_cutoff || epmv < 0 || empv <= 0
  -> Negative values of energy: EnergyHG < 0 || (EnergyHG > 80 && EnergyLG < 0)

*/

void EventBuilder::processRawROOTEvent() {

    RawECALEvent* rawEvent = (RawECALEvent*)inEvent;
  
    std::map<int, std::vector<ECalHit>> bcidMap = {};
    int nHitsReadout = 0;

    for(int iSlab = 0; iSlab < SLBDEPTH; iSlab++) {
      if(rawEvent->slboard_id[iSlab] < 0) {
	if(_debug) std::cout << "Ignoring slab " << iSlab << ". Slot: " << rawEvent->slot[iSlab] << " ID: " << rawEvent->slboard_id[iSlab] << std::endl;
	continue;
      };
      for(int iChip = 0; iChip < NCHIP; iChip++) {
	if(rawEvent->chipid[iSlab][iChip] < 0) {
	  if(_debug) std::cout << "Ignoring chip " << iSlab << " " << iChip << ". ID: " << rawEvent->chipid[iSlab][iChip] << std::endl;
	  continue;
	}
	
        for(int iMem = 0; iMem < MEMDEPTH; iMem++){
	  if(rawEvent->badbcid[iSlab][iChip][iMem] != 0) {
	    if(_debug) std::cout << "Sca with BadBCID " << iSlab << " " << iChip << " " << iMem << " " << rawEvent->badbcid[iSlab][iChip][iMem] << std::endl;
	    continue;
	  }

	  if(rawEvent->corrected_bcid[iSlab][iChip][iMem] < 20) continue;
	  
	  for(int iChan = 0; iChan < NCHANNELS; iChan++) {
	    
	    if(rawEvent->hitbit_low[iSlab][iChip][iMem][iChan] == 0 || rawEvent->hitbit_high[iSlab][iChip][iMem][iChan] == 0) continue;
	    
	    if((*pedestalsMapHG)[iSlab][iChip][iChan][iMem].pedestal <= 0 || (*pedestalsMapHG)[iSlab][iChip][iChan][iMem].error <= 0 /*|| (*pedestalsMapLG)[iSlab][iChip][iChan][iMem].pedestal <= 0 || (*pedestalsMapLG)[iSlab][iChip][iChan][iMem].error <= 0*/) {
	      if(_debug) std::cout << "Removing Chan " << iSlab << " " << iChip << " " << iMem << " " << iChan << ". PedestalHG: " << (*pedestalsMapHG)[iSlab][iChip][iChan][iMem].pedestal << " +- " << (*pedestalsMapHG)[iSlab][iChip][iChan][iMem].error << " PedestalLG" << (*pedestalsMapLG)[iSlab][iChip][iChan][iMem].pedestal << " +- " << (*pedestalsMapLG)[iSlab][iChip][iChan][iMem].error << std::endl;
	      continue;
	    }
	    
	    if((*calibrationMapHG)[iSlab][iChip][iChan].mpv <= 0 || (*calibrationMapHG)[iSlab][iChip][iChan].empv <= 0 /*|| (*calibrationMapLG)[iSlab][iChip][iChan].mpv <= 0 || (*calibrationMapLG)[iSlab][iChip][iChan].empv <= 0*/) {
	      if(_debug) std::cout << "Removing Chan " << iSlab << " " << iChip << " " << iMem << " " << iChan << ". CalibrationHG: " << (*calibrationMapHG)[iSlab][iChip][iChan].mpv << " +- " << (*calibrationMapHG)[iSlab][iChip][iChan].empv << " CalibrationLG" << (*calibrationMapLG)[iSlab][iChip][iChan].mpv << " +- " << (*calibrationMapLG)[iSlab][iChip][iChan].empv << std::endl;
	      continue;
	    }

	    // Fill all hits in a readout for visualization
	    if(!tools->readoutDone) {
	      tools->histograms1D.at("Readout")->Fill(rawEvent->bcid[iSlab][iChip][iMem]);
	      tools->histograms1D.at("CorrectedReadout")->Fill(rawEvent->corrected_bcid[iSlab][iChip][iMem]);
	    }
        
	    if(_debug) std::cout << "Computing hit energy" << std::endl;
	    
	    float adcErr = 1.f; // Check this with Adrian	    

	    float hg =  (float)rawEvent->adc_high[iSlab][iChip][iMem][iChan] - (*pedestalsMapHG)[iSlab][iChip][iChan][iMem].pedestal;
	    float hgErrSq = pow(adcErr,2) + pow((*pedestalsMapHG)[iSlab][iChip][iChan][iMem].error,2); 
	    
	    float lg =  (float)rawEvent->adc_low[iSlab][iChip][iMem][iChan] - (*pedestalsMapLG)[iSlab][iChip][iChan][iMem].pedestal;
	    float lgErrSq = pow(adcErr,2) + pow((*pedestalsMapLG)[iSlab][iChip][iChan][iMem].error,2);
	    
	    float energyHG = hg/(*calibrationMapHG)[iSlab][iChip][iChan].mpv;
	    float energyHGErr = energyHG*sqrt(hgErrSq/pow(hg,2) + pow((*calibrationMapHG)[iSlab][iChip][iChan].empv,2)/pow((*calibrationMapHG)[iSlab][iChip][iChan].mpv,2));
	    
	    float energyLG = lg/(*calibrationMapLG)[iSlab][iChip][iChan].mpv;
	    float energyLGErr = energyLG*sqrt(lgErrSq/pow(lg,2) + pow((*calibrationMapLG)[iSlab][iChip][iChan].empv,2)/pow((*calibrationMapLG)[iSlab][iChip][iChan].mpv,2));
	    
	    float energy = energyHG;
	    float energyE = energyHGErr;
	    /*if(energy > 80) {
	      energy = energyLG;
	      energyE = energyLGErr;
	      }*/
	    
	    if(energy <= mip_cutoff) continue;
	    
	    if(_debug) std::cout << "Creating new ECal hit" << std::endl;	    
        
	    ECalHit newHit;
	    newHit.energy = energy;
	    newHit.energyE = energyE;

	    std::map<int,Chip>* currMapping = nullptr;

	    if(ecalConfig->at(iSlab).ASU.find("FEV") != std::string::npos) currMapping = mapping;
	    else currMapping = cobMapping; 
	      
	    newHit.I = (*currMapping)[iChip].chanMapping[iChan].first;
	    newHit.J = (*currMapping)[iChip].chanMapping[iChan].second;
	    newHit.K = iSlab;
	    
	    if(newHit.I < 16) newHit.position[0] = -((15 - newHit.I)*5.5 + 3.8) - ecalConfig->at(iSlab).deltaX;
	    else newHit.position[0] = (newHit.I - 16)*5.5 + 3.8 - ecalConfig->at(iSlab).deltaX;
	    if(newHit.J < 16) newHit.position[1] = -((15 - newHit.J)*5.5 + 3.8);
	    else newHit.position[1] = (newHit.J - 16)*5.5 + 3.8;
	    newHit.position[2] = iSlab*15;

	    newHit.CHP = iChip;
	    newHit.CHN = iChan;
	    newHit.SCA = iMem;

	    newHit.bcid = rawEvent->corrected_bcid[iSlab][iChip][iMem];
	    newHit.badbcid = rawEvent->badbcid[iSlab][iChip][iMem];
	    
	    tools->histograms1D.at("BCID")->Fill(rawEvent->bcid[iSlab][iChip][iMem]);
	    tools->histograms1D.at("CorrectedBCID")->Fill(rawEvent->corrected_bcid[iSlab][iChip][iMem]);
	    
	    bcidMap[rawEvent->corrected_bcid[iSlab][iChip][iMem]].push_back(newHit);
	    nHitsReadout++;

	  } // End of Channel loop
        } // End of Sca loop
      } // End of Chip loop
    } // End of Layer loop

    tools->readoutDone = true;
    tools->histograms1D.at("NHitsReadout")->Fill(nHitsReadout);

    if(_debug) std::cout << "Starting BCID Merging. NBins in readout: " << bcidMap.size() << std::endl;	    
    
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

      if(_debug) std::cout << "Event with " << totalHits << " number of hits" << std::endl;
      
      LCEventImpl* evt = new LCEventImpl();
      evt->setEventNumber(evtNr);
      evt->setTimeStamp(maxIt->first);

      LCCollectionVec* outCol = new LCCollectionVec(LCIO::CALORIMETERHIT);
      outCol->setFlag(0|(1 << LCIO::RCHBIT_LONG));

      CellIDEncoder<CalorimeterHitImpl> cd("I:5,J:5,K:4,CHP:4,CHN:6,SCA:4",outCol);
      
      std::map<int,std::map<int,std::map<std::pair<int,int>,CalorimeterHitImpl*>>> hitMap = {};
      std::map<int,std::map<int,std::map<std::pair<int,int>,int>>> mergeMap = {};
      
      float sumEnergy = 0.;
      float sumEnergyErrSq = 0.;
      
      while(bcidIt != scanIt) {
	for(auto hitIt = bcidIt->second.begin(); hitIt != bcidIt->second.end(); hitIt++) {

	  if(_debug) std::cout << "Creating new CalorimeterHit" << std::endl; 

	  CalorimeterHitImpl* lcioHit = new CalorimeterHitImpl();
	  lcioHit->setEnergy(hitIt->energy);
	  lcioHit->setEnergyError(pow(hitIt->energyE,2));
	  
	  lcioHit->setPosition(hitIt->position);
	  lcioHit->setTime(bcidIt->first);

	  cd["CHP"] = hitIt->CHP;
	  cd["CHN"] = hitIt->CHN;
	  cd["SCA"] = hitIt->SCA;
	  cd["I"] = hitIt->I;
	  cd["J"] = hitIt->J;
	  cd["K"] = hitIt->K;
	  cd.setCellID(lcioHit);
	  
	  if(hitMap[hitIt->K][hitIt->CHP].find(std::pair<int,int>(hitIt->I,hitIt->J)) != hitMap[hitIt->K][hitIt->CHP].end()) {

	    continue;
	    
	    tools->histograms2D.at("MergedEA_EB")->Fill(hitMap[hitIt->K][hitIt->CHP][std::pair<int,int>(hitIt->I,hitIt->J)]->getEnergy(), hitIt->energy);	    
	    
	    hitMap[hitIt->K][hitIt->CHP][std::pair<int,int>(hitIt->I,hitIt->J)]->setEnergy(hitMap[hitIt->K][hitIt->CHP][std::pair<int,int>(hitIt->I,hitIt->J)]->getEnergy() + hitIt->energy); 

	    hitMap[hitIt->K][hitIt->CHP][std::pair<int,int>(hitIt->I,hitIt->J)]->setEnergyError(hitMap[hitIt->K][hitIt->CHP][std::pair<int,int>(hitIt->I,hitIt->J)]->getEnergyError() + pow(hitIt->energyE,2));

	    tools->histograms1D.at("Merged_I")->Fill(hitIt->I);
	    tools->histograms1D.at("Merged_J")->Fill(hitIt->J);
	    tools->histograms1D.at("Merged_K")->Fill(hitIt->K);
	    tools->histograms1D.at("Merged_DeltaBCID")->Fill(bcidIt->first - hitMap[hitIt->K][hitIt->CHP][std::pair<int,int>(hitIt->I,hitIt->J)]->getTime());

	    mergeMap[hitIt->K][hitIt->CHP][std::pair<int,int>(hitIt->I,hitIt->J)]++;
	    
	  }
	  else {	    
	    hitMap[hitIt->K][hitIt->CHP][std::pair<int,int>(hitIt->I,hitIt->J)] = lcioHit;
	    mergeMap[hitIt->K][hitIt->CHP][std::pair<int,int>(hitIt->I,hitIt->J)] = 0;
	  }
	  
	  sumEnergy += hitIt->energy;
	  sumEnergyErrSq += pow(hitIt->energyE,2);
	  	  
	  tools->histograms1D.at("HitEnergy")->Fill(hitIt->energy);
	  tools->histograms1D.at("I")->Fill(hitIt->I);
	  tools->histograms1D.at("J")->Fill(hitIt->J);
	  tools->histograms1D.at("K")->Fill(hitIt->K);

	}
	bcidIt++;
      } // End of scan loop

      for(auto layer : mergeMap) {
	for(auto chip : layer.second) {
	  for(auto hit : chip.second) {
	    if(hit.second != 0) tools->histograms1D.at("NMerges")->Fill(hit.second);
	  }
	}
      }
      
      int nHits, nLayers, nChips;
      nHits = nLayers = nChips = 0;

      for(auto layerIt = hitMap.begin(); layerIt != hitMap.end(); layerIt++) {
	int nHitsInLayer = 0;
	for(auto chipIt = layerIt->second.begin(); chipIt != layerIt->second.end(); chipIt++) {
	  tools->histograms1D.at("NHitsChip")->Fill(chipIt->second.size());
	  nHitsInLayer += chipIt->second.size();
	  nChips++;
	  for(auto lcioHit : chipIt->second) {
	    lcioHit.second->setEnergyError(sqrt(lcioHit.second->getEnergyError()));
	    outCol->addElement(lcioHit.second);
	    nHits++;
	  }
	}
	tools->histograms1D.at("NHitsLayer")->Fill(nHitsInLayer);
	nLayers++;
      }

      tools->histograms1D.at("NHits")->Fill(nHits);
      tools->histograms1D.at("NChips")->Fill(nChips);
      tools->histograms1D.at("NLayers")->Fill(nLayers);
      tools->histograms1D.at("SumEnergy")->Fill(sumEnergy);

      outCol->parameters().setValue("NLayers", nLayers);
      outCol->parameters().setValue("NChips", nChips);
      outCol->parameters().setValue("SumEnergy", sumEnergy);
      outCol->parameters().setValue("SumEnergyError", (float)sqrt(sumEnergyErrSq));
      
      evt->addCollection(outCol, _outputColName.c_str());
      outputWriter->writeEvent(evt);
      delete evt;

      evtNr++;
    } // End of loop over BICD Map
    
}


void EventBuilder::processBuiltEvent() {

     BuiltROOTEvent* builtEvent = (BuiltROOTEvent*)inEvent;

     readoutNr = builtEvent->cycle;
     
     if(builtEvent->nhit_len > ARRAYSIZE) {
       std::cout << "Not enough space in arrays. Hits = " << builtEvent->nhit_len << " - Array size = " << ARRAYSIZE << std::endl;
       return;
     }
     
     LCEventImpl* evt = new LCEventImpl();
     evt->setEventNumber(evtNr);
     evt->setTimeStamp(builtEvent->bcid);

     tools->histograms1D.at("CorrectedBCID")->Fill(builtEvent->bcid);
     
     LCCollectionVec* outCol = new LCCollectionVec(LCIO::CALORIMETERHIT);
     outCol->setFlag(0|(1 << LCIO::RCHBIT_LONG));

     CellIDEncoder<CalorimeterHitImpl> cd("I:5,J:5,K:4,CHP:4,CHN:6,SCA:4,",outCol);

     std::map<int,std::map<int,std::map<std::pair<int,int>,int>>> hitMap = {};
     
     float evtEnergy = 0.;
     int nHits = 0;
     
     for(int iHit = 0; iHit < builtEvent->nhit_len; iHit++) { 

       if(!builtEvent->hit_isHit[iHit] || builtEvent->hit_isMasked[iHit] || !builtEvent->hit_isCommissioned[iHit]) continue;
       
	  CalorimeterHitImpl* lcioHit = new CalorimeterHitImpl();
	  
	  int I, J, K;

	  if(builtEvent->hit_x[iHit] < 0) I = (int)((builtEvent->hit_x[iHit] + ecalConfig->at(builtEvent->hit_slab[iHit]).deltaX + 86.5)/5.5); 
	  else I = (int)((builtEvent->hit_x[iHit] + ecalConfig->at(builtEvent->hit_slab[iHit]).deltaX)/5.5) + 16; 
	  if(builtEvent->hit_y[iHit] < 0) J = (int)((builtEvent->hit_y[iHit] + 86.5)/5.5);
	  else J = (int)(builtEvent->hit_y[iHit]/5.5) + 16;
	  K = builtEvent->hit_slab[iHit];
	  
	  float pos[3];

	  pos[0] = builtEvent->hit_x[iHit];
	  pos[1] = builtEvent->hit_y[iHit];
	  pos[2] = builtEvent->hit_z[iHit];
	  
	  lcioHit->setPosition(pos);
	  lcioHit->setTime(builtEvent->bcid);

	  cd["CHP"] = builtEvent->hit_chip[iHit];
	  cd["CHN"] = builtEvent->hit_chan[iHit];
	  cd["SCA"] = builtEvent->hit_sca[iHit];
	  cd["I"] = I;
	  cd["J"] = J;
	  cd["K"] = K;
	  cd.setCellID(lcioHit);

	  float hitEnergy = builtEvent->hit_energy[iHit];
	  if(hitEnergy > 80) hitEnergy = builtEvent->hit_energy_lg[iHit];

	  lcioHit->setEnergy(hitEnergy);
	  evtEnergy += hitEnergy;

	  if(hitMap[K][builtEvent->hit_chip[iHit]][std::pair<int,int>(I,J)]) {
	    std::cout << "Found two hits in the same channel: " << hitMap[K][builtEvent->hit_chip[iHit]][std::pair<int,int>(I,J)] << " " << builtEvent->bcid << std::endl;  
	  }
	  else hitMap[K][builtEvent->hit_chip[iHit]][std::pair<int,int>(I,J)] = builtEvent->bcid;
	  
	  outCol->addElement(lcioHit);
	  nHits++;
	  
	  tools->histograms1D.at("I")->Fill(I);
	  tools->histograms1D.at("J")->Fill(J);
	  tools->histograms1D.at("K")->Fill(K);
     }

     int nLayers, nChips;
     nLayers = nChips = 0;
     
     for(auto layerIt = hitMap.begin(); layerIt != hitMap.end(); layerIt++) {
       int nHitsInLayer = 0;
       for(auto chipIt = layerIt->second.begin(); chipIt != layerIt->second.end(); chipIt++) {
	 tools->histograms1D.at("NHitsChip")->Fill(chipIt->second.size());
	 nHitsInLayer += chipIt->second.size();
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
     outCol->parameters().setValue("SumEnergy", evtEnergy);

     evt->addCollection(outCol, _outputColName.c_str());
     outputWriter->writeEvent(evt);
     delete evt;
     
     evtNr++;
     
}
