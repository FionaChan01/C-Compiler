#include <string>
#include<vector>
using namespace std;

#ifndef LexicalAnalyzerH
#define LexicalAnalyzerH

class LexicalAnalyzer{
public:
    void lexicalAnalyzer(string s);
    void lexicalAnalyzer2(char c,string ss);
    void putCode(string c,string codeT);
    int isExpression(string s);
};


#endif