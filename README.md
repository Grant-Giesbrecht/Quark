# Quark

A programming language for homebrew CPUs. The objective of this language is to
make programming in machine code as painless as possible. Rewritting a C
compiler for a homebrew CPU is a large undertaking. Quark allows you to program
in machine code while still providing structures such as variables, functions,
loops, and more to make the process more like using a high level language such
as C.

## Files

Quark uses a few different types of files:
* _Quark Source Code (.qrk)_: Source code to Quark compiler
* _Machine Code (.mc)_: List of instructions and their assoc. data corresponding
	directly to one CPU instruction each. Formatted with instruction codes not
	yet translated to binary. Must specify design ISV at top.
* _Machine Code Binary (.mcb)_: Same as above, except instructions and data must
	all have explicitly defined individual addresses, and all instructions and
	data are formatted in ASCII binary (1 and 0 characters, not binary file).
* _Instruction Set Source (.isf || .iss)_: Specifies the CPU's instruction set. This file
	is also used by the control matrix generator program. This file specifies
	how each control line must be set during each phase of each instruction,
	along with the number of data bits for the instruction, the instruction's
	binary representation, an optional description (and perhaps more).
* _Instruction Set Matrix (.isb || .ism)_:
