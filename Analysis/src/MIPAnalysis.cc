 #include "MIPAnalysis.h"

#include <marlin/Exceptions.h>

#include <UTIL/CellIDDecoder.h>

#include <EVENT/LCCollection.h>
#include <EVENT/CalorimeterHit.h>

#include <TCanvas.h>
#include <TF1.h>

#include <iostream>

#include <TMath.h>

MIPAnalysisProc a_MIPAnalysisProc;

//=========================================================
MIPAnalysisProc::MIPAnalysisProc()
  :Processor("MIPAnalysisProc")
{
  
  //Input Collections    
  _ecalCollections.push_back(std::string("ECAL_MIPEvents"));
  registerInputCollections(LCIO::RAWCALORIMETERHIT, 
			   "ECALCollections",  
			   "ECAL Collection Names",  
			   _ecalCollections, 
			   _ecalCollections); 
  
  //Output ROOT name
  _logRootName = "LogROOT_MIPAnalysis";
  registerProcessorParameter("logRoot_Name" ,
                             "LogRoot name",
                             _logRootName,
			     _logRootName);

  //Alignment file name
  _alignmentFileName = "MIPAlignment.txt";
  registerProcessorParameter("AlignmentFileName" ,
                             "Name of the MIP Alignment file",
                             _alignmentFileName,
			     _alignmentFileName);

  //Dead cells file name
  _deadCellsFileName = "../eventbuilding/DeadCellsMapping.txt";
  registerProcessorParameter("DeadCellsFileName",
                             "Name of the MIP deadCells file",
                             _deadCellsFileName,
			     _deadCellsFileName);
  
  //MIPs Per Run
  _MIPsPerRun = 10000;
  registerProcessorParameter("MIPsPerRun" ,
                             "Number of MIPs to analyze per run",
                             _MIPsPerRun,
			     _MIPsPerRun);

  /*//Input angles to search the MIPs
  _refAngles = {};
  registerProcessorParameter("InputAngles" ,
                             "Angles of reference to search the MIPs",
                             _refAngles,
			     _refAngles);*/

  //Scan window
  _scan = 2;
  registerProcessorParameter("ScanWindow" ,
                             "ScanWindow",
                             _scan,
			     _scan);

  //KGap
  kGap = 13;
  registerProcessorParameter("KGap" ,
                             "KGap",
                             kGap,
			     kGap);

} 

void MIPAnalysisProc::init() {

     streamlog_out(DEBUG4) << "MIPAnalysis Init" << std::endl;

     _outputFile = new TFile((_logRootName + ".root").c_str(),"RECREATE");	  

     /*globalHistograms1D["SlopeX"] = new TH1F("GlobalSlopeX", ";SlopeX;Entries", 1000, -5., 5.);
     globalHistograms1D["SlopeXErr"] = new TH1F("GlobalSlopeXErr", ";SlopeXErr;Entries", 1000, 0., 0.01);
     globalHistograms1D["ParXErr"] = new TH1F("GlobalParXErr", ";ParXErr;Entries", 1000, 0., 2.0);
     globalHistograms1D["ChiNDFX"] = new TH1F("GlobalChiNDFX", ";CHiNDFX;Entries", 500, 0., 200.);

     globalHistograms1D["SlopeY"] = new TH1F("GlobalSlopeY", ";SlopeY;Entries", 1000, -5., 5.);
     globalHistograms1D["ChiNDFY"] = new TH1F("GlobalChiNDFY", ";ChiNDFY;Entries", 500, 0., 200.);
     globalHistograms1D["SlopeYErr"] = new TH1F("GlobalSlopeYErr", ";SlopeYErr;Entries", 1000, 0., 0.01);
     globalHistograms1D["ParYErr"] = new TH1F("GlobalParYErr", ";ParYErr;Entries", 1000, 0., 2.0);
     */

     std::cout << "----- ALIGNMENT -----" << std::endl;
     
     alignmentFile.open(_alignmentFileName.c_str());
     if(!alignmentFile.is_open()) {
       std::cout << "Could not open alignment file: " << _alignmentFileName << std::endl;
       throw(marlin::StopProcessingException(&a_MIPAnalysisProc));
     }
     
     std::string tmp_hdr, tmp_lay, tmp_alignX, tmp_alignXErr, tmp_alignY, tmp_alignYErr;

     std::getline(alignmentFile, tmp_hdr);
     while(alignmentFile) {
       alignmentFile >> tmp_lay >> tmp_alignX >> tmp_alignXErr >> tmp_alignY >> tmp_alignYErr;

       alignmentMap[std::stoi(tmp_lay)] = std::pair<std::pair<float,float>,std::pair<float,float>>(std::pair<float,float>(std::stof(tmp_alignX),std::stof(tmp_alignXErr)), std::pair<float,float>(std::stof(tmp_alignY), std::stof(tmp_alignYErr)));

     }

     alignmentFile.close();
     
     std::cout << "----- DEADCELLS -----" << std::endl;

     deadCellsFile.open(_deadCellsFileName.c_str());

     std::getline(deadCellsFile,tmp_hdr);
     if(!deadCellsFile.is_open()) {
       throw(marlin::StopProcessingException(&a_MIPAnalysisProc));
     }
     
     std::string tmp_I, tmp_J, tmp_Dead;

     std::getline(alignmentFile,tmp_hdr);
     while(deadCellsFile) {
       deadCellsFile >> tmp_lay >> tmp_I >> tmp_J >> tmp_Dead;
       
       int layer = std::stoi(tmp_lay);
       int I = std::stoi(tmp_I);
       int J = std::stoi(tmp_J);
       int dead = std::stoi(tmp_Dead);

       deadCellsMap[layer][std::pair<int,int>(I,J)] = dead;
       
     }

     deadCellsFile.close();
     
     nMIPsTotal = nMIPsRun = 0;
     
     printParameters();
}


