/*******************************************************************************
This is the Quark compiler.

Syntax:

G Giesbrecht
16-1-2021

*******************************************************************************/

#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <gstd.hpp>
#include <gcolors.hpp>
#include <fstream>
#include <cctype>
#include <ktable.hpp>

//CXCOMPILE g++ quark_comp.cpp -o quark_comp -lgstd -std=c++11 -lktable
//Skip CXCOMPILE ./quark_comp Source\ Files/dev.qrk -v -info
//CXCOMPILE ./quark_comp BrPgm3.qrk -v -info
//CXGENRUN FALSE
//CXPRINTCOM TRUE

#define COMPILER_NAME string("QUARK")

using namespace std;
using namespace gstd;

/*
Describes a contiguous block of code. When the compiler converts abstract memory
locations to physical locations, these contiguous blocks are used to determine
the restrictions on what code can be located where.
*/
typedef struct{
	size_t idx_prog_start;
	size_t idx_prog_end;
	string starting_AML;
}contiguous;

/*
Describes an abstract memory location.
*/
typedef struct{
	string name;
	size_t phys_addr;
	bool addr_assigned;
	size_t source_line;
}aml;

/*
Describes a line of code, both it contents and original line number.
*/
typedef struct{
	string str;
	size_t lnum;
}line;

/*
Describes a macro blocksub.
*/
typedef struct{
	string name;
	vector<string> arguments;
	vector<line> contents;
	size_t declare_line;
}blocksub;

/*
Describes a message from the compiler. This includes the text of the message
and a message level.

Levels:
	1 - Error
	2 - Warning
	3 - Info
	4 - Spam
*/
typedef struct{
	int level;
	string str;
}message;

class CompilerParams{
public:

	CompilerParams(){
		default_init();
	}

	CompilerParams(string pmem_val, int true_val){

		default_init();

		pmem = pmem_val;
		true_value = true_val;
	}

	void default_init(){
		pmem = "FLASH0";
		true_value = 1;
		false_value = 0;
		add_missing_halt = true;

		isv_series = "Unspecified";
		isv_major = -1;
		isv_minor = -1;
		isv_patch = -1;

		keep_comments = false;
		annotate = false;
		verbose = false;

		print_level = 2;
		quit_on_error = false;
	}

	//*********************** COMPILER PARAMETERS ****************************//

	/*
	Program memory location. This is where the program will be run from in the
	computer. THis is relevant because it specifies how commands such as copy
	will be implemented (ex. copy RAM->RAM or FLASH->FLASH).
	*/
	string pmem; //TODO: Is this used?

	/*
	Value for 'true'. In using complex case structures, this numeric value will
	be checked for as the 'true' condition. The keyword 'true' will also be
	expanded to this numeric value.
	*/
	int true_value;

	/*
	Value for 'false'. The keyword 'false' will also be expanded to this numeric
	value.

	NOTE: This plays no real role in complex case structures, as
	false is computationally defined as 'not true' (ie. it doesnt check that a
	value matches this false condition, but rather if it does or doesnt match
	the true condition ).
	*/
	int false_value;

	/*
	Tells compiler if it should add a halt statement to the end of the program
	if it is missing. A warning will still be generated.
	*/
	bool add_missing_halt;

	/*
	ISV specifier. Quark checks that a valid ISV specifier is present, but does
	not currently verify that the code is compliant with that version (this may
	be added later).
	*/
	string isv_series;
	int isv_major;
	int isv_minor;
	int isv_patch;

	/*
	If true, leaves comments throughout each step of compilation and in the
	output files.
	*/
	bool keep_comments;

	/*
	If true, annotates the output with comments as the compiler performs various
	operations such as code expansions.
	*/
	bool annotate;

	/*
	Determines if the compiler should have verbose output
	*/
	bool verbose;

	/*
	Specifies a level of log levels to print
	*/
	int print_level;

	/*
	If true, Quark aborts if any error occurs.

	TODO: This is not yet implemented
	*/
	bool quit_on_error;

	//*********************** MESSAGING FUNCTIONS ****************************//

	void error(string err_str){

		string start_str = CompilerParams::levelToID(1);
		while (start_str.length() < 9) start_str = " " + start_str;
		start_str = CompilerParams::levelToFormat(1) + start_str + gcolor::normal;

		cout << start_str << err_str << endl;
		message new_mess;
		new_mess.str = err_str;
		new_mess.level = 1;
		messages.push_back(new_mess);
	}

	void error(string err_str, size_t lnum){

		string start_str = CompilerParams::levelToID(1);
		while (start_str.length() < 9) start_str = " " + start_str;
		start_str = CompilerParams::levelToFormat(1) + start_str + gcolor::normal;

		err_str = err_str + " Line: " + to_string(lnum);
		cout << start_str << err_str << endl;
		message new_mess;
		new_mess.str = err_str;
		new_mess.level = 1;
		messages.push_back(new_mess);
	}

