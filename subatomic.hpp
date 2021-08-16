#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <fstream>
// #include <IEGA/string_manip.hpp>
#include "gstd.hpp"
#include <ktable.hpp>
#include <sstream>
#include <bitset>
#include <map>
#include <cmath>
#include <ctgmath>

#define NUM_WORD 8

#define COUT_ERROR cout << "ERROR [f='" << readfile << "'][L=" << to_string(line_num) << "]: "

using namespace gstd;
using namespace std;

#define ALU_OPERATION 'X'
#define FPU_OPERATION 'S'
#define GENERAL_OPERATION 'C'

typedef struct{
	string name;
	int instruction_no;
	int data_bits;
	char subsystem;
	map <int, map<int, map<int, bool> > > ctrls; //Phase-Word-Pin (Data)
	string desc;
	string prgm_replac;
}operation;

typedef struct{
	string name;
	int word;
	int pin;
	bool active_low;
	bool default_to_on;
}control_line;

typedef struct{
	string arch;
	string series;
	int major;
	int minor;
	int patch;
}isv_data;

typedef struct{
	size_t lnum;
	std::string str;
}fline;

typedef struct{
	std::vector<fline> blk_content;
	size_t num_arg_expect;
}isd_repl_block;

typedef struct{
	std::vector<fline> contents;
	map<std::string, isd_repl_block> repl;
}isd_internal;

void findAndReplaceAll(std::string & data, std::string toSearch, std::string replaceStr){
    // Get the first occurrence
    size_t pos = data.find(toSearch);
    // Repeat till end is reached
    while( pos != std::string::npos){
        // Replace this occurrence of Sub String
        data.replace(pos, toSearch.size(), replaceStr);
        // Get the next occurrence from the current position
        pos =data.find(toSearch, pos + replaceStr.size());
    }
}

/*
Clears the operation's data fields
*/
void clear_operation(operation& x){
	x.name = "";
	x.instruction_no = -1;
	x.data_bits = -1;
	x.ctrls.clear();
	x.desc = "";
	x.prgm_replac = "";
	x.subsystem = GENERAL_OPERATION;
}

/*
Finds the control line at pin X word Y and returns its default value. True indicates
default = ON, False indicates default = OFF.

If no control is found, retuns val_not_found value.
*/
bool get_control_default(std::vector<control_line> controls, int word, int pin, bool val_not_found=false){

	for (size_t c = 0 ; c < controls.size() ; c++){
		if (controls[c].word == word && controls[c].pin == pin){
			return controls[c].default_to_on;
		}
	}

	return val_not_found;
}

/*
Finds the control line at pin X word Y and returns its active low setting. True
indicates it IS active low, false indicates it is not.

If no control is found, retuns val_not_found value.
*/
bool get_active_low(std::vector<control_line> controls, int word, int pin, bool val_not_found=false){

	for (size_t c = 0 ; c < controls.size() ; c++){
		if (controls[c].word == word && controls[c].pin == pin){
			return controls[c].active_low;
		}
	}

	return val_not_found;
}

