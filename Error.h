#include <string>
#include<vector>
#include "Unit.h"
using namespace std;

#ifndef ERRORH
#define ERRORH

extern vector<string>errorList;

void outputError();
void Error(char c, int lineNum);

#endif