	void warning(string warn_str){

		string start_str = CompilerParams::levelToID(2);
		while (start_str.length() < 9) start_str = " " + start_str;
		start_str = CompilerParams::levelToFormat(2) + start_str + gcolor::normal;

		cout << start_str << warn_str << gcolor::normal << endl;
		message new_mess;
		new_mess.str = warn_str;
		new_mess.level = 2;
		messages.push_back(new_mess);
	}

	void warning(string warn_str, size_t lnum){

		string start_str = CompilerParams::levelToID(2);
		while (start_str.length() < 9) start_str = " " + start_str;
		start_str = CompilerParams::levelToFormat(2) + start_str + gcolor::normal;

		warn_str = warn_str + " Line: " + to_string(lnum);
		cout << start_str << warn_str << gcolor::normal << endl;
		message new_mess;
		new_mess.str = warn_str;
		new_mess.level = 2;
		messages.push_back(new_mess);
	}

	void info(string info_str){

		string start_str = CompilerParams::levelToID(3);
		while (start_str.length() < 9) start_str = " " + start_str;
		start_str = CompilerParams::levelToFormat(3) + start_str + gcolor::normal;

		cout << start_str << info_str << gcolor::normal << endl;
		message new_mess;
		new_mess.str = info_str;
		new_mess.level = 3;
		messages.push_back(new_mess);
	}

	void info(string info_str, size_t lnum){

		string start_str = CompilerParams::levelToID(3);
		while (start_str.length() < 9) start_str = " " + start_str;
		start_str = CompilerParams::levelToFormat(3) + start_str + gcolor::normal;

		info_str = info_str + " Line: " + to_string(lnum);
		cout << start_str << info_str << gcolor::normal << endl;
		message new_mess;
		new_mess.str = info_str;
		new_mess.level = 3;
		messages.push_back(new_mess);
	}

	void spam(string spam_str){
		message new_mess;
		new_mess.str = spam_str;
		new_mess.level = 4;
		messages.push_back(new_mess);
	}

	void spam(string spam_str, size_t lnum){

		spam_str = spam_str + " Line: " + to_string(lnum);

		message new_mess;
		new_mess.str = spam_str;
		new_mess.level = 4;
		messages.push_back(new_mess);
	}

	/*
	Prints messages at the specified level

	level - print level
	isExactly - if true, only prints messages at the level and not those also
		higher.
	*/
	void printMessages(int level = 3, bool isExactly=false){
		cout << "Compiler Messages:\t\tlvl=" + to_string(level) << endl;
		for (size_t i =  0 ; i < messages.size() ; i++){
			if (messages[i].level == level || (!isExactly && messages[i].level <= level)){
				string start_str = CompilerParams::levelToID( messages[i].level);
				while (start_str.length() < 9) start_str = " " + start_str;
				start_str = CompilerParams::levelToFormat(messages[i].level) + start_str + gcolor::normal;
				cout << start_str << messages[i].str << endl;
			}
		}
	}

private:
	std::vector<message> messages;

	std::string levelToID(int level){
		switch(level){
			case (1):
				return "ERROR:";
				break;
			case(2):
				return "WARNING:";
				break;
			case(3):
				return "INFO:";
				break;
			case(4):
				return "SPAM:";
				break;
			default:
				return "[?]:";
				break;
		}
	}

	std::string levelToFormat(int level){
		switch(level){
			case (1):
				return gcolor::normal + gcolor::red +gcolor::bb;
				break;
			case(2):
				return gcolor::normal + gcolor::red;
				break;
			case(3):
				return gcolor::normal + gcolor::blue;
				break;
			case(4):
				return gcolor::normal;
				break;
			default:
				return gcolor::normal;
				break;
		}
	}
};

//PURGES
void purge_comments(vector<line>& program, bool verbose);

//EXPANSIONS
bool check_isv(vector<line>& program, CompilerParams& params);
bool read_options(vector<line>& program, CompilerParams& params);
bool expand_while_statements(vector<line>& program, CompilerParams& params);
bool expand_if_statements(vector<line>& program, CompilerParams& params);
bool expand_boolean_values(vector<line>& program, CompilerParams& params);
bool load_blocksub_definitions(vector<line>& program, vector<blocksub>& subs, CompilerParams& params);
bool expand_blocksub_statements(vector<line>& program, vector<blocksub>& subs, CompilerParams& params);



//FINAL EXPANSIONS
void get_all_amls(vector<line>& program, vector<aml>& amls);
void get_contiguous_blocks(vector<line>& program, CompilerParams& params, vector<aml> amls, vector<contiguous>& contigs);

//OTHER
void print_program(vector<line> program, string options="");
void rich_print_program(vector<line> program, vector<contiguous> contigs, CompilerParams& params, string options="");
void print_blocksubs(vector<blocksub> subs);
void print_amls(vector<aml> amls);
void print_contiguous(vector<contiguous> contiguous);
bool get_block_contents(vector<line>& block_contents, line input, int& blocks_open, bool ignore_first_open=false, bool annotate=false);
bool is_valid_name(string s);
bool begins_with_subname(string line, vector<blocksub> subs);


