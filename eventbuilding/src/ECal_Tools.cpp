#include "ECal_Tools.h"

#include <iostream>
#include <fstream>


ECalTools::ECalTools(bool createLogFile, std::string logFileName, bool debug) {

  debugMode = debug;
  hasLogFile = createLogFile;

  if(!hasLogFile) return;
  InitFileAndHistograms(logFileName);
  
}

ECalTools::~ECalTools() {

  if(!hasLogFile) return;
  WriteAndClose();
  
}

// --- Reading functiEndons 

std::map<int,Slot>* ECalTools::ReadConfigFile(std::string configFileName) {

  std::ifstream file(configFileName);
  if(!file) return nullptr;
  
  std::map<int, Slot>* outMap = new std::map<int, Slot>();

  int tmp_slot, tmp_layer, tmp_slab, tmp_slabID, tmp_slabAdd;
  float tmp_W, tmp_X0, tmp_X0Acc;
  std::string tmp_gliss, tmp_ASU, tmp_wafer, tmp_header;

  std::getline(file, tmp_header);
  while(file) {
    file >> tmp_slot >> tmp_layer >> tmp_slab >> tmp_slabID >> tmp_slabAdd >> tmp_ASU >> tmp_wafer >> tmp_gliss >> tmp_W >> tmp_X0 >> tmp_X0Acc;
    (*outMap)[tmp_slot].layer = tmp_layer;
    (*outMap)[tmp_slot].slab = tmp_slab;
    (*outMap)[tmp_slot].slabID = tmp_slabID;
    (*outMap)[tmp_slot].slabAdd = tmp_slabAdd;
    (*outMap)[tmp_slot].glissiere = tmp_gliss;
    (*outMap)[tmp_slot].W = tmp_W;
    (*outMap)[tmp_slot].X0 = tmp_X0;
    (*outMap)[tmp_slot].X0Acc = tmp_X0Acc;
    (*outMap)[tmp_slot].ASU = tmp_ASU;
    (*outMap)[tmp_slot].wafer = tmp_wafer;
  }

  return outMap;
}

std::map<int,Chip>* ECalTools::ReadMappingFile(std::string mapFileName) {

  std::ifstream file(mapFileName);
  if(!file) return nullptr;

  std::map<int, Chip>* outMap = new std::map<int, Chip>();

  int tmp_chip, tmp_chan;
  float tmp_x0, tmp_y0, tmp_x, tmp_y;
  std::string tmp_header;

  std::getline(file, tmp_header);
  while(file) {
    file >> tmp_chip >> tmp_x0 >> tmp_y0 >> tmp_chan >> tmp_x >> tmp_y;
    if(tmp_chan == 0) {
      (*outMap)[tmp_chip].X0 = tmp_x0;
      (*outMap)[tmp_chip].Y0 = tmp_y0;
    }
    (*outMap)[tmp_chip].chanMapping[tmp_chan].first = tmp_x;
    (*outMap)[tmp_chip].chanMapping[tmp_chan].second = tmp_y;
  }

  return outMap;
}


std::map<int,std::map<int,std::map<int,std::vector<Sca>>>>* ECalTools::ReadPedestalsFile(std::string pedestalsFileName) {

  std::ifstream file(pedestalsFileName);
  if(!file) return nullptr;

  std::map<int,std::map<int,std::map<int,std::vector<Sca>>>>* outMap = new std::map<int,std::map<int,std::map<int,std::vector<Sca>>>>();

  int tmp_layer, tmp_chip, tmp_chan;
  float tmp_ped, tmp_err, tmp_coherent, tmp_incoherent1, tmp_incoherent2;
  std::string tmp_header;

  std::getline(file, tmp_header);
  std::getline(file, tmp_header);

  while(file) {
    file >> tmp_layer >> tmp_chip >> tmp_chan;
    for(int iSca = 0; iSca < 15; iSca++) {

      file >> tmp_ped >> tmp_err;
      if(tmp_err > - 8 && tmp_err < 0) file >> tmp_coherent >> tmp_incoherent1;
      else file >> tmp_coherent >> tmp_incoherent1 >> tmp_incoherent2;
      
      Sca newSca;
      newSca.pedestal = tmp_ped;
      newSca.error = tmp_err;
      newSca.noiseCoherent = tmp_coherent;
      newSca.noiseIncoherent1 = tmp_incoherent1;
      newSca.noiseIncoherent2 = tmp_incoherent2;
      (*outMap)[tmp_layer][tmp_chip][tmp_chan].push_back(newSca);
    }
  }

  return outMap;
}

