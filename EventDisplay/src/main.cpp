/* #####################################################

   Program to create event displays of the ECAL slcio data
   ILCSoft: v02-02-02
   Author: Hector Garcia Cabrera

######################################################## */

#include "ECal_EventDisplay.h"

#include <argp.h> //Documentation in https://www.gnu.org/software/libc/manual/html_node/Argp.html
#include <iostream>

error_t parse_opt(int key, char *arg, struct argp_state *state) {
  
  EventDisplay* disp = (EventDisplay*)state->input;
  
  switch(key) {
  case 'i':
    disp->_inputFileName = arg; break;
  case 'c':
    disp->_inputColName = arg; break;
  case 'e':
    disp->_inputEventsFileName = arg; break;
  case 'o':
    disp->_outputFileName = arg; break;
  }
  return 0;
}

int main(int argc , char** argv)
{

  argp_program_version = "v00_00_01";
  argp_program_bug_address = "Hector.Garcia2@ciemat.es -- NO SPAM";

  std::string doc = "Program to create event displays of the ECAL slcio data.";
  std::string argDoc = "-i INPUTFILENAME";  
  
  struct argp_option options[] = {
				  {"in_file_name", 'i', "INFILENAME", 0, "Input file name"},
 				  {"in_col_name", 'c', "INCOLNAME", 0, "Input collection name"},
				  {"in_events_file_name", 'e', "INEVENTFILENAME", 0, "Input file with the events list to display"},
				  {"out_file_name", 'o', "OUTFILENAME", 0, "Output file name"},
				  { 0 }
  };
  struct argp argumentsParser = {options, parse_opt, argDoc.c_str(), doc.c_str() };
  
  EventDisplay* display = new EventDisplay();

  argp_parse(&argumentsParser, argc, argv, 0, 0, (void*)display);

  if(display->_inputFileName == "") {
    std::cout << "The INPUTFILENAME must be specified" << std::endl;
    return 0;
  }

  std::cout << "###########################" << std::endl;
  std::cout << "Event Display version " << argp_program_version << std::endl;
  
  std::cout << "\t InputFileName: " << display->_inputFileName << std::endl;
  std::cout << "\t InputColName: " << display->_inputColName << std::endl;
  std::cout << "\t InputEventsFileName: " << display->_inputEventsFileName << std::endl;
  std::cout << "\t OutputFileName: " << display->_outputFileName << std::endl;
 
  std::cout << "###########################" << std::endl;

  
  display->Check();

  bool ready = display->Init();
  if(ready) display->Process();

  display->End();
  
  return 0;
}


