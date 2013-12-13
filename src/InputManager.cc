



#include "InputManager.hh"
#include <vector>
#include <string>
#include <iomanip>

//



InputManager::InputManager()
{
  //set defualts
  runNum=-1;
  numFiles=1;//assume 1 file
  timingMode="softwareCFD";

  specificFileName="";

  //defualt Filter settings see pixie manual
  FL=3;
  FG=0;
  d=3; //in clock ticks
  w =0;
  ext_flag=false;//defualt to none meta run format

  long_gate =17;
  short_gate=7;

  long_gate2 =17;
  short_gate2=5;

  UpdateAll=false;

  reMakePulseShape=false;
  sigma=1.0;

  GammaPeak=4.08274;
  GammaPeak2=0.249663;
  TOFFudge=1.0;

  nothingWrong=true;

  traceDelay=50;
  fast=false;
  lean=false;
  maxEntry=-1;
  ext_sigma_flag=false;

  notes="";

  isDDASEvent=false;

  validTimingModes.push_back("internalCFD");
  validTimingModes.push_back("softwareCFD");
  validTimingModes.push_back("fitting");
  BuildInputMap();
}
InputManager::~InputManager()
{

}

void InputManager::BuildInputMap(){

  ValidNumericalInputs["numfiles"]=&numFiles;
  ValidNumericalInputs["fl"]=&FL;
  ValidNumericalInputs["fg"]=&FG;
  ValidNumericalInputs["w"]=&w;
  ValidNumericalInputs["d"]=&d;
  ValidNumericalInputs["lg"]=&long_gate;
  ValidNumericalInputs["sg"]=&short_gate;
  ValidNumericalInputs["lg2"]=&long_gate2;
  ValidNumericalInputs["sg2"]=&short_gate2;
  ValidNumericalInputs["maxentry"]=&maxEntry;
  ValidNumericalInputs["gammapeak"]=&GammaPeak;
  ValidNumericalInputs["gammapeak2"]=&GammaPeak2;
  ValidNumericalInputs["toffudge"]=&TOFFudge;
  
  // ValidBoolInputs["remake"]=&reMakePulseShape;

  ValidStringInputs["timingmode"]=&timingMode;
  ValidStringInputs["notes"]=&notes;
  ValidStringInputs["inputfile"]=&specificFileName;
  
  ValidBoolInputs["updateall"]=&UpdateAll;
  ValidBoolInputs["isddasevent"]=&isDDASEvent;



  for (map<string,Double_t *>::iterator ii = ValidNumericalInputs.begin();
       ii!=ValidNumericalInputs.end();ii++){
    ChangedInputsMap[ii->first]=false;
    
  }
  for (map<string,Bool_t *>::iterator ii = ValidBoolInputs.begin();
       ii!=ValidBoolInputs.end();ii++){
        ChangedInputsMap[ii->first]=false;
  }
  for (map<string,string *>::iterator ii = ValidStringInputs.begin();
       ii!=ValidStringInputs.end();ii++){
    ChangedInputsMap[ii->first]=false;    
  }
}


Bool_t InputManager::CheckOption(string s){

  if ( ChangedInputsMap.count(s) != 0 ){

    return ChangedInputsMap[s];
  }else {
    throw "crap";
  }

}

Bool_t InputManager::loadInputs2(vector <string> & inputs){

  for (int i=0;i<(int)inputs.size();i++){
    if (lowerCase(inputs[i]) == "help"){
      DumpAllOpitions();
      return false;
    }
  }

  vector <string> Flags;
  vector <string> Arguments;


  vector <string> temp(2,"");
  //the first input has to be the run number
  
  if ( atoi(inputs[0].c_str() ) == 0 )
    cout<<"Must supply a integer runNumber"<<endl;
  else
    runNum = atoi(inputs[0].c_str());
  
  for (int i =1;i<(int) inputs.size();++i){
    
    temp = split(inputs[i],':');
    if (temp.size() != 2 ){//input was not a:v
      cout<<"***Warning input "<<inputs[i]<<" not recognized***"<<endl;
      nothingWrong=false;
    } else{
      if (lowerCase(temp[0])=="inputfile"){
	Flags.push_back(lowerCase(temp[0]));
	Arguments.push_back(temp[1]);
      }else {
	Flags.push_back(lowerCase(temp[0]));
	Arguments.push_back(lowerCase(temp[1]));
      }
    }
  }

  for ( int i=0;i<(int)Flags.size();i++){
    if (ValidBoolInputs.find(Flags[i])!=ValidBoolInputs.end()){
      //Bool option found
      if (Arguments[i] == "true" || Arguments[i] == "yes" ||
	  Arguments[i] == "1"){

	*(ValidBoolInputs[Flags[i]])=true;
      }else if ( Arguments[i] == "false" || Arguments[i] == "no" ||
		 Arguments[i] == "0"){
	

	*(ValidBoolInputs[Flags[i]])=false;
      } else {
	// bool argument not found 
	cout<<"***Warning "<<Arguments[i]<<" Is not a valid argument for bool flag: "<<Flags[i]<<endl;
	nothingWrong=false;
      }
    } else if ( ValidNumericalInputs.find(Flags[i]) != ValidNumericalInputs.end()){
      // Numerical option found
      *(ValidNumericalInputs[Flags[i]])=atof(Arguments[i].c_str());
      
      //Special internal flag setting
      if (Flags[i] == "fl" || Flags[i]=="fg" || Flags[i]=="w" ||Flags[i]=="d"){
	ext_flag=true;
      } else if (Flags[i]=="sg" || Flags[i]=="lg"){
	reMakePulseShape=true;
      }

    } else if (ValidStringInputs.find(Flags[i]) != ValidStringInputs.end()){
      //a string input
      *(ValidStringInputs[Flags[i]]) = Arguments[i];
    } else {
      cout<<"Unkown option "<<Flags[i]<<endl;
      nothingWrong=false;
    }
    ///mark any known flags as havign bee updated
    if (ChangedInputsMap.count(Flags[i])!=0){
      ChangedInputsMap[Flags[i]]=true;
    }

  }

  PrintValues();

  return checkValues();
}

