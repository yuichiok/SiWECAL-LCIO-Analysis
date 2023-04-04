#include "MIPSelector.h"

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

MIPSelectorProc a_MIPSelectorProc;

//=========================================================
MIPSelectorProc::MIPSelectorProc()
  :Processor("MIPSelectorProc")
{
  
  //Input Collections    
  _ecalCollections.push_back(std::string("ECAL_BeamEvents"));
  registerInputCollections(LCIO::RAWCALORIMETERHIT, 
			   "ECALCollections",  
			   "ECAL Collection Names",  
			   _ecalCollections, 
			   _ecalCollections); 
  
  //Output file base name for muons
  _outFileNameMIPs="TB_ECALMIPEvents_";
  registerProcessorParameter("LCIOOutputFileMIPs", 
			     "Base LCIO file name",
			     _outFileNameMIPs,
			     _outFileNameMIPs);

  //Output collection name for muons
  _outColNameMIPs="ECAL_MIPEvents";
  registerProcessorParameter("OutputCollectionNameMIPs", 
			     "Name of the produced collection in this processor",
			     _outColNameMIPs,
			     _outColNameMIPs);

  
  //Output file base name for showers
  _outFileNameShowers="TB_ECALShowerEvents_";
  registerProcessorParameter("LCIOOutputFileShowers", 
			     "Base LCIO file name",
			     _outFileNameShowers,
			     _outFileNameShowers);

  //Output collection name
  _outColNameShowers="ECAL_ShowerEvents";
  registerProcessorParameter("OutputCollectionNameShowers", 
			     "Name of the produced collection in this processor",
			     _outColNameShowers,
			     _outColNameShowers);

  //Output ROOT name
  _logRootName = "LogROOT_MIPSelection";
  registerProcessorParameter("logRoot_Name" ,
                             "LogRoot name",
                             _logRootName,
			     _logRootName);

  //Density cut to select muons
  _densityCutMIPs = 3.0;
  registerProcessorParameter("DensityCutMIPs",
                             "Density cut to select muons or cosmic rays",
                             _densityCutMIPs,
			     _densityCutMIPs);

  /* //Density cut to select noise and cosmic rays in the detector
  _densityCutDiscarded = 2.5;
  registerProcessorParameter("DensityCutDiscarded",
                             "Density cut to select noise and cosmic rays in the detector",
                             _densityCutDiscarded,
			     _densityCutDiscarded);*/

  //Second maximum cut to select muons 
  _secondMaxCutMIPs = 10;
  registerProcessorParameter("SecondMaxCutMIPs",
                             "Second maximum cut to select muons ",
                             _secondMaxCutMIPs,
			     _secondMaxCutMIPs);

  /* //Second maximum cut to select noise or cosmic rays
  _secondMaxCutDiscarded = 10;
  registerProcessorParameter("SecondMaxCutDiscarded",
                             "Second maximum cut to select noise or comic rays",
                             _secondMaxCutDiscarded,
			     _secondMaxCutDiscarded);*/
    
} 

void MIPSelectorProc::init() {

     streamlog_out(DEBUG4) << "MIPSelector Init" << std::endl;

     _lcWriterMIPs = LCFactory::getInstance()->createLCWriter() ;
     _lcWriterMIPs->setCompressionLevel( 0 );

     _lcWriterShowers = LCFactory::getInstance()->createLCWriter() ;
     _lcWriterShowers->setCompressionLevel( 0 );

     _outputFile = new TFile((_logRootName + ".root").c_str(),"RECREATE");	  
     
     printParameters();
  
}

