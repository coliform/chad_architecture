# write tests:


## summat.asm

sums 4x4 mats

mat1 in MEM[0x100] to 0x10F, mat2 in MEM[0x110] to 0x11F

result = mat1 + mat2 in 0x120 to 0x12F

assume no overflow (values < 2^32)

a11 in 0x100, a12 in 0x101, a21 in 0x104


## triangle.asm

prints (to the monitor) a mashush

base is horizontal, centered, in line 128

its length, height shall be specified in 0x100,0x101 respectively

assume it fits in the screen (height <= 128+1)


## clock.asm

a clock on the 7-segment display

format is HHMMSS (in decimal)

starts in 075955, ends in 080005


## disktest.asm

checks if sector 0 == sector 1

if so, led &= 0x0000FFFF

else,  led &= 0xFFFF0000
