#include <string>
#include<vector>
using namespace std;

#ifndef Mips
#define Mips


void generateMIPS();
void outputMIPS();
void outputIntermediateCode();
enum  mipsOp{
    sll,
    sub,
    bgez,
    bltz,
    dive,
    moveop,
    mflo,
    la,
    label,
    textSet,
    lw,
    sw,
    mult,
    jal,
    blez,
    add,
    j,
    subi,
    mul,
    bne,
    syscall,
    bge,
    beq,
    li,
    strSet,
    jr,
    ble,
    bgtz,
    mfhi,
    globalSet,
    blt,
    dataSet,
    bgt,
    addi
};
class mips {
public:
    string y;
    mips(mipsOp inOp,string inResult, string inX,string inY,int inImm):op(inOp),result(inResult),x(inX),y(inY),imm(inImm){ }
    mipsOp op;
    string x;
    int imm;
    string result;
};


#endif