void MIPAnalysisProc::processRunHeader( LCRunHeader* runHd )
{

  streamlog_out(MESSAGE) << "MIPAnalysis processing run: " << runHd->getRunNumber() << std::endl;

  if(runNumber != -1) {
    computeEffAndMult("Run");
    writeHistograms();
    std::cout << "Analyzed MIP in this run: " << nMIPsRun << std::endl;
  }
  runNumber = runHd->getRunNumber();

  // ---- Creation of the histograms ----

  streamlog_out(DEBUG4) << "Creating new local histograms" << std::endl;

  _outputFile->mkdir(std::to_string(runNumber).c_str());

  for(int iLayer = 0; iLayer < 15; iLayer++) {
    
    histograms1D[("X_Layer" + std::to_string(iLayer)).c_str()] = new TH1F(("X_Layer" + std::to_string(iLayer)).c_str(),";X;Entries", 256*2, -86.5*2, 86.5*2);
    histograms1D[("Y_Layer" + std::to_string(iLayer)).c_str()] = new TH1F(("Y_Layer" + std::to_string(iLayer)).c_str(),";Y;Entries", 256*2, -86.5*2, 86.5*2);

    histograms1D[("I_Layer" + std::to_string(iLayer)).c_str()] = new TH1F(("I_Layer" + std::to_string(iLayer)).c_str(),";I;Entries", 64, -32., 32.);
    histograms1D[("J_Layer" + std::to_string(iLayer)).c_str()] = new TH1F(("J_Layer" + std::to_string(iLayer)).c_str(),";J;Entries", 64, -32., 32.);

    histograms1D[("TrackX_Layer" + std::to_string(iLayer)).c_str()] = new TH1F(("TrackX_Layer" + std::to_string(iLayer)).c_str(),";X;Entries", 256*2, -86.5*2, 86.5*2);
    histograms1D[("TrackY_Layer" + std::to_string(iLayer)).c_str()] = new TH1F(("TrackY_Layer" + std::to_string(iLayer)).c_str(),";Y;Entries", 256*2, -86.5*2, 86.5*2);

    histograms1D[("TrackI_Layer" + std::to_string(iLayer)).c_str()] = new TH1F(("TrackI_Layer" + std::to_string(iLayer)).c_str(),";I;Entries", 64, -32., 32.);
    histograms1D[("TrackJ_Layer" + std::to_string(iLayer)).c_str()] = new TH1F(("TrackJ_Layer" + std::to_string(iLayer)).c_str(),";J;Entries", 64, -32., 32.);

    
  }

  histograms1D["SlopeX"] = new TH1F("SlopeX", ";SlopeX;Entries", 100, -5., 5.);
  histograms1D["SlopeXErr"] = new TH1F("SlopeXErr", ";SlopeXErr;Entries", 100, 0., 0.01);
  histograms1D["ParXErr"] = new TH1F("ParXErr", ";ParXErr;Entries", 100, 0., 2.0);
  histograms1D["ChiNDFX"] = new TH1F("ChiNDFX", ";CHiNDFX;Entries", 1000, 0., 1000.);

  histograms1D["SlopeY"] = new TH1F("SlopeY", ";SlopeY;Entries", 100, -5., 5.);
  histograms1D["ChiNDFY"] = new TH1F("ChiNDFY", ";ChiNDFY;Entries", 1000, 0., 1000.);
  histograms1D["SlopeYErr"] = new TH1F("SlopeYErr", ";SlopeYErr;Entries", 100, 0., 0.01);
  histograms1D["ParYErr"] = new TH1F("ParYErr", ";ParYErr;Entries", 100, 0., 2.0);
  
  // ---- Reset counters and maps ----

  nMIPsTotal += nMIPsRun;
  evtnum = nMIPsRun = 0;
  
  runHitEff.clear();
  runMult.clear();

}

