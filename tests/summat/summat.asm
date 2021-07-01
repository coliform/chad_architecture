add $a1, $zero, $imm2, 0, 0x100		# $a1=mat1 address
add $a2, $zero, $imm2, 0, 0x110		# $a2=mat2 address
add $v0, $zero, $imm2, 0, 0x120		# $v0=address of result
add $s0, $zero, $imm2, 0, 0		# $s0 = 0 (counter)

LOOP: # loop, add while looping
	lw  $t1, $a1, $s0, 0, 0		# $t1 = mat1[counter]
	lw  $t2, $a2, $s0, 0, 0		# $t2 = mat2[counter]
	add $t0, $t1, $t2, 0, 0		# result = $t0 = $t1 + $t2
	sw  $t0, $v0, $s0, 0, 0		# MEM[$v0(mat3)+$s0(counter)]=$t0
	add $s0, $s0, $imm2, 0, 1	# $s0++
	bne $imm2, $s0, $imm1, 16, LOOP	# jump to LOOP if $s0 != 16

halt $zero, $zero, $zero, 0, 0

# REMEMBER TO SET WORDS
.word 0x100 2
.word 0x110 4
.word 0x10F 420