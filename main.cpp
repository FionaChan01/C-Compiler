#include <fstream>
#include <vector>
#include <sstream>
#include "LexicalAnalyzer.h"
#include "Unit.h"
#include "Parser.h"
#include "Error.h"
#include "intermediateCode.h"
#include "Mips.h"
using namespace std;

vector<string> codeType;
vector<string> code;
vector<string> ans;

int main() {
    LexicalAnalyzer lexicalAnalyzer;
    Parser parser;

    ifstream infile;
    ofstream outfile;
    infile.open("testfile.txt");
    stringstream readBuffer;
    readBuffer << infile.rdbuf();
    string contents(readBuffer.str());
    outfile.open("output.txt");
       
    code.push_back("begin");
    codeType.push_back("begin");
    lines.push_back(0);

    lexicalAnalyzer.lexicalAnalyzer(contents);

    code.push_back("finish1");
    codeType.push_back("finish1");
    code.push_back("finish2");
    codeType.push_back("finish2");

    parser.analyse();
    ans = parser.getAns();

    generateMIPS();
    outputMIPS();
    outputIntermediateCode();


    for (int i = 0; i < ans.size(); i++) {
        outfile << ans[i] << endl;
    }
    outputError();
    infile.close();
    outfile.close();
    return 0;
}