void MIPAnalysisProc::processEvent( LCEvent * evtP ) 
{

  streamlog_out(DEBUG4) << "Start MIPAnalysis process" << std::endl;

  if(nMIPsRun > _MIPsPerRun && _MIPsPerRun > 0) return;

  if(evtP){ //Check the evtP
    for(unsigned int i=0; i < _ecalCollections.size(); i++){//loop over collections
      try{

	evtnum++;
	
	LCCollection* col = evtP->getCollection(_ecalCollections[i].c_str());
	int nHits = col->getNumberOfElements();
	
	CellIDDecoder<CalorimeterHit> cd("I:5,J:5,K:4,CHP:4,CHN:6,SCA:4");

	std::map<int,std::map<std::pair<int,int>,std::pair<float,float>>> hitMap = {}; 
	std::map<int, std::pair<int,std::pair<float,float>>> trackMap = {};
	
	for(int iHit = 0; iHit < nHits; iHit++) {
	  
	  CalorimeterHit* hit = dynamic_cast<CalorimeterHit*>(col->getElementAt(iHit));

	  const float* pos = hit->getPosition();
	  
	  int K = (int)cd(hit)["K"];
	  int I = (int)cd(hit)["I"] - 16;
	  int J = (int)cd(hit)["J"] - 16;

	  trackMap[K].first++;
	  trackMap[K].second.first += pos[0] - alignmentMap.at(K).first.first;
	  trackMap[K].second.second += pos[1] - alignmentMap.at(K).second.first;
	  
	  hitMap[K][std::pair<int,int>(I,J)] = std::pair<float,float>(pos[0] - alignmentMap.at(K).first.first,pos[1] - alignmentMap.at(K).second.first);

	  histograms1D.at(("X_Layer" + std::to_string(K)).c_str())->Fill(pos[0] - alignmentMap.at(K).first.first);
	  histograms1D.at(("Y_Layer" + std::to_string(K)).c_str())->Fill(pos[1] - alignmentMap.at(K).second.first);

	  histograms1D.at(("I_Layer" + std::to_string(K)).c_str())->Fill(I);
	  histograms1D.at(("J_Layer" + std::to_string(K)).c_str())->Fill(J);

	}

	for(int iLayer = 0; iLayer < 15; iLayer++) {

	  std::vector<float> x, y, z;
	  x = y = z = {};
	  
	  for(auto layerIt = trackMap.begin(); layerIt != trackMap.end(); layerIt++) {
	    if(layerIt->first == iLayer) continue;
	    
	    float xMean = layerIt->second.second.first/(float)layerIt->second.first;
	    float yMean = layerIt->second.second.second/(float)layerIt->second.first;
		 
	    float xVal, yVal;
	    xVal = yVal = 0;
	    int NHits = 0;

	    for(auto hit : hitMap.at(layerIt->first)) {

	      if(TMath::Abs(hit.second.first - xMean) > 8*5.5 && TMath::Abs(hit.second.second - yMean) > 8*5.5) continue;

	      NHits++;
	      xVal += hit.second.first;
	      yVal += hit.second.second;
	      
	    }
 
	    xVal /= NHits;
	    yVal /= NHits;
	    x.push_back(xVal);
	    y.push_back(yVal);
	    z.push_back(layerIt->first*kGap);		  

	  }

	  if(x.size() < 3) continue;

	  streamlog_out(DEBUG) << "Fitting tracks" << std::endl;
	  
	  TGraph *trackXZ = new TGraph(z.size(), z.data(), x.data());
	  trackXZ->Fit("pol1", "Q");
	  double* trackParametersX = (trackXZ->GetFunction("pol1"))->GetParameters();
	  double chiNDFX = (trackXZ->GetFunction("pol1"))->GetChisquare()/(trackXZ->GetFunction("pol1"))->GetNDF();
  
	  TGraph *trackYZ = new TGraph(z.size(), z.data(), y.data());
	  trackYZ->Fit("pol1", "Q");
	  double* trackParametersY = (trackYZ->GetFunction("pol1"))->GetParameters();
	  double chiNDFY = (trackYZ->GetFunction("pol1"))->GetChisquare()/(trackYZ->GetFunction("pol1"))->GetNDF();
	  
	  histograms1D.at("SlopeX")->Fill(trackParametersX[1]);
	  histograms1D.at("ChiNDFX")->Fill(chiNDFX);
	  histograms1D.at("SlopeXErr")->Fill((trackXZ->GetFunction("pol1"))->GetParErrors()[1]);
	  histograms1D.at("ParXErr")->Fill((trackXZ->GetFunction("pol1"))->GetParErrors()[0]);
	  
	  histograms1D.at("SlopeY")->Fill(trackParametersY[1]);
	  histograms1D.at("ChiNDFY")->Fill(chiNDFY);
	  histograms1D.at("SlopeYErr")->Fill((trackYZ->GetFunction("pol1"))->GetParErrors()[1]);
	  histograms1D.at("ParYErr")->Fill((trackYZ->GetFunction("pol1"))->GetParErrors()[0]);
	  
	    
	  std::pair<float,float> refTrack = { trackParametersX[0] + trackParametersX[1]*iLayer*kGap, trackParametersY[0] + trackParametersY[1]*iLayer*kGap};
	  
	    
	    /*float std_X, std_Y;
	     std_X = std_Y = 0.;
	     
	     int nHitsInSTD = 0;

	     for(auto layerIt = hitMap.begin(); layerIt != hitMap.end(); layerIt++) {
		  if(layerIt->first == iLayer) continue;
		  
		  for(auto hitIt : layerIt->second) {

		    if(TMath::Abs(hitIt.second.first - track.first) > 8*5.5 && TMath::Abs(hitIt.second.second - track.second) > 8*5.5) continue;
		    
		    std_X += (hitIt.second.first - refTrack.first)* (hitIt.second.first - refTrack.first);
		    std_Y += (hitIt.second.second - refTrack.second)* (hitIt.second.second - refTrack.second);
		    nHitsInSTD++;
		  }
	     }
	     
	     std_X = TMath::Sqrt(std_X/nHitsInSTD);
	     std_Y = TMath::Sqrt(std_Y/nHitsInSTD);

	     histograms1D.at("STD_X")->Fill(std_X);
	     histograms1D.at("STD_Y")->Fill(std_Y);

	     if(std_X > 15.0 || std_Y > 15.0) continue;*/

	  streamlog_out(DEBUG3) << "Computing Track I and J" << std::endl;
	  
	     int trackI, trackJ;
	     float dx = 0.f;
	     if(iLayer == fev13Layers[0] || iLayer == fev13Layers[1]) dx = cobDx;
	  
	     
	     if(refTrack.first - dx + centerDx < 0) {
	       trackI = (int)((refTrack.first - dx + centerDx)/5.5) - 1;	       
	     }
	     else {
	       trackI = (int)((refTrack.first - dx - centerDx)/5.5);
	     }

	     if(refTrack.second + centerDx < 0) {
	       trackJ = (int)((refTrack.second + centerDx)/5.5) - 1;	       
	     }
	     else {
	       trackJ = (int)((refTrack.second - centerDx)/5.5);
	     }

	     
	     histograms1D.at(("TrackX_Layer" + std::to_string(iLayer)).c_str())->Fill(refTrack.first);
	     histograms1D.at(("TrackY_Layer" + std::to_string(iLayer)).c_str())->Fill(refTrack.second);
	     
	     histograms1D.at(("TrackI_Layer" + std::to_string(iLayer)).c_str())->Fill(trackI);
	     histograms1D.at(("TrackJ_Layer" + std::to_string(iLayer)).c_str())->Fill(trackJ);

	     if(trackI < -16 || trackI > 15 || trackJ < -16 || trackJ > 15) {
	       if((iLayer != fev13Layers[0] && iLayer != fev13Layers[1]) || trackI <= 15) { 
		 streamlog_out(DEBUG) << "Track Pads outside of boundaries: " << trackI << " " << trackJ << ". Layer = " << iLayer << " Track X: " << refTrack.first << " Track Y: " << refTrack.second << std::endl;
	       }
	       continue;
	     }
	     
	     unsigned int trackHitID = iLayer*1536 + (trackJ + 16)*32 + trackI + 16;

	     runHitEff[trackHitID].first++;
	     detHitEff[trackHitID].first++;
	     if(hitMap.find(iLayer) == hitMap.end()) continue;
	     
	     if(!deadCellsMap.at(iLayer).at(std::pair<int,int>(trackI + 16,trackJ + 16))) {
	       techEff[trackHitID].first++;
	     }
	     
	     
	     bool isHitEff = false;
	     std::pair<int,int> foundHit;

	     for(auto hit : hitMap.at(iLayer)) {

	       if(TMath::Abs(hit.second.first - refTrack.first) <= 5.5*_scan && TMath::Abs(hit.second.second - refTrack.second) <= 5.5*_scan) {

		 isHitEff = true;
		 foundHit.first = hit.first.first;
		 foundHit.second = hit.first.second;
		 runHitEff[trackHitID].second++;
		 detHitEff[trackHitID].second++;
		 if(!deadCellsMap.at(iLayer).at(std::pair<int,int>(trackI + 16,trackJ + 16))) {
		   techEff[trackHitID].second++;
		 }
		 break;
	       }

	     }

	     if(isHitEff) {

		  std::vector<std::pair<int,int>> cluster = { foundHit };
		  for(auto hit : cluster) {
		       if(hitMap.at(iLayer).find(std::pair<int,int>(hit.first - 1,hit.second)) != hitMap.at(iLayer).end() && std::find(cluster.begin(),cluster.end(),std::pair<int,int>(hit.first - 1,hit.second)) == cluster.end()) cluster.push_back(std::pair<int,int>(hit.first - 1,hit.second));
		       if(hitMap.at(iLayer).find(std::pair<int,int>(hit.first + 1,hit.second)) != hitMap.at(iLayer).end() && std::find(cluster.begin(),cluster.end(),std::pair<int,int>(hit.first + 1,hit.second)) == cluster.end()) cluster.push_back(std::pair<int,int>(hit.first + 1,hit.second));
		       if(hitMap.at(iLayer).find(std::pair<int,int>(hit.first,hit.second - 1)) != hitMap.at(iLayer).end() && std::find(cluster.begin(),cluster.end(),std::pair<int,int>(hit.first,hit.second - 1)) == cluster.end()) cluster.push_back(std::pair<int,int>(hit.first,hit.second - 1));
		       if(hitMap.at(iLayer).find(std::pair<int,int>(hit.first,hit.second + 1)) != hitMap.at(iLayer).end() && std::find(cluster.begin(),cluster.end(),std::pair<int,int>(hit.first,hit.second + 1)) == cluster.end()) cluster.push_back(std::pair<int,int>(hit.first,hit.second + 1));
		  }
		  
		  runMult[trackHitID] += cluster.size();
		  detMult[trackHitID] += cluster.size();
	     }
		  
	  } // End loop over layers

	  nMIPsRun++;
	
      }catch (lcio::DataNotAvailableException zero) { streamlog_out(DEBUG) << "ERROR READING COLLECTION: " << _ecalCollections[i] << std::endl;} 
    }//end loop over collection
  }
  
}


