#include "CleanupAndBeamSelector.h"

#include <UTIL/CellIDDecoder.h>

#include <EVENT/LCCollection.h>
#include <EVENT/CalorimeterHit.h>

#include <IMPL/LCEventImpl.h>
#include <IMPL/LCCollectionVec.h>
#include <IMPL/CalorimeterHitImpl.h>

#include <utility>
#include <iostream>

CleanupAndBeamSelector a_CleanupAndBeamSelector;

//=========================================================
CleanupAndBeamSelector::CleanupAndBeamSelector()
  :Processor("CleanupAndBeamSelector")
{
  
  //Input Collections    
  _ecalCollections.push_back(std::string("ECAL_Events"));
  registerInputCollections(LCIO::CALORIMETERHIT, 
			   "ECALCollections",  
			   "ECAL Collection Names",  
			   _ecalCollections, 
			   _ecalCollections); 
  
  //Output file base name
  _outFileName="TB_ECALBeamEvents_";
  registerProcessorParameter("LCIOOutputFile", 
			     "Base LCIO file name",
			     _outFileName,
			     _outFileName);

  //Output collection name
  _outColName="ECAL_BeamEvents";
  registerProcessorParameter("OutputCollectionName", 
			     "Name of the produced collection in this processor",
			     _outColName,
			     _outColName);

  //Output ROOT name
  _logRootName = "LogROOT_";
  registerProcessorParameter("logRoot_Name" ,
                             "LogRoot name",
                             _logRootName,
			     _logRootName);

  //Cut in the number of hits in a chip to be suspicious of a noisy chip
  _noisyChipCut = 20;
  registerProcessorParameter("NoisyChipCut" ,
                             "Cut in the number of hits in a chip to be suspicious of a noisy chip",
                             _noisyChipCut,
			     _noisyChipCut);


  //Last layer that defines the beginning of the detector to scan for hits
  _lastScanLayer = 10;
  registerProcessorParameter("LastScanLayer",
                             "Last layer that defines the beginning of the detector to scan for hits",
                             _lastScanLayer,
			     _lastScanLayer);


  //Cut in the number of hits at the beginning of the detector scan
  _nLayersScanCut = 5;
  registerProcessorParameter("NLayersScanCut",
                             "Cut in the number of hits at the beginning of the detector scan",
                             _nLayersScanCut,
			     _nLayersScanCut);
  
} 

void CleanupAndBeamSelector::init() {

     streamlog_out(DEBUG3) << "First ECAL Processor Init" << std::endl;

     _lcWriter = LCFactory::getInstance()->createLCWriter() ;
     _lcWriter->setCompressionLevel( 0 );
     
     _outputFile = new TFile((_logRootName + ".root").c_str(),"RECREATE");	  
     
     printParameters();
  
}