void MIPSelectorProc::processRunHeader( LCRunHeader* runHd )
{

  streamlog_out(MESSAGE) << "MIPSelector processing run: " << runHd->getRunNumber() << std::endl;

  if(runNumber != -1) {
    _lcWriterMIPs->close();
    _lcWriterShowers->close();
    writeHistograms();
  }
  runNumber = runHd->getRunNumber();

  // ---- Creation of the output files ----
  
  streamlog_out(DEBUG4) << "New lcio outputs" << std::endl;

  _lcWriterMIPs->open((_outFileNameMIPs + std::to_string(runNumber) + ".slcio").c_str(),LCIO::WRITE_NEW);
  _lcWriterMIPs->writeRunHeader(runHd);

  _lcWriterShowers->open((_outFileNameShowers + std::to_string(runNumber) + ".slcio").c_str(),LCIO::WRITE_NEW);
  _lcWriterShowers->writeRunHeader(runHd);
  
  streamlog_out(DEBUG4) << "Creating new local histograms" << std::endl;

  _outputFile->mkdir(std::to_string(runNumber).c_str());
  _outputFile->cd(std::to_string(runNumber).c_str());

  // ---- Creation of the histograms ----

  histograms1D["K"] = new TH1F("K", ";K;Entries", 15, 0., 15.);

  histograms1D["NHits"] = new TH1F("NHits", ";NHits;Entries", 250, 0., 2000.);
  histograms1D["NHitsMIPs"] = new TH1F("NHitsMIPs", ";NHits;Entries", 250, 0., 2000.);
  histograms1D["NHitsShowers"] = new TH1F("NHitsShowers", ";NHits;Entries", 250, 0., 2000.);
  histograms1D["NHitsDiscarded"] = new TH1F("NHitsDiscarded", ";NHits;Entries", 250, 0., 2000.);

  histograms1D["NLayers"] = new TH1F("NLayers", ";NLayers;Entries", 16, 0., 16.);

  histograms1D["Density"] = new TH1F("Density", ";Density(nHits/nLayers);Entries", 1000, 0., 100.);
  //histograms1D["DensityPC"] = new TH1F("DensityPC", ";Density(nHits/nLayers);Entries", 1000, 0., 100.);
  //histograms1D["DensityNoPC"] = new TH1F("DensityNoPC", ";Density(nHits/nLayers);Entries", 1000, 0., 100.);

  histograms1D["DensityMIPs"] = new TH1F("DensityMIPs", ";Density(nHits/nLayers);Entries", 1000, 0., 100.);
  histograms1D["DensityShowers"] = new TH1F("DensityShowers", ";Density(nHits/nLayers);Entries", 1000, 0., 100.);
  histograms1D["DensityDiscarded"] = new TH1F("DensityDiscarded", ";Density(nHits/nLayers);Entries", 1000, 0., 100.);

  histograms1D["FirstMax"] = new TH1F("FirstMax", ";FirstMax;Entries", 200, 0., 200.);
  
  histograms1D["SecondMax"] = new TH1F("SecondMax", ";SecondMax;Entries", 200, 0., 200.);
  //histograms1D["SecondMaxPC"] = new TH1F("SecondMaxPC", ";SecondMax;Entries", 200, 0., 200.);
  //histograms1D["SecondMaxNoPC"] = new TH1F("SecondMaxNoPC", ";SecondMax;Entries", 200, 0., 200.);

  histograms1D["SecondMaxMIPs"] = new TH1F("SecondMaxMIPs", ";SecondMax;Entries", 200, 0., 200.);
  histograms1D["SecondMaxShowers"] = new TH1F("SecondMaxShowers", ";SecondMax;Entries", 200, 0., 200.);
  histograms1D["SecondMaxDiscarded"] = new TH1F("SecondMaxDiscarded", ";SecondMax;Entries", 200, 0., 200.);

  histograms2D["DensityVsSecondMax"] = new TH2F("DensityVsSecondMax", ";Density;SecondMax", 1000, 0., 100., 200, 0., 200.);
  histograms2D["DensityVsSecondMaxMIPs"] = new TH2F("DensityVsSecondMaxMIPs", ";Density;SecondMax", 1000, 0., 100., 200, 0., 200.);
  histograms2D["DensityVsSecondMaxShowers"] = new TH2F("DensityVsSecondMaxShowers", ";Density;SecondMax", 1000, 0., 100., 200, 0., 200.);
  //histograms2D["DensityVsSecondMaxDiscarded"] = new TH2F("DensityVsSecondMaxDiscarded", ";Density;SecondMax", 1000, 0., 100., 200, 0., 200.);

  //histograms2D["DensityVsSecondMaxPC"] = new TH2F("DensityVsSecondPCMax", ";Density;SecondMax", 1000, 0., 100., 200, 0., 200.);
  //histograms2D["DensityVsSecondMaxNoPC"] = new TH2F("DensityVsSecondNoPCMax", ";Density;SecondMax", 1000, 0., 100., 200, 0., 200.);
  
  /*histograms1D["FirstQuarterLength"] = new TH1F("FirstQuarterLength", "FirstQuarter;NLayers;Entries", 20, 0., 20.);
  histograms1D["SecondQuarterLength"] = new TH1F("SecondQuarterLength", "FirstQuarter;NLayers;Entries", 20, 0., 20.);
  histograms1D["ThirdQuarterLength"] = new TH1F("ThirdQuarterLength", "FirstQuarter;NLayers;Entries", 20, 0., 20.);
  histograms1D["LastQuarterLength"] = new TH1F("LastQuarterLength", "FirstQuarter;NLayers;Entries", 20, 0., 20.);

  histograms1D["NLayersFirstQuarter"] = new TH1F("NLayersFirstQuarter", "FirstQuarter;NLayers;Entries", 20, 0., 20.);
  histograms1D["NLayersSecondQuarter"] = new TH1F("NLayersSecondQuarter", "SecondQuarter;NLayers;Entries", 20, 0., 20.);
  histograms1D["NLayersThirdQuarter"] = new TH1F("NLayersThirdQuarter", "ThirdQuarter;NLayers;Entries", 20, 0., 20.);
  histograms1D["NLayersLastQuarter"] = new TH1F("NLayersLastQuarter", "LastQuarter;NLayers;Entries", 20, 0., 20.);
  
  histograms1D["TrackLength"] = new TH1F("TrackLength", ";Length;Entries", 50, 0., 50.);
  
  histograms1D["EntryLayer"] = new TH1F("EntryLayer", ";Layer;Entries", 50, 0., 50.);
  histograms1D["EntryLayerPC"] = new TH1F("EntryLayerPC", ";Layer;Entries", 50, 0., 50.);
  histograms1D["EntryLayerNoPC"] = new TH1F("EntryLayerNoPC", ";Layer;Entries", 50, 0., 50.);

  histograms2D["EntryPoint"] = new TH2F("EntryPoint", ";I;J", 100, 0., 100., 100, 0., 100.);
  histograms2D["EntryPointPC"] = new TH2F("EntryPointPC", ";I;J", 100, 0., 100., 100, 0., 100.);
  histograms2D["EntryPointNoPC"] = new TH2F("EntryPointNoPC", ";I;J", 100, 0., 100., 100, 0., 100.);  
  */
  
  streamlog_out(DEBUG4) << "Creating systematics tools" << std::endl;
  
  // ---- Reset counters ----

  evtnum = nDiscarded = nMIPs = nShowers = 0;
  
}

