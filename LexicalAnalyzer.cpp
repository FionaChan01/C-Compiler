#include <vector>
#include "LexicalAnalyzer.h"
#include "Unit.h"
#include "Util.h"
#include "Error.h"

using namespace std;
int lineNumber = 1;
vector<int> lines;

vector<pair<string,string>> expression =vector<pair<string,string>>({make_pair("const","CONSTTK"), make_pair("int","INTTK")
                                                    ,make_pair("char","CHARTK"),make_pair("void","VOIDTK")
                                                    ,make_pair("main","MAINTK"),make_pair("if","IFTK"),
                                                    make_pair("else","ELSETK"),make_pair("switch","SWITCHTK")
                                                    ,make_pair("case","CASETK"),make_pair("default","DEFAULTTK")
                                                    ,make_pair("while","WHILETK"),make_pair("for","FORTK")
                                                    ,make_pair("scanf","SCANFTK"),make_pair("printf","PRINTFTK")
                                                    ,make_pair("return","RETURNTK") });


vector<pair<char,string>> unitSymbol = vector<pair<char,string>>({make_pair('+',"PLUS"),make_pair('-',"MINU")
        ,make_pair('/',"DIV"),make_pair('*',"MULT")
        ,make_pair('(',"LPARENT"),make_pair(')',"RPARENT")
        ,make_pair('[',"LBRACK"),make_pair(']',"RBRACK")
        ,make_pair('{',"LBRACE"),make_pair('}',"RBRACE")
        ,make_pair(',',"COMMA"),make_pair(':',"COLON")
        ,make_pair(';',"SEMICN")});


void LexicalAnalyzer::putCode(string c, string codeT) {
    code.push_back(c);
    codeType.push_back(codeT);
    lines.push_back(lineNumber);
}


int LexicalAnalyzer::isExpression(string s)
{
    s = toLowerCase(s);
    for (int i = 0; i < 15; i += 1) {
        if (s == expression[i].first) {
            return i;
        }
    }
    return -1;
}


void LexicalAnalyzer::lexicalAnalyzer2(char c, string ss) {
    codeType.push_back(ss);
    lines.push_back(lineNumber);
    string s;
    s.push_back(c);
    code.push_back(s);
}


void LexicalAnalyzer::lexicalAnalyzer(string s) {
    int i = 0;
    int size = s.size();
    while (i < size) {
        char c = s[i];
        if (c == ' ' || c == '\t' || c == '\n') {
            if (c == '\n') {
                lineNumber++;
            }
            i += 1;
            continue;
        }
        else if (isalpha(c) || c == '_'){
            int temp = i;
            if (i + 1 < s.size()) {
                i += 1;
                char tempc = s[i];
                while (tempc == '_' || isalpha(tempc)  || isalnum(tempc)  ){
                    if (i + 1 < s.size()) {
                        i += 1;
                        tempc = s[i];
                        continue;
                    }
                    break;
                }
                string tempCode = fillString(s, temp, i);
                string tempCodeType;
                int flag = isExpression(tempCode);
                if (flag != -1) {
                    tempCodeType = expression[flag].second;
                }
                else {
                    tempCodeType = "IDENFR";
                }
                putCode(tempCode, tempCodeType);
                continue;
            }
        }
        else if (isalnum(c)){
            int temp = i;
            if (i + 1 < s.size()) {
                i += 1;
                char tempc = s[i];
                while (isalnum(tempc)) {
                    if (i + 1 < s.size()) {
                        i += 1;
                        tempc = s[i];
                        continue;
                    }
                    break;
                }
                string tempCode = fillString(s, temp, i);
                putCode(tempCode, "INTCON");
                continue;
            }

        }
        else if (c == '='){
            if (i + 1 < s.size()) {
                int temp = i;
                i += 1;
                char tempc = s[i];
                if (tempc == '=') {
                    i += 1;
                    string tempCode = fillString(s, temp, i);
                    putCode(tempCode, "EQL");
                    continue;
                }
                else {
                    lexicalAnalyzer2(c, "ASSIGN");
                }
            }
            else {
                lexicalAnalyzer2(c, "ASSIGN");
                i += 1;
            }
        }
        else if (c == '"'){
            int temp = i;
            if (i + 1 < s.size()) {
                i += 1;
                char tempc = s[i];
                if (tempc == '"') {
                    Error('a', lineNumber);
                }
                while (tempc != '"') {
                    if (!(tempc == 32 || tempc == 33 || (tempc <= 126 && tempc >= 35))) {
                        Error('a', lineNumber);
                    }
                    if (i + 1 < s.size()) {
                        i += 1;
                        tempc = s[i];
                        continue;
                    }
                    break;
                }
                string tempCode = fillString(s, temp + 1, i);
                tempCode = addSlash(tempCode);
                i += 1;
                putCode(tempCode, "STRCON");
            }
        }
         else if (c == '!'){
            if (i + 1 < s.size()) {
                int temp = i;
                i += 1;
                char tempc = s[i];
                if (tempc == '=') {
                    i += 1;
                    string tempCode = fillString(s, temp, i);
                    putCode(tempCode, "NEQ");
                }
            }
        }
        else if (c == '\''){
            if (i + 2 < s.size()) {
                int temp = i;
                i += 1;
                char tempc = s[i];
                if (!(tempc == '+' || tempc == '-' || tempc == '*' || tempc == '/' || isalnum(tempc) || isalpha(tempc) || tempc == '_')) {
                    Error('a', lineNumber);
                }
                i += 1;
                char e = s[i];
                if (e == '\'') {
                    string tempCode = fillString(s, temp + 1, i);
                    putCode(tempCode, "CHARCON");
                    i += 1;
                }
            }
        }
        else if (c == '<'){
            if (i + 1 < s.size()) {
                int temp = i;
                i += 1;
                char tempc = s[i];
                if (tempc == '=') {
                    i += 1;
                    string tempCode = fillString(s, temp, i);
                    putCode(tempCode, "LEQ");
                }
                else {
                    lexicalAnalyzer2(c, "LSS");
                }
            }
            else {
                lexicalAnalyzer2(c, "LSS");
                i += 1;
            }
        }
        else if (c == '>'){
            if (i + 1 < s.size()) {
                int temp = i;
                i += 1;
                char tempc = s[i];
                if (tempc == '=') {
                    i += 1;
                    string tempCode = fillString(s, temp, i);
                    putCode(tempCode, "GEQ");
                }
                else {
                    lexicalAnalyzer2(c, "GRE");
                }
            }
            else {
                lexicalAnalyzer2(c, "GRE");
                i += 1;
            }
        }
        else{
            int unitSize = unitSymbol.size();
            for (int i=0;i<unitSize;i++){
                if(c == unitSymbol[i].first){
                    lexicalAnalyzer2(c,unitSymbol[i].second);
                    break;
                }
            }
            i += 1;
        }
    }
}
