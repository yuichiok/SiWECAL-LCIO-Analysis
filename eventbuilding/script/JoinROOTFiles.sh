#!/bin/bash

if [ -z "$ILCSOFT" ]; then
   echo " You should source an init_ilcsoft.sh script"
   exit
fi

commonFolder=${1}
baseName=${2}

for run in ${commonFolder}/*bin_*.root; do

    runNumber=$( echo ${run#*"run_"} | cut -d '_' -f 1 ) 
    
    hadd -f ${run}/${baseName}_run_${runNumber}_raw.root ${run}/${baseName}*.root 
    
done
