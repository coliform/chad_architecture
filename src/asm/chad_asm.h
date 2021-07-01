#ifndef H_CHAD_ASM
#define H_CHAD_ASM

#include "chad_utils.h"

/*
*
* CHAD assembler
*
*/

#include "chad_asm.h"


bool immediate_to_int(char* in, unsigned int* out, label* labels, int labels_count);
char string_to_opcode(char* in);
char string_to_register(char* in);
bool is_label(char* line);
bool is_dotword(char* line);
void sanitize_line(char* line);
char* split_pop(char* line, char delimiter, int index);
unsigned long long instruction_to_unsigned_long_long(instruction ins);
char* instruction_to_hex(instruction ins);
int compile(char** lines, char** lines_memory);

//int main(int argc, char *argv[]);

#endif