int main(int argc, char** argv){

	//**********************************************************************//
	//******************** GET USER OPTIONS ********************************//

	bool save_ahsm = false; //Tell compiler to save .AHSM file
	bool save_hsm = false;



	//Initialize default compiler parameters
	CompilerParams params;

	//Get input file's name
	if (argc < 2){

		params.error("Requires .qrk file's name (no spaces allowed) as input.");
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
				params.keep_comments = true;
			}else if(string(argv[arg]) == "-n"){
				params.annotate = true;
			}else if(string(argv[arg]) == "-v"){
				params.verbose = true;
			}else if(string(argv[arg]) == "-error"){
				params.print_level = 1;
			}else if(string(argv[arg]) == "-warning"){
				params.print_level = 2;
			}else if(string(argv[arg]) == "-info"){
				params.print_level = 3;
			}else if(string(argv[arg]) == "-spam"){
				params.print_level = 4;
			}else{
				params.warning("Ignoring unrecognized flag '" + string(argv[arg]) + "'.");
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
	vector<blocksub> subs;
	vector<aml> amls;
	vector<contiguous> contigs;
	ifstream file(filename.c_str());
	if (!file.is_open()){
		params.error("Failed to open file '" + filename + "'.");
		return -1;
	}
	//
	string line_in;
	size_t line_num = 0;
	while (getline(file, line_in)) { //For each line...
		line_num++; //Increment line number
		if (line_in.length() == 0) continue; //Skip blank lines
		ensure_whitespace(line_in, "{"); //Ensure whitespace around certain characters
		ensure_whitespace(line_in, "(,)"); //Ensure whitespace around certain characters
		trim_whitespace(line_in); //remove leading, trailing whitespace

		//Create 'line' struct
		line templine;
		templine.str = line_in;
		templine.lnum = line_num;

		program.push_back(templine); //Add line to program
	}

	//***********************************************************************//
	//*************************** EXPAND, COMPILE, ETC **********************//

	bool step_failed = false;

	cout << "ORIGINAL: " << endl;
	if (params.verbose) print_program(program);

	if (!params.keep_comments) purge_comments(program, params.verbose);
	params.spam("Purged Comments");
	if (params.verbose && !params.keep_comments){
		cout << "\nCOMMENTS PURGED:" << endl;
		if (params.verbose) print_program(program);
	}

	if (!read_options(program, params)){
		step_failed = true;
	}
	params.spam("Read Option Directives");
	if (params.verbose){
		cout << "Read Option Directives" << endl;
		print_program(program);
	}


	if (!check_isv(program, params)){
		step_failed = true;
	}
	if (params.verbose){
		cout << "CHECKED ISV" << endl;
	}

	if (!expand_while_statements(program, params)){
		step_failed = true;
	}
	params.spam("Expanded While Statements");
	if (params.verbose){
		cout << "EXPANDED WHILE STATEMENTS" << endl;
		print_program(program);
	}


	if (!expand_if_statements(program, params)){
		step_failed = true;
	}
	params.spam("Expanded If Statements");
	if (params.verbose){
		cout << "EXPANDED IF STATEMENTS" << endl;
		print_program(program);
	}

	if (!expand_boolean_values(program, params)){
		step_failed = true;
	}
	params.spam("Expanded boolean values");
	if (params.verbose){
		cout << "EXPANDED BOOLEAN VALUES" << endl;
		print_program(program);
	}


	if (!load_blocksub_definitions(program, subs, params)){
		step_failed = true;
	}
	params.spam("Loaded blocksub Defininitions");
	if (params.verbose){
		cout << "LOADED BLOCK SUBSTITUTIONS" << endl;
	}
	if (params.verbose) print_blocksubs(subs);
	if (params.verbose) print_program(program);

	//The steps above this are the first few that will need to run

	/*

	There are probably lots of steps that need to go here


	*/

	//The steps below this are the last few that need to run

	if (!expand_blocksub_statements(program, subs, params)){
		step_failed = true;
	}
	params.spam("Expanded blocksub Calls");
	if (params.verbose){
		cout << "EXPANDED BLOCKSUB CALLS" << endl;
		print_program(program);
	}

	get_all_amls(program, amls);
	if (params.verbose) print_amls(amls);


	get_contiguous_blocks(program, params, amls, contigs);
	if (params.verbose) print_contiguous(contigs);

	rich_print_program(program, contigs, params);

	// assign_physical_locations(program, verbose, params, amls);


	cout << endl << endl;
	params.printMessages(params.print_level);

	// if (params.verbose) print_program(program);

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
Reads the ISV directive at the top of the program and verifies that it follows the
correct syntax. Expects '#ISV series_name_no_whitespace_or_period <major>.<minor>.<patch>'
*/
bool check_isv(vector<line>& program, CompilerParams& params){

	string isv_keyword = "#ISV";

	for (size_t i = 0 ; i < program.size() ; i++){ //For each line...

		if (program[i].str.substr(0, isv_keyword.length()) == isv_keyword){

			string temp_line = program[i].str;
			ensure_whitespace(temp_line, ".");
			cout << temp_line << endl;
			vector<string> words = parse(temp_line, " \t");

			//Check enough arguments present
			if (words.size() < 7){
				params.error("ISV specifier in incorrect format. Must contain series name and 3-digit version number. Line: " + to_string(program[i].lnum));
				return false;
			}

			params.isv_series = words[1];

			try{
				params.isv_major = stoi(words[2]);
				params.isv_minor = stoi(words[4]);
				params.isv_patch = stoi(words[6]);
			}catch(const std::invalid_argument& ia){
				params.isv_major = -1;
				params.isv_minor = -1;
				params.isv_patch = -1;
				params.error("Failed to read 3-digit version number in ISV specifier. Line: " + to_string(program[i].lnum));
				return false;
			}

			params.info("Design ISV = " + params.isv_series + " " + to_string(params.isv_major) + "." + to_string(params.isv_minor) + "." + to_string(params.isv_patch));


		}

	}

}

/*
Reads compiler option directives from program.
*/
bool read_options(vector<line>& program, CompilerParams& params){

	string directive_keyword = "#OPTION";

	for (size_t i = 0 ; i < program.size() ; i++){ //For each line...

		if (program[i].str.substr(0, directive_keyword.length()) == directive_keyword){

			//Parse
			vector<string> words = parse(program[i].str, " \t");

			//Check enough arguments present
			if (words.size() < 4){
				params.error("Compiler option directives require a minimum of four words. Line: " + to_string(program[i].lnum));
				return false;
			}

			if (words[2] != "="){
				params.error("Compiler option directives require an equal sign between the parameter and its value. Line: " + to_string(program[i].lnum));
				return false;
			}

			//******************* FIND DIRECTIVE PARAMETER *******************//

			cout << "\t\t" << words[1] << endl;

			if (words[1] == "PROGRAM_MEMORY"){
				params.pmem = words[3];
				params.info("Program Memory location set to '" + words[3] + "'");
			}else if(words[1] == "TRUE_VALUE"){
				try{
					int val = stoi(words[3]);
					if (val == params.false_value){
						params.warning("TRUE_VALUE (" + to_string(val) +") set to same value as FALSE_VALUE ("+ to_string(params.false_value)+").");
					}
					params.true_value = val;
					params.info("Compiler TRUE_VALUE set to " + to_string(params.true_value));
				}catch(...){
					params.warning("Failed to convert '" + words[3] +  "' to an integer value for compiler parameter TRUE_VALUE.", program[i].lnum);
				}
			}else if(words[1] == "FALSE_VALUE"){
				try{
					int val = stoi(words[3]);
					if (val == params.true_value){
						params.warning("FALSE_VALUE (" + to_string(val) +") set to same value as TRUE_VALUE ("+ to_string(params.true_value)+").");
					}
					params.false_value = val;
					params.info("Compiler FALSE_VALUE set to " + to_string(params.false_value));
				}catch(...){
					params.warning("Failed to convert '" + words[3] +  "' to an integer value for compiler parameter FALSE_VALUE.", program[i].lnum);
				}
			}else if(words[1] == "ADD_MISSING_HALT"){
				try{
					params.add_missing_halt = gstd::to_bool(words[3]);
					params.info("Compiler ADD_MISSING_HALT set to " + bool_to_str(params.add_missing_halt, 'l'));
				}catch(...){
					params.warning("Failed to convert '" + words[3] +  "' to a boolean value for compiler parameter ADD_MISSING_HALT.", program[i].lnum);
				}
			}else{
				params.warning("Compiler directive '" + words[1] + "' unrecognized.");
			}

			//******************* Erase Line *******************//

			program.erase(program.begin()+i); //Erase line
			i--;

		}

	}

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
bool expand_while_statements(vector<line>& program, CompilerParams& params){

	size_t num_del = 0;
	size_t num_inline = 0;
	for (size_t i = 0 ; i < program.size() ; i++){ //For each line...

		if (program[i].str.substr(0, 9) == "WHILEZERO"){ //If beginning of line is WHILEZERO keyword

			//******************* ENSURE CORRECT SYNTAX ******************//

			//Parse string
			vector<string> words = parse(program[i].str, " \t");

			//Enure two words or more, and that second word is block-opening bracket
			if (words.size() < 2 || words[1] != "{"){
				params.error("No block-opening bracket after WHILEZERO keyword. Line: " + to_string(program[i].lnum));
				return false;
			}

			//***************** GET BLOCK CONTENTS ************************//

			//Read first line...
			int blocks_open = 1;
			vector<line> block_contents;
			get_block_contents(block_contents, program[i], blocks_open, true, params.annotate); //Get any block-contents after block-opening character
			size_t opening_line = program[i].lnum;
			size_t opening_index = i;

			//Keep reading lines until entire block-contents have been read
			while (blocks_open > 0){

				i++; //Proceed to next line

				//Stay within no. lines in program
				if (i >= program.size()){
					params.error("Block starting on line " + to_string(opening_line) + " didn't close before program end.");
					return false;
				}

				get_block_contents(block_contents, program[i], blocks_open, false, params.annotate);
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
				params.error("No block-opening bracket after WHILECARRY keyword. Line: " + to_string(program[i].lnum));
				return false;
			}

			//***************** GET BLOCK CONTENTS ************************//

			//Read first line...
			int blocks_open = 1;
			vector<line> block_contents;
			get_block_contents(block_contents, program[i], blocks_open, true, params.annotate); //Get any block-contents after block-opening character
			size_t opening_line = program[i].lnum;
			size_t opening_index = i;

			//Keep reading lines until entire block-contents have been read
			while (blocks_open > 0){

				i++; //Proceed to next line

				//Stay within no. lines in program
				if (i >= program.size()){
					params.error("Block starting on line " + to_string(opening_line) + " didn't close before program end.");
					return false;
				}

				get_block_contents(block_contents, program[i], blocks_open, false, params.annotate);
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

/*
Finds if statements and expands them into AHSM code. It will collapse the
IF statements into JUMP-HERE statements.
Syntax rules for if statements:
	* the if-keyword must be the first token in a line.
	* if-keywords:
	 	* IFZERO
		* IFCARRY
	* block opening bracket ({) must immediately follow if-keyword and on same line
*/
bool expand_if_statements(vector<line>& program, CompilerParams& params){

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
				params.error("No block-opening bracket after IF keyword. Line: " + to_string(program[i].lnum));
				return false;
			}

			//***************** GET BLOCK CONTENTS ************************//

			//Read first line...
			int blocks_open = 1;
			vector<line> block_contents;
			get_block_contents(block_contents, program[i], blocks_open, true, params.annotate); //Get any block-contents after block-opening character
			size_t opening_line = program[i].lnum;
			size_t opening_index = i;

			//Keep reading lines until entire block-contents have been read
			while (blocks_open > 0){

				i++; //Proceed to next line

				//Stay within no. lines in program
				if (i >= program.size()){
					params.error("Block starting on line " + to_string(opening_line) + " didn't close before program end.");
					return false;
				}

				get_block_contents(block_contents, program[i], blocks_open, false, params.annotate);
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
				get_block_contents(else_block_contents, program[i], blocks_open, true, params.annotate); //Get any block-contents after block-opening character

				//Keep reading lines until entire block-contents have been read
				while (blocks_open > 0){

					i++; //Proceed to next line

					//Stay within no. lines in program
					if (i >= program.size()){
						params.error("Block starting on line " + to_string(opening_line) + " didn't close before program end.");
						return false;
					}

					get_block_contents(else_block_contents, program[i], blocks_open, false, params.annotate);
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

bool expand_boolean_values(vector<line>& program, CompilerParams& params){

	size_t num_true_exp = 0;
	size_t num_false_exp = 0;

	for (size_t i = 0 ; i < program.size() ; i++){ //For each line...

		//Parse string

		string line_str = program[i].str;
		ensure_whitespace(line_str, "(),=;");
		vector<string> words = parse(line_str, " \t");

		if (gstd::inVector(string("true"), words)){
			gstd::findAndReplace(program[i].str, "true", to_string(params.true_value));
		}

		if (gstd::inVector(string("false"), words)){
			gstd::findAndReplace(program[i].str, "false", to_string(params.false_value));
		}

	}

}

/*
Reads through the program, removes all blocksub definitions, turns them into
blocksub structs, and places them in the variable 'subs'.

Returns true if no errors occur.
*/
bool load_blocksub_definitions(vector<line>& program, vector<blocksub>& subs, CompilerParams& params){

	size_t num_def = 0;
	for (size_t i = 0 ; i < program.size() ; i++){ //For each line...

		if (program[i].str.substr(0, 11) == "#BLOCKSUB"){ //If beginning of line is #BLOCKSUB keyword

			blocksub temp_sr;
			temp_sr.declare_line = program[i].lnum;

			//******************* ENSURE CORRECT SYNTAX ******************//

			//Parse string
			vector<string> words = parse(program[i].str, " \t");

			//Enure two words or more
			if (words.size() < 5){

				params.error("blocksub definition requires name, parentheses, and opening curly bracket. Line: " + to_string(program[i].lnum));
				return false;
			}

			//************************** GET NAME *************************//

			//Ensure valid blocksub name
			if (!is_valid_name(words[1])){
				params.error("blocksub name '" + words[1] + "' is not a permissible name. Line: " + to_string(program[i].lnum));
				return false;
			}

			temp_sr.name = words[1];

			//************************ GET ARGUMENTS **********************//

			//Ensure opening parentheses in right spot
			if (words[2] != "("){
				params.error("Opening parentheses must be present after blocksub name. Line: " + to_string(program[i].lnum));
				return false;
			}

			//Ensure closing parentheses in right spot
			if (words[words.size()-2] != ")"){
				params.error("Closed parentheses must be second to last token in blocksub definition line. Line: " + to_string(program[i].lnum));
				return false;
			}

			//Ensure opening bracket
			if (words[words.size()-1] != "{"){
				params.error("Opening curly brackets must be last token in blocksub definition line. Line: " + to_string(program[i].lnum));
				return false;
			}

			//read in arguments list
			string last_word = ",";
			for (size_t w = 3 ; w < words.size()-2 ; w++){

				//Ensure commas separate words
				if (last_word != ","){
					if (words[w] == ","){
						last_word = ",";
						continue;
					}else{
						params.error("comma required after argument '" + last_word + "'.", program[i].lnum);
						return false;
					}
				}

				//Add word to arugments list
				temp_sr.arguments.push_back(words[w]);
				last_word = words[w];
			}


			//***************** GET BLOCK CONTENTS ************************//

			//Read first line...
			int blocks_open = 1;
			get_block_contents(temp_sr.contents, program[i], blocks_open, true, params.annotate); //Get any block-contents after block-opening character
			size_t opening_line = program[i].lnum;
			size_t opening_index = i;

			//Keep reading lines until entire block-contents have been read
			while (blocks_open > 0){

				i++; //Proceed to next line

				//Stay within no. lines in program
				if (i >= program.size()){
					params.error("Block starting on line " + to_string(opening_line) + " didn't close before program end.");
					return false;
				}

				get_block_contents(temp_sr.contents, program[i], blocks_open, false, params.annotate);
			}


			//*************** COMPLETE EXPANSION OF WHILE ******************//

			//Erase definition
			if (i >= program.size()){
				program.erase(program.begin()+opening_index, program.end()+1);
			}else{
				program.erase(program.begin()+opening_index, program.begin()+i+1);
			}

			//Insert comment if requested
			if (params.annotate){
				line temp_line;
				temp_line.lnum = opening_line;
				temp_line.str = "//QUARK: blocksub definition. Name: '" + temp_sr.name + "' Arguments: " + to_string(temp_sr.arguments.size()) + " Block size: " + to_string(temp_sr.contents.size());
				program.insert(program.begin()+opening_index, temp_line);
			}

			//Change program index
			i = opening_index;
			if (!params.annotate) i--;

			subs.push_back(temp_sr);

		}
	}

	return true;
}

/*
Finds blocksub calls and expands them into AHSM code.
Syntax rules for while statements:
	* The blocksub name, with a carat attacted to the front, must be the first
	  word. (ex. ^blocksub_name)
	* The arguments must follow in parenthesis. If no arguments are taken, empty
	  parentheses must be given. The arguments are separated by commas.
	* Whatever is provided as each argument is drop-in replaced for the argument
	  in the blocksub's text.

Returns true if no errors occur.
*/
bool expand_blocksub_statements(vector<line>& program, vector<blocksub>& subs, CompilerParams& params){
	cout << "\n\n" << endl;
	size_t num_del = 0;
	size_t num_inline = 0;
	for (size_t i = 0 ; i < program.size() ; i++){ //For each line...

		if (program[i].str.length() > 0 && program[i].str[0] == '^' && begins_with_subname(program[i].str, subs)){

			//Parse string
			string callline = program[i].str;
			ensure_whitespace(callline, "(,)");
			vector<string> words = parse(callline, " \t");

			//******************* FIND blocksub IDX *******************//

			//Check if first word is in subs vector
			string sub_name = words[0].substr(1);
			size_t sub_idx;
			for (size_t si = 0 ; si < subs.size() ; si++){

				//If a match is found, return true
				if (subs[si].name == sub_name ) sub_idx = si;
			}

			//******************* ENSURE CORRECT SYNTAX ******************//



			//Enure 3 words or more, and that second word is open paren, last is close paren
			if (words.size() < 3 || words[1] != "(" || words[words.size()-1] != ")"){
				params.error("Incorrect syntax in blocksub cal. Parentheses are required after blocksub name. Line: " + to_string(program[i].lnum));
				return false;
			}

			//***************** GET ARGUMENTS *****************************//

			//Scan through each word that could be an arugment (based on index)
			vector<string> args;
			bool need_comma = false;
			for (size_t a = 2 ; a < words.size()-1 ; a++){

				//Check if word is commma
				if (words[a] == ","){
					if (need_comma){ //Update comma flag if all is well
						need_comma = false;
						continue;
					}else{ //If comma not okay, send error
						params.error("Incorrect syntax in blocksub call. Extraneous comma found within argument list", program[i].lnum);
						return false;
					}
				}else if(need_comma){ //Check if comma was needed but missing
					params.error("Incorrect syntax in blocksub call. Missing comma within argument list.", program[i].lnum);
					return false;
				}

				//Else is an argument
				args.push_back(words[a]);
				need_comma = true;
			}

			//Check that correct number of arguments present
			if (args.size() != subs[sub_idx].arguments.size()){
				params.error("Incorrect number of arguments in call to blocksub '" + subs[sub_idx].name + "'. Expected " + to_string(subs[sub_idx].arguments.size()) + " but found " + to_string(args.size()) + ".", program[i].lnum);
				return false;
			}

			//******************** ASSEMBLE EXPANDED blocksub **************//

			vector<line> fill;
			line new_line;

			//for each line in blocksub block...
			for (size_t l = 0 ; l < subs[sub_idx].contents.size() ; l++){

				new_line.str = subs[sub_idx].contents[l].str;
				new_line.lnum = program[i].lnum;


				//Replace each instance of each argument
				for (size_t r = 0 ; r < subs[sub_idx].arguments.size() ; r++){
					gstd::findAndReplace(new_line.str, subs[sub_idx].arguments[r], args[r]);
				}

				//Add new line to fill
				trim_whitespace(new_line.str);
				fill.push_back(new_line);
				cout << "\t\t" << new_line.str << endl;
			}

			//Erase call
			program.erase(program.begin()+i, program.begin()+i+1);

			//Insert expanded while loop
			program.insert(program.begin()+i, fill.begin(), fill.end());

			//Change program index
			i += fill.size(); //TODO: I think the increment system I'm using here and elsewhere is off by a few lines

		}
	}

}

/*
Finds every abstract memory location in the program and adds them all to a list.
When an AML is repeated, it will not be repeated in the list.
*/
void get_all_amls(vector<line>& program, vector<aml>& amls){

	string aml_name;
	std::vector<string> words;

	//For each line in program
	for (size_t i = 0 ; i < program.size() ; i++){

		//If @ does not appear, continue to next line
		if (program[i].str.find("@") == string::npos) continue;

		//Parse into words for identification process.
		string line_str = program[i].str;
		ensure_whitespace(line_str, "(),=;");
		words = parse(line_str, " \t");

		//For each word...
		for (size_t w = 0 ; w < words.size() ; w++){

			//Check if it is an abstracted variable
			if (words[w][0] == '@'){
				aml_name = words[w].substr(1);

				//If so, see if its already recorded
				bool add_aml = true;
				for (size_t a = 0 ; a < amls.size() ; a++){
					if (amls[a].name == aml_name){
						add_aml = false;
						break;
					}
				}

				//Add aml if new
				if (add_aml){
					aml new_aml;
					new_aml.name = aml_name;
					new_aml.phys_addr = 0;
					new_aml.addr_assigned = false;
					new_aml.source_line = program[i].lnum;
					amls.push_back(new_aml);
				}

			}
		}

	}

}

/*

NOTE: This defines contiguous blocks based on `INDEX` and NOT line number. This means
as soon as new code is added or any modifications are made, this must be recalculated. As
such, this should be run right at the end, before phyiscal addresses are assigned.
*/
void get_contiguous_blocks(vector<line>& program, CompilerParams& params, vector<aml> amls, vector<contiguous>& contigs){


	//Initialize temp contiguous struct
	contiguous new_contig;
	new_contig.idx_prog_start = 0;
	new_contig.starting_AML = " --- ";

	//For each line of program
	for (size_t i = 0 ; i < program.size() ; i++ ){

		//Check if a gap due to if-structure's jump ends here
		if (program[i].str.substr(0, 15) == "#HERE @TRUE_IF_"){

			if (i == 0){
				params.warning("Skipped continuous block indicator ('#HERE @TRUE_IF_') on line 0. No preceeding block. There is probably an error in the compiled code.");
				continue;
			}

			//Record line number
			new_contig.idx_prog_end = i-1;
			contigs.push_back(new_contig);

			//Start new contiguous block
			new_contig.idx_prog_start = i;
			new_contig.starting_AML = program[i].str.substr(7);


		}

	}

	new_contig.idx_prog_end = program.size()-1;
	contigs.push_back(new_contig);
}

/*
Prints the program.

Options:
i - print index in addition to line number
*/
void print_program(vector<line> program, string options){

	bool print_index = (options.find("i") != string::npos);


	string start_str;
	for (size_t i = 0 ; i < program.size() ; i++){
		start_str = "[" + to_string(program[i].lnum) + "]";
		if (print_index) start_str = start_str + ", " + to_string(i);
		while (start_str.length() < 10) start_str = " " + start_str;
		start_str = start_str + ": ";
		cout << start_str << program[i].str << endl;
	}
}

/*
Runs print_program with additional information displayed such as contiguous
block visualizations.
*/
void rich_print_program(vector<line> program, vector<contiguous> contigs, CompilerParams& params, string options){

	for (size_t c = 0 ; c < contigs.size() ; c++){

		// cout << "BLOCK " + to_string(c) << endl;

		if (contigs[c].idx_prog_end > program.size() || contigs[c].idx_prog_start > program.size()){
			params.error("Quark compiler caused an error. Contiguous block larger than program detected in program print operation.");
		}

		vector<line>::const_iterator first = program.begin() + contigs[c].idx_prog_start;
		vector<line>::const_iterator last = program.begin() + contigs[c].idx_prog_end;
		vector<line> subvec(first, last);

		print_program(subvec, options);
		if (c+1 < contigs.size()){
			cout << "\t\t.\n\t\t.\n\t\t." << endl;
		}
	}

}

/*
Prints the list of blocksubs.
*/
void print_blocksubs(vector<blocksub> subs){

	string start_str;

	for (size_t i = 0 ; i < subs.size() ; i++){

		cout << "blocksub: '" << subs[i].name << "' (Decl: " << to_string(subs[i].declare_line) << ")" << endl;
		cout << "    ARGUMENTS: ";
		for (size_t j = 0 ; j < subs[i].arguments.size() ; j++){
			cout << subs[i].arguments[j];
			if (j+1 < subs[i].arguments.size()) cout << ", ";
		}
		if (subs[i].arguments.size() == 0) cout << "NONE" << endl;
		cout << endl;
		for (size_t j = 0 ; j < subs[i].contents.size() ; j++){

			start_str = "[" + to_string(subs[i].contents[j].lnum) + "]:";
			while (start_str.length() < 9) start_str = " " + start_str;
			cout << start_str << subs[i].contents[j].str << endl;
		}
		cout << endl;
	}
}

/*
Print table of AMLs
*/
void print_amls(vector<aml> amls){

	KTable kt;
	kt.table_title("Abstracted Memory Locations:");
	kt.row({"AML Name", "Physical Address", "Source Line"});

	// cout << "ABSTRACTED MEMORY LOCATIONS:" << endl;
	for (size_t a = 0 ; a < amls.size() ; a++){

		string paddr = to_string(amls[a].phys_addr);
		if (!amls[a].addr_assigned){
			paddr = " ----- ";
		}

		kt.row({amls[a].name, paddr, to_string(amls[a].source_line)});
		// cout << "\t" << to_string(a) << ".: " << amls[a].name << endl;
	}

	cout << kt.str() << endl;

}

/*
Print table of contiguous blocks
*/
void print_contiguous(vector<contiguous> contigs){

	// size_t idx_prog_start;
	// size_t idx_prog_end;
	// string starting_AML;

	KTable kt;
	kt.table_title("Contiguous Blocks:");
	kt.row({"Starting AML", "Start Address", "End Address"});

	for (size_t a = 0 ; a < contigs.size() ; a++){

		kt.row({contigs[a].starting_AML, to_string(contigs[a].idx_prog_start), to_string(contigs[a].idx_prog_end)});
	}

	cout << kt.str() << endl;

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
			trim_whitespace(temp_line.str);
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
			trim_whitespace(temp_line.str);
			temp_line.lnum = input.lnum;

			if (temp_line.str.length() > 0) block_contents.push_back(temp_line);
		}
	}

	return true;
}

/*
Returns true if name is valid variable name:
	* only numbers, letters, and underscores
	* first char is letter
	* len > 0
*/
bool is_valid_name(string s){

	//Ensure at least one character
	if (s.length() < 1) return false;

	//Ensure first character is letter
	if (!isalpha(s[0])) return false;

	//TODO: Check 's' is not a keyword, including true or false, directives, etc

	//Ensure all characters are number, letter, or underscore
	for (size_t i = 1 ; i < s.length() ; i++){
		if (!isalnum(s[i]) && s[i] != '_') return false;
	}

	return true;
}

/*
Checks if a line begins with the name of a blocksub. This function is only
used by the expand blocksub function but helps keep the code readable. The
blocksub name will have a caret appended to it to match the language's syntax.

Accepts 'line' to check, and 'subs' as a the list of blocksubs.

Returns true if the first word in the line is a valid blocksub callsign.
*/
bool begins_with_subname(string line, vector<blocksub> subs){

	//Remove arguments/parentheses
	ensure_whitespace(line, "(,)");

	//Parse string
	vector<string> words = parse(line, " \t");

	//Check words is not empty, string not zero len
	if (words.size() < 1) return false;
	if (words[0].length() < 1) return false;

	//Check first char is caret, then remove
	if (words[0][0] != '^') return false;
	words[0] = words[0].substr(1);

	//Check if first word is in subs vector
	for (size_t i = 0 ; i < subs.size() ; i++){

		//If a match is found, return true
		if (subs[i].name == words[0] ) return true;
	}

	return false;
}






























//
