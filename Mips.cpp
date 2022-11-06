#include <fstream>
#include <iostream>
#include <string>
#include<stack>
#include<vector>
#include "Unit.h"
#include<algorithm>
#include "Parser.h"
#include "Mips.h"
#include "algorithm"

using namespace std;

extern map <string, SymbolUnit> globalSymTable;
extern map<string, map<string, SymbolUnit>> tempT;
extern vector<string> funcList;
extern vector<string> strList;
extern vector<intermediateCode> IntermediateCode;

stack <intermediateCode> funcParaList;
vector<intermediateCode> choices;
string functionId = "";
vector<mips> MIPS;
map <string, SymbolUnit> symbolTable;
bool haveReturnValue = false;
string tReg[10];

string gReg[10];


typedef struct variable {
	string name;
		int time;
}variable;
bool cmp(variable a, variable b) {
	if (a.time <= b.time) {
		return false;
	}
	else {
		return true;
	}
}
bool isVar(string in) {
	if (symbolTable.find(in) != symbolTable.end()) {
		if (in[0] == '@') {
			return false;
		}
		if (symbolTable[in].kind == 2) {
			return true;
		}
	}
	return false;
}
void updateRegisters(int i) {
	map<string , SymbolUnit>::iterator iter;
	vector<variable> tmpVector;
	while(i<IntermediateCode.size()){
		if (IntermediateCode[i].op == FUNC) {
			break;
		}
		intermediateCode sequence = IntermediateCode[i];
		if (isVar(sequence.result)) {
			bool find = false;
			for (int j = 0; j < tmpVector.size(); j++) {
				if (tmpVector[j].name == sequence.result) {
					find = true;
					tmpVector[j].time++;
				}
			}
			if (!find){
				variable tmp;
				tmp.time = 1, tmp.name = sequence.result;
				tmpVector.emplace_back(tmp);
			}
		}
		if (isVar(sequence.x)) {
			bool find = false;
			for (int j = 0; j < tmpVector.size(); j++) {
				if (tmpVector[j].name == sequence.x) {
					find = true;
					tmpVector[j].time++;
				}
			}
			if (!find) {
				variable tmp;
				tmp.time = 1, tmp.name = sequence.x;
				tmpVector.emplace_back(tmp);
			}
		}
		if (isVar(sequence.y)) {
			bool find = false;
			for (int j = 0; j < tmpVector.size(); j++) {
				if (tmpVector[j].name == sequence.y) {
					find = true;
					tmpVector[j].time++;
				}
			}
			if (!find) {
				variable tmp;
				tmp.time = 1, tmp.name = sequence.y;
				tmpVector.emplace_back(tmp);
			}
		}
		i++;
	}
	sort(tmpVector.begin(), tmpVector.end(), cmp);
	for (int i = 0; i < tmpVector.size(); i++) {
		if (i <= 7) {
			gReg[i] = tmpVector[i].name;
		}
		else {
			break;
		}
	}

}

int findEmptyTmpReg() {
	for (int i = 5; i <= 9; i++) {
		if (tReg[i] == " ") {
			return i;
		}
	}
	return -1;
}
string findUseAllReg(string name) {
	for (int i = 0; i <= 7; i++) {
		if (gReg[i] == name) {
			return "$s"+int2string(i);
		}
	}
	return " ";
}
int findUseTmpReg(string name) {
	for (int i = 5; i <= 9; i++) {
		if (tReg[i] == name) {
			return i;
		}
	}
	return FAIL;
}

string insertTmpReg(string in) {
	int find = findEmptyTmpReg();
	if (find != -1) {
		tReg[find] = in;
		return "$t" + int2string(find);
	}
	return " ";
}

vector<string>  split(const string& str, const string& delim) {
	vector<string> res;
	if ("" == str) return  res;

	string strs = str + delim;
	size_t pos;
	size_t size = strs.size();

	for (int i = 0; i < size; ++i) {
		pos = strs.find(delim, i);
		if (pos < size) {
			string s = strs.substr(i, pos - i);
			res.emplace_back(s);
			i = pos + delim.size() - 1;
		}

	}
	return res;
}


void storeValue(string& name, string& reg) {
	int addr;
	if (symbolTable.find(name) != symbolTable.end()) {
		addr = symbolTable[name].addr;
		MIPS.emplace_back(mips(sw, reg, "$sp", " ", 4 * addr));
	}
	else if (globalSymTable.find(name) != globalSymTable.end()) {
		addr = globalSymTable[name].addr;
		MIPS.emplace_back(mips(sw, reg, "$gp", "", 4 * addr));
	}
}


void getValue(string& name, string& reg, int& va, bool need) {

	int addr;
	SymbolUnit sym;
	int value;
	if (symbolTable.find(name) != symbolTable.end()) {
		sym = symbolTable[name];
		if (sym.kind == 1) {
			if (sym.type == 2) {
				value = sym.intValue;
			}
			else if (sym.type == 3) {
				value = int(sym.charValue);
			}
			va = value;
			if (need) {
				MIPS.emplace_back(mips(li, reg, "", "", value));
			}
		}
		else {
			int find = findUseTmpReg(name);
			if (find != FAIL) {
				reg = "$t" + int2string(find);
				tReg[find] = " ";
			}
			else {
				string sfind = findUseAllReg(name);
				if (sfind != " ") {
					reg = sfind;
				}
				else {
					addr = sym.addr;
					MIPS.emplace_back(mips(lw, reg, "$sp", "", 4 * addr));
				}
			}
		}
	}
	else if (globalSymTable.find(name) != globalSymTable.end()) {
		sym = globalSymTable[name];
		if (sym.kind == 1) {
			if (sym.type == 2) {
				value = sym.intValue;
			}
			else if (sym.type == 3) {
				value = sym.charValue;
			}
			va = value;
			if (need) {
				MIPS.emplace_back(mips(li, reg, "", "", value));
			}
		}
		else {
			addr = sym.addr;
			MIPS.emplace_back(mips(lw, reg, "$gp", "", 4 * addr));
		}
	}
	else {
		if (name.size() > 0) {
			va = string2int(name);
			if (need) {
				MIPS.emplace_back(mips(li, reg, "", "", string2int(name)));
			}
		}
	}
}
string getStrIndex(string in) {
	for (int i = 0; i < strList.size(); i++) {
		if (strList[i] == in) {
			return "str" + int2string(i);
		}
	}
}

