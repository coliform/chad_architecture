/*
*
* CHAD simulator
*
*/

#include <chad_sim.h>


char **imemin_text, **dmemin_text, **diskin_text, **irq2in_text;

// program metadata
uint32 pc;
instruction instructions[MAX_SIZE_PC];
char **instructions_text;
int instruction_count, sector_count, sector_head;
FILE *f_dmemout, *f_regout, *f_trace, *f_hwregtrace, *f_cycles, *f_leds, *f_display7seg, *f_diskout, *f_monitor, *f_monitoryuv;
bool irq0, irq1, irq2, irq, irq_routine;


uint32 R[COUNT_REGISTERS];
uint32 IORegister[COUNT_IOREGISTERS];
uint32 IORegister_SIZE_IN[COUNT_IOREGISTERS] = {
	0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000FFF, 0x00000FFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000001, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000003, 0x0000007F, 0x00000FFF, 0x00000001, 0x00000000, 0x00000000, 0x0000FFFF, 0x000000FF, 0x00000000
};
uint32 IORegister_SIZE_OUT[COUNT_IOREGISTERS] = {
	0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000FFF, 0x00000FFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000001, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000003, 0x0000007F, 0x00000FFF, 0x00000001, 0x00000000, 0x00000000, 0x0000FFFF, 0x000000FF, 0x00000001
};
uint32 MEM[MAX_DMEM_ITEMS];
uint32 disk[SIZE_HDD_SECTORS_H][SIZE_HDD_SECTORS_W];
uint8 monitor[SIZE_MONITOR_H*SIZE_MONITOR_W];
uint32 irq2in_i, irq2in_count;
uint32 *irq2in_feed;
int irq1_i;

#if DEBUG==1
char* opcode_names[COUNT_OPCODES]={"add", "sub", "and", "or", "sll", "sra", "srl", "beq", "bne", "blt", "bgt", "ble", "bge", "jal", "lw", "sw", "reti", "in", "out", "halt"};
char* register_names[COUNT_REGISTERS]={"zero", "imm1", "imm2", "v0", "a0", "a1", "a2", "t0", "t1", "t2", "s0", "s1", "s2", "gp", "sp", "ra"};
char* ioregister_names[COUNT_IOREGISTERS]={"irq0enable", "irq1enable", "irq2enable", "irq0status", "irq1status", "irq2status", "irqhandler", "irqreturn", "clks", "leds", "display", "timerenable", "timercurrent", "timermax", "diskcmd", "disksector", "diskbuffer", "diskstatus", "reserved", "reserved", "monitoraddr", "monitordata", "monitorcmd"};

void print_registers() {
	int i;
	
	for (i = 0; i < COUNT_REGISTERS; i++) {
		printf("%s=%d, ", register_names[i], R[i]);
	}
	printf("\r\n");
}

void press_enter() {
	char ch;
	printf("Press 0 to stop: ");
	scanf("%c",&ch);
	if (ch == '0') pc = instruction_count;
}

void write_pretty_trace() {
	fprintf(f_trace, "%d", pc);
	fprintf(f_trace, "\t");
	fprintf(f_trace, "%s", opcode_names[instructions[pc].opcode]);
	for (int i=0; i<COUNT_REGISTERS; i++) {
		fprintf(f_trace, "\t");
		fprintf(f_trace, "%d", R[i]);
	}
	fprintf(f_trace, "\n");
}
#endif

void write_trace() {
	char* hex;
	fprintf(f_trace, "%s", llu_to_hex(pc, 3));
	fprintf(f_trace, " %s", imemin_text[pc]);
	for (int i=0; i<COUNT_REGISTERS; i++) {
		hex = llu_to_hex_low((llu)R[i],8);
		fprintf(f_trace, " %s", hex);
		free(hex);
	}
	fprintf(f_trace, "\n");
}

void write_cycles() {
	fprintf(f_cycles, "%d", IORegister[clks]);
}

