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
#include "LutTypes.hpp"

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

// /*
// Takes the instruction, word, and phase as arguments and returns a ROM address which
// must contain that data.
//
// NOTE: This function is CPU specific. The weights at the top of the function
// definition must be modified for new CPU designs.
// */
// int get_address_Zeta3(int phase, int instruction, int word){
//
// 	int phase_weights[4] = {13, 8, 9, 11};
// 	int instr_weights[7] = {14, 12, 7, 6, 5, 4, 10};
// 	// int word_weights[4] = {0, 1, 2, 3};
// 	int word_weights[4] = {3, 2, 1, 0}; //Flipped for BSM-159,1
//
// 	vector<bool> bin_phase = gstd::int_to_bin(phase, 4);
// 	vector<bool> bin_inst = gstd::int_to_bin(instruction, 6);
// 	vector<bool> bin_word = gstd::int_to_bin(word, 4);
//
// 	int shifted_phase = 0;
// 	for (size_t i = 0 ; i < 4 ; i++){
// 		if (bin_phase[i]) shifted_phase += round(pow(2, phase_weights[i]));
// 	}
//
// 	int shifted_instr = 0;
// 	for (size_t i = 0 ; i < 6 ; i++){
// 		if (bin_inst[i]) shifted_instr += round(pow(2, instr_weights[i]));
// 	}
//
// 	int shifted_word = 0;
// 	for (size_t i = 0 ; i < 4 ; i++){
// 		if (bin_word[i]) shifted_word += round(pow(2, word_weights[i]));
// 	}
//
// 	int addr = shifted_phase + shifted_instr + shifted_word;
// 	return addr;
//
// }

// bool sort_intline(int_line x, int_line y){
// 	return x.addr < y.addr;
// }

