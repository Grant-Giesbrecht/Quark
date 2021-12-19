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
enum TokenType {key, id, num, op, cd, sop, typ, cmt, sep, lit, ins, nl}; // Token categories

typedef struct{
	// Type of token
	TokenType type; // Member of Cats

	bool use_float;

	// Token value
	std::string str;
	int vali;
	double valf;
	bool valb;
	int valv[3]; //For versions

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
			(tok.use_float) ? str = "[num: \"" + gcolor::magenta + to_string(tok.valf) +gcolor::normal + "\"]" : str = "[num: \"" + gcolor::magenta + to_string(tok.vali) + gcolor::normal + "\"]";
			break;
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
		case TokenType::nl:
			str = "Newline";
			break;
		default:
			str = tok.str;
			break;
	}

	return str;
}

//============================= COMPILER STATE =================================

class CompilerState{
public:
	CompilerState(InstructionSet is, GLogger& log);

	InstructionSet is;
	GLogger& log;

	// Output vector
	std::vector<abs_line> bpir;

	// Next empty line in memory
	size_t next_line;
	int compile_status = 0; //0 = good, 1 = compiler error, 2 = data error

	// Option for which format to use in printing numbers
	int num_format; // 0 = dec, 1 = bin, 2 = hex

	std::map<std::string, span_t> ram_alloc; // RAM Variable Allocation Map
	std::map<std::string, span_t> sub_alloc; // PMEM Subroutine Allocation Map
	std::map<std::string, size_t> jump_addr; // PMEM Jump Location Map
	// std::vector<MacSig> macs;

	void add(std::string data);
	void add(std::string data, size_t lnum);

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
	num_format = 0;
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
	if (lnum > bpir[bpir.size()-1].idx){ // If new line is at end of vector...
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

//============================== MACHINE CODE STATEMENT ========================

class MachineCodeStatement{

public:

	MachineCodeStatement();

	qtoken instruction;
	std::vector<qtoken> data_bytes;

	bool exec(CompilerState& cs, GLogger& log);

};

MachineCodeStatement::MachineCodeStatement(){
	// Do Nothing
}

bool MachineCodeStatement::exec(CompilerState& cs, GLogger& log){

	bool state = true;

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
