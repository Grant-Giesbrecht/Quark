//Perihelion language - high level language with subroutines and abstracted memory
//
// * Semicolons ignored
// * Whitespace (mostly ignored - used to distinguish words, not commands)
// * ^ means next word is name of subroutines
// * All contents (separated by comma) in parentheses replace the contents of the
//   parenthesis in the subroutine definition
// * #SUBROUTINE used to indicate subroutine is about to be defined
// * @ used to indicate next word is name of abstracted memory location
// * = and : can be used interchangably for readability
// * Comments made with dual forward slash

@alpha = 12;
@beta: 14;

^DISP_ADD (@alpha, @beta);



//DEFINE SUBROUTINE
#SUBROUTINE DISP_ADD(x, y){

	RAM_REGA x;
	RAM_REGB y;
	ADD;
	REGC_DISPA;

}
