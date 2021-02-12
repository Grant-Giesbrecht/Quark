# Syntax

## Comments:

* Everything after `//` is a comment and is ignored by the compiler. This works both for comment lines and inline comments.

## Directives



## While Statements:

* While-statements are collapsed into equivalent if-statements.
* Two types of while statements exist:
 * `WHILEZERO`: Continues to loop while the zero flag is set.
 * `WHILECARRY`: Continues to loop while the carry flag is set.
* A block opening character `{` must follow the while keyword on the same line.
 * Note: Switching to whileNzero and whileNcarry is probably a good idea.

## Blocks:

Blocks are blocks of code enclosed in curly brackets that are used in if statements, while statements, and subroutines.

## If Statements:

Fill this in later.

## Abstracted Memory Locations:

Abstracted memory locations indicate locations in memory that are not explicitly defined (ie. assigned numeric values) until the last stage of compilation. This allows them to be used for variable names or line numbers.

* Variable analogs can be declared by `@` followed by the variable name. A value can then be assigned to it.

* Declared abstracted line numbers with `#HERE`. Indicates that the next line's line number will be jumped to by the abstracted memory location specified by the following abstract memory location name.

* You can specify the location of an abstract memory location using the `#REQLOC` command. If the compiler cannot fit it in that location, an error will be displayed.

_NOTE_: AMLs beginning with 'TRUE_IF_' are interpreted by quark as components of an expanded 'if' statement. As such, they will be recognized as acceptable locations to insert a gap between two contiguous blocks.

## Block Substitutions:

Block Substitutions (Blocksubs) are a poor-man's subroutine. They're find and replace style code blocks.

### Blocksub Definition:

* A blocksub is defined by starting a line with `#BLOCKSUB`. It must be followed by the function name, then opening and closing parentheses. Anything within the parentheses will be interpreted as blocksub arguments, with commas separating arguments. An opening block character `{` must be on the same line.

### Interrupt Service Routine (ISR) Declarations:

An ISR is a specific type of subroutine which is triggered by an interrupt.

* It needs to have a return (`RTN`) command as the last statement.
* It most likely needs an explicitly defined starting location. This can be achieved by giving it an abstract memory location (using `#HERE @<location_name>`), then requesting the AML be given a specific address using `#REQLOC`.

### Blocksub Calls:

* Blocksubs are called by writing the name preceeded by a carat `^`. The arguments must be in parentheses.

## Acceptable Names for Variables, Subroutines:

Fill this in later.

## List of Compiler Option Directives:

* `PROGRAM_MEMORY`: Program memory value (string)
* `TRUE_VALUE`: Integer value for 'TRUE' condition in conditional checks. Also used as replacement for 'True' keyword.
* `FALSE_VALUE`: Integer value for 'FALSE' keyword. Note this is not used in conditional checks, as false is defined logically as 'not true'.
* `ADD_MISSING_HALT`: boolean value. If true, compiler adds a `HALT` instruction at end of program if not present.

Example: `#OPTION FALSE_VALUE 14`


## List of Symbols and Keywords:

* `=`: Assigns the numeric value on the RHS to the specified value (LHS must be a memory location, including abstract memory locations).
* `{}`: Open an closing brackets of a code block.
* `^`: Indicates the word is a blocksub call.
* `@`: Indicates the word is an abstract memory location.
* `#HERE`: Followed by abstract memory location, says next command is located on line given by specified abstract memory location.
 * `#HERE @aml_name;ADD` Makes AML 'aml_name' refer to line of instruction `ADD`.
* `#BLOCKSUB`: Declares a block-substitution
* `#ISR`: Declares a subroutine with ISR requirements.
* `#OPTION`: Compiler option directive. Followed by option name and value.

## ISV

* ISV series name is a string, it must be contiguous (no whitespace), and cannot have periods within.
