# Quark Subatomic Programming

## Introduction

Quark is a programming language designed for homemade CPUs. It accepts instruction
set definitions from ASCII files (.isd & .cw) and creates binary programs for
that architecture.

## Included Command Line Tools

* *qmm:* Quark Minus Minus Compiler. Compiler for the extremely simple q--
language used before Quark was fully operational. Operates on .q-- files.
* *quark:* Quark compiler. Compiler for the quark programming language.
* *isvgen:* ISV Generator. Creates instruction set version lookup tables from
ASCII source files (.isd & .cw). Saves table in .lut ASCII file, which can be
uploaded to a CPU control unit lookup table.
* *isvpub:* ISV Publisher. Saves the most recent .isd, .cw, and .lut files to a
directory under a specific version number. Used to archive versions of instruction
sets.
