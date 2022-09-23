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

*hadd PathToFolder/BaseName_run_XXXX_raw.root PathToDataFolder/\*_bin\*.root*

and then move it to a folder with all the runs you want to build together:

*mv PathToFolder/BaseName_run_XXXX_raw.root rawfiles/*

## Run the event builder

The final step is to run the event builder in the folder with all the runs to be built. Just run:

*./script/buildFolder.sh*

However, to work properly some variables in the script need to be changed accordingly (There is documentation within the script):

buildFolder -> path to the *rawfiles* folder 
commisioningFolder -> a common folder with all the commissioning files like calibration, pedestals, etc.
outputFolder -> the produced .slcio and LogROOT will be moved to this folder
outputBaseName -> The prefix of the output files

Change the paths to the corresponding commisioning files: *pedestals_HG, pedestals_LG, mipCalibration_HG, mipCalibration_LG, mappingFile, mappingCob*

There is an extra file: the *configFile* which indicates information about the layer. Format:

*slot layer slab slabID slabAdd ASI wafer W(mm) DeltaX(mm)*

there is the example of March 2022 in the commisioning folder.

## Troubleshooting

Any problems send an email to: Hector.Garcia2@ciemat.es





