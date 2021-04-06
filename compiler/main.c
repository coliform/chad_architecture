/*
*
* CHAD assembler
*
*/

#define DEBUG 0

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_SIZE_PC		4096
#define MAX_SIZE_LINE		500
#define MAX_SIZE_LABEL		50
#define MAX_DMEM_ITEMS		4096
#define MAX_DMEM_CELL		8
#define FLAG_REGULAR		0
#define FLAG_LABEL		1
#define FLAG_DOTWORD		2
#define ERROR_FILE_ACCESS	-1000
#define ERROR_COMPILE_TIME	-2000
#define ERROR_PARAMETERS	-3000
#define COMPILED_SUCCESSFULLY	-10000
#define CHAR_COMMENT		'#'

#define debug_print(fmt, ...) \
	do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while(0)

typedef struct t_instruction {
#if DEBUG==1
	char* line;
#endif
	unsigned char opcode;		// 8 bits	47:40
	unsigned char rd;		// 4 bits	39:36
	unsigned char rs;		// 4 bits	35:32
	unsigned char rt;		// 4 bits	31:28
	unsigned char reserved;		// 4 bits	27:24
	int immediate1;	// 12 bits	23:12
	int immediate2;	// 12 bits	11:0
} instruction;

typedef struct t_label {
	unsigned char* name;
	unsigned int pointer; // to pc
} label;

typedef struct t_dotword {
	unsigned int address;
	unsigned int value;
} dotword;

