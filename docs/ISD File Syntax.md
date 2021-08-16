# ISD File Syntax

## Purpose

The ISD file format describes each phase of each instruction, thus defining the
computer's instruction set (hence the name, 'Instruction Set Descriptor' file).

## Syntax

Note: <> indicates the text inside is replaced with the value, and () indicates an optional argument.
* Anything after `//` interpreted as a _comment_.
* There are four types of lines, other than comments.
1. Instruction declarations: Must come before the remaining types of lines.

 `* <Instruction Name> <Operation Code (in decimal format)>` <Number of data bytes>`

 **Ex:**

`* RAM_REGA 16 2`

 2. _Instruction Descriptors:_ Everything on a line following a question mark `?`
 is counted as a descriptor. This is simply text to help the programmer understand
 what the instruction does.
 3. _Program Instruction Mapping:_ Some instructions do not match between the LUT
 and the program source. These are called special instructions (see Blinkenrechner
 documentation for an exmplaination), and the same operation code will have a
 different name and meaning on the progra side and the LUT side.

 `#PRGM <Program Inst. Name>`

 4. _Phase Control State Descriptor:_ Describes how to configure the control lines
 at each phase of the instruction.

 `<phase>: <control_line> (= <ON/OFF>) (<repeat with as many control lines as req'd)`

 5. _ISV Series:_ Word after `#SERIES` specifies the ISV series.

 6. _Target Architecture:_ Word after `#ARCH` specifies the target architecture.


 7. _Subprocessor Declaration:_ If an instruction might produce a `CARRY` or `ZERO`
 flag, the CPU must know to check the flag from the ALU vs the FPU. Bit 6 of the
 instruction determines the flag source. By declaring an instruction as an FPU or
 ALU instruction, an error checker can verify that this rule is not broken.

 8. _Block Definition:_ When `#DEF` appears, followed on a later line by `#END`,
 a the inbetween lines are used as a drop in replacement for when `#REPL` is called.
 A block name must appear on the same line as `#DEF` and must be followed by the
 number of arguments if its is nonzero.

 8. _Replacement Block Arguments:_ Appear on the same line as normal
 `<Number>: <CTRL1> <CTRL2> ...` instructions in a replacement block, however `@`
 symbol indicates that the `@` is replaced by arguments provided during a call to
 `#REPL`

 9. _Replacement Calls:_ When `#REPL` appears, the line is replaced with the contents
 of a block definition.

**Full Example:**

	#SERIES Alpha
	#ARCH Z3

	#DEF NoArgsFoundHere
	6: CTRL_LINE1
	7: CTRL_LINE7
	#END

	#DEF ArgsFoundHere 3
	4: CTRL_LINE1 @ @
	5: CTRL_LINE2 @
	#END

	* JUMP_TRUE 17 2
	? When a program conditional jump evaluates to true, this is called.
	?
	#PRGM JUMP_CARRY
	0:PC0_OE FLSH_MAR0
	1:PC8_OE FLSH_MAR8 FLSH_OPC0=OFF FLSH_OPC8=ON FLSH_TRIG
	2:REGINS_WR FLSH_CLEAR KEY_INT_ENABLE PC_CLK
	3:PC0_OE FLSH_MAR0
	#REPL ArgsFoundHere CTRL_LINE3 CTRL_LINE4 CTRL_LINE5

	// The above '#REPL' line will be replaced with :
	//   4: CTRL_LINE1 CTRL_LINE3 CTRL_LINE4
	//	 5: CTRL_LINE2 CTRL_LINE5

	#REPL NoArgsFoundHere

	//The above line will be replaced with:
	//   6: CTRL_LINE1
	//   7: CTRL_LINE7