/*

? I think this takes an instruction/operation description that is half-populated,
and fills the remaining phases with the default state. Because Z-III can abort
an instruction early, it is probably neccesary for me to change this to instead
skip to the next instruction. Furthermore, I should probably name it something
like 'finish_operation()'
?

*/
void fill_defaults(operation& nextOp, std::vector<control_line>& controls){

	map<int, map<int, map<int, bool> > >::iterator phase_it;
	map<int, map<int, bool> >::iterator word_it;
	map<int, bool>::iterator pin_it;

	//For each phase
	for (phase_it = nextOp.ctrls.begin() ; phase_it != nextOp.ctrls.end() ; phase_it++){

		//For each word
		// for (word_it = phase_it->second.begin() ; word_it != phase_it->second.end() ; word_it++){
		for (int word = 0 ; word < NUM_WORD ; word++){

			if (nextOp.ctrls[phase_it->first].find(word) == nextOp.ctrls[phase_it->first].end()){
				map<int, bool> temp_map;
				nextOp.ctrls[phase_it->first][word] = temp_map;
			}

			//For each pin
			for (int p = 0 ; p < 8 ; p++){

				//If not listed, add default value
				// if (word_it->second.find(p) == word_it->second.end()){ //Pin not listed, take value from defaults
				// 	nextOp.ctrls[phase_it->first][word_it->first][p] = get_control_default(controls, word_it->first, p);
				// }
				if (nextOp.ctrls[phase_it->first][word].find(p) == nextOp.ctrls[phase_it->first][word].end()){ //Pin not listed, take value from defaults
					nextOp.ctrls[phase_it->first][word][p] = get_control_default(controls, word, p);
				}

			}

		}


	}


}

/*
Accepts a control line name and a list of controls and sets the control line's word
and pin in the variables word and pin. Returns true if found.
*/
bool get_word_pin(std::string ctrl_line, std::vector<control_line> controls, int& word, int& pin){

	std::map<int, std::map<int, bool> > intMap;

	for (size_t i = 0 ; i < controls.size() ; i++){
		if (controls[i].name == ctrl_line){
			word = controls[i].word;
			pin = controls[i].pin;
			return true;
		}
	}

	return false;

}

/*
Reads a .cw (control wiring) file. This file describes how control lines are
physically addressed in the CPU, and thus allows them to be assigned addresses in
the LUT.

For syntax on the .cw file format, see "CW File Syntax.md".
*/
bool read_CW(string readfile, std::vector<control_line>& controls){

	controls.clear();

	map<std::string, operation> ops;

	vector<string> words;

	size_t line_num = 0;

	//read through file
	ifstream file(readfile.c_str());
	if (file.is_open()) {

		string line;

		control_line nextCtrl;

		while (getline(file, line)) {

			line_num++;

			trim_whitespace(line); //Remove whitespace from line

			if (line.length() == 0) continue; //Continue if blank
			if (line.length() >= 2 && line.substr(0, 2) == "//") continue; //Skip comments

			//Parse words
			gstd::ensure_whitespace(line, "*=@:!");
			words = gstd::parse(line, " \t");

			//Ensure words exist
			if (words.size() < 1){
				continue;
			}

			//Check for fewer than min characters
			if (words.size() < 5){
				COUT_ERROR << "Too few words." << endl;
				return false;
			}

			//Check for missing colon
			if (words[2] != ":"){
				COUT_ERROR << "3rd token (" << words[2] << ") must be colon." << endl;
				return false;
			}

			//Get name
			nextCtrl.name = words[0];

			//Get word
			try{
				nextCtrl.word = stoi(words[1]);
			}catch(std::invalid_argument){
				COUT_ERROR << "Failed to convert " << words[1] << " to int for word num" << endl;
				return false;
			}

			//Get pin
			try{
				nextCtrl.pin = stoi(words[3]);
			}catch(std::invalid_argument){
				COUT_ERROR << "Failed to convert " << words[3] << " to int for pin num" << endl;
				return false;
			}

			//Get default state
			if (words[4] == "ON"){
				nextCtrl.default_to_on = true;
			}else if (words[4] == "OFF"){
				nextCtrl.default_to_on = false;
			}else{
				COUT_ERROR << "Invalid value. Word 5 (" << words[5] << ") is neither ON/OFF. " << endl;
			}

			//Set active low
			if (words.size() == 6 && words[5] == "!"){
				nextCtrl.active_low = true;
			}else{
				nextCtrl.active_low = false;
			}

			controls.push_back(nextCtrl);

		}
		file.close();
	}else{
		cout << "ERROR: Failed to read '" << readfile << "'." << endl;
		return false;
	}

	return true;

}

