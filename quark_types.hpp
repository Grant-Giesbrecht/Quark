#ifndef QUARK_TYPES_HPP
#define QUARK_TYPES_HPP

#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <bitset>
#include <map>
#include <cmath>
#include <ctgmath>
#include "subatomic.hpp"

/*
KEY: (Standard) Keyword
ID: Identifier
NUM: Numeric value
OP: Operator
CD: Compiler Directive
SOP: Subroutine Operator
TYP: Type keyword
CMT: Comment
SEP: Seperator
LIT: Literal keyword
INS: Insruction
NL: Newline
*/
enum TokenType {key, id, num, op, cd, sop, typ, cmt, sep, lit, ins, nl, ver}; // Token categories

// Types of statements
enum StatementType {st_declaration, st_machine_code, st_reassignment, st_directive, st_if, st_while, st_subroutine, st_macro, st_expansion, st_subroutine_call};

// Program Memory Locations
enum PmemLoc {pmem_ram, pmem_flash0, pmem_flash1};

typedef struct{
	// Type of token
	TokenType type; // Member of Cats

	bool use_float;

	// Token value
	std::string str;
	int vali;
	double valf;
	bool valb;
	int valv[3]; //For versions (0-> major, 1->minor, 2-> patch)

	// Metadata
	size_t lnum; // Declaration line
}qtoken;

/*
Convert a qtoken to a string for printing
*/
std::string tokenstr(qtoken tok){

	std::string str;

	switch(tok.type){
		case TokenType::key:
			str = "[key: \"" + gcolor::magenta + tok.str + gcolor::normal + "\"]";
			break;
		case TokenType::id:
			str = "[id: \"" + gcolor::magenta + tok.str + gcolor::normal + "\"]";
			break;
		case TokenType::num:
			(tok.use_float) ? str = "[num: " + gcolor::magenta + to_string(tok.valf) +gcolor::normal + "]" : str = "[num: " + gcolor::magenta + to_string(tok.vali) + gcolor::normal + "]";
			break;
		case TokenType::ver:
			str = "[ver: \"" + gcolor::magenta + to_string(tok.valv[0]) + "." + to_string(tok.valv[1]) + "." + to_string(tok.valv[2]) + gcolor::normal + "]";
		case TokenType::op:
			str = "[op: \"" + gcolor::magenta + tok.str + gcolor::normal + "\"]";
			break;
		case TokenType::cd:
			str = "[cd: \"" + gcolor::magenta + tok.str + gcolor::normal + "\"]";
			break;
		case TokenType::sop:
			str = "[sop: \"" + gcolor::magenta + tok.str + gcolor::normal + "\"]";
			break;
		case TokenType::typ:
			str = "[typ: \"" + gcolor::magenta + tok.str + gcolor::normal + "\"]";
			break;
		case TokenType::cmt:
			str = "[cmt: \"" + gcolor::magenta + tok.str + gcolor::normal + "\"]";
			break;
		case TokenType::sep:
			str = "[sep: \"" + gcolor::magenta + tok.str + gcolor::normal + "\"]";
			break;
		case TokenType::lit:
			str = "[lit: \"" + gcolor::magenta + tok.str + gcolor::normal + "\"]";
			break;
		case TokenType::ins:
			str = "[" + gcolor::magenta + "ins" + gcolor::normal + ": \"" + gcolor::magenta + tok.str + gcolor::normal + "\"]";
			break;
		case TokenType::nl:
			str = "[newline]";
			break;
	}

	return str;
}

/*
Convert a qtoken to a string for BPIR
*/
std::string tokenvalstr(qtoken tok, int fmt){

	std::string str;

	switch(tok.type){
		case TokenType::num:
			(tok.use_float) ? str = to_string(tok.valf) : str = to_string(tok.vali);
			break;
		case TokenType::nl: // Should never be used for BPIR
			str = "Newline";
			break;
		case TokenType::ver: // Should never be used for BPIR
			str = "Version";
			break;
		default:
			str = tok.str;
			break;
	}

	return str;
}

