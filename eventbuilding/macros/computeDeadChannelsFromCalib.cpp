void computeDeadChannelsFromCalib(std::string calibrationFileName = "/home/hecgc/Physics/Repos/SiWECAL-LCIO-Analysis/commissioning/calibration/20220512/original_layer_sorting/MIP_pedestalsubmode2_fromMIPScan_LowEnergyElectrons_highgain.txt", std::string configFileName = "/home/hecgc/Physics/Repos/SiWECAL-LCIO-Analysis/commissioning/config/March2022/ecalConfiguration_March2022_2.txt", std::string mappingFileName = "/home/hecgc/Physics/Repos/SiWECAL-LCIO-Analysis/commissioning/mapping/Mapping_Files/Mapping_BGA.txt", std::string mappingFileNameCob = "/home/hecgc/Physics/Repos/SiWECAL-LCIO-Analysis/commissioning/mapping/Mapping_Files/Mapping_COB.txt", std::string deadCellsMappingFileName = "DeadCellsMapping.txt") {

  std::cout << "----- CALIBRATION -----" << std::endl;

  ifstream calibFile(calibrationFileName.c_str());
  
  if(!calibFile.is_open()) {
    std::cout << "Could not open calibration file: " << calibrationFileName << std::endl;
    return;
  }
  
  std::string tmp_hdr, tmp_lay, tmp_chip, tmp_chan, tmp_mpv, tmp_empv, tmp;

  std::map<int,std::map<int,std::map<int,bool>>> deadChanMap = {};
  
  std::getline(calibFile,tmp_hdr);
  std::getline(calibFile,tmp_hdr);
  while(calibFile) {
    calibFile >> tmp_lay >> tmp_chip >> tmp_chan >> tmp_mpv >> tmp_empv >> tmp >> tmp >> tmp;

    int layer = std::stoi(tmp_lay);
    int chip = std::stoi(tmp_chip);
    int chan = std::stoi(tmp_chan);
    float mpv = std::stof(tmp_mpv);
    float empv = std::stof(tmp_empv);
    
    if(mpv <= 0.f || empv <= 0) deadChanMap[layer][chip][chan] = true;
    else deadChanMap[layer][chip][chan] = false;
        
  }

  calibFile.close();

  std::cout << "----- MAPPING FEV -----" << std::endl;

  ifstream mappingFile(mappingFileName.c_str());

  if(!mappingFile.is_open()) {
    std::cout << "Could not open mapping file: " << mappingFileName << std::endl;
    return;
  }

  std::string tmp_I, tmp_J;

  std::map<int,std::map<int,std::pair<int,int>>> fevMapping = {};
  
  std::getline(mappingFile,tmp_hdr);
  std::getline(mappingFile,tmp_hdr);
  std::getline(mappingFile,tmp_hdr);
  while(mappingFile) {
    mappingFile >> tmp_chip >> tmp_chan >> tmp_I >> tmp_J;

    int chip = std::stoi(tmp_chip);
    int chan = std::stoi(tmp_chan);
    int I = std::stoi(tmp_I);
    int J = std::stoi(tmp_J);

    fevMapping[chip][chan] = std::pair<int,int>(I,J);
    
  }
  
  mappingFile.close();

  std::cout << "----- MAPPING COB -----" << std::endl;

  mappingFile.open(mappingFileNameCob.c_str());

  if(!mappingFile.is_open()) {
    std::cout << "Could not open mapping file COB: " << mappingFileNameCob << std::endl;
    return;
  }

  std::map<int,std::map<int,std::pair<int,int>>> cobMapping = {};
  
  std::getline(mappingFile,tmp_hdr);
  std::getline(mappingFile,tmp_hdr);
  std::getline(mappingFile,tmp_hdr);
  while(mappingFile) {
    mappingFile >> tmp_chip >> tmp_chan >> tmp_I >> tmp_J;

    int chip = std::stoi(tmp_chip);
    int chan = std::stoi(tmp_chan);
    int I = std::stoi(tmp_I);
    int J = std::stoi(tmp_J);

    cobMapping[chip][chan] = std::pair<int,int>(I,J);
    
  }
  
  mappingFile.close();

  std::cout << "----- CONFIG -----" << std::endl;

  ifstream configFile(configFileName.c_str());

  if(!configFile.is_open()) {
    std::cout << "Could not open config file: " << configFileName << std::endl;
  }

  std::string tmp_type;

  std::map<int,std::string> configMapping = {};
  
  std::getline(configFile,tmp_hdr);
  while(configFile) {
    configFile >> tmp >> tmp_lay >> tmp >> tmp >> tmp >> tmp_type >> tmp >> tmp >> tmp;

    int layer = std::stoi(tmp_lay);

    configMapping[layer] = tmp_type;
  }
  
  configFile.close();

  std::cout << "----- DEAD CHANNELS MAPPING -----" << std::endl;

  ofstream deadFile(deadCellsMappingFileName.c_str());

  if(!deadFile.is_open()) {
    std::cout << "Could not open dead cells file: " << deadCellsMappingFileName << std::endl;
    return;
  }

  deadFile << "#Layer I J IsDead" << std::endl; 
  
  for(auto layer : deadChanMap) {
    for(auto chip : layer.second) {
      for(auto chan : chip.second) {
	int I,J;

	if(configMapping.at(layer.first).find("FEV") != std::string::npos) {
	  I = fevMapping.at(chip.first).at(chan.first).first;
	  J = fevMapping.at(chip.first).at(chan.first).second;
	}
	else {
	  I = cobMapping.at(chip.first).at(chan.first).first;
	  J = cobMapping.at(chip.first).at(chan.first).second;
	}

	deadFile << layer.first << " " << I << " " << J << " " << chan.second << std::endl;
	
      }
    }
  }
}
