#include <stdio.h>
#include <stdlib.h>

int get_byte(int src, int pos);
void print_byte(int src);

int main(){

	char input[1491]; // max word, instr char lines
	int instruction;

	while(fgets(input, sizeof(input), stdin) != NULL){
		instruction = atoi(input);
		//printf("Instruction: %d\n", instruction);
		print_byte(instruction);
		printf("\n");
	}

	int tmp = get_byte(15, 1);
	printf("\n");
	print_byte(tmp);

	return 0;
}

int get_byte(int src, int pos){	return (src >> (4*(pos-1))) & 15; }

void print_byte(int src){
	for(int i = 31; i >= 0; --i){
		printf("%d", (src >> i) & 1);
		if(i%4==0 && i != 31 && i != 0)
			printf(" ");
	}
}