unsigned long hash(unsigned char *str)
{
    // djb2 hash by Dan Bernstein
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

char CHARSET_HEX[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

char* STR_OPCODES[20] = {
	"add","sub","and","or","sll","sra",
	"srl","beq","bne","blt","bgt","ble",
	"bge","jal","lw","sw","reti","in",
	"out","halt"
};

char* STR_REGISTERS[16] = {
	"zero","imm1","imm2","v0","a0","a1",
	"a2","t0","t1","t2","s0","s1",
	"s2","gp","sp","ra"
};

char* STR_IOREGISTERS[23] = {
	"irq0enable","irq1enable","irq2enable","irq0status",
	"irq1status","irq2status","irqhandler","irqreturn",
	"clks","leds","display","timerenable","timercurrent",
	"timermax","diskcmd","disksector","diskbuffer","diskstatus",
	"reserved","reserved","monitoraddr","monitordata","monitorcmd"
};

unsigned long HASH_OPCODES[20];
unsigned long HASH_REGISTERS[16];
unsigned long HASH_IOREGISTERS[23];

void throw_error(const int reason, const char* details) {
	switch (reason) {
		case ERROR_FILE_ACCESS:
			printf("Error accessing file in path \"%s\"\n", details);
			break;
		case ERROR_COMPILE_TIME:
			printf("Aborted compilation due to an error in the following line:\n%s\n", details);
			break;
		case ERROR_PARAMETERS:
			printf("Format is as following:\n");
			printf("%s <program path> <memory path>\n", details);
			break;
		default:
			printf("An unknown error (code %d) has occurred!\nDetails:%s\n", reason, details);
			break;
	}
	exit(1);
}

bool is_whitespace(char c) { return c==' '||c=='\t'; }

bool char_to_unsigned_int(char* in, unsigned int* out) {
	// returns 1 if error, 0 if ok
	char* p = in;
	*out = 0;
	if (p[0] != 0 && p[1] != 1 && (p[1] == 'x' || p[1] == 'X') ) {
		// hex
		p += 2; // skip 0x
		for (; *p != '\0' && *p != ','; p++) {
			*out *= 16;
			if (*p >= '0' && *p >= '9') *out += (*p) - ((unsigned int)'0');
			else if (*p >= 'A' && *p <= 'F') *out += (*p) - ((unsigned int)'A');
			else if (*p >= 'a' && *p <= 'f') *out += (*p) - ((unsigned int)'A');
			else return 1;
		}
	} else {
		// dec
		for (; *p >= '0' && *p <= '9'; p++) {
			*out *= 10;
			*out += (*p) - ((unsigned int)'0');
		}
	}
	return 0;
}

bool char_to_int(char* in, int* out) {
	// returns 1 if error, 0 if ok
	bool negative = false;
	char* p = in;
	*out = 0;
	if (p[0] != 0 && p[1] != 1 && (p[1] == 'x' || p[1] == 'X') ) {
		// hex
		p += 2; // skip 0x
		for (; *p != '\0' && *p != ','; p++) {
			*out *= 16;
			if (*p >= '0' && *p >= '9') *out += (*p) - ((int)'0');
			else if (*p >= 'A' && *p <= 'F') *out += (*p) - ((int)'A');
			else if (*p >= 'a' && *p <= 'f') *out += (*p) - ((int)'A');
			else return 1;
		}
	} else {
		// dec
		if (*p=='-') {
			negative = true;
			p++;
		}
		for (; *p >= '0' && *p <= '9'; p++) {
			*out *= 10;
			*out += (*p) - ((int)'0');
		}
	}
	if (negative) *out *= -1;
	return 0;
}

int count_occ(char* line, char c) {
	int len, i, occ;
	len = strlen(line);
	occ=0;
	for (i = 0; i < len; i++) {
		if (line[i]==c) occ++;
	}
	return occ;
}

char** split(char* s, char del) {
	char** out;
	char* p;
	int s_len, out_len;
	int i, j, last_del, out_pos;

	out_len = count_occ(s, del)+1;
	out = malloc((out_len+1)*sizeof(char*));
	out[out_len] = 0;

	s_len = strlen(s);
	last_del = -1;
	out_pos = 0;
	for (i = 0; i <= s_len; i++) {
		if (s[i]==del || s[i]==0) {
			out[out_pos] = malloc((i-last_del)*sizeof(char));
			out[out_pos][i-(last_del+1)] = 0;
			for (j=i-1; j>last_del; j--) out[out_pos][j-(last_del+1)] = s[j];
			out_pos++;
			last_del = i;
		}
	}

	return out;
}

void free_lines(char** lines) {
	char** p = lines;
	for (; *p; p++) free(*p);
	free(lines);
}

void pop_char(char* line, int index) {
	int i, len;
	len = strlen(line);
	if (index<0 || index>=len) return;
	for (i=index; i<=len-2; i++) line[i]=line[i+1];
	line[len-1] = 0;
}

unsigned int* memtext_to_uint_arr(char** lines) {
	// no last character, we should know this from lines
	unsigned int* out;
	int i;

	for (i=0; lines[i]!=0; i++);
	out = malloc((i+0)*sizeof(unsigned long long*));
	for (i=0; lines[i]!=0; i++) char_to_unsigned_int(lines[i], out+i);

	return out;
}


bool immediate_to_int(char* in, unsigned int* out, label* labels, int labels_count) {
	int i, pos, len, label_len;
	if ((in[0] >= '0' && in[0] <= '9') || in[0]=='-') return char_to_int(in, out);
	len = strlen(in);

	for (i=0; i<labels_count; i++) {
		label_len = strlen(labels[i].name);
		if (label_len != len) continue;
		for (pos=0; pos<label_len; pos++) {
			if (labels[i].name[pos] != in[pos]) pos=label_len+10;
		}
		if (pos==label_len) {
			*out = i;
			return 0;
		}
	}
	return 1;
}

char string_to_opcode(char* in) {
	int i;
	int in_hash = hash(in);
	for (i=0; i<=19; i++) {
		if (in_hash==HASH_OPCODES[i]) return (char)i;
	}
	return -1;	
}

char string_to_register(char* in) {
	int i, last;
	last=strlen(in)-1;
	if (in[last]==' ') pop_char(in, last);
	if (in[0]=='$') pop_char(in, 0);
	unsigned long in_hash = hash(in);
	for (i=0; i<=15; i++) {
		if (in_hash==HASH_REGISTERS[i]) return (char)i;
	}
	return -1;
}

bool is_label(char* line) {
	char* p = line;
	if (!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z'))) return false;
	for (p=p+1; *p != 0 && (int)(p-line) < MAX_SIZE_LABEL; p++) {
		if (!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9'))) {
			return *p == ':';
		}
	}
	
	return false;
}

bool is_dotword(char* line) {
	char* p = line;
	if (strlen(p) <= 5 || p[0]!='.' || p[1]!='w' || p[2]!='o' || p[3]!='r' || p[4]!='d' || p[5]!=' ') return false;
	return true;
}

void sanitize_line(char* line) {
	char* new, *p;
	int i, len, new_len;
	bool saw_space;
	
	len = strlen(line);
	p = line;
	new = malloc(len*sizeof(char));
	
	saw_space = true; // remove spaces in the beginning too
	new_len=0;
	for (; *p != 0; p++) {
		if (*p == CHAR_COMMENT || *p=='\n' || *p=='\r') {
			//printf("saw end at %c\n",*p);
			*p = 0;
			break;
		} else if (*p==' '||*p=='\t') {
			*p=' '; //replace tab with space
			if (!saw_space) {
				new[new_len]=' ';
				new_len++;
			}
			saw_space = true;
		} else if (*p==',') {
			continue;
		} else {
			saw_space = false;
			new[new_len]=*p;
			new_len++;
		}
	}
	
	for (i=0; i<new_len; i++) line[i]=new[i];
	line[new_len]=0;
	if (line[new_len-1]==' ') line[new_len-1]=0;
}

char* split_pop(char* line, char delimiter, int index) {
	char* out;
	char** lines;
	int i,j;
	if (index<0) return NULL;
	lines = split(line, delimiter);
	for(i=0; i<index && lines[i]; i++);
	if (i==index) {
		out = malloc((strlen(lines[i])+1)*sizeof(char));
		for (j=0; j<strlen(lines[i]); j++) out[j]=lines[i][j];
		out[j]=0;
	} else out = NULL;
	free_lines(lines);
	return out;
}

char* unsigned_long_long_to_hex(unsigned long long number) {
	char* result;
	unsigned long long backup;
	int hex_count,i;

	result=malloc((12+1)*sizeof(char));
	for (i=0; i<12; i++) result[i]='0';
	result[12]=0;
	backup=number;
	for (backup=number, i=11; backup>0; backup>>=4, i--) {
		//printf("backup&15=%llu and %llu\n",backup&15, backup);
		result[i] = CHARSET_HEX[backup&((unsigned long long)15)];
	}
	//printf("%llu\t%d, %s <-> ", number, i, result);
	result[hex_count]=0;
	return result;
}

char* unsigned_int_to_hex(unsigned int number) {
	return unsigned_long_long_to_hex((unsigned long long) number);
}

unsigned long long instruction_to_unsigned_long_long(instruction ins) {
	unsigned long long number = 0;
	number += ((unsigned long long)ins.opcode)<<40;
	number += ((unsigned long long)ins.rd)<<36;
	number += ((unsigned long long)ins.rs)<<32;
	number += ((unsigned long long)ins.rt)<<28;
	number += 0	<<24;
	number += ((unsigned long long)ins.immediate1)<<12;
	number += ((unsigned long long)ins.immediate2)<<0;
	return number;
}

char* instruction_to_hex(instruction ins) {
	return unsigned_long_long_to_hex(instruction_to_unsigned_long_long(ins));
}

int compile(char** lines, char** lines_memory) {
	FILE *fptr;
	int i;
	int pc, len, labels_c, dotwords_c, address_int, value_int, len_lines;
	char *immediate1, *immediate2, *rt, *rd, *rs, *value, *address, *opcode;
	char* line;
	char** p;
	unsigned int* memory_initial = memtext_to_uint_arr(lines_memory);
	unsigned int dotwords[MAX_DMEM_ITEMS];
	instruction* instructions = (instruction*) malloc(MAX_SIZE_PC*sizeof(instruction));
	label* labels = (label*) malloc(MAX_SIZE_PC*sizeof(label));
	for (i=0; i<MAX_DMEM_ITEMS; i++) dotwords[i]=0;
	for (i=0; lines_memory[i]!=0; i++) dotwords[i]=memory_initial[i];

	for (p=lines; *p!=0; p++);
	len_lines=(int)(p-lines)-1;


	labels_c = 0;
	dotwords_c = 0;
	pc = 0; // we only increase pc for regular instructions
	for (i=0; i<len_lines; i++) {
		//printf("loop at line %d, meaning %d<%d\n",i+1,i,len_lines);
		line = lines[i];
		len = strlen(line);
		if (!len) continue;
		sanitize_line(line);
		if (is_label(line)) {
			labels[labels_c].name = malloc((MAX_SIZE_LABEL+2)*sizeof(char));
			strcpy(labels[labels_c].name, line);
			labels[labels_c].name[strlen(line)-1] = 0;
			labels[labels_c].pointer = pc;
			labels_c++;
		} else if (is_dotword(line)) {
			if (count_occ(line, ' ') != 2) throw_error(ERROR_COMPILE_TIME, line);
			address = split_pop(line, ' ', 1);
			value = split_pop(line, ' ', 2);
			if (char_to_unsigned_int(address, &address_int)) throw_error(ERROR_COMPILE_TIME, line);
			if (char_to_unsigned_int(value, &value_int)) throw_error(ERROR_COMPILE_TIME, line);
			dotwords[address_int]=value_int;
			free(address);
			free(value);
			dotwords_c++;
		} else {
			pc++;
		}
	}
	//exit(1);
	dotwords_c = 0;
	pc = 0; // we only increase pc for regular instructions
	for (i=0; i<len_lines; i++) {
		line = lines[i];
		len = strlen(line);
		if (!len) continue;
		debug_print("Performing:\t'%s'\n", line);
		if (is_label(line)) {
			continue;
		} else if (is_dotword(line)) {
			continue;
		} else {
			if (count_occ(line, ' ') != 5) throw_error(ERROR_COMPILE_TIME, line);
			immediate2 = split_pop(line, ' ', 5);
			immediate1 = split_pop(line, ' ', 4);
			rt = split_pop(line, ' ', 3);
			rd = split_pop(line, ' ', 2);
			rs = split_pop(line, ' ', 1);
			opcode = split_pop(line, ' ', 0);
			instructions[pc].rt = string_to_register(rt);
			instructions[pc].rs = string_to_register(rs);
			instructions[pc].rd = string_to_register(rd);
			instructions[pc].opcode = string_to_opcode(opcode);
#if DEBUG==1
			instructions[pc].line = line;
#endif
			if (instructions[pc].rt==-1 || instructions[pc].rt==-1 ||
				instructions[pc].rs == -1 || instructions[pc].opcode == -1 ||
				immediate_to_int(immediate2, &instructions[pc].immediate2, labels, labels_c) ||
				immediate_to_int(immediate1, &instructions[pc].immediate1, labels, labels_c)) throw_error(ERROR_COMPILE_TIME, line);
			free(immediate2);
			free(immediate1);
			free(rt);
			free(rd);
			free(rs);
			free(opcode);
			pc++;
		}
	}

	if ((fptr = fopen("dmemin.txt", "w"))==NULL) return ERROR_FILE_ACCESS;
	for (i = 0; i < dotwords_c; i++) fprintf(fptr, "%x\n", dotwords[i]);
	fclose(fptr);

	if ((fptr = fopen("imemin.txt", "w"))==NULL) return ERROR_FILE_ACCESS;
	for (i = 0; i < pc; i++) {
		line = instruction_to_hex(instructions[i]);
		fprintf(fptr, "%s\n", line);
		free(line);
	}
	fclose(fptr);

	free(instructions);
	for (i=0; i<labels_c; i++) free(labels[i].name);
	free(labels);
	free(memory_initial);

	return COMPILED_SUCCESSFULLY;
}

long get_file_size(FILE *f) {
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);
	return fsize;
}

char* get_file_str(char* path) {
	FILE* f;
	long fsize;
	char* s;
	int i;

	if ((f = fopen(path, "r"))==NULL) throw_error(ERROR_FILE_ACCESS, path);
	fsize = get_file_size(f);
	s = malloc(fsize + 1);
	fread(s, 1, fsize, f);
	fclose(f);
	s[fsize] = 0;
	return s;
}

char** get_lines(char* path) {
	char* raw;
	char** result;
	raw = get_file_str(path);
	result = split(raw,'\n');
	free(raw);
	return result;
}


int main(int argc, char *argv[]) {
	char** lines_program;
	char** lines_mem;

	int i=0;
	for(i=0;i<20;i++) HASH_OPCODES[i]=hash(STR_OPCODES[i]);
	for(i=0;i<16;i++) HASH_REGISTERS[i]=hash(STR_REGISTERS[i]);
	for(i=0;i<23;i++) HASH_IOREGISTERS[i]=hash(STR_IOREGISTERS[i]);

	if (argc < 2 || argc > 3) {
		throw_error(ERROR_PARAMETERS, argv[0]);
		/*printf("Format is as following:\n");
		printf("%s <program path> <memory path>", argv[0]);
		return 1;*/
	}

	lines_program = get_lines(argv[1]);
	if (argc==3) lines_mem = get_lines(argv[2]);
	else lines_mem = split("", '\n');

	compile(lines_program, lines_mem);

	free_lines(lines_program);
	free_lines(lines_mem);
	return 0;
}