/*
Reads an operation file (.OPF) and returns a map of operations.

readfile - filename to read
controls - To check that control lines exist. Also (I think) fills the remaining phases with 'defaults'. TODO: Remove the mandate for filling all phases.
output - Populates 'output' with the operation definition data in the file
isv - Populates 'isv' with the ISV data in the file
isdi - Populates with the file contents, and the replacement blocks. ONly useful for debugging

*/
 bool read_ISD(string readfile, std::vector<control_line> controls, map<std::string, operation>& output, isv_data& isv, isd_internal& isdi){

	output.clear();

	map<std::string, operation> ops;

	map<std::string, isd_repl_block > replacements;

	bool found_arch = false;
	bool found_series = false;

	std::vector<fline> contents;

	vector<string> words;

	size_t line_num = 0;

	//read through file
	ifstream file(readfile.c_str());
	if (file.is_open()) {
		string line;

		operation nextOp;

		// Read through file, save to 'contents' (after replaceing REPL block)
		while (getline(file, line)) {

			line_num++;

			gstd::ensure_whitespace(line, "*=@?:()");
			words = gstd::parse(line, " \t");

			//Ensure words exist
			if (words.size() < 1){
				continue;
			}

			//Look for definitions or replacements
			if (words[0] == "#DEF"){

				isd_repl_block def;

				if (words.size() < 2){
					COUT_ERROR << "Too few words in '#DEF' block." << endl;
					return false;
				}

				//Check for argument description
				if (words.size() == 5){
					def.num_arg_expect = stoi(words[3]);
				}else{
					def.num_arg_expect = 0;
				}

				std::string rep_name = words[1];

				while (getline(file, line)) {

					line_num++;

					gstd::ensure_whitespace(line, "*=@?:");
					words = gstd::parse(line, " \t");

					//Ensure words exist
					if (words.size() < 1){
						continue;
					}

					// Check if this is end of definition...
					if (words[0] == "#END"){ //If so, save to replacement list
 						replacements[rep_name] = def;
						break;
					}else{ // Otherwise save line to new replacement being built
						fline fn;
						fn.lnum = line_num;
						fn.str = line;
						def.blk_content.push_back(fn);
					}
				}

			}else if(words[0] == "#REPL"){

				// Check at least #REPL and replacement block name provided
				if (words.size() < 2){
					COUT_ERROR << "Too few words in '#REPL' statement." << endl;
					return false;
				}

				// Check that requirested repl.block exists
				if (replacements.find(words[1]) == replacements.end()){
					COUT_ERROR << "#REPL call accessed an undefined block '" << words[1] << "'." << endl;
					return false;
				}

				// If block requires arguments, ensure arguments are provided
				if (words.size() - 2 != replacements[words[1]].num_arg_expect){
					COUT_ERROR << "'#REPL' statement missing required arguments (" << replacements[words[1]].num_arg_expect << ") for block '" << words[1] << "'." << endl;
					return false;
				}

				//Make a copy of block contents. Replace any arguments
				vector<fline> block;
				fline fn;
				size_t arg_idx = 2; //Count the index in 'words' for the next argument to sub
				string modstr;
				for (size_t bi = 0 ; bi < replacements[words[1]].blk_content.size() ; bi++){ //For each line in the block...

					modstr = replacements[words[1]].blk_content[bi].str;
					size_t str_idx;

					// Loop until all '@' are found and replaced with arguments
					while (true){

						//Search for an '@'
						 str_idx = modstr.find("@");

						//If '@' found:
						if (str_idx != std::string::npos){

							// Make sure haven't exceeded argument count
							if (arg_idx-1 > words.size()){
								COUT_ERROR << "Too many argument place holders in the block '" << words[1] << "'." << endl;
								return false;
							}

							// Replace '@' with next argument (next argument is in 'words'). Increment arg_idx
							modstr = modstr.substr(0, str_idx) + words[arg_idx++] + modstr.substr(str_idx+1);
						}else{
							break;
						}

					}

					//Add to block
					fn.str = modstr;
					fn.lnum = replacements[words[1]].blk_content[bi].lnum;
					block.push_back(fn);

				}

				contents.insert(std::end(contents), std::begin(block), std::end(block));

			}else{

				fline fn;
				fn.lnum = line_num;
				fn.str = line;
				contents.push_back(fn);

			}

		}

		for (size_t cidx = 0 ; cidx < contents.size() ; cidx++){

			line = contents[cidx].str;
			line_num = contents[cidx].lnum;

			trim_whitespace(line); //Remove whitespace from line

			if (line.length() == 0) continue; //Continue if blank
//			if (line.length() >= 2 && line.substr(0, 2) == "//") continue; //Skip comments

			//Trim everything after double forward slash (comment), INCLUDING those in quotes!
			bool skip_line = false;
			for (size_t i = 0 ; i < line.length()-1 ; i++){
				if (line.substr(i, 2) == "//"){
					if (i == 0){
						skip_line = true;
						break;
					}else{
						line = line.substr(0, i-1);
						break;
					}
				}
			}
			if (skip_line) continue;

			//Parse words
			gstd::ensure_whitespace(line, "*=@?:");
			words = gstd::parse(line, " \t");

			//Ensure words exist
			if (words.size() < 1){
				continue;
			}

			if (words[0] == "*"){

				//If initialized, add last op to map
				if (nextOp.name != ""){
					fill_defaults(nextOp, controls);
					ops[nextOp.name] = nextOp;
				}

				//Clear old operation
				clear_operation(nextOp);

				//Ensure correct number of words
				if (words.size() < 4){
					COUT_ERROR << "contains more/less than 4 tokens" << endl;
					return false;
				}

				//Read in fields
				nextOp.name = words[1];
				try{
					nextOp.instruction_no = stoi(words[2]);
				}catch(std::invalid_argument){
					COUT_ERROR << " Failed to convert " << words[2] << " to integer" << endl;
					return false;
				}
				try{
					nextOp.data_bits = stoi(words[3]);
				}catch(std::invalid_argument){
					COUT_ERROR << " Failed to convert " << words[3] << " to integer" << endl;
					return false;
				}

				// //Set flag state
				// if (words.size() == 5){
				// 	if (words[4] == "^0"){
				// 		nextOp.flag = FLAG_CLR;
				// 	}else if(words[4] == "^1"){
				// 		nextOp.flag = FLAG_SET;
				// 	}else{
				// 		COUT_ERROR << "Invlaid token '" << words[4] << "' for flag specifier (^0/^1)." << endl;
				// 	}
				// }else{
				// 	nextOp.flag = FLAG_X;
				// }
			}else if(words[0] == "#PROCESSOR"){

				if (words.size() < 2){
					COUT_ERROR << "Too few words in '#REPL' statement." << endl;
					return false;
				}

				if (words[1] == "FPU"){
					nextOp.subsystem = FPU_OPERATION;
				}else if(words[1] == "ALU"){
					nextOp.subsystem = ALU_OPERATION;
				}else{
					COUT_ERROR << "Unrecognized subsystem processor '" << words[1] << "'" << endl;
					return false;
				}

			}else if (words[0] == "#END"){

				COUT_ERROR << "Extraneous '#END' slipped through!" << endl;
				return false;

			}else if (words[0] == "#REPL"){

				COUT_ERROR << "Extraneous '#REPL' slipped through!" << endl;
				return false;

			}else if (words[0] == "#DEF"){

				COUT_ERROR << "Extraneous '#DEF' slipped through!" << endl;
				return false;

			}else if (words[0] == "#ARCH"){

				if (words.size() < 2){
					COUT_ERROR << "Too few words" << endl;
					return false;
				}

				isv.arch = words[1];

				found_arch = true;

			}else if (words[0] == "#SERIES"){

				if (words.size() < 2){
					COUT_ERROR << "Too few words" << endl;
					return false;
				}

				isv.series = words[1];

				found_series = true;

			}else if (words[0] == "#PRGM"){

				if (words.size() < 2){
					COUT_ERROR << "Too few words" << endl;
					return false;
				}

				nextOp.prgm_replac = words[1];


			}else if (words[0] == "?"){

				//Add new line if previous lines exist
				if (nextOp.desc.length() > 0){
					nextOp.desc = nextOp.desc + "\n";
				}

				std::size_t found = line.find("?");
				nextOp.desc = nextOp.desc + line.substr(found+1);

			}else{

				//Check for fewer than min characters
				if (words.size() < 2){
					COUT_ERROR << "Too few characters." << endl;
					return false;
				}

				//Check for missing colon
				if (words[1] != ":"){
					COUT_ERROR << "2nd token (" << words[1] << ") on phase description line must be colon." << endl;
					return false;
				}

				//Get phase
				int phase;
				try{
					phase = stoi(words[0]);
				}catch(std::invalid_argument){
					COUT_ERROR << "Failed to convert " << words[0] << " to int for phase" << endl;
					return false;
				}

				//For each listed value
				bool nextIsValue = false;
				bool mayBeValue = false;
				string ctrl_name;
				bool value;
				bool addToList = false;
				for (size_t i = 2 ; i < words.size() ; i++){ //Is Equals

					if (words[i] == "="){

						//equals means next is value, if may not be value return false
						if (!mayBeValue){
							COUT_ERROR << "Equals appeared when expecting ctrl_line name" << endl;
							return false;
						}

						//Set that next character is value
						nextIsValue = true;

					}else if(nextIsValue){ //is value
						if (to_upper(words[i]) == "ON"){
							value = true;
						}else if (to_upper(words[i]) == "OFF"){
							value = false;
						}else{
							COUT_ERROR << "Failed to interpret '" << words[i] << "' as bool. " << endl;
							return false;
						}
						mayBeValue = false;
						nextIsValue = false;
					}else{ //Is ctrl name

						//Add to master list if possible
						if (addToList){
							int word, pin;
							if (!get_word_pin(ctrl_name, controls, word, pin)){
								COUT_ERROR << "Failed to find control line '" << ctrl_name << "'." << endl;
								return false;
							}
							nextOp.ctrls[phase][word][pin] = value;
						}

						//Start new control line data
						ctrl_name = words[i];
						value = true;
						addToList = true;
						mayBeValue = true;
					}

				}

				if (addToList){
					int word, pin;
					if (!get_word_pin(ctrl_name, controls, word, pin)){
						COUT_ERROR << "Failed to find control line '" << ctrl_name << "'." << endl;
					}
					nextOp.ctrls[phase][word][pin] = value;
				}



			}

		}

		//If initialized, add last op to map
		if (nextOp.name != ""){
			fill_defaults(nextOp, controls);
			ops[nextOp.name] = nextOp;
		}

		file.close();
	}else{
		cout << "ERROR: Failed to read '" << readfile << "'." << endl;
		return false;
	}

	//Save data to output variable
	output = ops;

	if (!found_arch || !found_series){
		cout << "ERROR: Missing architecture or series statement." << endl;
		return false;
	}

	//Populate 'isdi'
	isdi.contents = contents;
	isdi.repl = replacements;

	return true;
}

