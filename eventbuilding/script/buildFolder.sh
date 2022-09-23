#!/bin/bash

if [ -z "$ILCSOFT" ]; then
   echo " You should source an init_ilcsoft.sh script"
   exit
fi

buildFolder="/home/hecgc/Physics/Repos/SiWECAL-LCIO-Analysis/Data/MIPAngle/rawfiles"
commissioningFolder="/home/hecgc/Physics/Repos/SiWECAL-LCIO-Analysis/commissioning"
outputFolder="/home/hecgc/Physics/Repos/SiWECAL-LCIO-Analysis/Data/Showers"
outputBaseName="MIPAngle_Run_"
buildOption=1

configFile="config/March2022/ecalConfiguration_March2022_2.txt"

for file in ${buildFolder}/*.root; do

    runNumber=$( echo ${file#*"run_"} | cut -d '_' -f 1 )
    energy=${file%"GeV"*}
    energy=${energy##*"/"}
    cap=${file%"pF"*}
    cap=${cap##*"/"}

    echo ${cap}
    
    outLogFile="LogROOT_ECalEventBuilding_${runNumber#*"0"}.root"

    outputName="${outputFolder}/${cap}pF_${outputBaseName}"
    
    /home/hecgc/Physics/Repos/SiWECAL-LCIO-Analysis/eventbuilding/app/ECal_EventBuilding -i ${file} -o ${outputName} -p ${buildOption} -r ${runNumber} -t siwecaldecoded --configuration_file=${configFile} -c ${commissioningFolder}/ --pedestals_file_HG=pedestals/20220430/original_layer_sorting/Pedestal_method2_fromMIPScan_LowEnergyElectrons_highgain.txt --pedestals_file_LG=pedestals/20220430/original_layer_sorting/Pedestal_method2_fromMIPScan_LowEnergyElectrons_lowgain.txt --mip_calibration_file_HG=calibration/20220512/original_layer_sorting/MIP_pedestalsubmode2_fromMIPScan_LowEnergyElectrons_highgain.txt --mip_calibration_file_LG=calibration/20220512/original_layer_sorting/MIP_pedestalsubmode2_fromMIPScan_LowEnergyElectrons_lowgain.txt --mapping_file=mapping/IJmapping_type_fev10_flipx1_flipy1.txt --mapping_file_cob=mapping/IJmapping_type_fev11_cob_rotate_flipx1_flipy1.txt

    mv ${outLogFile} ${outputFolder}/${outLogFile}
    
done