bool isVersion(std::string str){

	// Try to find seperators
	size_t idx0 = str.find(".");
	size_t idx1 = str.find(".", idx0+1);

	// Return false if less than 2 punkt
	if (idx0 == std::string::npos || idx1 == std::string::npos) return false;

	// Get string
	std::string maj = str.substr(0, idx0-1);
	std::string min = str.substr(idx0+1, idx1-1);
	std::string patch = str.substr(idx1+1);

	// Return false if any number string is missing (zero length)
	if (maj.length() < 1 || min.length() < 1 || patch.length() < 1) return false;

	// Return false if any strings can't be converted to ints
	try{
		stoi(maj);
		stoi(min);
		stoi(patch);
	}catch(...){
		return false;
	}

	return true;
}

//============================= COMPILER STATE =================================

class CompilerState{
public:
	CompilerState(InstructionSet is, GLogger& log);

	InstructionSet is;
	GLogger& log;

	// Output vector
	std::vector<abs_line> bpir;

	size_t next_line; // Next empty line in memory
	int compile_status; //0 = good, 1 = compiler error, 2 = data error

	// Option for which format to use in printing numbers
	int num_format; // 0 = dec, 1 = bin, 2 = hex

	std::map<std::string, span_t> ram_alloc; // RAM Variable Allocation Map
	std::map<std::string, span_t> sub_alloc; // PMEM Subroutine Allocation Map
	std::map<std::string, size_t> jump_addr; // PMEM Jump Location Map
	// std::vector<MacSig> macs;

	void add(std::string data);
	void add(std::string data, size_t lnum);

	std::string str();

	// Low Level Settings
	int true_val;
	int false_val;
	int pmem;

	// ISV Characteristics
	std::string architecture;
	std::string isv_series;
	int isv_major;
	int isv_minor;
	int isv_patch;
	bool isv_exact_match;


private:
	static const int mask_compiler_error = 0b00000001;
	static const int mask_data_error     = 0b00000010;

};

/*
CompilerState Initializer
*/
CompilerState::CompilerState(InstructionSet is, GLogger& log) : log(log){
	CompilerState::is = is;
	CompilerState::log = log;

	next_line = 0;
	compile_status = 0;

	num_format = 0;

	// Low Level Settings
	true_val = 1;
	false_val = 0;
	pmem = pmem_flash0;

	// ISV Characteristics
	architecture = "unknown_arch";
	isv_series = "unknown_series";
	isv_major = -1;
	isv_minor = -1;
	isv_patch = -1;
	isv_exact_match = false;
}

/*
Adds a string of data and a line number to the BPIR vector. Keeps the vector
sorted by line number.

Automaticcally adds at 'next_line' and updates next_line.
*/
void CompilerState::add(std::string data){
	CompilerState::add(data, next_line++);
}

/*
Adds a string of data and a line number to the BPIR vector. Keeps the vector
sorted by line number.

Does not update next_line.
*/
void CompilerState::add(std::string data, size_t lnum){

	// Create new element to add
	abs_line line;
	line.str = data;
	line.idx = lnum;
	line.id = "";

	// Add to BPIR vector (at correct location)
	if ( bpir.size() ==  0 || lnum > bpir[bpir.size()-1].idx){ // If new line is at end of vector...
		bpir.push_back(line);
	}else{ // Else find where to place in vector
		size_t nidx = bpir.size()-2;
		while(bpir[nidx].idx > lnum) nidx--; // Reduce index until lnum > .idx
		if (bpir[nidx].idx == lnum){
			log.critical("Compiler overwrote address " + std::to_string(lnum) + " of output file!");
			compile_status |= mask_compiler_error; // Set compiler error
		}else{
			bpir.insert(bpir.begin()+nidx, line);
		}
	}
}

std::string CompilerState::str(){

	std::string s = "";

	for (size_t i = 0 ; i < bpir.size() ; i++){
		s = s + to_string(bpir[i].idx) + ":" + bpir[i].str;
		if (bpir[i].id.length() > 0){
			s = s + " (id: " + bpir[i].id + ")";
		}
		s = s + "\n";
	}

	return s;
}

