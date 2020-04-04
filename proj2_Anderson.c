#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
	int op, immed, rs, rt, rd, branchTarg;
	int shamt, funct;
	char type;
}instruction;

typedef struct{
	instruction instr;
	int pc4;
}ifid;
typedef struct{
	instruction instr;
	int pc4, rs, rt, rd, branchTarg;
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

char* get_name(int op, int funct, int shamt);

char* get_regName(char regNum);
int get_regNum(char* regName);

void print_instruction(instruction* instr);

void copy_state(state* dest, state* input);
void copy_instr(instruction* dest, instruction* input);

void push_ifid(ifid* regIfid, int pc, instruction* instr);
void push_idex(ifid* regIfid, idex* regIdex, int* regFile);
void push_exmem(idex* regIdex, exmem* regExmem, int* regFile);
int push_memwb(exmem* regExmem, memwb* regMemwb);

int main(){

	instruction instructions[100];
	int dataMem[32], regFile[32];
	bPredictor bPredictors[100];
	state preCycle, postCycle;

	init_state(&preCycle);
	init_state(&postCycle);

	int i, dataSeg = -1;
	for(i = 0; i < 100; ++i){ init_instr(&instructions[i]); }
	for(i = 0; i < 32; ++i){ dataMem[i] = 0; regFile[i] = 0; }
	
	char input[1491]; // max word, instr char lines

	i = 0;
	while(fgets(input, sizeof(input), stdin) != NULL){
	
		if(input[0] == '\n') continue;
	
		int halt = strcmp("halt", get_name(get_code(atoi(input), "op"),
			get_code(atoi(input), "funct"), get_code(atoi(input), "shamt")));

		if(halt == 0){ dataSeg = 0; }
		else if(dataSeg > -1){
			dataMem[dataSeg++] = atoi(input);
			continue;
		}
	
		int code = atoi(input);
		instructions[i].type = get_type(code);
		instructions[i].op = get_code(code, "op");

		if(instructions[i].type == 'r' || instructions[i].type == 'i'){
			
			instructions[i].rs = get_code(code, "rs");
			instructions[i].rt = get_code(code, "rt");

			if(instructions[i].type == 'r'){
				instructions[i].rd = get_code(code, "rd");
				instructions[i].shamt = get_code(code, "shamt");
			}
		}
		instructions[i].immed = get_code(code, "immed");
		instructions[i].branchTarg = get_code(code, "targadd");
		instructions[i].funct = get_code(code, "funct");

		print_instruction(&instructions[i]);
		++i;
	}

	int dataStart = i * 4;
	printf("DS %d\n", dataStart);

	int j = 0;
	for(i = 0; i < 100; ++i){
		if(strcmp("bne", get_name(instructions[i].op,
		instructions[i].funct, instructions[i].shamt)) == 0){
			bPredictors[j++].pc = i * 4;
			bPredictors[j++].branchTarg = get_code(atoi(input),
				"targadd");
		}
	}

	int pc = 0;
	int end = -1;
	int instr_count = 0;
	int ifid, idex, exmem, memwb;
	ifid = idex = exmem = memwb = 1;

	do{
		end = strcmp("halt", get_name(preCycle.regMemwb.instr.op,
		preCycle.regMemwb.instr.funct, preCycle.regMemwb.instr.shamt));
		
		print(&preCycle, pc / 4, pc, dataMem, regFile);
		

		if(ifid != 0){
			ifid = strcmp("halt", get_name(
			instructions[instr_count].op,
			instructions[instr_count].funct,
			instructions[instr_count].shamt));
		
			push_ifid(&postCycle.regIfid, pc,
				&instructions[instr_count++]);
		}
		else{
			instruction tmp;
			init_instr(&tmp);
			push_ifid(&postCycle.regIfid, pc, &tmp);
		}

		if(idex == 1)
			push_idex(&preCycle.regIfid, &postCycle.regIdex, regFile);

		if(exmem == 1){
			// Update data memory here
			push_exmem(&preCycle.regIdex, &postCycle.regExmem, regFile);
		}
		if(memwb == 1){
			regFile[postCycle.regMemwb.wReg] = postCycle.regMemwb.wALU;
			push_memwb(&preCycle.regExmem, &postCycle.regMemwb);
		}
		copy_state(&preCycle, &postCycle);

		pc += 4;
	}while(end != 0);

	printf("********************\n");
	printf("Total number of cycles executed: %d\n", pc / 4);
	printf("Total number of stalls: %d\n", 0);
	printf("Total number of branches: %d\n", 0);
	printf("Total number of mispredicted branches: %d\n", 0);

	return 0;
}

void print(state* st, int cycle, int pc, int *dataMem, int *regFile){

	int i;

	printf("********************\n");
	printf("State at the beginning of cycle %d\n", cycle+1);
	printf("\tPC = %d\n", pc);
	
	printf("\tData Memory:\n");
	for(i = 0; i < 16; ++i){
		printf("\t\tdataMem[%d] = %d\tdataMem[%d] = %d\n",
				i, dataMem[i], i+16, dataMem[i+16]);
	}
	printf("\tRegisters:\n");
	for(i = 0; i < 16; ++i){
		printf("\t\tregFile[%d] = %d\tregFile[%d] = %d\n",
				i, regFile[i], i+16, regFile[i+16]);
	}

	printf("\tIF/ID:\n");
	printf("\t\tInstruction: ");
	print_instruction(&st->regIfid.instr);
	printf("\t\tPCPlus4: %d\n", st->regIfid.pc4);

	printf("\tID/EX:\n");
	printf("\t\tInstruction: ");
	print_instruction(&st->regIdex.instr);
	printf("\t\tPCPlus4: %d\n", st->regIdex.pc4);
	printf("\t\tbranchTarget: %d\n", st->regIdex.branchTarg);
	printf("\t\treadData1: %d\n", st->regIdex.readData1);
	printf("\t\treadData2: %d\n", st->regIdex.readData2);
	printf("\t\timmed: %d\n", st->regIdex.immed);
	printf("\t\trs: %s\n", get_regName(st->regIdex.rs));
	printf("\t\trt: %s\n", get_regName(st->regIdex.rt));
	printf("\t\trd: %s\n", get_regName(st->regIdex.rd));

	printf("\tEX/MEM\n");
	printf("\t\tInstruction: ");
	print_instruction(&st->regExmem.instr);
	printf("\t\taluResult: %d\n", st->regExmem.aluResult);
	printf("\t\twriteDataReg: %d\n", st->regExmem.writeData);
	printf("\t\twriteReg: %s\n", get_regName(st->regExmem.writeReg));

	printf("\tMEM/WB\n");
	printf("\t\tInstruction: ");
	print_instruction(&st->regMemwb.instr);
	printf("\t\twriteDataMem: %d\n", st->regMemwb.wMem);
	printf("\t\twriteDataALU: %d\n", st->regMemwb.wALU);
	printf("\t\twriteReg: %s\n", get_regName(st->regMemwb.wReg));
}

void init_instr(instruction* instr){
	instr->op = instr->shamt = instr->funct = 0;
	instr->immed = instr->type = 0;
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
	if(instruction == 1)
		return 0;
	else if(get_code(instruction, "op") == 0)
		return 'r';
	else if(get_code(instruction, "op") == 2)
		return 'j';
	else if(get_code(instruction, "op") > 1)
		return 'i';
	else
		return 0;
}

char* get_name(int op, int funct, int shamt){
	if(op == 0){
		switch(funct){
			case 32:return "add";
			case 34:return "sub";
			case 1: return "halt";
			case 0: if(shamt == 0)
					return "NOOP";
				return "sll";
		}
	}

	switch(op){
		case 35:return "lw";
		case 43:return "sw";
		case 12:return "andi";
		case 13:return "ori";
		case 5:	return "bne";
		case 1:	return "halt";
	}
	return "NULL";
}

char* get_regName(char regNum){
	if(regNum == 0){
		return "0";
	}
	
	switch(regNum){
		case 8:	return "t0";
			break;
		case 9:	return "t1";
			break;
		case 10:return "t2";
			break;
		case 11:return "t3";
			break;
		case 12:return "t4";
			break;
		case 13:return "t5";
			break;
		case 14:return "t6";
			break;
		case 15:return "t7";
			break;
		case 16:return "s0";
			break;
		case 17:return "s1";
			break;
		case 18:return "s2";
			break;
		case 19:return "s3";
			break;
		case 20:return "s4";
			break;
		case 21:return "s5";
			break;
		case 22:return "s6";
			break;
		case 23:return "s7";
			break;
	}
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

void print_instruction(instruction* instr){

	printf("%s", get_name(instr->op, instr->funct, instr->shamt));

	if(instr->type == 'r'){
		if(strcmp("sll", get_name(instr->op, instr->funct,
		instr->shamt)) == 0){
			printf(" $%s,$%s,%d", get_regName(instr->rd),
				get_regName(instr->rt), instr->shamt);
		}
		else{
			printf(" $%s,$%s,$%s", get_regName(instr->rd),
			get_regName(instr->rs), get_regName(instr->rt));
		}
	}
	else if(instr->type == 'i'){
		if(instr->op == 35 || instr->op == 43){
			printf(" $%s,%d($%s)", get_regName(instr->rt),
				instr->immed, get_regName(instr->rs));
		}
		else if(instr->op == 5){
			printf(" $%s,$%s,%d", get_regName(instr->rs),
				get_regName(instr->rt), instr->immed);
		}
		else{
			printf(" $%s,$%s,%d", get_regName(instr->rt),
				get_regName(instr->rs), instr->immed);
		}
	}
	printf("\n");
}

void copy_ifid(ifid* dest, ifid* input){
	dest->instr = input->instr;
	dest->pc4 = input->pc4;
}
void copy_idex(idex* dest, idex* input){
	dest->instr = input->instr;
	dest->pc4 = input->pc4;
	dest->branchTarg = input->branchTarg;
	dest->rs = input->rs;
	dest->rt = input->rt;
	dest->rd = input->rd;
	dest->readData1 = input->readData1;
	dest->readData2 = input->readData2;
	dest->immed = input->immed;
}
void copy_exmem(exmem* dest, exmem* input){
	dest->instr = input->instr;
	dest->aluResult = input->aluResult;
	dest->writeData = input->writeData;
	dest->writeReg = input->writeReg;
}
void copy_memwb(memwb* dest, memwb* input){
	dest->instr = input->instr;
	dest->wMem = input->wMem;
	dest->wALU = input->wALU;
	dest->wReg = input->wReg;
}

void copy_state(state* dest, state* input){
	copy_ifid(&dest->regIfid, &input->regIfid);
	copy_idex(&dest->regIdex, &input->regIdex);
	copy_exmem(&dest->regExmem, &input->regExmem);
	copy_memwb(&dest->regMemwb, &input->regMemwb);	
}

void copy_instr(instruction* dest, instruction* input){
	dest->op = input->op;
	dest->immed = input->immed;
	dest->rs = input->rs;
	dest->rt = input->rt;
	dest->rd = input->rd;
	dest->branchTarg = input->branchTarg;
	dest->shamt = input->shamt;
	dest->funct = input->funct;
	dest->type = input->type;
}

void push_ifid(ifid* regIfid, int pc, instruction* instr){
	copy_instr(&regIfid->instr, instr);
	regIfid->pc4 = pc + 4;
}

void push_idex(ifid* regIfid, idex* regIdex, int* regFile){
	
	copy_instr(&regIdex->instr, &regIfid->instr);
	
	regIdex->pc4 = regIfid->pc4;
	regIdex->readData1 = 0;
	regIdex->readData2 = 0;
	regIdex->rs = 0;
	regIdex->rt = 0;
	regIdex->rd = 0;
	
	if(regIdex->instr.type == 'r'){
		regIdex->rs = regIfid->instr.rs;
		regIdex->rt = regIfid->instr.rt;
		regIdex->rd = regIfid->instr.rd;
		regIdex->readData1 = regFile[regIdex->rs];
		regIdex->readData2 = regFile[regIdex->rt];
	}
	if(regIdex->instr.type == 'i'){
		regIdex->rs = regIfid->instr.rs;
		regIdex->rt = regIfid->instr.rt;
		regIdex->readData1 = regFile[regIdex->rs];
	}
	
	regIdex->immed = regIdex->instr.immed;
	regIdex->branchTarg = (regIdex->instr.immed * 4) + regIdex->pc4;
}

void push_exmem(idex* regIdex, exmem* regExmem, int* regFile){
	copy_instr(&regExmem->instr, &regIdex->instr);
	
	regExmem->aluResult = 0;
	regExmem->writeData = 0;
	regExmem->writeReg = 0;

	if(regExmem->instr.type == 'i'){
		regExmem->writeReg = regIdex->instr.rt;
		
		switch(regExmem->instr.op){ 
			case 43:
			case 35:regExmem->aluResult = regFile[regIdex->rs] +
				regExmem->instr.immed;
				break;
			case 12:regExmem->aluResult = regFile[regIdex->rs] &
				regExmem->instr.immed;
				break;
			case 13:regExmem->aluResult =
				regExmem->instr.immed|regExmem->instr.rt;
				break;
			case 5: 
				break;
		}
	}
	else if(regExmem->instr.type == 'r'){
		regExmem->writeData = regFile[regIdex->rt];
		regExmem->writeReg = regIdex->rd;

		switch(regExmem->instr.funct){
			case 32:regExmem->aluResult = regFile[regIdex->rs] +
				regFile[regIdex->rt];
				break;
			case 34:regExmem->aluResult = regFile[regIdex->rs] -
				regFile[regIdex->rt];
				break;
			case 0:	regExmem->aluResult = regFile[regIdex->rs] <<
				regExmem->instr.shamt;
				break;
		}
	}
	
}

int push_memwb(exmem* regExmem, memwb* regMemwb){
	copy_instr(&regMemwb->instr, &regExmem->instr);
	regMemwb->wMem = regExmem->writeData;
	regMemwb->wALU = regExmem->aluResult;
	regMemwb->wReg = regExmem->writeReg;
	return 0;
}
