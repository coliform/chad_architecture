CC=gcc
CFLAGS=-Isrc/. -Isrc/asm/. -Isrc/sim/.
OUT=out
SRC=src
TESTDIR=tests
TESTS=fib
ODIR=$(OUT)/intermediate
DEPS = $(SRC)/chad_utils.h $(SRC)/asm/chad_asm.h


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

asm: $(SRC)/chad_utils.o $(SRC)/asm/chad_asm.o
	$(CC) -o $(OUT)/asm $(SRC)/chad_utils.o $(SRC)/asm/chad_asm.o
	-make move_objects

sim: $(SRC)/chad_utils.o $(SRC)/sim/chad_sim.o
	$(CC) -o $(OUT)/sim $(SRC)/chad_utils.o $(SRC)/sim/chad_sim.o
	-make move_objects
	
test_asm:
	-make asm
	-$(OUT)/asm $(TESTDIR)/fib/fib.asm
	-if diff -q --strip-trailing-cr imemin.txt $(TESTDIR)/fib/imemin.txt; then echo "Equal"; else echo "Neq"; fi

move_objects:
	-mv *.o $(ODIR)
	-mv $(SRC)/*.o $(ODIR)
	-mv $(SRC)/asm/*.o $(ODIR)
	-mv $(SRC)/sim/*.o $(ODIR)