void write_registers() {
	char* hex;
	for (int i=3; i<COUNT_REGISTERS; i++) {
		hex = llu_to_hex((llu)R[i],8);
		fprintf(f_regout, "%s\n", hex);
		free(hex);
	}
}

void write_trace_hwreg(char* action, int number, uint32 value) {
	char* hex;
	hex = llu_to_hex_low((llu)value, 8);
	fprintf(f_hwregtrace, "%d %s %s %s\n", IORegister[clks], action, ioregister_names[number], hex);
	free(hex);
}

void write_display7seg() {
	char* hex;
	hex = llu_to_hex_low((llu)IORegister[display], 8);
	fprintf(f_display7seg, "%d %s\n", IORegister[clks], hex);
	free(hex);
}

void write_dmemout() {
	char* hex;
	int last_nonzero, i;
	
	last_nonzero = -1;
	for (i=0; i<MAX_DMEM_ITEMS; i++) {
		if (MEM[i]!=0) last_nonzero = i;
	}
	if (last_nonzero==-1) return;
	
	for (i=0; i<=last_nonzero; i++) {
		hex = llu_to_hex((llu)MEM[i], 8);
		fprintf(f_dmemout, "%s\n", hex);
		free(hex);
	}
}

void write_diskout() {
	char* hex;
	int sector_number, sector_cell;
	for (sector_number=0; sector_number<SIZE_HDD_SECTORS_H; sector_number++) {
		for (sector_cell=0; sector_cell<SIZE_HDD_SECTORS_W; sector_cell++) {
			hex = llu_to_hex((llu)disk[sector_number][sector_cell], 8);
			fprintf(f_diskout, "%s", hex);
			free(hex);
		}
	}
}

void except(const char* details) {
	printf("Simulator crashed. PC=%d\n", pc);
	printf("%s\n", details);
	fclose(f_trace);
	exit(1);
}

void parse_instruction(char* line, int index) {
	int i1, i2;
	llu binary;
	strip(line);
	hex_to_unsigned_long_long(line, &binary);
	instructions[index].opcode 		= (binary>>40)&0xFF;
	instructions[index].rd			= (binary>>36)&0xF;
	instructions[index].rs			= (binary>>32)&0xF;
	instructions[index].rt			= (binary>>28)&0xF;
	i1					= (binary>>12)&0xFFF;
	i2					= (binary>>0 )&0xFFF;
	// extend 12 -> 32 bits
	i1 = (i1 >> 11) == 0 ? i1 : -1 ^ 0xFFF | i1;
	i2 = (i2 >> 11) == 0 ? i2 : -1 ^ 0xFFF | i2;
	instructions[index].immediate1 = i1;
	instructions[index].immediate2 = i2;
	/*int sign1, sign2;
	sign1 = (1<<11)&instructions[index].immediate1;
	sign2 = (1<<11)&instructions[index].immediate2;
	instructions[index].immediate1 &= (~sign1);
	instructions[index].immediate1 |= (sign1 >> 11) << 31;
	instructions[index].immediate2 &= (~sign2);
	instructions[index].immediate2 |= (sign2 >> 11) << 31;*/
			printf("ins.rd = %x\n", (unsigned int)((binary>>36)&0xF));
}

void read_instructions(char** lines) {
	int i, counted;
	for (i=0, counted=0; lines[i]!=0; i++) {
		if (strlen(lines[i])==0) continue;
		parse_instruction(lines[i], counted);
		printf("%s, -> (%d)\t%s %d %d %d %d %d\n", lines[i], counted, opcode_names[instructions[counted].opcode], instructions[counted].rd, instructions[counted].rs, instructions[counted].rt, instructions[counted].immediate1, instructions[counted].immediate2);
		counted++;
		//if (counted >= 5) exit(0);
	}
	instruction_count = counted;
}

void parse_disk_sector(char* line, int sector_number, int sector_cell) {
	hex_to_uint32(line, &disk[sector_number][sector_cell]);
}