//==============================================================
void MIPAnalysisProc::end()
{

  nMIPsTotal += nMIPsRun;
  std::cout << "NMIPsEndTotal: " << nMIPsTotal << std::endl;

  computeEffAndMult("Run");
  computeEffAndMult("Tech");
  computeEffAndMult("Total");
  writeHistograms();

  _outputFile->cd();

  for(auto gHist : globalHistograms1D) {
       gHist.second->Write();
       delete gHist.second;
  }

  globalHistograms1D.clear();

  _outputFile->Close();
  delete _outputFile;
  
  streamlog_out(DEBUG4) << "Ending MIPAnalysis" << std::endl;

}

void MIPAnalysisProc::computeEffAndMult(std::string type) {

     std::map<int, std::pair<int,int>>* effMap;
     std::map<int,int>* multMap;

     std::string prefix = "";

     if(type == "Run") {
	  effMap = &runHitEff;
	  multMap = &runMult;
	  prefix = std::to_string(runNumber) + "/" + type;
     }
     else if(type == "Total") {
	  effMap = &detHitEff;
	  multMap = &detMult;
	  prefix = type;	  
     }
     else if(type == "Tech") {
       effMap = &techEff;
       multMap = &detMult;
       prefix = type;
     }
     else {
	  std::cout << "Wrong type computing the efficiencies and multiplicities: " << type << std::endl;
	  return;
     }

     streamlog_out(DEBUG2) << "Creating Efficiency and Multiplicity folders" << std::endl;

     _outputFile->cd();
     _outputFile->mkdir((prefix + "Stats").c_str());
     _outputFile->mkdir((prefix + "ChipStatBoxes").c_str());
     _outputFile->mkdir((prefix + "ChipEfficiencies").c_str());
     _outputFile->mkdir((prefix + "ChipMultiplicities").c_str());
     _outputFile->mkdir((prefix + "HitEfficiencies").c_str());

     std::map<int,std::pair<int,float>> layerEfficiency = {};
     std::map<int,std::pair<int,float>> layerMultiplicity = {};

     std::map<int,std::pair<int,float>> ChipEfficiency = {};
     std::map<int,std::pair<int,float>> ChipMultiplicity = {};
        
     for(auto hit : *effMap) {
	  
	  int K = (int)hit.first/(1536);

	  int DI = 0;
	  if(K == fev13Layers[0] || K == fev13Layers[1]) DI = 11;
	  
	  int I = (int)(hit.first%(1536))%32 - DI;
	  int J = (int)(hit.first%(1536))/32;

	  /*std::cout << " ----- K = " << K << std::endl;
	  std::cout << " ----- I = " << I << std::endl;
	  std::cout << " ----- J = " << J << std::endl;
	  */
	  // ---- Layer efficiencies ----

	  streamlog_out(DEBUG2) << "Computing efficiencies in layer: " << K << std::endl;
	 
	  layerEfficiency[K].first++;
	  layerEfficiency[K].second += hit.second.second/(float)hit.second.first;

	  // ---- Layer stats ----

	  streamlog_out(DEBUG2) << "Computing stats in layer: " << K << std::endl;
	  
	  if(histograms2D.find((type + "Stats_Layer" + std::to_string(K)).c_str()) == histograms2D.end()) histograms2D[(type + "Stats_Layer" + std::to_string(K)).c_str()] = new TH2F((type + "Stats_Layer" + std::to_string(K)).c_str(), ";I;J", 32, 0., 32., 32, 0., 32.);	 
	  
	  histograms2D.at((type + "Stats_Layer" + std::to_string(K)).c_str())->SetBinContent(I + 1,J + 1, hit.second.first);

	  // ---- Chip efficiencies ---- 

	  streamlog_out(DEBUG2) << "Computing Chip efficiencies" << std::endl;

	  int ChipID = K*16 + (J/8)*4 + I/8;
	    	  
	  ChipEfficiency[ChipID].first++;
	  ChipEfficiency[ChipID].second += hit.second.second/(float)hit.second.first;

	  // ---- HitEfficiencies ----

	  streamlog_out(DEBUG2) << "Computing hit efficiencies" << std::endl;

	  if(histograms2D.find((type + "HitEfficiency_Layer" + std::to_string(K)).c_str()) == histograms2D.end()) histograms2D[(type + "HitEfficiency_Layer" + std::to_string(K)).c_str()] = new TH2F((type + "HitEfficiency_Layer" + std::to_string(K)).c_str(), ";I;J", 32, 0., 32., 32, 0., 32.);	 
	  
	  histograms2D[(type + "HitEfficiency_Layer" + std::to_string(K)).c_str()]->SetMaximum(100.);
	  histograms2D[(type + "HitEfficiency_Layer" + std::to_string(K)).c_str()]->SetMinimum(-0.01);

	  histograms2D.at((type + "HitEfficiency_Layer" + std::to_string(K)).c_str())->SetBinContent(I + 1,J + 1,hit.second.second*100/(float)hit.second.first);

	  // ---- Multiplicities ----

	  streamlog_out(DEBUG2) << "Computing multiplicities" << std::endl;

	  if(hit.second.second != 0) {

	       layerMultiplicity[K].first++;
	       layerMultiplicity[K].second += multMap->at(hit.first)/(float)hit.second.second;

	       streamlog_out(DEBUG2) << "ChipID: " << ChipID << std::endl;
	       
	       ChipMultiplicity[ChipID].first++;
	       ChipMultiplicity[ChipID].second += multMap->at(hit.first)/(float)hit.second.second;
	  }

     }
     
     // ---- Chip Efficiencies ----

     streamlog_out(DEBUG2) << "Computing Chip Efficiencies" << std::endl;

     for(auto asic : ChipEfficiency) {

	  int K = (int)asic.first/16;
	  int ChipI = (asic.first%16)%4;
	  int ChipJ = (asic.first%16)/4;

	  streamlog_out(DEBUG2) << "K = " << K << std::endl;
	  
	  statBoxes[(type + "ChipPerct_" + std::to_string(asic.first)).c_str()] = new TPaveText(ChipI + 0.5, ChipJ + 0.5, ChipI + 1, ChipJ + 1);
	  statBoxes[(type + "ChipPerct_" + std::to_string(asic.first)).c_str()]->SetName(("Chip_" + std::to_string(asic.first)).c_str());

	  statBoxes[(type + "ChipPerct_" + std::to_string(asic.first)).c_str()]->AddText((std::to_string(asic.second.first*100/(float)layerEfficiency.at(K).first) + "%").c_str());
	       
	  if(histograms2D.find((type + "ChipEfficiency_Layer" + std::to_string(K)).c_str()) == histograms2D.end()) histograms2D[(type + "ChipEfficiency_Layer" + std::to_string(K)).c_str()] = new TH2F((type + "ChipEfficiency_Layer" + std::to_string(K)).c_str(), ";ChipI;ChipJ", 4, 0., 4., 4, 0., 4.);
	  histograms2D.at((type + "ChipEfficiency_Layer" + std::to_string(K)).c_str())->SetMaximum(100.);
	  histograms2D.at((type + "ChipEfficiency_Layer" + std::to_string(K)).c_str())->SetMinimum(-0.01);
	  histograms2D.at((type + "ChipEfficiency_Layer" + std::to_string(K)).c_str())->SetBinContent(ChipI + 1,ChipJ + 1,asic.second.second*100/(float)asic.second.first);
	  
     }

     // ---- Chip Multiplicities ----

     streamlog_out(DEBUG2) << "Computing Chip Multiplicities" << std::endl;
     
     for(auto asic : ChipMultiplicity) {

	  int K = (int)asic.first/16;
	  int ChipI = (asic.first%16)%4;
	  int ChipJ = (asic.first%16)/4;
	       
	  if(histograms2D.find((type + "ChipMultiplicity_Layer" + std::to_string(K)).c_str()) == histograms2D.end()) histograms2D[(type + "ChipMultiplicity_Layer" + std::to_string(K)).c_str()] = new TH2F((type + "ChipMultiplicity_Layer" + std::to_string(K)).c_str(), ";ChipI;ChipJ", 4, 0., 4., 4, 0., 4.);
	  histograms2D.at((type + "ChipMultiplicity_Layer" + std::to_string(K)).c_str())->SetMaximum(5.);
	  histograms2D.at((type + "ChipMultiplicity_Layer" + std::to_string(K)).c_str())->SetMinimum(-0.01);

	  histograms2D.at((type + "ChipMultiplicity_Layer" + std::to_string(K)).c_str())->SetBinContent(ChipI + 1,ChipJ + 1,asic.second.second/(float)asic.second.first);
	  
     }

     // ---- LayerEfficiencies ----

     std::vector<float> k, eff, mult, stats;
     k = eff = mult = stats = {};

     float meanLayerEff, meanLayerMult;
     meanLayerEff = meanLayerMult = 0.;

     int nValidMult = 0;	  

     for(auto layer : layerEfficiency) {
	  k.push_back(layer.first);
	  eff.push_back(layer.second.second*100/(float)layer.second.first);
	  if(layerMultiplicity.find(layer.first) != layerMultiplicity.end()) {
	       mult.push_back(layerMultiplicity.at(layer.first).second/(float)layerMultiplicity.at(layer.first).first);
	       nValidMult++;
	      
	       meanLayerMult += layerMultiplicity.at(layer.first).second/(float)layerMultiplicity.at(layer.first).first;
	  }
	  else { mult.push_back(0.); }
	  stats.push_back(layer.second.first);
	  
	  meanLayerEff += layer.second.second*100/(float)layer.second.first;
     }

     if(k.size() == 0) return;

     meanLayerEff /= k.size();
     meanLayerMult /= nValidMult;

     float effStd, multStd;
     effStd = multStd = 0;

     for(auto layer : layerEfficiency) {
	  effStd += TMath::Power(meanLayerEff - layer.second.second*100/(float)layer.second.first,2);
	  if(layerMultiplicity.find(layer.first) != layerMultiplicity.end()) {
	       multStd += TMath::Power(meanLayerMult - layerMultiplicity.at(layer.first).second/(float)layerMultiplicity.at(layer.first).first,2);
	  }
     }

     effStd = TMath::Sqrt(effStd/k.size());
     multStd = TMath::Sqrt(multStd/nValidMult);

     streamlog_out(DEBUG2) << "Creating layer efficiencies graph" << std::endl;

     graphs[(type + "LayersEfficiency").c_str()] = new TGraph(k.size(), k.data(), eff.data());
     graphs[(type + "LayersEfficiency").c_str()]->SetTitle(";Layer;Eff(%)");
     graphs[(type + "LayersEfficiency").c_str()]->SetName((type + "LayersEfficiency").c_str());

     TF1* hLine = new TF1("MeanLayerEfficiency", "[0]", k.front(), k.back());
     hLine->SetParameter(0,meanLayerEff);
     hLine->SetParError(0,effStd);
     hLine->SetLineColor(kGreen);
     graphs[(type + "LayersEfficiency").c_str()]->GetListOfFunctions()->Add(hLine);
     
     graphs[(type + "LayersEfficiency")]->SetMaximum(100.);
     graphs[(type + "LayersEfficiency")]->SetMinimum(0.);
     
     streamlog_out(DEBUG2) << "Computing layer multiplicities" << std::endl;
     
     graphs[(type + "LayersMultiplicity").c_str()] = new TGraph(k.size(), k.data(), mult.data());
     graphs[(type + "LayersMultiplicity").c_str()]->SetTitle(";Layer;Mult(nHit)");
     graphs[(type + "LayersMultiplicity").c_str()]->SetName((type + "LayersMultiplicity").c_str());
     
     TF1* hLineMult = new TF1("MeanLayerMultiplicity","[0]", k.front(), k.back()); 
     hLineMult->SetParameter(0,meanLayerMult);
     hLineMult->SetParError(0,multStd);
     hLine->SetLineColor(kGreen);
     graphs[(type + "LayersMultiplicity").c_str()]->GetListOfFunctions()->Add(hLineMult);
     
     graphs[(type + "LayersMultiplicity").c_str()]->SetMaximum(3.);
     graphs[(type + "LayersMultiplicity").c_str()]->SetMinimum(0.);
     
     streamlog_out(DEBUG2) << "Creating layer stats" << std::endl;
     
     graphs[(type + "LayersStats").c_str()] = new TGraph(k.size(), k.data(), stats.data());
     graphs[(type + "LayersStats").c_str()]->SetTitle(";Layer;Entries");
     graphs[(type + "LayersStats").c_str()]->SetName((type + "LayersStats").c_str());
     
}

