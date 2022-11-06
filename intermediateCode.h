#include <string>
#include<vector>
using namespace std;

#ifndef MIDCODEH
#define MIDCODEH

enum operation {  
	PLUS,
	MINUS,
	MULT,
	DIV,
	LSS,
	LEQ,
	GRE,
	GEQ,
	EQL,
	NEQ,
	BNZ,
	BZ,
	ASSIGN,
	GOTO,
	SCANF,
	PRINTF,
	LABEL,
	CONST,
	ARRAY,
	VAR,
	FUNC,
	PARAM,
	GETARRAY,
	PUTARRAY,
	RET,
	CALL,
	PARA,
	USE,
	RETVALUE,
	SWITCH,
	CASE,
	EXIT,
};

class intermediateCode {
public:
	operation op;
	string result;
	string x;
	string y;
	intermediateCode(operation opIn,  string xIn, string yIn, string resultIn ) :op(opIn),  x(xIn), y(yIn), result(resultIn) {}
};
#endif