add $a1, $zero, $imm2, 0, 0x100		# $a1=mat1 address
add $a2, $zero, $imm2, 0, 0x10F		# $a2=mat2 address
add $v0, $zero, $imm2, 0, 0x120		# $v0=address of result
add $s0, $zero, $imm2, 0, 0			# $s0 = 0

LOOP: # loop, add while looping
	lw  $t1, $a1, $s0, 0, 0		# $t1 = mat1 + counter
	lw  $t2, $a2, $s0, 0, 0		# $t2 = mat2 + counter
	add $t0, $t1, $t2, 0, 0		# $t0 = $t1 + $t2
	sw  $t0, $v0, $s0, 0, 0		# store $t0 in res + counter
	add $s0, $s0, $imm2, 0, 1		# $s0++
	bne $imm2, $s0, $imm1, 16, LOOP	# jump to LOOP unconditionally

halt $zero, $zero, $zero, 0, 0

# REMEMBER TO SET WORDS
