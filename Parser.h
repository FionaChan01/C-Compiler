#include <string>
#include<vector>
#include "Unit.h"
#include "SymbolTable.h"

using namespace std;

#ifndef ParserH

#define ParserH
string int2string(int in);
int string2int(string in);
class Parser {
public:
    int VoidStatement(int init);
    void prev();
    int MainFunction(int init);
    int LoopStatement(int init);
    int MultiOperation(int init);
    int Term(int init,int isPara,string &value);
    int VariableDefinition(int index);
    vector<string> getAns();
    int RelationOperation();
    int CaseStatement(int init);
    int StatementList(int init,int funcType);
    int CaseTable(int init,int type);
    void VariableState(int index);
    int ConstState(int index);
    int CaseSubStatement(int init,int type);
    int AssignStatement(int init);
    int MultiStatement(int init,int funcType);
    int Statement(int funcType);
    vector<string> res;
    void ConstDefinition(int index);
    int ReturnStatement(int init,int funcType);
    int Default(int init);
    int AddOperation(int init);
    int ParameterTable(int init, string funcName);
    int ConditionStatement(int init,int funcType);
    void VariableDefinitionWithInitOError(int tempType);
    int Integer();
    int ReturnStatement(int init);
    int VariableDefinitionWithInit(int init,int index);
    void isChangeLineWhenKLM(char type);
    int Step(int init);
    int Announcement();
    int Condition(int init,string &result);
    int ValueList(int init,string funcName);
    int Expression(int init,int isPara,string &value);
    void analyse();
    SymbolUnit isIDENFR(string in);
    int VoidFunctionDefinition(int init);
    int WriteStatement(int init);
    string toLowerCase(string in);
    int NunInteger();
    void next();
    int ReadStatement(int init);
    void Output(string in);
    void insertIDENFR(string name, int inKind, int inType, int inIntValue, char inCharValue, int index,int addr,int length);
    int Factor(int init,int isPara,string &value);
    void RecoverMode(int in);
    int Const();
    string String();
    int VariableDefinitionWithoutInit(int init,int index);
    int FunctionWithReturnValueDefinition(int init);
};

#endif