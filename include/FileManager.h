//
//  File managerment class
//

#include "TFile.h"
#include <sstream>
#include <string>
using namespace std;
class FileManager {

public:
  FileManager();
  TString loadFile(Int_t runNum,Int_t );
  
  TString LoadFile(Int_t runNum,Int_t,string Template);
  
  inline TString GetOutPutFileName(){return outputFileName.str();}

  TFile * getOutputFile();
  TFile * getOutputFile(Double_t FL, Double_t FG, Double_t d, Double_t w);
  TFile * getOutputFile(Double_t sigma);
  TFile * getOutputFile(Int_t LG, Int_t SG); 
  TFile * getOutputFile(string );
  

  
  //File name streams
  std::stringstream fileName;
  std::stringstream outputFileName;
  
  Int_t frunNum;

  string fileNotes;
  

};
