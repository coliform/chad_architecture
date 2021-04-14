	out $imm1, $imm2, $zero, 1,  1	# IORegister[$imm1+$zero]=$imm2 (irq1=1)
	out $imm2, $imm1, $zero, 9,  0	# IORegister[$imm1+$zero]=$imm2 (leds=0)
	out $imm2, $imm1, $zero, 9, 0xFFFF	# leds = 0x0000FFFF

SET0:						# set to copy sector 0
	add $s1,   $zero, $imm1, 0,    0	# $t1 = 0 (disksector = 0)
	add $s2,   $zero, $zero, 0,    0	# $t2 = 0 (diskbuffer = 0)
	out $imm1, $imm2, $zero, SET1, 7	# IORegister[7] = PC(SET1)
	
SET1:						# set to copy sector 1
	add $s1,   $zero, $imm1, 1, 0		# $t1 = 0 (disksector = 0)
	add $s2,   $zero, $zero, 0, 0		# $t2 = 0 (diskbuffer = 0)
	out $imm1, $imm2, $zero, COPIED, 7	# IORegister[7] = PC(SET1)

COPIED:					# finished copying, compare
	add $s0, $zero, $zero, 0, 0		# i = 0
	add $t1, $s1, $imm1, 
	add $s0, $zero, $imm2, 0, 1		# i++

SCOPY:						# start disk[$t1]->MEM[$t2]
	in  $t0,   $imm1, $zero, 17, 0	# $t0 <- IORegister[$imm1]. $t0=diskstatus
	bne $imm1, $t0,   $zero, FAIL, 0	# abort if busy
	out $s1,   $imm1, $zero, 15, 0	# disksector = $t1
	out $s2,   $imm1, $zero, 16, 0	# diskbuffer = $t2
	out $imm1, $imm1, $zero, 14, 1	# diskcmd = 1 (read)


ISNEQ:						# sectors not equal
	in $t0, $imm1, $zero, 9, 0		# $to = leds
	sll $t0, $t0, $imm1, 4, 0		# $to <<= 4
	out $t0, $imm1, $zero, 9, 0		# leds = $to
ISEQ:						# sectors are equal
						# nop

FAIL:
HALT:
	halt $zero, $zero, $zero, 0, 0
