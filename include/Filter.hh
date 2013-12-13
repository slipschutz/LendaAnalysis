

#include <vector>
#include <iostream>
#include "TFile.h"
#include "TMatrixD.h"


class Filter {


public:

  Filter();
  

  
  void FastFilter(std::vector <UShort_t> &trace,
		  std::vector <Double_t> &thisEventsFF,Double_t FL,Double_t FG);
  
  void FastFilterFull(std::vector <UShort_t> &trace,
		      std::vector <Double_t> &thisEventsFF,
		      Double_t FL,Double_t FG,Double_t decayTime);


  
  std::vector <Double_t> CFD( std::vector <Double_t> &,Double_t,Double_t);

  
  Double_t GetZeroCrossing(std::vector <Double_t> &);

  Double_t GetZeroCubic(std::vector <Double_t> &);
  
  Double_t fitTrace(std::vector <UShort_t> &,Double_t, Double_t );
  
  Double_t getEnergy(std::vector <UShort_t> &trace);

  Double_t getGate(std::vector <UShort_t> &trace,int start,int L);

  double getFunc(TMatrixD, double);

  Double_t numOfBadFits;

};
