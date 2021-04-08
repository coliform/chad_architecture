#include <chad_utils.h>

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


void strip(char* in) {
	for (char* p = in; *p != '\0'; p++) {
		if (*p == '\n' || *p == '\r') {
			*p = '\0';
			return;
		}
	}
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

int get_file_lines(char* path, char*** result) {
	int lines_count;
	char* raw;
	raw = get_file_str(path);
	lines_count = split(raw,'\n', result);
	free(raw);
	return lines_count;
}

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
		case ERROR_PARAMETERS_SIM:
			printf("Simulator format is as following:\n");
			printf("%s <imemin.txt> <dmemin.txt> <diskin.txt> <irq2in.txt>\n", details);
			break;
		default:
			printf("An unknown error (code %d) has occurred!\nDetails:%s\n", reason, details);
			break;
	}
	exit(1);
}

bool is_whitespace(char c) { return c==' '||c=='\t'; }


// CODE DUPLICATION WARNING
bool hex_to_unsigned_int(char* in, unsigned int* out) {
	char* p = in;
	*out = 0;
	for (; *p != '\0'; p++) {
		*out *= 16;
		if (*p >= '0' && *p <= '9') *out += (*p) - ((unsigned int)'0');
		else if (*p >= 'A' && *p <= 'F') *out += (*p) - ((unsigned int)'A');
		else if (*p >= 'a' && *p <= 'f') *out += (*p) - ((unsigned int)'a');
		else return 1;
	}
}

bool hex_to_unsigned_long_long(char* in, unsigned long long* out) {
	char* p = in;
	*out = 0;
	for (; *p != '\0'; p++) {
		*out *= 16;
		if (*p >= '0' && *p <= '9') *out += (*p) - ((unsigned long long)'0');
		else if (*p >= 'A' && *p <= 'F') *out += (*p) - ((unsigned long long)'A');
		else if (*p >= 'a' && *p <= 'f') *out += (*p) - ((unsigned long long)'a');
		else return 1;
	}
}

bool hex_to_uint32(char* in, uint32* out) {
	char* p = in;
	*out = 0;
	for (; *p != '\0'; p++) {
		*out *= 16;
		if (*p >= '0' && *p <= '9') *out += (*p) - ((uint32)'0');
		else if (*p >= 'A' && *p <= 'F') *out += (*p) - ((uint32)'A');
		else if (*p >= 'a' && *p <= 'f') *out += (*p) - ((uint32)'a');
		else return 1;
	}
}

char* unsigned_long_long_to_hex(unsigned long long number) {
	char* result;
	//printf("i got %llu\n", number);
	unsigned long long backup;
	int hex_count,i;

	result=malloc((12+1)*sizeof(char));
	for (i=0; i<12; i++) result[i]='0';
	result[12]=0;
	backup=number;
	for (backup=number, i=11; backup>0; backup>>=4, i--) {
		//printf("backup&15=%llu and %llu\n",backup&15, backup);
		result[i] = CHARSET_HEX[backup&((unsigned long long)15)];
		//printf("result is %s\n", result);
	}
	//printf("%llu\t%d, %s <-> ", number, i, result);
	result[hex_count]=0;
	return result;
}

char* unsigned_int_to_hex(unsigned int number) {
	return unsigned_long_long_to_hex((unsigned long long) number);
}

bool char_to_unsigned_int(char* in, unsigned int* out) {
	// returns 1 if error, 0 if ok
	bool negative = false;
	char* p = in;
	*out = 0;
	if (p[0] != 0 && p[1] != 0 && (p[1] == 'x' || p[1] == 'X') ) {
		// hex
		p += 2; // skip 0x
		hex_to_unsigned_int(in, out);
	} else {
		// dec
		if (*p=='-') {
			negative = true;
			p++;
		}
		for (; *p >= '0' && *p <= '9'; p++) {
			*out *= 10;
			*out += (*p) - ((unsigned int)'0');
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

int split(char* s, char del, char*** out) {
	char* p;
	int s_len, out_len;
	int i, j, last_del, out_pos;

	out_len = count_occ(s, del)+1;
	*out = malloc((out_len+1)*sizeof(char*));
	(*out)[out_len] = 0;

	s_len = strlen(s);
	last_del = -1;
	out_pos = 0;
	for (i = 0; i <= s_len; i++) {
		if (s[i]==del || s[i]==0) {
			(*out)[out_pos] = malloc((i-last_del)*sizeof(char));
			(*out)[out_pos][i-(last_del+1)] = 0;
			for (j=i-1; j>last_del; j--) (*out)[out_pos][j-(last_del+1)] = s[j];
			out_pos++;
			last_del = i;
		}
	}

	return out_len;
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

unsigned long hash(unsigned char *str)
{
    // djb2 hash by Dan Bernstein
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}
