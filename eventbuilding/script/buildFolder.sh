#!/bin/bash

if [ -z "$ILCSOFT" ]; then
   echo " You should source an init_ilcsoft.sh script"
   exit
fi

# The paths to folders do not need a '/' at the end

buildFolder="/home/hecgc/Physics/Repos/SiWECAL-LCIO-Analysis/Data/MIPScan/rawfiles" # All the .root files procuded by the conversion from binary to ROOT that want to be built are in this folder
commissioningFolder="/home/hecgc/Physics/Repos/SiWECAL-LCIO-Analysis/commissioning" # A folder where are all the commisioning is grouped together for simpler handling
outputFolder="/home/hecgc/Physics/Repos/SiWECAL-LCIO-Analysis/Data/MIPScan"
outputBaseName="MIPScan_Run_" # The Base name of the output file, the program will add the run number. Example: outputBaseName = "Test_Run_" -> outputFile = ${Prefix}_Test_Run_XXXX.slcio. The prefix is computed in the loop adding info like the run energy or capacitance or anything else you want to include.
buildOption=1 # This changes the build starting point. If following the RawROOT method, then do not change. The option 0 is not ready yet and 1 was to convert from Jonas built file into LCIO.

# ------------ Commisioning Files ------------
# Change this files to the corresponding ones from your Beam Test campaign. If their format changes from the versions of March2022 let me know.
#
# The following files are relative to the commisioningFolder, if you want to use full paths then commisioningFolder=""

configFile="config/March2022/ecalConfiguration_March2022_2.txt"
pedestals_HG="pedestals/20220430/original_layer_sorting/Pedestal_method2_fromMIPScan_LowEnergyElectrons_highgain.txt"
pedestals_LG="pedestals/20220430/original_layer_sorting/Pedestal_method2_fromMIPScan_LowEnergyElectrons_lowgain.txt"
mipCalibration_HG="calibration/20220512/original_layer_sorting/MIP_pedestalsubmode2_fromMIPScan_LowEnergyElectrons_highgain.txt"
mipCalibration_LG="calibration/20220512/original_layer_sorting/MIP_pedestalsubmode2_fromMIPScan_LowEnergyElectrons_lowgain.txt"
mappingFile="mapping/IJmapping_type_fev10_flipx1_flipy1.txt"
mappingCob="mapping/IJmapping_type_fev11_cob_rotate_flipx1_flipy1.txt"

for file in ${buildFolder}/*.root; do

    runNumber=$( echo ${file#*"run_"} | cut -d '_' -f 1 ) # This expects to find the run number in the input file name in the format: run_XXXX.slcio. Change it if necesary
    energy=""
    if [[ "${file}" == *"GeV"* ]]; then
	energy=${file%"GeV"*}
	energy=${energy##*"/"} # The / is the expected separator before the energy value in the file name
	energy="${energy}GeV_"
    fi
    cap=""
    if [[ "${file}" == *"pF"* ]]; then
	cap=${file%"pF"*}
	cap=${cap##*"/"} # The / is the expected separator before the cap value in the file name
	cap="${cap}pF_"
    fi
    
    outLogFile="LogROOT_ECalEventBuilding_${runNumber#*"0"}.root" # Change this name if you want a different naming for the Log ROOT file
    outputName="${outputFolder}/${cap}${energy}${outputBaseName}"
    
    /home/hecgc/Physics/Repos/SiWECAL-LCIO-Analysis/eventbuilding/app/ECal_EventBuilding -i ${file} -o ${outputName} -p ${buildOption} -r ${runNumber} -t siwecaldecoded --configuration_file=${configFile} -c ${commissioningFolder}/ --pedestals_file_HG=${pedestals_HG} --pedestals_file_LG=${pedestals_LG} --mip_calibration_file_HG=${mipCalibration_HG} --mip_calibration_file_LG=${mipCalibration_LG} --mapping_file=${mappingFile} --mapping_file_cob=${mappingCob}

   mv ${outLogFile} ${outputFolder}/${outLogFile}
    
done
