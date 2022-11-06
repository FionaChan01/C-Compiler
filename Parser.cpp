#include <vector>
#include "intermediateCode.h"
#include "Error.h"
#include<string>
#include "SymbolTable.h"
#include "algorithm"
#include <fstream>
#include <sstream>
#include "Parser.h"
#include <fstream>
#include<map>
#include <iostream>

using namespace std;
int idTemp = 0;
map<string, map<string, SymbolUnit>> tempT;
int constChar = 0;
int iword = 0;
string wordType;
int labelId = 0;
vector<intermediateCode> IntermediateCode;
int linenumber = 0;
vector<string> voidList;
vector<string> funcList;
int tempAddr = 0;
int globalAddr = 0;
vector<string> strList;
vector<string> ReturnList;
map <string, SymbolUnit> tempSymTable;
bool Main = false;
int allSwitchId = 0;
int switchId = 0;
map <string, SymbolUnit> globalSymTable;
int returnTime;
string word;


string genSwitchLabel() {
    switchId++;
    return "label_switch_" + int2string(switchId);
}

void initTempTable(string name) {
    tempT.insert(make_pair(name,tempSymTable));
    funcList.push_back(name);
    tempSymTable.clear();
    tempAddr = 0;
} 
int string2int(string in) {
    stringstream s;
    s << in;
    int t;
    s >> t;
    return t;
}
string int2string(int in) {
    stringstream s;
    s << in;
    return s.str();
}

string genLabel(string name) {
    labelId++;
    return "label_" + int2string(labelId) + "_" + name;

}
string genId() {
    idTemp++;
    return "@Temp" + int2string(idTemp);
}


void Parser::insertIDENFR(string name, int inKind, int inType, int inIntValue, char inCharValue, int index,int addr,int length) {
    if (index == 0) {
        if (globalSymTable.find(name) != globalSymTable.end()) {
            Error('b', linenumber);
        }
        else {
            globalSymTable.insert(make_pair(name, SymbolUnit(name, inKind, inType,globalAddr, inIntValue, inCharValue,length)));
            globalAddr+=addr;
        }
    }
    else if (index == 1) {
        if (tempSymTable.find(name) != tempSymTable.end()) {
            Error('b', linenumber);
        }
        else {
            tempSymTable.insert(make_pair(name, SymbolUnit(name, inKind, inType,tempAddr, inIntValue, inCharValue,length)));
            tempAddr+=addr;
        }
    }
}


void Parser::isChangeLineWhenKLM(char type) {
    prev();
    Error(type, lines[iword]);
}

vector<string> specialToken = {"INTCON","PLUS","MINU"};

void Parser::VariableDefinitionWithInitOError(int tempType) {

    if (codeType[iword+1] == "CHARCON") {
        if (tempType == 2) {
            Error('o', linenumber);
        }
    }
    else if (find(specialToken.begin(),specialToken.end(),codeType[iword+1]) != specialToken.end()) {
        if (tempType == 3) {
            Error('o', linenumber);
        }
    }
}


SymbolUnit Parser::isIDENFR(string in) {
    SymbolUnit returnValue;
    if (tempSymTable.find(in) != tempSymTable.end()) {
        returnValue = tempSymTable[in];
    }
    else {
        if (globalSymTable.find(in) != globalSymTable.end()) {
            returnValue = globalSymTable[in];
        }
        else {
            Error('c', linenumber);
        }
    }
    return returnValue;
}




string Parser::toLowerCase(string in) {
    for (int i = 0; i < in.size(); i++) {
        if (in[i] <= 'Z' && in[i] >= 'A') {
            in[i] = in[i] - 'A' + 'a';
        }
    }
    return in;
}



void Parser::RecoverMode(int in) {

    int frequency = iword - in;
    iword = in;
    word = code[iword];
    linenumber = lines[iword];

    for (int i = 0; i < frequency; i++) {
        string temp = res.at(res.size() - 1);
        if (temp[0] == '<' && temp[temp.size() - 1] == '>') {
            i--;
        }
        res.pop_back();
    }
}

void Parser::next() {
    iword++;
    word = code[iword];
    wordType = codeType[iword];
    if (wordType == "CHARCON") {
        constChar = int(word[0]);
    }
    res.push_back(wordType + " " + word);
    linenumber = lines[iword];
}

void Parser::prev() {
    if (res.size()==0) {
        return;
    }
    else {
        iword--;
        word = code[iword];
        linenumber = lines[iword];
        wordType = codeType[iword];
        res.pop_back();
    }
}

vector<string> Parser::getAns() {
    return res;
}

void Parser::Output(string in) {
    res.push_back(in);
}


string Parser::String() {
    next();
    if (wordType != "STRCON") {
        prev();
        return int2string(FAIL);
    }
    else {
        Output("<字符串>");
        return word;

    }
}


int Parser::Step(int init) {
    int length = NunInteger();
    if (length == FAIL) {
        RecoverMode(init);
        return FAIL;
    }
    else {
        Output("<步长>");
        return length;

    }
}



