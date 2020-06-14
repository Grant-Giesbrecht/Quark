#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <IEGA/string_manip.hpp>
#include <IEGA/KTable.hpp>
#include <fstream>

//CXCOMPILE g++ hackomp.cpp -o hackomp -lIEGA -std=c++11
//CXCOMPILE ./hackomp hacksembly.hack -v
//CXGENRUN FALSE
//CXPRINTCOM TRUE

using namespace std;

typedef struct{
	string str;
	size_t lnum;
}line;

//PURGES
void purge_comments(vector<line>& program, bool verbose);

//EXPANSIONS
bool expand_while_statements(vector<line>& program, bool verbose, bool annotate);
bool expand_if_statements(vector<line>& program, bool verbose, bool annotate);
bool expand_subroutine_statements(vector<line>& program, bool verbose);

//OTHER
void print_program(vector<line>& program);
bool get_block_contents(vector<line>& block_contents, line input, int& blocks_open, bool ignore_first_open=false, bool annotate=false);

int main(int argc, char** argv){

	//**********************************************************************//
	//******************** GET USER OPTIONS ********************************//

	bool save_ahsm = false; //Tell compiler to save .AHSM file
	bool save_hsm = false;

	bool keep_comments = false;
	bool annotate = false;
	bool verbose = false;

	//Get input file's name
	if (argc < 2){
		cout << "ERROR: Requires .HACK file's name (no spaces allowed) as input." << endl;
		return -1;
	}
	string filename = argv[1];

	//If flags included...
	if (argc > 2){

		//Scan through flags
		for (int arg = 2 ; arg < argc ; arg++){
			if (string(argv[arg]) == "-ahsm"){
				save_ahsm = true;
			}else if(string(argv[arg]) == "-hsm"){
				save_hsm = true;
			}else if(string(argv[arg]) == "-c"){
				keep_comments = true;
			}else if(string(argv[arg]) == "-n"){
				annotate = true;
			}else if(string(argv[arg]) == "-v"){
				verbose = true;
			}else{
				cout << "WARNING: Ignoring unrecognized flag '" << argv[arg] << "'." << endl;
			}
		}

	}

	//Save HSM if not specified...
	if (!(save_hsm || save_hsm)){
		save_hsm = true;
	}

	//***********************************************************************//
	//************************* LOAD SOURCECODE *****************************//

	//Read file into 'program' vector. Keeps every non-blank line (incl. comments)
	vector<line> program;
	ifstream file(filename.c_str());
	if (!file.is_open()){
		cout << "ERROR: Failed to open file '" << filename << "'." << endl;
		return -1;
	}
	//
	string line_in;
	size_t line_num = 0;
	while (getline(file, line_in)) { //For each line...
		line_num++; //Increment line number
		if (line_in.length() == 0) continue; //Skip blank lines
		ensure_whitespace(line_in, "{"); //Ensure whitespace around certain characters
		remove_end_whitespace(line_in); //remove leading, trailing whitespace

		//Create 'line' struct
		line templine;
		templine.str = line_in;
		templine.lnum = line_num;

		program.push_back(templine); //Add line to program
	}

	//***********************************************************************//
	//*************************** EXPAND, COMPILE, ETC **********************//


	cout << "ORIGINAL: " << endl;
	if (verbose) print_program(program);

	if (!keep_comments) purge_comments(program, verbose);

	if (verbose && !keep_comments){
		cout << "\nCOMMENTS PURGED:" << endl;
		if (verbose) print_program(program);
	}

	expand_while_statements(program, verbose, annotate);

	cout << "\nWHILE STATMENTS EXPANDED:" << endl;
	if (verbose) print_program(program);

	expand_if_statements(program, verbose, annotate);

	cout << "\nIF STATEMENTS EXPANDED" << endl;
	if (verbose) print_program(program);






	// if (verbose) print_program(program);


	return 0;
}

//***************************************************************************//
//**************************** MAIN PROGRAM CODE ****************************//
//***************************************************************************//
//***************************************************************************//
//***************************************************************************//
//****************************** FUNCTION DEFINITIONS ***********************//
//***************************************************************************//


