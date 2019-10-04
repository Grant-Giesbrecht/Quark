#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <IEGA/string_manip.hpp>
#include <IEGA/KTable.hpp>
#include <fstream>

//CXCOMPILE g++ hackomp.cpp -o hackomp -lIEGA -std=c++11
//CXCOMPILE ./hackomp example3.hack -v
//CXGENRUN FALSE

using namespace std;

typedef struct{
	string str;
	size_t lnum;
}line;

//PURGES
void purge_comments(vector<line>& program, bool verbose);

//EXPANSIONS
void expand_while_statements(vector<line>& program, bool verbose);
bool expand_if_statements(vector<line>& program, bool verbose);
bool expand_subroutine_statements(vector<line>& program, bool verbose);

//OTHER
void print_program(vector<line>& program);

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
		remove_end_whitespace(line_in); //remove leading, trailing whitespace

		//Create 'line' struct
		line templine;
		templine.str = line_in;
		templine.lnum = line_num;

		program.push_back(templine); //Add line to program
	}

	//***********************************************************************//
	//*************************** EXPAND, COMPILE, ETC **********************//

	if (!keep_comments) purge_comments(program, verbose);

	// expand_while_statements();










	if (verbose) print_program(program);


	return 0;
}

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
Prints the program
*/
void print_program(vector<line>& program){
	for (size_t i = 0 ; i < program.size() ; i++){
		cout << "[" << to_string(program[i].lnum) << "]: " << program[i].str << endl;
	}
}




































//