//============================= STATEMENT BASE =================================

class Statement{
public:
	Statement(vector<qtoken>& tokens, size_t start_idx, size_t end_idx, StatementType t);

	StatementType type; // Type of statement
	std::vector<qtoken> src; // Tokens that were source for the statement

	bool exec(CompilerState& cs, GLogger& log);
	bool exec_machine_code(CompilerState& cs, GLogger& log);
	bool exec_directive(CompilerState& cs, GLogger& log);
};

Statement::Statement(vector<qtoken>& tokens, size_t start_idx, size_t end_idx, StatementType t){

	// Save type
	type = t;

	// Save source
	vector<qtoken> subvec(tokens.begin() + start_idx, tokens.begin() + end_idx);
	src = subvec;

}

bool Statement::exec(CompilerState& cs, GLogger& log){

	bool status = false;

	switch (type){
		case (st_machine_code):
			status = exec_machine_code(cs, log);
			break;
		case (st_directive):
			status = exec_directive(cs, log);
			break;
		default:
			log.error("Failed to evaluate statement.");
			status = false;
			break;
	}

	return status;
}

bool Statement::exec_directive(CompilerState& cs, GLogger& log){

	bool state = true;

	// Save instruction token
	qtoken directive = src[0];

	// Save data bit tokens
	vector<qtoken> data_tokens(src.begin() + 1, src.end());

	// Create list of compiler directives
	vector<string> comp_directives;
	comp_directives.push_back("#ARCH");
	comp_directives.push_back("#SERIES");
	comp_directives.push_back("#ISV");
	comp_directives.push_back("#ISV_EXACT");
	comp_directives.push_back("#PMEM");
	comp_directives.push_back("#TRUEVAL");
	comp_directives.push_back("#FALSEVAL");
	comp_directives.push_back("#INCLUDE");
	comp_directives.push_back("#SUBOPERATOR");
	comp_directives.push_back("#HERE");

	//=============== Initialization Above, Exec Below =========================

	// Error check directive
	if (!inVector(directive.str, comp_directives)){ // Check directive exists
		log.lerror("Unrecognized compiler directive '"+directive.str+"'.", directive.lnum);
		state = false;
		return state;
	}

	if (directive.str.compare("#ARCH") == 0){

		if (data_tokens.size() != 1){
			log.lerror("#ARCH compiler directive requires exactly 1 argument.", directive.lnum);
			state = false;
			return state;
		}
		if (data_tokens[0].type != id){
			log.lerror("#ARCH compiler directive requires an identifier token.", directive.lnum);
			state = false;
			return state;
		}

		// Save architecture type
		cs.architecture = data_tokens[0].str;

	}else if (directive.str.compare("#SERIES") == 0){

		if (data_tokens.size() != 1){
			log.lerror("#SERIES compiler directive requires exactly 1 argument.", directive.lnum);
			state = false;
			return state;
		}
		if (data_tokens[0].type != id){
			log.lerror("#SERIES compiler directive requires an identifier token.", directive.lnum);
			state = false;
			return state;
		}

		// Save ISV series
		cs.isv_series = data_tokens[0].str;

	}else if (directive.str.compare("#ISV") == 0){

		if (data_tokens.size() != 1){
			log.lerror("#ISV compiler directive requires exactly 1 argument.", directive.lnum);
			state = false;
			return state;
		}
		if (data_tokens[0].type != ver){
			log.lerror("#ISV compiler directive requires a version token.", directive.lnum);
			state = false;
			return state;
		}

		// Save version
		cs.isv_major   = data_tokens[0].valv[0];
		cs.isv_minor   = data_tokens[0].valv[1];
		cs.isv_patch = data_tokens[0].valv[2];
		cs.isv_exact_match = false;

	}else if (directive.str.compare("#ISV_EXACT") == 0){

		if (data_tokens.size() != 1){
			log.lerror("#ISV_EXACT compiler directive requires exactly 1 argument.", directive.lnum);
			state = false;
			return state;
		}
		if (data_tokens[0].type != ver){
			log.lerror("#ISV_EXACT compiler directive requires a version token.", directive.lnum);
			state = false;
			return state;
		}

		// Save version
		cs.isv_major   = data_tokens[0].valv[0];
		cs.isv_minor   = data_tokens[0].valv[1];
		cs.isv_patch = data_tokens[0].valv[2];
		cs.isv_exact_match = true;

	}else if (directive.str.compare("#PMEM") == 0){

		if (data_tokens.size() != 1){
			log.lerror("#PMEM compiler directive requires exactly 1 argument.", directive.lnum);
			state = false;
			return state;
		}
		if (data_tokens[0].type != lit){
			log.lerror("#PMEM compiler directive requires a literal token.", directive.lnum);
			state = false;
			return state;
		}

		// Save PMEM selection
		if (data_tokens[0].str.compare("FLASH0") == 0){
			cs.pmem = pmem_flash0;
		}else if (data_tokens[0].str.compare("FLASH1") == 0){
			cs.pmem = pmem_flash1;
		}else if (data_tokens[0].str.compare("RAM0") == 0){
			cs.pmem = pmem_ram;
		}

	}else if (directive.str.compare("#TRUEVAL") == 0){

		if (data_tokens.size() != 1){
			log.lerror("#TRUEVAL compiler directive requires exactly 1 argument.", directive.lnum);
			state = false;
			return state;
		}
		if (data_tokens[0].type != num){
			log.lerror("#TRUEVAL compiler directive requires a numeric token.", directive.lnum);
			state = false;
			return state;
		}

		// Save ISV series
		cs.true_val = data_tokens[0].vali;

	}else if (directive.str.compare("#FALSEVAL") == 0){

		if (data_tokens.size() != 1){
			log.lerror("#FALSEVAL compiler directive requires exactly 1 argument.", directive.lnum);
			state = false;
			return state;
		}
		if (data_tokens[0].type != num){
			log.lerror("#FALSEVAL compiler directive requires a numeric token.", directive.lnum);
			state = false;
			return state;
		}

		// Save ISV series
		cs.false_val = data_tokens[0].vali;
	}else if (directive.str.compare("#INCLUDE") == 0){

		//TODO:
		cout << "HOw to handle this?" << endl;

	}else if (directive.str.compare("#SUBOPERATOR") == 0){

		//TODO:
		cout << "HOw to handle this?" << endl;

	}else{
		log.lerror("Unrecognized compiler directive '" + directive.str + "'", directive.lnum);
		state = false;
		return state;
	}

	return state;
}