void outputMIPS() {
    ofstream mipsFile;
    mipsFile.open("mips.txt");
    for (int i = 0; i < MIPS.size(); i++) {
        mips sequence = MIPS[i];

        if(sequence.op == add){
            mipsFile << "addu " << sequence.result << "," << sequence.x << "," << sequence.y << "\n";
        }
        else if(sequence.op == subi){
            mipsFile << "subi " << sequence.result << "," << sequence.x << "," << sequence.imm << "\n";

        }
        else if(sequence.op == addi){
            mipsFile << "addi " << sequence.result << "," << sequence.x << "," << sequence.imm << "\n";

        }

        else if(sequence.op == sub){
            mipsFile << "sub " << sequence.result << "," << sequence.x << "," << sequence.y << "\n";

        }

        else if(sequence.op == mult){
            mipsFile << "mult " << sequence.result << "," << sequence.x << "\n";

        }
        else if(sequence.op == mul){
            mipsFile << "mul " << sequence.result << "," << sequence.x << "," << sequence.y << "\n";

        }
        else if(sequence.op == textSet){
            mipsFile << ".text" << "\n";
        }
        else if(sequence.op == dive){
            mipsFile << "div " << sequence.result << "," << sequence.x << "\n";

        }
        else if(sequence.op == mflo){
            mipsFile << "mflo " << sequence.result << "\n";
        }
        else if(sequence.op == moveop){
            mipsFile << "move " << sequence.result << "," << sequence.x << "\n";

        }
        else if(sequence.op == mfhi){
            mipsFile << "mflo " << sequence.result << "\n";
        }
        else if(sequence.op == dataSet){
            mipsFile << ".data" << "\n";
        }
        else if(sequence.op == jal){
            mipsFile << "jal " << sequence.result << "\n";
        }

        else if(sequence.op == strSet){
            mipsFile << sequence.result << ": .asciiz \"" << sequence.x << "\"\n";

        }
        else if(sequence.op == sll){
            mipsFile << "sll " << sequence.result << "," << sequence.x << "," << sequence.imm << "\n";

        }
        else if(sequence.op == li){
            mipsFile << "li " << sequence.result << "," << sequence.imm << "\n";

        }
        else if(sequence.op == la){
            mipsFile << "la " << sequence.result << "," << sequence.x << "\n";

        }
        else if(sequence.op == sw){
            mipsFile << "sw " << sequence.result << "," << sequence.imm << "(" << sequence.x << ")" << "\n";

        }
        else if(sequence.op == syscall){
            mipsFile << "syscall " << "\n";
        }
        else if(sequence.op == lw){
            mipsFile << "lw " << sequence.result << ", " << sequence.imm << "(" << sequence.x << ")" << "\n";
        }
        else if(sequence.op == bne){
            mipsFile << "bne " << sequence.result << ", " << sequence.x << ", " << sequence.y << "\n";

        }

        else if(sequence.op == jr){
            mipsFile << "jr " << sequence.result << "\n";
        }
        else if(sequence.op == beq){
            mipsFile << "beq " << sequence.result << ", " << sequence.x << ", " << sequence.y << "\n";

        }


        else if(sequence.op == bge){
            mipsFile << "bge " << sequence.result << ", " << sequence.x << ", " << sequence.y << "\n";

        }
        else if(sequence.op == bgt){
            mipsFile << "bgt " << sequence.result << ", " << sequence.x << ", " << sequence.y << "\n";

        }
        else if(sequence.op == blt){
            mipsFile << "blt " << sequence.result << ", " << sequence.x << ", " << sequence.y << "\n";

        }
        else if(sequence.op == ble){
            mipsFile << "ble " << sequence.result << ", " << sequence.x << ", " << sequence.y << "\n";

        }
        else if(sequence.op == j){
            mipsFile << "j " << sequence.result << "\n";
        }
        else if(sequence.op == label){
            mipsFile << sequence.result << ": \n";

        }

    }
    mipsFile.close();
}


