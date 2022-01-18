#include "ECalAnalysis.h"

#include <EVENT/LCCollection.h>

ECalAnalysisProc a_ECalAnalysisProc;

//=========================================================
ECalAnalysisProc::ECalAnalysisProc()
  :Processor("ECalAnalysisProc")
{
  
  //Collections to read 
  registerInputCollections( LCIO::CALORIMETERHIT, 
			    "Input_Collections",  
			    "Input Hit Collections Names",  
			    _inputCollections, 
			    _inputCollections); 

}


void ECalAnalysisProc::init() {

     streamlog_out(DEBUG) << "ECalAnalysis Init" << std::endl;
     
     printParameters();
  
}


void ECalAnalysisProc::processRunHeader( LCRunHeader* runH )
{

  //if(runH->getRunNumber() == currRun) return;

  currRun = runH->getRunNumber();  
  streamlog_out(MESSAGE) << "Processing header " << currRun << std::endl;
 
  if(logROOT != nullptr) writeHistograms();

  //Log ROOT
  logROOT = new TFile("LogROOT_SiWECal.root","RECREATE");
     
  nHitHist = new TH1F("NHit", "", 250, 0., 2000.);
  nHitHist->SetTitle(";nHit;Entries");

}

void ECalAnalysisProc::processEvent( LCEvent * evtP ) 
{

  streamlog_out(DEBUG) << "Process ECal Analysis in run: " << currRun << std::endl;

  if(evtP){
    try{//try 1
      
      for(unsigned int i=0; i < _inputCollections.size(); i++){//loop over collections
	try{//try2
	  
	  std::string collName = _inputCollections[i];
	  
	  LCCollection* col = evtP->getCollection(collName.c_str());

	  int nHits = col->getParameters().getIntVal("NHits");
	  nHitHist->Fill(nHits);
	  
	}catch (lcio::DataNotAvailableException zero) { streamlog_out(DEBUG) << "Error Catch 2" << std::endl; } //catch 2
      }//end loop over collection
    }catch (lcio::DataNotAvailableException err) { streamlog_out(DEBUG) << "Error Catch 1." << std::endl; }//catch 1
  }
}


//==============================================================
void ECalAnalysisProc::end()
{
  streamlog_out(MESSAGE) << "Ending ECalAnalysis" << std::endl;

  writeHistograms();

}



void ECalAnalysisProc::writeHistograms()
{

  nHitHist->Write();
  delete nHitHist;
  
  delete logROOT;

  streamlog_out(DEBUG) << "Ending writing histograms" << std::endl;

}

 