/*
Erases every line of 'program' which is a comment - ie. which starts
with '//' and removes inline comments.
*/
void purge_comments(vector<line>& program, bool verbose){

	size_t num_del = 0;
	size_t num_inline = 0;
	for (size_t i = 0 ; i < program.size() ; i++){ //For each line...

		size_t found = program[i].str.find("//"); //See if comment exists...
		if (found == string::npos){ //Go to next line if no comment marker...
			continue;
		}

		if (program[i].str.substr(0, 2) == "//"){ //If full line is comment...
			program.erase(program.begin()+i); //Erase line
			i--; //Decrement counter
			num_del++; //increment deletion count
		}else{ //Inline comment
			program[i].str = program[i].str.substr(0, found); //Erase comment
			num_inline++;
		}
	}

	if (verbose) cout << "Purged " << to_string(num_del) << " comment lines and " << to_string(num_inline) << " inline comments." << endl;

}

/*
Finds while statements and expands them into AHSM code. It will collapse the
while statements into IF-JUMP statements.
Syntax rules for while statements:
	* the while-keyword must be the first token in a line.
	* While-keywords:
	 	* WHILEZERO
		* WHILECARRY
	* block opening bracket ({) must immediately follow while-keyword and on same line
*/
bool expand_while_statements(vector<line>& program, bool verbose, bool annotate){

	size_t num_del = 0;
	size_t num_inline = 0;
	for (size_t i = 0 ; i < program.size() ; i++){ //For each line...

		if (program[i].str.substr(0, 9) == "WHILEZERO"){ //If beginning of line is WHILEZERO keyword

			//******************* ENSURE CORRECT SYNTAX ******************//

			//Parse string
			vector<string> words = parse(program[i].str, " \t");

			//Enure two words or more, and that second word is block-opening bracket
			if (words.size() < 2 || words[1] != "{"){
				cout << "ERROR: No block-opening bracket after WHILEZERO keyword. Line: " << to_string(program[i].lnum) << endl;
				return false;
			}

			//***************** GET BLOCK CONTENTS ************************//

			//Read first line...
			int blocks_open = 1;
			vector<line> block_contents;
			get_block_contents(block_contents, program[i], blocks_open, true, annotate); //Get any block-contents after block-opening character
			size_t opening_line = program[i].lnum;
			size_t opening_index = i;

			//Keep reading lines until entire block-contents have been read
			while (blocks_open > 0){

				i++; //Proceed to next line

				//Stay within no. lines in program
				if (i >= program.size()){
					cout << "ERROR: Block starting on line " << to_string(opening_line) << " didn't close before program end." << endl;
					return false;
				}

				get_block_contents(block_contents, program[i], blocks_open, false, annotate);
			}

			//*************** COMPLETE EXPANSION OF WHILE ******************//

			//Mark the beginning of the while loop
			line temp_line;
			temp_line.lnum = opening_line;
			temp_line.str = "#HERE @START_LOOP_NUM"+to_string(opening_line);
			block_contents.insert(block_contents.begin(), temp_line);

			//Add 'jump to top' at bottom of while
			temp_line.str = "IFZERO {";
			block_contents.push_back(temp_line);
			temp_line.str = "\tJUMP @START_LOOP_NUM"+to_string(opening_line);
			block_contents.push_back(temp_line);
			temp_line.str = "}";
			block_contents.push_back(temp_line);

			//Erase while loop
			if (i >= program.size()){
				program.erase(program.begin()+opening_index, program.end()+1);
			}else{
				program.erase(program.begin()+opening_index, program.begin()+i+1);
			}

			//Insert expanded while loop
			program.insert(program.begin()+opening_index, block_contents.begin(), block_contents.end());

			//Change program index
			i = opening_index + block_contents.size();

		}else if (program[i].str.substr(0, 10) == "WHILECARRY"){ //If beginning of line is WHILECARRY keyword

			//******************* ENSURE CORRECT SYNTAX ******************//

			//Parse string
			vector<string> words = parse(program[i].str, " \t");

			//Enure two words or more, and that second word is block-opening bracket
			if (words.size() < 2 || words[1] != "{"){
				cout << "ERROR: No block-opening bracket after WHILECARRY keyword. Line: " << to_string(program[i].lnum) << endl;
				return false;
			}

			//***************** GET BLOCK CONTENTS ************************//

			//Read first line...
			int blocks_open = 1;
			vector<line> block_contents;
			get_block_contents(block_contents, program[i], blocks_open, true, annotate); //Get any block-contents after block-opening character
			size_t opening_line = program[i].lnum;
			size_t opening_index = i;

			//Keep reading lines until entire block-contents have been read
			while (blocks_open > 0){

				i++; //Proceed to next line

				//Stay within no. lines in program
				if (i >= program.size()){
					cout << "ERROR: Block starting on line " << to_string(opening_line) << " didn't close before program end." << endl;
					return false;
				}

				get_block_contents(block_contents, program[i], blocks_open, false, annotate);
			}

			//*************** COMPLETE EXPANSION OF WHILE ******************//

			//Mark the beginning of the while loop
			line temp_line;
			temp_line.lnum = opening_line;
			temp_line.str = "#HERE @START_LOOP_NUM"+to_string(opening_line);
			block_contents.insert(block_contents.begin(), temp_line);

			//Add 'jump to top' at bottom of while
			temp_line.str = "IFCARRY {";
			block_contents.push_back(temp_line);
			temp_line.str = "\tJUMP @START_LOOP_NUM"+to_string(opening_line);
			block_contents.push_back(temp_line);
			temp_line.str = "}";
			block_contents.push_back(temp_line);

			//Erase while loop
			if (i >= program.size()){
				program.erase(program.begin()+opening_index, program.end()+1);
			}else{
				program.erase(program.begin()+opening_index, program.begin()+i+1);
			}

			//Insert expanded while loop
			program.insert(program.begin()+opening_index, block_contents.begin(), block_contents.end());

			//Change program index
			i = opening_index + block_contents.size();

		}
	}

}

