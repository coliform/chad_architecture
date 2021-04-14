# initialize
out $imm1, $imm2, $zero, 1,  1	# IORegister[$imm1+$zero]=$imm2 (irq1=1)
out $imm2, $imm1, $zero, 9,  0	# IORegister[$imm1+$zero]=$imm2 (leds=0)
out $imm2, $imm1, $zero, 9, 0xFFFF	# leds = 0x0000FFFF
add $s1, $zero, $imm2, 0, 0x0000	# store sector 0 starting 0
add $s2, $zero, $imm2, 0, 0x1000	# store sector 0 starting 4096

SET0: # set to copy sector 0
	add $a1,   $zero, $imm2, 0,    0	# $t1 = 0 (disksector = 0)
	add $a2,   $zero, $s1,   0,    0	# $t2 = 0 (diskbuffer = 0)
	out $imm2, $imm1, $zero, 7, SET1	# IORegister[7] = PC(SET1)
	out $imm2, $imm1, $zero, 14, 1	# diskcmd = 1 (read)
	beq $imm2, $zero, $zero, 0, DISKWAIT	# start waiting
	
SET1: # set to copy sector 1
	add $a1,   $zero, $imm2, 1, 0		# $t1 = 0 (disksector = 0)
	add $a2,   $zero, $s2,   0, 0		# $t2 = 0 (diskbuffer = 0)
	out $imm2, $imm1, $zero, 7, COMPARE	# IORegister[7] = PC(COMPARE)
	out $imm2, $imm1, $zero, 14, 1	# diskcmd = 1 (read)
	beq $imm2, $zero, $zero, 0, DISKWAIT	# start waiting

COMPARE: # finished copying, compare
	add $s0, $zero, $zero, 0, 0		# i = 0
	lw $t1, $s1, $s0, 0, 0			# $t1 = MEM[0+counter]
	lw $t2, $s2, $s0, 0, 0			# $t2 = MEM[4096+counter]
	bne $imm2, $t1, $t2, 0, FAIL		# if neq then sectors neq => fail
	add $s0, $zero, $imm2, 0, 1		# i++
	bne $imm2, $s0, $imm1, 4096, COMPARE	# jump to start if not the end
	beq $imm2, $zero, $zero, 0, HALT	# otherwise, end cleanly

DISKWAIT: # wait here until the disk finishes and irq triggers
	beq $imm2, $zero, $zero, 0, DISKWAIT	# check diskstatus

FAIL:
	in $t0, $imm1, $zero, 9, 0		# $t0 = leds
	sll $t0, $t0, $imm1, 15, 0		# $t0 <<= 15
	out $t0, $imm1, $zero, 9, 0		# leds = $t0
HALT:
	halt $zero, $zero, $zero, 0, 0
