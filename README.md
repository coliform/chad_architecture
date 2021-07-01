# CHAD

Assembler & simulator for an ISA called CHAD


## folders

### src/asm

The assembler is responsible for converting .asm files into machine code


### src/sim

The simulator is responsible for simulating a CHAD processor environment


### tests

Contains test files - disktest, summat

Each test has its own separate folder.


## make cheatsheet

`make sim`

`make asm`

`make sim_test`

`make asm_test`

`NAME=disktest make test`