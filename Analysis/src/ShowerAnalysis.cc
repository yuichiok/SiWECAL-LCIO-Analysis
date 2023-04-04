#include "ShowerAnalysis.h"

#include <UTIL/CellIDDecoder.h>

#include <EVENT/LCCollection.h>
#include <EVENT/CalorimeterHit.h>

#include <IMPL/LCEventImpl.h>
#include <IMPL/LCCollectionVec.h>
#include <IMPL/CalorimeterHitImpl.h>

#include "TPaveText.h"

#include <iostream>
#include <limits>

#include <TMath.h>

ShowerAnalysisProc a_ShowerAnalysisProc;

//=========================================================
ShowerAnalysisProc::ShowerAnalysisProc()
  :Processor("ShowerAnalysisProc")
{
  
  //Input Collections    
  _ecalCollections.push_back(std::string("ECAL_BeamEvents"));
  registerInputCollections(LCIO::RAWCALORIMETERHIT, 
			   "ECALCollections",  
			   "ECAL Collection Names",  
			   _ecalCollections, 
			   _ecalCollections); 
  
  //Output ROOT name
  _logRootName = "LogROOT_ShowerAnalysis";
  registerProcessorParameter("logRoot_Name" ,
                             "LogRoot name",
                             _logRootName,
			     _logRootName);

} 

void ShowerAnalysisProc::init() {

     streamlog_out(DEBUG4) << "ShowerAnalysis Init" << std::endl;

     _outputFile = new TFile((_logRootName + ".root").c_str(),"RECREATE");	  
     
     printParameters();
  
}

void ShowerAnalysisProc::processRunHeader( LCRunHeader* runHd )
{

  streamlog_out(MESSAGE) << "ShowerAnalysis processing run: " << runHd->getRunNumber() << std::endl;

  if(runNumber != -1) {
    writeHistograms();
  }
  runNumber = runHd->getRunNumber();

  // ---- Creation of the output files ----
  
  streamlog_out(DEBUG4) << "Creating new local histograms" << std::endl;

  _outputFile->mkdir(std::to_string(runNumber).c_str());
  _outputFile->cd(std::to_string(runNumber).c_str());

  // ---- Creation of the histograms ----

  histograms1D["NHits"] = new TH1F("NHits", ";NHits;Entries", 250, 0., 2000.);
  histograms1D["HitEnergy"] = new TH1F("HitEnergy", ";HitEnergy;Entries", 100, 0., 1000.);
  histograms1D["SumEnergy"] = new TH1F("SumEnergy", ";SumEnergy;Entries", 100, 0., 1000.);

 
  // ---- Reset counters ----

  evtnum = 0;
  
}

void ShowerAnalysisProc::processEvent( LCEvent * evtP ) 
{

  streamlog_out(DEBUG4) << "Start ShowerAnalysis process" << std::endl;

  if(evtP){ //Check the evtP
    for(unsigned int i=0; i < _ecalCollections.size(); i++){//loop over collections
      try{

	evtnum++;
	
	LCCollection* col = evtP->getCollection(_ecalCollections[i].c_str());
	int nHits = col->getNumberOfElements();

	histograms1D.at("NHits")->Fill(nHits);
	
	CellIDDecoder<CalorimeterHit> cd("I:5,J:5,K:4,CHP:4,CHN:6,SCA:4");

	std::map<int,int> kDistribution = {};

	float sumEnergy = 0.;
	
	for(int iHit = 0; iHit < nHits; iHit++) {
	  
	  CalorimeterHit* hit = dynamic_cast<CalorimeterHit*>(col->getElementAt(iHit));

	  int K = (int)cd(hit)["K"];
	  kDistribution[K]++;

	  sumEnergy += hit->getEnergy();
	  histograms1D.at("HitEnergy")->Fill(hit->getEnergy());
	}

	histograms1D.at("SumEnergy")->Fill(sumEnergy);
	

      }catch (lcio::DataNotAvailableException zero) { streamlog_out(DEBUG) << "ERROR READING COLLECTION: " << _ecalCollections[i] << std::endl;} 
    }//end loop over collection
  }
  
}


//==============================================================
void ShowerAnalysisProc::end()
{
  
  
  writeHistograms();
  
  _outputFile->Close();
  delete _outputFile;

  streamlog_out(DEBUG4) << "Ending ShowerAnalysis" << std::endl;

}


void ShowerAnalysisProc::writeHistograms() {

  _outputFile->cd(std::to_string(runNumber).c_str());

  for(auto It1D = histograms1D.begin(); It1D != histograms1D.end(); It1D++) {
    It1D->second->Write();
    delete It1D->second;
  }

  for(auto It2D = histograms2D.begin(); It2D != histograms2D.end(); It2D++) {
    It2D->second->Write();
    delete It2D->second;
  }
  
}

 
