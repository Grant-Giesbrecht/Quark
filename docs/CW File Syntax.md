# CW File Syntax

## Purpose

The CW file format translates signal names to control word/pin addresses. It
also defines default states and whether they are logic high or low.

## Syntax

Note: <> indicates the text inside is replaced with the value, and () indicates an optional argument.
 * Default state represented using `OFF` or `ON`.
 * Optional `!` after default state indicates that control line uses negative logic
 * Anything after `//` interpreted as a _comment_.


`<Control Line Name>    <Word Number>:<Pin Number>       <Default State>(!)`

**Example:**

	//ALU OPERATIONS
	RSHIFT			2:0			OFF
	LSHIFT			2:1			OFF!
	FLSH_OPC8 		5:1			ON



## ToDo

* Adding a way to say if it writes to a bus, and the bus name, would allow me to
verify that a dual-write condition is not occuring.
