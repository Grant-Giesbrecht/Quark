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

**Full Example:**

	#SERIES Alpha
	#ARCH Z3

	* JUMP_TRUE 17 2
	? When a program conditional jump evaluates to true, this is called.
	?
	#PRGM JUMP_CARRY
	0:PC0_OE FLSH_MAR0
	1:PC8_OE FLSH_MAR8 FLSH_OPC0=OFF FLSH_OPC8=ON FLSH_TRIG
	2:REGINS_WR FLSH_CLEAR KEY_INT_ENABLE PC_CLK
	3:PC0_OE FLSH_MAR0