// vector<string> generate_LUT(InstructionSet is, bool useBadInstruc){
//
// 	//
// 	// map<string, operation>::iterator it;
// 	// map<int, map<int, map<int, bool> > >::iterator phase_it;
// 	// map<int, map<int, bool> >::iterator word_it;
// 	// map<int, bool>::iterator pin_it;
//
// 	// vector<int_line> int_lines;
// 	// int_line temp_il;
// 	// int_lines.reserve(ops.size()*16*16); //16 phases, 16 words
//
// 	for (size_t inst_no = 0 ; inst_no < 128 ; inst_no++){ // For each instruction... (Number of instructions: 2^7)
//
// 		// If does not have instruction, populate with null instruction, move to next
// 		if ( !is.hasInstruction(inst_no) ){
// 			is.addNullInst(inst_no, useBadInstruc);
// 			continue;
// 		}
//
// 		for (size_t phs_no = 0 ; phs_no < 16 ; phs_no++){ // For each phase...
//
// 			// If all phases reached, either skip remainder or mark as bad instructions
// 			if ( phs_no >= is.numPhase(inst_no) ){
// 				if (useBadInstruc){
// 					cout << "Bad Instruction not implemented!" << endl;
// 				}else{
// 					break;
// 				}
// 			}
//
// 			for (size_t ch_no = 0 ; ch_no < 16 ; ch_no++){ // For each channel/word
//
// 				int word_val = 0;
//
// 				//NOTE: Unlike all other indexing, the pin is counting from 1, not 0!
// 				for (size_t pin_no = 1 ; pin_no <= 8 ; pin_no++){ // For each pin
//
// 					// Check positive/negative logic status
// 					bool active_low = is.getActiveLow(ch_no, pin_no);
//
// 					// Get value from the ISD contained in IS. Will be:
// 					// 0 = OFF,  1 = ON,  -1  = MISSING
// 					int mapped_val = is.getPinValue(inst_no, phs_no, ch_no, pin_no);
//
// 					// Handle missing values
// 					if (mapped_val == -1){
// 						mapped_val = is.getDefaultValue(ch_no, pin_no);
// 					}
//
// 					// Determine if the pin's bit is set
// 					bool setBit = (mapped_val != 0);
// 					if (active_low) setBit = !setBit;
//
// 					// If bit is set, add to word
// 					if (setBit){
// 						word_val += round(pow(2, pin_no-1 ));
// 					}
//
// 				} // Pin LOOP
//
// 			} // Word LOOP
//
// 		} // Phase LOOP
//
//
// 	} // Instruction LOOP
//
// 	// //For each operation/instruction
// 	// for ( it = ops.begin(); it != ops.end(); it++){
// 	//
// 	// 	// cout << it->first << endl;
// 	//
// 	// 	//For each phase
// 	// 	for (phase_it = it->second.ctrls.begin() ; phase_it != it->second.ctrls.end() ; phase_it++){
// 	//
// 	// 		// cout << "\t" << phase_it->first << endl;
// 	//
// 	// 		//For each word
// 	// 		for (word_it = phase_it->second.begin() ; word_it != phase_it->second.end() ; word_it++){
// 	//
// 	// 			// cout << "\t\t" << word_it->first << endl;
// 	//
// 	// 			int pin_byte = 0;
// 	//
// 	// 			//For each pin
// 	// 			for (pin_it = word_it->second.begin() ; pin_it != word_it->second.end() ; pin_it++){
// 	//
// 	// 				//Get active low status
// 	// 				bool is_active_low = get_active_low(controls, word_it->first, pin_it->first);
// 	//
// 	// 				//Add bit if true
// 	// 				if (pin_it->second){
// 	// 					if (!is_active_low){
// 	// 						pin_byte += round(pow(2, pin_it->first));
// 	// 					}
// 	//
// 	// 				}else if (is_active_low){
// 	// 					pin_byte += round(pow(2, pin_it->first));
// 	// 				}
// 	//
// 	// 			}
// 	//
// 	// 			temp_il.byte = pin_byte;
// 	// 			temp_il.addr = get_address_Zeta3(phase_it->first, it->second.instruction_no, word_it->first);
// 	//
// 	// 			// //Handle flag clear condition
// 	// 			// bool used = false;
// 	// 			// if (it->second.flag == FLAG_X || it->second.flag == FLAG_CLR){
// 	// 			// 	temp_il.addr = get_address_Zeta3(phase_it->first, 0, it->second.instruction_no, word_it->first);
// 	// 			// 	int_lines.push_back(temp_il);
// 	// 			// 	used = true;
// 	// 			// }
// 	// 			//
// 	// 			// //Handle flag set condition
// 	// 			// if (it->second.flag == FLAG_X || it->second.flag == FLAG_CLR){
// 	// 			// 	 word_it->first);
// 	// 			int_lines.push_back(temp_il);
// 	// 			// 	used = true;
// 	// 			// }
// 	// 			//
// 	// 			// if (!used){
// 	// 			// 	cout << "ERROR: Operation not used!" << endl;
// 	// 			// }
// 	//
// 	// 		} //End word loop
// 	// 	}//End phase loop
// 	// }// End operation loop
//
// 	//Sort int_lines
// 					// sort(int_lines.begin(), int_lines.end(), sort_intline);
// 					//
// 					// //Generate output LUT as strings from address and byte data
// 					// vector<string> lines;
// 					// lines.reserve(int_lines.size());
// 					// string line;
// 					// for(size_t i = 0 ; i < int_lines.size() ; i++){
// 					// 	line = to_string(int_lines[i].addr) + ":" + to_string(int_lines[i].byte);
// 					// 	lines.push_back(line);
// 					// }
// 					//
// 					// return lines;
//
// }

