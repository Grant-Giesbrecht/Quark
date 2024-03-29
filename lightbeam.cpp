#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <fstream>
// #include <IEGA/string_manip.hpp>
#include "gstd.hpp"
#include <sstream>
#include <bitset>
#include <map>
#include <cmath>
#include <ctgmath>

//CXCOMPILE make

#define NUM_WORD 8

#define COUT_ERROR cout << "ERROR [f='" << readfile << "'][L=" << to_string(line_num) << "]: "

using namespace gstd;
using namespace std;

#define FLAG_X 'X'
#define FLAG_SET 'S'
#define FLAG_CLR 'C'

typedef struct{
	string name;
	int instruction_no;
	int data_bits;
	char flag;
	map <int, map<int, map<int, bool> > > ctrls;
}operation;

typedef struct{
	string name;
	int word;
	int pin;
	bool active_low;
	bool default_to_on;
}control_line;


/*
Clears the operation's data fields
*/
void clear_operation(operation& x){
	x.name = "";
	x.instruction_no = -1;
	x.data_bits = -1;
	x.ctrls.clear();
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
*/
 bool read_OPF(string readfile, std::vector<control_line> controls, map<std::string, operation>& output){

	output.clear();

	map<std::string, operation> ops;

	vector<string> words;

	size_t line_num = 0;

	//read through file
	ifstream file(readfile.c_str());
	if (file.is_open()) {
		string line;

		operation nextOp;

		while (getline(file, line)) {
			line_num++;

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
			gstd::ensure_whitespace(line, "*=@:");
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

				//Set flag state
				if (words.size() == 5){
					if (words[4] == "^0"){
						nextOp.flag = FLAG_CLR;
					}else if(words[4] == "^1"){
						nextOp.flag = FLAG_SET;
					}else{
						COUT_ERROR << "Invlaid token '" << words[4] << "' for flag specifier (^0/^1)." << endl;
					}
				}else{
					nextOp.flag = FLAG_X;
				}


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

	return true;
}

/*
Prints a vector of controL-line  structs.
*/
void print_controls(vector<control_line> controls){

	for (size_t i = 0 ; i < controls.size() ; i++){
		cout << "************** " << controls[i].name << " *************" << endl;
		cout << "\tWord: " << to_string(controls[i].word) << endl;
		cout << "\tPin: " << to_string(controls[i].pin) << endl;
		cout << "\tActive low: " << bool_to_str(controls[i].active_low) << endl;
		cout << "\tDefault to on: " << bool_to_str(controls[i].default_to_on) << endl;
		cout << endl;
	}
}

/*
Prints a map of strings to operations.
*/
void print_operations(map<string, operation> ops, size_t pin_cols = 4){

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
Calculate an address (in the ROM lookup-table) from a phase, flag, instruction,
and word. TODO: How have flags changed?

Maps (LSB -> MSB):
	Instruction: 4, 5, 6, 7, 12, 14
	Phase: 11, 9, 8, 13
	Flag: 10
	Word: 0, 1, 2, 3
*/
int get_address(int phase, int flag, int instruction, int word){

	int phase_weights[4] = {11, 9, 8, 13};
	int instr_weights[6] = {4, 5, 6, 7, 12, 14};
	int word_weights[4] = {0, 1, 2, 3};
	int flag_weight = 10;

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

	int shifted_flag = 0;
	if (flag == 1){
		shifted_flag = round(pow(2, flag_weight));
	}


	int addr = shifted_phase + shifted_instr + shifted_word + shifted_flag;
	cout << "P:" << phase << " F:" << flag << " I:" << instruction << " W:" << word << " >> [" << addr << "] " << endl;
	cout << "\t\t" << bin_to_str(bin_phase) << " " << bin_to_str(bin_inst) << " " << bin_to_str(bin_word) << endl;
	return addr;

}

typedef struct{
	int addr;
	int byte;
}int_line;

bool sort_intline(int_line x, int_line y){
	return x.addr < y.addr;
}

vector<string> generate_bcm(map<string, operation> ops, vector<control_line> controls){

	map<string, operation>::iterator it;
	map<int, map<int, map<int, bool> > >::iterator phase_it;
	map<int, map<int, bool> >::iterator word_it;
	map<int, bool>::iterator pin_it;

	vector<int_line> int_lines;
	int_line temp_il;
	int_lines.reserve(ops.size()*4*16); //4 phases, 16 words

	//For each operation
	for ( it = ops.begin(); it != ops.end(); it++){

		cout << it->first << endl;

		//For each phase
		for (phase_it = it->second.ctrls.begin() ; phase_it != it->second.ctrls.end() ; phase_it++){

			cout << "\t" << phase_it->first << endl;

			//For each word
			for (word_it = phase_it->second.begin() ; word_it != phase_it->second.end() ; word_it++){

				cout << "\t\t" << word_it->first << endl;

				int pin_byte = 0;

				//For each pin
				for (pin_it = word_it->second.begin() ; pin_it != word_it->second.end() ; pin_it++){

					//Get active low status
					bool is_active_low = get_active_low(controls, word_it->first, pin_it->first);

					//Add bit if true
					if (pin_it->second){
						if (!is_active_low){
							pin_byte += round(pow(2, pin_it->first));
						}

					}else if (is_active_low){
						pin_byte += round(pow(2, pin_it->first));
					}

				}

				temp_il.byte = pin_byte;

				//Handle flag clear condition
				bool used = false;
				if (it->second.flag == FLAG_X || it->second.flag == FLAG_CLR){
					temp_il.addr = get_address(phase_it->first, 0, it->second.instruction_no, word_it->first);
					int_lines.push_back(temp_il);
					used = true;
				}

				//Handle flag set condition
				if (it->second.flag == FLAG_X || it->second.flag == FLAG_CLR){
					temp_il.addr = get_address(phase_it->first, 1, it->second.instruction_no, word_it->first);
					int_lines.push_back(temp_il);
					used = true;
				}

				if (!used){
					cout << "ERROR: Operation not used!" << endl;
				}

			} //End word loop
		}//End phase loop
	}// End operation loop

	//Sort int_lines
	sort(int_lines.begin(), int_lines.end(), sort_intline);

	vector<string> lines;
	lines.reserve(int_lines.size());
	string line;
	for(size_t i = 0 ; i < int_lines.size() ; i++){
		line = to_string(int_lines[i].addr) + ":" + to_string(int_lines[i].byte);
		lines.push_back(line);
	}

	return lines;

}

void print_bcm(vector<string> bcm){
	for (size_t i = 0 ; i < bcm.size() ; i++){
		cout << bcm[i] << endl;
	}
}

bool save_bcm(string filename, vector<string> bcm){


	srand(time(NULL));

	ofstream file;
	file.open (filename);

	if (!file.is_open()){
		return false;
	}

	for (size_t i = 0 ; i < bcm.size() ; i++){
		file << bcm[i] << endl;
	}

	file.close();

	return true;

}

int main(int argc, char** argv){

	std::vector<control_line> controls;
	map<string, operation> ops;

	if (argc < 3){
		read_CW("./opfiles/memorydelta.cw", controls);
	}else{
		read_CW(argv[1], controls);
		cout << "Reading (CW): " << argv[2] << endl;
	}

	print_controls(controls);

	if (argc < 2){
		read_OPF("./opfiles/memorydelta.opf", controls, ops);
	}else{
		read_OPF(argv[1], controls, ops);
		cout << "Reading (OPF): " << argv[1] << endl;
	}

	print_operations(ops);

	std::vector<string> bcm = generate_bcm(ops, controls);

	cout << endl;
	print_bcm(bcm);

	string bcm_out = "./opfiles/memorydelta.bcm";
	if (save_bcm(bcm_out, bcm)){
		cout << "Successfully saved BCM data to file '" << bcm_out << "'." << endl;
	}else{
		cout << "ERROR: Failed to write file '" << bcm_out << "'!" << endl;
	}

	return 0;
}
