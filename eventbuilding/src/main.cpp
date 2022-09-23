/* #####################################################

   Program to create LCIO events from data. v00_00_03
   Current version processes RawROOTFiles or BuiltROOTFiles
   Formatted for the March of 2022 beam test data. 
   ILCSoft: v02-02-02
   Author: Hector Garcia Cabrera

######################################################## */

#include "ECal_EventBuilding.h"

#include <argp.h> //Documentation in https://www.gnu.org/software/libc/manual/html_node/Argp.html
#include <iostream>

error_t parse_opt(int key, char *arg, struct argp_state *state) {

  EventBuilder* builder = (EventBuilder*)state->input;
  
  switch(key) {
  case 'n':
    builder->_maxEntries = std::stoi(arg); break;
  case 'i':
    builder->_inputFileName = arg; break;
  case 'p':
    builder->_inType = std::stoi(arg); break;
  case 't':
    builder->_inputTreeName = arg; break;
  case 'o':
    builder->_outputFileName = arg; break;
  case 'c':
    builder->_commissioningFolder = arg; break;
  case 'm':
    builder->_excMode = arg; break;
  case 'r':
    builder->_runNumber = std::stoi(arg); break;
  case 999:
    builder->_outputColName = arg; break;
  case 1000:
    builder->_configFile = arg; break;
  case 1001:
    builder->_mappingFile = arg; break;
  case 1002:
    builder->_mappingFileCob = arg; break;
  case 1003:
    builder->_pedestalsFileHG = arg; break;
  case 1004:
    builder->_pedestalsFileLG = arg; break;
  case 1005:
    builder->_mipCalibrationFileHG = arg; break;
  case 1006:
    builder->_mipCalibrationFileLG = arg; break;
  }
  return 0;
}

int main(int argc , char** argv)
{

  argp_program_version = "v00_00_03";
  argp_program_bug_address = "Hector.Garcia2@ciemat.es -- NO SPAM";

  std::string doc = "Program to convert the RawROOTFiles from SiWEcal Beam Test 2021 or BuiltROOTFiles from SiWEcal Beam Test 2022 into LCIO format.";
  std::string argDoc = "-i INPUTFILENAME";  
  
  struct argp_option options[] = {
				  {"max_entries", 'n', "MAXENTRIES", 0, "Number of entries to process from the input file"},
				  {"in_file_name", 'i', "INFILENAME", 0, "Input file name"},
				  {"in_tree_name", 't', "INTREENAME", 0, "Input TTree name"},
				  {"in_type", 'p', "INTYPE", 0, "Input type. 0 -> Binary reconstruction. 1 -> RawROOT reconstruction. 2 -> BuiltROOT reconstruction."},
				  {"out_file_name", 'o', "OUTFILENAME", 0, "Output file name"},
		        	  {"out_col_name", 999, "OUTCOLNAME", 0, "Output collection name"},
				  {"comissioning_folder", 'c', "COMFOLDER", 0, "Path to the comissioning folder"},
				  {"exc_mode", 'm', "EXCMODE", 0, "Execution mode of this program: default -> executes with minimal output ; debug -> executes with all output ; setup -> only reads and prints all the input files"},
				  {"run_number", 'r', "RUNNUMBER", 0, "Run number. By default -1"},				   
				  {"configuration_file", 1000, "CONFIG", 0, "Layer configuration of the calorimeter"},
				  {"mapping_file", 1001, "MAPFILE", 0, "Mapping file name"},
				  {"mapping_file_cob", 1002, "MAPFILECOB", 0, "Mapping file name for the cob layers"},
				  {"pedestals_file_HG", 1003, "PEDFILE", 0, "Pedestals file name HG"},
				  {"pedestals_file_LG", 1004, "PEDFILE", 0, "Pedestals file name LG"},
				  {"mip_calibration_file_HG", 1005, "MIPFILE", 0, "Mip calibration file name HG"},
				  {"mip_calibration_file_LG", 1006, "MIPFILE", 0, "Mip calibration file name LG"},
				  { 0 }
  };
  struct argp argumentsParser = {options, parse_opt, argDoc.c_str(), doc.c_str() };
  
  argumentsParser.options = options;
  
  EventBuilder* builder = new EventBuilder();

  argp_parse(&argumentsParser, argc, argv, 0, 0, (void*)builder);

  if(builder->_inputFileName == "" && builder->_excMode != "setup") {
    std::cout << "The INPUTFILENAME must be specified" << std::endl;
    return 0;
  }

  std::cout << "###########################" << std::endl;
  std::cout << "Event Builder: RawToLCIO " << argp_program_version << std::endl;
  
  std::cout << "\t MaxEntries: " << builder->_maxEntries << std::endl;
  std::cout << "\t InputFileName: " << builder->_inputFileName << std::endl;
  std::cout << "\t InputType: " << builder->_inType << std::endl;
  std::cout << "\t InputTreeName: " << builder->_inputTreeName << std::endl;
  std::cout << "\t OutputFileName: " << builder->_outputFileName << std::endl;
  std::cout << "\t OutputColName: " << builder->_outputColName << std::endl;
  std::cout << "\t ConfigurationFile: " << builder->_configFile << std::endl;
  std::cout << "\t MappingFile: " << builder->_mappingFile << std::endl;
  std::cout << "\t MappingFileCob: " << builder->_mappingFileCob << std::endl;
  std::cout << "\t PedestalsFileHG: " << builder->_pedestalsFileHG << std::endl;
  std::cout << "\t PedestalsFileLG: " << builder->_pedestalsFileLG  << std::endl;
  std::cout << "\t MipCalibrationFileHG: " << builder->_mipCalibrationFileHG << std::endl;
  std::cout << "\t MipCalibrationFileLG: " << builder->_mipCalibrationFileLG << std::endl;
  std::cout << "\t Commissioning folder: " << builder->_commissioningFolder << std::endl; 
  std::cout << "\t Run number: " << builder->_runNumber << std::endl; 
  std::cout << "\t Execution mode: " << builder->_excMode << std::endl; 

  std::cout << "###########################" << std::endl;
  
  builder->Check();

  bool ready = builder->Init();
  if(ready) builder->BuildEvents();

  builder->End();
  
  return 0;
}