std::map<int,std::map<int,std::map<int,Chan>>>* ECalTools::ReadCalibrationFile(std::string calibrationFileName) {
  
  std::ifstream file(calibrationFileName);
  if(!file) return nullptr;

  std::map<int,std::map<int,std::map<int,Chan>>>* outMap = new std::map<int,std::map<int,std::map<int,Chan>>>();

  int tmp_layer, tmp_chip, tmp_chan, tmp_nentries;
  float tmp_mpv, tmp_empv, tmp_widthmpv, tmp_chi2ndf;
  std::string tmp_header;

  std::getline(file, tmp_header);
  std::getline(file, tmp_header);

  std::map<int, std::map<int,std::pair<float,int>>> meanValues;
  while(file) {
    file >> tmp_layer >> tmp_chip >> tmp_chan >> tmp_mpv >> tmp_empv >> tmp_widthmpv >> tmp_chi2ndf >> tmp_nentries;
    (*outMap)[tmp_layer][tmp_chip][tmp_chan].mpv = tmp_mpv;
    (*outMap)[tmp_layer][tmp_chip][tmp_chan].empv = tmp_empv;
    (*outMap)[tmp_layer][tmp_chip][tmp_chan].widthmpv = tmp_widthmpv;
    (*outMap)[tmp_layer][tmp_chip][tmp_chan].chi2ndf = tmp_chi2ndf;
    (*outMap)[tmp_layer][tmp_chip][tmp_chan].nentries = tmp_nentries;
    if(tmp_mpv > 0) {
      meanValues[tmp_layer][tmp_chip].first += tmp_mpv;
      meanValues[tmp_layer][tmp_chip].second++;
    }
  }

  for(auto layerIt = outMap->begin(); layerIt != outMap->end(); layerIt++) {
    if(meanValues.find(layerIt->first) == meanValues.end()) continue;
    for(auto chipIt = layerIt->second.begin(); chipIt != layerIt->second.end(); chipIt++) {
      if(meanValues.at(layerIt->first).find(chipIt->first) == meanValues.at(layerIt->first).end()) continue;
      for(auto chanIt = chipIt->second.begin(); chanIt != chipIt->second.end(); chanIt++) {
	if(chanIt->second.empv < 0) {
	  chanIt->second.mpv = meanValues.at(layerIt->first).at(chipIt->first).first/meanValues.at(layerIt->first).at(chipIt->first).second;
	}
      }
    }
    
  }
  
  return outMap;
}

std::map<int,std::map<int, std::map<int,bool>>>* ECalTools::ReadMaskedFile(std::string maskedFileName) {

  std::ifstream file(maskedFileName);
  if(!file) return nullptr;

  std::map<int,std::map<int,std::map<int,bool>>>* outMap = new std::map<int,std::map<int,std::map<int,bool>>>();

  int tmp_layer, tmp_chip, tmp_masked;
  std::string tmp_header;

  std::getline(file, tmp_header);
  while(file) {
    file >> tmp_layer >> tmp_chip;
    for(int iChan = 0; iChan < 64; iChan++) {
      file >> tmp_masked;
      (*outMap)[tmp_layer][tmp_chip][iChan] = tmp_masked; 
    }
  }

  return outMap;
}

// --- Displaying functions

void ECalTools::DisplayMapping(std::map<int,Chip>* mapping) {

  for(auto chipIt = mapping->begin(); chipIt != mapping->end(); chipIt++) {

    std::cout << " ### --- Chip: " << chipIt->first << ". X0 = " << chipIt->second.X0 << " Y0 = " << chipIt->second.Y0 << std::endl;
    for(auto chanIt = chipIt->second.chanMapping.begin(); chanIt != chipIt->second.chanMapping.end(); chanIt++) {
      std::cout << "| Channel " << chanIt->first << ". X = " << chanIt->second.first << " Y = " << chanIt->second.second << " |"; 
    }
    std::cout << std::endl;
  }
  
}

void ECalTools::DisplayConfiguration(std::map<int,Slot>* config) {

  for(auto slotIt = config->begin(); slotIt != config->end(); slotIt++) {
    std::cout << " ### --- Slot: " << slotIt->first << ". Layer = " << slotIt->second.layer << " slab = " << slotIt->second.slab << " slabID = " << slotIt->second.slabID << " slabAdd = " << slotIt->second.slabAdd << " ASUType = " << slotIt->second.ASU << " Wafer = " << slotIt->second.wafer << " Glissiere = " << slotIt->second.glissiere << " W = " << slotIt->second.W << " X0 = " << slotIt->second.X0 << " X0Acc = " << slotIt->second.X0Acc << std::endl;
  }
  
}

