#ifndef H_CHAD_SIM
#define H_CHAD_SIM

#include <chad_utils.h>

#define SIZE_HDD_SECTORS_H		128
#define SIZE_HDD_SECTORS_W		512

#define zero		0
#define imm1		1
#define imm2		2
#define v0		3
#define a0		4
#define a1		5
#define a2		6
#define t0		7
#define t1		8
#define t2		9
#define s0		10
#define s1		11
#define s2		12
#define gp		13
#define sp		14
#define ra		15

#define irq0enable		0
#define irq1enable		1
#define irq2enable		2
#define irq0status		3
#define irq1status		4
#define irq2status		5
#define irqhandler		6
#define irqreturn		7
#define clks			8
#define leds			9
#define display		10
#define timerenable		11
#define timercurrent		12
#define timermax		13
#define diskcmd		14
#define disksector		15
#define diskbuffer		16
#define diskstatus		17
#define reserved_1		18
#define reserved_2		19
#define monitoraddr		20
#define monitordata		21
#define monitorcmd		22

#endif