void read_disk(char** lines) {
	int i, counted, sector_number, sector_cell;
	for (i=0, counted=0; lines[i]!=0; i++) {
		if (strlen(lines[i])==0) continue;
		if (sector_cell == SIZE_HDD_SECTORS_W) {
			sector_cell = 0;
			sector_number++;
		}
		if (sector_number == SIZE_HDD_SECTORS_H) {
			except("DISK TOO LARGE");
		}
		
		parse_disk_sector(lines[i], sector_number, sector_cell);
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
	// TODO: maybe sort
	//hex_to_uint32(line, &(irq2in_feed[index]));
	irq2in_feed[index] = atoi_custom(line)+1;
}

void read_irq2in(char** lines) {
	int i, counted;
	
	for (i=0, counted=0; lines[i]!=0; i++) {
		strip(lines[i]);
		if (strlen(lines[i])==0) continue;
		counted++;
	}
	irq2in_i = 0;
	irq2in_count = counted;
	irq2in_feed = malloc((irq2in_count)*sizeof(uint32));
	
	for (i=0, counted=0; lines[i]!=0; i++) {
		if (strlen(lines[i])==0) continue;
		parse_irq2in_line(lines[i], counted);
		counted++;
	}
	
	for (i=0; i<counted; i++) {
		printf("%d\n", irq2in_feed[i]);
	}
	//exit(0);
}

void write_leds() {
	char* hex;
	uint32 value = IORegister[leds];
	hex = llu_to_hex_low((llu)value, 8);
	fprintf(f_leds, "%d %s\n", IORegister[clks], hex);
	free(hex);
}

void print_monitor_to_file() {
	int i;
	char* hex;
	f_monitor=freopen(NULL,"w",f_monitor);
	for (i=0; i<SIZE_MONITOR; i++) {
		hex = llu_to_hex((llu)monitor[i], 2);
		fprintf(f_monitor, "%s\n", hex);
		free(hex);
	}
}

void print_monitor_to_file_yuv() {
	f_monitoryuv=freopen(NULL,"wb",f_monitoryuv);
	// APPARENTLY FWRITE IS BUILT DIFFERENTLY THAN FPRINTF LOL
	fwrite(&monitor, sizeof(uint8), SIZE_MONITOR, f_monitoryuv);
}

void write_monitor() {
	uint32 addr, data;
	if (!IORegister[monitorcmd]) return;
	addr = IORegister[monitoraddr];
	data = IORegister[monitordata];
	monitor[addr] = data;
	print_monitor_to_file();
	print_monitor_to_file_yuv();
}

// register_write:
// safely writes into a register, crashes if register is invalid
void register_write(int number, uint32 value) {
	if (number < 0 || number >= COUNT_REGISTERS) except("EXCEPTION IN REGISTER WRITE, INVALID REGISTER");
	if (number == 0 || number == 1 || number == 2) return;
	R[number] = value;
}

// register_read:
// safely reads from a register, crashes if register is invalid
uint32 register_read(int number) {
	if (number < 0 || number >= COUNT_REGISTERS) except("EXCEPTION IN REGISTER READ, INVALID REGISTER");
	return R[number];
}

void memory_write(int number, uint32 value) {
	if (number < 0 || number >= MAX_DMEM_ITEMS) except("EXCEPTION IN MEMORY WRITE, INVALID MEMORY ADDRESS");
	MEM[number] = value;
}

uint32 memory_read(int number) {
	printf("memory read at %d is %d\n", number, MEM[number]);
	if (number < 0 || number >= MAX_DMEM_ITEMS) except("EXCEPTION IN MEMORY READ, INVALID MEMORY ADDRESS");
	return MEM[number];
}

void DMA_write() {
	// MEM -> disk
	// disksector -	sector number
	// sizeof(disksector)=7 bits (128 sectors total)
	// disk[disksector] -	actual sector being written
	// sizeof(disk[disksector])=512 bytes
	// diskbuffer -	address to buffer, 12 bits long (0-4095)
	// MEM[diskbuffer] -	start of actual data being copied to disk
	// sizeof(MEM[diskbuffer]) = sizeof(word) = 32 bits = 4 bytes
	// we copy one word at a time
	// 128 sectors, 128 4-byte-words in each = 65536 bytes = 64 kilobytes

	// 1024 runs in total somehow
	// if we wanted to fill a whole sector
	// one sector is 512 bytes
	// so 4096 bits
	// written in 4 byte words
	// meaning 32 bits
	// 128 runs in total, idk why 1024
	if (IORegister[diskcmd]!=2) return; // verifies write command
	if (sector_head <= SIZE_HDD_SECTORS_W) {
		disk[disksector][sector_head] = MEM[diskbuffer+sector_head];
	}
}

void DMA_read() {
	// disk -> MEM
	if (IORegister[diskcmd]!=1) return; // verifies write command
	if (sector_head <= SIZE_HDD_SECTORS_W) {
		MEM[diskbuffer+sector_head] = disk[disksector][sector_head];
	}
}

void DMA_write_now() {
	// do it immediately
	for (sector_head = 0; sector_head < SIZE_HDD_SECTORS_W; sector_head++) {
		DMA_write();
	}
	// and reset head
	sector_head = 0;
}

void DMA_read_now() {
	// do it immediately
	for (sector_head = 0; sector_head < SIZE_HDD_SECTORS_W; sector_head++) {
		DMA_read();
	}
	// and reset head
	sector_head = 0;
}

void assign_disk_cmd(uint32 cmd) {
	if (IORegister[diskstatus]==1) {
		// disk is busy
		return;
	}
	
	sector_head = 0;
	IORegister[diskcmd] = cmd;
	IORegister[diskstatus] = 1;
}

void ioregister_write(int number, uint32 value) {
	if (number < 0 || number >= COUNT_IOREGISTERS) except("EXCEPTION IN IO REGISTER WRITE, INVALID IOREGISTER");
	value &= IORegister_SIZE_OUT[number];
	switch (number) {
		case leds:
			IORegister[number] = value;
			write_leds();
			break;
		case monitorcmd:
			IORegister[number] = value;
			write_monitor();
			break;
		case display:
			IORegister[number] = value;
			write_display7seg();
			break;
		case diskcmd:
			// don't assign automatically here
			assign_disk_cmd(IORegister[number]);
			break;
		default:
			IORegister[number] = value & IORegister_SIZE_OUT[number];
			break;
	}
	write_trace_hwreg("WRITE", number, value);
}

uint32 ioregister_read(int number) {
	if (number < 0 || number >= COUNT_IOREGISTERS) except("EXCEPTION IN IO REGISTER READ, INVALID IOREGISTER");
	write_trace_hwreg("READ", number, IORegister[number] & IORegister_SIZE_IN[number]);
	return IORegister[number] & IORegister_SIZE_IN[number];
}


/*
*
* *** OPCODES ***
* The functions below contain the opcodes
*
*/

void set_pc(int i) {
	pc = (i&0x00000FFF)-1;
}

void clock_tick() {
	if (IORegister[clks]==0xFFFFFFFF) IORegister[clks]=0;
	else IORegister[clks]++;
}

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
	if (register_read(rs)==register_read(rt)) set_pc(register_read(rd));
}

