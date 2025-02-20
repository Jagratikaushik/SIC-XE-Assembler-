/**Pass 1 of the Two Pass SIC/XE Assembler. It declares functions and tables file as  header files. **/
#include "functions.cpp" 
#include "tables.cpp"

using namespace std;

string fileName;
bool error_flag=false;
int program_length;
string *BLocksNumToName;

string firstExecutable_Sec;

void handle_LTORG(string& litPrefix, int& lineNumberDelta,int lineNumber,int& LOCCTR, int& lastDeltaLOCCTR, int currentBlockNumber){
  string litAddress,litValue;
  litPrefix = "";
  for(auto const& it: LITTAB){
    litAddress = it.second.address;
    litValue = it.second.value;
    if(litAddress!="?"){
    }
    else{
      lineNumber += 5;
      lineNumberDelta += 5;
      LITTAB[it.first].address = intToStringHex(LOCCTR);
      LITTAB[it.first].blockNumber = currentBlockNumber;
      litPrefix += "\n" + to_string(lineNumber) + "\t" + intToStringHex(LOCCTR) + "\t" + to_string(currentBlockNumber) + "\t" + "*" + "\t" + "="+litValue + "\t" + " " + "\t" + " ";

      if(litValue[0]=='X'){
        LOCCTR += (litValue.length() -3)/2;
        lastDeltaLOCCTR += (litValue.length() -3)/2;
      }
      else if(litValue[0]=='C'){
        LOCCTR += litValue.length() -3;
        lastDeltaLOCCTR += litValue.length() -3;
      }
    }
  }
}