bool expand_if_statements(vector<line>& program, bool verbose, bool annotate){

	size_t num_del = 0;
	size_t num_inline = 0;
	bool was_carry = false;
	for (size_t i = 0 ; i < program.size() ; i++){ //For each line...

		if (program[i].str.substr(0, 6) == "IFZERO" || program[i].str.substr(0, 7) == "IFCARRY"){ //If beginning of line is WHILEZERO keyword

			//Record if was zero or carry
			if (program[i].str.substr(0, 6) == "IFZERO"){
				was_carry = false;
			}else{
				was_carry = true;
			}

			//******************* ENSURE CORRECT SYNTAX ******************//

			//Parse string
			vector<string> words = parse(program[i].str, " \t");

			//Enure two words or more, and that second word is block-opening bracket
			if (words.size() < 2 || words[1] != "{"){
				cout << "ERROR: No block-opening bracket after IF keyword. Line: " << to_string(program[i].lnum) << endl;
				return false;
			}

			//***************** GET BLOCK CONTENTS ************************//

			//Read first line...
			int blocks_open = 1;
			vector<line> block_contents;
			get_block_contents(block_contents, program[i], blocks_open, true, annotate); //Get any block-contents after block-opening character
			size_t opening_line = program[i].lnum;
			size_t opening_index = i;

			//Keep reading lines until entire block-contents have been read
			while (blocks_open > 0){

				i++; //Proceed to next line

				//Stay within no. lines in program
				if (i >= program.size()){
					cout << "ERROR: Block starting on line " << to_string(opening_line) << " didn't close before program end." << endl;
					return false;
				}

				get_block_contents(block_contents, program[i], blocks_open, false, annotate);
			}

			//************ CHECK IF ELSE STATEMENT PROVIDED ****************//

			blocks_open = 2; //Make blocks open 2 so the initial close bracket is skipped
			vector<line> else_block_contents;
			size_t else_opening_line = program[i].lnum;
			size_t else_opening_index = i;
			bool has_else = false;

			//If else is found...
			if (program[i].str.find("ELSE") != string::npos){

				has_else = true;

				//Read else block

				//Read first line...
				get_block_contents(else_block_contents, program[i], blocks_open, true, annotate); //Get any block-contents after block-opening character

				//Keep reading lines until entire block-contents have been read
				while (blocks_open > 0){

					i++; //Proceed to next line

					//Stay within no. lines in program
					if (i >= program.size()){
						cout << "ERROR: Block starting on line " << to_string(opening_line) << " didn't close before program end." << endl;
						return false;
					}

					get_block_contents(else_block_contents, program[i], blocks_open, false, annotate);
				}
			}


			//*************** COMPLETE EXPANSION OF WHILE ******************//

			//Add the jump location for the TRUE clause
			line temp_line;
			temp_line.lnum = opening_line;
			temp_line.str = "#HERE @TRUE_IF_NUM"+to_string(opening_line);
			block_contents.insert(block_contents.begin(), temp_line);

			//Add the jump location for the end of the IF statement
			temp_line.lnum = opening_line;
			temp_line.str = "#HERE @END_IF_NUM"+to_string(opening_line);
			block_contents.insert(block_contents.end(), temp_line);

			//Add the jump command that prevents executing TRUE if FALSE
			temp_line.lnum = opening_line;
			temp_line.str = "JUMP @END_IF_NUM"+to_string(opening_line);
			block_contents.insert(block_contents.begin(), temp_line);

			//Add else clause if provided
			if (has_else){
				for (long int ei = else_block_contents.size()-1 ; ei >= 0 ; ei--){
					line temp_line_test;
					temp_line_test.lnum = 0;
					temp_line_test.str = "*";
					block_contents.insert(block_contents.begin(), else_block_contents[ei]);

					if (i > 1e3) exit(1);
				}

			}

			//Add the initial JUMPIF_ statement
			temp_line.lnum = opening_line;
			if (was_carry){
				temp_line.str = "JUMPIFCARRY @TRUE_IF_NUM"+to_string(opening_line);
			}else{
				temp_line.str = "JUMPIFZERO @TRUE_IF_NUM"+to_string(opening_line);
			}
			block_contents.insert(block_contents.begin(), temp_line);

			//Erase IF statement
			if (i >= program.size()){
				program.erase(program.begin()+opening_index, program.end()+1);
			}else{
				program.erase(program.begin()+opening_index, program.begin()+i+1);
			}

			//Insert expanded if statement
			program.insert(program.begin()+opening_index, block_contents.begin(), block_contents.end());

			//Change program index
			i = opening_index + block_contents.size();

		}
	}

}


