void drawDeadChannelsMap(std::string mapName = "") {

  gStyle->SetPalette(51);
  gStyle->SetOptStat(0001);
  
  std::fstream mapFile;
  mapFile.open(mapName.c_str());
  if(!mapFile.is_open()) {
    std::cout << "Could not open dead channels map file: " << mapName << std::endl;
    return;
  }

  std::map<int,std::map<std::pair<int,int>, bool>> map;
  
  std::string tmp_header, tmp_layer, tmp_I, tmp_J, tmp_dead;
  std::getline(mapFile,tmp_header);
  while(mapFile) {
    mapFile >> tmp_layer >> tmp_I >> tmp_J >> tmp_dead;
    map[std::stoi(tmp_layer)][std::pair<int,int>(std::stoi(tmp_I),std::stoi(tmp_J))] = std::stoi(tmp_dead);
  }

  const int nLayers = 15;

  std::vector<TH2F*> histograms2D = {};
  for(int iLayer = 0; iLayer < nLayers; iLayer++) {

    TH2F* newLayerHist = new TH2F(("DeadMap_Layer" + std::to_string(iLayer)).c_str(), ";I;J", 32, 0., 32., 32, 0., 32.);
    histograms2D.push_back(newLayerHist);
    
  }
  
  for(auto layer : map) {
    for(auto hit : layer.second) {
      if(hit.second == 0) histograms2D.at(layer.first)->Fill(hit.first.first,hit.first.second);

    }
  }

  TCanvas* canvas = new TCanvas("Canvas");
  canvas->Divide(4,4);

  for(int i = 0; i < nLayers; i++) {
    canvas->cd(i+1);
    histograms2D.at(i)->Draw("COL");
  }
  
  canvas->SaveAs("DeadChannelsMap.png");
  
}
