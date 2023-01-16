#include "MIPAlignment.h"

#include <UTIL/CellIDDecoder.h>

#include <EVENT/LCCollection.h>
#include <EVENT/CalorimeterHit.h>

#include <TFitResult.h>
#include <TCanvas.h>
#include <TMath.h>

#include <iostream>

MIPAlignmentProc a_MIPAlignmentProc;

//=========================================================
MIPAlignmentProc::MIPAlignmentProc()
  :Processor("MIPAlignmentProc")
{
  
  //Input Collections    
  _ecalCollections.push_back(std::string("ECAL_MIPEvents"));
  registerInputCollections(LCIO::RAWCALORIMETERHIT, 
			   "ECALCollections",  
			   "ECAL Collection Names",  
			   _ecalCollections, 
			   _ecalCollections); 
  
  //Output ROOT name
  _logRootName = "LogROOT_MIPAlignment";
  registerProcessorParameter("logRoot_Name" ,
                             "LogRoot name",
                             _logRootName,
			     _logRootName);

} 

void MIPAlignmentProc::init() {

     streamlog_out(DEBUG4) << "MIPAlignment Init" << std::endl;

     _outputFile = new TFile((_logRootName + ".root").c_str(),"RECREATE");	  
     
     printParameters();
}


void MIPAlignmentProc::processRunHeader( LCRunHeader* runHd )
{

  streamlog_out(MESSAGE) << "MIPAlignment processing run: " << runHd->getRunNumber() << std::endl;

  if(runNumber != -1) {
    writeHistograms();
  }
  runNumber = runHd->getRunNumber();

  // ---- Creation of the histograms ----

  streamlog_out(DEBUG4) << "Creating new local histograms" << std::endl;

  _outputFile->mkdir(std::to_string(runNumber).c_str());

  histograms1D["K"] = new TH1F("K", ";K;Entries", 15, 0., 15.);
  
  for(int iLayer = 0; iLayer < 15; iLayer++) {
    
    histograms1D[("XFull_Layer" + std::to_string(iLayer)).c_str()] = new TH1F(("XFull_Layer" + std::to_string(iLayer)).c_str(),";X;Entries", 256, -86.5, 86.5);
    histograms1D[("YFull_Layer" + std::to_string(iLayer)).c_str()] = new TH1F(("YFull_Layer" + std::to_string(iLayer)).c_str(),";Y;Entries", 256, -86.5, 86.5);
    
    histograms1D[("X_Layer" + std::to_string(iLayer)).c_str()] = new TH1F(("X_Layer" + std::to_string(iLayer)).c_str(),";X;Entries", 256, -86.5, 86.5);
    histograms1D[("Y_Layer" + std::to_string(iLayer)).c_str()] = new TH1F(("Y_Layer" + std::to_string(iLayer)).c_str(),";Y;Entries", 256, -86.5, 86.5);

    histograms1D[("I_Layer" + std::to_string(iLayer)).c_str()] = new TH1F(("I_Layer" + std::to_string(iLayer)).c_str(),";I;Entries", 32, 0., 32.);
    histograms1D[("J_Layer" + std::to_string(iLayer)).c_str()] = new TH1F(("J_Layer" + std::to_string(iLayer)).c_str(),";J;Entries", 32, 0., 32.);

  }
  
  // ---- Reset counter ----

  evtnum = 0;
  
}