/*
Prints the program
*/
void print_program(vector<line>& program){
	string start_str;
	for (size_t i = 0 ; i < program.size() ; i++){
		start_str = "[" + to_string(program[i].lnum) + "]: ";
		while (start_str.length() < 9) start_str = " " + start_str;
		cout << start_str << program[i].str << endl;
	}
}

/*
block_contents - vector of lines that constitutes the inside of a block
input - current line to process (ie. add to block contents, and/or change # blocks_open)
blocks_open - No. curly braces open - ie. No. closing curly braces needed before block ends
ignore_first_open - Defines if opening brackets should be ignored for this line. Set to true when running on the line when the block opens.
annotate - specify if annotations should be injected into the output

Block content rules:
	* If no opening or closing brackets, line is added verbatim
	* If more than one bracket of the same type (ie. two opening, two closing) exist on one line, this code will NOT detect it.
*/
bool get_block_contents(vector<line>& block_contents, line input, int& blocks_open, bool ignore_first_open, bool annotate){

	size_t found_open = input.str.find("{"); //See if opening bracket exists
	size_t found_closed = input.str.find("}"); //See if closing bracket exists
	if (found_open == string::npos && found_closed == string::npos){ //Go to next line if no comment marker...
		block_contents.push_back(input);
	}

	//Does closing bracket exist?
	if (found_closed != string::npos){
		blocks_open--;

		if (blocks_open == 0){

			//Annotate if requested
			if (annotate){
				line note_line;
				note_line.str = "//HKOMP: END BLOCK CONTENTS";
				note_line.lnum = input.lnum;
				block_contents.push_back(note_line);
			}

			//Add substring
			line temp_line;
			temp_line.str = input.str.substr(0, found_closed);
			remove_end_whitespace(temp_line.str);
			temp_line.lnum = input.lnum;
			if (temp_line.str.length() > 0) block_contents.push_back(temp_line);

			return false;
		}
	}

	//Does opening bracket exist?
	if (found_open != string::npos){
		if (!ignore_first_open){ //Proceed as normal, increment the blocks_open count
			blocks_open++;
		}else{ //Only add the part of the line after the opening brackets

			//Annotate if requested
			if (annotate){
				line note_line;
				note_line.str = "//HKOMP: BEGIN BLOCK CONTENTS";
				note_line.lnum = input.lnum;
				block_contents.push_back(note_line);
			}

			//Add substring
			line temp_line;
			temp_line.str = input.str.substr(found_open+1);
			remove_end_whitespace(temp_line.str);
			temp_line.lnum = input.lnum;

			if (temp_line.str.length() > 0) block_contents.push_back(temp_line);
		}
	}

	return true;
}


































//
