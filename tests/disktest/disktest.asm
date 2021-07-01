# initialize
out $imm1, $imm2, $zero, 1,  1		# irq1enable = 1
add $s1, $zero, $imm2, 0, 0	# store sector 0 starting 0
add $s2, $zero, $imm2, 0, 128	# store sector 0 starting 128
add $s0, $zero, $zero, 0, 0		# i = 0

SET0: # set to copy sector 0
	out $imm2, $imm1, $zero, 15, 0			# IORegister[disksector] = 0
	out $imm2, $imm1, $zero, 16, 0			# IOregister[diskbuffer] = 0
	out $imm2, $imm1, $zero, 6, SET0IRQ		# IORegister[irqhandler] = PC(SET0IRQ)
	out $imm2, $imm1, $zero, 14, 1			# IORegister[diskcmd] = 1 (read)
	beq $imm2, $zero, $zero, 0, DISKWAIT	# start waiting
SET0IRQ: # we get teleported here once disk finishes copying
	out $imm2, $imm1, $zero, 7, SET1		# IORegister[irqreturn] = PC(SET1)
	reti $zero, $zero, $zero, 0, 0			# now go to SET1

SET1: # set to copy sector 1
	out $imm2, $imm1, $zero, 15, 1			# IORegister[disksector] = 1
	out $imm2, $imm1, $zero, 16, 128		# IOregister[diskbuffer] = 1
	out $imm2, $imm1, $zero, 6, SET1IRQ		# IORegister[7] = PC(COMPARE)
	out $imm2, $imm1, $zero, 14, 1			# diskcmd = 1 (read)
	beq $imm2, $zero, $zero, 0, DISKWAIT	# start waiting
SET1IRQ: # we are basically done, just get rid of irq routine
	out $imm2, $imm1, $zero, 7, COMPARE		# IORegister[irqreturn] = PC(COMPARE)
	reti $zero, $zero, $zero, 0, 0			# now go to COMPARE

COMPARE: # finished copying, compare
	lw $t1, $s1, $s0, 0, 0			# $t1 = MEM[0+counter]
	lw $t2, $s2, $s0, 0, 0			# $t2 = MEM[128+counter]
	bne $imm2, $t1, $t2, 0, FAIL		# if neq then sectors neq => fail
	add $s0, $s0, $imm2, 0, 1		# i++
	bne $imm2, $s0, $imm1, 128, COMPARE	# jump to start if i != 128
	beq $imm2, $zero, $zero, 0, SUCCESS	# otherwise, end cleanly

DISKWAIT: # wait here until the disk finishes and irq triggers
	beq $imm2, $zero, $zero, 0, DISKWAIT	# check diskstatus

FAIL:
	lw $t0, $zero, $imm2, 0, 0x200	# $t0 = 0x0000FFFF
	out $t0, $imm1, $zero, 9, 0		# leds = $t0
	halt $zero $zero $zero 0 0

SUCCESS:
	lw $t0, $zero, $imm2, 0, 0x201
	out $t0, $imm1, $zero, 9, 0	# leds = 0x0000FFFF
HALT:
	halt $zero, $zero, $zero, 0, 0

.word 0x200 0xFFFF0000
.word 0x201 0x0000FFFF