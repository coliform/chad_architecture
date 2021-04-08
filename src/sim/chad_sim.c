/*
*
* CHAD simulator
*
*/

#include <chad_sim.h>


// program metadata
instruction instructions[MAX_SIZE_PC];
int instruction_count, sector_count;


int pc, cycles;
uint32 R[COUNT_REGISTERS];
uint32 IORegister[COUNT_IOREGISTERS];
uint32 IORegister_SIZE[COUNT_IOREGISTERS] = {
	0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000FFF, 0x00000FFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000001, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000003, 0x0000007F, 0x00000FFF, 0x00000001, 0x00000000, 0x00000000, 0x0000FFFF, 0x000000FF, 0x00000001
}
uint32 MEM[MAX_DMEM_ITEMS];
uint8 disk[SIZE_HDD_SECTORS_H][SIZE_HDD_SECTORS_W];

void parse_instruction(char* line, int index) {
	llu binary;
	strip(line);
	hex_to_unsigned_long_long(line, &binary);
	instructions[index].opcode 		= (binary>>40)&0xFF;
	instructions[index].rd			= (binary>>32)&0xF;
	instructions[index].rs			= (binary>>28)&0xF;
	instructions[index].rt			= (binary>>24)&0xF;
	instructions[index].immediate1	= (binary>>12)&0xFFF;
	instructions[index].immediate2	= (binary>>0 )&0xFFF;
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

void parse_disk_sector(char* line, int index) {
	// TODO
}

void read_disk(char** lines) {
	int i, counted;
	for (i=0, counted=0; lines[i]!=0; i++) {
		if (strlen(lines[i])==0) continue;
		parse_disk_sector(lines[i], counted);
		counted++;
	}
	sector_count = counted;
}

void read_dmemin(char** lines) {
	int i, counted;
	for (i=0, counted=0; lines[i]!=0; i++) {
		if (strlen(lines[i])==0) continue;
		hex_to_uint32(lines[i], &MEM[counted]);
		counted++;
	}
	instruction_count = counted;
}

void parse_irq2in_line(char* line, int index) {
	// TODO
}

void read_irq2in(char** lines) {
	int i, counted;
	for (i=0, counted=0; lines[i]!=0; i++) {
		if (strlen(lines[i])==0) continue;
		// TODO
		//parse_irq2in_line(lines[i], &MEM[counted]);
		counted++;
	}
}

// register_write:
// safely writes into a register, crashes if register is invalid
void register_write(int number, uint32 value) {
	if (number < 0 || number >= COUNT_REGISTERS) throw_error(ERROR_RUNTIME, "EXCEPTION IN REGISTER WRITE");
	if (number == 0 || number == 1 || number == 2) return;
	R[number] = value;
}

// register_read:
// safely reads from a register, crashes if register is invalid
uint32 register_read(int number) {
	if (number < 0 || number >= COUNT_REGISTERS) throw_error(ERROR_RUNTIME, "EXCEPTION IN REGISTER READ");
	return R[number];
}

void memory_write(int number, uint32 value) {
	if (number < 0 || number >= MAX_DMEM_ITEMS) throw_error(ERROR_RUNTIME, "EXCEPTION IN MEMORY WRITE");
	MEM[number] = value;
}

uint32 memory_read(int number) {
	if (number < 0 || number >= MAX_DMEM_ITEMS) throw_error(ERROR_RUNTIME, "EXCEPTION IN MEMORY READ");
	return MEM[number];
}

void ioregister_write(int number, uint32 value) {
	if (number < 0 || number >= COUNT_IOREGISTERS) throw_error(ERROR_RUNTIME, "EXCEPTION IN IO REGISTER WRITE");
	IORegister[number] = value & IORegister_SIZE[number];
}

uint32 ioregister_read(int number) {
	if (number < 0 || number >= COUNT_IOREGISTERS) throw_error(ERROR_RUNTIME, "EXCEPTION IN IO REGISTER READ");
	return IORegister[number];
}


/*
*
* *** OPCODES ***
* The functions below contain the opcodes
*
*/

void add(int rd, int rs, int rt) {
	register_write(rd, register_read(rs)+register_read(rt));
}


void sub(int rd, int rs, int rt) {
	register_write(rd, register_read(rs)-register_read(rt));
}

void and(int rd, int rs, int rt) {
	register_write(rd, register_read(rs)&register_read(rt));
}

void or(int rd, int rs, int rt) {
	register_write(rd, register_read(rs)|register_read(rt));
}

void sll(int rd, int rs, int rt) {
	register_write(rd, register_read(rs)<<register_read(rt));
}

void sra(int rd, int rs, int rt) {
	register_write(rd, (register_read(rs) & 0x80000000) | (register_read(rs) >> register_read(rt)));
}

void srl(int rd, int rs, int rt) {
	register_write(rd, register_read(rs)>>register_read(rt));
}

void beq(int rd, int rs, int rt) {
	if (register_read(rs)==register_read(rt)) pc = register_read(rd) & 0x00000FFF;
}

void bne(int rd, int rs, int rt) {
	if (register_read(rs)!=register_read(rt)) pc = register_read(rd) & 0x00000FFF;
}

void blt(int rd, int rs, int rt) {
	if (register_read(rs)<register_read(rt)) pc = register_read(rd) & 0x00000FFF;
}

void bgt(int rd, int rs, int rt) {
	if (register_read(rs)>register_read(rt)) pc = register_read(rd) & 0x00000FFF;
}

void ble(int rd, int rs, int rt) {
	if (register_read(rs)<=register_read(rt)) pc = register_read(rd) & 0x00000FFF;
}

void bge(int rd, int rs, int rt) {
	if (register_read(rs)>=register_read(rt)) pc = register_read(rd) & 0x00000FFF;
}

void jal(int rd, int rs, int rt) {
	register_write(ra, pc+1);
	pc = register_read(rd) & 0x00000FFF;
}

void lw(int rd, int rs, int rt) {
	register_write( rd, memory_read( ((int)register_read(rs))+((int)register_read(rt)) ) );
}

void sw(int rd, int rs, int rt) {
	memory_write( ((int)register_read(rs))+((int)register_read(rt)), register_read(rd) );
}

void reti(int rd, int rs, int rt) {
	pc = ioregister_read(irqreturn);
}

void in(int rd, int rs, int rt) {
	register_write(rd, ioregister_read(((int)register_read(rs))+((int)register_read(rt))));
}

void out(int rd, int rs, int rt) {
	ioregister_write(((int)register_read(rs))+((int)register_read(rt)),register_read(rd));
}

void halt(int rd, int rs, int rt) {
	exit(0);
}

void (*opcode_fn[COUNT_OPCODES])(int, int, int) = {
	add, sub, and, or, sll, sra, beq, bne, blt, bgt, ble, bge, jal, lw, sw, reti, in, out, halt
};

/*
*
* *** OPCODES END ***
*
*/


/* Perform current instruction:
* Performs the instruction at pc
*/
void perform_current_instruction() {
	instruction ins = instructions[pc];
	if (ins.opcode < 0 || ins.opcode >= COUNT_OPCODES) throw_error(ERROR_RUNTIME, "INVALID OPCODE");
	if (ins.rd < 0 || ins.rd > COUNT_REGISTERS
		|| ins.rs < 0 || ins.rs > COUNT_REGISTERS
		|| ins.rt < 0 || ins.rt > COUNT_REGISTERS) throw_error(ERROR_RUNTIME, "INVALID REGISTER");
	R[imm1] = ins.immediate1;
	R[imm2] = ins.immediate2;
	(*opcode_fn[ins.opcode])(ins.rd, ins.rs, ins.rt);
	pc++;
}

void perform_instruction_loop() {
	int i, irq;
	
	for (i=0; i<COUNT_REGISTERS; i++) R[i] = 0;
	for (i=0; i<COUNT_IOREGISTERS; i++) IORegister[i] = 0;
	for (i=0; i<MAX_DMEM_ITEMS; i++) MEM[i] = 0;
	
	pc = 0;
	cycles = 0;
	while (0 <= pc && pc < instructions_count) {
		irq = (IORegister[irq0enable] & IORegister[irq0status]) 
			| (IORegister[irq1enable] & IORegister[irq1status])
			| (IORegister[irq2enable] & IORegister[irq2status]);
		if (irq) {
			// TODO
		} else {
			if (instructions
			perform_current_instruction();
		}
		// TODO halt, trace, etc
		IORegister[clks]++;
	}
}


int main(int argc, char *argv[]) {
	// %s <imemin.txt> <dmemin.txt> <diskin.txt> <irq2in.txt>
	int i;
	char **instructions_text, **dmemin_text, **diskin_text, **irq2in_text;
	
	if (argc < 5) throw_error(ERROR_PARAMETERS_SIM, argv[0]);
	
	get_file_lines(argv[1], &instructions_text);
	read_instructions(instructions_text);
	free_lines(instructions_text);
	
	get_file_lines(argv[2], &dmemin_text);
	read_dmemin(dmemin_text);
	free_lines(dmemin_text);
	
	get_file_lines(argv[3], &diskin_text);
	read_disk(diskin_text);
	free_lines(diskin_text);
	
	get_file_lines(argv[4], &irq2in_text);
	read_disk(diskin_text);
	free_lines(diskin_text);
	
	perform_instruction_loop();
	return 0;
}