void ECalTools::DisplayPedestals(std::map<int,std::map<int,std::map<int,std::vector<Sca>>>>* pedestals) {

  for(auto layerIt = pedestals->begin(); layerIt != pedestals->end(); layerIt++) {
    for(auto chipIt = layerIt->second.begin(); chipIt != layerIt->second.end(); chipIt++) {
      for(auto chanIt = chipIt->second.begin(); chanIt != chipIt->second.end(); chanIt++) {
	std::cout << "Layer = " << layerIt->first << " Chip = " << chipIt->first << " Channel = " << chanIt->first << " - ";
	for(int iSca = 0; iSca < 15; iSca++) {
	  Sca currSca = chanIt->second.at(iSca);
	  std::cout << "SCA" << iSca << " = " << currSca.pedestal << " " << currSca.error << " " << currSca.noiseCoherent << " " << currSca.noiseIncoherent1 << " " << currSca.noiseIncoherent2 << " - ";  
	}
	std::cout << std::endl;
      }
    }
  }
}

void ECalTools::DisplayCalibration(std::map<int,std::map<int,std::map<int,Chan>>>* calibration) {

  for(auto layerIt = calibration->begin(); layerIt != calibration->end(); layerIt++) {
    for(auto chipIt = layerIt->second.begin(); chipIt != layerIt->second.end(); chipIt++) {
      for(auto chanIt = chipIt->second.begin(); chanIt != chipIt->second.end(); chanIt++) {
	std::cout << "Layer = " << layerIt->first << " Chip = " << chipIt->first << " Channel = " << chanIt->first << " MPV = " << chanIt->second.mpv << " EMPV = " << chanIt->second.empv << " WidthMPV = " << chanIt->second.widthmpv << " Chi = " << chanIt->second.chi2ndf << " NEntries = " << chanIt->second.nentries << std::endl;
      }
    }
  }
}


void ECalTools::DisplayMasked(std::map<int,std::map<int, std::map<int,bool>>>* masked) {

  for(auto layerIt = masked->begin(); layerIt != masked->end(); layerIt++) {
    for(auto chipIt = layerIt->second.begin(); chipIt != layerIt->second.end(); chipIt++) {
      for(auto chanIt = chipIt->second.begin(); chanIt != chipIt->second.end(); chanIt++) {
	std::cout << "Layer = " << layerIt->first << " Chip = " << chipIt->first << " Channel = " << chanIt->first << " Masked = " << chanIt->second << std::endl;
      }
    }
  }
  
}


// Log File functions


void ECalTools::InitFileAndHistograms(std::string logFileName) {

  if(debugMode) std::cout << "Creating the LogROOT and histograms" << std::endl;
  
  logFile = new TFile(logFileName.c_str(), "RECREATE");

  histograms1D["CorrectedBCID"] = new TH1F("CorrectedBCID", "Corrected BCID;BCID;Entries", 10000, 0., 10000.);
  histograms1D["Readout"] = new TH1F("Readout", "Readout No-Correction;BCID;Entries", 5000, 0., 5000.);
  histograms1D["CorrectedReadout"] = new TH1F("CorrectedReadout", "Readout Corrected;BCID;Entries", 5000, 0., 5000.);
  histograms1D["NHitsReadout"] = new TH1F("NHitsReadout", "NHits Per Readout;NHits;Entries", 10000, 0., 10000.);
  //histograms1D["NoisePerTimeBin"] = new TH1F("NoisePerTimeBin", ";MeanNoise(NHit);Entries", 100, 0., 2.);

  histograms1D["NHits"] = new TH1F("NHits", "NHits Per Event;NHits;Entries", 15000, 0., 15000.);
  histograms1D["NHitsLayer"] = new TH1F("NHitsPerLayer", "NHits Per Layer;NHits;Entries", 10000, 0., 10000.);
  histograms1D["NHitsChip"] = new TH1F("NHitsPerChip", "NHits Per Chip;NHits;Entries", 1000, 0., 1000.);
  
  histograms1D["NChips"] = new TH1F("NChips", "NChips;NChips;Entries", 1000, 0., 1000.);
  histograms1D["NLayers"] = new TH1F("NLayers", "NLayers;NLayers;Entries", 20, 0., 20.);

  histograms1D["I"] = new TH1F("I", "I;I(Pad);Entries", 35, 0., 35.);
  histograms1D["J"] = new TH1F("J", "J;J(Pad);Entries", 35, 0., 35.);
  histograms1D["K"] = new TH1F("K", "K;K(Pad);Entries", 20, 0., 20.);

}


void ECalTools::WriteAndClose() {

  if(debugMode) std::cout << "Writing histograms and closing the LogROOT file" << std::endl;
  
  logFile->cd();

  for(auto It1D = histograms1D.begin(); It1D != histograms1D.end(); It1D++) {
    It1D->second->Write();
    delete It1D->second;
  }

  histograms1D.clear();

  
  logFile->Close();
  delete logFile;
  
}
