// OPERATORS
//
// Standard Operators (OP)
//		: Address, value operator (direct write operator)
//
//	Subroutine Operators (Call a subroutine specified by OSR+) (SOP)
//		+
//		-
//		*
//		/
//		^
//		%
//		|
//		&

// KEYWORDS
//
// Standard Keywords:
// 		subroutine
//		macro
//		expand
//		true
//		false
//
//	Option Keywords: Keywords pertaining to system hardware
//		FLASH0
//		FLASH1
//		RAM0
//		zero
//		nzero
//		carry
//		ncarry
//
// Type Keywords:
//		int		(int8)
//		float	(float8)
//		addr	(addr8)
//		bool
//		float32
//		addr32
//		int32
//		type8
//		type16
//		type32
//		(arg)


// Compiler Directives:
//		#ARCH <str>	Specify target architecture
//		#SERIES <#>.<#>.<#>	Specify target ISV series
//		#ISV <#>.<#>.<#> Specify target ISV minimum version
//		#ISV_EXACT <#>.<#>.<#> Specify target ISV exact match requirement
//		#PMEM <str>	Default program memory location
//		#TRUEVAL <#> Specify mapping of 'true' keyword
//		#FALSEVAL <#> Specify mapping of 'false' keyword
//		#INCLUDE <str> Copy and paste file contents to top of file
//		#SUBOPERATOR <str-operator> <str-sub-name> Tell compiler what subroutine to use for the specified operator

// Limitations:
//		No strings so far
//		No lists so far

// Symbols:
//		=	Either AML initialization or PMEM -> RAM0
//		;	Line end
//		()	Argument list
//		#	Compiler directive indicator
//		// 	Comment identifier
//		{}
//		@	Manual declaration symbol



 Quark language
//
// Keywords:
// 		subroutine
//		macro
//		expand
//		true
//		false
//
//	Option Keywords:
//		FLASH0
//		FLASH1
//		RAM0
//		zero
//		nzero
//		carry
//		ncarry
//
// Type Keywords:
//		int
//		addr
//		bool
//		float32
//		addr32
//		int32
//		(arg)
//
// Symbols:
//		=	Either AML initialization or PMEM -> RAM0
//		;	Line end
//		()	Argument list
//		#	Compiler directive indicator
//		// 	Comment identifier
//		{}
//
// Compiler Directives:
//		#ARCH	Specify target architecture
//		#SERIES	Specify target ISV series
//		#ISV	Specify target ISV minimum version
//		#ISV_EXACT	Specify target ISV exact match requirement
//		#PMEM	Default program memory location
//		#TRUEVAL Specify mapping of 'true' keyword
//		#FALSEVAL Specify mapping of 'false' keyword
//



//
//
// Data Formats:
//		0b<#,#>	Binary, ex: 0b0000,1011 or 0b00001011
//		0x<#>	Hex, ex: 0xFF
//		<#>		Decimal, ex: 12
//		'<N>'	Char, translates character to number via charmap, ex 'a'
//
// Quark-0.0 standard, as defined on 7-11-2021

// Arch, series and ISV statements are all required
#ARCH Matterhorn821	// What hardware is it designed for?
#SERIES Bergheim	// What ISV series is used?
#ISV 0.0.4			// Minimum ISV allowed? (can replace any digit w/ x for wildcard ex. 0.0.x)
#ISV_EXACT 0.0.4	// Can be used to specify exactly this ISV version is required. Incompatable with '#ISV'

//
#PMEM FLASH0

//Define initial variables
int f = 1e3;		//Declare 8 bit AML with 'int' or 'addr'. Both do same thing
int C = 1e-12;
int pi = 3.1415926535;

int x = 12 @ &x = 4;

int val_1 = 1;
int val_2 = 2;
addr jump_loc; // Addr defines an 8 bit address without telling the compiler to assume its an integer
bool lights_on = false;

//************* Main Program body **************
//	calculate: print 1/(2*pi*f*c)

//Multiply 2*pi*f*C
RAM_REGA val_2;	// All variables are AMLs, which are similar to C pointers. Accordingly, the address operator (&) is not required. Conversely, the dereference operator (*) says to use the value instead of the address.
RAM_REGB pi;
MULT
expand CHAIN_MULT(f);
expand CHAIN_MULT(C); // The expand keyword tells a macro to be expanded here

float32 f = 1e3;		// Declare 32 bit AML with 'float32'. 'int32' fine also, just now compiler can run checks
float32 C = 1e-12;
float32 prod;
RAM_REGD f
RAM_REGE C
MULT
REGF_RAM prod

int a;
*a = 12; // Some common instruction are abstracted from assembly and can use 'common' syntax. For example, this equals is shorthand for PMEM -> RAM0 (assuming all AMLs saved in RAM0).

//Calculate inverse
REGC_RAM temp
RAM_REGB temp
RAM_REGA val_1

//Display result
REGC_DISPA

// IFZERO{
// 	RPLAN 0 //Reprogrammable light on
// }ELSE{
// 	RPAUS 0 //Reprogrammable light off
// }

IFZERO CLAUSE_IF	#HERE start_if
//else stuff
SUB
JUMP END_IF
#HERE CLAUSE_IF
//if stuff
ADD
#NEXT END_IF
//
// IFCARRY{
// 	RPLAN 1 //Reprogrammable light on
// }ELSE{
// 	RPAUS 1 //Reprogrammable light off
// }
//
// WHILEZERO{
// 	//things
// 	ADD
// }


//address @start_loop refers to this add instruction. This is for jump instructions. Putting #HERE allows you to name a line with an AML
ADD #HERE start_loop
//things
// IFZERO{
// 	JUMP start_loop
// }

//*************** Define Subroutines *************

// //DEFINE SUBROUTINE
// #SUBROUTINE CHAIN_MULT(x){
// 	REGC_REGA
// 	RAM_REGB x
// 	MULT
// }

// Macros can be defined with the 'macro' keyword. They are like crude functions
// that instead of using the stack and returns, just copy and paste a block of code
// with optional substitutions. They are inserted into the program with 'expand'
// statements
macro cpu_lights_on(){
	// Do things
}

alu_multiply(f, c, x); // Subroutines are called just like in C

// Define functions/subroutines with the 'subroutine' keyword. No return type
// exists for subroutines because if they do return a value, it will just be
// stored in a specific register or memory location
subroutine alu_multiply(int x, addr32 y, arg X){ //Types *can* be provided in the arugment list, but can also be skipped using the 'arg' keyword



	return; // The return instruction must be last
}

// While loops are defined with the 'while' keyword. They take one argument, which
// is the conditional type. Options include: zero, nzero, carry, ncarry
while(nzero){



}

// If statements are defined using the 'if' keyword and take one argument, which
// is the conditional type. Options include: zero, nzero, carry, ncarry. Else
// can be used as a negative condition
if(zero){

}else{

}
