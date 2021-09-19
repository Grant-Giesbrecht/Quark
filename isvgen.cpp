/*
Takes a .ISD and .CW and creates an instruction set version in a .LUT file.

TODO:
* Fix: if two instructions have the same instruction number, this will not be detected, no error will be given

--------------------------------------------------------------------------------
Created by G. Giesbrecht
14-8-2021
*/

//CXCOMPILE make isvgen

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

#include "subatomic.hpp"

/*
Calculate an address (in the ROM lookup-table) from a phase, flag, instruction,
and word. TODO: How have flags changed?

Maps (LSB -> MSB):
	Instruction: 4, 5, 6, 7, 12, 14
	Phase: 11, 9, 8, 13
	Flag: 10
	Word: 0, 1, 2, 3
*/
int get_address_Zeta2(int phase, int flag, int instruction, int word){

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
	int word_weights[4] = {3, 2, 1, 0};

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

typedef struct{
	int addr;
	int byte;
}int_line;

bool sort_intline(int_line x, int_line y){
	return x.addr < y.addr;
}

vector<string> generate_LUT(map<string, operation> ops, vector<control_line> controls){

	//
	map<string, operation>::iterator it;
	map<int, map<int, map<int, bool> > >::iterator phase_it;
	map<int, map<int, bool> >::iterator word_it;
	map<int, bool>::iterator pin_it;

	vector<int_line> int_lines;
	int_line temp_il;
	int_lines.reserve(ops.size()*4*16); //4 phases, 16 words

	//For each operation
	for ( it = ops.begin(); it != ops.end(); it++){

		// cout << it->first << endl;

		//For each phase
		for (phase_it = it->second.ctrls.begin() ; phase_it != it->second.ctrls.end() ; phase_it++){

			// cout << "\t" << phase_it->first << endl;

			//For each word
			for (word_it = phase_it->second.begin() ; word_it != phase_it->second.end() ; word_it++){

				// cout << "\t\t" << word_it->first << endl;

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
				temp_il.addr = get_address_Zeta3(phase_it->first, it->second.instruction_no, word_it->first);

				// //Handle flag clear condition
				// bool used = false;
				// if (it->second.flag == FLAG_X || it->second.flag == FLAG_CLR){
				// 	temp_il.addr = get_address_Zeta3(phase_it->first, 0, it->second.instruction_no, word_it->first);
				// 	int_lines.push_back(temp_il);
				// 	used = true;
				// }
				//
				// //Handle flag set condition
				// if (it->second.flag == FLAG_X || it->second.flag == FLAG_CLR){
				// 	 word_it->first);
				int_lines.push_back(temp_il);
				// 	used = true;
				// }
				//
				// if (!used){
				// 	cout << "ERROR: Operation not used!" << endl;
				// }

			} //End word loop
		}//End phase loop
	}// End operation loop

	//Sort int_lines
	sort(int_lines.begin(), int_lines.end(), sort_intline);

	//Generate output LUT as strings from address and byte data
	vector<string> lines;
	lines.reserve(int_lines.size());
	string line;
	for(size_t i = 0 ; i < int_lines.size() ; i++){
		line = to_string(int_lines[i].addr) + ":" + to_string(int_lines[i].byte);
		lines.push_back(line);
	}

	return lines;

}

// vector<string> generate_bcm_old(map<string, operation> ops, vector<control_line> controls){
//
// 	map<string, operation>::iterator it;
// 	map<int, map<int, map<int, bool> > >::iterator phase_it;
// 	map<int, map<int, bool> >::iterator word_it;
// 	map<int, bool>::iterator pin_it;
//
// 	vector<int_line> int_lines;
// 	int_line temp_il;
// 	int_lines.reserve(ops.size()*4*16); //4 phases, 16 words
//
// 	//For each operation
// 	for ( it = ops.begin(); it != ops.end(); it++){
//
// 		cout << it->first << endl;
//
// 		//For each phase
// 		for (phase_it = it->second.ctrls.begin() ; phase_it != it->second.ctrls.end() ; phase_it++){
//
// 			cout << "\t" << phase_it->first << endl;
//
// 			//For each word
// 			for (word_it = phase_it->second.begin() ; word_it != phase_it->second.end() ; word_it++){
//
// 				cout << "\t\t" << word_it->first << endl;
//
// 				int pin_byte = 0;
//
// 				//For each pin
// 				for (pin_it = word_it->second.begin() ; pin_it != word_it->second.end() ; pin_it++){
//
// 					//Get active low status
// 					bool is_active_low = get_active_low(controls, word_it->first, pin_it->first);
//
// 					//Add bit if true
// 					if (pin_it->second){
// 						if (!is_active_low){
// 							pin_byte += round(pow(2, pin_it->first));
// 						}
//
// 					}else if (is_active_low){
// 						pin_byte += round(pow(2, pin_it->first));
// 					}
//
// 				}
//
// 				temp_il.byte = pin_byte;
//
// 				//Handle flag clear condition
// 				bool used = false;
// 				if (it->second.flag == FLAG_X || it->second.flag == FLAG_CLR){
// 					temp_il.addr = get_address_Zeta2(phase_it->first, 0, it->second.instruction_no, word_it->first);
// 					int_lines.push_back(temp_il);
// 					used = true;
// 				}
//
// 				//Handle flag set condition
// 				if (it->second.flag == FLAG_X || it->second.flag == FLAG_CLR){
// 					temp_il.addr = get_address_Zeta2(phase_it->first, 1, it->second.instruction_no, word_it->first);
// 					int_lines.push_back(temp_il);
// 					used = true;
// 				}
//
// 				if (!used){
// 					cout << "ERROR: Operation not used!" << endl;
// 				}
//
// 			} //End word loop
// 		}//End phase loop
// 	}// End operation loop
//
// 	//Sort int_lines
// 	sort(int_lines.begin(), int_lines.end(), sort_intline);
//
// 	vector<string> lines;
// 	lines.reserve(int_lines.size());
// 	string line;
// 	for(size_t i = 0 ; i < int_lines.size() ; i++){
// 		line = to_string(int_lines[i].addr) + ":" + to_string(int_lines[i].byte);
// 		lines.push_back(line);
// 	}
//
// 	return lines;
//
// }

void print_lut(vector<string> bcm){
	for (size_t i = 0 ; i < bcm.size() ; i++){
		cout << bcm[i] << endl;
	}
}

bool save_lut(string filename, vector<string> bcm){


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

void print_contents(vector<fline> fl){

	for (size_t i = 0 ; i < fl.size() ; i++){

		cout << "[" << fl[i].lnum << "]: " << fl[i].str << endl;

	}

}

int main(int argc, char** argv){

	std::vector<control_line> controls;
	map<string, operation> ops;
	isv_data isv;
	isd_internal isdi;

	std::string cw_filename;
	if (argc < 3){
		cw_filename = "./Source Files/blinkenrechner.cw";
	}else{
		cw_filename = string(argv[2]);
		cout << "Reading (CW): " << argv[2] << endl;
	}

	if (!read_CW(cw_filename, controls)){
		cout << "Exiting" << endl;
		return -1;
	}

	print_controls(controls);

	std::string isd_filename;
	if (argc < 2){
		isd_filename = "./Source Files/blinkenrechner.isd";
	}else{
		isd_filename = argv[1];
	}

	cout << "Reading (ISD): " << isd_filename << endl;
	if (!read_ISD(isd_filename, controls, ops, isv, isdi)){
		cout << "Exiting" << endl;
		return -1;
	}

	print_contents(isdi.contents);

	print_operation_summary(ops);



	std::vector<string> lut = generate_LUT(ops, controls);

	cout << endl;
	print_lut(lut);

	string lut_out = "./ISVs/blinkenrechner.lut";
	if (save_lut(lut_out, lut)){
		cout << "Successfully saved LUT data to file '" << lut_out << "'." << endl;
	}else{
		cout << "ERROR: Failed to write file '" << lut_out << "'!" << endl;
	}

	cout << "\nTarget Architecture: " << isv.arch << endl;
	cout << "\nISV Series: " << isv.series << endl;

	return 0;
}

// int main(int argc, char** argv){
//
// 	std::vector<control_line> controls;
// 	map<string, operation> ops;
//
//
// 	return 0;
// }
