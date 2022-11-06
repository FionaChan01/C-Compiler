#include <iostream>
#include <vector>
#include <fstream>
#include "Unit.h"
#include "Error.h"

using namespace std;

vector<string> errorList;
char errors[10000] = { '0' };

void Error(char c, int lineNum) {
    errors[lineNum] = c;
    errorList.push_back(to_string(lineNum) +" " + c);
}

void outputError() {
    ofstream errorFile;
    errorFile.open("error.txt");
    int lineNum = lines.back() + 1;
    int i=0;
    while(i<lineNum){
        if (errors[i] <= 'p' && errors[i] >= 'a') {
            errorFile << i << ' ' << errors[i] << endl;
        }
        ++i;
    }
    errorFile.close();

}