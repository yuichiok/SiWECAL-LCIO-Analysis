#ifndef __ECAL_EVENTSCHEME__
#define __ECAL_EVENTSCHEME__

//#include <vector>
#include <map>

#define SLBDEPTH 15
#define NCHIP 16
#define MEMDEPTH 15
#define NCHANNELS 64

struct RawECALEvent {

  int event;
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

  int lowGain[SLBDEPTH][NCHIP][MEMDEPTH][NCHANNELS];
  int highGain[SLBDEPTH][NCHIP][MEMDEPTH][NCHANNELS];
  int gain_hit_low[SLBDEPTH][NCHIP][MEMDEPTH][NCHANNELS];
  int gain_hit_high[SLBDEPTH][NCHIP][MEMDEPTH][NCHANNELS];

  std::map<std::string, void*> branchesMap = {
						{"event",&event},
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
						{"charge_lowGain",lowGain},
						{"charge_hiGain",highGain},
						{"gain_hit_low",gain_hit_low},
						{"gain_hit_high",gain_hit_high}
						
  };
  
};

#endif