int Parser::LoopStatement(int init) {
    next();
    if (wordType == "WHILETK") {
        next();
        string labBegin, labEnd;
        labBegin = genLabel("begin");
        IntermediateCode.emplace_back(intermediateCode(LABEL,"","",labBegin));
        if (wordType == "LPARENT") {
            string result;
            if (Condition(iword,result) != FAIL) {
                next();
                if (wordType != "RPARENT") {
                    isChangeLineWhenKLM('l');
                }
                labEnd = genLabel("end");
                IntermediateCode.emplace_back(intermediateCode(BZ,result , "", labEnd));
                if (Statement(0) != FAIL) {
                    Output("<循环语句>");
                    IntermediateCode.emplace_back(intermediateCode(GOTO, "", "", labBegin));
                    IntermediateCode.emplace_back(intermediateCode(LABEL, "", "", labEnd));
                    return 1;
                }
            }
        }
    }

    else if (wordType == "FORTK") {
        next();
        if (wordType == "LPARENT") {
            next();
            if (wordType == "IDENFR") {
                string name = toLowerCase(word);
                SymbolUnit id1 = isIDENFR(name);
                next();
                if (wordType == "ASSIGN") {
                    string value;
                    if (Expression(iword, 0,value) != FAIL) {
                        next();
                        IntermediateCode.emplace_back(intermediateCode(ASSIGN, value, "", name));
                        if (wordType != "SEMICN") {
                            isChangeLineWhenKLM('k');
                        }
                        string result, labelEnd, labelBegin;
                        labelEnd = genLabel("End");
                        labelBegin = genLabel("Begin");
                        IntermediateCode.emplace_back(intermediateCode(LABEL, "", "", labelBegin));
                        if (Condition(iword,result) != FAIL) {
                            next();
                            if (wordType != "SEMICN") {
                                isChangeLineWhenKLM('k');
                            }
                            IntermediateCode.emplace_back(intermediateCode(BZ,result,"",labelEnd));
                            next();
                            string namel, namer;
                            int stepLength;
                            operation op;
                            if (wordType == "IDENFR") {
                                namel = toLowerCase(word);
                                SymbolUnit id2 = isIDENFR(namel);
                                next();
                                if (wordType == "ASSIGN") {
                                    next();
                                    if (wordType == "IDENFR") {
                                        namer = toLowerCase(word);
                                        SymbolUnit id3 = isIDENFR(namer);
                                        next();
                                        op = wordType == "PLUS" ? PLUS : MINUS;
                                        if (wordType == "PLUS" || wordType == "MINU") {
                                            stepLength = Step(iword);
                                            if (stepLength!= FAIL) {
                                                next();
                                                if (wordType != "RPARENT") {
                                                    isChangeLineWhenKLM('l');
                                                }
                                                if (Statement(0) != FAIL) {
                                                    IntermediateCode.emplace_back(intermediateCode(op, namer, int2string(stepLength), namel));
                                                    IntermediateCode.emplace_back(intermediateCode(GOTO,  "", "", labelBegin));
                                                    IntermediateCode.emplace_back(intermediateCode(LABEL, "", "", labelEnd));
                                                    Output("<循环语句>");
                                                    return 1;
                                                }

                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else {
        RecoverMode(init);
        return FAIL;
    }
}


int Parser::ParameterTable(int init, string func) {
    next();
    SymbolUnit* funcSym;
    int type = 0;
    if (wordType == "CHARTK" || wordType == "INTTK") {
        funcSym = &globalSymTable[func];
        type = (wordType == "CHARTK") ? 3 : 2;
        (*funcSym).insert(type);
        next();
        if (wordType == "IDENFR") {
            word = toLowerCase(word);
            if (tempSymTable.find(word) != tempSymTable.end()) {
                Error('b', linenumber);
            }
            else {
                IntermediateCode.emplace_back(intermediateCode(PARA, type == 3 ? "char" : "int", "", word));
                tempSymTable.insert(make_pair(word, SymbolUnit(word, 2, type,tempAddr)));
                tempAddr++;
            }
            next();
            while (word == ",") {
                next();
                if (wordType == "CHARTK" || wordType == "INTTK") {
                    type = (wordType == "CHARTK") ? 3 : 2;
                    (*funcSym).insert(type);
                    next();
                    if (wordType == "IDENFR") {
                        word = toLowerCase(word);
                        if (tempSymTable.find(word) != tempSymTable.end()) {
                            Error('b', linenumber);
                        }
                        else {
                            IntermediateCode.emplace_back(intermediateCode(PARA, type == 3 ? "char" : "int", "", word));
                            tempSymTable.insert(make_pair(word, SymbolUnit(word, 2, type,tempAddr)));
                            tempAddr++;
                        }
                        next();
                    }
                }
            }
            prev();
            Output("<参数表>");
            return 1;
        }
        else {
            return FAIL;
        }
    }
    else if (wordType == "RPARENT") {
        RecoverMode(init);
        Output("<参数表>");
        return 1;
    }
    else if (wordType == "LBRACE") {
        RecoverMode(init);
        Output("<参数表>");
        return 1;
    }
    else {
        return FAIL;
    }
}

int Parser::Integer() {
    int number = NunInteger();
    if (number != FAIL) {
        Output("<整数>");
        return number;
    }
    else {
        next();
        if (wordType == "PLUS") {
            int number = NunInteger();
            if (number != FAIL) {
                Output("<整数>");
                return number;
            }
        }
        else if (wordType == "MINU") {
            int number = NunInteger();
            if (number != FAIL) {
                Output("<整数>");
                return -1 * number;
            }
        }
        else {
            prev();
            return FAIL;
        }
    }
}



int Parser::CaseTable(int init, int type) {
    int frequency = 0;
    while (CaseSubStatement(iword, type) != FAIL) {
        frequency++;
    }
    if (frequency >= 1) {
        Output("<情况表>");
        return 1;
    }
    else {
        RecoverMode(init);
        return FAIL;
    }
}


int Parser::CaseSubStatement(int init, int type) {
    next();
    int caseType = 0;
    if (wordType == "CASETK") {
        next();
        if (wordType == "CHARCON") {
            caseType = 3;
            prev();
        }
        else {
            caseType = 2;
            prev();
        }
        if (caseType != type) {
            Error('o', linenumber);
        }
        int value = Const();
        if (value != FAIL) {
            next();
            if (word == ":") {
                string label = genSwitchLabel();
                IntermediateCode.emplace_back(intermediateCode(LABEL, "", "", label));
                IntermediateCode.emplace_back(intermediateCode(CASE, int2string(value), "", label));
                if (Statement(0) != FAIL) {
                    IntermediateCode.emplace_back(intermediateCode(GOTO, "", "", "SWITCH_END_" + int2string(allSwitchId)));
                    Output("<情况子语句>");
                    return 1;
                }
            }
        }
    }
    else {
        RecoverMode(init);
        return FAIL;
    }
}


int Parser::FunctionWithReturnValueDefinition(int init) {
    next();
    string wordName,funType;
    if (wordType == "CHARTK" || wordType == "INTTK") {
        funType = wordType;
        next();
        if (wordType == "IDENFR") {
            wordName = toLowerCase(word);
        }
        RecoverMode(init);
        int funcType = Announcement();
        if (funcType != FAIL) {
            next();
            SymbolUnit funcItem;
            IntermediateCode.emplace_back(intermediateCode(FUNC, funType == "CHARTK" ? "char" : "int", "", wordName));
            if (globalSymTable.find(wordName) != globalSymTable.end()) {
                Error('b', linenumber);
            }
            else {
                funcItem = SymbolUnit(wordName, 3, funcType, globalAddr);
                globalSymTable.insert(make_pair(wordName, funcItem));
                globalAddr++;
            }
            if (wordType == "LPARENT") {
                if (ParameterTable(iword, wordName) != FAIL) {
                    next();
                    if (wordType != "RPARENT") {
                        isChangeLineWhenKLM('l');
                    }
                    next();
                    if (wordType == "LBRACE") {
                        returnTime = 0;
                        if (MultiStatement(iword, funcType) != FAIL) {
                            next();
                            if (wordType == "RBRACE") {
                                globalSymTable[wordName].length = tempAddr;
                                initTempTable(wordName);
                                Output("<有返回值函数定义>");
                                return 1;
                            }
                        }
                    }
                }
            }
            else {
                RecoverMode(init);
                return FAIL;
            }
        }
    }
    else {
        RecoverMode(init);
        return FAIL;
    }
}


int Parser::CaseStatement(int init) {
    next();
    if (wordType == "SWITCHTK") {
        next();
        if (wordType == "LPARENT") {
            string value;
            int switchType = Expression(iword, 0,value);
            if (switchType != FAIL) {
                next();
                if (wordType != "RPARENT") {
                    isChangeLineWhenKLM('l');
                }
                next();
                if (wordType == "LBRACE") {
                    IntermediateCode.emplace_back(intermediateCode(GOTO, "", "", "SWITCH_BEGIN_" + int2string(allSwitchId)));
                    if (CaseTable(iword, switchType) != FAIL) {
                        IntermediateCode.emplace_back(intermediateCode(LABEL, "", "", "SWITCH_BEGIN_" + int2string(allSwitchId)));
                        IntermediateCode.emplace_back(intermediateCode(SWITCH, "", "", value));
                        if (Default(iword) != FAIL) {
                            next();
                            if (wordType == "RBRACE") {
                                IntermediateCode.emplace_back(intermediateCode(LABEL, "", "", "SWITCH_END_" + int2string(allSwitchId)));
                                allSwitchId++;
                                Output("<情况语句>");
                                return 1;
                            }
                        }
                        else {
                            next();
                            Error('p', linenumber);
                            while (wordType != "RBRACE") {
                                next();
                            }
                            return 1;
                        }
                    }
                }
            }
        }
    }
    else {
        RecoverMode(init);
        return FAIL;
    }
}



void Parser::ConstDefinition(int index) {
    next();
    if (wordType == "INTTK") {
        next();
        if (wordType == "IDENFR") {
            string name = toLowerCase(word);
            next();
            if (wordType == "ASSIGN") {
                int number = Integer();
                if (number != FAIL) {

                    next();
                    insertIDENFR(name, 1, 2, number, ' ',index,1,0);
                    while (word == ",") {
                        next();
                        if (wordType == "IDENFR") {
                            name = toLowerCase(word);
                            next();
                            if (wordType == "ASSIGN") {
                                number = Integer();
                                if (number != FAIL) {

                                    insertIDENFR(name, 1, 2, number,' ', index,1,0);

                                    next();
                                }
                            }
                        }
                    }
                    prev();
                    Output("<常量定义>");
                }
            }
        }
    }
    else if (wordType == "CHARTK") {
        next();
        if (wordType == "IDENFR") {
            string name = toLowerCase(word);
            next();
            if (wordType == "ASSIGN") {
                next();
                if (wordType == "CHARCON") {
                    char tempChar = word[0];
                    string charValue = word;
                    insertIDENFR(name, 1, 3, FAIL,tempChar, index,1,0);

                    next();

                    while (word == ",") {
                        next();
                        if (wordType == "IDENFR") {
                            name = toLowerCase(word);
                            next();
                            if (wordType == "ASSIGN") {
                                next();
                                if (wordType == "CHARCON") {
                                    tempChar = word[0];
                                    string charValue = word;

                                    insertIDENFR(name, 1, 3, FAIL, tempChar, index,1,0);
                                    next();
                                }
                            }
                        }
                    }
                    prev();
                    Output("<常量定义>");
                }
            }
        }
    }
}



int Parser::ConstState(int index) {
    next();
    if (wordType == "CONSTTK") {
        ConstDefinition(index);
        next();
        if (wordType != "SEMICN") {
            isChangeLineWhenKLM('k');
        }
        next();
        while (wordType == "CONSTTK") {
            ConstDefinition(index);
            next();
            if (wordType != "SEMICN") {
                isChangeLineWhenKLM('k');
            }
            next();
        }
        prev();
        Output("<常量说明>");
        return 1;
    }
    else {
        prev();
        return FAIL;
    }
}


int Parser::MultiStatement(int init, int funcType) {
    ConstState(1);
    VariableState(1);
    if (StatementList(iword, funcType) != FAIL) {
        Output("<复合语句>");
        return 1;
    }
    else {
        RecoverMode(init);
        return FAIL;
    }
}


int Parser::NunInteger() {
    next();
    if (wordType == "INTCON") {
        Output("<无符号整数>");
        return stoi(word);
    }
    else {
        prev();
        return FAIL;
    }
}



int Parser::Const() {
    int tempNumber = Integer();
    if (tempNumber != FAIL) {
        Output("<常量>");
        return tempNumber;
    }
    else {
        next();
        if (wordType == "CHARCON") {
            Output("<常量>");
            return int(word[0]);
        }
        else {
            prev();
            return FAIL;
        }
    }
}

/*＜变量定义及初始化＞ ::= ＜类型标识符＞＜标识符＞=＜常量＞|＜类型标识符＞＜标识符＞'['＜无符号整数＞']'
='{'＜常量＞{,＜常量＞}'}'|＜类型标识符＞＜标识符＞'
['＜无符号整数＞']''['＜无符号整数＞']'='{''{'＜常量＞{,＜常量＞}'}'{, '{'＜常量＞{,＜常量＞}'}'}'}'*/
int Parser::VariableDefinitionWithInit(int init, int index) {
    next();
    if (wordType == "INTTK" || wordType == "CHARTK") {
        int tempType = (wordType == "INTTK") ? 2 : 3;
        next();
        int time = 0;
        if (wordType == "IDENFR") {
            string name = toLowerCase(word);
            int length1 = FAIL, length2 = FAIL;
            int addr = 1;
            next();
            while (wordType == "LBRACK") {
                time++;
                int tempLength = NunInteger();
                if (tempLength != FAIL) {
                    next();
                    addr += addr * tempLength;
                    if (wordType != "RBRACK") {
                        isChangeLineWhenKLM('m');
                    }
                    if (time == 1) {
                        length1 = tempLength;
                    }
                    if (time == 2) {
                        length2 = tempLength;
                    }
                    next();
                }
                else {
                    Error('i', linenumber);
                    while (wordType != "RBRACK") {
                        next();
                    }
                    next();
                }
            }
            if (wordType == "ASSIGN") {
                VariableDefinitionWithInitOError(tempType);
                int value = Const();
                if (value != FAIL) {
                    insertIDENFR(name, 2, tempType, value, char(value), index,addr,0);
                    IntermediateCode.emplace_back(intermediateCode(ASSIGN, int2string(value), "", name));
                    Output("<变量定义及初始化>");
                    return 1;
                }
                else {
                    if (time == 1) {
                        next();
                        if (wordType == "LBRACE") {
                            VariableDefinitionWithInitOError(tempType);
                            int tempValue = Const();
                            int valueTimes = 0;
                            if (tempValue != FAIL) {
                                IntermediateCode.emplace_back(intermediateCode(PUTARRAY, int2string(valueTimes), int2string(tempValue), name));
                                valueTimes++;
                                next();
                                while (wordType == "COMMA") {
                                    VariableDefinitionWithInitOError(tempType);
                                    tempValue = Const();
                                    if (tempValue != FAIL) {
                                        IntermediateCode.emplace_back(intermediateCode(PUTARRAY, int2string(valueTimes), int2string(tempValue), name));
                                        valueTimes++;
                                        next();
                                    }
                                }
                                if (wordType == "RBRACE") {
                                    if (valueTimes != length1) {
                                        Error('n', linenumber);
                                    }
                                    insertIDENFR(name, 4, tempType, FAIL, ' ', index,addr,0);
                                    Output("<变量定义及初始化>");
                                    return 1;
                                }
                            }
                            else if (wordType == "LBRACE") {
                                Error('n', linenumber);
                                while (wordType != "SEMICN") {
                                    next();
                                }
                                prev();
                                Output("<变量定义及初始化>");
                                return 1;
                            }
                        }
                        else {
                            Error('n', linenumber);
                            Output("<变量定义及初始化>");
                            return 1;
                        }
                    }
                    else if (time == 2) {
                        next();
                        int valueTimes1 = 0, valueTimes2 = 0;
                        if (wordType == "LBRACE") {
                            next();
                            if (wordType == "LBRACE") {
                                valueTimes1++;
                                VariableDefinitionWithInitOError(tempType);
                                int tempValue = Const();
                                if (tempValue != FAIL) {
                                    IntermediateCode.emplace_back(intermediateCode(PUTARRAY, int2string(valueTimes1 - 1) + " " + int2string(valueTimes2), int2string(tempValue), name));
                                    valueTimes2++;
                                    next();
                                    while (wordType == "COMMA") {
                                        VariableDefinitionWithInitOError(tempType);
                                        tempValue = Const();
                                        if (tempValue != FAIL) {
                                            IntermediateCode.emplace_back(intermediateCode(PUTARRAY, int2string(valueTimes1 - 1) + " " + int2string(valueTimes2), int2string(tempValue), name));
                                            valueTimes2++;
                                            next();
                                        }
                                    }
                                    if (valueTimes2 != length2) {
                                        Error('n', linenumber);
                                    }
                                    if (wordType == "RBRACE") {
                                        next();
                                        while (wordType == "COMMA") {
                                            valueTimes1++;
                                            next();
                                            valueTimes2 = 0;
                                            if (wordType == "LBRACE") {
                                                VariableDefinitionWithInitOError(tempType);
                                                tempValue = Const();
                                                if (tempValue != FAIL) {
                                                    IntermediateCode.emplace_back(intermediateCode(PUTARRAY, int2string(valueTimes1 - 1) + " " + int2string(valueTimes2), int2string(tempValue), name));
                                                    valueTimes2++;
                                                    next();
                                                    while (wordType == "COMMA") {
                                                        VariableDefinitionWithInitOError(tempType);
                                                        tempValue = Const();
                                                        if (tempValue != FAIL) {
                                                            IntermediateCode.emplace_back(intermediateCode(PUTARRAY, int2string(valueTimes1 - 1) + " " + int2string(valueTimes2), int2string(tempValue), name));
                                                            valueTimes2++;
                                                            next();
                                                        }
                                                    }
                                                    if (wordType == "RBRACE") {
                                                        if (valueTimes2 != length2) {
                                                            Error('n', linenumber);
                                                        }
                                                        next();
                                                    }
                                                }
                                            }
                                        }
                                        if (wordType == "RBRACE") {
                                            if (valueTimes1 != length1) {
                                                Error('n', linenumber);
                                            }
                                            insertIDENFR(name, 5, tempType, FAIL, ' ', index,addr,length2);
                                            Output("<变量定义及初始化>");
                                            return 1;
                                        }
                                    }
                                }
                                else if (wordType == "LBRACE") {
                                    Error('n', linenumber);
                                    while (wordType != "SEMICN") {
                                        next();
                                    }
                                    prev();
                                    Output("<变量定义及初始化>");
                                    return 1;
                                }
                            }
                            else {
                                Error('n', linenumber);
                                next();
                                Output("<变量定义及初始化>");
                                return 1;
                            }
                        }
                        else {
                            Error('n', linenumber);
                            return FAIL;
                            Output("<变量定义及初始化>");
                            return 1;
                        }
                    }
                }
            }
            else {
                RecoverMode(init);
                return FAIL;
            }
        }
        else {
            RecoverMode(init);
            return FAIL;
        }
    }
    else {
        RecoverMode(init);
        return FAIL;
    }
}

int Parser::MultiOperation(int init) {
    next();
    if (wordType == "MULT") {
        return 1;
    }
    if (wordType == "DIV") {
        return 2;
    }
    else {
        RecoverMode(init);
        return FAIL;
    }
}


int Parser::VariableDefinitionWithoutInit(int init, int index) {
    next();
    int type = 0;
    if (wordType == "CHARTK" || wordType == "INTTK") {
        type = (wordType == "CHARTK") ? 3 : 2;
        next();
        if (wordType == "IDENFR") {
            string name = toLowerCase(word);
            int addr = 1;
            int length = 0;
            int time = 0;
            next();
            while (wordType == "LBRACK") {
                int arrayIndex = NunInteger();
                if (arrayIndex != FAIL) {
                    time++;
                    if (time == 2) {
                        length = arrayIndex;
                    }
                    addr += addr * arrayIndex;
                    next();
                    if (wordType != "RBRACK") {
                        isChangeLineWhenKLM('m');
                    }
                    next();
                }
                else {
                    Error('i', linenumber);
                    while (wordType != "RBRACK") {
                        next();
                    }
                    next();
                }
            }
            while (wordType == "COMMA") {
                next();
                if (wordType == "IDENFR") {
                    addr = 1;
                    int tempLength = 0;
                    int tempTime = 0;
                    string tempName = toLowerCase(word);
                    next();
                    while (wordType == "LBRACK") {
                        int arrayIndex = NunInteger();
                        if (arrayIndex != FAIL) {
                            tempTime++;
                            if (tempTime == 2) {
                                tempLength = arrayIndex;
                            }
                            addr += addr * arrayIndex;
                            next();
                            if (wordType != "RBRACK") {
                                isChangeLineWhenKLM('m');
                            }
                            next();
                        }
                        else {
                            Error('i', linenumber);
                            while (wordType != "RBRACK") {
                                next();
                            }
                            next();
                        }
                    }
                    if(tempTime == 1)insertIDENFR(tempName, 4, type, FAIL, ' ', index, addr, tempLength);
                    else if (tempTime == 2)insertIDENFR(tempName, 5, type, FAIL, ' ', index, addr, tempLength);
                    else if (tempTime == 0)insertIDENFR(tempName, 2, type, FAIL, ' ', index, addr, tempLength);
                }
            }
            if (wordType == "SEMICN") {
                if (time == 1)insertIDENFR(name, 4, type, FAIL, ' ', index, addr, length);
                else if (time == 2)insertIDENFR(name, 5, type, FAIL, ' ', index, addr, length);
                else if (time == 0)insertIDENFR(name, 2, type, FAIL, ' ', index, addr, length);
                prev();
                Output("<变量定义无初始化>");
                return 1;
            }
            else {
                string nextWord = wordType;
                if (nextWord == "ASSIGN" || nextWord == "LPARENT" ||
                    nextWord == "LBRACK") {
                    RecoverMode(init);
                    return FAIL;
                }
                else {
                    Output("<变量定义无初始化>");
                    prev();
                    return 1;
                }
            }
        }
        else {
            RecoverMode(init);
            return FAIL;
        }
    }
    else {
        RecoverMode(init);
        return FAIL;
    }
}

vector<string> RS = {"EQL","LEQ","NEQ","ASSIGN","GEQ","LSS","GRE"};

int Parser::RelationOperation() {
    next();
    if (std::find(RS.begin(), RS.end(), wordType)!=RS.end()) {
        return 1;
    }
    else {
        prev();
        return FAIL;
    }
}


int Parser::Term(int init, int isPara,string &value) {
    int time = 0;
    string value1, value2;
    string type1, type2;
    int termType = Factor(iword, isPara,value1);
    if (termType != FAIL) {
        int multi = MultiOperation(iword);
        while (multi != FAIL) {
            if ( Factor(iword, isPara, value2) != FAIL) {
                time++;
               if ((value1[0] <= '9' && value1[0] >= '0' || value1[0] == '+' || value1[0] == '-')
                    && (value2[0] <= '9' && value2[0] >= '0' || value2[0] == '+' || value2[0] == '-'))
                {
                    int v1 = string2int(value1);
                    int v2 = string2int(value2);
                    if (multi == 1) {
                        value1 = int2string(v1 * v2);
                    }
                    else if (multi == 2) {
                        value1 = int2string(v1 / v2);
                    }
                }
                else {
                    string idTemp = genId();
                    tempSymTable.insert(make_pair(idTemp, SymbolUnit(idTemp, 2, 2, tempAddr)));
                    tempAddr++;
                    if (multi == 1) IntermediateCode.emplace_back(intermediateCode(MULT, value1, value2, idTemp));
                    else  IntermediateCode.emplace_back(intermediateCode(DIV, value1, value2, idTemp));
                    value1 = idTemp;
               }
                multi = MultiOperation(iword);
            }
            else {
                break;
            }
        }
        if (time >= 1) {
            termType = 2;
        }
        value = value1;
        Output("<项>");
        return termType;
    }
    else {
        RecoverMode(init);
        return FAIL;
    }
}


int Parser::VoidFunctionDefinition(int init) {
    next();
    string wordName;
    if (wordType == "VOIDTK") {
        next();
        if (wordType == "IDENFR") {
            SymbolUnit funcItem;
            wordName = toLowerCase(word);
            if (globalSymTable.find(wordName) != globalSymTable.end()) {
                Error('b', linenumber);
            }
            else {
                funcItem = SymbolUnit(wordName, 3, 1, globalAddr);
                globalSymTable.insert(make_pair(wordName, funcItem));
                globalAddr++;
            }
            IntermediateCode.emplace_back(intermediateCode(FUNC, "void", "", wordName));
            next();
            if (wordType == "LPARENT") {
                if (ParameterTable(iword, wordName) != FAIL) {
                    next();
                    if (wordType != "RPARENT") {
                        isChangeLineWhenKLM('l');
                    }
                    voidList.push_back(wordName);
                    next();
                    if (wordType == "LBRACE") {
                        returnTime = 0;
                        if (MultiStatement(iword, 1) != FAIL) {
                            next();
                            if (wordType == "RBRACE") {
                                globalSymTable[wordName].length = tempAddr;
                                initTempTable(wordName);
                                Output("<无返回值函数定义>");
                                return 1;
                            }
                        }
                    }
                }
            }
        }
        else {
            RecoverMode(init);
            return FAIL;
        }
    }
    else {
        RecoverMode(init);
        return FAIL;
    }
}




int Parser::AddOperation(int init) {
    next();
    if (wordType == "PLUS" ){
        return 1;
    }
    if (wordType == "MINU") {
        return 2;
    }
    else {
        RecoverMode(init);
        return FAIL;
    }
}


int Parser::Default(int init) {
    next();
    if (wordType == "DEFAULTTK") {
        next();
        if (wordType == "COLON") {
            if (Statement(0) != FAIL) {
                Output("<缺省>");
                return 1;
            }
        }
    }
    else {
        RecoverMode(init);
        return FAIL;
    }
}


int Parser::ReadStatement(int init) {
    next();
    SymbolUnit readSym;
    if (wordType == "SCANFTK") {
        next();
        if (wordType == "LPARENT") {
            next();
            if (wordType == "IDENFR") {
                string name = toLowerCase(word);
                if (tempSymTable.find(name) != tempSymTable.end()) {
                    readSym = tempSymTable[name];
                    if (readSym.kind == 1) {
                        Error('j', linenumber);
                    }
                }
                else {
                    if (globalSymTable.find(name) != globalSymTable.end()) {
                        readSym = globalSymTable[name];
                        if (readSym.kind == 1) {
                            Error('j', linenumber);
                        }
                    }
                    else {
                        Error('c', linenumber);
                    }
                }
                next();
                if (wordType != "RPARENT") {
                    isChangeLineWhenKLM('l');
                }
                IntermediateCode.emplace_back(intermediateCode(SCANF, "", "", name));
                Output("<读语句>");
                return 1;
            }
        }
    }
    else {
        RecoverMode(init);
        return FAIL;
    }
}


int Parser::StatementList(int init, int funcType) {
    int frequency;
    for (frequency = 0;Statement(funcType) != FAIL;)
        frequency++;

    if (frequency < 0) {
        RecoverMode(init);
        return FAIL;
    }
    else {
        Output("<语句列>");
        return 1;
    }
}


int Parser::WriteStatement(int init) {
    next();
    if (wordType == "PRINTFTK") {
        next();
        if (wordType == "LPARENT") {
            string value = String();
            if (value != int2string(FAIL)) {
                IntermediateCode.emplace_back(intermediateCode(PRINTF, "4", " ", value));
                strList.push_back(value);
                next();
                if (word == ",") {
                    string value;
                    int expType = Expression(iword, 0, value);
                    if (expType != FAIL) {
                        next();
                        if (wordType != "RPARENT") {
                            isChangeLineWhenKLM('l');;
                        }
                        IntermediateCode.emplace_back(intermediateCode(PRINTF, int2string(expType), " ", value));
                        IntermediateCode.emplace_back(intermediateCode(PRINTF, "5", " ", "NextLine"));
                        Output("<写语句>");
                        return 1;

                    }
                }
                else if (wordType == "RPARENT") {
                    IntermediateCode.emplace_back(intermediateCode(PRINTF,  "5"," ", "NextLine"));
                    Output("<写语句>");
                    return 1;
                }

                else {
                    isChangeLineWhenKLM('l');
                    Output("<写语句>");
                    return 1;
                }
            }
            else {
                string expValue;
                int expType = Expression(iword, 0, expValue);
                if (expType != FAIL) {
                    next();
                    if (wordType != "RPARENT") {
                        isChangeLineWhenKLM('l');
                    }
                    IntermediateCode.emplace_back(intermediateCode(PRINTF, int2string(expType), " ", expValue ));
                    IntermediateCode.emplace_back(intermediateCode(PRINTF, "5"," ", "NextLine" ));
                    Output("<写语句>");
                    return 1;
                }
            }
        }
    }
    else {
        RecoverMode(init);
        return FAIL;
    }
}

int Parser::ReturnStatement(int init) {
    next();

    if (wordType == "IDENFR") {
        string funcName = toLowerCase(word);
        SymbolUnit funcSym;
        next();
        if (wordType == "LPARENT") {
            if (globalSymTable.find(funcName) == globalSymTable.end()) {
                Error('c', linenumber);
            }
            else {
                funcSym = globalSymTable[funcName];
            }
            if (funcSym.kind == 0) {
                while (wordType != "SEMICN") {
                    next();
                }
                return FAIL;
            }
        }
        prev();
        if (funcSym.kind == 3 && funcSym.type != 1) {
            if (std::find(ReturnList.begin(), ReturnList.end(),funcName )!=ReturnList.end()) {
                next();
                if (wordType == "LPARENT") {
                    if (ValueList(iword, funcName) != FAIL) {
                        next();
                        if (wordType != "RPARENT") {
                            isChangeLineWhenKLM('l');
                        }
                        int returnValue = funcSym.type;
                        IntermediateCode.emplace_back(intermediateCode(CALL, "", "", funcName));
                        Output("<有返回值函数调用语句>");
                        return returnValue;

                    }
                }
            }
        }
        else {
            RecoverMode(init);
            return FAIL;
        }
    }
    else {
        RecoverMode(init);
        return FAIL;
    }
}



int Parser::ValueList(int init, string funcName) {
    int time = 0;
    SymbolUnit funcSym = globalSymTable[funcName];
    vector<int> typeList;
    vector<int> tempList = funcSym.paraList;
    string value;
    int length = funcSym.paraList.size();
    int type = Expression(iword, 0,value);
    if (type != FAIL) {
        IntermediateCode.emplace_back(intermediateCode(USE, "", "", value));
        typeList.push_back(type);
        time++;
        next();
        while (word == ",") {
            type = Expression(iword, 0,value);
            if (type != FAIL) {
                IntermediateCode.emplace_back(intermediateCode(USE, "", "", value));
                typeList.push_back(type);
                time++;
                next();
            }
        }
        if (time != length) {
            Error('d', linenumber);
        }
        else {
            for (int i = 0; i < time; i++) {
                if (typeList[i] != tempList[i]) {
                    Error('e', linenumber);
                    break;
                }
            }
        }
        prev();
        Output("<值参数表>");
        return 1;
    }
    else {
        next();
        if (wordType == "RPARENT") {
            if (tempList.size() != 0) {
                Error('d', linenumber);
            }
            prev();
            Output("<值参数表>");
            return 1;
        }
        else {
            RecoverMode(init);
            return 1;
        }
    }
}




int Parser::VoidStatement(int init) {
    next();
    if (wordType == "IDENFR") {
        string funcName = toLowerCase(word);
        next();
        SymbolUnit voidSym;
        if (wordType == "LPARENT") {
            voidSym = isIDENFR(funcName);
            if (voidSym.kind == 0) {
                while (wordType != "SEMICN") {
                    next();
                }
                return FAIL;
            }
        }
        prev();
        if (voidSym.type == 1 && voidSym.kind == 3) {
            if (std::find(voidList.begin(), voidList.end(), funcName)!=voidList.end()) {
                next();
                if (wordType == "LPARENT") {
                    if (ValueList(iword, funcName) != FAIL) {
                        next();
                        if (wordType != "RPARENT") {
                            isChangeLineWhenKLM('l');
                        }
                        IntermediateCode.emplace_back(intermediateCode(CALL, "", "", funcName));
                        Output("<无返回值函数调用语句>");
                        return 1;
                    }
                }
                else {
                    RecoverMode(init);
                    return FAIL;
                }
            }
        }
        else {
            RecoverMode(init);
            return FAIL;
        }
    }
    else {
        RecoverMode(init);
        return FAIL;
    }
}


int Parser::ReturnStatement(int init, int funcType) {
    next();
    if (wordType == "RETURNTK") {
        next();
        if (wordType == "LPARENT") {
            next();
            if (funcType == 1) {
                Error('g', linenumber);
            }
            if (wordType == "RPARENT") {
                if (funcType == 2 || funcType == 3) {
                    Error('h', linenumber);
                }
                if (!Main) {
                    IntermediateCode.emplace_back(intermediateCode(RET, "", "", int2string(FAIL)));
                }
                else if (Main) {
                    IntermediateCode.emplace_back(intermediateCode(EXIT, "", "", 0));
                }
                Output("<返回语句>");
                returnTime++;
                return 1;
            }
            prev();
            string value;
            int returnType = Expression(iword, 0,value);
            if (returnType != FAIL) {
                next();
                if (returnType != funcType && (funcType == 2 || funcType == 3)) {
                    Error('h', linenumber);
                }
                if (wordType != "RPARENT") {
                    isChangeLineWhenKLM('l');
                }
                returnTime++;
                IntermediateCode.emplace_back(intermediateCode(RET, "", "", value));
                
                Output("<返回语句>");
                return 1;

            }
        }
        else {
            prev();
            if (funcType == 3 || funcType == 2) {
                Error('h', linenumber);
            }
            returnTime++;
            if (Main) {
                IntermediateCode.emplace_back(intermediateCode(EXIT, "", "", ""));
            }
            else{
                IntermediateCode.emplace_back(intermediateCode(RET, "", "", int2string(FAIL)));
            }
            Output("<返回语句>");
            return 1;
        }
    }
    else {
        if ((funcType == 2 || funcType == 3) && returnTime == 0) {
            if (wordType == "RBRACE") {
                Error('h', linenumber);
            }
        }
        RecoverMode(init);
        return FAIL;
    }
}



int Parser::Expression(int init, int isPara,string &value) {
    next();
    int time = 0;
    if (wordType == "PLUS" || wordType == "MINU") {
        operation op = wordType == "PLUS" ? PLUS : MINUS;
        string value1,value2;
        int expressType = Term(iword, isPara,value1);
        if (expressType != 0) {
            string idTemp = genId();
            tempSymTable.insert(make_pair(idTemp, SymbolUnit(idTemp, 2, 2, tempAddr)));
            tempAddr++;
            IntermediateCode.emplace_back(intermediateCode(op, "0", value1, idTemp));
            value1 = idTemp;
            int tempOp = AddOperation(iword);
            op = tempOp == 1 ? PLUS : MINUS;
            while (tempOp != FAIL) {
                if (Term(iword, isPara, value2) != FAIL) {
                    time++;
                    if ((value1[0] <= '9' && value1[0] >= '0' || value1[0] == '+' || value1[0] == '-') 
                        && (value2[0] <= '9' && value2[0] >= '0' || value2[0] == '+' || value2[0] == '-')) {
                        int v1 = string2int(value1);
                        int v2 = string2int(value2);
                        if (op == PLUS) {
                            value1 = int2string(v1 + v2);
                        }
                        else {
                            value1 = int2string(v1 - v2);
                        }
                    }
                    else {
                        string idTemp = genId();
                        tempSymTable.insert(make_pair(idTemp, SymbolUnit(idTemp, 2, 2, tempAddr)));
                        tempAddr++;
                        IntermediateCode.emplace_back(intermediateCode(op, value1, value2, idTemp));
                        value1 = idTemp;
                    }
                    int tempOp = AddOperation(iword);
                    op = tempOp == 1 ? PLUS : MINUS;
                }
                else {
                    break;
                }
            }
            if (time >= 1) {
                expressType = 2;
            }
            value = value1;
            Output("<表达式>");
            return expressType;
        }
        else {
            RecoverMode(init);
            return FAIL;
        }
    }
    else {
        prev();
        string value1, value2;
        int expressType = Term(iword, isPara,value1);
        if (expressType != FAIL) {
            int tempOp = AddOperation(iword);
            operation op = tempOp == 1 ? PLUS : MINUS;
            while (tempOp != FAIL) {
                if (Term(iword, isPara,value2) != FAIL) {
                    time++;
                    string idTemp = genId();
                    tempSymTable.insert(make_pair(idTemp, SymbolUnit(idTemp, 2, 2, tempAddr)));
                    tempAddr++;
                    IntermediateCode.emplace_back(intermediateCode(op, value1, value2, idTemp));
                    value1 = idTemp;
                    int tempOp = AddOperation(iword);
                    op = tempOp == 1 ? PLUS : MINUS;
                }
                else {
                    break;
                }
            }
            if (time >= 1) {
                expressType = 2;
            }
            value = value1;
            Output("<表达式>");
            return expressType;
        }
        else {
            RecoverMode(init);
            return FAIL;
        }
    }
}





int Parser::ConditionStatement(int init, int funcType) {
    next();
    if (wordType == "IFTK") {
        next();
        if (wordType == "LPARENT") {
            string result;
            if (Condition(iword,result) != FAIL) {
                next();
                if (wordType != "RPARENT") {
                    isChangeLineWhenKLM('l');
                }
                string labelEnd,labelElse;
                labelElse = genLabel("else");
                labelEnd = genLabel("end");
                IntermediateCode.emplace_back(intermediateCode(BZ,  "", "", labelElse));
                if (Statement(0) != FAIL) {
                    next();
                    if (wordType == "ELSETK") {
                        IntermediateCode.emplace_back(intermediateCode(GOTO, "", "", labelEnd));
                        IntermediateCode.emplace_back(intermediateCode(LABEL, "", "", labelElse));
                        if (Statement(0) != FAIL) {
                            IntermediateCode.emplace_back(intermediateCode(LABEL, "", "", labelEnd));
                            Output("<条件语句>");
                            return 1;
                        }
                    }
                    else {
                        IntermediateCode.emplace_back(intermediateCode(LABEL, "", "", labelElse));
                        prev();
                        Output("<条件语句>");
                        return 1;
                    }
                }
            }
        }
    }
    else {
        prev();
        return FAIL;
    }
}

int Parser::Factor(int init, int isPara,string &value) {
    next();
    SymbolUnit factorSym;
    int factorType = 0;
    if (wordType == "IDENFR") {
        string name = toLowerCase(word);
        if (isPara == 0) {
            factorSym = isIDENFR(name);
        }
        else {
            tempSymTable.insert(make_pair(word, SymbolUnit(name, 2,0,tempAddr)));
            tempAddr++;
        }
        next();
        if (wordType == "LPARENT") {
            RecoverMode(init);
            factorType = ReturnStatement(iword);
            if (factorType != FAIL) {
                string tmpName = genId();
                tempSymTable.insert(make_pair(tmpName, SymbolUnit(tmpName, 2, factorType, tempAddr)));
                tempAddr++;
                IntermediateCode.emplace_back(intermediateCode(RETVALUE, "", "", tmpName));
                value = tmpName;
                Output("<因子>");
                return factorType;
            }
        }
        else {
            int arrayTime = 0;
            string arrayY;
            while (word == "[") {
                string value;
                int expressionType = Expression(iword, 0, value);
                if (expressionType != FAIL) {
                    if (expressionType != 2) {
                        Error('i', linenumber);
                    }
                    next();
                    if (word != "]") {
                        isChangeLineWhenKLM('m');
                    }
                    arrayTime++;
                    if (arrayTime == 1) {
                        arrayY = value;
                    }
                    else if (arrayTime == 2) {
                        arrayY = arrayY + " " + value;
                    }
                    next();
                }
            }
            if (arrayTime == 0) {
                value = name;
            }
            if (arrayTime >= 1) {
                string idTemp = genId();
                tempSymTable.insert(make_pair(idTemp, SymbolUnit(idTemp, 2, factorSym.type, tempAddr)));
                tempAddr++;
                IntermediateCode.emplace_back(intermediateCode(GETARRAY,  name, arrayY, idTemp));
                value = idTemp;
            }
            prev();
            Output("<因子>");
            factorType = factorSym.type;
            return factorType;
        }

    }
    else if (wordType == "LPARENT") {
        factorType = Expression(iword, 0,value);
        if (factorType != FAIL) {
            next();
            if (wordType != "RPARENT") {
                isChangeLineWhenKLM('l');
            }
            Output("<因子>");
            factorType = 2;
            return factorType;
        }
    }
    else if (wordType == "CHARCON") {
        value = int2string(constChar);
        Output("<因子>");
        return 3;
    }
    else {
        prev();
        int intValue = Integer();
        if (intValue != FAIL) {
            value = int2string(intValue);
            Output("<因子>");
            return 2;
        }
        else {
            RecoverMode(init);
            return FAIL;
        }
    }

}


int Parser::Condition(int init,string &result) {
    result = genId();
    tempSymTable.insert(make_pair(result, SymbolUnit(result, tempAddr, 2, 2)));
    tempAddr++;
    string value1, value2;
    int conditionType = Expression(iword, 0,value1);
    if (conditionType != FAIL) {
        if (conditionType != 2) {
            Error('f', linenumber);
        }
        if (RelationOperation() != FAIL) {
            string symbol = wordType;
            operation op;
            int conditionType = Expression(iword, 0,value2);
            if (conditionType != 2) {
                Error('f', linenumber);
            }
            if (symbol == "LSS") {
                op = LSS;
            }
            else if (symbol == "LEQ") {
                op = LEQ;
            }
            else if (symbol == "GRE") {
                op = GRE;
            }
            else if (symbol == "GEQ") {
                op = GEQ;
            }
            else if (symbol == "EQL") { 
                op = EQL;
            }
            else if (symbol == "NEQ") {
                op = NEQ;
            }
            IntermediateCode.emplace_back(intermediateCode(op, value1, value2, ""));
            Output("<条件>");
            return 1;
        }
        else {
            RecoverMode(init);
            return FAIL;
        }
    }
    else {
        RecoverMode(init);
        return FAIL;
    }
}

/*＜语句＞    ::= ＜循环语句＞｜＜条件语句＞| ＜有返回值函数调用语句＞;  |＜无返回值函数调用语句＞;
｜＜赋值语句＞;｜＜读语句＞;｜＜写语句＞;｜＜情况语句＞｜＜空＞;|＜返回语句＞; | '{'＜语句列＞'}'  */
int Parser::Statement(int funcType) {
    next();
    if (wordType == "SEMICN") {
        Output("<语句>");
        return 1;
    }
    else if (wordType == "LBRACE") {
        if (StatementList(iword, funcType) != FAIL) {
            next();
            if (wordType == "RBRACE") {
                Output("<语句>");
                return 1;
            }
        }
    }
    else {
        prev();
        if (ConditionStatement(iword, funcType) != FAIL || LoopStatement(iword) != FAIL ||
            CaseStatement(iword) != FAIL) {
            Output("<语句>");
            return 1;
        }
        else if (ReturnStatement(iword) != FAIL || VoidStatement(iword) != FAIL || AssignStatement(iword) != FAIL ||
            ReadStatement(iword) != FAIL || WriteStatement(iword) != FAIL ||
            ReturnStatement(iword, funcType) != FAIL) {
            next();
            if (wordType != "SEMICN") {
                isChangeLineWhenKLM('k');
            }
            Output("<语句>");
            return 1;
        }
        else {
            return FAIL;
        }
    }
}



int Parser::AssignStatement(int init) {
    next();
    if (wordType == "IDENFR") {
        string name = toLowerCase(word);
        SymbolUnit assignSym;
        next();
        int time = 0;
        string arrayX;
        while (word == "[") {
            string tempValue;
            int type = Expression(iword, 0, tempValue);
            if (type != FAIL) {
                if (type != 2) {
                    Error('i', linenumber);
                }
                next();
                if (word != "]") {
                    isChangeLineWhenKLM('m');
                }
                time++;
                if (time == 1) {
                    arrayX = tempValue;
                }
                else if (time == 2) {
                    arrayX =arrayX+ " " + tempValue;
                }
                next();
            }
        }
        if (wordType == "ASSIGN") {
            if (tempSymTable.find(name) == tempSymTable.end()) {
                if (globalSymTable.find(name) == globalSymTable.end()) {
                    Error('c', linenumber);
                }
                else {
                    assignSym = globalSymTable[name];
                    if (assignSym.kind == 1) {
                        Error('j', linenumber);
                    }
                }
            }
            else {
                assignSym = tempSymTable[name];
                if (assignSym.kind == 1) {
                    Error('j', linenumber);
                }
            }
            string value;
            if (Expression(iword, 0,value) != FAIL) {
                if (time == 0 ) {
                    IntermediateCode.emplace_back(intermediateCode(ASSIGN, value, "", name));
                }
                else if (time >= 1 ) {
                    IntermediateCode.emplace_back(intermediateCode(PUTARRAY,arrayX , value, name));
                }
                
                Output("<赋值语句>");
                return 1;
            }
        }
        else {
            RecoverMode(init);
            return FAIL;
        }
    }
    else {
        RecoverMode(init);
        return FAIL;
    }
}



int Parser::Announcement() {
    next();
    int funcType = 0;
    if (wordType == "INTTK" || wordType == "CHARTK") {
        if (wordType == "INTTK") {
            funcType = 2;
        }
        else if (wordType == "CHARTK") {
            funcType = 3;
        }
        next();
        if (wordType == "IDENFR") {
            word = toLowerCase(word);
            ReturnList.push_back(word);
            Output("<声明头部>");
            return funcType;
        }
    }
    else {
        prev();
        return FAIL;
    }
}


int Parser::MainFunction(int init) {
    next();
    if (wordType == "VOIDTK") {
        next();
        if (wordType == "MAINTK") {
            SymbolUnit mainItem = SymbolUnit("main", 3, 1, globalAddr);
            globalAddr++;
            IntermediateCode.emplace_back(intermediateCode(FUNC, "void", "", "main"));
            next();
            if (wordType == "LPARENT") {
                next();
                if (wordType != "RPARENT") {
                    isChangeLineWhenKLM('l');
                }
                next();
                if (wordType == "LBRACE") {
                    Main = true;
                    if (MultiStatement(iword, 1) != FAIL) {
                        next();
                        if (wordType == "RBRACE") {
                            mainItem.length = tempAddr;
                            globalSymTable.insert(make_pair("main", mainItem));
                            initTempTable("main");
                            Output("<主函数>");
                            return 1;
                        }
                    }
                }
            }
        }
    }
}


int Parser::VariableDefinition(int index) {
    if (VariableDefinitionWithInit(iword, index) != FAIL) {
        Output("<变量定义>");
        return 1;
    }
    else if (VariableDefinitionWithoutInit(iword, index) != FAIL) {
        Output("<变量定义>");
        return 1;
    }
    else {
        return FAIL;
    }
}


void Parser::VariableState(int index) {
    int frequency = 0;
    while (VariableDefinition(index) != FAIL) {
        next();
        if (wordType == "SEMICN") {
            frequency++;
        }
        else {
            isChangeLineWhenKLM('k');
            break;
        }
    }
    if (frequency >= 1) {
        Output("<变量说明>");
    }
}


void Parser::analyse() {
    ConstState(0);
    VariableState(0);
    initTempTable("global");
    IntermediateCode.emplace_back(intermediateCode(GOTO, "", "", "main"));
    while (FunctionWithReturnValueDefinition(iword) != FAIL || VoidFunctionDefinition(iword) != FAIL) {
        ;
    }
    if (MainFunction(iword)) {
        Output("<程序>");
    }
}