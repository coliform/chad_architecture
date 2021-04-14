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
	-rm -rf imemin.txt; true
	-rm -rf dmemin.txt; true

test_sim:
	-make sim
	-$(OUT)/sim $(TESTDIR)/fib/imemin.txt $(TESTDIR)/fib/dmemin.txt $(TESTDIR)/fib/diskin.txt $(TESTDIR)/fib/irq2in.txt drafts/dmemout.txt drafts/regout.txt drafts/trace.txt drafts/hwregtrace.txt drafts/cycles.txt drafts/leds.txt drafts/display7seg.txt drafts/diskout.txt drafts/monitor.txt drafts/monitor.yuv
	-if diff -q --strip-trailing-cr drafts/trace.txt $(TESTDIR)/fib/trace.txt; then echo "Equal"; else echo "Neq"; fi
	-if diff -q --strip-trailing-cr drafts/leds.txt $(TESTDIR)/fib/leds.txt; then echo "Equal"; else echo "Neq"; fi
	-if diff -q --strip-trailing-cr drafts/monitor.txt $(TESTDIR)/fib/monitor.txt; then echo "Equal"; else echo "Neq"; fi
	-if diff -q --strip-trailing-cr drafts/monitor.yuv $(TESTDIR)/fib/monitor.yuv; then echo "Equal"; else echo "Neq"; fi
	-if diff -q --strip-trailing-cr drafts/regout.txt $(TESTDIR)/fib/regout.txt; then echo "Equal"; else echo "Neq"; fi
	-if diff -q --strip-trailing-cr drafts/hwregtrace.txt $(TESTDIR)/fib/hwregtrace.txt; then echo "Equal"; else echo "Neq"; fi
#	-if diff -q --strip-trailing-cr imemin.txt $(TESTDIR)/fib/imemin.txt; then echo "Equal"; else echo "Neq"; fi
#	-rm -rf imemin.txt; true
#	-rm -rf dmemin.txt; true

test_sim_valgrind:
	-make sim
	-valgrind --leak-check=full --track-origins=yes $(OUT)/sim $(TESTDIR)/fib/imemin.txt $(TESTDIR)/fib/dmemin.txt $(TESTDIR)/fib/diskin.txt $(TESTDIR)/fib/irq2in.txt drafts/dmemout.txt drafts/regout.txt drafts/trace.txt drafts/hwregtrace.txt drafts/cycles.txt drafts/leds.txt drafts/display7seg.txt drafts/diskout.txt drafts/monitor.txt drafts/monitor.yuv
#	-if diff -q --strip-trailing-cr imemin.txt $(TESTDIR)/fib/imemin.txt; then echo "Equal"; else echo "Neq"; fi
#	-rm -rf imemin.txt; true
#	-rm -rf dmemin.txt; true

test: $(SRC)/chad_utils.o $(SRC)/chad_test.o
	$(CC) -o $(OUT)/test $(SRC)/chad_utils.o $(SRC)/chad_test.o
	-make move_objects
	-$(OUT)/test

valtest: $(SRC)/chad_utils.o $(SRC)/chad_test.o
	$(CC) -o $(OUT)/test $(SRC)/chad_utils.o $(SRC)/chad_test.o
	-make move_objects
	-valgrind --leak-check=full --track-origins=yes $(OUT)/test
	

move_objects:
	-mv -f *.o $(ODIR) 2>/dev/null; true
	-mv -f $(SRC)/*.o $(ODIR) 2>/dev/null; true
	-mv -f $(SRC)/asm/*.o $(ODIR) 2>/dev/null; true
	-mv -f $(SRC)/sim/*.o $(ODIR) 2>/dev/null; true
