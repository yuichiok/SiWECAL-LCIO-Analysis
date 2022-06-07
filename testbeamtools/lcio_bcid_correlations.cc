#include "lcio.h"
#include <stdio.h>

#include "IO/LCReader.h"
#include "IMPL/LCTOOLS.h"
#include "EVENT/LCRunHeader.h" 

//#include "EVENT/SimCalorimeterHit.h" 
#include "EVENT/CalorimeterHit.h" 
//#include "EVENT/RawCalorimeterHit.h" 
#include "EVENT/ReconstructedParticle.h"
#include "UTIL/CellIDDecoder.h"
#include "UTIL/Operators.h"
#include "UTIL/LCIterator.h"

//---- ROOT stuff --------------
#include "TString.h"
#include "TApplication.h"
#include "TCanvas.h"
#include "TH1I.h"
#include "TH2F.h"

//----------------------

#include <cstdlib>

using namespace std ;
using namespace lcio ;

//static FILE* _file = 0 ;

std::vector<TH1I* > h_bcid_layer;
std::vector<TH1I* > h_bcid_diff_layer;
std::vector<TH2F* > h_bcid_correl_layer;

std::map<int, int> layerOrder = { //{module,layer}
				 { 1, 0 }, { 2, 1 }, { 3, 2 }
				 //         { 4, 3 }, { 5, 4 }                                                                                                                                     
};

int getPlaneNumberFromCHIPID(int chipid) {
  //return (chipid >> 8);                
  int module = (chipid >> 8);
  //   std::cout << " CHIP  " << chipid << " is in Module " << module << std::endl;
  auto searchIterator = layerOrder.find(module);
  if (searchIterator == layerOrder.end()) {
    std::cout << "Module " << module << " is not in mapping";
    return -1;
  }
  auto Layer = searchIterator->second;
  return Layer;
  //   return result;                                                              
}


