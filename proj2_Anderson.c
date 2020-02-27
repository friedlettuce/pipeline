#include <stdio.h>
#include <stdlib.h>

typedef struct{
	int instruction, immed;
	char type;
	int rs, rt, rd, branchTarget;
}state;

int get_byte(int src, int pos);
void print_byte(int src, int num);

void print(int cycle, int pc, int *dataMem, int *regFile);

void init_state(state* st);

int get_op(int instruction);

int main(){

	
	int instruction[100], dataMem[32], regFile[32];
	state pipe1, pipe2, pipe3, pipe4;

	for(int i = 0; i < 100; ++i){ instruction[i] = 0; }
	for(int i = 0; i < 32; ++i){ dataMem[i] = i; regFile[i] = i; }

	char input[1491]; // max word, instr char lines

	int i = 0;
	while(fgets(input, sizeof(input), stdin) != NULL){
		instruction[i++] = atoi(input);
	}
	instruction[i] = -1;

	for(int i = 0; i < 100; ++i){
		if(instruction[i] == -1)
			break;
		print_byte(instruction[i]);
		printf("\top code: ");
		print_byte(get_op(instruction[i]));
		printf("\n");
	}

	print(1, 0, dataMem, regFile);

	return 0;
}

int get_byte(int src, int pos){	return (src >> (4*(pos-1))) & 15; }

void print_byte(int src, int num){
	for(int i = 31 - (31 - (num-1)); i >= 0; --i){
		printf("%d", (src >> i) & 1);
		if(i%4==0 && i != 31 && i != 0)
			printf(" ");
	}
}

void print(int cycle, int pc, int *dataMem, int *regFile){

	printf("********************\n");
	printf("State at the beginning of cycle %d\n", cycle);
	printf("\tPC = %d\n", pc);
	
	printf("\tData Memory:\n");
	for(int i = 0; i < 32; i+=2){
		printf("\t\tdataMem[%d] = %d\tdataMem[%d] = %d\n",
				i, dataMem[i], i+1, dataMem[i+1]);
	}
	printf("\tRegisters:\n");
	for(int i = 0; i < 32; i+=2){
		printf("\t\tregFile[%d] = %d\tregFile[%d] = %d\n",
				i, regFile[i], i+1, regFile[i+1]);
	}

	printf("\tIF/ID:\n");
	printf("\t\tInstruction: \n");
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

void init_state(state* st){
	st->instruction = st->immed = 0;
	st->type = 0;
	st->rs = st->rt = st->rd = st->branchTarget = 0;
}

int get_op(int instruction){
	return (instruction >> 26) & 63;
}