void generateMIPS() {
    fill(tReg, tReg+10, " ");
    fill(gReg, gReg+10, " ");
	MIPS.emplace_back(mips(dataSet, "", "", "", 0));
	for (int i = 0; i < strList.size(); i++) {
		MIPS.emplace_back(mips(strSet, "str" + int2string(i), strList[i], "", 0));
	}
	MIPS.emplace_back(mips(strSet, "nextLine", "\\n", "", 0));
	MIPS.emplace_back(mips(textSet, "", "", "", 0));
	for (int intermediateId = 0; intermediateId < IntermediateCode.size(); intermediateId++) {
		int va = FAIL, addr = 0, va1 = FAIL, va2 = FAIL,va3= FAIL;
		intermediateCode sequence = IntermediateCode[intermediateId];

		if (sequence.op == PRINTF){
            if (sequence.x == "4") {
                string strName = getStrIndex(sequence.result);
                MIPS.emplace_back(mips(la, "$a0", strName, "", 0));
                MIPS.emplace_back(mips(li, "$v0", "", "", 4));
                MIPS.emplace_back(mips(syscall, "", "", "", 0));
            }
            else if (sequence.x == "5") {
                MIPS.emplace_back(mips(la, "$a0", "nextLine", "", 0));
                MIPS.emplace_back(mips(li, "$v0", "", "", 4));
                MIPS.emplace_back(mips(syscall, "", "", "", 0));
            }
            else {
                string a0 = "$a0";
                getValue(sequence.result, a0, va, true);
                if (a0 != "$a0") {
                    MIPS.emplace_back(mips(moveop, "$a0", a0, "", 0));
                }
                if (sequence.x == "2") {
                    MIPS.emplace_back(mips(li, "$v0", "", "", 1));
                }
                else {
                    MIPS.emplace_back(mips(li, "$v0", "", "", 11));
                }
                MIPS.emplace_back(mips(syscall, "", "", "", 0));
            }
		}
        else if (sequence.op == SCANF){
            SymbolUnit scan;
            string sfind = findUseAllReg(sequence.result);
            if (sfind != " ") {
                scan = symbolTable[sequence.result];
                MIPS.emplace_back(mips(li, "$v0", "", "", scan.type == 2 ? 5 : 12));
                MIPS.emplace_back(mips(syscall, "", "", "", 0));
                MIPS.emplace_back(mips(moveop, sfind, "$v0", "", 0));
            }
            else {
                if (symbolTable.find(sequence.result) != symbolTable.end()) {
                    scan = symbolTable[sequence.result];
                    MIPS.emplace_back(mips(li, "$v0", "", "", scan.type == 2 ? 5 : 12));
                    MIPS.emplace_back(mips(syscall, "", "", "", 0));
                    addr = symbolTable[sequence.result].addr;
                    MIPS.emplace_back(mips(sw, "$v0", "$sp", "", 4 * addr));
                }
                else if (globalSymTable.find(sequence.result) != globalSymTable.end()) {
                    scan = globalSymTable[sequence.result];
                    MIPS.emplace_back(mips(li, "$v0", "", "", scan.type == 2 ? 5 : 12));
                    MIPS.emplace_back(mips(syscall, "", "", "", 0));
                    addr = globalSymTable[sequence.result].addr;
                    MIPS.emplace_back(mips(sw, "$v0", "$gp", "", 4 * addr));
                }
            }
        }
        else if (sequence.op == MINUS){
            string x = "$t0", y = "$t1", result = "$t2";
            string find = " ";
            if (sequence.result[0] == '@') {
                find = insertTmpReg(sequence.result);
                if (find != " ") {
                    result = find;
                }
            }
            string sfind = findUseAllReg(sequence.result);
            if (sfind != " ") {
                result = sfind;
            }
            getValue(sequence.x, x, va1, false);
            getValue(sequence.y, y, va2, false);
            if (va1 != FAIL && va2 != FAIL) {
                MIPS.emplace_back(mips(li, result, "", "", va1-va2));
            }
            else if (va1 != FAIL && va2 == FAIL) {
                MIPS.emplace_back(mips(addi, result, y, "", -va1));
                MIPS.emplace_back(mips(sub, result, "$0", result,0));
            }
            else if (va1 == FAIL && va2 != FAIL) {
                MIPS.emplace_back(mips(addi, result,x, "", -va2));
            }
            else {
                MIPS.emplace_back(mips(sub, result, x, y, 0));
            }
            if (sequence.result[0] == '@') {
                if (find == " ") {
                    storeValue(sequence.result, result);
                }
            }
            else {
                if (sfind == " ") {
                    storeValue(sequence.result, result);
                }
            }
        }
        else if (sequence.op == RETVALUE){
            if (symbolTable.find(sequence.result) != symbolTable.end()) {
                if (sequence.result[0] == '@') {
                    string find = insertTmpReg(sequence.result);
                    if (find != " ") {
                        MIPS.emplace_back(mips(moveop, find, "$v0", "", 0));
                    }
                }
                string sfind = findUseAllReg(sequence.result);
                if (sfind != " ") {
                    MIPS.emplace_back(mips(moveop, sfind, "$v0", "", 0));
                }
                else {
                    SymbolUnit sym = symbolTable[sequence.result];
                    int addr = sym.addr;
                    MIPS.emplace_back(mips(sw, "$v0", "$sp", "", 4 * addr));
                }
            }
        }
        else if (sequence.op == PLUS){
            string x = "$t0", y = "$t1", result = "$t2";
            string find = " ";
            if (sequence.result[0] == '@') {
                find = insertTmpReg(sequence.result);
                if (find != " ") {
                    result = find;
                }
            }
            string sfind = findUseAllReg(sequence.result);
            if (sfind != " ") {
                result = sfind;
            }
            getValue(sequence.x, x, va1, false);
            getValue(sequence.y, y, va2, false);
            if (va1 != FAIL && va2 != FAIL) {
                MIPS.emplace_back(mips(li, result, "", "", va1 + va2));
            }
            else if (va1 != FAIL && va2 == FAIL) {
                MIPS.emplace_back(mips(addi, result, y, "", va1 ));
            }
            else if (va1 == FAIL && va2 != FAIL) {
                MIPS.emplace_back(mips(addi, result, x, "", va2));
            }
            else {
                MIPS.emplace_back(mips(add, result, x, y, 0));
            }
            if (sequence.result[0] == '@') {
                if (find == " ") {
                    storeValue(sequence.result, result);
                }
            }
            else {
                if (sfind == " ") {
                    storeValue(sequence.result, result);
                }
            }
        }
        else if (sequence.op == ASSIGN){
            string name = sequence.result;
            if (sequence.result[0] == '@') {
                string find = insertTmpReg(sequence.result);
                if (find != " ") {
                    string t0 = find;
                    getValue(sequence.x, t0, va, true);
                    if (t0 !=  find) {
                        MIPS.emplace_back(mips(moveop,find, t0, "", 0));
                    }
                }
                else {
                    string t0 = "$t0";
                    getValue(sequence.x, t0, va, true);
                    storeValue(name, t0);
                }
            }
            else {
                if (symbolTable.find(name) != symbolTable.end()) {
                    string t0 = "$t0";
                    string sfind = findUseAllReg(sequence.result);
                    if (sfind != " ") {
                        t0 = sfind;
                        getValue(sequence.x, t0, va, true);
                        if (t0 != sfind) {
                            MIPS.emplace_back(mips(moveop, sfind, t0, "", 0));
                        }
                    }
                    else {
                        getValue(sequence.x, t0, va, true);
                        storeValue(name, t0);
                    }
                }
                else if (globalSymTable.find(name) != globalSymTable.end()) {
                    string t0 = "$t0";
                    int va;
                    getValue(sequence.x, t0, va, true);
                    storeValue(name, t0);
                }
            }
        }
        else if (sequence.op == MULT){
            string result = "$t2", x = "$t0", y = "$t1";
            string find = " ",sfind = " ";
            if (sequence.result[0] == '@') {
                find = insertTmpReg(sequence.result);
                if (find != " ") {
                    result = find;
                }
            }
            sfind = findUseAllReg(sequence.result);
            if (sfind != " ") {
                result = sfind;
            }
            getValue(sequence.x, x, va1, false);
            getValue(sequence.y, y, va2, false);
            if (va1 != FAIL && va2 != FAIL) {
                MIPS.emplace_back(mips(li, result, "", "", va1*va2));
            }
            else if (va1 != FAIL && va2 == FAIL) {
                if (va1 == 1) {
                    MIPS.emplace_back(mips(moveop, result, y, "", 0));
                }
                else if (va1 == 0) {
                    MIPS.emplace_back(mips(li, result, "", "", 0));
                }
                else {
                    MIPS.emplace_back(mips(li, x, "", "", va1));
                    MIPS.emplace_back(mips(mul, result, x, y, 0));
                }
            }
            else if (va1 == FAIL && va2 != FAIL) {
                if (va2 == 1) {
                    MIPS.emplace_back(mips(moveop,result,x,"",0));
                }
                else if (va2 == 0) {
                    MIPS.emplace_back(mips(li, result, "", "", 0));
                }
                else {
                    MIPS.emplace_back(mips(li, y, "", "", va2));
                    MIPS.emplace_back(mips(mul, result, x,y, 0));
                }
            }
            else {
                MIPS.emplace_back(mips(mul, result, x, y, 0));
            }
            if (sequence.result[0] == '@') {
                if (find == " ") {
                    storeValue(sequence.result, result);
                }
            }
            else {
                if (sfind == " ") {
                    storeValue(sequence.result, result);
                }
            }
        }
        else if (sequence.op == DIV){
            string result = "$t2", x = "$t0", y = "$t1";
            string find,sfind;
            if (sequence.result[0] == '@') {
                string find = insertTmpReg(sequence.result);
                if (find != " ") {
                    result = find;
                }
            }
            sfind = findUseAllReg(sequence.result);
            if (sfind != " "){
                result = sfind;
            }
            getValue(sequence.x, x, va1, false);
            getValue(sequence.y, y, va2, false);
            if (va1 != FAIL && va2 != FAIL) {
                MIPS.emplace_back(mips(li, result, "", "", va1 / va2));
            }
            else if (va1 != FAIL && va2 == FAIL) {
                if (va1 == 0) {
                    MIPS.emplace_back(mips(li, result, "", "", 0));
                }
                else {
                    MIPS.emplace_back(mips(li, x, "", "", va1));
                    MIPS.emplace_back(mips(dive, x, y, "", 0));
                    MIPS.emplace_back(mips(mflo, result, "", "", 0));
                }
            }
            else if (va1 == FAIL && va2 != FAIL) {
                if (va2 == 1) {
                    MIPS.emplace_back(mips(moveop, result, x, "", 0));
                }
                else {
                    MIPS.emplace_back(mips(li, y, "", "", va2));
                    MIPS.emplace_back(mips(dive, x, y, "", 0));
                    MIPS.emplace_back(mips(mflo, result, "", "", 0));
                }
            }
            else {
                MIPS.emplace_back(mips(dive, x, y, "", 0));
                MIPS.emplace_back(mips(mflo, result, "", "", 0));
            }
            if (sequence.result[0] == '@') {
                if (find == " ") {
                    storeValue(sequence.result, result);
                }
            }
            else {
                if (sfind == " ") {
                    storeValue(sequence.result, result);
                }
            }
        }
        else if (sequence.op == GETARRAY){
            string index = sequence.y;
            string name = sequence.result;
            vector<string> indexs = split(index, " ");
            if (indexs.size() == 1) {
                string x = "$t1";
                string address = "$t0";
                getValue(sequence.y, x, va, false);
                if (x != "$t1") {
                    MIPS.emplace_back(mips(moveop, "$t1", x, "", 0));
                    x = "$t1";
                }
                if (symbolTable.find(sequence.x) != symbolTable.end()) {
                    addr = symbolTable[sequence.x].addr;
                    if (va != FAIL) {
                        MIPS.emplace_back(mips(lw, "$t2", "$sp", "", 4 * (addr + va)));
                    }
                    else {
                        MIPS.emplace_back(mips(addi, address, "$sp", "", 4 * addr));
                        MIPS.emplace_back(mips(sll, x, x, "", 2));
                        MIPS.emplace_back(mips(add, address, address, x, 0));
                        MIPS.emplace_back(mips(lw, "$t2", address, "", 0));
                    }
                }
                else if (globalSymTable.find(sequence.x) != globalSymTable.end()) {
                    addr = globalSymTable[sequence.x].addr;
                    if (va != FAIL) {
                        MIPS.emplace_back(mips(lw, "$t2", "$gp", "", 4 * (addr + va)));
                    }
                    else {
                        MIPS.emplace_back(mips(addi, address, "$gp", "", 4 * addr));
                        MIPS.emplace_back(mips(sll, x, x, "", 2));
                        MIPS.emplace_back(mips(add, address, address, x, 0));
                        MIPS.emplace_back(mips(lw, "$t2", address, "", 0));
                    }
                }
                string storeReg = "$t2";
                string find = " ", sfind = " ";
                sfind = findUseAllReg(sequence.result);
                if (sequence.result[0] == '@') {
                    find = insertTmpReg(sequence.result);
                    if (find != " ") {
                        MIPS.emplace_back(mips(moveop, find, storeReg, "", 0));
                    }
                }
                else if (sfind != " ") {
                    MIPS.emplace_back(mips(moveop, sfind, storeReg, "", 0));
                }
                else {
                    storeValue(sequence.result, storeReg);
                }
            }
            else if (indexs.size() == 2) {
                string address = "$t0";
                string x = "$t1", y = "$t2";
                string arryValue = "$t3";
                getValue(indexs[0], x, va1, false);
                getValue(indexs[1], y, va2, false);
                if (x != "$t1") {
                    MIPS.emplace_back(mips(moveop, "$t1", x, "", 0));
                    x = "$t1";
                }
                if (y != "$t2") {
                    MIPS.emplace_back(mips(moveop, "$t2", y, "", 0));
                    y = "$t2";
                }
                if (symbolTable.find(sequence.x) != symbolTable.end()) {
                    addr = symbolTable[sequence.x].addr;
                    int length = symbolTable[sequence.x].length;
                    if (va1 != FAIL && va2 != FAIL) {
                        MIPS.emplace_back(mips(lw, arryValue, "$sp", "", 4 * (addr + length * va1 + va2)));
                    }
                    else if (va1 != FAIL && va2 == FAIL) {
                        MIPS.emplace_back(mips(addi, address, "$sp", "", 4 * addr));
                        MIPS.emplace_back(mips(addi, x, y, "", length * va1));
                        MIPS.emplace_back(mips(sll, x, x, "", 2));
                        MIPS.emplace_back(mips(add, address, address, x, 0));
                        MIPS.emplace_back(mips(lw, arryValue, address, "", 0));
                    }
                    else if (va1 == FAIL && va2 != FAIL) {
                        MIPS.emplace_back(mips(addi, address, "$sp", "", 4 * addr));
                        int vaTemp;
                        string len = "$t4", strLength = int2string(length);
                        getValue(strLength, len, vaTemp, true);
                        MIPS.emplace_back(mips(mul, x, x, len, 0));
                        MIPS.emplace_back(mips(addi, x, x, "", va2));
                        MIPS.emplace_back(mips(sll, x, x, "", 2));
                        MIPS.emplace_back(mips(add, address, address, x, 0));
                        MIPS.emplace_back(mips(lw, arryValue, address, "", 0));
                    }
                    else {
                        MIPS.emplace_back(mips(addi, address, "$sp", "", 4 * addr));
                        int vaTemp;
                        string len = "$t4", strLength = int2string(length);
                        getValue(strLength, len, vaTemp, true);
                        MIPS.emplace_back(mips(mul, x, x, len, 0));
                        MIPS.emplace_back(mips(add, x, x, y, 0));
                        MIPS.emplace_back(mips(sll, x, x, "", 2));
                        MIPS.emplace_back(mips(add, address, address, x, 0));
                        MIPS.emplace_back(mips(lw, arryValue, address, "", 0));
                    }
                }
                else if (globalSymTable.find(sequence.x) != globalSymTable.end()) {
                    addr = globalSymTable[sequence.x].addr;
                    int length = globalSymTable[sequence.x].length;
                    if (va1 != FAIL && va2 != FAIL) {
                        MIPS.emplace_back(mips(lw, arryValue, "$gp", "", 4 * (addr + length * va1 + va2)));
                    }
                    else if (va1 != FAIL && va2 == FAIL) {
                        MIPS.emplace_back(mips(addi, address, "$gp", "", 4 * addr));
                        MIPS.emplace_back(mips(addi, x, y, "", length * va1));
                        MIPS.emplace_back(mips(sll, x, x, "", 2));
                        MIPS.emplace_back(mips(add, address, address, x, 0));
                        MIPS.emplace_back(mips(lw, arryValue, address, "", 0));
                    }
                    else if (va1 == FAIL && va2 != FAIL) {
                        MIPS.emplace_back(mips(addi, address, "$gp", "", 4 * addr));
                        int vaTemp;
                        string len = "$t4", strLength = int2string(length);
                        getValue(strLength, len, vaTemp, true);
                        MIPS.emplace_back(mips(mul, x, x, len, 0));
                        MIPS.emplace_back(mips(addi, x, x, "", va2));
                        MIPS.emplace_back(mips(sll, x, x, "", 2));
                        MIPS.emplace_back(mips(add, address, address, x, 0));
                        MIPS.emplace_back(mips(lw, arryValue, address, "", 0));
                    }
                    else {
                        MIPS.emplace_back(mips(addi, address, "$gp", "", 4 * addr));
                        int vaTemp;
                        string len = "$t4", strLength = int2string(length);
                        getValue(strLength, len, vaTemp, true);
                        MIPS.emplace_back(mips(mul, x, x, len, 0));
                        MIPS.emplace_back(mips(add, x, x, y, 0));
                        MIPS.emplace_back(mips(sll, x, x, "", 2));
                        MIPS.emplace_back(mips(add, address, address, x, 0));
                        MIPS.emplace_back(mips(lw, arryValue, address, "", 0));
                    }
                }
                string find = " ", sfind = " ";
                sfind = findUseAllReg(sequence.result);
                if (sequence.result[0] == '@') {
                    find = insertTmpReg(sequence.result);
                    if (find != " ") {
                        MIPS.emplace_back(mips(moveop, find, arryValue, "", 0));
                    }
                }
                else if (sfind != " ") {
                    MIPS.emplace_back(mips(moveop, sfind, arryValue, "", 0));
                }
                else {
                    storeValue(sequence.result, arryValue);
                }
            }
        }
        else if (sequence.op == PUTARRAY){
            string index = sequence.x;
            string name = sequence.result;
            vector<string> indexs = split(index, " ");
            if (indexs.size() == 1) {
                string y = "$t0";
                string x = "$t1";
                getValue(sequence.y, y, va1, true);
                getValue(indexs[0], x, va2, false);
                if (x != "$t1") {
                    MIPS.emplace_back(mips(moveop,"$t1",x,"",0));
                    x = "$t1";
                }
                if (symbolTable.find(name) != symbolTable.end()) {
                    addr = symbolTable[name].addr;
                    if (va2 != FAIL) {
                        MIPS.emplace_back(mips(sw, y, "$sp", "", (4 * (addr+va2))));
                    }
                    else {
                        MIPS.emplace_back(mips(addi, "$t2", "$sp", "", 4 * addr));
                        MIPS.emplace_back(mips(sll, x, x, "", 2));
                        MIPS.emplace_back(mips(add, "$t2", "$t2", x, 0));
                        MIPS.emplace_back(mips(sw, y, "$t2", "", 0));
                    }
                }
                else if (globalSymTable.find(name) != globalSymTable.end()) {

                    addr = globalSymTable[name].addr;
                    if (va2 != FAIL) {
                        MIPS.emplace_back(mips(sw, y, "$gp", "", (4 * (addr + va2))));
                    }
                    else {
                        MIPS.emplace_back(mips(addi, "$t2", "$gp", "", 4 * addr));
                        MIPS.emplace_back(mips(sll, x, x, "", 2));
                        MIPS.emplace_back(mips(add, "$t2", "$t2", x, 0));
                        MIPS.emplace_back(mips(sw, y, "$t2", "", 0));
                    }
                }
            }
            else if (indexs.size() == 2) {
                string y = "$t0";
                string x1 = "$t1";
                string x2 = "$t2";
                string len = "$t3";
                getValue(sequence.y, y, va, true);
                getValue(indexs[0], x1, va2, false);
                getValue(indexs[1], x2, va3, false);

                if (x1 != "$t1") {
                    MIPS.emplace_back(mips(moveop, "$t1", x1, "", 0));
                    x1 = "$t1";
                }
                if (x2 != "$t2") {
                    MIPS.emplace_back(mips(moveop, "$t2", x2, "", 0));
                    x2 = "$t2";
                }
                if (symbolTable.find(name) != symbolTable.end()) {
                    int length = symbolTable[name].length;
                    string step = int2string(length);
                    addr = symbolTable[name].addr;
                    if (va2 != FAIL && va3 != FAIL) {
                        MIPS.emplace_back(mips(sw, y, "$sp", "", 4*(addr+va2*length+va3)));
                    }
                    else if (va2 == FAIL && va3 != FAIL) {
                        getValue(step, len, va1, true);
                        MIPS.emplace_back(mips(addi, "$t4", "$sp", "", 4 * addr));
                        MIPS.emplace_back(mips(mul, x1, len, x1, 0));
                        MIPS.emplace_back(mips(addi, x1, x2, "", va3));
                        MIPS.emplace_back(mips(sll, x1, x1, "", 2));
                        MIPS.emplace_back(mips(add, "$t4", "$t4", x1, 0));
                        MIPS.emplace_back(mips(sw, y, "$t4", "", 0));
                    }
                    else if (va2 != FAIL && va3 == FAIL) {
                        MIPS.emplace_back(mips(addi, "$t4", "$sp", "", 4 * addr));
                        MIPS.emplace_back(mips(li, x1, "", "", va2*length));
                        MIPS.emplace_back(mips(add, x1, x1, x2, 0));
                        MIPS.emplace_back(mips(sll, x1, x1, "", 2));
                        MIPS.emplace_back(mips(add, "$t4", "$t4", x1, 0));
                        MIPS.emplace_back(mips(sw, y, "$t4", "", 0));
                    }
                    else {
                        getValue(step, len, va1, true);
                        MIPS.emplace_back(mips(addi, "$t4", "$sp", "", 4 * addr));
                        MIPS.emplace_back(mips(mul, x1, len, x1, 0));
                        MIPS.emplace_back(mips(add, x1, x1, x2, 0));
                        MIPS.emplace_back(mips(sll, x1, x1, "", 2));
                        MIPS.emplace_back(mips(add, "$t4", "$t4", x1, 0));
                        MIPS.emplace_back(mips(sw, y, "$t4", "", 0));
                    }
                }
                else if (globalSymTable.find(name) != globalSymTable.end()) {
                    int length = globalSymTable[name].length;
                    string step = int2string(length);
                    addr = globalSymTable[name].addr;
                    if (va2 != FAIL && va3 != FAIL) {
                        MIPS.emplace_back(mips(sw, y, "$gp", "", 4 * (addr + va2 * length + va3)));
                    }
                    else if (va2 == FAIL && va3 != FAIL) {
                        getValue(step, len, va1, true);
                        MIPS.emplace_back(mips(addi, "$t4", "$gp", "", 4 * addr));
                        MIPS.emplace_back(mips(mul, x1, len, x1, 0));
                        MIPS.emplace_back(mips(addi, x1, x2, "", va3));
                        MIPS.emplace_back(mips(sll, x1, x1, "", 2));
                        MIPS.emplace_back(mips(add, "$t4", "$t4", x1, 0));
                        MIPS.emplace_back(mips(sw, y, "$t4", "", 0));
                    }
                    else if (va2 != FAIL && va3 == FAIL) {
                        MIPS.emplace_back(mips(addi, "$t4", "$gp", "", 4 * addr));
                        MIPS.emplace_back(mips(li, x1, "", "", va2 * length));
                        MIPS.emplace_back(mips(add, x1, x1, x2, 0));
                        MIPS.emplace_back(mips(sll, x1, x1, "", 2));
                        MIPS.emplace_back(mips(add, "$t4", "$t4", x1, 0));
                        MIPS.emplace_back(mips(sw, y, "$t4", "", 0));
                    }
                    else {
                        getValue(step, len, va1, true);
                        MIPS.emplace_back(mips(addi, "$t4", "$gp", "", 4 * addr));
                        MIPS.emplace_back(mips(mul, x1, len, x1, 0));
                        MIPS.emplace_back(mips(add, x1, x1, x2, 0));
                        MIPS.emplace_back(mips(sll, x1, x1, "", 2));
                        MIPS.emplace_back(mips(add, "$t4", "$t4", x1, 0));
                        MIPS.emplace_back(mips(sw, y, "$t4", "", 0));
                    }

                }
            }
        }
        else if (sequence.op == USE){
            funcParaList.push(sequence);
        }
        else if (sequence.op == CALL){
            SymbolUnit sym = globalSymTable[sequence.result];
            int paraLength = globalSymTable[sequence.result].paraList.size();
            vector<string>tmpList, allList;
            for (int i = 5; i <= 9; i++) {
                if (tReg[i] != " ") {
                    tmpList.emplace_back("$t" + int2string(i));
                }
            }
            for (int i = 0; i <= 7; i++) {
                if (gReg[i] != " ") {
                    allList.emplace_back("$s" + int2string(i));
                }
            }
            int tmpListLen = tmpList.size();
            int allListLen = allList.size();
            for (int i = 0; i < paraLength; i++) {
                int va;
                string tmpReg = "$t4";
                intermediateCode para = funcParaList.top();
                funcParaList.pop();
                getValue(para.result, tmpReg, va, true);
                MIPS.emplace_back(mips(sw, tmpReg, "$sp", "", -4 * sym.length - 8 - 4 * (tmpListLen + allListLen) + 4 * (paraLength - i - 1)));
            }
            MIPS.emplace_back(mips(addi, "$sp", "$sp", "", -4 * sym.length - 8 - 4 * (tmpListLen + allListLen)));
            MIPS.emplace_back(mips(sw, "$ra", "$sp", "", sym.length * 4 + 4));
            for (int i = 0; i < tmpListLen; i++) {
                MIPS.emplace_back(mips(sw, tmpList[i], "$sp", "", sym.length * 4 + 4 + (i + 1) * 4));
            }
            for (int i = 0; i < allListLen; i++) {
                MIPS.emplace_back(mips(sw, allList[i], "$sp", "", sym.length * 4 + 4 + 4 * tmpListLen + (i + 1) * 4));
            }
            MIPS.emplace_back(mips(jal, sequence.result, "", "", 0));
            for (int i = 0; i < allListLen; i++) {
                MIPS.emplace_back(mips(lw, allList[i], "$sp", "", sym.length * 4 + 4 + 4 * tmpListLen + (i + 1) * 4));
            }
            for (int i = 0; i < tmpListLen; i++) {
                MIPS.emplace_back(mips(lw, tmpList[i], "$sp", "", sym.length * 4 + 4 + (i + 1) * 4));
            }
            MIPS.emplace_back(mips(lw, "$ra", "$sp", "", sym.length * 4 + 4));
            MIPS.emplace_back(mips(addi, "$sp", "$sp", "", 4 * sym.length + 8 + 4 * (tmpListLen + allListLen)));

        }
        else if (sequence.op == LABEL){
            MIPS.emplace_back(mips(label, sequence.result, "", "", 0));
        }
        else if (sequence.op == LEQ){
            string x = "$t0";
            string y = "$t1";
            getValue(sequence.x, x, va1, false);
            getValue(sequence.y, y, va2, false);

            intermediateCode nextOrder = IntermediateCode[intermediateId + 1];
            if (nextOrder.op == BZ) {
                if (va1 != FAIL && va2 != FAIL) {
                    if (va1 > va2) {
                        MIPS.emplace_back(mips(j, nextOrder.result, "", "", 0));
                    }
                }
                else if (va1 == FAIL && va2 != FAIL) {
                    MIPS.emplace_back(mips(bgt, x, int2string(va2), nextOrder.result, 0));
                }
                else if (va2 == FAIL && va1 != FAIL) {
                    MIPS.emplace_back(mips(blt, y, int2string(va1), nextOrder.result, 0));
                }
                else {
                    MIPS.emplace_back(mips(bgt, x, y, nextOrder.result, 0));
                }
            }
            else if (nextOrder.op == BNZ) {
                if (va1 != FAIL && va2 != FAIL) {
                    if (va1 <= va2) {
                        MIPS.emplace_back(mips(j, nextOrder.result, "", "", 0));
                    }
                }
                else if (va1 == FAIL && va2 != FAIL) {
                    MIPS.emplace_back(mips(ble, x, int2string(va2), nextOrder.result, 0));
                }
                else if (va2 == FAIL && va1 != FAIL) {
                    MIPS.emplace_back(mips(bge, y, int2string(va1), nextOrder.result, 0));
                }
                else {
                    MIPS.emplace_back(mips(ble, x, y, nextOrder.result, 0));
                }
            }
            intermediateId++;
        }
        else if (sequence.op == LSS){
            string x = "$t0";
            string y = "$t1";
            getValue(sequence.x, x, va1, false);
            getValue(sequence.y, y, va2, false);

            intermediateCode nextOrder = IntermediateCode[intermediateId + 1];
            if (nextOrder.op == BZ) {
                if (va1 != FAIL && va2 != FAIL) {
                    if (va1 >= va2) {
                        MIPS.emplace_back(mips(j, nextOrder.result, "", "", 0));
                    }
                }
                else if (va1 == FAIL && va2 != FAIL) {
                    MIPS.emplace_back(mips(bge, x, int2string(va2), nextOrder.result, 0));
                }
                else if (va2 == FAIL && va1 != FAIL) {
                    MIPS.emplace_back(mips(ble, y, int2string(va1), nextOrder.result, 0));
                }
                else {
                    MIPS.emplace_back(mips(bge, x, y, nextOrder.result, 0));
                }
            }
            else if (nextOrder.op == BNZ) {
                if (va1 != FAIL && va2 != FAIL) {
                    if (va1 <= va2) {
                        MIPS.emplace_back(mips(j, nextOrder.result, "", "", 0));
                    }
                }
                else if (va1 == FAIL && va2 != FAIL) {
                    MIPS.emplace_back(mips(blt, x, int2string(va2), nextOrder.result, 0));
                }
                else if (va2 == FAIL && va1 != FAIL) {
                    MIPS.emplace_back(mips(bgt, y, int2string(va1), nextOrder.result, 0));
                }
                else {
                    MIPS.emplace_back(mips(blt, x, y, nextOrder.result, 0));
                }
            }
            intermediateId++;
        }
        else if (sequence.op == GRE){
            string x = "$t0";
            string y = "$t1";
            getValue(sequence.x, x, va1, false);
            getValue(sequence.y, y, va2, false);

            intermediateCode nextOrder = IntermediateCode[intermediateId + 1];
            if (nextOrder.op == BZ) {
                if (va1 != FAIL && va2 != FAIL) {
                    if (va1 <= va2) {
                        MIPS.emplace_back(mips(j, nextOrder.result, "", "", 0));
                    }
                }
                else if (va1 == FAIL && va2 != FAIL) {
                    MIPS.emplace_back(mips(ble, x, int2string(va2), nextOrder.result, 0));
                }
                else if (va2 == FAIL && va1 != FAIL) {
                    MIPS.emplace_back(mips(bge, y, int2string(va1), nextOrder.result, 0));
                }
                else {
                    MIPS.emplace_back(mips(ble, x, y, nextOrder.result, 0));
                }
            }
            else if (nextOrder.op == BNZ) {
                if (va1 != FAIL && va2 != FAIL) {
                    if (va1 <= va2) {
                        MIPS.emplace_back(mips(j, nextOrder.result, "", "", 0));
                    }
                }
                else if (va1 == FAIL && va2 != FAIL) {
                    MIPS.emplace_back(mips(bgt, x, int2string(va2), nextOrder.result, 0));
                }
                else if (va2 == FAIL && va1 != FAIL) {
                    MIPS.emplace_back(mips(blt, y, int2string(va1), nextOrder.result, 0));
                }
                else {
                    MIPS.emplace_back(mips(bgt, x, y, nextOrder.result, 0));
                }
            }
            intermediateId++;
        }
        else if (sequence.op == GEQ){
            string x = "$t0";
            string y = "$t1";
            getValue(sequence.x, x, va1, false);
            getValue(sequence.y, y, va2, false);

            intermediateCode nextOrder = IntermediateCode[intermediateId + 1];
            if (nextOrder.op == BZ) {
                if (va1 != FAIL && va2 != FAIL) {
                    if (va1 < va2) {
                        MIPS.emplace_back(mips(j, nextOrder.result, "", "", 0));
                    }
                }
                else if (va1 == FAIL && va2 != FAIL) {
                    MIPS.emplace_back(mips(blt, x, int2string(va2), nextOrder.result, 0));
                }
                else if (va2 == FAIL && va1 != FAIL) {
                    MIPS.emplace_back(mips(bgt, y, int2string(va1), nextOrder.result, 0));
                }
                else {
                    MIPS.emplace_back(mips(blt, x, y, nextOrder.result, 0));
                }
            }
            else if (nextOrder.op == BNZ) {
                if (va1 != FAIL && va2 != FAIL) {
                    if (va1 <= va2) {
                        MIPS.emplace_back(mips(j, nextOrder.result, "", "", 0));
                    }
                }
                else if (va1 == FAIL && va2 != FAIL) {
                    MIPS.emplace_back(mips(bge, x, int2string(va2), nextOrder.result, 0));
                }
                else if (va2 == FAIL && va1 != FAIL) {
                    MIPS.emplace_back(mips(ble, y, int2string(va1), nextOrder.result, 0));
                }
                else {
                    MIPS.emplace_back(mips(bge, x, y, nextOrder.result, 0));
                }
            }
            intermediateId++;
        }
        else if (sequence.op == EQL){
            string x = "$t0";
            string y = "$t1";
            getValue(sequence.x, x, va1, false);
            getValue(sequence.y, y, va2, false);

            intermediateCode nextOrder = IntermediateCode[intermediateId + 1];
            if (nextOrder.op == BZ) {
                if (va1 != FAIL && va2 != FAIL) {
                    if (va1 != va2) {
                        MIPS.emplace_back(mips(j, nextOrder.result, "", "", 0));
                    }
                }
                else if (va1 == FAIL && va2 != FAIL) {
                    MIPS.emplace_back(mips(li, y, "", "", va2));
                    MIPS.emplace_back(mips(bne, x, y, nextOrder.result, 0));
                }
                else if (va2 == FAIL && va1 != FAIL) {
                    MIPS.emplace_back(mips(li, x, "", "", va1));
                    MIPS.emplace_back(mips(bne, x, y, nextOrder.result, 0));
                }
                else {
                    MIPS.emplace_back(mips(bne, x, y, nextOrder.result, 0));
                }
            }
            else if (nextOrder.op == BNZ) {
                if (va1 != FAIL && va2 != FAIL) {
                    if (va1 == va2) {
                        MIPS.emplace_back(mips(j, nextOrder.result, "", "", 0));
                    }
                }
                else if (va1 == FAIL && va2 != FAIL) {
                    MIPS.emplace_back(mips(li, y, "", "", va2));
                    MIPS.emplace_back(mips(beq, x, y, nextOrder.result, 0));
                }
                else if (va2 == FAIL && va1 != FAIL) {
                    MIPS.emplace_back(mips(li, x, "", "", va1));
                    MIPS.emplace_back(mips(beq, y, x, nextOrder.result, 0));
                }
                else {
                    MIPS.emplace_back(mips(beq, x, y, nextOrder.result, 0));
                }
            }
            intermediateId++;
        }
        else if (sequence.op == NEQ){
            string x = "$t0";
            string y = "$t1";
            getValue(sequence.x, x, va1, false);
            getValue(sequence.y, y, va2, false);

            intermediateCode nextOrder = IntermediateCode[intermediateId + 1];
            if (nextOrder.op == BZ) {
                if (va1 != FAIL && va2 != FAIL) {
                    if (va1 == va2) {
                        MIPS.emplace_back(mips(j, nextOrder.result, "", "", 0));
                    }
                }
                else if (va1 == FAIL && va2 != FAIL) {
                    MIPS.emplace_back(mips(li, y, "", "", va2));
                    MIPS.emplace_back(mips(beq, x, y, nextOrder.result, 0));
                }
                else if (va2 == FAIL && va1 != FAIL) {
                    MIPS.emplace_back(mips(li, x, "", "", va1));
                    MIPS.emplace_back(mips(beq, x, y, nextOrder.result, 0));
                }
                else {
                    MIPS.emplace_back(mips(beq, x, y, nextOrder.result, 0));
                }
            }
            else if (nextOrder.op == BNZ) {
                if (va1 != FAIL && va2 != FAIL) {
                    if (va1 != va2) {
                        MIPS.emplace_back(mips(j, nextOrder.result, "", "", 0));
                    }
                }
                else if (va1 == FAIL && va2 != FAIL) {
                    MIPS.emplace_back(mips(li, y, "", "", va2));
                    MIPS.emplace_back(mips(bne, x, y, nextOrder.result, 0));
                }
                else if (va2 == FAIL && va1 != FAIL) {
                    MIPS.emplace_back(mips(li, x, "", "", va1));
                    MIPS.emplace_back(mips(bne, y, x, nextOrder.result, 0));
                }
                else {
                    MIPS.emplace_back(mips(bne, x, y, nextOrder.result, 0));
                }
            }
            intermediateId++;
        }
        else if (sequence.op == GOTO){
            MIPS.emplace_back(mips(j, sequence.result, "", "", 0));
        }
        else if (sequence.op == RET){
            int va;
            string v0 = "$v0";
            if (sequence.result != int2string(FAIL)) {
                getValue(sequence.result, v0, va, true);
                if (v0 != "$v0") {
                    MIPS.emplace_back(mips(moveop, "$v0", v0, "", 0));
                }
            }
            MIPS.emplace_back(mips(jr, "$ra", "", "", 0));
        }
        else if (sequence.op == PARA){
            string sfind = findUseAllReg(sequence.result);
            if (symbolTable.find(sequence.result) != symbolTable.end()) {
                SymbolUnit sym = symbolTable[sequence.result];
                if (sfind != " ") {
                    MIPS.emplace_back(mips(lw, sfind, "$sp", "", 4 * sym.addr));
                }
            }
        }
        else if (sequence.op == FUNC){
            SymbolUnit sym = globalSymTable[sequence.result];
            int length = sym.length;
            if (haveReturnValue) {
                MIPS.emplace_back(mips(jr, "$ra", "", "", 0));
            }
            MIPS.emplace_back(mips(label, sequence.result, "", "", 0));
            if (sequence.result == "main") {
                MIPS.emplace_back(mips(addi, "$sp", "$sp", "", -4 * length - 8));
            }
            functionId = sequence.result;
            symbolTable = tempT[functionId];
            for (int i = 0; i <= 7; i++) {
                gReg[i] = " ";
            }
            updateRegisters(intermediateId+1);
            haveReturnValue = true;
        }
        else if (sequence.op == CASE){
            choices.emplace_back(sequence);
        }
        else if (sequence.op == SWITCH){
            string variable = sequence.result;
            string switchVar = "$t0";
            getValue(variable, switchVar, va, false);
            for (int i = 0; i < choices.size(); i++) {
                intermediateCode caseSym = choices[i];
                if (va == FAIL) {
                    MIPS.emplace_back(mips(beq, switchVar, caseSym.x, caseSym.result, 0));
                }
                else {
                    if (string2int(caseSym.x) == va) {
                        MIPS.emplace_back(mips(j, caseSym.result, "","" , 0));
                    }
                }
            }
            choices.clear();
        }
        else if (sequence.op == EXIT){
            MIPS.emplace_back(mips(li, "$v0", "", "", 10));
            MIPS.emplace_back(mips(syscall, "", "", "", 0));
        }
    }
	MIPS.emplace_back(mips(li, "$v0", "", "", 10));
	MIPS.emplace_back(mips(syscall, "", "", "", 0));

}