/*
Prints a vector of controL-line  structs.
*/
void print_controls_old(vector<control_line> controls){

	for (size_t i = 0 ; i < controls.size() ; i++){
		cout << "************** " << controls[i].name << " *************" << endl;
		cout << "\tWord: " << to_string(controls[i].word) << endl;
		cout << "\tPin: " << to_string(controls[i].pin) << endl;
		cout << "\tActive low: " << bool_to_str(controls[i].active_low) << endl;
		cout << "\tDefault to on: " << bool_to_str(controls[i].default_to_on) << endl;
		cout << endl;
	}
}

void print_controls(vector<control_line> controls){

	KTable kt;

	kt.table_title("Control Wiring Summary");
	kt.row({"Ctrl Line Name", "Ctrl Word", "Pin", "Active Low", "Default State"});

	std::vector<std::string> trow;
	for (size_t i = 0 ; i < controls.size() ; i++){

		trow.clear();

		trow.push_back(controls[i].name);
		trow.push_back(to_string(controls[i].word));
		trow.push_back(to_string(controls[i].pin));
		trow.push_back(bool_to_str(controls[i].active_low));
		if (controls[i].default_to_on){
			trow.push_back("ON");
		}else{
			trow.push_back("OFF");
		}


		kt.row(trow);
	}

	cout << kt.str() << endl;

}