void CleanupAndBeamSelector::processRunHeader( LCRunHeader* runHd )
{

  streamlog_out(MESSAGE) << "First ECAL Processor processing run: " << runHd->getRunNumber() << std::endl;

  if(runNumber != -1) {
    _lcWriter->close();
    writeHistograms();
  }
  runNumber = runHd->getRunNumber();

  // ---- Creation of the output files ----

  _lcWriter->open((_outFileName + std::to_string(runNumber) + ".slcio").c_str(),LCIO::WRITE_NEW);
  _lcWriter->writeRunHeader(runHd);
  
  _outputFile->mkdir(std::to_string(runNumber).c_str());
  _outputFile->cd(std::to_string(runNumber).c_str());

  // ---- Creation of the histograms ----

  histograms1D["K"] = new TH1F("K", ";K;Entries", 15, 0., 15.);

  histograms1D["NHits"] = new TH1F("NHits", ";NHits;Entries", 500, 0., 1000.);
  histograms1D["NHitsBeam"] = new TH1F("NHitsBeam", ";NHits;Entries", 500, 0., 1000.);
  
  histograms1D["NLayers"] = new TH1F("NLayers", ";NLayers;Entries", 16, 0., 16.);
  histograms1D["NLayersBeam"] = new TH1F("NLayersBeam", ";NLayers;Entries", 16, 0., 16.);
  
  histograms1D["Energy"] = new TH1F("Energy", ";Energy(MIP);Entries", 10000, 0., 1000.);
  histograms1D["EnergyBeam"] = new TH1F("EnergyBeam", ";Energy(MIP);Entries", 10000, 0., 1000.);

  histograms1D["NHitsInChip"] = new TH1F("NHitsInChip", ";NHits;Entries", 65, 0., 65.);
  histograms1D["NHitsInChipCleanup"] = new TH1F("NHitsInChipCleanup", ";NHits;Entries", 65, 0., 65.);
 
  histograms1D["NHitsInChipPre"] = new TH1F("NHitsInChipPre", ";NHits;Entries", 65, 0., 65.);
  histograms1D["NHitsInChipPreCleanup"] = new TH1F("NHitsInChipPreCleanup", ";NHits;Entries", 65, 0., 65.);

  histograms1D["NHitsInChipPost"] = new TH1F("NHitsInChipPost", ";NHits;Entries", 65, 0., 65.);
  histograms1D["NHitsInChipPostCleanup"] = new TH1F("NHitsInChipPostCleanup", ";NHits;Entries", 65, 0., 65.);

  histograms2D["NHitsPreVsPost"] = new TH2F("NHitsPreVsPost", ";NHitsPre;NHitsPost", 65, 0., 65., 65, 0., 65.);

  histograms1D["NChipsInLayer"] = new TH1F("NChipsInLayer", ";NChips;Entries", 17, 0., 17.);
  
  // ---- Reset counters ----

  evtnum = nBeamParticles = 0;
  
}