void MIPAnalysisProc::writeHistograms() {

  streamlog_out(DEBUG2) << "Writing 2D histograms" << std::endl;

  for(auto It2D = histograms2D.begin(); It2D != histograms2D.end(); It2D++) {

       std::string prefix;
       if(It2D->first.find("Run") != std::string::npos) prefix = std::to_string(runNumber) + "/Run";
       else if(It2D->first.find("Total") != std::string::npos) prefix = "Total";
       else prefix = "Tech";
       
       if(It2D->first.find("HitEfficiency",0) != std::string::npos) {
	    _outputFile->cd((prefix + "HitEfficiencies").c_str());
       }
       else if(It2D->first.find("Stats",0) != std::string::npos) {
	 _outputFile->cd((prefix + "Stats").c_str());
       }
       else if(It2D->first.find("ChipEfficiency",0) != std::string::npos) {
	    _outputFile->cd((prefix + "ChipEfficiencies").c_str());
       }
       else if(It2D->first.find("ChipMultiplicity",0) != std::string::npos) {
	    _outputFile->cd((prefix + "ChipMultiplicities").c_str());
       }
       It2D->second->Write();
       delete It2D->second;
  }

  histograms2D.clear();

  for(auto box : statBoxes) {
       std::string prefix;
       if(box.first.find("Run") != std::string::npos) prefix = std::to_string(runNumber) + "/Run";
       else if(box.first.find("Total") != std::string::npos) prefix = "Total";
       else prefix = "Tech";

       if(box.first.find("ChipPerct",0) != std::string::npos) {
	    _outputFile->cd((prefix + "ChipStatBoxes").c_str());
       }
       box.second->Write();
       delete box.second;
  }

  statBoxes.clear();

  _outputFile->cd((std::to_string(runNumber)).c_str());

  streamlog_out(DEBUG2) << "Writing 1D histograms" << std::endl;

  TCanvas* canvases[8];
  canvases[0] = new TCanvas("CanvasX");
  canvases[0]->Divide(4,4);
  canvases[1] = new TCanvas("CanvasY");
  canvases[1]->Divide(4,4);
  canvases[2] = new TCanvas("CanvasI");
  canvases[2]->Divide(4,4);
  canvases[3] = new TCanvas("CanvasJ");
  canvases[3]->Divide(4,4);
  canvases[4] = new TCanvas("CanvasTrackX");
  canvases[4]->Divide(4,4);
  canvases[5] = new TCanvas("CanvasTrackY");
  canvases[5]->Divide(4,4);
  canvases[6] = new TCanvas("CanvasTrackI");
  canvases[6]->Divide(4,4);
  canvases[7] = new TCanvas("CanvasTrackJ");
  canvases[7]->Divide(4,4);
  
  for(auto It1D = histograms1D.begin(); It1D != histograms1D.end(); It1D++) {
    int canvasIndex = -1;

    if(It1D->first.find("X",0) != std::string::npos) canvasIndex = 0;
    else if(It1D->first.find("Y",0) != std::string::npos) canvasIndex = 1;
    else if(It1D->first.find("I",0) != std::string::npos) canvasIndex = 2;
    else if(It1D->first.find("J",0) != std::string::npos) canvasIndex = 3;

    if(It1D->first.find("TrackX",0) != std::string::npos) canvasIndex = 4;
    else if(It1D->first.find("TrackY",0) != std::string::npos) canvasIndex = 5;  
    else if(It1D->first.find("TrackI",0) != std::string::npos) canvasIndex = 6;
    else if(It1D->first.find("TrackJ",0) != std::string::npos) canvasIndex = 7;  

    if(It1D->first.find("Chi",0) != std::string::npos || It1D->first.find("Par",0) != std::string::npos || It1D->first.find("Slope",0) != std::string::npos) canvasIndex = -1;
    
    if(canvasIndex != -1) {
      int layer =  std::stoi(It1D->first.substr(It1D->first.find("Layer") + 5));
	 
      canvases[canvasIndex]->cd(layer%16 + 1);
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
  canvases[6]->Write();
  canvases[7]->Write();

  delete canvases[0];
  delete canvases[1];
  delete canvases[2];
  delete canvases[3];
  delete canvases[4];
  delete canvases[5];
  delete canvases[6];
  delete canvases[7];
  
  histograms1D.clear();

  streamlog_out(DEBUG2) << "Writing graphs" << std::endl;

  for(auto ItGraphs = graphs.begin(); ItGraphs != graphs.end(); ItGraphs++) {

       std::string prefix;
       if(ItGraphs->first.find("Run") != std::string::npos) prefix = std::to_string(runNumber);
       else prefix = "";

       _outputFile->cd(prefix.c_str());

       ItGraphs->second->Write();
       delete ItGraphs->second;
  }
  
  graphs.clear();

}