/*
Prints a map of strings to operations.
*/
void print_operations_full(map<string, operation> ops, size_t pin_cols = 4){

	map<string, operation>::iterator it;
	map<int, map<int, map<int, bool> > >::iterator phase_it;
	map<int, map<int, bool> >::iterator word_it;
	map<int, bool>::iterator pin_it;

	//For each operation
	for ( it = ops.begin(); it != ops.end(); it++){

		//Print title
		cout << "******************** " << it->first << " **********************" << endl;

		//For each phase
		for (phase_it = it->second.ctrls.begin() ; phase_it != it->second.ctrls.end() ; phase_it++){

			cout << "\tPhase " << to_string(phase_it->first) << ": " <<endl;

			//For each word
			for (word_it = phase_it->second.begin() ; word_it != phase_it->second.end() ; word_it++){
				cout << "\t\tWord " << to_string(word_it->first) << ": " << endl;

				size_t count = 0;
				cout << "\t\t\t";
				for (pin_it = word_it->second.begin() ; pin_it != word_it->second.end() ; pin_it++){

					//Start newline ever 'x' columns

					if (count%pin_cols == 0 && count != 1){
						cout << endl;
						cout << "\t\t\t";
					}
					count++;

					//Print pin data
					cout << "[" << pin_it->first << "]:" << bool_to_str(pin_it->second) << "\t";
				}
				cout << endl;

			}


		}

	}

}