// vector<string> generate_LUT(map<string, operation> ops, vector<control_line> controls){
//
// 	//
// 	map<string, operation>::iterator it;
// 	map<int, map<int, map<int, bool> > >::iterator phase_it;
// 	map<int, map<int, bool> >::iterator word_it;
// 	map<int, bool>::iterator pin_it;
//
// 	vector<int_line> int_lines;
// 	int_line temp_il;
// 	int_lines.reserve(ops.size()*16*16); //16 phases, 16 words
//
// 	//For each operation/instruction
// 	for ( it = ops.begin(); it != ops.end(); it++){
//
// 		// cout << it->first << endl;
//
// 		//For each phase
// 		for (phase_it = it->second.ctrls.begin() ; phase_it != it->second.ctrls.end() ; phase_it++){
//
// 			// cout << "\t" << phase_it->first << endl;
//
// 			//For each word
// 			for (word_it = phase_it->second.begin() ; word_it != phase_it->second.end() ; word_it++){
//
// 				// cout << "\t\t" << word_it->first << endl;
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
// 				temp_il.addr = get_address_Zeta3(phase_it->first, it->second.instruction_no, word_it->first);
//
// 				// //Handle flag clear condition
// 				// bool used = false;
// 				// if (it->second.flag == FLAG_X || it->second.flag == FLAG_CLR){
// 				// 	temp_il.addr = get_address_Zeta3(phase_it->first, 0, it->second.instruction_no, word_it->first);
// 				// 	int_lines.push_back(temp_il);
// 				// 	used = true;
// 				// }
// 				//
// 				// //Handle flag set condition
// 				// if (it->second.flag == FLAG_X || it->second.flag == FLAG_CLR){
// 				// 	 word_it->first);
// 				int_lines.push_back(temp_il);
// 				// 	used = true;
// 				// }
// 				//
// 				// if (!used){
// 				// 	cout << "ERROR: Operation not used!" << endl;
// 				// }
//
// 			} //End word loop
// 		}//End phase loop
// 	}// End operation loop
//
// 	//Sort int_lines
// 	sort(int_lines.begin(), int_lines.end(), sort_intline);
//
// 	//Generate output LUT as strings from address and byte data
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

// void print_lut(vector<string> bcm){
// 	for (size_t i = 0 ; i < bcm.size() ; i++){
// 		cout << bcm[i] << endl;
// 	}
// }
//
// bool save_lut(string filename, vector<string> bcm){
//
//
// 	srand(time(NULL));
//
// 	ofstream file;
// 	file.open (filename);
//
// 	if (!file.is_open()){
// 		return false;
// 	}
//
// 	for (size_t i = 0 ; i < bcm.size() ; i++){
// 		file << bcm[i] << endl;
// 	}
//
// 	file.close();
//
// 	return true;
//
// }

// void print_contents(vector<fline> fl){
//
// 	for (size_t i = 0 ; i < fl.size() ; i++){
//
// 		cout << "[" << fl[i].lnum << "]: " << fl[i].str << endl;
//
// 	}
//
// }

int main(int argc, char** argv){

	InstructionSet is;

	// std::vector<control_line> controls;
	// map<string, operation> ops;
	// isv_data isv;
	// isd_internal isdi;

	// Get control wiring filename
	std::string cw_filename;
	if (argc < 3){
		cw_filename = "./Source Files/blinkenrechner.cw";
	}else{
		cw_filename = string(argv[2]);
	}

	// Read Control Wiring file
	cout << "Reading (CW): " << argv[2] << endl;
	if (!is.load_cw(cw_filename)){
		cout << "Exiting" << endl;
		return -1;
	}

	// Print control wiring
	is.print_cw();


	// Get ISD filename
	std::string isd_filename;
	if (argc < 2){
		isd_filename = "./Source Files/blinkenrechner.isd";
	}else{
		isd_filename = argv[1];
	}

	// Read ISD file
	cout << "Reading (ISD): " << isd_filename << endl;
	if (!is.load_isd(isd_filename)){
		cout << "Exiting" << endl;
		return -1;
	}

	// Print Instruction Set Description
	is.print_isd();

	// Print summary
	is.print_operation_summary();

	// Generate LUT
	is.generate_LUT(false);

	// Display lookup table
	is.print_lut();

	// Get Look-Up Table Filename
	string lut_filename = "./ISVs/blinkenrechner.lut";

	// Save LUT file
	if (is.save_lut(lut_filename)){
		is.log.msg("Successfully saved LUT data to file '" + lut_filename);
	}else{
		is.log.msg("ERROR: Failed to write file '" + lut_filename);
	}

	// Print messages
	cout << "\nTarget Architecture: " << is.isv.arch << endl;
	cout << "\nISV Series: " << is.isv.series << endl;

	cout << is.log.all() << endl;

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
