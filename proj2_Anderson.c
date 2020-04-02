#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
	int code, immed, rs, rt, rd, branchTarg;
	char type;
}instruction;

typedef struct{
	instruction instr;
	int pc4;
}ifid;
typedef struct{
	instruction instr;
	int pc4, branchTarg, rs, rt, rd;
	int readData1, readData2, immed;
}idex;
typedef struct{
	instruction instr;
	int aluResult;
	int writeData, writeReg;
}exmem;
typedef struct{
	instruction instr;
	int wMem, wALU, wReg;
}memwb;

typedef struct{
	ifid regIfid;
	idex regIdex;
	exmem regExmem;
	memwb regMemwb;
}state;

typedef struct{
	int pc, branchTarg;
	state st;
}bPredictor;

void print(state* st, int cycle, int pc, int *dataMem, int *regFile);

void init_instr(instruction* instr);
void init_state(state* st);

int get_code(int instruction, char* type);
char get_type(int instruction);

char* get_name(int instr);

char* get_regName(char regNum);
int get_regNum(char* regName);

int main(){

	int instructions[100], dataMem[32], regFile[32], bPredictor[100];
	state preCycle, postCycle;

	int i, dataSeg = -1;
	for(i = 0; i < 100; ++i){ instructions[i] = 0; }
	for(i = 0; i < 32; ++i){ dataMem[i] = 0; regFile[i] = 0; }
	
	char input[1491]; // max word, instr char lines

	i = 0;
	while(fgets(input, sizeof(input), stdin) != NULL){
		
		instructions[i++] = atoi(input);
		/*
		int nullop = strcmp("NULL", get_name(atoi(input)));
		int noop = strcmp("noop", get_name(atoi(input)));
		}
		
		if(dataSeg == -1 && nullop != 0){

		}
		else if(noop == 0){
			++dataSeg;
		}
		else{
			dataMem[dataSeg++] = instructions[i-1];
		}
		*/
	}
	instructions[i] = -1;

	print(&preCycle, 1, 0, dataMem, regFile);
	return 0;
}

void print(state* st, int cycle, int pc, int *dataMem, int *regFile){

	int i;

	printf("********************\n");
	printf("State at the beginning of cycle %d\n", cycle);
	printf("\tPC = %d\n", pc);
	
	printf("\tData Memory:\n");
	for(i = 0; i < 16; ++i){
		printf("\t\tdataMem[%d] = %d\tdataMem[%d] = %d\n",
				i, dataMem[i], i+16, dataMem[i+16]);
	}
	printf("\tRegisters:\n");
	for(i = 0; i < 32; i+=2){
		printf("\t\tregFile[%d] = %d\tregFile[%d] = %d\n",
				i, regFile[i], i+1, regFile[i+1]);
	}

	printf("\tIF/ID:\n");
	printf("\t\tInstruction: %s\n", get_name(st->regIfid.instr.code));
	printf("\t\tPCPlus4: %d\n", st->regIfid.pc4);

	printf("\tID/EX:\n");
	printf("\t\tInstruction: %s\n", get_name(st->regIdex.instr.code));
	printf("\t\tPCPlus4: %d\n", st->regIdex.pc4);
	printf("\t\tbranchTarget: %d\n", st->regIdex.branchTarg);
	printf("\t\treadData1: %d\n", st->regIdex.readData1);
	printf("\t\treadData2: %d\n", st->regIdex.readData2);
	printf("\t\timmed: %d\n", st->regIdex.immed);
	printf("\t\trs: %d\n", st->regIdex.rs);
	printf("\t\trt: %d\n", st->regIdex.rt);
	printf("\t\trd: %d\n", st->regIdex.rd);

	printf("\tEX/MEM\n");
	printf("\t\tInstruction: %s\n", get_name(st->regExmem.instr.code));
	printf("\t\taluResult: %d\n", st->regExmem.aluResult);
	printf("\t\twriteDataReg: %d\n", st->regExmem.writeData);
	printf("\t\twriteReg: %d\n", st->regExmem.writeReg);

	printf("\tMEM/WB\n");
	printf("\t\tInstruction: %s\n", get_name(st->regMemwb.instr.code));
	printf("\t\twriteDataMem: %d\n", st->regMemwb.wMem);
	printf("\t\twriteDataALU: %d\n", st->regMemwb.wALU);
	printf("\t\twriteReg: %d\n", st->regMemwb.wReg);

	printf("********************\n");
}

void init_instr(instruction* instr){
	instr->code = instr->immed = 0;
	instr->type = 0;
	instr->rs = instr->rt = instr->rd = instr->branchTarg = 0;
}
void init_state(state* st){
	
	init_instr(&st->regIfid.instr);
	st->regIfid.pc4 = 0;

	init_instr(&st->regIdex.instr);
	st->regIdex.pc4 = st->regIdex.branchTarg = st->regIdex.rs = 0;
	st->regIdex.rt = st->regIdex.rd = st->regIdex.readData1 = 0;
	st->regIdex.readData2 = st->regIdex.immed = 0;
	
	init_instr(&st->regExmem.instr);
	st->regExmem.aluResult = st->regExmem.writeData = 0;
	st->regExmem.writeReg = 0;
	
	init_instr(&st->regMemwb.instr);
	st->regMemwb.wMem = st->regMemwb.wALU = st->regMemwb.wReg = 0;
}

int get_code(int instruction, char *type){
	if(strcmp(type, "op") == 0)
		return (instruction >> 26) & 63;
	else if(strcmp(type, "rs") == 0)
		return (instruction >> 21) & 31;
	else if(strcmp(type, "rt") == 0)
		return (instruction >> 16) & 31;
	else if(strcmp(type, "rd") == 0)
		return (instruction >> 11) & 31;
	else if(strcmp(type, "shamt") == 0)
		return (instruction >> 6) & 31;
	else if(strcmp(type, "funct") == 0)
		return instruction & 63;
	else if(strcmp(type, "immed") == 0)
		return instruction & 65535;
	else if(strcmp(type, "targadd") == 0)
		return instruction & 67108863;
	else
		return -1;
}

char get_type(int instruction){
	if(get_code(instruction, "op") == 0)
		return 'r';
	else if(get_code(instruction, "op") == 2)
		return 'j';
	else
		return 'i';
}

char* get_name(int instr){
	switch(get_code(instr, "op")){
		case 32:return "add";
		case 34:return "sub";
		case 35:return "lw";
		case 43:return "sw";
		case 12:return "andi";
		case 13:return "ori";
		case 5:	return "bne";
		case 1:	return "halt";
		case 0:	if(get_code(instr, "funct") == 0)
				return "noop";
			return "sll";
	}
	return "NULL";
}

char* get_regName(char regNum){
	if(regNum == 0)
		return "$0";
	if(regNum < 16)
		return "$t" + regNum;
	return "$s" + regNum;
}

int get_regNum(char* regName){
	if(strcmp(regName, "$0") == 0)
		return 0;
	
	int reg = 0;

	if(strncmp(regName, "$t", 2) == 0)
		reg += 8;
	else if(strncmp(regName, "$s", 2) == 0)
		reg += 16;

	return reg += (int)(regName[2]);
} 
