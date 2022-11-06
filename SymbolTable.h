#include <string>
#include<vector>
#include "Unit.h"
using namespace std;

#ifndef SymbolUnitH
#define SymbolUnitH

class SymbolUnit {
public:
    string name;
    int kind;
    int type;
    int length;
    int intValue;
    char charValue;
    int addr;
    string STRING;
    vector<int> paraList;

    SymbolUnit(string inName = " ", int inKind = 0, int inType = 0,int inAddr = 0, int inIntValue = FAIL, char inCharValue = ' ',
        int inLength = FAIL) :
        name(inName), kind(inKind), type(inType),addr(inAddr), intValue(inIntValue), charValue(inCharValue), length(inLength) {
    }

    void insert(int t) {
        paraList.push_back(t);
    }
};

#endif