/*
Prints a map of strings to operations.
*/
void print_operation_summary(map<string, operation> ops, size_t pin_cols = 4, size_t desc_len = 25){

	map<string, operation>::iterator it;
	map<int, map<int, map<int, bool> > >::iterator phase_it;
	map<int, map<int, bool> >::iterator word_it;
	map<int, bool>::iterator pin_it;

	KTable kt;

	kt.table_title("ISD Operation Summary");
	kt.row({"Operation", "Phases", "Operation Code", "No. Data Bytes", "Subprocessor", "Description", "Prgm Inst. Mapping"});

	std::vector<std::string> trow;
	std::string desc_str;

	//For each operation
	for ( it = ops.begin(); it != ops.end(); it++){

		trow.clear();
		trow.push_back(it->second.name);
		trow.push_back(""); //Phases
		trow.push_back(to_string(it->second.instruction_no));
		trow.push_back(to_string(it->second.data_bits));

		if (it->second.subsystem == ALU_OPERATION){
			trow.push_back("ALU");
		}else if (it->second.subsystem == ALU_OPERATION){
			trow.push_back("FPU");
		}else{
			trow.push_back("-");
		}

		desc_str = it->second.desc;
		findAndReplaceAll(desc_str, "\n", "\\\\ ");
		if (desc_str.length() > desc_len){
			desc_str = desc_str.substr(0, desc_len-3) + "...";
		}
		trow.push_back(desc_str);

		trow.push_back(it->second.prgm_replac);



		kt.row(trow);

	}

	cout << kt.str() << endl;

}

void isv_explorer(map<string, operation> ops, vector<control_line> controls){



}