void outputIntermediateCode() {
	ofstream intermediateCodeOutputFile;
	intermediateCodeOutputFile.open("�м����?.txt");
	for (int i = 0; i < IntermediateCode.size(); i++) {
		intermediateCode sequence = IntermediateCode[i];
		if(sequence.op==PLUS) {
            intermediateCodeOutputFile << sequence.result << " " << sequence.x << " + " << sequence.y << endl;
        }
        if(sequence.op==MINUS) {
            intermediateCodeOutputFile << sequence.result << " " << sequence.x << " - " << sequence.y << endl;
        }
        else if(sequence.op==MULT) {
            intermediateCodeOutputFile << sequence.result << " " << sequence.x << " * " << sequence.y << endl;
        }
        else if(sequence.op==DIV) {
            intermediateCodeOutputFile << sequence.result << " " << sequence.x << " / " << sequence.y << endl;
        }
        else if(sequence.op==LSS) {
            intermediateCodeOutputFile << sequence.x << " < " << sequence.y << endl;
        }
        else if(sequence.op==LEQ) {
            intermediateCodeOutputFile << sequence.x << " <= " << sequence.y << endl;
        }
        else if(sequence.op==GRE) {
            intermediateCodeOutputFile << sequence.x << " > " << sequence.y << endl;
        }
        else if(sequence.op==GEQ) {
            intermediateCodeOutputFile << sequence.x << " >= " << sequence.y << endl;
        }
        else if(sequence.op==EQL) {
            intermediateCodeOutputFile << sequence.x << " == " << sequence.y << endl;
        }
        else if(sequence.op==NEQ) {
            intermediateCodeOutputFile << sequence.x << " != " << sequence.y << endl;
        }
        else if(sequence.op==BNZ) {
            intermediateCodeOutputFile << "BNZ " << sequence.result << endl;
        }
        else if(sequence.op==BZ) {
            intermediateCodeOutputFile << "BZ " << sequence.result << endl;
        }
        else if(sequence.op==ASSIGN) {
            intermediateCodeOutputFile << sequence.result << " = " << sequence.x << endl;
        }
        else if(sequence.op==GOTO) {
            intermediateCodeOutputFile << "GOTO " << sequence.result << endl;
        }
        else if(sequence.op==SCANF) {
            intermediateCodeOutputFile << "SCANF " << sequence.result << endl;
        }
        else if(sequence.op==PRINTF) {
            intermediateCodeOutputFile << "PRINTF " << sequence.result << endl;
        }
        else if(sequence.op==LABEL) {
            intermediateCodeOutputFile << "LABEL " << sequence.result << endl;
        }
        else if(sequence.op==FUNC) {
            intermediateCodeOutputFile << "FUNC " << sequence.x << " " << sequence.result << "()" << endl;
        }
        else if(sequence.op==PARA) {
            intermediateCodeOutputFile << "PARA " << sequence.x << " " << sequence.result << endl;
        }
        else if(sequence.op==GETARRAY) {
            intermediateCodeOutputFile << sequence.result << " = " << sequence.x << "[" << sequence.y << "]" << endl;
        }
        else if(sequence.op==PUTARRAY) {
            intermediateCodeOutputFile << sequence.result << "[" << sequence.x << "]" << " = " << sequence.y << endl;
        }
        else if(sequence.op==CALL) {
            intermediateCodeOutputFile << "CALL " << sequence.result << endl;
        }
        else if(sequence.op==USE) {
            intermediateCodeOutputFile << "PUSH " << sequence.result << endl;
        }
        else if(sequence.op==RETVALUE) {
            intermediateCodeOutputFile << "RET " << sequence.result << endl;
        }
        else if(sequence.op==SWITCH) {
            intermediateCodeOutputFile << "SWITCH " << sequence.result << endl;
        }
        else if(sequence.op==CASE) {
            intermediateCodeOutputFile << "CASE " << sequence.result << " = " << sequence.x << endl;
        }


	}
}