void MIPSelectorProc::processEvent( LCEvent * evtP ) 
{

  streamlog_out(DEBUG4) << "Start MIPSelector process" << std::endl;

  if(evtP){ //Check the evtP
    for(unsigned int i=0; i < _ecalCollections.size(); i++){//loop over collections
      try{

	evtnum++;
	
	LCCollection* col = evtP->getCollection(_ecalCollections[i].c_str());
	int nHits = col->getNumberOfElements();
	
	CellIDDecoder<CalorimeterHit> cd("I:5,J:5,K:4,CHP:4,CHN:6,SCA:4");

	std::map<int,int> kDistribution = {};

	IMPL::LCCollectionVec* outcolnew = new IMPL::LCCollectionVec(LCIO::CALORIMETERHIT);        
	outcolnew->setDefault();
	outcolnew->setFlag(outcolnew->getFlag() | (1 << LCIO::RCHBIT_LONG));

	IMPL::LCCollectionVec* eventcolnew = new IMPL::LCCollectionVec(LCIO::CALORIMETERHIT);
	eventcolnew->setFlag(col->getFlag());
	eventcolnew->setSubset(true);
	
        //std::vector<float> trackParameters;
  
        //col->parameters().getFloatVals("TrackParameters",trackParameters);

        //outcolnew->parameters().setValues("TrackParameters", trackParameters);
        //eventcolnew->parameters().setValues("TrackParameters", trackParameters);	

	for(int iHit = 0; iHit < nHits; iHit++) {
	  
	  CalorimeterHit* hit = dynamic_cast<CalorimeterHit*>(col->getElementAt(iHit));

	  int K = (int)cd(hit)["K"];
	  kDistribution[K]++;

	  histograms1D.at("K")->Fill(K);
	  
	  CalorimeterHitImpl* copyHit = new CalorimeterHitImpl(*(static_cast<CalorimeterHitImpl*>(hit)));
	  outcolnew->addElement(copyHit);
	  eventcolnew->addElement(hit);
	  
	}

	/*int KLow, KLow_X, KLow_Y, KUp, KUp_X, KUp_Y;
	KLow_X = KLow_Y = 0;
	KUp_X = KUp_Y = 15;

	if(TMath::Abs(trackParameters.at(0)) > 0.0001) {
	     if(trackParameters.at(0) > 0) {
		  KLow_X = (int)TMath::Max((float)KLow_X, (float)(-trackParameters.at(1)/trackParameters.at(0)));
		  KUp_X = (int)TMath::Min((float)KUp_X, (float)((31-trackParameters.at(1))/trackParameters.at(0)));
	     }
	     else {
	       KLow_X = (int)TMath::Max((float)KLow_X, (float)((31-trackParameters.at(1))/trackParameters.at(0)));
		  KUp_X = (int)TMath::Min((float)KUp_X, (float)(-trackParameters.at(1)/trackParameters.at(0)));
	     }
	}

	streamlog_out(DEBUG4)<< "TrackParametersX: " << trackParameters.at(0) << " " << trackParameters.at(1) << std::endl;
	streamlog_out(DEBUG4)<< "KLow_X: " << KLow_X << " KUp_X: " << KUp_X << std::endl;

	if(TMath::Abs(trackParameters.at(2)) > 0.0001) {
	     if(trackParameters.at(2) > 0) {
		  KLow_Y = (int)TMath::Max((float)KLow_Y, (float)(-trackParameters.at(3)/(trackParameters.at(2)*28)));
		  KUp_Y = (int)TMath::Min((float)KUp_Y, (float)((96*10.4-trackParameters.at(3))/(trackParameters.at(2)*28)));
	     }
	     else {
		  KLow_Y = (int)TMath::Max((float)KLow_Y, (float)((96*10.4-trackParameters.at(3))/(trackParameters.at(2)*28)));
		  KUp_Y = (int)TMath::Min((float)KUp_Y, (float)(-trackParameters.at(3)/(trackParameters.at(2)*28)));
	     }
	}

	streamlog_out(DEBUG4)<< "TrackParametersY: " << trackParameters.at(2) << " " << trackParameters.at(3) << std::endl;
	streamlog_out(DEBUG4)<< "KLow_Y: " << KLow_Y << " KUp_Y: " << KUp_Y << std::endl;

	float xLow = trackParameters.at(0)*KLow_X + trackParameters.at(1);
	streamlog_out(DEBUG4)<< "xLow: " << xLow << std::endl;

	if(xLow < 0 || xLow > 96*10.4) {
	     KLow = KLow_Y;
	}
	else KLow = KLow_X;

	float yLow = trackParameters.at(2)*KLow + trackParameters.at(3); 

	histograms2D.at("EntryPoint")->Fill((int)(xLow/10.4),(int)(yLow/10.4));

	float xUp = trackParameters.at(2)*KUp_X + trackParameters.at(3);
	streamlog_out(DEBUG4)<< "xUp: " << xUp << std::endl;

	if(xUp < 0 || xUp > 96*10.4) {
	     KUp = KUp_Y;
	}
	else KUp = KUp_X;

	int trackLength = KUp - KLow;
        
	histograms1D.at("EntryLayer")->Fill(KLow);
	histograms1D.at("TrackLength")->Fill(trackLength);

	histograms1D.at("FirstQuarterLength")->Fill((int)trackLength*0.25);
	histograms1D.at("SecondQuarterLength")->Fill((int)trackLength*0.3);
	histograms1D.at("ThirdQuarterLength")->Fill((int)trackLength*0.31);
	histograms1D.at("LastQuarterLength")->Fill((int)trackLength*0.14);
	*/
	
	float density = 0.;
	
	int nLayers, nHitsFirstMax, nHitsSecondMax; //, nLayersFirstQuarter, nLayersSecondQuarter,  nLayersThirdQuarter, nLayersLastQuarter;
        nLayers = nHitsFirstMax = nHitsSecondMax  = 0; //= nLayersFirstQuarter = nLayersSecondQuarter =  nLayersThirdQuarter =  nLayersLastQuarter =0;

	for(auto kIt = kDistribution.begin(); kIt != kDistribution.end(); kIt++) {

	  nLayers++;

	  if(kIt->second > nHitsFirstMax) {
	    nHitsSecondMax = nHitsFirstMax;
	    nHitsFirstMax = kIt->second;
	  }
	  else if(kIt->second > nHitsSecondMax) nHitsSecondMax = kIt->second; 

	  /*
	  if(kIt->first >= KLow && kIt->first < KLow + (int)trackLength*0.25 ) nLayersFirstQuarter++; 
	  else if(kIt->first >= KLow + (int)trackLength*0.25 && kIt->first < KLow + (int)trackLength*0.55) nLayersSecondQuarter++;
	  else if(kIt->first >= KLow + (int)trackLength*0.55 && kIt->first < KLow + (int)trackLength*0.86) nLayersThirdQuarter++; 
	  else nLayersLastQuarter++;*/ 
	  
	}

	density = nHits/(float)nLayers;

	/*histograms1D.at("NLayersFirstQuarter")->Fill(nLayersFirstQuarter);
	histograms1D.at("NLayersSecondQuarter")->Fill(nLayersSecondQuarter);
	histograms1D.at("NLayersThirdQuarter")->Fill(nLayersThirdQuarter);
	histograms1D.at("NLayersLastQuarter")->Fill(nLayersLastQuarter);*/

	histograms1D.at("NHits")->Fill(nHits);
	histograms1D.at("Density")->Fill(density);
	histograms1D.at("FirstMax")->Fill(nHitsFirstMax);
	histograms1D.at("SecondMax")->Fill(nHitsSecondMax);

        histograms2D.at("DensityVsSecondMax")->Fill(density, nHitsSecondMax);	
	
	/*bool PC, PCUp, PCDown;
	PC = PCUp = PCDown= false;
	
        if(nLayersFirstQuarter >= (int)trackLength*0.16 && nLayersSecondQuarter >= (int)trackLength*0.24 && nLayersThirdQuarter >= (int)trackLength*0.22 && nLayersLastQuarter >= (int)trackLength*0.08 ) {
	  PC = true;
	  }*/
	
	//eventcolnew->parameters().setValue("PC", (int)PC);
	//eventcolnew->parameters().setValue("SecondMax", (int)nHitsSecondMax);

	LCEventImpl* evt = new LCEventImpl();
	evt->setRunNumber(evtP->getRunNumber());
	evt->setTimeStamp(evtP->getTimeStamp());
        
	/*if(PC) {
        histograms1D.at("EntryLayerPC")->Fill(KLow);
	histograms1D.at("DensityPC")->Fill(density);
	histograms1D.at("SecondMaxPC")->Fill(nHitsSecondMax);
	histograms2D.at("DensityVsSecondMaxPC")->Fill(density, nHitsSecondMax);
	histograms2D.at("EntryPointPC")->Fill((int)(xLow/10.4),(int)(yLow/10.4));*/

	if(density < _densityCutMIPs && nHitsSecondMax < _secondMaxCutMIPs) {

	  histograms1D.at("NLayers")->Fill(nLayers);
	  histograms1D.at("NHitsMIPs")->Fill(nHits);
	  histograms1D.at("DensityMIPs")->Fill(density);
	  histograms1D.at("SecondMaxMIPs")->Fill(nHitsSecondMax);
	  histograms2D.at("DensityVsSecondMaxMIPs")->Fill(density, nHitsSecondMax);
	  
	  evt->setEventNumber(++nMIPs);
	  evt->addCollection(outcolnew,_outColNameMIPs);
	  evtP->addCollection(eventcolnew,_outColNameMIPs);
		  
	  _lcWriterMIPs->writeEvent(evt);
	}
	else {
	  histograms1D.at("NHitsShowers")->Fill(nHits);
	  histograms1D.at("DensityShowers")->Fill(density);
	  histograms1D.at("SecondMaxShowers")->Fill(nHitsSecondMax);
	  histograms2D.at("DensityVsSecondMaxShowers")->Fill(density, nHitsSecondMax);
	  
	  evt->setEventNumber(++nShowers);
	  evt->addCollection(outcolnew,_outColNameShowers);
	  evtP->addCollection(eventcolnew,_outColNameShowers);
		  
	  _lcWriterShowers->writeEvent(evt);
	}
	/*}
	else {
	  histograms1D.at("DensityNoPC")->Fill(density);
	  histograms1D.at("SecondMaxNoPC")->Fill(nHitsSecondMax);
	  histograms2D.at("DensityVsSecondMaxNoPC")->Fill(density, nHitsSecondMax);  
	  histograms2D.at("EntryPointNoPC")->Fill((int)(xLow/10.4),(int)(yLow/10.4));

	  nHitsMIPs = -999;

	  if(density < _densityCutDiscarded && nHitsSecondMax < _secondMaxCutDiscarded) {
	       nHitsShowersNotDiscarded = -999;
	  
	       histograms1D.at("EntryLayerNoPC")->Fill(KLow);
	       histograms1D.at("NHitsDiscarded")->Fill(nHits);
	       histograms1D.at("DensityDiscarded")->Fill(density);
	       histograms1D.at("SecondMaxDiscarded")->Fill(nHitsSecondMax);
	       histograms2D.at("DensityVsSecondMaxDiscarded")->Fill(density, nHitsSecondMax);
	       nDiscarded++;
	  }
	  else {
	       histograms1D.at("NHitsShowers")->Fill(nHits);
	       histograms1D.at("DensityShowers")->Fill(density);
	       histograms1D.at("SecondMaxShowers")->Fill(nHitsSecondMax);
	       histograms2D.at("DensityVsSecondMaxShowers")->Fill(density, nHitsSecondMax);
	  
	       evt->setEventNumber(++nShowers);
	       evt->addCollection(outcolnew,_outColNameShowers);
	       evtP->addCollection(eventcolnew,_outColNameShowers);
	  
	       _lcWriterShowers->writeEvent(evt);
	  }
	  }*/
	
	delete evt;

      }catch (lcio::DataNotAvailableException zero) { streamlog_out(DEBUG) << "ERROR READING COLLECTION: " << _ecalCollections[i] << std::endl;} 
    }//end loop over collection
  }
  
}