int main(int argc, char** argv ){


  char* FILEN ;
  // read file name from command line (only argument) 
  if( argc < 3) {
    
    cout << "    usage: lcio_bcid_correlations filename refLayer" << endl ;
    exit(1) ;
  }
 
  FILEN = argv[1] ;
  int ref_layer = atoi(argv[2]);

  for(int ilayer=0; ilayer<15+3; ilayer++) {
    TH1I* temp = new TH1I(TString::Format("h_bcid_layer%i",ilayer),TString::Format("h_bcid_layer%i",ilayer),4096,-0.5,4095.5);
    h_bcid_layer.push_back(temp);
    
    TH1I* temp2 = new TH1I(TString::Format("h_bcid_diff_layer%i",ilayer),TString::Format("h_bcid_diff_layer%i",ilayer),101,-50.5,50.5);//8193,-4096.5,4096.5);
    h_bcid_diff_layer.push_back(temp2);

    TH2F* temp3 = new TH2F(TString::Format("h_bcid_correl_layer%i",ilayer),TString::Format("h_bcid_correl_layer%i",ilayer),4096,-0.5,4095.5,4096,-0.5,4095.5);
    h_bcid_correl_layer.push_back(temp3);
        
  }
  
  //--------------------------------------------------------------------------------
  LCReader* lcReader = LCFactory::getInstance()->createLCReader(LCReader::directAccess) ;
  
  LCEvent* evt(0) ;

  try{
    
    lcReader->open( FILEN ) ;
  
    TApplication app("lcio_bcid_correlations", &argc, argv, 0, 0);
    //    _file = fopen( "lcio_bcid_correlations.txt" , "w" ) ; 
    int counter=0;
    int nevents= lcReader->getNumberOfEvents ();
    for(int ievent=0; ievent<nevents; ievent++) {
      evt = lcReader->readNextEvent() ; 
      
      //------------------------------------------------
      if(evt==0) continue;
      const std::vector<std::string> *pCollectionNames = evt->getCollectionNames();

      std::map<int, std::vector<int>> bcids_layer;
      std::map<int, std::vector<int>>::iterator it;
      
      for(int ilayer=0; ilayer<15+3; ilayer++) {
	it=bcids_layer.find(ilayer);
	if(it != bcids_layer.end()) {
	  std::vector<int> temp;
	  bcids_layer[ilayer]=temp;
	}
      }
      
      for(std::vector<std::string>::const_iterator colIter = pCollectionNames->begin();
	  colIter != pCollectionNames->end() ; ++colIter)
	{

  
	  const std::string &collectionName(*colIter);
	  
	  EVENT::LCCollection *pLCCollection = evt->getCollection(collectionName);
	  
	  if(collectionName=="EUDAQDataScCAL") {
	    if(pLCCollection->getTypeName() == EVENT::LCIO::LCGENERICOBJECT)
	      {
		const int nElements = pLCCollection->getNumberOfElements();
		for(int e=0 ; e<nElements ; e++)
		  {
		    const EVENT::LCGenericObject *const pAHCALRaw = 
		      dynamic_cast<const EVENT::LCGenericObject *const>(pLCCollection->getElementAt(e));
		    //		    cout<<pAHCALRaw->getIntVal(0)<<" AHCAL BCID: "<<pAHCALRaw->getIntVal(1)<<endl;
		    int chipid=pAHCALRaw->getIntVal(3);
		    int layer=getPlaneNumberFromCHIPID(chipid);
		    int bcid=pAHCALRaw->getIntVal(1);
		    h_bcid_layer.at(15+layer)->Fill(bcid);
		    bcids_layer[15+layer].push_back(bcid);
		  }
	      }
	  }
	  
	  if(collectionName=="EUDAQDataSiECAL") {
	    if(pLCCollection->getTypeName() == EVENT::LCIO::LCGENERICOBJECT)
	      {
		const int nElements = pLCCollection->getNumberOfElements();
		for(int e=0 ; e<nElements ; e++)
		  {
		    const EVENT::LCGenericObject *const pSiECALRaw = 
		      dynamic_cast<const EVENT::LCGenericObject *const>(pLCCollection->getElementAt(e));
		      int layer=pSiECALRaw->getIntVal(3);
		      int bcid=pSiECALRaw->getIntVal(1);
		      h_bcid_layer.at(layer)->Fill(bcid);
		      bcids_layer[layer].push_back(bcid);
		  }
	      }
	  }
	}
      for(int ilayer=0; ilayer<15+3; ilayer++) {
	if(bcids_layer[ref_layer].size()>0 && bcids_layer[ilayer].size()>0 ) {
	  for(int i=0; i<bcids_layer[ref_layer].size(); i++) {
	    for(int j=0; j<bcids_layer[ilayer].size(); j++) {
	      h_bcid_diff_layer.at(ilayer)->Fill(bcids_layer[ref_layer].at(i)-bcids_layer[ilayer].at(j));
	      h_bcid_correl_layer.at(ilayer)->Fill(bcids_layer[ref_layer].at(i),bcids_layer[ilayer].at(j));

	    }
	  }
	}
      }
	    
      if ( counter > 100 && counter % 1000 ==0 ) cout<<100.*float(counter)/float(nevents)<<" %"<<endl;
      counter++;
    }
    cout<<"END"<<endl;

    TCanvas *c1 = new TCanvas("BCID_per_layer","BCID_per_layer",1200,1200);
    c1->Divide(4,5);
    for(int ilayer=0; ilayer<15+3; ilayer++) {
      c1->cd(1+ilayer);
      gPad->SetLogy();
      h_bcid_layer.at(ilayer)->GetXaxis()->SetTitleSize(0.06);
      h_bcid_layer.at(ilayer)->GetXaxis()->SetLabelSize(0.06);
      h_bcid_layer.at(ilayer)->GetXaxis()->SetLabelOffset(0.004);
      h_bcid_layer.at(ilayer)->GetXaxis()->SetTitleOffset(0.85);
      h_bcid_layer.at(ilayer)->GetXaxis()->SetTitle(TString::Format("Bcid Layer_%i",ilayer));
      h_bcid_layer.at(ilayer)->Draw("histo");
    }

    TCanvas *c2 = new TCanvas(TString::Format("BCIDDiff_ref=%i",ref_layer),TString::Format("BCIDDiff_ref=%i",ref_layer),1200,1200);
    c2->Divide(4,5);
    for(int ilayer=0; ilayer<15+3; ilayer++) {
      c2->cd(1+ilayer);
      h_bcid_diff_layer.at(ilayer)->GetXaxis()->SetTitle(TString::Format("Bcid Layer_%i - Layer_%i",ref_layer,ilayer));
      h_bcid_diff_layer.at(ilayer)->GetXaxis()->SetLabelSize(0.06);
      h_bcid_diff_layer.at(ilayer)->GetXaxis()->SetTitleSize(0.06);
      h_bcid_diff_layer.at(ilayer)->GetXaxis()->SetLabelOffset(0.004);
      h_bcid_diff_layer.at(ilayer)->GetXaxis()->SetTitleOffset(0.85);   
      h_bcid_diff_layer.at(ilayer)->Draw("histo");
    }
    /*  TCanvas *c3 = new TCanvas(TString::Format("BCIDCorrel_ref=%i",ref_layer),TString::Format("BCIDCorrel_ref=%i",ref_layer),1200,1200);
    c3->Divide(4,5);
    for(int ilayer=0; ilayer<15+3; ilayer++) {
      c3->cd(1+ilayer);
      h_bcid_correl_layer.at(ilayer)->GetXaxis()->SetTitle(TString::Format("Bcid Layer_%i - Layer_%i",ref_layer,ilayer));
      cout<<h_bcid_correl_layer.at(ilayer)->GetEntries()<<endl;
      h_bcid_correl_layer.at(ilayer)->Draw("p");
      }*/


    
    app.Run();

    lcReader->close() ;

    //    fclose( _file ) ;
    
  }  catch( IOException& e) {
    cout << e.what() << endl ;
    exit(1) ;
  }
  
  return 0 ;
}


