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
#include <filesystem>
#include <gcolors.hpp>

#include <GLogger.hpp>

// #include "LutTypes.hpp"

#ifndef SUBATOMIC_HPP
#define SUBATOMIC_HPP

#define NUM_WORD 8

#define COUT_ERROR cout << "ERROR [f='" << readfile << "'][L=" << to_string(line_num) << "]: "

using namespace gstd;
using namespace std;
namespace fs = std::filesystem;
namespace gc = gstd::gcolor;

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
	string path;
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
	int addr;
	int byte;
}int_line;

typedef struct{
	std::vector<fline> contents;
	map<std::string, isd_repl_block> repl;
}isd_internal;

class InstructionSet{

public:

	InstructionSet();

	bool load_cw(std::string filename);
	bool load_isd(std::string filename);

	void print_cw();
	void print_isd();
	void print_operation_summary(size_t pin_cols = 4, size_t desc_len = 25);
	void print_lut();

	bool hasInstruction(size_t inst_no);
	std::string getInstKey(size_t inst_no);
	size_t getControlWireIdx(int word, int pin, bool& found);
	size_t numPhase(size_t inst_no);
	void addNullInst(size_t inst_no, bool useBadInstruc);
	bool getActiveLow(size_t channel, size_t pin);
	int getPinValue(size_t inst_no, size_t phs_no, size_t ch_no, size_t pin_no);
	int getDefaultValue(size_t ch_no, size_t pin_no);

	void generate_LUT(bool useBadInstruc);

	bool save_lut(std::string filename);

	bool null_instruc_warning_given;

	// Read from files
	std::vector<control_line> ctrls;
	isd_internal isd_contents;
	isv_data isv;

	// Created from ctrls and isd_contents
	std::map<std::string, operation> ops;

	// Created from ops
	vector<int_line> lut_int;
	vector<std::string> lut_str;

	// Create logger
	GLogger log;

};

bool sort_intline(int_line x, int_line y){
	return x.addr < y.addr;
}

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
Takes the instruction, word, and phase as arguments and returns a ROM address which
must contain that data.