void bne(int rd, int rs, int rt) {
	if (register_read(rs)!=register_read(rt)) set_pc(register_read(rd));
}

void blt(int rd, int rs, int rt) {
	if (register_read(rs)<register_read(rt)) set_pc(register_read(rd));
}

void bgt(int rd, int rs, int rt) {
	if (register_read(rs)>register_read(rt)) set_pc(register_read(rd));
}

void ble(int rd, int rs, int rt) {
	if (register_read(rs)<=register_read(rt)) set_pc(register_read(rd));
}

void bge(int rd, int rs, int rt) {
	if (register_read(rs)>=register_read(rt)) set_pc(register_read(rd));
}

void jal(int rd, int rs, int rt) {
	register_write(ra, pc+1);
	printf("set pc from jal to %d\n",register_read(rd));
	set_pc(register_read(rd));
//	printf("instruction there is %s\n", opcode_names[instructions[pc].opcode]);
}

void lw(int rd, int rs, int rt) {
	register_write( rd, memory_read( ((int)register_read(rs))+((int)register_read(rt)) ) );
}

void sw(int rd, int rs, int rt) {
	memory_write( ((int)register_read(rs))+((int)register_read(rt)), register_read(rd) );
}

void reti(int rd, int rs, int rt) {
	set_pc(IORegister[irqreturn]);
	irq_routine = 0;
}

