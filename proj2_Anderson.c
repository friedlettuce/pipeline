#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
	int instruction, immed, rs, rt, rd, branchTarg;
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
	int instruction;
	int aluResult;
	int writeData, writeReg;
}exmem;
typedef struct{
	int instruction;
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

int get_byte(int src, int pos);
void print_byte(int src, int num);

void print(int cycle, int pc, int *dataMem, int *regFile);

void init_instr(instruction* instr);
void print_state(state* st);

int get_code(int instruction, char* type);
char get_type(int instruction);

char* get_name(int op, int funct);

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

		int nullop = strcmp("NULL", get_name(get_code(
			instructions[i-1], "op"), get_code(
			instructions[i-1], "funct")));
		int noop = strcmp("noop", get_name(get_code(
			instructions[i-1], "op"), get_code(
			instructions[i-1], "funct")));

		if(nullop != 0 && dataSeg == -1 && noop != 0){
			set_state(&ifid, instructions[i-1]);
			print_state(&ifid);
			printf("\n");
		}
		else if(noop == 0){
			++dataSeg;
		}
		else{
			dataMem[dataSeg++] = instructions[i-1];
		}
	}
	instructions[i] = -1;

	print(1, 0, dataMem, regFile);
	return 0;
}

int get_byte(int src, int pos){	return (src >> (4*(pos-1))) & 15; }

/*
void print_byte(int src, int num){
	for(int i = 31 - (31 - (num-1)); i >= 0; --i){
		printf("%d", (src >> i) & 1);
		if(i%4==0 && i != 31 && i != 0)
			printf(" ");
	}
}
*/

void print(int cycle, int pc, int *dataMem, int *regFile){

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
	printf("\t\tInstruction: NULL\n");
	printf("\t\tPCPlus4: 0\n");

	printf("\tID/EX:\n");
	printf("\t\tInstruction: NA\n");
	printf("\t\tPCPlus4: NA\n");
	printf("\t\tbranchTarget: NA\n");
	printf("\t\treadData1: NA\n");
	printf("\t\treadData2: NA\n");
	printf("\t\timmed: NA\n");
	printf("\t\trs: NA\n");
	printf("\t\trt: NA\n");
	printf("\t\trd: NA\n");

	printf("\tEX/MEM\n");
	printf("\t\tInstruction: NA\n");
	printf("\t\taluResult: NA\n");
	printf("\t\twriteDataReg: NA\n");
	printf("\t\twriteReg: NA\n");

	printf("\tMEM/WB\n");
	printf("\t\tInstruction: NA\n");
	printf("\t\twriteDataMem: NA\n");
	printf("\t\twriteDataALU: NA\n");
	printf("\t\twriteReg: NA\n");

	printf("********************\n");
}

void init_instr(instruction* instr){
	instr->instr = instr->immed = 0;
	instr->type = 0;
	instr->rs = instr->rt = instr->rd = instr->branchTarget = 0;
}

void print_state(state* st){
	printf("Function: %s, ", get_name(st->instruction,
				get_code(st->instruction, "funct")));
	printf("Rs: %d, Rt: %d, Rd: %d\n", st->rs, st->rt, st->rd);
	printf("Immed: %d, Branch Target: %d\n", st->immed, st->branchTarget);
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

char* get_name(int op, int funct){
	switch(op){
		case 32:return "add";
		case 34:return "sub";
		case 35:return "lw";
		case 43:return "sw";
		case 12:return "andi";
		case 13:return "ori";
		case 5:	return "bne";
		case 1:	return "halt";
		case 0:	if(funct == 0)
				return "noop";
			return "sll";
	}
	return "NULL";
}

char* get_reg(char regNum){
	if(regNum == 0)
		return "$0";
	if(regNum < 16)
		return "$t" + regNum;
	return "$s" + regNum;
}

int get_reg(char* regName){
	if(strcmp(regName, "$0") == 0)
		return 0;
} 
