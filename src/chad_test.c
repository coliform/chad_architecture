#include <chad_utils.h>

int main(int argc, char *argv[]) {
	char* bibi;
	bibi = malloc(sizeof(char)*5);
	bibi[0]='h';
	bibi[1]='a';
	bibi[2]='l';
	bibi[3]='o';
	bibi[4]=0;
	printf("%s\n", bibi);
	pad_left(&bibi, '0', 12);
	printf("%s\n", bibi);
	free(bibi);
	int t1, t2;
	t1 = 2;
	t2 = 1 << 31;
	printf("%d\n", t2+t1);
	return SUCCESS;
}
