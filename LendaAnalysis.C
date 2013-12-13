





#include <iostream>

#include <TChain.h>
#include <TTree.h>


#include "LendaEvent.hh"
#include "DDASEvent.h"
#include "InputManager.hh"
#include "FileManager.h"

#include <vector>
#include <sstream>
#include <TH1F.h>

int main(int argc, char **argv){

  vector <string> inputs;
  for (int i=1;i<argc;++i){
    inputs.push_back(string(argv[i]));
  }
  if (inputs.size() == 0 ){ // no argumnets display helpful message
    cout<<"Usage: ./LendaAnalysis runNumber [options:value]"<<endl;
    return 0;
  }  
  
  InputManager theInputManager;
  if ( !  theInputManager.loadInputs2(inputs) ){
    return 0;
  }

  
  ////////////////////////////////////////////////////////////////////////////////////

  //load settings

  Int_t runNum=theInputManager.runNum;
  Int_t numFiles=theInputManager.numFiles;

  Long64_t maxentry=theInputManager.maxEntry;

  

  Bool_t extFlag=theInputManager.ext_flag;
  Bool_t ext_sigma_flag=theInputManager.ext_sigma_flag;

  //defualt Filter settings see pixie manual
  Double_t FL=theInputManager.FL;
  Double_t FG=theInputManager.FG;
  int CFD_delay=theInputManager.d; //in clock ticks
  Double_t CFD_scale_factor =theInputManager.w;

  

  //prepare files
  ////////////////////////////////////////////////////////////////////////////////////
  TFile *outFile=0;
  
  FileManager * fileMan = new FileManager();
  fileMan->fileNotes = theInputManager.notes;
  
  ///The input trees are put into a TChain
  TChain * inT;
  if (theInputManager.specificFileName !=""){
    inT= new TChain("flt");
    for (int i=0;i<numFiles;i++){
      TString s =fileMan->LoadFile(runNum,i,theInputManager.specificFileName);
      inT->Add(s);
      cout<<"Adding file "<<s<<endl;
    }
  }


  Long64_t nentry=(Long64_t) (inT->GetEntries());
  cout <<"The number of entires is : "<< nentry << endl ;
  
  // Openning output Tree and output file

  outFile = new TFile("TestOut.root","recreate");



  //////////////////////////////////////////////
  //Declare A Bunch Of histograms
  //////////////////////////////////////////////
  Int_t NumOfChannelsInModule=16;
  
  

  vector <TH1F*> EnergiesRaw(NumOfChannelsInModule);
  Double_t ERawBinning[3];
  ERawBinning[0]=1000;
  ERawBinning[1]=0;
  ERawBinning[2]=40000;
  
  
  TH1F * ChannelsRaw = new TH1F("ChannelsRaw","ChannelsRaw",16,0,15);
  TH1F * Multiplicity = new TH1F("Multiplicity","Multiplicity",16,0,15);
  
  stringstream nameStream;
  
  for (int i=0;i<NumOfChannelsInModule;i++){
    nameStream.str("");
    nameStream<<"ERaw"<<i;
    EnergiesRaw[i] = new TH1F(nameStream.str().c_str(),nameStream.str().c_str(),
			      ERawBinning[0],ERawBinning[1],ERawBinning[2]);

  }

  




  //////////////////////////////////////////////
  //Main Analysis Loop
  //////////////////////////////////////////////

  LendaEvent * theEvent = new LendaEvent();
  inT->SetBranchAddress("Event",&theEvent);
  for (Long64_t jentry=0;jentry<nentry;jentry++){ // LOOP OVER ALL EVENTS
    inT->GetEntry(jentry);
    for (int i=0;i<theEvent->N;i++){ //Loop Over all Channel Firings in the Event

      
      ///Fill Raw Energy Histograms
      EnergiesRaw[theEvent->channels[i]]->Fill(theEvent->energies[i]);
      
      ///Fill Raw Channels Histogram
      ChannelsRaw->Fill(theEvent->channels[i]);


    }//End Loop over Channels In Event
    
    Multiplicity->Fill(theEvent->N);
    
    if ( jentry % 100000 == 0){
      cout<<"ON "<<jentry<<endl;
    }

  }//End Main Loop over Everything



  for (int i=0;i<EnergiesRaw.size();i++){
    EnergiesRaw[i]->Write();
  }
  ChannelsRaw->Write();
  Multiplicity->Write();
  outFile->Close();
  
  cout<<"\n\n*****Finished******"<<endl;

  return 0;
}