//==============================================================
void MIPSelectorProc::end()
{
  
  _lcWriterMIPs->close();  
  delete _lcWriterMIPs;
  _lcWriterShowers->close();  
  delete _lcWriterShowers;
  
  writeHistograms();
  
  _outputFile->Close();
  delete _outputFile;

  streamlog_out(DEBUG4) << "Ending MIPSelector" << std::endl;

}


void MIPSelectorProc::writeHistograms() {

  _outputFile->cd(std::to_string(runNumber).c_str());
  
  TPaveText stats(0.05, 0.1, 0.95, 0.9);
  stats.AddText(std::to_string(runNumber).c_str());
  stats.AddText(("Percentage of MIPs:" + std::to_string(nMIPs*100/(float)evtnum) + "%").c_str());
  stats.AddText(("Percentage of Showers:" + std::to_string(nShowers*100/(float)evtnum) + "%").c_str());
  stats.AddText(("Percentage of Discarded:" + std::to_string(nDiscarded*100/(float)evtnum) + "%").c_str());
  stats.Write();

  for(auto It1D = histograms1D.begin(); It1D != histograms1D.end(); It1D++) {
    It1D->second->Write();
    delete It1D->second;
  }

  for(auto It2D = histograms2D.begin(); It2D != histograms2D.end(); It2D++) {
    It2D->second->Write();
    delete It2D->second;
  }
  
}

 
