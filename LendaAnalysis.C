





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
#include <TH2F.h>
#include <cmath>
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

  

  

  //prepare files
  ////////////////////////////////////////////////////////////////////////////////////
  TFile *outFile=0;
  
  FileManager * fileMan = new FileManager();
  fileMan->fileNotes = theInputManager.notes;
  
  ///The input trees are put into a TChain
  TChain * inT;
  if (theInputManager.specificFileName !=""){
    inT= new TChain("flt");
    for (int i=theInputManager.StartFile;i<theInputManager.StartFile+numFiles;i++){
      TString s =fileMan->LoadFile(runNum,i,theInputManager.specificFileName);
      inT->Add(s);
      cout<<"Adding file "<<s<<endl;
    }
  }


  Long64_t nentry=(Long64_t) (inT->GetEntries());
  cout <<"The number of entires is : "<< nentry << endl ;
  
  // Openning output Tree and output file

  stringstream temp;
  if ( theInputManager.StartFile !=0 )
    temp<<"~/analysis/run"<<runNum<<"/Run"<<runNum<<"LA"<<theInputManager.StartFile<<".root";
  else 
    temp<<"~/analysis/run"<<runNum<<"/Run"<<runNum<<"LA0"<<".root";
  outFile = new TFile(temp.str().c_str(),"recreate");


  //////////////////////////////////////////////  //////////////////////////////////////////////  //////////////////////////////////////////////  //////////////////////////////////////////////
  //Declare A Bunch Of histograms
  //////////////////////////////////////////////  //////////////////////////////////////////////  //////////////////////////////////////////////  //////////////////////////////////////////////
  Int_t NumOfChannelsInModule=16;

  vector <TH1F*> EnergiesRaw(NumOfChannelsInModule);
  vector <TH1F*> EnergiesNoOFs(NumOfChannelsInModule);  

  vector < vector <TH1F*> > EnergiesRawForN; //outer vector for various Multiplicities
  vector < vector <TH1F*> > EnergiesNoOFsForN; //outer vector for various Multiplicities

  //Defualt Binning For Energy Histograms
  Double_t ERawBinning[3];
  ERawBinning[0]=1000;
  ERawBinning[1]=0;
  ERawBinning[2]=40000;
  
  //Raw Channels Histogram
  TH1F * ChannelsRaw = new TH1F("ChannelsRaw","ChannelsRaw",16,0,15);

  //Channels Histogram for different multiplicities
  vector <TH1F*> ChannelsForN;

  //Energy Cut settings for Channel histograms
  Double_t NumOfChannelECuts = 20;
  Double_t MaxEnergy = 7000;
  vector <TH1F *> ChannelsECut(NumOfChannelECuts);

  TH1F * Multiplicity = new TH1F("Multiplicity","Multiplicity",16,0,15);


  stringstream nameStream;
  for (int i=0;i<NumOfChannelsInModule;i++){
    
    nameStream.str("");
    nameStream<<"ERaw"<<i;
    EnergiesRaw[i]= new TH1F(nameStream.str().c_str(),nameStream.str().c_str(),
				 ERawBinning[0],ERawBinning[1],ERawBinning[2]);
    nameStream.str("");
    nameStream<<"ENoOverFlows"<<i;
    EnergiesNoOFs[i]= new TH1F(nameStream.str().c_str(),nameStream.str().c_str(), // use same binning as raw histos
			       ERawBinning[0],ERawBinning[1],ERawBinning[2]);
  }

  TH2F* test = new TH2F("test","",100,0,10000,100,0,10000);
  

  Int_t MaxMultiplicity=6;
  EnergiesRawForN.resize(MaxMultiplicity);
  EnergiesNoOFsForN.resize(MaxMultiplicity);
  ChannelsForN.resize(MaxMultiplicity);

  //Declare things histograms for particular multiplicities
  for (int j=0;j<MaxMultiplicity;j++){
    for (int i=0;i<NumOfChannelsInModule;i++){
      nameStream.str("");
      nameStream<<"ERaw"<<i<<"N"<<j+1;
      EnergiesRawForN[j].push_back( new TH1F(nameStream.str().c_str(),nameStream.str().c_str(),
					 ERawBinning[0],ERawBinning[1],ERawBinning[2]));
      nameStream.str("");
      nameStream<<"ENoOverFlows"<<i<<"N"<<j+1;
      EnergiesNoOFsForN[j].push_back( new TH1F(nameStream.str().c_str(),nameStream.str().c_str(), // use same binning as raw histos
					   ERawBinning[0],ERawBinning[1],ERawBinning[2]));

    }
    nameStream.str("");
    nameStream<<"ChannelsN"<<j+1;
    ChannelsForN[j] = new TH1F(nameStream.str().c_str(),nameStream.str().c_str(),16,0,15);
  }


  for (int i=0;i<NumOfChannelECuts;i++){
    nameStream.str("");
    nameStream<<"ChannelsECut_"<<floor((i/NumOfChannelECuts)*MaxEnergy);

    ChannelsECut[i] = new  TH1F(nameStream.str().c_str(),nameStream.str().c_str(),16,0,15);
  }




  //////////////////////////////////////////////  //////////////////////////////////////////////  //////////////////////////////////////////////  //////////////////////////////////////////////
  //Main Analysis Loop
  //////////////////////////////////////////////  //////////////////////////////////////////////  //////////////////////////////////////////////  //////////////////////////////////////////////

  LendaEvent * theEvent = new LendaEvent();
  inT->SetBranchAddress("Event",&theEvent);
  for (Long64_t jentry=0;jentry<nentry;jentry++){ // LOOP OVER ALL EVENTS
    inT->GetEntry(jentry);

    for (int i=0;i<theEvent->N;i++){ //Loop Over all Channel Firings in the Event
      //Fill Raw energy Histogram
      EnergiesRaw[theEvent->channels[i]]->Fill(theEvent->energies[i]);
      
      ///Fill Raw Energy Histograms for particlar Multiplicities
      EnergiesRawForN[theEvent->N-1][theEvent->channels[i]]->Fill(theEvent->energies[i]);
      
      if (theEvent->OverFlows[i] == false){
	//Fill Energies no OFs for particular Multiplicities
	EnergiesNoOFsForN[theEvent->N-1][theEvent->channels[i]]->Fill(theEvent->energies[i]);
	//Fill Raw No overflow histogram
	EnergiesNoOFs[theEvent->channels[i]]->Fill(theEvent->energies[i]);
      }
      ///Fill Raw Channels Histogram
      ChannelsRaw->Fill(theEvent->channels[i]);
      
      //Fill Channels Histogram with energy cut and remove OFs
      for (int a=0;a<ChannelsECut.size();a++){
	if (theEvent->energies[i]>(double(a)/ChannelsECut.size())*MaxEnergy && theEvent->OverFlows[i]==false){
	  ChannelsECut[a]->Fill(theEvent->channels[i]);
	}
      }
      //Fill channels for different multiplicities 
      ChannelsForN[theEvent->N-1]->Fill(theEvent->channels[i]);
				      
    }//End Loop over Channels In Event
    
    if (theEvent->N==2)
      test->Fill(theEvent->energies[0],theEvent->energies[1]);

    Multiplicity->Fill(theEvent->N);


    if ( jentry % 100000 == 0){
      cout<<"ON "<<jentry<<endl;
    }

  }//End Main Loop over Everything


  //////////////////////////////////////////////
  //Write all the histograms to file
  //////////////////////////////////////////////

  for (int i=0;i<EnergiesNoOFsForN.size();i++){
    for (int j=0;j<EnergiesNoOFsForN[i].size();j++){
	EnergiesNoOFsForN[i][j]->Write();	
	EnergiesRawForN[i][j]->Write();
      }
  }

  for (int i=0;i<ChannelsForN.size();i++){
    ChannelsForN[i]->Write();
  }
  
  for (int i=0;i<EnergiesRaw.size();i++){
    EnergiesRaw[i]->Write();
    EnergiesNoOFs[i]->Write();
  }

  for (int i=0;i<ChannelsECut.size();i++){
    ChannelsECut[i]->Write();
  }
  ChannelsRaw->Write();

  Multiplicity->Write();
  test->Write();
  outFile->Close();
  
  cout<<"\n\n*****Finished******"<<endl;

  return 0;
}