bool Statement::exec_machine_code(CompilerState& cs, GLogger& log){

	bool state = true;

	// Save instruction token
	qtoken instruction = src[0];

	size_t offset = 0;
	if (src[src.size()-1].type == sep && src[src.size()-1].str == ";"){
		offset = 1;
	}else{
		log.lerror("Missing ';'.", src[0].lnum);
		state = false;
	}

	// Save data bit tokens
	vector<qtoken> data_bytes(src.begin() + 1, src.end()-offset);

	//=============== Initialization Above, Exec Below =========================

	// First element of statement is machine code instruction
	cs.add(instruction.str);

	// Error check instruction
	if (cs.is.ops.count(instruction.str) < 1){ // Check instruction exists
		log.lerror("Failed to find instruction '"+instruction.str+"'.", instruction.lnum);
		state = false;
	}
	if (cs.is.ops[instruction.str].data_bits != data_bytes.size()){
		log.lerror("Instruction '"+instruction.str+"' given incorrect number of data bytes (" + to_string(data_bytes.size()) + " instead of " + to_string(cs.is.ops[instruction.str].data_bits) + ").", instruction.lnum);
		state = false;
	}

	// Next element of line is all data bits
	for (size_t i = 0 ; i < data_bytes.size() ; i++){
		cs.add(tokenvalstr(data_bytes[i], cs.num_format));
	}

	return state;
}


// class IfElseStatement{
//
// public:
//
// 	IfElseStatement();
//
//
//
//
// };

#endif
