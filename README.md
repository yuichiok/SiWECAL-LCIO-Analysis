Process to compile and run the ROOT to LCiO converter

First source ilcsoft
Then run in the eventbuilding folder ./script/build Full
To run eventbuilding/app/ECal_EventBuilding --help for a list of all options. The program needs to find all calibration, masking, pedestals, config and mapping files to start. Tune the arguments as necessary.
The default output name is SiWECal_TB2021.lcio

Same procedure for to compile and the EventDisplay.
Bugs and doubts to: Hector.Garcia2@ciemat.es
