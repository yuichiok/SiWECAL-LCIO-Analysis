#include "ECal_EventDisplay.h"

#include <EVENT/LCCollection.h>
#include <EVENT/CalorimeterHit.h>
#include <UTIL/CellIDDecoder.h>

#include "TCanvas.h"
#include "TH2F.h"

#include <iostream>
#include <fstream>

EventDisplay::EventDisplay() {

  // --- Initializing input to the default values
  _inputColName = _inputColNameDefault;
  _inputEventsFileName = _inputEventsFileNameDefault;
  _outputFileName = _outputFileNameDefault;
  
}


// Check if the arguments are correct
void EventDisplay::Check() {

  //Input Collection Name
  if(_inputColName == "") {
    std::cout << "Input TTree name empty. Using default name: " << _inputColNameDefault << std::endl;
    _inputColName = _inputColNameDefault;
  }

  //Input Events File Name
  if(_inputEventsFileName == "") {
    std::cout << "Input events file name empty. Using default name: " << _inputEventsFileNameDefault << std::endl;
    _inputEventsFileName = _inputEventsFileNameDefault;
  }
  
  //Output File Name
  if(_outputFileName == "") {
    std::cout << "Output File name empty. Using default name: " << _outputFileNameDefault << std::endl;
    _outputFileName = _outputFileNameDefault;  
  }
  
}


bool EventDisplay::Init() {
  
  // --- Opening the input slcio

  inputReader = LCFactory::getInstance()->createLCReader(IO::LCReader::directAccess);
  inputReader->open(_inputFileName);

  _runNr = inputReader->readNextRunHeader()->getRunNumber();
  
  _nEntries = inputReader->getNumberOfEvents();
  
  // --- Creating the output TFile
  
  outputFile = new TFile(_outputFileName.c_str(), "RECREATE", "OutputFile");
  if(!outputFile->IsOpen()) {
    std::cout << "Could not open the ROOT output file: " << _outputFileName << std::endl;
    return false;
  }

  std::ifstream evtsFile(_inputEventsFileName);
  if(!evtsFile) {
    std::cout << "Could not open the events file: " << _inputEventsFileName << std::endl;
    return false;
  }

  std::string tmp_header;
  int tmp_evt;

  std::getline(evtsFile,tmp_header);
  while(evtsFile) {
    evtsFile >> tmp_evt;
    if(std::find(evtsMap.begin(),evtsMap.end(),tmp_evt) != evtsMap.end()) continue;
    evtsMap.push_back(tmp_evt);
  }

  evtsFile.close();
  
  return true;
}


void EventDisplay::Process() {

    for(auto evtNr : evtsMap ) {

      if(evtNr > _nEntries) {
	std::cout << "In run: " << _runNr << ". Event: " << evtNr << " is larger than the number of entries in the file: " << _nEntries << std::endl;
	continue;
      }

      LCEvent* evt = inputReader->readEvent(0,evtNr);

      if(evt == NULL) {
	std::cout << "Evt: " << evtNr << " in Run: " << _runNr << " not found in the file." << std::endl;
	continue;
      }

      outputFile->mkdir((std::to_string(_runNr) + "/" + std::to_string(evtNr)).c_str());
      outputFile->cd((std::to_string(_runNr) + "/" + std::to_string(evtNr)).c_str());

      DisplayEvent(evt);
	
    }
}


void EventDisplay::End() {
  
  //Closing files and freeing memory

  if(inputReader != nullptr) {
    inputReader->close();
    delete inputReader;
  }

  if(outputFile != nullptr) {
    outputFile->Close();
    delete outputFile;
  }
  
}

void EventDisplay::DisplayEvent(LCEvent* evt) {

  try{
    
    LCCollection* col = evt->getCollection(_inputColName.c_str());
    int nHits = col->getNumberOfElements();
    
    CellIDDecoder<CalorimeterHit> decoder("I:5,J:5,K:4,CHP:4,CHN:6,SCA:4");
    
    TH2F* IJ = new TH2F("IJ", ";I[Pad];J[Pad]", 32, 0., 32., 32, 0., 32.);
    TH2F* IK = new TH2F("IK", ";I[Pad];K[Layer]", 32, 0., 32., 15, 0., 15.);
    TH2F* KJ = new TH2F("KJ", ";K[Layer];J[Pad]", 15, 0., 15., 32, 0., 32.);

    TH2F* IJ_E = new TH2F("IJ_E", ";I[Pad];J[Pad]", 32, 0., 32., 32, 0., 32.);
    TH2F* IK_E = new TH2F("IK_E", ";I[Pad];K[Layer]", 32, 0., 32., 15, 0., 15.);
    TH2F* KJ_E = new TH2F("KJ_E", ";K[Layer];J[Pad]", 15, 0., 15., 32, 0., 32.);
    
    for(int iHit = 0; iHit < nHits; iHit++) {

      CalorimeterHit* hit = dynamic_cast<CalorimeterHit*>(col->getElementAt(iHit));

      int I = (int)decoder(hit)["I"];
      int J = (int)decoder(hit)["J"];
      int K = (int)decoder(hit)["K"];

      if(K < 2) I -= 10;
      
      IJ->Fill(I,J);
      IK->Fill(I,K);
      KJ->Fill(K,J);
      
      float hitE = hit->getEnergy();

      IJ_E->SetBinContent(I + 1, J + 1, hitE);
      IK_E->SetBinContent(I + 1, K + 1, hitE);
      KJ_E->SetBinContent(K + 1, J + 1, hitE);
      
    }

    TCanvas* canvas = new TCanvas(("Display_" + std::to_string(evt->getEventNumber())).c_str());
    canvas->Divide(2,2);

    canvas->cd(1);
    IK->Draw("COLZ");

    canvas->cd(3);
    IJ->Draw("COLZ");

    canvas->cd(4);
    KJ->Draw("COLZ");

    canvas->Write();
    delete canvas;

    delete IJ;
    delete IK;
    delete KJ;

    TCanvas* canvas_E = new TCanvas(("Display_E_" + std::to_string(evt->getEventNumber())).c_str());
    canvas_E->Divide(2,2);

    canvas_E->cd(1);
    IK_E->Draw("COLZ");

    canvas_E->cd(3);
    IJ_E->Draw("COLZ");

    canvas_E->cd(4);
    KJ_E->Draw("COLZ");

    canvas_E->Write();
    delete canvas_E;
    
    delete IJ_E;
    delete IK_E;
    delete KJ_E;

    
  }catch (lcio::DataNotAvailableException zero) { std::cout << "Error reading collection: "<< _inputColName << std::endl; }
       
}