void in(int rd, int rs, int rt) {
	register_write(rd, ioregister_read(((int)register_read(rs))+((int)register_read(rt))));
}

void out(int rd, int rs, int rt) {
	ioregister_write(((int)register_read(rs))+((int)register_read(rt)),register_read(rd));
}

void halt(int rd, int rs, int rt) {
	pc = instruction_count;
}

void (*opcode_fn[COUNT_OPCODES])(int, int, int) = {
	add, sub, and, or, sll, sra, srl, beq, bne, blt, bgt, ble, bge, jal, lw, sw, reti, in, out, halt
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
	if (ins.opcode < 0 || ins.opcode >= COUNT_OPCODES) except("INVALID OPCODE");
	if (ins.rd < 0 || ins.rd > COUNT_REGISTERS
		|| ins.rs < 0 || ins.rs > COUNT_REGISTERS
		|| ins.rt < 0 || ins.rt > COUNT_REGISTERS) except("INVALID REGISTER");

	R[imm1] = ins.immediate1;
	R[imm2] = ins.immediate2;
	write_trace();
	printf("%s $%s, $%s, $%s, %d, %d\n", opcode_names[instructions[pc].opcode], register_names[instructions[pc].rd], register_names[instructions[pc].rs], register_names[instructions[pc].rt], instructions[pc].immediate1, instructions[pc].immediate2);
	(*opcode_fn[ins.opcode])(ins.rd, ins.rs, ins.rt);
	pc++;
}

void check_interrupt_0() {
	// TIMER
	irq0 = IORegister[irq0enable] & IORegister[irq0status];
	irq |= irq0;
	if (IORegister[timerenable] == 0) return;
	if (IORegister[timercurrent] < IORegister[timermax]) {
		IORegister[timercurrent] += 1;
		IORegister[irq0status] = 0;
	} else {
		IORegister[timercurrent] = 0;
		IORegister[irq0status] = 1;
	}
}

void check_interrupt_1() {
	// DISK
	if (IORegister[irq1enable] == 0) return;
	if (IORegister[diskcmd] == 0 && IORegister[diskstatus] == 0) {
		IORegister[irq1status] = 0;
	}
	
	if (IORegister[diskstatus]==1) {
		switch (IORegister[diskcmd]) {
			case 1:
				DMA_write();
				break;
			case 2:
				DMA_read();
				break;
			default: break;
		}
		sector_head++;
		
		// rather than the DMA functions setting it:
		if (sector_head == 1024) {
			IORegister[diskcmd] = 0;
			IORegister[diskstatus] = 0;
			// we want irq to trigger simultaneously if access is done
			IORegister[irq1status] = 1;
		}
	}
	
	irq1 = IORegister[irq1enable] & IORegister[irq1status];
	if (irq1) {
		IORegister[diskcmd] = 0;
		IORegister[diskstatus] = 0;
	}
	irq |= irq1;
}

void check_interrupt_2() {
	// EXTERNAL
	if (irq2in_i >= irq2in_count || irq2in_i < 0) return;
	if (irq2in_feed[irq2in_i]==IORegister[clks]) {
		IORegister[irq2status] = 1;
		irq2in_i++;
	} else {
		IORegister[irq2status] = 0;
	}
	irq2 = IORegister[irq2enable] & IORegister[irq2status];
	irq |= irq2;
	if (irq2) {
		printf("caught irq2\n");
		//exit(0);
	}
}

