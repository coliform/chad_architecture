#ifndef H_CHAD_SIM
#define H_CHAD_SIM

#include "chad_utils.h"

// 128 sectors, 512 bytes per sector = 4096 bits
#define DISK_SECTOR_COUNT				128
#define DISK_SECTOR_SIZE_BYTES			512
#define DISK_FILE_ROW_LENGTH			32
#define DISK_SECTOR_SIZE_BITS			SECTOR_SIZE_BYTES*8
#define DISK_FILE_ROW_BITS				DISK_FILE_ROW_LENGTH*4
#define DISK_FILE_ROW_BYTES				(int)(DISK_FILE_ROW_BITS/8)
#define DISK_FILE_ROWS_PER_SECTOR		(int)(DISK_SECTOR_SIZE_BYTES/DISK_FILE_ROW_BYTES)
#define DISK_FILE_MAX_ROWS				DISK_FILE_ROWS_PER_SECTOR*DISK_SECTOR_COUNT
// disk is of type uint32 (DISK_FILE_ROW_BITS==32)
// per sector (4096 bits) we have 128 of those
#define SIZE_HDD_SECTORS_H		128
#define SIZE_HDD_SECTORS_W		128
#define SIZE_HDD_SECTOR_CELL	4

#define SIZE_MONITOR_H			256
#define SIZE_MONITOR_W			256
#define SIZE_MONITOR	SIZE_MONITOR_H*SIZE_MONITOR_W

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
