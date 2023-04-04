#!/bin/bash

scanValues=( 9 11 13 15 17 19 )

for value in "${scanValues[@]}"; do

	Marlin steer/MIPAnalysis.xml \
	    --constant.ScanValue="${value}" 
        
	mv TB_ROOT_MIPAnalysis.root TB_ROOT_MIPAnalysis_Scan${value}.root
        
	root -l ./Macros/EffAndMult.cpp\(\"TB_ROOT_MIPAnalysis_Scan${value}.root\"\,\"EffAndMultTech_Scan${value}\"\,\"Tech\"\)

done