void CleanupAndBeamSelector::processEvent( LCEvent * evtP ) 
{

  streamlog_out(DEBUG3) << "First ECAL process" << std::endl;

  if(evtP){ //Check the evtP
    for(unsigned int i=0; i < _ecalCollections.size(); i++){//loop over collections
      try{

	evtnum++;
	
	LCCollection* col = evtP->getCollection(_ecalCollections[i].c_str());

	IMPL::LCCollectionVec* outcolnew = new IMPL::LCCollectionVec(LCIO::CALORIMETERHIT);
	outcolnew->setFlag(col->getFlag());
	
	IMPL::LCCollectionVec* eventcolnew = new IMPL::LCCollectionVec(LCIO::CALORIMETERHIT);
	eventcolnew->setFlag(col->getFlag());
	eventcolnew->setSubset(true);
	
	int nHits = col->getNumberOfElements();
	int nLayers = col->parameters().getIntVal("NLayers");
	float energy = 0.f;
	
	streamlog_out(DEBUG3) << "Initial number of hits: " << nHits << std::endl;
	
	CellIDDecoder<CalorimeterHit> cd("I:5,J:5,K:4,CHP:4,CHN:6,SCA:4");

	std::map<int, std::map<int,std::vector<CalorimeterHit*>>> hitMap = {};
	
	for(int iHit = 0; iHit < nHits; iHit++) {
	  
	  CalorimeterHit* hit = dynamic_cast<CalorimeterHit*>(col->getElementAt(iHit));

	  if(hit->getEnergy() < 0) continue; 
	  
	  int I = (int)cd(hit)["I"];
	  int J = (int)cd(hit)["J"];
	  int K = (int)cd(hit)["K"];

	  histograms1D.at("K")->Fill(K);
	  
	  int chipID = 4*(J/8) + I/8;

	  energy += hit->getEnergy();
	  
	  hitMap[K][chipID].push_back(hit);

	}

	histograms1D.at("NHits")->Fill(nHits);
	histograms1D.at("NLayers")->Fill(nLayers);
	histograms1D.at("Energy")->Fill(energy);

	energy = 0.f;
	nHits = nLayers = 0;
	
	for(auto layer : hitMap) {

	  int nChips = 0;

	  if(layer.second.size() > 6) continue;
	  
	  for(auto chip : layer.second) {
	    histograms1D.at("NHitsInChip")->Fill(chip.second.size());
	    
	    if(chip.second.size() >= _noisyChipCut) {
	      int nHitsPre, nHitsPost;
	      nHitsPre = nHitsPost = 0;

	      if(hitMap.find(layer.first - 1) != hitMap.end() && hitMap.at(layer.first - 1).find(chip.first) != hitMap.at(layer.first - 1).end()) nHitsPre = hitMap.at(layer.first - 1).at(chip.first).size();
	      if(hitMap.find(layer.first + 1) != hitMap.end() && hitMap.at(layer.first + 1).find(chip.first) != hitMap.at(layer.first + 1).end()) nHitsPost = hitMap.at(layer.first + 1).at(chip.first).size();

	      histograms1D.at("NHitsInChipPre")->Fill(nHitsPre);
	      histograms1D.at("NHitsInChipPost")->Fill(nHitsPost);
	      histograms2D.at("NHitsPreVsPost")->Fill(nHitsPre,nHitsPost);

	      if(nHitsPre <= 2 && nHitsPost <= 2) continue;
	      if(nHitsPre == 0 && nHitsPost >= _noisyChipCut) return; // Check this two conditions in the case of showers;
	      if(nHitsPost == 0 && nHitsPre >= _noisyChipCut) return;

	      histograms1D.at("NHitsInChipPreCleanup")->Fill(nHitsPre);
	      histograms1D.at("NHitsInChipPostCleanup")->Fill(nHitsPost);
	      
	    }

	    histograms1D.at("NHitsInChipCleanup")->Fill(chip.second.size());
	    
	    nChips++;
	    for(auto hit : chip.second) {
	      energy += hit->getEnergy();
	      nHits++;

	      CalorimeterHitImpl* copyHit = new CalorimeterHitImpl(*(static_cast<CalorimeterHitImpl*>(hit)));
	      outcolnew->addElement(copyHit);
	      eventcolnew->addElement(hit);
    	      
	    }
	    
	  }

	  histograms1D.at("NChipsInLayer")->Fill(nChips);
	  if(nChips > 0) nLayers++;  
	}
	
	if(nLayers >= _nLayersScanCut ) {

	  nBeamParticles++;

	  histograms1D.at("NLayersBeam")->Fill(nLayers);
	  histograms1D.at("NHitsBeam")->Fill(nHits);
	  histograms1D.at("EnergyBeam")->Fill(energy);

	  LCEventImpl* evt = new LCEventImpl();
	  evt->setRunNumber(evtP->getRunNumber());
	  evt->setTimeStamp(evtP->getTimeStamp());
	  evt->setEventNumber(nBeamParticles);

	  evt->addCollection(outcolnew,_outColName);
	  evtP->addCollection(eventcolnew,_outColName);	

	  _lcWriter->writeEvent(evt);
	  delete evt;

	  streamlog_out(DEBUG) << "Beam particle found: " << evtnum << std::endl;
	  
	}
	else {
	  delete outcolnew;
	  delete eventcolnew;
	}

      }catch (lcio::DataNotAvailableException zero) { streamlog_out(DEBUG) << "ERROR READING COLLECTION: " << _ecalCollections[i] << std::endl;} 
    }//end loop over collection
  }
  
}


//==============================================================
void CleanupAndBeamSelector::end()
{

  writeHistograms();

  _lcWriter->close();  
  delete _lcWriter;
  
  _outputFile->Close();
  delete _outputFile;

  streamlog_out(DEBUG3) << "Ending First ECAL Processor" << std::endl;

}


void CleanupAndBeamSelector::writeHistograms() {

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

 
