#include <stdio.h>
#include <stdlib.h>

int get_byte(int src, int pos);
void print_byte(int src);

int main(){

	char input[1491]; // max word, instr char lines
	int instruction;

	while(fgets(input, sizeof(input), stdin) != NULL){
		instruction = atoi(input);
		printf("Instruction: %d\n", instruction);
	}

	int tmp;
	tmp = get_byte(tmp, tmp);
	print_byte(tmp);

	return 0;
}

int get_byte(int src, int pos){
	
	int full = 2147483647;

	return full;
}

void print_byte(int src){

	int i, bit;

	for(i = 31; i >= 0; ++i){

		bit = 
		printf("%d", src & 1);
		src = src >> 1;
		printf("(%d)", src);
		
		if(i+1 % 4 == 0 && i != 31)
			printf(" ");
		else if(i == 31)
			printf("\n");
	}
}