NOTE: This function is CPU specific. The weights at the top of the function
definition must be modified for new CPU designs.
*/
int get_address_Zeta3(int phase, int instruction, int word){

	int phase_weights[4] = {13, 8, 9, 11};
	int instr_weights[7] = {14, 12, 7, 6, 5, 4, 10};
	// int word_weights[4] = {0, 1, 2, 3};
	int word_weights[4] = {3, 2, 1, 0}; //Flipped for BSM-159,1

	vector<bool> bin_phase = gstd::int_to_bin(phase, 4);
	vector<bool> bin_inst = gstd::int_to_bin(instruction, 6);
	vector<bool> bin_word = gstd::int_to_bin(word, 4);

	int shifted_phase = 0;
	for (size_t i = 0 ; i < 4 ; i++){
		if (bin_phase[i]) shifted_phase += round(pow(2, phase_weights[i]));
	}

	int shifted_instr = 0;
	for (size_t i = 0 ; i < 6 ; i++){
		if (bin_inst[i]) shifted_instr += round(pow(2, instr_weights[i]));
	}

	int shifted_word = 0;
	for (size_t i = 0 ; i < 4 ; i++){
		if (bin_word[i]) shifted_word += round(pow(2, word_weights[i]));
	}

	int addr = shifted_phase + shifted_instr + shifted_word;
	return addr;

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

?2 I think the above description is wrong and what it actually does is takes the *already populated* phases and fills them with default cases for missing words. ie. it doesn't add null phases, but rather completes the LUT with default data for everything that wasn't populated explicitly in the .ISD file.
?

*/
void fill_defaults(operation& nextOp, std::vector<control_line>& controls){

	map<int, map<int, map<int, bool> > >::iterator phase_it;
	map<int, map<int, bool> >::iterator word_it;
	map<int, bool>::iterator pin_it;

	//For each phase that is populated...
	for (phase_it = nextOp.ctrls.begin() ; phase_it != nextOp.ctrls.end() ; phase_it++){

		//For each word
		// for (word_it = phase_it->second.begin() ; word_it != phase_it->second.end() ; word_it++){
		for (int word = 0 ; word < NUM_WORD ; word++){

			// If `word` is not found in `ctrls`...
			if (nextOp.ctrls[phase_it->first].find(word) == nextOp.ctrls[phase_it->first].end()){

				// Add to ctrls
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
Unspecified instructions should report a 'bad instruction'.

How to handle an undefined phase?
*/
void fill_missing_instructions(map<std::string, operation>& ops){

	long int max_inst_code = 127;

	//Create template for 'badinstruc' ie. a non-defined instruction
	operation badinst;
	badinst.name = "BADINST";
	badinst.data_bits = 0;
	badinst.subsystem = GENERAL_OPERATION;
	badinst.desc = "";
	badinst.prgm_replac = "";

	//TODO: Finish! (22-8-2021)

	// //Scan over all possible instructions
	// for (for size_t inst_code = 0 ; inst_code <= max_inst_code ){
	//
	// 	// instruction was not defined
	// 	if (){
	//
	// 		//Replace with badinstruc
	//
	// 	}
	//
	// }

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

*/
bool load_conf(std::string readfile, std::map<std::string, std::string>& settings){

	settings.clear();

	vector<string_idx> words;

	size_t line_num = 0;

	//read through file
	ifstream file(readfile.c_str());
	if (file.is_open()) {

		std::string line;

		control_line nextCtrl;

		while (getline(file, line)) {

			line_num++;

			trim_whitespace(line); //Remove whitespace from line

			if (line.length() == 0) continue; //Continue if blank
			if (line.length() >= 2 && line.substr(0, 2) == "//") continue; //Skip full-comment lines

			//Immediately remove comments
			trim_end_comment(line, "//"); //Remove end comments

			//Parse words
			gstd::ensure_whitespace(line, "=");
			words = gstd::parseIdx(line, " \t");

			//Ensure words exist
			if (words.size() < 1){
				continue;
			}

			//Check for fewer than min characters
			if (words.size() < 3){
				COUT_ERROR << "Too few words." << endl;
				return false;
			}

			//Check for missing colon
			if (words[1].str != "="){
				COUT_ERROR << "2nd token (" << words[1].str << ") must be colon." << endl;
				return false;
			}

			//Get name
			settings[words[0].str] = line.substr(words[2].idx);

			// Trim whitespace
			trim_whitespace(settings[words[0].str]);

		}
		file.close();
	}else{
		cout << "ERROR: Failed to read '" << readfile << "'." << endl;
		return false;
	}

	return true;

}

void show_conf(std::map<std::string, std::string> settings){

	for(std::map<string,string>::iterator it = settings.begin(); it != settings.end(); it++) {
		std::cout << "\t" << it->first <<  " = " << it->second << std::endl;
	}

}

isv_data newest_version(std::vector<isv_data> ver){

	size_t max_maj = 0;
	size_t max_fs = 0;
	size_t max_patch = 0;

	// Find max major
	for (size_t i = 0 ; i < ver.size() ; i++){
		if (ver[i].major > max_maj) max_maj = ver[i].major;
	}

	// Find max featureset
	for (size_t i = 0 ; i < ver.size() ; i++){
		if ((ver[i].major == max_maj) && (ver[i].minor > max_fs)) max_fs = ver[i].minor;
	}

	// Find max patch
	for (size_t i = 0 ; i < ver.size() ; i++){
		if ((ver[i].major == max_maj) && (ver[i].minor == max_fs) && (ver[i].patch > max_patch)) max_patch = ver[i].patch;
	}

	// Return match
	for (size_t i = 0 ; i < ver.size() ; i++){
		if ((ver[i].major == max_maj) && (ver[i].minor == max_fs) && (ver[i].patch == max_patch)){
			return ver[i];
		}
	}

	cout << "Error! 'newest_version()' doesn't work!" << endl;
	isv_data blank;
	return blank;
}

bool read_arch_version(isv_data id, InstructionSet& is){

	bool found_cw = false;
	bool found_isd = false;
	std::string cw_path = "";
	std::string isd_path = "";

	// Search path of specified directory for CW and ISD files
	for (const auto & entry : fs::directory_iterator(id.path)){

		// Check if file is desired type
		if (to_upper(entry.path().extension()) == "CW"){
			cw_path = entry.path();
			found_isd = true;
		}else if(to_upper(entry.path().extension()) == "ISD"){
			isd_path = entry.path();
			found_cw = true;
		}
	}

	// CHeck that files were found
	if(!found_cw || !found_isd){
		cout << "Failed to find path!" << endl;
		return false;
	}

	if (!is.load_cw(cw_path)){
		cout << "Exiting" << endl;
		return false;
	}

	// Read ISD File
	if (!is.load_isd(isd_path)){
		cout << "Exiting" << endl;
		return false;
	}

	return true;
}


/*
Given a path to a archive directory for architecture 'arch_name', reads all files
and returns a vector of isv_data structs containing ea. directory's version
*/
std::vector<isv_data> get_arch_titles(std::string arch_path, std::string arch_name){

	std::vector<isv_data> isv_titles;
	std::vector<std::string> words;

	// For each directory...
	for (const auto & entry : fs::directory_iterator(arch_path)){

		// Check if it is a directory
		if (entry.is_directory()){

			// Get file root and check for series and version
			words = parse(entry.path().stem(), "_");

			// Check format
			if (words.size() != 4){
				cout << "Warning: Unrecognized directory name in archive: " << entry.path() << endl;
				continue;
			}

			isv_data nd;
			nd.arch = arch_name;
			nd.series = words[0];
			nd.path = entry.path();

			try{
				nd.major = stoi(words[1]);
				nd.minor = stoi(words[2]);
				nd.patch = stoi(words[3]);
			}catch(...){
				cout << "Warning: Unrecognized directory name in archive: " << entry.path() << endl;
				continue;
			}

			// Add to list
			isv_titles.push_back(nd);

		}

	}

	return isv_titles;
}

void show_dir_contents(std::string dir_path){
	for (const auto & entry : fs::directory_iterator(dir_path)){
		if (entry.is_directory()){
			std::cout << "DIR: " << entry.path() << std::endl;
		}else{
			std::cout << entry.path() << std::endl;
			cout << "\troot_name: " << entry.path().root_name() << endl;
			cout << "\troot_directory: " << entry.path().root_directory() << endl;
			cout << "\troot_path: " << entry.path().root_path() << endl;
			cout << "\trelative_path: " << entry.path().relative_path() << endl;
			cout << "\tparent_path: " << entry.path().parent_path() << endl;
			cout << "\tfilename: " << entry.path().filename() << endl;
			cout << "\tstem: " << entry.path().stem() << endl;
			cout << "\textension: " << entry.path().extension() << endl;
		}

	}
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
			if (line.length() >= 2 && line.substr(0, 2) == "//") continue; //Skip full-comment lines

			//Immediately remove comments
			trim_end_comment(line, "//"); //Remove end comments

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

// void print_controls(vector<control_line> controls){
//
// 	KTable kt;
//
// 	kt.table_title("Control Wiring Summary");
// 	kt.row({"Ctrl Line Name", "Ctrl Word", "Pin", "Active Low", "Default State"});
//
// 	std::vector<std::string> trow;
// 	for (size_t i = 0 ; i < controls.size() ; i++){
//
// 		trow.clear();
//
// 		trow.push_back(controls[i].name);
// 		trow.push_back(to_string(controls[i].word));
// 		trow.push_back(to_string(controls[i].pin));
// 		trow.push_back(bool_to_str(controls[i].active_low));
// 		if (controls[i].default_to_on){
// 			trow.push_back("ON");
// 		}else{
// 			trow.push_back("OFF");
// 		}
//
//
// 		kt.row(trow);
// 	}
//
// 	cout << kt.str() << endl;
//
// }

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
// void print_operation_summary(map<string, operation> ops, size_t pin_cols = 4, size_t desc_len = 25){
//
// 	map<string, operation>::iterator it;
// 	map<int, map<int, map<int, bool> > >::iterator phase_it;
// 	map<int, map<int, bool> >::iterator word_it;
// 	map<int, bool>::iterator pin_it;
//
// 	KTable kt;
//
// 	kt.table_title("ISD Operation Summary");
// 	kt.row({"Operation", "Phases", "Operation Code", "No. Data Bytes", "Subprocessor", "Description", "Prgm Inst. Mapping"});
//
// 	std::vector<std::string> trow;
// 	std::string desc_str;
//
// 	//For each operation
// 	for ( it = ops.begin(); it != ops.end(); it++){
//
// 		trow.clear();
// 		trow.push_back(it->second.name);
// 		trow.push_back(""); //Phases
// 		trow.push_back(to_string(it->second.instruction_no));
// 		trow.push_back(to_string(it->second.data_bits));
//
// 		if (it->second.subsystem == ALU_OPERATION){
// 			trow.push_back("ALU");
// 		}else if (it->second.subsystem == ALU_OPERATION){
// 			trow.push_back("FPU");
// 		}else{
// 			trow.push_back("-");
// 		}
//
// 		desc_str = it->second.desc;
// 		findAndReplaceAll(desc_str, "\n", "\\\\ ");
// 		if (desc_str.length() > desc_len){
// 			desc_str = desc_str.substr(0, desc_len-3) + "...";
// 		}
// 		trow.push_back(desc_str);
//
// 		trow.push_back(it->second.prgm_replac);
//
//
//
// 		kt.row(trow);
//
// 	}
//
// 	cout << kt.str() << endl;
//
// }

void isv_explorer(map<string, operation> ops, vector<control_line> controls){



}

//------------------------------------------------------------------------------
// These bits were in LutTypes, then I jumbled up the importing with a circular
// import. Now its all in one.
//------------------------------------------------------------------------------

// class InstructionSet{
//
// public:
//
// 	InstructionSet();
//
// 	bool load_cw(std::string filename);
// 	bool load_isd(std::string filename);
//
// 	void print_cw();
// 	void print_isd();
// 	void print_operation_summary(size_t pin_cols = 4, size_t desc_len = 25);
// 	void print_lut();
//
// 	bool hasInstruction(size_t inst_no);
// 	std::string getInstKey(size_t inst_no);
// 	size_t getControlWireIdx(int word, int pin, bool& found);
// 	size_t numPhase(size_t inst_no);
// 	void addNullInst(size_t inst_no, bool useBadInstruc);
// 	bool getActiveLow(size_t channel, size_t pin);
// 	int getPinValue(size_t inst_no, size_t phs_no, size_t ch_no, size_t pin_no);
// 	int getDefaultValue(size_t ch_no, size_t pin_no);
//
// 	void generate_LUT(bool useBadInstruc);
//
// 	bool save_lut(std::string filename);
//
// 	bool null_instruc_warning_given;
//
// 	// Read from files
// 	std::vector<control_line> ctrls;
// 	isd_internal isd_contents;
// 	isv_data isv;
//
// 	// Created from ctrls and isd_contents
// 	std::map<std::string, operation> ops;
//
// 	// Created from ops
// 	vector<int_line> lut_int;
// 	vector<std::string> lut_str;
//
// 	// Create logger
// 	GLogger log;
//
// };

InstructionSet::InstructionSet(){
	//Do nothing

	null_instruc_warning_given = false;
}

bool InstructionSet::load_cw(std::string filename){

	return read_CW(filename, ctrls);

}

bool InstructionSet::load_isd(std::string filename){

	return read_ISD(filename, ctrls, ops, isv, isd_contents);

}

void InstructionSet::print_cw(){

	KTable kt;

	kt.table_title("Control Wiring Summary");
	kt.row({"Ctrl Line Name", "Ctrl Word", "Pin", "Active Low", "Default State"});

	std::vector<std::string> trow;
	for (size_t i = 0 ; i < ctrls.size() ; i++){

		trow.clear();

		trow.push_back(ctrls[i].name);
		trow.push_back(to_string(ctrls[i].word));
		trow.push_back(to_string(ctrls[i].pin));
		trow.push_back(bool_to_str(ctrls[i].active_low));
		if (ctrls[i].default_to_on){
			trow.push_back("ON");
		}else{
			trow.push_back("OFF");
		}


		kt.row(trow);
	}

	cout << kt.str() << endl;

}

void InstructionSet::print_isd(){

	for (size_t i = 0 ; i < isd_contents.contents.size() ; i++){

		cout << "[" << isd_contents.contents[i].lnum << "]: " << isd_contents.contents[i].str << endl;

	}

}


void InstructionSet::print_operation_summary(size_t pin_cols, size_t desc_len){

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

bool InstructionSet::hasInstruction(size_t inst_no){
// Returns true if the object's isd_contents contains an instruction with number
// inst_no.

	return (getInstKey(inst_no) != "");
}

std::string InstructionSet::getInstKey(size_t inst_no){
// Searches for an instuction based on its inst_no and returns the key to access
// it. If not found, returns empty string.

	map<std::string, operation>::iterator i_it;

	// Loop through all instructions in map
	for (i_it = ops.begin() ; i_it != ops.end() ; i_it++){

		// Check if matches instuction number
		if (i_it->second.instruction_no == inst_no){
			return i_it->first;
		}
	}

	return "";

}

size_t InstructionSet::getControlWireIdx(int word, int pin, bool& found){
// Returns the index (of ctrls) where the control_line for 'word' and 'pin' can
// be found. If it is found, 'found' will be set true, otehrwise it will be set
// false. NOTE: Expects the pin numbers in the control_line vector (ie. from the
// CW file) to be from 0-7, but the pins number argument spans from 1-8 and
// shifts down by one in the code.

	// Scan over all control lines
	for (size_t idx = 0 ; idx < ctrls.size() ; idx++){
		// cout << ctrls[idx].word << " " << ctrls[idx].pin << " " << word << " " << pin << endl;
		if ((ctrls[idx].word == word) && (ctrls[idx].pin == pin-1)){
			found  = true;
			return idx;
		}
	}

	found = false;
	return 0;

}

size_t InstructionSet::numPhase(size_t inst_no){
// Returns number of phases. NOTE: This is not the same as the maximum phase
// number. If 3 phases existed, this would return 3, despite the max phase being
// 2.

	std::string key = getInstKey(inst_no);

	return ops[key].ctrls.size();

}

void InstructionSet::addNullInst(size_t inst_no, bool useBadInstruc){
// Populates the LUT with a null instruction for instruction number 'inst_no'.
// If useBadInstruc is true, will replace non-fetch phases with bad instruction
// markers. Otherwise, the null instruction will just contain fetch instructions.

	if (!null_instruc_warning_given){
		cout << gcolor::yellow << "WARNING: Null instruction not provided." << gcolor::normal << endl;
		log.warning("Null instruction not provided.");
		null_instruc_warning_given = true;
	}

}

bool InstructionSet::getActiveLow(size_t channel, size_t pin){
// Returns true if the specified control line is active low. Returns false if it
// uses positive logic.


	// Get index of control wire in 'ctrls'
	bool found_idx;
	size_t idx = getControlWireIdx(channel, pin, found_idx);
	if (!found_idx){
		cout << gcolor::red << "WARNING: Shit! Failed to find the wire :'(" << gcolor::normal << endl;
		log.error("Shit! Failed to find the wire :'(", true);
		return -1;
	}

	// Return active low
	return ctrls[idx].active_low;

}

int InstructionSet::getPinValue(size_t inst_no, size_t phs_no, size_t ch_no, size_t pin_no){
// Takes an instruction/phase/channel/pin input, and returns the ON/OFF state the
// ISD file specified for it.
//	0 = OFF
//  1 = ON
// -1 = MISSING

	pin_no = pin_no - 1;
	// Should be bounded by 1-8. If > 100, means subtracted from zero and overflw
	// In that case, function is being called with wrong pin indexing (0-7
	// instead of 1-8).
	if (pin_no > 100){
		cout << gcolor::red << "WARNING: Wrong pin indexing. THis is definitely going to crash!" << gcolor::normal << endl;
		log.error("Wrong pin indexing. THis is definitely going to crash!", true);
	}

	// Get instruction key
 	std::string key = getInstKey(inst_no);

	// Fetch value
	try{

		return ops[key].ctrls[phs_no][ch_no][pin_no];

	}catch(...){ // Handle case of missing value
		return -1;
	}


}

int InstructionSet::getDefaultValue(size_t ch_no, size_t pin_no){
// Returns the defualt value (0 = OFF, 1 = ON) for the pin specified by ch_no and pin_no

	// Get index of control wire in 'ctrls'
	bool found_idx;
	size_t idx = getControlWireIdx(ch_no, pin_no, found_idx);
	if (!found_idx){
		cout << gcolor::red << "WARNING: Shit! Failed to find the wire :'(" << gcolor::normal << endl;
		log.error("Shit! Failed to find the wire :'(", true);
		return -1;
	}

	// Return default value
	if (ctrls[idx].default_to_on){
		return 1;
	}else{
		return 0;
	}

}

void InstructionSet::generate_LUT(bool useBadInstruc){

	//
	// map<string, operation>::iterator it;
	// map<int, map<int, map<int, bool> > >::iterator phase_it;
	// map<int, map<int, bool> >::iterator word_it;
	// map<int, bool>::iterator pin_it;


	int_line temp_il;
	lut_int.reserve(ops.size()*16*16); //16 phases, 16 words TODO: Should this be 128*16*16?

	for (size_t inst_no = 0 ; inst_no < 128 ; inst_no++){ // For each instruction... (Number of instructions: 2^7)

		// If does not have instruction, populate with null instruction, move to next
		if ( !hasInstruction(inst_no) ){
			addNullInst(inst_no, useBadInstruc);
			continue;
		}

		for (size_t phs_no = 0 ; phs_no < 16 ; phs_no++){ // For each phase...

			// If all phases reached, either skip remainder or mark as bad instructions
			if ( phs_no >= numPhase(inst_no) ){
				if (useBadInstruc){
					cout << "Bad Instruction not implemented!" << endl;
					log.warning("Bad Instruction requested but *not* implemeented!");
					useBadInstruc = false; // TODO: remove this when implement bad instruc
				}else{
					break;
				}
			}

			for (size_t ch_no = 0 ; ch_no < 16 ; ch_no++){ // For each channel/word

				int word_val = 0;

				//NOTE: Unlike all other indexing, the pin is counting from 1, not 0!
				for (size_t pin_no = 1 ; pin_no <= 8 ; pin_no++){ // For each pin

					// Check positive/negative logic status
					bool active_low = getActiveLow(ch_no, pin_no);

					// Get value from the ISD contained in IS. Will be:
					// 0 = OFF,  1 = ON,  -1  = MISSING
					int mapped_val = getPinValue(inst_no, phs_no, ch_no, pin_no);

					// Handle missing values
					if (mapped_val == -1){
						mapped_val = getDefaultValue(ch_no, pin_no);
					}

					// Determine if the pin's bit is set
					bool setBit = (mapped_val != 0);
					if (active_low) setBit = !setBit;

					// If bit is set, add to word
					if (setBit){
						word_val += round(pow(2, pin_no-1 ));
					}

				} // Pin LOOP

				// Save to lut_int
				temp_il.byte = word_val;
				temp_il.addr = get_address_Zeta3(phs_no, inst_no, ch_no);
				lut_int.push_back(temp_il);

			} // Word LOOP

		} // Phase LOOP


	} // Instruction LOOP

	// Sort int_lines
	sort(lut_int.begin(), lut_int.end(), sort_intline);

	//Generate output LUT as strings from address and byte data
	lut_str.reserve(lut_int.size());
	string line;
	for(size_t i = 0 ; i < lut_int.size() ; i++){
		line = to_string(lut_int[i].addr) + ":" + to_string(lut_int[i].byte);
		lut_str.push_back(line);
	}

}

void InstructionSet::print_lut(){
	for (size_t i = 0 ; i < lut_str.size() ; i++){
		cout << lut_str[i] << endl;
	}
}

bool InstructionSet::save_lut(std::string filename){


	srand(time(NULL));

	ofstream file;
	file.open (filename);

	if (!file.is_open()){
		return false;
	}

	for (size_t i = 0 ; i < lut_str.size() ; i++){
		file << lut_str[i] << endl;
	}

	file.close();

	return true;

}



#endif