void MIPAlignmentProc::processEvent( LCEvent * evtP ) 
{

  streamlog_out(DEBUG4) << "Start MIPAlignment process" << std::endl;

  if(evtP){ //Check the evtP
    for(unsigned int i=0; i < _ecalCollections.size(); i++){//loop over collections
      try{

	evtnum++;
	
	LCCollection* col = evtP->getCollection(_ecalCollections[i].c_str());
	int nHits = col->getNumberOfElements();
	
	CellIDDecoder<CalorimeterHit> cd("I:5,J:5,K:4,CHP:4,CHN:6,SCA:4");

	std::map<int,std::vector<std::pair<float,float>>> hitsPosMap = {};
	std::map<int,std::vector<std::pair<int,int>>> hitsPadMap = {};
	
	bool acceptEvent = false;
	
	for(int iHit = 0; iHit < nHits; iHit++) {
	  
	  CalorimeterHit* hit = dynamic_cast<CalorimeterHit*>(col->getElementAt(iHit));

	  const float* pos = hit->getPosition();
	  
	  int I = (int)cd(hit)["I"];
	  int J = (int)cd(hit)["J"];
	  int K = (int)cd(hit)["K"];

	  histograms1D.at("K")->Fill(K);
	  
	  histograms1D.at(("XFull_Layer" + std::to_string(K)).c_str())->Fill(pos[0]);
	  histograms1D.at(("YFull_Layer" + std::to_string(K)).c_str())->Fill(pos[1]);
	  
	  if(K == 7 && ((I >= 10 && I <= 13) || (I >= 18 && I <= 21)) && ((J >= 10 && J <= 13) || (J >= 18 && J <= 21))) acceptEvent = true; 
	  
	  hitsPosMap[K].push_back(std::pair<float,float>(pos[0],pos[1]));
	  hitsPadMap[K].push_back(std::pair<int,int>(I,J));
	  
	}

	if(!acceptEvent) return;
	
	for(auto layer : hitsPosMap) {
	  for(auto hit : layer.second) {
	    histograms1D.at(("X_Layer" + std::to_string(layer.first)).c_str())->Fill(hit.first);
	    histograms1D.at(("Y_Layer" + std::to_string(layer.first)).c_str())->Fill(hit.second);
	  }
	}

	for(auto layer : hitsPadMap) {
	  for(auto pad : layer.second) {
	    histograms1D.at(("I_Layer" + std::to_string(layer.first)).c_str())->Fill(pad.first);
	    histograms1D.at(("J_Layer" + std::to_string(layer.first)).c_str())->Fill(pad.second);
	  }
	}
	
      }catch (lcio::DataNotAvailableException zero) { streamlog_out(DEBUG) << "ERROR READING COLLECTION: " << _ecalCollections[i] << std::endl;} 
    }//end loop over collection
  }
  
}


//==============================================================
void MIPAlignmentProc::end()
{

  writeHistograms();

  _outputFile->cd();

  _outputFile->Close();
  delete _outputFile;
  
  streamlog_out(DEBUG4) << "Ending MIPAlignment" << std::endl;

}

void MIPAlignmentProc::writeHistograms() {

  _outputFile->cd((std::to_string(runNumber)).c_str());

  streamlog_out(DEBUG) << "Writing 1D histograms" << std::endl;

  TCanvas* canvases[6];
  canvases[0] = new TCanvas("CanvasXFull");
  canvases[0]->Divide(4,4);
  canvases[1] = new TCanvas("CanvasYFull");
  canvases[1]->Divide(4,4);
  canvases[2] = new TCanvas("CanvasX");
  canvases[2]->Divide(4,4);
  canvases[3] = new TCanvas("CanvasY");
  canvases[3]->Divide(4,4);
  canvases[4] = new TCanvas("CanvasI");
  canvases[4]->Divide(4,4);
  canvases[5] = new TCanvas("CanvasJ");
  canvases[5]->Divide(4,4);
  
  for(auto It1D = histograms1D.begin(); It1D != histograms1D.end(); It1D++) {

    int canvasIndex = -1;

    if(It1D->first.find("XFull",0) != std::string::npos) canvasIndex = 0;
    else if(It1D->first.find("YFull",0) != std::string::npos) canvasIndex = 1;
    else if(It1D->first.find("X",0) != std::string::npos) canvasIndex = 2;
    else if(It1D->first.find("Y",0) != std::string::npos) canvasIndex = 3;
    else if(It1D->first.find("I",0) != std::string::npos) canvasIndex = 4;
    else if(It1D->first.find("J",0) != std::string::npos) canvasIndex = 5;  

    if(canvasIndex != -1) {
      int layer =  std::stoi(It1D->first.substr(It1D->first.find("Layer") + 5));
	 
      canvases[canvasIndex]->cd(layer%16 + 1);

      It1D->second->Scale(1./It1D->second->GetEntries());
      
      TFitResultPtr fitRes = It1D->second->Fit("gaus","QS");

      const double* pars = fitRes->GetParams();

      It1D->second->Fit("gaus","Q", "", TMath::Max(pars[1] - 2*pars[2], -86.4), TMath::Min(pars[1] + 2*pars[2], 86.4));
      
      It1D->second->Draw();
    }
    else {
      It1D->second->Write();
      delete It1D->second;
    }
  }

  canvases[0]->Write();
  canvases[1]->Write();
  canvases[2]->Write();
  canvases[3]->Write();
  canvases[4]->Write();
  canvases[5]->Write();
  
  delete canvases[0];
  delete canvases[1];
  delete canvases[2];
  delete canvases[3];
  delete canvases[4];
  delete canvases[5];
  
  histograms1D.clear();

}