void check_interrupts() {
	irq = 0;
	check_interrupt_0();
	check_interrupt_1();
	check_interrupt_2();
	printf("%d %d %d\n", irq0, irq1, irq2);
	if (irq == 1) {
		if (irq_routine) {
			return;
		} else {
			printf("triggered irq\n");
			irq_routine = 1;
			IORegister[irqreturn] = pc;
			pc = IORegister[irqhandler];
		}
		
	} else {
		return;
	}
}

void perform_instruction_loop() {
	int i;
	irq_routine = 0;
	
	for (i=0; i<COUNT_REGISTERS; i++) R[i] = 0;
	for (i=0; i<COUNT_IOREGISTERS; i++) IORegister[i] = 0;
	
	pc = 0;
	while (0 <= pc && pc < instruction_count) {
		check_interrupts();
		perform_current_instruction();
		clock_tick();
		//if (IORegister[clks] >= 10) exit(0);
		//print_registers();
		//printf("Cycle %d, ", IORegister[clks]-1);
		//press_enter();
	}
	
	write_cycles();
	write_registers();
	write_dmemout();
	write_diskout();
}


int main(int argc, char *argv[]) {
	// %s <imemin.txt> <dmemin.txt> <diskin.txt> <irq2in.txt>
	int i;
	
/*	llu fifi;
	hex_to_unsigned_long_long("0E4020000040", &fifi);
	printf("%llu\n", fifi);
	fifi = 0;
	fifi 
	exit(0);*/
	
	if (argc != 15) throw_error(ERROR_PARAMETERS_SIM, argv[0]); // god help me
	
	get_file_lines(argv[1], &imemin_text);
	read_instructions(imemin_text);
	
	get_file_lines(argv[2], &dmemin_text);
	read_dmemin(dmemin_text);
	
	get_file_lines(argv[3], &diskin_text);
	read_disk(diskin_text);
	
	get_file_lines(argv[4], &irq2in_text);
	read_irq2in(irq2in_text);
	

	if ((f_dmemout = fopen(argv[5], "w"))==NULL) throw_error(ERROR_FILE_ACCESS, argv[5]);
	if ((f_regout = fopen(argv[6], "w"))==NULL) throw_error(ERROR_FILE_ACCESS, argv[6]);
	if ((f_trace = fopen(argv[7], "w"))==NULL) throw_error(ERROR_FILE_ACCESS, argv[7]);
	if ((f_hwregtrace = fopen(argv[8], "w"))==NULL) throw_error(ERROR_FILE_ACCESS, argv[8]);
	if ((f_cycles = fopen(argv[9], "w"))==NULL) throw_error(ERROR_FILE_ACCESS, argv[9]);
	if ((f_leds = fopen(argv[10], "w"))==NULL) throw_error(ERROR_FILE_ACCESS, argv[10]);
	if ((f_display7seg = fopen(argv[11], "w"))==NULL) throw_error(ERROR_FILE_ACCESS, argv[11]);
	if ((f_diskout = fopen(argv[12], "w"))==NULL) throw_error(ERROR_FILE_ACCESS, argv[12]);
	if ((f_monitor = fopen(argv[13], "w"))==NULL) throw_error(ERROR_FILE_ACCESS, argv[13]);
	if ((f_monitoryuv = fopen(argv[14], "wb"))==NULL) throw_error(ERROR_FILE_ACCESS, argv[14]);
	
	//fprintf(f_trace, "PC	INST	R0	R1	R2	R3	R4	R5	R6	R7	R8	R9	R10	R11	R12	R13	R14	R15\n");
	
	perform_instruction_loop();
	
	fclose(f_dmemout);
	fclose(f_regout);
	fclose(f_trace);
	fclose(f_hwregtrace);
	fclose(f_cycles);
	fclose(f_leds);
	fclose(f_display7seg);
	fclose(f_diskout);
	fclose(f_monitor);
	fclose(f_monitoryuv);
	
	free_lines(imemin_text);
	free_lines(dmemin_text);
	free_lines(diskin_text);
	free_lines(irq2in_text);
	return 0;
}
