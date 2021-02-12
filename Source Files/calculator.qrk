// This would turn Der Blinkenrechner into a calculator

//Note: Make hacksembler check for ISR without return statement

#HACKSEMBLER IF_DEFAULT 1 //Note: Add hacksembler directives
#HACKSEMBLER TRUE_DEFAULT 1 //Note: Add hacksembler directives

//Declare variables
@stack_x = 0;
@stack_y = 0;
@stack_z = 0;
@stack_t = 0;
@val_keyA = 'A'; //NOTE: Must add char to number support
@val_10 = 10;
@val_1 = 1;
@key_starts_new_word = true;

//ISR Runs for keypress '1', adds one to value
#ISR keypad_1 { //NOTE: Must add ISR keyword

	IF ( @key_starts_new_word ){ //Create new X value

		//Overwrite x-stack with '1'
		RAM_CPY @val_1 @stack_x;

	}ELSE{ //Add to current value

		//Multiply number by 10
		RAM_FPUA @stack_x;
		RAM_FPUB @val_10;
		FPU_MULT

		//Add value '1'
		FPUC_FPUA
		RAM_FPUB @val_1
		FPU_ADD

		//Save to x
		FPUC_RAM @stack_x
	}

	//Display
	RAM_DISPX @stack_x

	RTN;
}

//ISR Runs for keypress 'enter'
#ISR keypad_ENT { //NOTE: Must add ISR keyword

	//Specify new keypress starts new number
	@key_starts_new_word = true; //NOTE: Make TRUE a keyword

	RAM_CPY @stack_z @stack_t; //Copies from first to second
	RAM_CPY @stack_y @stack_z;
	RAM_CPY @stack_x @stack_y;

	RTN;
}
