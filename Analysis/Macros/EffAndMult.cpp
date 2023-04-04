void EffAndMult(string inputFileNames = "", string outFileBaseName = "EffAndMult", string prefix = "Total")
{

     TFile* outputFile = new TFile((outFileBaseName + ".root").c_str(),"RECREATE");
     
     std::vector<std::string> files = {};
     std::string fileName = "";
     for(int iChar = 0; iChar <= inputFileNames.size(); iChar++) {
	  
	  if(iChar == inputFileNames.size() || inputFileNames.at(iChar) == ' ') {
	       files.push_back(fileName);
	       fileName.clear();
	       continue;
	  }
	  
	  fileName += inputFileNames.at(iChar);
	  
     }

     gStyle->SetOptStat(0000);

     std::vector<float> meanEffs, meanMults, meanEffsErrors, meanMultsErrors;
     meanEffs = meanMults = meanEffsErrors = meanMultsErrors = {};

     for(auto fileIt = files.begin(); fileIt != files.end(); fileIt++) {

	  TFile* inputFile = new TFile((*fileIt).c_str(), "READ");
	  if(!inputFile->IsOpen()) {
	       std::cout << "Could not open file: " << *fileIt << std::endl;
	       exit(0);
	  }

	  outputFile->cd();
	  
	  /*float angle = TMath::ATan(((TH1F*)inputFile->Get("GlobalSlopeX"))->GetMean())*180.0/TMath::Pi();
	  float angleErr = TMath::ATan(((TH1F*)inputFile->Get("GlobalSlopeX"))->GetStdDev())*180.0/TMath::Pi();
	  angles.push_back(angle);
	  anglesErr.push_back(angleErr);
	  anglesRad.push_back(angle*TMath::Pi()/180.);
	  anglesRadErr.push_back(angleErr*TMath::Pi()/180.);
	  
	  
	  outputFile->mkdir((std::to_string(angle) + "Deg").c_str());
	  outputFile->cd((std::to_string(angle) + "Deg").c_str());
	  */
	  
	  TGraph* efficiency = (TGraph*)inputFile->Get((prefix + "LayersEfficiency").c_str());
	  efficiency->SetMinimum(0.);
	  efficiency->SetMaximum(101.);
	  efficiency->SetMarkerStyle(21);
	  efficiency->SetMarkerColor(kBlue);
	  efficiency->SetMarkerSize(0.6);
	  TF1* meanEff = efficiency->GetFunction("MeanLayerEfficiency");

	  std::vector<float> x, y, ex, ey;
	  x = y = ey = {};

	  for(int iLayer = 0; iLayer < 15; iLayer++) {
	       x.push_back(iLayer);
	       y.push_back(meanEff->GetParameter(0)); 
	       ey.push_back(meanEff->GetParError(0));
	  }
	  
	  TGraphErrors* meanEffBand = new TGraphErrors(x.size(), x.data(), y.data(), 0, ey.data());
	  meanEffBand->SetFillColor(kGreen);
	  meanEffBand->SetFillStyle(3005);
 
	  meanEffs.push_back(meanEff->GetParameter(0));
	  meanEffsErrors.push_back(meanEff->GetParError(0));

	  TCanvas* effCanvas = new TCanvas("EffCanvas");
	  efficiency->Draw("AP");
	  meanEffBand->Draw("Same3");
	  effCanvas->Write();
	  delete effCanvas;

	  TGraph* multiplicity = (TGraph*)inputFile->Get((prefix + "LayersMultiplicity").c_str());
	  multiplicity->SetMarkerStyle(22);
	  multiplicity->SetMarkerColor(kRed);
	  multiplicity->SetMarkerSize(0.7);
	  TF1* meanMult = multiplicity->GetFunction("MeanLayerMultiplicity");
        
	  y = ey = {};

	  for(int iLayer = 0; iLayer < 15; iLayer++) {
	       y.push_back(meanMult->GetParameter(0)); 
	       ey.push_back(meanMult->GetParError(0));
	  }
        
	  TGraphErrors* meanMultBand = new TGraphErrors(x.size(), x.data(), y.data(), 0, ey.data());
	  meanMultBand->SetFillColor(kBlack);
	  meanMultBand->SetFillStyle(3005);
 
	  meanMults.push_back(meanMult->GetParameter(0));
	  meanMultsErrors.push_back(meanMult->GetParError(0));

	  TCanvas* multCanvas = new TCanvas("MultCanvas");
	  multiplicity->Draw("APE");
	  meanMultBand->Draw("same3");
	  multCanvas->Write();
	  delete multCanvas;

	  TDirectory* statsDir = inputFile->GetDirectory((prefix + "Stats").c_str(),true);
	  std::vector<TH2F*> stats = {};
	  for(const auto&& key: *statsDir->GetListOfKeys()) {
	       stats.push_back((TH2F*)statsDir->Get(key->GetName()));
	  }
	  	  
	  outputFile->mkdir("Stats");
	  outputFile->cd("Stats");
	  
	  TCanvas* canvas;
	  canvas = new TCanvas("CanvasStats","",0,0,2160,2160);
	  canvas->Divide(4,4);
	  
	  for(int iHist = 0; iHist < stats.size(); iHist++) {
	       std::string name = stats.at(iHist)->GetName();
	       int layer =  std::stoi(name.substr(name.rfind("Layer") + 5));
	       int iCanvas = layer/16;
	       canvas->cd(layer%16 + 1);
	       stats.at(iHist)->Draw("COLZ");
	  }
	  
	  canvas->Write();
	  delete canvas;
	 
	  gStyle->SetPalette(kDarkBodyRadiator);
	  
	  TDirectory* effDir = inputFile->GetDirectory((prefix + "HitEfficiencies").c_str(),true);
	  std::vector<TH2F*> effs = {};
	  for(const auto&& key: *effDir->GetListOfKeys()) {
	       effs.push_back((TH2F*)effDir->Get(key->GetName()));
	  }

	  outputFile->mkdir("HitEfficiencies");
	  outputFile->cd("HitEfficiencies");
	  
	  canvas = new TCanvas("CanvasHitEffs","",0,0,2160,2160);
	  canvas->Divide(4,4);
	  
	  for(int iHist = 0; iHist < effs.size(); iHist++) {
	       std::string name = effs.at(iHist)->GetName();
	       int layer =  std::stoi(name.substr(name.rfind("Layer") + 5));
	       int iCanvas = layer/16;
	       canvas->cd(layer%16 + 1);
	       effs.at(iHist)->Draw("COLZ");
	  }
	  
	  canvas->Write();
	  delete canvas;

	  TDirectory* ChipEffDir = inputFile->GetDirectory((prefix + "ChipEfficiencies").c_str(),true);
	  std::vector<TH2F*> ChipEffs = {};
	  for(const auto&& key: *ChipEffDir->GetListOfKeys()) {
	       ChipEffs.push_back((TH2F*)ChipEffDir->Get(key->GetName()));
	  }

	  TDirectory* ChipBoxesDir = inputFile->GetDirectory((prefix + "ChipStatBoxes").c_str(),true);
	  std::vector<TPaveText*> ChipBoxes = {};
	  for(const auto&& key: *ChipBoxesDir->GetListOfKeys()) {
	       ChipBoxes.push_back((TPaveText*)ChipBoxesDir->Get(key->GetName()));
	  }

	  outputFile->mkdir("ChipEfficiencies");
	  outputFile->cd("ChipEfficiencies");
	  
	  for(int iHist = 0; iHist < ChipEffs.size(); iHist++) {
	       std::string name = ChipEffs.at(iHist)->GetName();
	       int layer =  std::stoi(name.substr(name.rfind("Layer") + 5));
	       TCanvas* newCanvas = new TCanvas(("CanvasChipEffs_" + std::to_string(layer)).c_str());
	       ChipEffs.at(iHist)->Draw("COLZ");
	       for(auto box : ChipBoxes){
		    std::string boxName = box->GetName();
		    int ChipID = std::stoi(boxName.substr(boxName.rfind("_") + 1));
		    if(ChipID/16 == layer) box->Draw("SAME");
	       }
	       newCanvas->Write();
	       delete newCanvas;
	  }
    
     }

     outputFile->cd();

     /* TGraphErrors* effsPerAngle = new TGraphErrors(angles.size(), angles.data(), meanEffs.data(), anglesErr.data(), meanEffsErrors.data());
     effsPerAngle->SetName("EffsPerAngle");
     effsPerAngle->SetTitle(";Angle(#theta);Eff(%)");
     effsPerAngle->SetMarkerStyle(21);
     effsPerAngle->GetXaxis()->SetLimits(-5,35);
     effsPerAngle->Write();
     delete effsPerAngle;

     TGraphErrors* multsPerAngle = new TGraphErrors(angles.size(), angles.data(), meanMults.data(), anglesErr.data(), meanMultsErrors.data());
     multsPerAngle->SetName("MultsPerAngle");
     multsPerAngle->SetTitle("Angle(#theta);Multiplicity(nHits)");
     multsPerAngle->SetMarkerStyle(22);
     multsPerAngle->GetXaxis()->SetLimits(-5,35);
     multsPerAngle->Write();
     delete multsPerAngle;

     TGraphErrors* multsPerAngleRad = new TGraphErrors(anglesRad.size(), anglesRad.data(), meanMults.data(), anglesRadErr.data(), meanMultsErrors.data());
     
     std::cout << anglesRad.size() << std::endl;

     TF1* cosFunc = new TF1("CosF",(std::to_string(meanMults.at(0)) + "/cos([0]*x)").c_str(), 0, 1.0);
     TF1* sinFunc = new TF1("SinF", "sin([0]*x)", 0, 30);

     multsPerAngleRad->Fit(cosFunc, "E");
     
     sinFunc->SetParameter(0,cosFunc->GetParameter(0));

     vector<float> meanMultsCorr, meanMultsCorrErrUp, meanMultsCorrErrDown;
     meanMultsCorr = meanMultsCorrErrUp = meanMultsCorrErrDown = {};

     for(int iAngle = 0; iAngle < angles.size(); iAngle++) {
	  float corr = meanMults.at(iAngle)*TMath::Cos(cosFunc->GetParameter(0)*anglesRad.at(iAngle));
	  meanMultsCorr.push_back(corr);
	  float errUp, errDown;
	  errUp = (meanMults.at(iAngle) + meanMultsErrors.at(iAngle))*TMath::Cos((cosFunc->GetParameter(0) - cosFunc->GetParError(0))*TMath::Abs(anglesRad.at(iAngle) - anglesRadErr.at(iAngle))) - corr;
	  meanMultsCorrErrUp.push_back(errUp);
	  errDown = corr - (meanMults.at(iAngle) - meanMultsErrors.at(iAngle))*TMath::Cos((cosFunc->GetParameter(0) + cosFunc->GetParError(0))*TMath::Abs(anglesRad.at(iAngle) + anglesRadErr.at(iAngle)));
	  meanMultsCorrErrDown.push_back(errDown);
     }

     multsPerAngleRad->Write();
     delete multsPerAngleRad;

     TGraphAsymmErrors* multsPerAngleCorr = new TGraphAsymmErrors(angles.size(), angles.data(), meanMultsCorr.data(), anglesErr.data(), anglesErr.data(), meanMultsCorrErrDown.data(), meanMultsCorrErrUp.data());
     multsPerAngleCorr->SetName("MultsPerAngleCorr");
     multsPerAngleCorr->SetTitle("Angle(#theta);Multiplicity(nHits)");
     multsPerAngleCorr->SetMarkerStyle(22);
     multsPerAngleCorr->GetXaxis()->SetLimits(-5,35);
     
     TF1* fitFunc = new TF1("FitFunc", "pol1", -5, 35);
     fitFunc->SetParameters(1.7,0.);
     multsPerAngleCorr->Fit(fitFunc);

     multsPerAngleCorr->Write();
     delete multsPerAngleCorr;
     */
     
     outputFile->Close();
     delete outputFile;

     exit(0);
     
}


