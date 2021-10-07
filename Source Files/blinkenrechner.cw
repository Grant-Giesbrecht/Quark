// Control Wiring File for Blinkenrechner
//
//
// G. Giesbrecht
// 15-8-2021

// Notes:
// * (BHM-159,10 Inverted): This signal would be (-) logic, except BHM-159,10
//   caused this to be inverted between Z-III and Sys.B.

//***************** Word 0 *****************

DT2S0_ADDR_WR		0:0		OFF
DT2S0_DATA_BUF_WR	0:1		OFF
FLASH0_TRIG_RAW 	0:2		OFF	//Note: *(BHM-159,10 Inverted)
FLASH1_TRIG_RAW 	0:3		OFF	//Note: *(BHM-159,10 Inverted)
DT2S1_ADDR_WR		0:4		OFF
DT2S1_DATA_BUF_WR	0:5		OFF
SR1_MAR_WR			0:6		OFF
RAMS1_WR			0:7		OFF	//Note: *(BHM-159,10 Inverted)

//***************** Word 1 *****************

SR0_MAR_WR			1:0		OFF
RAMS0_WR			1:1		OFF	//Note: *(BHM-159,10 Inverted)
FCP_WR_A0			1:2		OFF
FCP_WR_A1			1:3		OFF
FCP_WR_A2			1:4		OFF
FCP_WR_A3			1:5		OFF
FCP_WR_B0			1:6		OFF
FCP_WR_B1			1:7		OFF		//TODO: HANDLE 'TIED' WIRES DIFFERENTLY

//***************** Word 2 *****************

FCP_WR_B2			2:0		OFF
FCP_WR_B3			2:1		OFF
FCP_OP_TRIG			2:2		OFF
TSTOP0_WR			2:3		OFF
PC_CLK				2:4		OFF
REG_AA0_WR			2:5		OFF
REG_AA8_WR			2:6		OFF
CACHE_A_WR			2:7		OFF

//***************** Word 3 *****************

CACHE_B_WR			3:0		OFF
INST_REG_WR			3:1		OFF
REG_TIMER_ISR_WR	3:2		OFF
REG_IRTN_WR			3:3		OFF
REG_KEY_ISR_WR		3:4		OFF
REGA_WR				3:5		OFF
REGB_WR				3:6		OFF
REGC_WR				3:7		OFF

//***************** Word 4 *****************

DISP_CTRL0			4:0		OFF
DISP_CTRL1			4:1		OFF
DISP_CTRL2			4:2		OFF
DISP_CTRL3			4:3		OFF
DISP_CTRL4			4:4		OFF
DISP_CTRL5			4:5		OFF
DISP_CTRL6			4:6		OFF
DISP_CTRL7			4:7		OFF

//***************** Word 5 *****************

EXPS0_CTRL0			5:0		OFF
EXPS0_CTRL1			5:1		OFF
EXPS0_CTRL2			5:2		OFF
EXPS0_CTRL3			5:3		OFF
EXPS0_CTRL4			5:4		OFF
EXPS0_CTRL5			5:5		OFF
EXPS0_CTRL6			5:6		OFF
EXPS0_CTRL7			5:7		OFF

//***************** Word 6 *****************

EXPS1_CTRL0			6:0		OFF
EXPS1_CTRL1			6:1		OFF
EXPS1_CTRL2			6:2		OFF
EXPS1_CTRL3			6:3		OFF
EXPS1_CTRL4			6:4		OFF
EXPS1_CTRL5			6:5		OFF
EXPS1_CTRL6			6:6		OFF
EXPS1_CTRL7			6:7		OFF

//***************** Word 7 *****************

EXPS1_CTRL8			7:0		OFF
EXPS1_CTRL9			7:1		OFF
EXPS1_CTRL10		7:2		OFF
_PMEM_TRIG			7:3		OFF! //TODO: HOW DOES PMEM_TRIG WORK?
_FCP_OE_C0			7:4		OFF!
_FCP_OE_C1			7:5		OFF!
_FCP_OE_C2			7:6		OFF!
_FCP_OE_C3			7:7		OFF!

