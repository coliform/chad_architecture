#!/usr/bin/env python3

opcode_names=["add", "sub", "and", "or", "sll", "sra", "srl", "beq", "bne", "blt", "bgt", "ble", "bge", "jal", "lw", "sw", "reti", "in", "out", "halt"]
register_names=["zero", "imm1", "imm2", "v0", "a0", "a1", "a2", "t0", "t1", "t2", "s0", "s1", "s2", "gp", "sp", "ra"]

s = ""
with open('trace.txt', 'r') as f: s = f.read()
s=s.replace("\r","").split("\n")[:-1]

ins = []
for line in s:
	line = line.split(" ")
	for i in range(len(line)): line[i] = "0x"+line[i]
	line[0] = str(int(line[0],16))
	line[1] = opcode_names[(int(line[1],16)&(0xFF<<40))>>40]
	for i in range(2,len(line)): line[i]=str(int(line[i],16))
	line = '\t'.join(line)
	ins.append(line)

s='\n'.join(ins)
with open('trace2.txt','w') as f: f.write("PC	INST	R0	R1	R2	R3	R4	R5	R6	R7	R8	R9	R10	R11	R12	R13	R14	R15\n"+s)
