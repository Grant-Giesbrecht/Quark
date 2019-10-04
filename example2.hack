//Perihelion language - high level language with subroutines and abstracted memory
//
// * Semicolons ignored
// * Whitespace (mostly ignored - used to distinguish words, not commands)
// * ^ means next word is name of subroutines
// * All contents (separated by comma) in parentheses replace the contents of the
//   parenthesis in the subroutine definition
// * #SUBROUTINE used to indicate subroutine is about to be defined
// * @ used to indicate next word is name of abstracted memory location
// * = and : can be used interchangeably for readability
// * Comments made with dual forward slash

//Define initial variables
@f = 1e3;
@C = 1e-12;
@pi = 3.1415926535;

@val_1 = 1;
@val_2 = 2;

//************* Main Program body **************
//	calculate: print 1/(2*pi*f*c)

//Multiply 2*pi*f*C
RAM_REGA @val_2
RAM_REGB @pi
MULT
^CHAIN_MULT(@f);
^CHAIN_MULT(@C);

//Calculate inverse
REGC_RAM @temp
RAM_REGB @temp
RAM_REGA @val_1

//Display result
REGC_DISPA

//*************** Define Subroutines *************

//DEFINE SUBROUTINE
#SUBROUTINE CHAIN_MULT(x){
	REGC_REGA
	RAM_REGB x
	MULT
}