string InputManager::lowerCase(string data){
  std::transform(data.begin(), data.end(), data.begin(), ::tolower);
  return data;
}

vector <string>  InputManager::split (const string &s, char delim, vector<string> &elems) {
  stringstream ss(s);
  string item;
  while(getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}


vector <string> InputManager::split(const string &s, char delim) {
  vector<string> elems;
  return split(s, delim, elems);
}






Bool_t InputManager::checkValues()
{
  //  Bool_t nothingWrong=true;
  Bool_t timingBool=false;
  
  if (numFiles <=0 || runNum <=0 )
    nothingWrong=false;

  
  for (int i=0;i<(int)validTimingModes.size();++i){
    if (timingMode == validTimingModes[i])
      timingBool=true;
  }
  
  if (timingBool == false ){
    nothingWrong =false;
    dumpValidModes();
  }

  if ( specificFileName==""&&isDDASEvent==false){
    nothingWrong =false;
    cout<<"You must specify an inputFile if you are starting with a LendaEvent Tree. inputFile:Name_of_File "<<endl;
  }
  
    


  return nothingWrong;
}

void InputManager::dumpValidModes(){
  cout<<"The valid timing modes are "<<endl;
  for (int i=0;i<(int) validTimingModes.size();++i)
    cout<<validTimingModes[i]<<endl;
}


void InputManager::PrintValues(){
  
  int width=10;
  
  cout<<"\n####Settings for Corrector####\n"<<endl;

  for (map<string,Double_t *>::iterator ii=ValidNumericalInputs.begin();
       ii!=ValidNumericalInputs.end();ii++){
    cout<<"Flag "<<setw(width)<<ii->first<<"   Value "<<setw(width)<<*ii->second<<endl;
  }

  
  for (map<string,Bool_t *>::iterator ii=ValidBoolInputs.begin();
       ii!=ValidBoolInputs.end();ii++){
    cout<<"Flag "<<setw(width)<<ii->first<<"   Value "<<setw(width)<<*ii->second<<endl;
  }

  
  for (map<string,string *>::iterator ii=ValidStringInputs.begin();
       ii!=ValidStringInputs.end();ii++){
    cout<<"Flag "<<setw(width)<<ii->first<<"   Value "<<setw(width)<<*ii->second<<endl;
  }
  
  cout<<"\n##############################\n"<<endl;
}

void InputManager::DumpAllOpitions(){

  int width =10;
  cout<<"\n####Valid Option Information for Corrector####\n"<<endl;

  cout<<"\nValid Flags that take numerical inputs are: \n"<<endl;
  for (map<string,Double_t *>::iterator ii=ValidNumericalInputs.begin();
       ii!=ValidNumericalInputs.end();ii++){
    cout<<"Flag "<<setw(width)<<ii->first<<" Default Value "<<setw(width)<<*ii->second<<endl;
  }

  cout<<"\nValid Flags that take bool inputs are: \n"<<endl;
  for (map<string,Bool_t *>::iterator ii=ValidBoolInputs.begin();
       ii!=ValidBoolInputs.end();ii++){
    cout<<"Flag "<<setw(width)<<ii->first<<" Default Value "<<setw(width)<<*ii->second<<endl;
  }

  cout<<"\nValid Flags that take string inputs are: \n"<<endl;
  for (map<string,string *>::iterator ii=ValidStringInputs.begin();
       ii!=ValidStringInputs.end();ii++){
    cout<<"Flag "<<setw(width)<<ii->first<<" Default Value "<<setw(width)<<*ii->second<<endl;
  }

  cout<<"\n#############################################\n"<<endl;
}

void InputManager::DumpInputsToFile(){




}