//***************** Word 8 *****************

_SYSREG_OE			8:0		OFF!
SYSREG_WR			8:1		OFF
_MODREG_OE			8:2		OFF!
MODREG_WR			8:3		OFF
_CACHE_C_OE			8:4		OFF!
CACHE_C_WR			8:5		OFF
_CACHE_D_OE			8:6		OFF!
CACHE_D_WR			8:7		OFF

//***************** Word 9 *****************

FLAG_ZERO_SET		9:0		OFF
FLAG_CARRY_SET		9:1		OFF
RAMS1_OE			9:2		OFF			// Previously named _RAM_S1_OE
FLAG_TIMER_INT_CLR	9:3		OFF
FLAG_IN_ISR_SET		9:4		OFF
FLAG_IN_ISR_CLR		9:5		OFF
_WRITE_PC			9:6		OFF!
_HCF_SET			9:7		OFF!

//***************** Word 10 *****************

_REG_AD8_OE			10:0	OFF!
REG_AD_WR			10:1	OFF
_REG_AD0_OE			10:2	OFF!
_STK_WR				10:3	OFF!	//TODO: WHY IS IT NEGATIVE?
STK_UP				10:4	OFF
STK_DN				10:5	OFF
_STKREG_OE			10:6	OFF!
TSTOP8_WR			10:7	OFF

//***************** Word 11 *****************

_ADD_EN			11:0	OFF!
CARRY_IN			11:1	OFF
INV_B				11:2	OFF
_BITOR_EN			11:3	OFF!
_BITAND_EN			11:4	OFF!
_BITXOR_EN			11:5	OFF!
_LSHIFT_EN			11:6	OFF!
_RSHIFT_EN			11:7	OFF!

//***************** Word 12 *****************

DT2S0_CPU_WR_EN		12:0	OFF
DT2S1_CPU_WR_EN		12:1	OFF
DT2SN_OPCD0			12:2	OFF
DT2SN_OPCD1			12:3	OFF
KEY_CLR_KEY_FLAG	12:4	OFF
_KEY_OE				12:5	OFF!
FCP_OP_0			12:6	OFF
FCP_OP_1			12:7	OFF

//***************** Word 13 *****************

FCP_OP_2			13:0	OFF
FCP_OP_3			13:1	OFF
FCP_OP_4			13:2	OFF
FCP_OP_5			13:3	OFF
FLAG_PMEM_RAM_SET	13:4	OFF
FLAG_PMEM_RAM_CLR	13:5	OFF
_FLAG_PMEM_EXT_SET	13:6	OFF!
_FLAG_PMEM_EXT_CLR	13:7	OFF!

// FLAG_PMEM_RAM_SET	13:0	OFF
// FLAG_PMEM_RAM_CLR	13:1	OFF
// _FLAG_PMEM_EXT_SET	13:2	OFF!
// _FLAG_PMEM_EXT_CLR	13:3	OFF!
// ERR_SET_BADINSTRUC	13:4	OFF
// ERR_SET_PRGM		13:5	OFF
// 					13:6
// 					13:7

//***************** Word 14 *****************

ERR_SET_BADINSTRUC	14:0	OFF
ERR_SET_PRGM		14:1	OFF
_REG_AA_OE			14:2	OFF!
_CACHE_A_OE			14:3	OFF!
_CACHE_B_OE			14:4	OFF!
_PC_OE				14:5	OFF!
IS_FETCH			14:6	OFF
RAMS0_OE			14:7	OFF	// Previously called _CU_RAM_OE, previously active low

//***************** Word 15 *****************

_CU_FLASH_OE		15:0	OFF!
_CU_EXTFLASH_OE		15:1	OFF!
CU_PMEM_OE			15:2	OFF
_REG_TIMER_ISR_OE	15:3	OFF!
_REG_IRTN_OE		15:4	OFF!
_REG_KEY_ISR_OE		15:5	OFF!
_REGC_OE			15:6	OFF!
PHI_RESET			15:7	OFF
