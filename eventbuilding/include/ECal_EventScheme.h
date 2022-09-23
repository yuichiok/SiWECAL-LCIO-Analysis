#ifndef __ECAL_EVENTSCHEME__
#define __ECAL_EVENTSCHEME__

#include <vector>
#include <map>

#define SLBDEPTH 15
#define NCHIP 16
#define MEMDEPTH 15
#define NCHANNELS 64
#define NSCA 15

#define ARRAYSIZE 9999

struct Branches {

  std::map<std::string, void*> branchesMap = {};
  
};


struct RawECALEvent : Branches {

  int acqNumber; 
  int n_slboards;

  int slot[SLBDEPTH];
  int slboard_id[SLBDEPTH]; 
  int rawTSD[SLBDEPTH];
  int rawAVDD0[SLBDEPTH];
  int rawAVDD1[SLBDEPTH];

  float startACQ[SLBDEPTH];
  float TSD[SLBDEPTH];
  float AVDD0[SLBDEPTH];
  float AVDD1[SLBDEPTH];

  int chipid[SLBDEPTH][NCHIP];
  int nColumns[SLBDEPTH][NCHIP];

  int bcid[SLBDEPTH][NCHIP][MEMDEPTH];
  int corrected_bcid[SLBDEPTH][NCHIP][MEMDEPTH];
  int badbcid[SLBDEPTH][NCHIP][MEMDEPTH];
  int nhits[SLBDEPTH][NCHIP][MEMDEPTH];

  int adc_low[SLBDEPTH][NCHIP][MEMDEPTH][NCHANNELS];
  int adc_high[SLBDEPTH][NCHIP][MEMDEPTH][NCHANNELS];
  int autogainbit_low[SLBDEPTH][NCHIP][MEMDEPTH][NCHANNELS];
  int autogainbit_high[SLBDEPTH][NCHIP][MEMDEPTH][NCHANNELS];
  int hitbit_low[SLBDEPTH][NCHIP][MEMDEPTH][NCHANNELS];
  int hitbit_high[SLBDEPTH][NCHIP][MEMDEPTH][NCHANNELS];
  
  RawECALEvent() {
    
    branchesMap = {
						{"acqNumber",&acqNumber},
						{"n_slboards",&n_slboards},
						{"slot",slot},
						{"slboard_id",slboard_id},
						{"rawTSD", rawTSD},
						{"rawAVDD0",rawAVDD0},
						{"rawAVDD1",rawAVDD1},
						{"startACQ",startACQ},
						{"TSD",TSD},
						{"AVDD0",AVDD0},
						{"AVDD1",AVDD1},
						{"chipid",chipid},
						{"nColumns",nColumns},
						{"bcid",bcid},
						{"corrected_bcid",corrected_bcid},
						{"badbcid",badbcid},
						{"nhits",nhits},
						{"adc_low",adc_low},
						{"adc_high",adc_high},
						{"autogainbit_low",autogainbit_low},
						{"autogainbit_high",autogainbit_high},
						{"hitbit_low",hitbit_low},
						{"hitbit_high",hitbit_high}
    };
  }
  
};


struct BuiltROOTEvent : Branches {

  int event; // Event counter within a cycle
  int spill; // DAQ cycles in which data was written
  int cycle; // Readout number 
  int bcid;
  int bcid_first_sca_full;
  int bcid_merge_end;
  int id_run; // Run number
  int id_dat; // Raw data file from which this event was read 
  int nhit_slab; // Number of slabs with hits
  int nhit_chip; // Number of chips wich hits
  int nhit_chan; // Number of channels with hits
  int nhit_len; // Array sizes. Should be equal to nhit_chan unless zero supression is removed

  float sum_energy;
  float sum_energy_lg;

  int hit_slab[ARRAYSIZE];
  int hit_chip[ARRAYSIZE];
  int hit_chan[ARRAYSIZE];
  int hit_sca[ARRAYSIZE];
  int hit_adc_high[ARRAYSIZE];
  int hit_adc_low[ARRAYSIZE];
  int hit_n_scas_filled[ARRAYSIZE];
  int hit_isHit[ARRAYSIZE];
  int hit_isMasked[ARRAYSIZE];
  int hit_isCommissioned[ARRAYSIZE];

  float hit_x[ARRAYSIZE];
  float hit_y[ARRAYSIZE];
  float hit_z[ARRAYSIZE];
  float hit_energy[ARRAYSIZE];
  float hit_energy_lg[ARRAYSIZE];

  BuiltROOTEvent() {
  
    branchesMap = {
					      {"event",&event},
					      {"spill",&spill},
					      {"cycle",&cycle},
					      {"bcid",&bcid},
					      {"bcid_first_sca_full",&bcid_first_sca_full},
					      {"bcid_merge_end",&bcid_merge_end},
					      {"id_run",&id_run},
					      {"id_dat",&id_dat},
					      {"nhit_slab",&nhit_slab},
					      {"nhit_chip",&nhit_chip},
					      {"nhit_chan",&nhit_chan},
					      {"nhit_len",&nhit_len},
					      {"sum_energy",&sum_energy},
					      {"sum_energy_lg",&sum_energy_lg},
					      {"hit_slab",hit_slab},
					      {"hit_chip",hit_chip},
					      {"hit_chan",hit_chan},
					      {"hit_sca",hit_sca},
					      {"hit_adc_high",hit_adc_high},
					      {"hit_adc_low",hit_adc_low},
					      {"hit_n_scas_filled",hit_n_scas_filled},
					      {"hit_isHit",hit_isHit},
					      {"hit_isMasked",hit_isMasked},
					      {"hit_isCommissioned",hit_isCommissioned},
					      {"hit_x",hit_x},
					      {"hit_y",hit_y},
					      {"hit_z",hit_z},
					      {"hit_energy",hit_energy},
					      {"hit_energy_lg",hit_energy_lg}					      
    };
  }
    
};

#endif
