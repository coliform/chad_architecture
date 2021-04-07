/*
*
* CHAD simulator
*
*/

#include <chad_sim.h>

int pc, instruction_count;
instruction instructions[MAX_SIZE_PC];


int main(int argc, char *argv[]) {
	// %s <imemin.txt> <dmemin.txt> <diskin.txt> <irq2in.txt>
	int i;
	
	printf("\n\n\n\n");
	if (argc < 5) throw_error(ERROR_PARAMETERS_SIM, argv[0]);
	
	
	for (i=0; i<MAX_SIZE_PC; i++) instructions[i]
	
	
	printf("\n\n\n\n");
	return 0;
}
