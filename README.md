# SiWECal: Binary to ROOT and event builder into LCIO

## Setup

First change the *softdir* variable in *eventbuilding/script/initWorkEnv.sh* to the corresponding ilcsoft version then:  
  *source eventbuilding/script/initWorkEnv.sh* 
to prepare the ilcsoft environment. 

## Build

*cd eventbuilding* 
*./script/build.sh Full*

With the latest versions of ilcsoft this step should not give any errors. Let me know if any problem arises.

Skip this step if you already built the program.

## Convert Binary to ROOT

Having all the binary files from a run in the same folder, run the Binary to ROOT converter:

*root -l macros/ConvertDirectorySL_Raw.cpp\(\"PathToDataFolder/\",\"XXXX\"\)*

where the program expects the following format for the in files: ${baseName}_run_XXXX_raw.bin_YYYY with XXXX the run number and YYYY the file counter. The base name is whatever prefix you have for the run.

## Join ROOT files

Join all the produced ROOT files into a single one with:

*hadd PathToFolder/BaseName_run_XXX_raw.root PathToDataFolder/\*_bin\*.root*





First source ilcsoft
Then run in the eventbuilding folder ./script/build Full
To run eventbuilding/app/ECal_EventBuilding --help for a list of all options. The program needs to find all calibration, masking, pedestals, config and mapping files to start. Tune the arguments as necessary.
The default output name is SiWECal_TB2021.lcio
