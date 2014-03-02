
#include <iostream>

#include <TChain.h>
#include <TTree.h>


#include "LendaEvent.hh"
#include "DDASEvent.h"
#include "InputManager.hh"
#include "FileManager.h"

#include "Bins.h"

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
  
  BuildBins();
  
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
  vector <TH1F*> EnergiesCorrected(NumOfChannelsInModule);  
  
  vector< vector <TH1F*> > ReferenceEnergiesScaled(100);

  vector < vector <TH1F*> > EnergiesRawForN; //outer vector for various Multiplicities
  vector < vector <TH1F*> > EnergiesNoOFsForN; //outer vector for various Multiplicities
  vector < vector <TH1F*> > EnergiesCorrectedForN;
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

    nameStream.str("");
    nameStream<<"ECor"<<i;
    EnergiesCorrected[i]= new TH1F(nameStream.str().c_str(),nameStream.str().c_str(), // use same binning as raw histos
			       ERawBinning[0],ERawBinning[1],ERawBinning[2]);

  }
  for (int i=0;i<ReferenceEnergiesScaled.size();i++){
    ReferenceEnergiesScaled[i].resize(100);
    for (int j=0;j<ReferenceEnergiesScaled[i].size();j++){
      nameStream.str("");
      nameStream<<"RefScaledE"<<i<<"_"<<j;
      ReferenceEnergiesScaled[i][j] = new TH1F(nameStream.str().c_str(),nameStream.str().c_str(), 
					    500,ERawBinning[1],ERawBinning[2]);
    }
  }
  

  Int_t MaxMultiplicity=6;
  EnergiesRawForN.resize(MaxMultiplicity);
  EnergiesNoOFsForN.resize(MaxMultiplicity);
  EnergiesCorrectedForN.resize(MaxMultiplicity);
  ChannelsForN.resize(MaxMultiplicity);


  TH2F* test = new TH2F("test","",1000,0,20000,1000,0,20000);
  
  TH2F* PulseShapeTrig = new TH2F("PulseShapeTrig","",1000,0,20000,1000,0,5);
  TH2F* LongVShortTrig = new TH2F("LongVShortTrig","",1000,0,20000,1000,0,20000);
  vector <TH2F*> LongVShortTrigForN(MaxMultiplicity);
  vector <TH2F*> PulseShapeTrigForN(MaxMultiplicity);


  TH2F* PulseShapeRef = new TH2F("PulseShapeRef","",1000,0,20000,1000,0,5);
  TH2F* LongVShortRef = new TH2F("LongVShortRef","",1000,0,20000,1000,0,20000);
  vector <TH2F*> LongVShortRefForN(MaxMultiplicity);
  vector <TH2F*> PulseShapeRefForN(MaxMultiplicity);




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

      nameStream.str("");
      nameStream<<"ECor"<<i<<"N"<<j+1;
      EnergiesCorrectedForN[j].push_back( new TH1F(nameStream.str().c_str(),nameStream.str().c_str(), // use same binning as raw histos
					   ERawBinning[0],ERawBinning[1],ERawBinning[2]));

    }
    nameStream.str("");
    nameStream<<"ChannelsN"<<j+1;
    ChannelsForN[j] = new TH1F(nameStream.str().c_str(),nameStream.str().c_str(),16,0,15);
    //Trigger Scint for N
    nameStream.str("");
    nameStream<<"LongVShortTrigN"<<j+1;
    LongVShortTrigForN[j] = new TH2F(nameStream.str().c_str(),"",1000,0,20000,1000,0,20000);

    nameStream.str("");
    nameStream<<"PulseShapeTrigN"<<j+1;
    PulseShapeTrigForN[j] = new TH2F(nameStream.str().c_str(),"",1000,0,20000,1000,0,5);
    //Reference Scint for N
    nameStream.str("");
    nameStream<<"LongVShortRefN"<<j+1;
    LongVShortRefForN[j] = new TH2F(nameStream.str().c_str(),"",1000,0,20000,1000,0,20000);

    nameStream.str("");
    nameStream<<"PulseShapeRefN"<<j+1;
    PulseShapeRefForN[j] = new TH2F(nameStream.str().c_str(),"",1000,0,20000,1000,0,5);

  }
  

  for (int i=0;i<NumOfChannelECuts;i++){
    nameStream.str("");
    nameStream<<"ChannelsECut_"<<floor((i/NumOfChannelECuts)*MaxEnergy);

    ChannelsECut[i] = new  TH1F(nameStream.str().c_str(),nameStream.str().c_str(),16,0,15);
  }

  int nBinsForTOF=16000;
  int nBinsForLLEnergy=200;
  int nBinsForEnergy=1000;
  ///Time of Flight spectra for lenda bar in coincidence with trigger
  TH1F * TOF = new TH1F("TOF","",nBinsForTOF,-150,150);// Time of flight in clock tics raw
  TH1F * TOFPS = new TH1F("TOFPS","",nBinsForTOF,-150,150);// Time of flight in clock tics with PS
  TH1F * ShiftTOF = new TH1F("ShiftTOF","",nBinsForTOF,-150,150);// Time of flight in clock tics After GammaPeak Shift
  TH1F * TOFWithPS = new TH1F("TOFWithPS","",nBinsForTOF,-150,150);// Time of Flight with only Gammas on trig
  TH1F * TOFEnergy = new TH1F("TOFEnergy","",nBinsForTOF,0,150); // the Energy from TOF Raw
  TH1F * TOFEnergyNeutrons = new TH1F("TOFEnergyNeutrons","",nBinsForEnergy,0,20); // TOF energy with gammas cut and PS
  TH1F* TOFEnergyRandomBkg = new TH1F("TOFEnergyRandomBkg","",nBinsForEnergy,0,20);// TOF energy for negative time Of Flights
  TH1F* TOFEnergyRBkgSubtracted = new TH1F("TOFEnergyRBkgSubtracted","",nBinsForEnergy,0,20);// TOFEnergyNeutrons-TOFEnergyRandomBkg 

  TH1F* TOFEnergyCFBins = new TH1F("TOFEnergyCFBins","",70,CFBins);  
  TH1F* TOFEnergyBkgCFBins = new TH1F("TOFEnergyBkgCFBins","",70,CFBins);  
  TH1F* TOFEnergySubCFBins = new TH1F("TOFEnergySubCFBins","",70,CFBins);  
  
  TH2F* EvsTOFEnergy = new TH2F("EvsTOFEnergy","",nBinsForEnergy,0,20,1000,0,20000);
  TH2F* EvsTOFEnergyBkg = new TH2F("EvsTOFEnergyBkg","",nBinsForEnergy,0,20,1000,0,20000);
  
  

  ////Time of Flight spectra for reference scintillator in coincidence with trigger
  TH1F * TOFL_L = new TH1F("TOFL_L","",nBinsForTOF,-150,150);// Time of flight in clock tics raw
  TH1F * TOFPSL_L = new TH1F("TOFPSL_L","",nBinsForTOF,-150,150);// Time of flight in clock tics with PS
  TH1F * ShiftTOFL_L = new TH1F("ShiftTOFL_L","",nBinsForTOF,-150,150);// Time of flight in clock tics After GammaPeak Shift
  TH1F * TOFWithPSL_L = new TH1F("TOFWithPSL_L","",nBinsForTOF,-150,150);// Time of Flight with only Gammas on trig
  TH1F * TOFWith2PSL_L = new TH1F("TOFWith2PSL_L","",nBinsForTOF,-150,150);// Time of Flight with only Gammas on trig
  TH1F * TOFEnergyL_L = new TH1F("TOFEnergyL_L","",nBinsForTOF,0,150); // the Energy from TOF Raw
  TH1F * TOFEnergyNeutronsL_L = new TH1F("TOFEnergyNeutronsL_L","",nBinsForLLEnergy,0,20); // TOF energy with gammas cut and PS
  TH1F * TOFEnergyRandomBkgL_L = new TH1F("TOFEnergyRandomBkgL_L","",nBinsForLLEnergy,0,20);// TOF energy for negative time Of Flights
  TH1F * TOFEnergyNeutrons1PSL_L = new TH1F("TOFEnergyNeutrons1PSL_L","",nBinsForLLEnergy,0,20); // TOF energy with gammas cut and PS
  TH1F * TOFEnergyRandomBkg1PSL_L = new TH1F("TOFEnergyRandomBkg1PSL_L","",nBinsForLLEnergy,0,20);// TOF energy for negative time Of Flights
  TH1F * TOFEnergyRBkgSubtracted1PSL_L = new TH1F("TOFEnergyRBkgSubtracted1PSL_L","",nBinsForLLEnergy,0,20);//
  TH1F* TOFEnergyCFBinsL_L = new TH1F("TOFEnergyCFBinsL_L","",70,CFBins);  
  TH1F* TOFEnergyBkgCFBinsL_L = new TH1F("TOFEnergyBkgCFBinsL_L","",70,CFBins);  
  TH1F* TOFEnergySubCFBinsL_L = new TH1F("TOFEnergySubCFBinsL_L","",70,CFBins);  

  TH2F* EvsTOFEnergyL_L = new TH2F("EvsTOFEnergyL_L","",nBinsForLLEnergy,0,20,1000,0,20000);
  TH2F* EvsTOFEnergyBkgL_L = new TH2F("EvsTOFEnergyBkgL_L","",nBinsForLLEnergy,0,20,1000,0,20000);

  double RefScintThresh=313.619;
  vector <TH1F*> ThreshHoldVaryL_L(100);
  vector <TH1F*> ThreshHoldVaryBkgL_L(100);
  vector <TH1F*> ThreshHoldVaryResultL_L(100);
  for (int i=0;i<ThreshHoldVaryL_L.size();i++){
    nameStream.str("");
    nameStream<<"ThreshHoldVaryL_L"<<i;
    ThreshHoldVaryL_L[i] = new TH1F(nameStream.str().c_str(),"",nBinsForLLEnergy,0,20);

    nameStream.str("");
    nameStream<<"ThreshHoldVaryBkgL_L"<<i;
    ThreshHoldVaryBkgL_L[i] = new TH1F(nameStream.str().c_str(),"",nBinsForLLEnergy,0,20);

    nameStream.str("");
    nameStream<<"ThreshHoldVaryResultL_L"<<i;
    ThreshHoldVaryResultL_L[i] = new TH1F(nameStream.str().c_str(),"",nBinsForLLEnergy,0,20);
  }


  //////////////////////////////////////////////  //////////////////////////////////////////////  //////////////////////////////////////////////  //////////////////////////////////////////////
  //Main Analysis Loop
  //////////////////////////////////////////////  //////////////////////////////////////////////  //////////////////////////////////////////////  //////////////////////////////////////////////

  LendaEvent * theEvent = new LendaEvent();
  inT->SetBranchAddress("Event",&theEvent);
  for (Long64_t jentry=0;jentry<nentry;jentry++){ // LOOP OVER ALL EVENTS
    inT->GetEntry(jentry);//Get the Event From The TREE

    for (int i=0;i<theEvent->N;i++){ //Loop Over all Channel Firings in the Event
      //Fill Raw energy Histogram
      EnergiesRaw[theEvent->channels[i]]->Fill(theEvent->energies[i]);

      if (theEvent->channels[i]==8&&theEvent->N==2){
	for (int a=0;a<ReferenceEnergiesScaled.size();a++){
	  for (int b=0;b<ReferenceEnergiesScaled[a].size();b++){
	    //	    ReferenceEnergiesScaled[a][b]->Fill(theEvent->energies[i]*(0.1+(double(a)/100.0) + b*1.0));
	    ReferenceEnergiesScaled[a][b]->Fill((theEvent->energies[i])*((double(a-50)/100))+(b-50)*10);
	  }
	}
      }
      
      ///Fill Raw Energy Histograms for particlar Multiplicities
      EnergiesRawForN[theEvent->N-1][theEvent->channels[i]]->Fill(theEvent->energies[i]);
      
      if (theEvent->OverFlows[i] == false){
	//Fill Energies no OFs for particular Multiplicities
	EnergiesNoOFsForN[theEvent->N-1][theEvent->channels[i]]->Fill(theEvent->energies[i]);
	//Fill Raw No overflow histogram
	EnergiesNoOFs[theEvent->channels[i]]->Fill(theEvent->energies[i]);
	//Fill Corrected Energies 
	EnergiesCorrected[theEvent->channels[i]]->Fill(theEvent->energiesCor[i]);
	EnergiesCorrectedForN[theEvent->N-1][theEvent->channels[i]]->Fill(theEvent->energiesCor[i]);
	
	if (theEvent->channels[i] == 9 && theEvent->longGates[i]>0){ //Pulse Shape discrimination for the trigger
	  PulseShapeTrig->Fill(theEvent->longGates[i],theEvent->longGates[i]/theEvent->shortGates[i]);
	  LongVShortTrig->Fill(theEvent->shortGates[i],theEvent->longGates[i]);
	  
	  LongVShortTrigForN[theEvent->N-1]->Fill(theEvent->shortGates[i],theEvent->longGates[i]);
	  PulseShapeTrigForN[theEvent->N-1]->Fill(theEvent->longGates[i],theEvent->longGates[i]/theEvent->shortGates[i]);
	}
	if (theEvent->channels[i] == 8 && theEvent->longGates[i]>0){ //Pulse Shape discrimination for the reference
	  PulseShapeRef->Fill(theEvent->longGates[i],theEvent->longGates[i]/theEvent->shortGates[i]);
	  LongVShortRef->Fill(theEvent->shortGates[i],theEvent->longGates[i]);
	  
	  LongVShortRefForN[theEvent->N-1]->Fill(theEvent->shortGates[i],theEvent->longGates[i]);
	  PulseShapeRefForN[theEvent->N-1]->Fill(theEvent->longGates[i],theEvent->longGates[i]/theEvent->shortGates[i]);
	}
	if (theEvent->N == 2 && theEvent->channels[0]==8 && theEvent->channels[1]==9&&theEvent->channels[i]==8){
	  if (TMath::Abs(theEvent->times[0]-theEvent->times[1]) < 35 ){
	    test->Fill(theEvent->shortGates[i],theEvent->longGates[i]);
	  }
	}
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
    
    //Fill total event histograms like TOF
    //For the Lenda bar in coincidence with the trigger
    if (theEvent->N==3 && theEvent->channels[0]==0 && theEvent->channels[1]==1 &&theEvent->channels[2]==9){
      TOF->Fill(theEvent->TOF);
      ShiftTOF->Fill(theEvent->ShiftTOF);
      TOFEnergy->Fill(theEvent->TOFEnergy);
      if (theEvent->longGates[2]>1200 && theEvent->longGates[2]/theEvent->shortGates[2]<1.14){
	TOFWithPS->Fill(theEvent->ShiftTOF);
	TOFPS->Fill(theEvent->TOF); // just ps no shift
	if (theEvent->ShiftTOF > 2.5 && theEvent->ShiftTOF<100){
	  TOFEnergyNeutrons->Fill(theEvent->TOFEnergy);
	  TOFEnergyCFBins->Fill(theEvent->TOFEnergy);
	  EvsTOFEnergy->Fill(theEvent->TOFEnergy,sqrt(theEvent->energies[0]*theEvent->energies[1]));
	}
	if (theEvent->ShiftTOF < -2.5 && theEvent->ShiftTOF>-100){
	  TOFEnergyRandomBkg->Fill(theEvent->TOFEnergy);
	  TOFEnergyBkgCFBins->Fill(theEvent->TOFEnergy);
	  EvsTOFEnergyBkg->Fill(theEvent->TOFEnergy,sqrt(theEvent->energies[0]*theEvent->energies[1]));
	}
      }
    }
    //for the reference in coincidence with the trigger
    if (theEvent->N==2 && theEvent->channels[0]==8 && theEvent->channels[1]==9){
      TOFL_L->Fill(theEvent->TOF);
      ShiftTOFL_L->Fill(theEvent->ShiftTOF);
      TOFEnergyL_L->Fill(theEvent->TOFEnergy);
      if (theEvent->longGates[1]>1200 && theEvent->longGates[1]/theEvent->shortGates[1]<1.14){
	TOFWithPSL_L->Fill(theEvent->ShiftTOF);
	TOFPSL_L->Fill(theEvent->TOF);
	
	
	if (theEvent->longGates[0]>2000 && theEvent->longGates[0]/theEvent->shortGates[0]>1.06){	
	  TOFWith2PSL_L->Fill(theEvent->ShiftTOF);
	  
	  if (theEvent->ShiftTOF > 2 && theEvent->ShiftTOF<35){
	    TOFEnergyNeutronsL_L->Fill(theEvent->TOFEnergy);

	  }
	  if (theEvent->ShiftTOF < -2 && theEvent->ShiftTOF>-35){
	    TOFEnergyRandomBkgL_L->Fill(theEvent->TOFEnergy);
	  }
	} //end if over second pusle shape discrimination	
	
	//Fill the histograms that have the specifed threhold cut in them
	if (theEvent->ShiftTOF > 2 && theEvent->ShiftTOF<35){
	  if ( theEvent->energies[0]>RefScintThresh){
	    TOFEnergyNeutrons1PSL_L->Fill(theEvent->TOFEnergy);
	    TOFEnergyCFBinsL_L->Fill(theEvent->TOFEnergy);
	    EvsTOFEnergyL_L->Fill(theEvent->TOFEnergy,theEvent->energies[0]);
	  }
	  for (int a=0;a<ThreshHoldVaryL_L.size();a++){
	    int size =ThreshHoldVaryL_L.size();
	    int half=size/2;
	    if (theEvent->energies[0] > ( RefScintThresh + 3*(double(a-50)) ) ){
	      ThreshHoldVaryL_L[a]->Fill(theEvent->TOFEnergy);
	    }
	  }
	  

	}
	if (theEvent->ShiftTOF < -2 && theEvent->ShiftTOF>-35){
	  if ( theEvent->energies[0]>RefScintThresh){
	    TOFEnergyRandomBkg1PSL_L->Fill(theEvent->TOFEnergy);
	    TOFEnergyBkgCFBinsL_L->Fill(theEvent->TOFEnergy);
	    EvsTOFEnergyBkgL_L->Fill(theEvent->TOFEnergy,theEvent->energies[0]);
	  }
	  for (int a=0;a<ThreshHoldVaryBkgL_L.size();a++){
	    int size =ThreshHoldVaryBkgL_L.size();
	    int half=size/2;
	    if (theEvent->energies[0] > ( RefScintThresh + 3*(double(a-50)) ) ){
	      ThreshHoldVaryBkgL_L[a]->Fill(theEvent->TOFEnergy);
	    }
	  }

	}
      }
    }



    Multiplicity->Fill(theEvent->N);


    if ( jentry % 100000 == 0){
      cout<<"ON "<<jentry<<endl;
    }

  }//End Main Loop over Everything

  ///Do analysis on whole historgrams 
  for (int i=1;i<=TOFEnergyNeutrons->GetXaxis()->GetNbins();i++){
    TOFEnergyRBkgSubtracted->SetBinContent(i,TOFEnergyNeutrons->GetBinContent(i)-TOFEnergyRandomBkg->GetBinContent(i));
    TOFEnergyRBkgSubtracted1PSL_L->SetBinContent(i,TOFEnergyNeutrons1PSL_L->GetBinContent(i)-TOFEnergyRandomBkg1PSL_L->GetBinContent(i));
  }
  for (int i=1;i<TOFEnergySubCFBins->GetXaxis()->GetNbins();i++){
    TOFEnergySubCFBins->SetBinContent(i,TOFEnergyCFBins->GetBinContent(i)-TOFEnergyBkgCFBins->GetBinContent(i));
    TOFEnergySubCFBinsL_L->SetBinContent(i,TOFEnergyCFBinsL_L->GetBinContent(i)-TOFEnergyBkgCFBinsL_L->GetBinContent(i));
  }
  for (int i=0;i<ThreshHoldVaryResultL_L.size();i++){
    int bins =ThreshHoldVaryResultL_L[i]->GetNbinsX();
    for (int j=0;j<bins;j++){
      double val1=ThreshHoldVaryL_L[i]->GetBinContent(j+1);
      double val2=ThreshHoldVaryBkgL_L[i]->GetBinContent(j+1);
      ThreshHoldVaryResultL_L[i]->SetBinContent(j+1,val1-val2);
    }
  }



  //////////////////////////////////////////////
  //Write all the histograms to file
  //////////////////////////////////////////////

  for (int i=0;i<EnergiesNoOFsForN.size();i++){
    for (int j=0;j<EnergiesNoOFsForN[i].size();j++){
	EnergiesNoOFsForN[i][j]->Write();	
	EnergiesRawForN[i][j]->Write();
	EnergiesCorrectedForN[i][j]->Write();  
    }
  }

  for (int i=0;i<ChannelsForN.size();i++){
    ChannelsForN[i]->Write();
  }
  
  for (int i=0;i<EnergiesRaw.size();i++){
    EnergiesRaw[i]->Write();
    EnergiesNoOFs[i]->Write();
    EnergiesCorrected[i]->Write();
  }

  for (int i=0;i<ChannelsECut.size();i++){
    ChannelsECut[i]->Write();
  }

  for (int i=0;i<LongVShortTrigForN.size();i++){
    LongVShortTrigForN[i]->Write();
    PulseShapeTrigForN[i]->Write();
    LongVShortRefForN[i]->Write();
    PulseShapeRefForN[i]->Write();
  }

  ChannelsRaw->Write();

  Multiplicity->Write();
  test->Write();

  PulseShapeTrig->Write();
  LongVShortTrig->Write();
  PulseShapeRef->Write();
  LongVShortRef->Write();


  TOFEnergy->Write();
  TOFEnergyNeutrons->Write();
  ShiftTOF->Write();
  TOF->Write();
  TOFWithPS->Write();
  TOFEnergyRandomBkg->Write();
  TOFEnergyRBkgSubtracted->Write();
  TOFEnergyCFBins->Write();
  TOFEnergyBkgCFBins->Write();
  TOFEnergySubCFBins->Write();
  EvsTOFEnergy->Write();
  TOFPSL_L->Write();
  TOFPS->Write();
  EvsTOFEnergyBkg->Write();
  

  TOFEnergyL_L->Write();
  TOFEnergyNeutronsL_L->Write();
  ShiftTOFL_L->Write();
  TOFL_L->Write();
  TOFWithPSL_L->Write();
  TOFWith2PSL_L->Write();
  TOFEnergyRandomBkgL_L->Write();
  TOFEnergyRBkgSubtracted1PSL_L->Write();
  TOFEnergyCFBinsL_L->Write();
  TOFEnergyBkgCFBinsL_L->Write();
  TOFEnergySubCFBinsL_L->Write();
  TOFEnergyNeutrons1PSL_L->Write();
  TOFEnergyRandomBkg1PSL_L->Write();
  EvsTOFEnergyL_L->Write();
  EvsTOFEnergyBkgL_L->Write();

  for (int i=0;i<ReferenceEnergiesScaled.size();i++){
    for (int j=0;j<ReferenceEnergiesScaled[i].size();j++){
      ReferenceEnergiesScaled[i][j]->Scale(20);
      //	ReferenceEnergiesScaled[i][j]->Write();
    }
  }
  for (int i=0;i<ThreshHoldVaryL_L.size();i++){
    // ThreshHoldVaryL_L[i]->Write();
    // ThreshHoldVaryBkgL_L[i]->Write();
    // ThreshHoldVaryResultL_L[i]->Write();

  }


  outFile->Close();
  
  cout<<"\n\n*****Finished******"<<endl;

  return 0;
}
