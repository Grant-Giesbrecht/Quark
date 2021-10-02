#include <string>
#include <vector>
#include <iostream>
#include <map>
#include "gstd.hpp"
#include "subatomic.hpp"
#include "gcolors.hpp"

class InstructionSet{

public:

	InstructionSet();

	bool load_cw(std::string filename);
	bool load_isd(std::string filename);

	void print_cw();
	void print_isd();
	void print_operation_summary(size_t pin_cols = 4, size_t desc_len = 25);

	bool hasInstruction(size_t inst_no);
	std::string getInstKey(size_t inst_no);
	size_t getControlWireIdx(int word, int pin, bool& found);
	size_t numPhase(size_t inst_no);
	void addNullInst(size_t inst_no, bool useBadInstruc);
	bool getActiveLow(size_t channel, size_t pin);
	int getPinValue(size_t inst_no, size_t phs_no, size_t ch_no, size_t pin_no);
	int getDefaultValue(size_t ch_no, size_t pin_no);

	void generate_LUT(bool useBadInstruc);

	// Read from files
	std::vector<control_line> ctrls;
	isd_internal isd_contents;
	isv_data isv;

	// Created from ctrls and isd_contents
	std::map<std::string, operation> ops;

	// Created from ops
	vector<int_line> lut_int;
	vector<std::string> lut_str;


};

InstructionSet::InstructionSet(){
	//Do nothing
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



}

bool InstructionSet::getActiveLow(size_t channel, size_t pin){
// Returns true if the specified control line is active low. Returns false if it
// uses positive logic.


	// Get index of control wire in 'ctrls'
	bool found_idx;
	size_t idx = getControlWireIdx(channel, pin, found_idx);
	if (!found_idx){
		cout << gcolor::red << "WARNING: Shit! Failed to find the wire :'('" << gcolor::normal << endl;
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
