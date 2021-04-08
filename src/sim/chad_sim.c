/*
*
* CHAD simulator
*
*/

#include <chad_sim.h>


// program metadata
instruction instructions[MAX_SIZE_PC];
int instruction_count;

int pc, cycles;

// IO registers start
bool irq0enable, irq1enable, irq2enable;
bool irq0status, irq1status, irq2status;
uint12 irqhandler;
uint12 irqreturn;
uint32 clks;
uint32 leds;
uint32 display;
bool timerenable;
uint32 timercurrent;
uint32 timermax;
uint2 diskcmd;
uint7 disksector;
uint12 diskbuffer;
bool diskstatus;
// reserved
// reserved
uint16 monitoraddr;
uint8 monitordata;
bool monitorcmd;
//IO registers end

uint8 disk[SIZE_HDD_SECTORS_H][SIZE_HDD_SECTORS_W];

void parse_instruction(char* line, int count) {
	llu instruction_binary;
	strip(line);
	printf("Line %s and ", line);
	instruction_binary = hex_to_unsigned_long_long(line, &instruction_binary);
	printf("\n");
	//printf("%llu\n", instruction_binary);
}

void read_instructions(char** lines) {
	int i, counted;
	for (i=0, counted=0; lines[i]!=0; i++) {
		if (strlen(lines[i])==0) continue;
		parse_instruction(lines[i], counted);
		counted++;
	}
	instruction_count = counted;
}


int main(int argc, char *argv[]) {
	// %s <imemin.txt> <dmemin.txt> <diskin.txt> <irq2in.txt>
	int i;
	
	printf("\n\n\n\n");
	if (argc < 5) throw_error(ERROR_PARAMETERS_SIM, argv[0]);
	
	char** instructions_text;
	instruction_count = get_file_lines(argv[1], &instructions_text);
	read_instructions(instructions_text);
	printf("done\n");

	free_lines(instructions_text);
	printf("\n\n\n\n");
	return 0;
}
