#!/bin/bash

if [ -z "$ILCSOFT" ]; then
   echo " You should source an init_ilcsoft.sh script"
   exit
fi

commonFolder="/home/hecgc/Physics/Repos/SiWECAL-LCIO-Analysis/Data/MIPScan"

for run in ${commonFolder}/3GeV_MIPscan_eudaq*; do

    runNumber=$( echo ${file#*"run_"} | cut -d '_' -f 1 ) 

    root -l /home/hecgc/Physics/Repos/SiWECAL-LCIO-Analysis/eventbuilding/macros/ConvertDirectorySL_Raw.cpp\(\"${run}/\",\"${runNumber}\"\)
    
done
