// This is the wiring specification for MEMORY DELTA
//
// Created for Program1.pbin on 22-7-20202

// *************** WORD 0/TAKES WORD 7'S CLOCK *****************************************//
//      **** REGISTERS ******
//
// NOTE: CPU W0P0 matches CU W0P0

REGC_OE			7:0			OFF!
REGC_WR			7:1			OFF
REGB_WR			7:2			OFF
REGA_WR			7:3			OFF
REGRTN8_OE		7:4			OFF!
REGRTN0_OE		7:5			OFF!
REGRTN8_WR		7:6			OFF
REGRTN0_WR		7:7			OFF

// *************** WORD 1 *****************************************//
//      **** MEMORY ACCESS, INSTRUCTION REG ******
//
// NOTE: CPU W1P0, W1P5 matches CU W1P6, W1P1 - Orientation flipped
//		 only middle 6 pins used

// NOTE: W1P0 unused
KEY_INT_ENABLE	1:1			OFF
RAM_OE			1:2			OFF!
FLASH_OE		1:3			OFF! //WARNING: This might not work
EXTFLASH_OE		1:4			OFF! //WARNING: This might not work
PMEM_OE			1:5			OFF
REGINS_WR		1:6			OFF
// NOTE: W1P7 unused


// *************** WORD 2 *****************************************//
//      **** ALU OPERATIONS ******
//
// NOTE: CPU W2P0 matches CU W2P0

RSHIFT			2:0			OFF!
LSHIFT			2:1			OFF!
BITXOR			2:2			OFF!
ADD				2:3			OFF!
BITOR			2:4			OFF!
INV_B			2:5			OFF
CARRY_IN		2:6			OFF
ADDER_EN		2:7			OFF!

// *************** WORD 3 *****************************************//
//      **** FLAG CONTROLS ******
//
// NOTE: CPU W3P0 matches CU W3P0

FLGCRY_CLR		3:0			OFF
FLGCRY_SET		3:1			OFF
FLGZRO_CLR		3:2			OFF
FLGZRO_SET		3:3			OFF
FLGPME_CLR		3:4			OFF  //FLAG PMEM EXTERNAL FLASH
FLGPME_SET		3:5			OFF  //FLAG PMEM EXTERNAL FLASH
FLGPMR_CLR		3:6			OFF  //FLAG PMEM RAM
FLGPMR_SET		3:7			OFF  //FLAG PMEM RAM

// *************** WORD 4 *****************************************//
//      **** PHASE COUNTER, PROG COUNTER ******
//
// NOTE: swapped - CPU W4P0 matches CU W4P7

PHSC_CLK		4:0			OFF	//PHASE COUNTER CLOCK
PHSC_RST		4:1			OFF //PHASE COUNTER RESET
PC0_OE			4:2			OFF!
PC8_OE			4:3			OFF!
PCO_WR			4:4			OFF
PC8_WR			4:5			OFF
PC_WR			4:6			OFF!
PC_CLK			4:7			OFF

// *************** WORD 5 *****************************************//
//      **** FLASH CONTROL PINS ******

FLSH_OPC0 		5:0			OFF
FLSH_OPC8 		5:1			ON
FLSH_TRIG 		5:2			OFF!
FLSH_CLEAR		5:3			OFF! 	//Flash: clear read pause
FLSH_DATA		5:4			OFF
FLSH_MAR0		5:5			OFF
FLSH_MAR8		5:6			OFF
HALT			5:7			OFF

// *************** WORD 6 *****************************************//
//      **** FLASH CONTROL PINS & ******

WAIT_FLSH		6:0			OFF		//HALT unit flash READ turns on
FLSH_OE			6:1			OFF!
TEMPA_OE		6:2			OFF!
TEMPB_OE		6:3			OFF!
TEMPA_WR		6:4			OFF
TEMPB_WR		6:5			OFF

// XXXXXXXXXXXXXXX WORD FALSE XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

RAM_MAR0		7:0			OFF
RAM_MAR8		7:1			OFF
RAM_WR			7:2			OFF
RAM_OE			7:3			OFF
