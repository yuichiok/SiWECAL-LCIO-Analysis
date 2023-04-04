void MIPAlignment(string inputFileNames = "", string outFileBaseName = "MIPAlignment")
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

     std::map<int,std::vector<float>> deltaX, deltaY, eDeltaX, eDeltaY;
     deltaX = deltaY = eDeltaX = eDeltaY = {};
     
     for(auto fileIt = files.begin(); fileIt != files.end(); fileIt++) {

	  TFile* inputFile = new TFile((*fileIt).c_str(), "READ");
	  if(!inputFile->IsOpen()) {
	       std::cout << "Could not open file: " << *fileIt << std::endl;
	       exit(0);
	  }

	  for(auto key : *inputFile->GetListOfKeys()) {

	    TString keyName = key->GetName();

	    std::cout << "Processing run: " << keyName << std::endl;

	    outputFile->mkdir(keyName);
	    outputFile->cd(keyName);

	    TCanvas* canvas_X = (TCanvas*)inputFile->Get(keyName + "/CanvasXFull");
	    TCanvas* canvas_Y = (TCanvas*)inputFile->Get(keyName + "/CanvasYFull");

	    std::vector<double> x, ex, y, ey, z;
	    x = ex = y = ey = z = {};

	    for(int iLayer = 0; iLayer < 15; iLayer++) {
	      
	      TH1F* histX = (TH1F*)canvas_X->GetPad(iLayer + 1)->GetPrimitive(("XFull_Layer" + std::to_string(iLayer)).c_str());
	      TH1F* histY = (TH1F*)canvas_Y->GetPad(iLayer + 1)->GetPrimitive(("YFull_Layer" + std::to_string(iLayer)).c_str());
	      
	      if(histX->GetEntries() < 25000 || histY->GetEntries() < 25000) continue;

	      TF1* gausFuncX = (TF1*)histX->GetListOfFunctions()->FindObject("gaus");
	      TF1* gausFuncY = (TF1*)histY->GetListOfFunctions()->FindObject("gaus");
	      
	      if(gausFuncX == 0 || gausFuncY == 0) continue;

	      const double* paramsX = gausFuncX->GetParameters();
	      const double* paramsY = gausFuncY->GetParameters();

	      if(paramsX[2] > 15.0 || paramsY[2] > 15.0) continue;

	      x.push_back(paramsX[1]);
	      ex.push_back(paramsX[2]);
	      y.push_back(paramsY[1]);
	      ey.push_back(paramsY[2]);
	      z.push_back(iLayer);

	    }  
            
	    if(z.size() == 0) continue;
	    
	    TGraphErrors* graphXZ = new TGraphErrors(z.size(), z.data(), x.data(), 0, ex.data());
	    graphXZ->SetTitle(";Layer;#mu_{X}");
	    graphXZ->SetName("FitMeanX");
	    
	    TGraphErrors* graphYZ = new TGraphErrors(z.size(), z.data(), y.data(), 0, ey.data());
	    graphYZ->SetTitle(";Layer;#mu_{Y}");
	    graphYZ->SetName("FitMeanY");
	    
	    TFitResultPtr fitMeanXRes = graphXZ->Fit("pol1", "QS");
	    TFitResultPtr fitMeanYRes = graphYZ->Fit("pol1", "QS");

	    if(fitMeanXRes->Status() != 0 || fitMeanYRes->Status() != 0) {
	      std::cout << "NotFit: X = " << fitMeanXRes->Status() << " Y = " << fitMeanYRes->Status() << std::endl;
	      continue;
	    }

	    const double* fitParamsMeanX = fitMeanXRes->GetParams();
	    const double* fitParamsErrX = fitMeanXRes->GetErrors();
	    
	    const double* fitParamsMeanY = fitMeanYRes->GetParams();
	    const double* fitParamsErrY = fitMeanYRes->GetErrors();
	    
	    for(auto layer : z) {

	      int index = std::find(z.begin(),z.end(),layer) - z.begin();

	      float trackX = fitParamsMeanX[0] + fitParamsMeanX[1]*layer;
	      float eTrackX = fitParamsErrX[0] + fitParamsErrX[1]*layer;

	      float trackY = fitParamsMeanY[0] + fitParamsMeanY[1]*layer;
	      float eTrackY = fitParamsErrY[0] + fitParamsErrY[1]*layer;
	      
	      deltaX[layer].push_back(trackX - x.at(index));
	      eDeltaX[layer].push_back(TMath::Sqrt(eTrackX*eTrackX + ex.at(index)*ex.at(index)));
	      deltaY[layer].push_back(trackY - y.at(index));
	      eDeltaY[layer].push_back(TMath::Sqrt(eTrackY*eTrackY + ey.at(index)*ey.at(index)));
	      
	    }

	    graphXZ->Write();
	    graphYZ->Write();

	    delete graphXZ;
	    delete graphYZ;
	    
	  }

	  inputFile->Close();
	  
     }

     std::vector<float> vDeltaX, vDeltaY, evDeltaX, evDeltaY, vLayer;
     vDeltaX = vDeltaY = evDeltaX = evDeltaY = vLayer = {};

     for(auto layer : deltaX) {

       float meanDx, sumErrDX;
       meanDx = sumErrDX = 0.;

       for(int iX = 0; iX < deltaX.at(layer.first).size(); iX++) {
	 meanDx += deltaX.at(layer.first).at(iX);
	 sumErrDX += eDeltaX.at(layer.first).at(iX);
       }
       meanDx /= deltaX.at(layer.first).size();

       float dx, sumErrDXSquare;
       dx = sumErrDXSquare = 0.;

       for(int iX = 0; iX < deltaX.at(layer.first).size(); iX++) {
	 dx += deltaX.at(layer.first).at(iX)*eDeltaX.at(layer.first).at(iX)/sumErrDX;	 
	 sumErrDXSquare += eDeltaX.at(layer.first).at(iX)*eDeltaX.at(layer.first).at(iX)/(sumErrDX*sumErrDX);
       } 

       float stdx = 0.;
       for(auto mu : deltaX.at(layer.first)) {
	 stdx += (meanDx - mu)*(meanDx - mu);
       }
       stdx = TMath::Sqrt(stdx/deltaX.at(layer.first).size());

       if(stdx <= 0) stdx = 1;
       
       float edx = stdx*TMath::Sqrt(sumErrDXSquare);

       float meanDy, sumErrDY;
       meanDy = sumErrDY = 0.;

       for(int iY = 0; iY < deltaY.at(layer.first).size(); iY++) {
	 meanDy += deltaY.at(layer.first).at(iY);
	 sumErrDY += eDeltaY.at(layer.first).at(iY);
       }
       meanDy /= deltaY.at(layer.first).size();

       float dy, sumErrDYSquare;
       dy = sumErrDYSquare = 0.;

       for(int iY = 0; iY < deltaY.at(layer.first).size(); iY++) {
	 dy += deltaY.at(layer.first).at(iY)*eDeltaY.at(layer.first).at(iY)/sumErrDY;	 
	 sumErrDYSquare += eDeltaY.at(layer.first).at(iY)*eDeltaY.at(layer.first).at(iY)/(sumErrDY*sumErrDY);
       } 

       float stdy = 0.;
       for(auto mu : deltaY.at(layer.first)) {
	 stdy += (meanDy - mu)*(meanDy - mu);
       }
       stdy = TMath::Sqrt(stdy/deltaY.at(layer.first).size());

       if(stdy <= 0) stdy = 1;
       
       float edy = stdy*TMath::Sqrt(sumErrDYSquare);
       
       vDeltaX.push_back(dx);
       vDeltaY.push_back(dy);
       evDeltaX.push_back(edx);
       evDeltaY.push_back(edy);
       vLayer.push_back(layer.first);
       
     }

     outputFile->cd();

     TGraphErrors* deltaXGraph = new TGraphErrors(vLayer.size(), vLayer.data(), vDeltaX.data(), 0, evDeltaX.data());
     deltaXGraph->SetTitle(";Layer;#DeltaX");
     deltaXGraph->SetName("DeltaX");
     deltaXGraph->Write();
     delete deltaXGraph;

     TGraphErrors* deltaYGraph = new TGraphErrors(vLayer.size(), vLayer.data(), vDeltaY.data(), 0, evDeltaY.data());
     deltaYGraph->SetTitle(";Layer;#DeltaY");
     deltaYGraph->SetName("DeltaY");
     deltaYGraph->Write();
     delete deltaYGraph;
     
     outputFile->Close();
     delete outputFile;

     ofstream outputAlignmentFile;
     outputAlignmentFile.open((outFileBaseName + ".txt").c_str());

     outputAlignmentFile << "Layer AlignmentX(mm) ErrorX(mm) AlignmentY(mm) ErrorY(mm)" << std::endl;
     
     for(int iLayer = 0; iLayer < 15; iLayer++) {

       auto layerIt = std::find(vLayer.begin(), vLayer.end(), iLayer);
       
       if(layerIt == vLayer.end()) {
	 outputAlignmentFile << iLayer << " -999 0 -999 0" << std::endl;
       }
       else {
	 int index = layerIt - vLayer.begin();
	 outputAlignmentFile << iLayer << " " << vDeltaX.at(index) << " " << evDeltaX.at(index) << " " << vDeltaY.at(index) << " " << evDeltaY.at(index) << std::endl;
       }

     }

     outputAlignmentFile.close();
     
}