void evaluateExpression(string expression, bool& relative,string& tempOperand,int lineNumber, ofstream& errorFile,bool& error_flag){
  string singleOperand="?",singleOperator="?",valueString="",valueTemp="",writeData="";
  int lastOperand=0,lastOperator=0,pairCount=0;
  char lastByte = ' ';
  bool Illegal = false;

  for(int i=0;i<expression.length();){
    singleOperand = "";

    lastByte = expression[i];
    while((lastByte!='+' && lastByte!='-' && lastByte!='/' && lastByte!='*') && i<expression.length()){
      singleOperand += lastByte;
      lastByte = expression[++i];
    }

    if(SYMTAB[singleOperand].exists=='y'){
      lastOperand = SYMTAB[singleOperand].relative;
      valueTemp = to_string(stringHexToInt(SYMTAB[singleOperand].address));
    }
    else if((singleOperand != "" || singleOperand !="?" ) && if_all_num(singleOperand)){
      lastOperand = 0;
      valueTemp = singleOperand;
    }
    else{
      writeData = "Line: "+to_string(lineNumber)+" : Can't find symbol. Found "+singleOperand;
      writeToFile(errorFile,writeData);
      Illegal = true;
      break;
    }

    if(lastOperand*lastOperator == 1){
      writeData = "Line: "+to_string(lineNumber)+" : Illegal expression";
      writeToFile(errorFile,writeData);
      error_flag = true;
      Illegal = true;
      break;
    }
    else if((singleOperator=="-" || singleOperator=="+" || singleOperator=="?")&&lastOperand==1){
      if(singleOperator=="-"){
        pairCount--;
      }
      else{
        pairCount++;
      }
    }
    valueString += valueTemp;

    singleOperator= "";
    while(i<expression.length()&&(lastByte=='+'||lastByte=='-'||lastByte=='/'||lastByte=='*')){
      singleOperator += lastByte;
      lastByte = expression[++i];
    }

    if(singleOperator.length()>1){
      writeData = "Line: "+to_string(lineNumber)+" : Illegal operator in expression. Found "+singleOperator;
      writeToFile(errorFile,writeData);
      error_flag = true;
      Illegal = true;
      break;
    }

    if(singleOperator=="*" || singleOperator == "/"){
      lastOperator = 1;
    }
    else{
      lastOperator = 0;
    }

    valueString += singleOperator;
  }

  if(!Illegal){
    if(pairCount==1){
      relative = 1;
      EvaluateString tempOBJ(valueString);
      tempOperand = intToStringHex(tempOBJ.getResult());
    }
    else if(pairCount==0){
      relative = 0;
      cout<<valueString<<endl;
      EvaluateString tempOBJ(valueString);
      tempOperand = intToStringHex(tempOBJ.getResult());
    }
    else{
      writeData = "Line: "+to_string(lineNumber)+" : Illegal expression";
      writeToFile(errorFile,writeData);
      error_flag = true;
      tempOperand = "00000";
      relative = 0;
    }
  }
  else{
    tempOperand = "00000";
    error_flag = true;
    relative = 0;
  }
}
void pass1(){
  ifstream sourceFile;
  ofstream intermediateFile, errorFile;

  sourceFile.open(fileName);
  if(!sourceFile){
    cout<<"Unable to open file: "<<fileName<<endl;
    exit(1);
  }

  intermediateFile.open("intermediate_" + fileName);
  if(!intermediateFile){
    cout<<"Unable to open file: intermediate_"<<fileName<<endl;
    exit(1);
  }
  writeToFile(intermediateFile,"Line\tAddress\tLabel\tOPCODE\tOPERAND\tComment");
  errorFile.open("error_"+fileName);
  if(!errorFile){
    cout<<"Unable to open file: error_"<<fileName<<endl;
    exit(1);
  }
  writeToFile(errorFile,"************PASS1************");  

  string fileLine;
  string writeData,writeDataSuffix="",writeDataPrefix="";
  int index=0;

  string currentBlockName = "DEFAULT";
  int currentBlockNumber = 0;
  int totalBlocks = 1;

  bool statusCode;
  string label,opcode,operand,comment;
  string tempOperand;

  int startAddress,LOCCTR,saveLOCCTR,lineNumber,lastDeltaLOCCTR,lineNumberDelta=0;
  lineNumber = 0;
  lastDeltaLOCCTR = 0;

  getline(sourceFile,fileLine);
  lineNumber += 5;
  while(checkCommentLine(fileLine)){
    writeData = to_string(lineNumber) + "\t" + fileLine;
    writeToFile(intermediateFile,writeData);
    getline(sourceFile,fileLine);
    lineNumber += 5;
    index = 0;
  }

  readFirstNonWhiteSpace(fileLine,index,statusCode,label);
  readFirstNonWhiteSpace(fileLine,index,statusCode,opcode);

  if(opcode=="START"){
    readFirstNonWhiteSpace(fileLine,index,statusCode,operand);
    readFirstNonWhiteSpace(fileLine,index,statusCode,comment,true);
    startAddress = stringHexToInt(operand);
    LOCCTR = startAddress;
    writeData = to_string(lineNumber) + "\t" + intToStringHex(LOCCTR-lastDeltaLOCCTR) + "\t" + to_string(currentBlockNumber) + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + comment;
    writeToFile(intermediateFile,writeData); 

    getline(sourceFile,fileLine); 
    lineNumber += 5;
    index = 0;
    readFirstNonWhiteSpace(fileLine,index,statusCode,label); 
    readFirstNonWhiteSpace(fileLine,index,statusCode,opcode);
  }
  else{
    startAddress = 0;
    LOCCTR = 0;
  }
  while(opcode!="END"){
    if(!checkCommentLine(fileLine)){
      if(label!=""){
        if(SYMTAB[label].exists=='n'){
          SYMTAB[label].name = label;
          SYMTAB[label].address = intToStringHex(LOCCTR);
          SYMTAB[label].relative = 1;
          SYMTAB[label].exists = 'y';
          SYMTAB[label].blockNumber = currentBlockNumber;
        }
        else{
          writeData = "Line: "+to_string(lineNumber)+" : Duplicate symbol for '"+label+"'. Previously defined at "+SYMTAB[label].address;
          writeToFile(errorFile,writeData);
          error_flag = true;
        }
      }
      if(OPTAB[getRealOpcode(opcode)].exists=='y'){
        if(OPTAB[getRealOpcode(opcode)].format==3){
          LOCCTR += 3;
          lastDeltaLOCCTR += 3;
          if(getFlagFormat(opcode)=='+'){
            LOCCTR += 1;
            lastDeltaLOCCTR += 1;
          }
          if(getRealOpcode(opcode)=="RSUB"){
            operand = " ";
          }
          else{
            readFirstNonWhiteSpace(fileLine,index,statusCode,operand);
            if(operand[operand.length()-1] == ','){
              readFirstNonWhiteSpace(fileLine,index,statusCode,tempOperand);
              operand += tempOperand;
            }
          }

          if(getFlagFormat(operand)=='='){
            tempOperand = operand.substr(1,operand.length()-1);
            if(tempOperand=="*"){
              tempOperand = "X'" + intToStringHex(LOCCTR-lastDeltaLOCCTR,6) + "'";
            }
            if(LITTAB[tempOperand].exists=='n'){
              LITTAB[tempOperand].value = tempOperand;
              LITTAB[tempOperand].exists = 'y';
              LITTAB[tempOperand].address = "?";
              LITTAB[tempOperand].blockNumber = -1;
            }
          }
        }
        else if(OPTAB[getRealOpcode(opcode)].format==1){
          operand = " ";
          LOCCTR += OPTAB[getRealOpcode(opcode)].format;
          lastDeltaLOCCTR += OPTAB[getRealOpcode(opcode)].format;
        }
        else{
          LOCCTR += OPTAB[getRealOpcode(opcode)].format;
          lastDeltaLOCCTR += OPTAB[getRealOpcode(opcode)].format;
          readFirstNonWhiteSpace(fileLine,index,statusCode,operand);
          if(operand[operand.length()-1] == ','){
            readFirstNonWhiteSpace(fileLine,index,statusCode,tempOperand);
            operand += tempOperand;
          }
        }
      }
      else if(opcode == "WORD"){
        readFirstNonWhiteSpace(fileLine,index,statusCode,operand);
        LOCCTR += 3;
        lastDeltaLOCCTR += 3;
      }
      else if(opcode == "RESW"){
        readFirstNonWhiteSpace(fileLine,index,statusCode,operand);
        LOCCTR += 3*stoi(operand);
        lastDeltaLOCCTR += 3*stoi(operand);
      }
      else if(opcode == "RESB"){
        readFirstNonWhiteSpace(fileLine,index,statusCode,operand);
        LOCCTR += stoi(operand);
        lastDeltaLOCCTR += stoi(operand);
      }
      else if(opcode == "BYTE"){
        readByteOperand(fileLine,index,statusCode,operand);
        if(operand[0]=='X'){
          LOCCTR += (operand.length() -3)/2;
          lastDeltaLOCCTR += (operand.length() -3)/2;
        }
        else if(operand[0]=='C'){
          LOCCTR += operand.length() -3;
          lastDeltaLOCCTR += operand.length() -3;
        }
      }
      else if(opcode=="BASE"){
        readFirstNonWhiteSpace(fileLine,index,statusCode,operand);
      }
      else if(opcode=="LTORG"){
        operand = " ";
        handle_LTORG(writeDataSuffix,lineNumberDelta,lineNumber,LOCCTR,lastDeltaLOCCTR,currentBlockNumber);
      }
      else if(opcode=="ORG"){
        readFirstNonWhiteSpace(fileLine,index,statusCode,operand);

        char lastByte = operand[operand.length()-1];
        while(lastByte=='+'||lastByte=='-'||lastByte=='/'||lastByte=='*'){
          readFirstNonWhiteSpace(fileLine,index,statusCode,tempOperand);
          operand += tempOperand;
          lastByte = operand[operand.length()-1];
        }

        int tempVariable;
        tempVariable = saveLOCCTR;
        saveLOCCTR = LOCCTR;
        LOCCTR = tempVariable;

        if(SYMTAB[operand].exists=='y'){
          LOCCTR = stringHexToInt(SYMTAB[operand].address);
        }
        else{
          bool relative;
          error_flag = false;
          evaluateExpression(operand,relative,tempOperand,lineNumber,errorFile,error_flag);
          if(!error_flag){
            LOCCTR = stringHexToInt(tempOperand);
          }
          error_flag = false;
        }
      }
      else if(opcode=="USE"){
        readFirstNonWhiteSpace(fileLine,index,statusCode,operand);
        BLOCKS[currentBlockName].LOCCTR = intToStringHex(LOCCTR);
        if(operand=="")
        {
          operand = "DEFAULT";
        }
        if(BLOCKS[operand].exists=='n'){
          BLOCKS[operand].exists = 'y';
          BLOCKS[operand].name = operand;
          BLOCKS[operand].number = totalBlocks++;
          BLOCKS[operand].LOCCTR = "0";
        }


        currentBlockNumber = BLOCKS[operand].number;
        currentBlockName = BLOCKS[operand].name;
        LOCCTR = stringHexToInt(BLOCKS[operand].LOCCTR);

      }
      else if(opcode=="EQU"){
        readFirstNonWhiteSpace(fileLine,index,statusCode,operand);
        tempOperand = "";
        bool relative;

        if(operand=="*"){
          tempOperand = intToStringHex(LOCCTR-lastDeltaLOCCTR,6);
          relative = 1;
        }
        else if(if_all_num(operand)){
          tempOperand = intToStringHex(stoi(operand),6);
          relative = 0;
        }
        else{
          char lastByte = operand[operand.length()-1];
        
          while(lastByte=='+'||lastByte=='-'||lastByte=='/'||lastByte=='*'){
            readFirstNonWhiteSpace(fileLine,index,statusCode,tempOperand);
            operand += tempOperand;
            lastByte = operand[operand.length()-1];
          }
         
          evaluateExpression(operand,relative,tempOperand,lineNumber,errorFile,error_flag);
        }

        SYMTAB[label].name = label;
        SYMTAB[label].address = tempOperand;
        SYMTAB[label].relative = relative;
        SYMTAB[label].blockNumber = currentBlockNumber;
        lastDeltaLOCCTR = LOCCTR - stringHexToInt(tempOperand);
      }
      else{
        readFirstNonWhiteSpace(fileLine,index,statusCode,operand);
        writeData = "Line: "+to_string(lineNumber)+" : Invalid OPCODE. Found " + opcode;
        writeToFile(errorFile,writeData);
        error_flag = true;
      }
      readFirstNonWhiteSpace(fileLine,index,statusCode,comment,true);
      if(opcode=="EQU" && SYMTAB[label].relative == 0){
        writeData = writeDataPrefix + to_string(lineNumber) + "\t" + intToStringHex(LOCCTR-lastDeltaLOCCTR) + "\t" + " " + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + comment + writeDataSuffix;
      } 
      else{
        writeData = writeDataPrefix + to_string(lineNumber) + "\t" + intToStringHex(LOCCTR-lastDeltaLOCCTR) + "\t" + to_string(currentBlockNumber) + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + comment + writeDataSuffix;

      }
      writeDataPrefix = "";
      writeDataSuffix = "";
    }
    else{
      writeData = to_string(lineNumber) + "\t" + fileLine;
    }
    writeToFile(intermediateFile,writeData);

    BLOCKS[currentBlockName].LOCCTR = intToStringHex(LOCCTR);
    getline(sourceFile,fileLine); 
    lineNumber += 5 + lineNumberDelta;
    lineNumberDelta = 0;
    index = 0;
    lastDeltaLOCCTR = 0;
    readFirstNonWhiteSpace(fileLine,index,statusCode,label); 
    readFirstNonWhiteSpace(fileLine,index,statusCode,opcode);
}

if(opcode=="END"){
	firstExecutable_Sec=SYMTAB[label].address;
	SYMTAB[firstExecutable_Sec].name=label;
	SYMTAB[firstExecutable_Sec].address=firstExecutable_Sec;
}

  readFirstNonWhiteSpace(fileLine,index,statusCode,operand);
  readFirstNonWhiteSpace(fileLine,index,statusCode,comment,true);

  
  currentBlockName = "DEFAULT";
  currentBlockNumber = 0;
  LOCCTR = stringHexToInt(BLOCKS[currentBlockName].LOCCTR);

  handle_LTORG(writeDataSuffix,lineNumberDelta,lineNumber,LOCCTR,lastDeltaLOCCTR,currentBlockNumber);

  writeData = to_string(lineNumber) + "\t" + intToStringHex(LOCCTR-lastDeltaLOCCTR) + "\t" + " " + "\t" + label + "\t" + opcode + "\t" + operand + "\t" + comment + writeDataSuffix;
  writeToFile(intermediateFile,writeData);

  int LocctrArr[totalBlocks];
  BLocksNumToName = new string[totalBlocks];
  for(auto const& it: BLOCKS){
    LocctrArr[it.second.number] = stringHexToInt(it.second.LOCCTR);
    BLocksNumToName[it.second.number] = it.first;
  }
  for(int i = 1 ;i<totalBlocks;i++){
    LocctrArr[i] += LocctrArr[i-1];
  }
  for(auto const& it: BLOCKS){
    if(it.second.startAddress=="?"){
      BLOCKS[it.first].startAddress= intToStringHex(LocctrArr[it.second.number - 1]);
    }
  }
  program_length = LocctrArr[totalBlocks - 1] - startAddress;
  sourceFile.close();
  intermediateFile.close();
  errorFile.close();
}
