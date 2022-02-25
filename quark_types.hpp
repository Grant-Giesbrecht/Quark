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
#include "data_types.hpp"

static_assert( -1 == ~0, "not 2's complement");

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
	long int vali;
	double valf;
	bool valb;
	int valv[3]; //For versions (0-> major, 1->minor, 2-> patch)

	// Metadata
	size_t lnum; // Declaration line
}qtoken;

typedef struct{

	// Specifies the name of the instruction used to transfer an 8-bit data
	// value from PMEM to RAM. Use in varaible declarations. Assumes order: data
	// value, addr0, addr8
	std::string op_data_to_ram;

}language_specs;

typedef struct{
	std::string id;
	std::string type;
	size_t addr;
	size_t len;
}variable;

typedef struct{
	size_t start;
	size_t end;
}addr_range;

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
Checks if a given string is a valid version literal (eg. "1.2.3")
*/
bool isVersion(std::string str){

	// Try to find seperators
	size_t idx0 = str.find(".");
	size_t idx1 = str.find(".", idx0+1);

	// Return false if less than 2 punkt
	if (idx0 == std::string::npos || idx1 == std::string::npos) return false;

	// Get string
	std::string maj = str.substr(0, idx0);
	std::string min = str.substr(idx0+1, idx1-idx0-1);
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

/*
Class capturing the internal state of the compiler. Contains data such as variables
and subroutines, along with the machine code list (bpir), loggers, the instruction
set definition, language specs, and memory allocation details.
*/
class CompilerState{
public:
	CompilerState(InstructionSet is, GLogger& log, language_specs ls);

	InstructionSet is;
	GLogger& log;
	language_specs lang;

	// Output vector
	std::vector<abs_line> bpir;

	// size_t next_ram_addr; // Next unallocated line in RAM0 for writing program variables
	size_t next_line; // Next empty line in memory pmem for writing program data
	int compile_status; //0 = good, 1 = compiler error, 2 = data error

	// Option for which format to use in printing numbers
	int num_format; // 0 = dec, 1 = bin, 2 = hex

	// If true, does not initialize variables to zero, but leaves memory in its original state.
	bool skip_default_init;

	std::vector<variable> vars; // List of all variables


	std::map<std::string, span_t> ram_alloc; // RAM Variable Allocation Map
	std::map<std::string, span_t> sub_alloc; // PMEM Subroutine Allocation Map
	std::map<std::string, size_t> jump_addr; // PMEM Jump Location Map
	// std::vector<MacSig> macs;

	void add(std::string data);
	void add(std::string data, size_t lnum);
	size_t addToken(qtoken tok, size_t expected_bytes);

	bool hasVar(std::string name);
	size_t idxVar(std::string name);

	size_t next_ram(size_t num_bytes, std::string useage, std::string name);
	size_t request_ram(size_t req_addr, size_t num_bytes, std::string useage, std::string name);

	std::string str();

	//Memory Usage
	std::vector<addr_range> ram_usage;
	size_t ram_size_bytes; //Not max address, for 4-bit address bus, would be 16, not 15.

	void show_ram_usage();
	bool mark_ram_used(size_t addr, size_t len);
	bool check_ram_avail(size_t addr, size_t len);

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
Convert a qtoken to a string for BPIR
*/
std::string tokenvalstr(qtoken tok, CompilerState& cs){

	std::string str;

	int fmt = cs.num_format;

	switch(tok.type){
		case TokenType::num:
			(tok.use_float) ? str = to_string(tok.valf) : str = to_string(tok.vali);
			break;
		// case TokenType::id: // Most likely a variable
		// 	if (cs.hasVar(tok.str)){
		//
		// 		cs.vars[cs.idxVar(tok.str)].addr
		//
		// 	// }else if(cs.hasSub(tok.str)){
		//
		// 	}else{
		// 		str = "InvalidID"
		// 		cs.log.error("Failed to convert token of type 'ID' to value. (Token: "+tokenstr(tok)+")");
		// 	}
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

/*
CompilerState Initializer
*/
CompilerState::CompilerState(InstructionSet is, GLogger& log, language_specs ls) : log(log){
	CompilerState::is = is;
	CompilerState::log = log;
	CompilerState::lang = ls;

	// next_ram_addr = 0;
	next_line = 0;
	compile_status = 0;

	num_format = 0;

	skip_default_init = true;

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

	ram_size_bytes = 65536;
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
Adds a token to the BPIR vector. Similar to 'add()', except by accepting tokens,
it can accept variables (type=id) which will have 2-byte addresses and thus couldn't
be added in a single call to 'add()'.

expected_bytes represents the size of numeric types, the final number is expected to fit within that many
bytes. This is used for machine code instructions, so an address like '1' would
be represented as 2 bytes instead of 1. It can also be used to error check. It is
not used by 'id' tokens. Values of expected_bytes can be 1, 2, or 4.

Error checking done outside of function by verifying returned
value matches expected_bytes.

tok: Token of type 'num' or 'id'. Tokens of type 'num' must have 8-bit data
     representing a value to place on a single line of the BPIR vector. Tokens
	 of type 'id' must have a 16-bit address which will be saved over 2 lines.


Returns number of bytes added.
*/
size_t CompilerState::addToken(qtoken tok, size_t expected_bytes){

	if (tok.type == num){ // Save numeric literal
		if (expected_bytes == 2){
			uint32_t full_value = tok.vali; // Create base value
			uint32_t mask8 = 255; // Create mask
			add(to_string( full_value & mask8) ); // Add byte 0
			add(to_string( full_value & (mask8 << 8) )); // Add byte 1
			return 2;
		}else if(expected_bytes == 4){
			uint32_t full_value = tok.vali; // Create base value
			uint32_t mask8 = 255; // Create mask
			add(to_string( full_value & mask8) ); // Add byte 0
			add(to_string( full_value & (mask8 << 8) )); // Add byte 1
			add(to_string( full_value & (mask8 << 16) )); // Add byte 2
			add(to_string( full_value & (mask8 << 24) )); // Add byte 3
			return 4;
		}else{ // Size is 1
			add(tokenvalstr(tok, *this));
			return 1;
		}
	}else if(tok.type == id){ //Dereference variables

		uint32_t full_value = tok.vali; // Create base value
		uint32_t mask8 = 255; // Create mask
		add(to_string( full_value & mask8) ); // Add byte 0
		add(to_string( full_value & (mask8 << 8) )); // Add byte 1
		return 2;
	}else{
		log.error("Unrecognized Token type passed to 'addToken()' (Token: " + tokenstr(tok) + ")");
		return 0;
	}

}

/*
Adds a string of data and a line number to the BPIR vector. Keeps the vector
sorted by line number.

data: 8-bit value to add to BPIR
lnum: BPIR address at which to add data.

ex:
BPIR Start:
0:0xFF
1:0xA0
2:0x6D
10:0xFF

add(0x55, 7)

BPIR End:
0:0xFF
1:0xA0
2:0x6D
7:0x55
10:0xFF

Does not update next_line. (Call to add without lnum does increment lnum).
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

/*
Checks if a variable with the id 'name' is in the CompilerState object.
*/
bool CompilerState::hasVar(std::string name){

	if (idxVar(name) == std::string::npos) return false;

	return true;
}

/*
Returns the index of the variable with id 'name'. Return string::npos if not found.
*/
size_t CompilerState::idxVar(std::string name){

	for (size_t v = 0 ; v < vars.size() ; v++){
		if (vars[v].id == name) return v;
	}

	return std::string::npos;
}

/*
Prints a summary of used addresses in RAM (according to ram_usage)
*/
void CompilerState::show_ram_usage(){

	cout << "RAM Usage:" << endl;
	for (size_t i = 0 ; i < ram_usage.size() ; i++){
		cout << "\t[" << ram_usage[i].start << ":" << ram_usage[i].end << "]" << endl;
	}

}

/*
Marks 'len' addresses, starting at 'addr' as occupied. If any bytes overlap with
used addresses, it will return false and not mark as used. Else, returns true.

Possible placements:
[50:100] 110:114
[0:10] 11:14 [50:100] adjacent_e
[0:10] 45:49 [50:100] adjacent_s
[0:10] 20:24 [50:100] new_block
[0:10]  9:13 [50:100] overlap   <-- this will trigger an error
*/
bool CompilerState::mark_ram_used(size_t addr, size_t len){

	// Verify RAM is available
	if (!check_ram_avail(addr, len)){
		return false;
	}

	// Scan over all usage blocks
	for (size_t i = 0 ; i <= ram_usage.size() ; i++){
		// cout << "\t" << i << " " << ram_usage.size() << endl;
		// Check fencepost case
		if (i == ram_usage.size()){

			// cout << "\tfencepost" << endl;

			if (i != 0 && ram_usage[i-1].end+1 == addr){ // Check if adjacent to previous block
				ram_usage[i-1].end = addr+len-1; //Extend previous block
				return true;
			}else{ //Create new block
				// cout << "\tnew" << endl;
				addr_range np;
				np.start = addr;
				np.end = addr+len-1;
				ram_usage.push_back(np);
				return true;
			}

		}

		// if (i != 0 && ram_usage[i-1].end )
		// cout << addr << " " << ram_usage[i].start << endl;
		if (addr < ram_usage[i].start){ // Place new block before this block (or modify block start if adjacent)
			// cout << "\tsandwich" << endl;
			if (i != 0 && ram_usage[i].start == addr+len){ // Check if adjacent to next block
				ram_usage[i].start = addr; //Extend previous block
				return true;
			}else{ //Create new block
				// cout << "\t\tnew" << endl;
				addr_range np;
				np.start = addr;
				np.end = addr+len-1;
				ram_usage.insert(ram_usage.begin()+i, np);
				return true;
			}

		}


	}

	return false;
}

/*
Checks if 'len' bytes, starting at 'addr' are available. Returns true if all are
unused, returns false otherwise.
*/
bool CompilerState::check_ram_avail(size_t addr, size_t len){

	// Get list of all bytes to check
	vector<size_t> all_bytes;
	for(size_t b = 0 ; b < len ; b++){
		all_bytes.push_back(addr+b);
	}

	//Check all bytes
	for (size_t b = 0 ; b < all_bytes.size() ; b++){

		// AT all usage blocks
		for (size_t i = 0 ; i < ram_usage.size() ; i++){
			// Check for collision
			if (all_bytes[b] >= ram_usage[i].start && all_bytes[b] <= ram_usage[i].end) return false;
		}
	}

	return true;
}

/*
Attempts to allocated the requested number of bytes at the requested addresses. If
not available, returns an address from next_ram(). The first address of the allocaed
memory is returned. This can be compared to the req_addr by the calling code if
a status check is desired.
*/
size_t CompilerState::request_ram(size_t req_addr, size_t num_bytes, std::string useage, std::string name){

	// Could save name and usage later if interested in making a RAM usage map

	// Check if the requested address/size is available
	if (check_ram_avail(req_addr, num_bytes)){

		// Mark block as used
		if (!mark_ram_used(req_addr, num_bytes)){
			// cout << "ERROR: Expected available memory was unaccessible (addr:" << req_addr << ", len: " << num_bytes << ")" << endl;
			throw std::logic_error("ERROR: Expected available memory was unaccessible (addr:" + to_hexstring(req_addr) + ", len: " + to_string(num_bytes) + ")");
		}

		// Return block address
		return req_addr;
	}

	// Return next available address
	return next_ram(num_bytes, useage, name);

}

/*
Gets the next unallocated address in RAM, returns it, and marks it as used.

If no more memory available, returns string::npos;
*/
size_t CompilerState::next_ram(size_t num_bytes, std::string useage, std::string name){

	// Could save name and usage later if interested in making a RAM usage map

	// Next address that might be open (prior is known unavailable)
	size_t next_check = 0;

	// Scan over all usage blocks
	for (size_t i = 0 ; i < ram_usage.size() ; i++){

		//Check if next_check through start of occupied block is big enough
		if (ram_usage[i].start-next_check >= num_bytes){

			// Mark block as used
			if (!mark_ram_used(next_check, num_bytes)){
				// cout << "ERROR: Expected available memory was unaccessible (addr:" << req_addr << ", len: " << num_bytes << ")" << endl;
				throw std::logic_error("ERROR: Expected available memory was unaccessible (addr:" + to_hexstring(next_check) + ", len: " + to_string(num_bytes) + ")");
			}

			// Return block address
			return next_check;

		}else{ // Otherwise mark next_check as byte after this block
			next_check = ram_usage[i].end+1;
		}

	}

	//Fencepost case - check if remaining space is sufficient
	if (ram_size_bytes-next_check){

		// Mark block as used
		if (!mark_ram_used(next_check, num_bytes)){
			// cout << "ERROR: Expected available memory was unaccessible (addr:" << req_addr << ", len: " << num_bytes << ")" << endl;
			throw std::logic_error("ERROR: Expected available memory was unaccessible (addr:" + to_hexstring(next_check) + ", len: " + to_string(num_bytes) + ")");
		}

		// Return block address
		return next_check;

	}else{ // No more memory available!

		// Return nichts
		return std::string::npos;
	}


}

// Returns BPIR as a string
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

/*
Class representing one Quark statement. This is typically over one line, however
for some special types of statements such as if-statements or while-loops (ie.
statements with curly braces), statements can span multiple lines and contain
blocks with additional statements therein.
*/
class Statement{
public:
	Statement(vector<qtoken>& tokens, size_t start_idx, size_t end_idx, StatementType t);

	StatementType type; // Type of statement
	std::vector<qtoken> src; // Tokens that were source for the statement

	bool exec(CompilerState& cs, GLogger& log);
	bool exec_machine_code(CompilerState& cs, GLogger& log);
	bool exec_directive(CompilerState& cs, GLogger& log);
	bool exec_declaration(CompilerState& cs, GLogger& log);

	// Specifies if statement has been executed. Used to determine if 'src' or
	// 'data_string' should be used for string representation.
	bool was_executed;

	// Color constants for data_string formatting
	string dc; // Data string data color
	string lc; // Data string label color
	string gn; // Normal

	// String representation of 'src', broken into statement parameters such as
	// (for a declaration) type, name, value, address, etc.
	std::string data_string; //TODO: Populate this string in each 'exec' function.
};

/*
Initializer
*/
Statement::Statement(vector<qtoken>& tokens, size_t start_idx, size_t end_idx, StatementType t){

	// Save type
	type = t;

	// Save source
	vector<qtoken> subvec(tokens.begin() + start_idx, tokens.begin() + end_idx);
	src = subvec;

	was_executed = false;
	data_string = "<string not populated>";

	dc = gcolor::blue;
	lc = gcolor::yellow;
	gn = gcolor::normal;
}

/*
Executes the statement. This involves converting the Statement, which is composed
of qtokens + etc, into a series of one or more machine code instructions (added
to the BPIR in the CompilerState object), and perhaps modifying the CompilerState
through the addition of variables or subroutines.
*/
bool Statement::exec(CompilerState& cs, GLogger& log){

	bool status = false;

	switch (type){
		case (st_declaration):
			status = exec_declaration(cs, log);
			break;
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

/*
Declarations work by creating qtokens for each element of the declaration statement
based on token order. Once tokens for the type, name, data, etc are had, the
type of variable (eg. int, float, addr, etc) is determined in an if-statement.

From here, a new variable in the CompilerState object 'cs' is created (the variable
added is a struct of type 'varaible', and it's added to the CompilterState's  'vars'
vector, which is simply a vector of 'variable' structs.). 'variable' structs have
an id, type, address and name. Thus, in later lines of code, this variable's ID
can be replaced with the variable's address.

If the declaration statement initializes the variable with a value, the exact same
behavior outline for non-initialized varaibles occurs, however an instruction is
added to the output program (via the cs.add() function) which instructs a specific
data value (ie. the initial value) to be written to the address assigned to the
variable.

Variable addresses are assigned such that they are only in RAM0, and the address
is assigned as the 'variable' struct is created (ie. in this function, as the
declaration statement is processed). The assigned address comes from the 'cs.next_'
*/
bool Statement::exec_declaration(CompilerState& cs, GLogger& log){

	bool state = true;

	// Save instruction token
	qtoken var_type = src[0];

	// Save ID token
	if (src[1].type != id){
		log.lerror("Variable declaration requires an identifier token after variable type.", src[0].lnum);
		state = false;
		return state;
	}
	qtoken var_id = src[1];
	data_string = "("+lc+"type"+gn+":" +dc+ var_type.str +gn + ") ("+lc+"var-name"+gn+": " + dc + var_id.str +  gn +")";

	// Check for semicolon
	if (!(src[src.size()-1].type == sep && src[src.size()-1].str == ";")){
		log.lerror("Missing ';'.", src[0].lnum);
		state = false;
	}

	// Check if declaration initializes with value
	bool has_value = false;
	bool addr_requested = false;
	qtoken addr;
	qtoken var_val = src[3];
	if (src[2].type == op && src[2].str == "="){

		has_value = true;

		// Save value
		if (src[3].type != num && src[3].type != lit){
			log.lerror("Variable declaration requires a numeric token as the value.", src[3].lnum);
			state = false;
			return state;
		}


		// Check for location specifier

		if (src.size() > 5){

			// Check for '@' separator
			if (src[4].type != sep || src[4].str != "@"){
				log.lerror("Missing '@' in location-specific variable declaration.", src[0].lnum);
				state = false;
				return false;
			}

			// Check for reference ('&') operator
			if (src[5].type != op || src[5].str != "&"){
				log.lerror("Missing reference operator (&) in location-specific variable declaration.", src[0].lnum);
				state = false;
				return false;
			}

			// Check for variable name, ensure it refers to
			if (src[6].type != id){
				log.lerror("Missing identifier in location-specific variable declaration.", src[0].lnum);
				state = false;
				return false;
			}

			// Make sure variable's address name matches assigned variable's name
			if (src[6].str.compare(var_id.str) != 0){
				log.lerror("Variable of declared address (" + src[6].str + ") does not match name of variable declared in same statement (" + var_id.str + ").", src[0].lnum);
				state = false;
			}

			// Check for assignment (=) operator
			if (src[7].type != op || src[7].str != "="){
				log.lerror("Missing assignment operator (=) in location-specific variable declaration.", src[0].lnum);
				state = false;
				return false;
			}

			// Get address
			if (src[8].type != num){
				log.lerror("Location-specific variable declaration requires a numeric token after assignment operator", src[0].lnum);
				state = false;
				return false;
			}

			addr_requested = true;
			addr = src[8];

		}
	}

	//=============== Initialization Above, Exec Below =========================

	// Create data bytes based on variable type specified in declaration
	vector<int> var_bytes;
	string init_val;
	variable nv;
	if (var_type.str.compare("uint") == 0 || var_type.str.compare("uint8") == 0 || var_type.str.compare("type") == 0 || var_type.str.compare("type8") == 0){
		// This is for all types initialized with an 8-bit unsigned integer

		// Check literal type
		if (has_value && var_val.type != num){
			log.lerror("Variables of type '" + var_type.str + "' can only be initialized with numeric tokens." , src[0].lnum);
			state = false;
			return state;
		}

		// Check that numeric type is an integer
		if (has_value && var_val.use_float){
			log.lerror("Variables of type '" + var_type.str + "' can only be initialized with integer literals." , src[0].lnum);
			state = false;
			return state;
		}

		// Check maximum value
		int maxval = 255;
		int minval = 0;
		if (has_value && (var_val.vali > maxval || var_val.vali < minval)){
			log.lerror("Variable of type '" + var_type.str + "' initialized with value outside of range [" + to_string(minval) + ", " + to_string(maxval) + "]." , src[0].lnum);
			state = false;
			return state;
		}

		// Set value
		if (has_value){
			var_bytes.push_back(var_val.vali);
			init_val = to_string(var_val.vali);
		}else{
			var_bytes.push_back(0);
			init_val = "0";
		}

		// Create variable object
		nv.id = var_id.str;
		nv.type = var_type.str;
		nv.len = 1;
		if (addr_requested){
			// Get an address from the memory allocator
			nv.addr = cs.request_ram(addr.vali, nv.len, "var", nv.id);
			if (nv.addr != addr.vali){
				log.lwarning("Variable '"+nv.id+"' was denied requested access to address " + to_hexstring(addr.vali) + ", assigned instead " + to_hexstring(nv.addr) , src[0].lnum);
			}
		}else{
			nv.addr = cs.next_ram(nv.len, "var", nv.id);
		}
		cs.vars.push_back(nv);

		// Check for out of memory
		if (nv.addr == std::string::npos){
			log.lerror("Out of random-access memory on target machine", src[0].lnum, true);
			return false;
		}


	}else if (var_type.str.compare("float8") == 0){

		// TODO: How handle this
		cout << "Not implemented" << endl;

	}else if (var_type.str.compare("addr") == 0 || var_type.str.compare("addr16") == 0 || var_type.str.compare("type16") == 0 || var_type.str.compare("uint16") == 0){
		// This is for all types initialized from a 16 bit unsigned integer



		// Set value
		if (has_value){

			// Check literal type
			if (var_val.type != num){
				log.lerror("Variables of type '" + var_type.str + "' can only be initialized with numeric tokens." , src[0].lnum);
				state = false;
				return state;
			}

			// Check that numeric type is an integer
			if (var_val.use_float){
				log.lerror("Variables of type '" + var_type.str + "' can only be initialized with integer literals." , src[0].lnum);
				state = false;
				return state;
			}

			if (!uint16_bytes(var_val.vali, var_bytes)){
				log.lerror("Variable of type '" + var_type.str + "' initialized with value outside of range [0, 65535]." , src[0].lnum);
			}
			init_val = to_string(var_val.vali);
		}else{
			var_bytes.push_back(0);
			var_bytes.push_back(0);
			init_val = "0";
		}

		// Create variable object
		nv.id = var_id.str;
		nv.type = var_type.str;
		nv.len = 2;
		if (addr_requested){
			// Get an address from the memory allocator
			nv.addr = cs.request_ram(addr.vali, nv.len, "var", nv.id);
			if (nv.addr != addr.vali){
				log.lwarning("Variable '"+nv.id+"' was denied requested access to address " + to_hexstring(addr.vali) + ", assigned instead " + to_hexstring(nv.addr) , src[0].lnum);
			}
		}else{
			nv.addr = cs.next_ram(nv.len, "var", nv.id);
		}
		cs.vars.push_back(nv);

		// Check for out of memory
		if (nv.addr == std::string::npos){
			log.lerror("Out of random-access memory on target machine", src[0].lnum, true);
			return false;
		}

	}else if (var_type.str.compare("bool") == 0){

		// Check literal type
		if (has_value && var_val.type != lit ){
			log.lerror("Variables of type '" + var_type.str + "' can only be initialized with boolean literal tokens." , src[0].lnum);
			state = false;
			return state;
		}

		// Check value
		if (has_value && var_val.str.compare("true") != 0 && var_val.str.compare("false") != 0){
			log.lerror("Variable of type '" + var_type.str + "' initialized with invalid literal (" + var_val.str+"). Must be 'true' or 'false'." , src[0].lnum);
			state = false;
			return state;
		}

		// Set value
		if (has_value){
			if (var_val.str.compare("false")){
				var_bytes.push_back(cs.false_val);
				init_val = "false";
			}else{
				var_bytes.push_back(cs.true_val);
				init_val = "true";
			}
		}else{
			var_bytes.push_back(cs.false_val);
			init_val = "false";
		}

		// Create variable object
		nv.id = var_id.str;
		nv.type = var_type.str;
		nv.len = 1;
		if (addr_requested){
			// Get an address from the memory allocator
			nv.addr = cs.request_ram(addr.vali, nv.len, "var", nv.id);
			if (nv.addr != addr.vali){
				log.lwarning("Variable '"+nv.id+"' was denied requested access to address " + to_hexstring(addr.vali) + ", assigned instead " + to_hexstring(nv.addr) , src[0].lnum);
			}
		}else{
			nv.addr = cs.next_ram(nv.len, "var", nv.id);
		}
		cs.vars.push_back(nv);

		// Check for out of memory
		if (nv.addr == std::string::npos){
			log.lerror("Out of random-access memory on target machine", src[0].lnum, true);
			return false;
		}

	}else if (var_type.str.compare("int") == 0 || var_type.str.compare("int8") == 0){
		// This is for all types initialized from an 8 bit signed integer

		trouble("A");

		// Check literal type
		if (has_value && var_val.type != num){
			log.lerror("Variables of type '" + var_type.str + "' can only be initialized with numeric tokens." , src[0].lnum);
			state = false;
			return state;
		}

		// Check that numeric type is an integer
		if (has_value && var_val.use_float){
			log.lerror("Variables of type '" + var_type.str + "' can only be initialized with integer literals." , src[0].lnum);
			state = false;
			return state;
		}

		// Check maximum value
		int maxval = 127; // From (2^8)/2-1
		int minval = -128;
		if (has_value && (var_val.vali > maxval || var_val.vali < minval)){
			log.lerror("Variable of type '" + var_type.str + "' initialized with value outside of range [" + to_string(minval) + ", " + to_string(maxval) + "]." , src[0].lnum);
			state = false;
			return state;
		}

		trouble("B");

		// Set value
		if (has_value){
			if (!int8_bytes(var_val.vali, var_bytes)){
				log.lerror("Variable of type '" + var_type.str + "' initialized with value outside of range [-128, 127]." , src[0].lnum);
			}
			init_val = to_string(var_val.vali);
		}else{
			var_bytes.push_back(0);
			init_val = "0";
		}

		trouble("C");

		// Create variable object
		nv.id = var_id.str;
		nv.type = var_type.str;
		nv.len = 1;
		if (addr_requested){
			// Get an address from the memory allocator
			nv.addr = cs.request_ram(addr.vali, nv.len, "var", nv.id);
			if (nv.addr != addr.vali){
				log.lwarning("Variable '"+nv.id+"' was denied requested access to address " + to_hexstring(addr.vali) + ", assigned instead " + to_hexstring(nv.addr) , src[0].lnum);
			}
		}else{
			trouble("D");
			nv.addr = cs.next_ram(nv.len, "var", nv.id);
			trouble("E");
		}
		cs.vars.push_back(nv);

		// Check for out of memory
		if (nv.addr == std::string::npos){
			log.lerror("Out of random-access memory on target machine", src[0].lnum, true);
			return false;
		}

		trouble("F");

	}else if (var_type.str.compare("int32") == 0){
		// This is for all types initialized from a 16 bit signed integer

		// Set value
		if (has_value){

			// Check literal type
			if (var_val.type != num){
				log.lerror("Variables of type '" + var_type.str + "' can only be initialized with numeric tokens." , src[0].lnum);
				state = false;
				return state;
			}

			// Check that numeric type is an integer
			if (var_val.use_float){
				log.lerror("Variables of type '" + var_type.str + "' can only be initialized with integer literals." , src[0].lnum);
				state = false;
				return state;
			}

			if (!int32_bytes(var_val.vali, var_bytes)){
				log.lerror("Variable of type '" + var_type.str + "' initialized with value outside of range [0, 65535]." , src[0].lnum);
			}
			init_val = to_string(var_val.vali);
		}else{
			var_bytes.push_back(0);
			var_bytes.push_back(0);
			var_bytes.push_back(0);
			var_bytes.push_back(0);
			init_val = "0";
		}

		// Create variable object

		nv.id = var_id.str;
		nv.type = var_type.str;
		nv.len = 4;
		if (addr_requested){
			// Get an address from the memory allocator
			nv.addr = cs.request_ram(addr.vali, nv.len, "var", nv.id);
			if (nv.addr != addr.vali){
				log.lwarning("Variable '"+nv.id+"' was denied requested access to address " + to_hexstring(addr.vali) + ", assigned instead " + to_hexstring(nv.addr) , src[0].lnum);
			}
		}else{
			nv.addr = cs.next_ram(nv.len, "var", nv.id);
		}
		cs.vars.push_back(nv);

		// Check for out of memory
		if (nv.addr == std::string::npos){
			log.lerror("Out of random-access memory on target machine", src[0].lnum, true);
			return false;
		}

	}else if (var_type.str.compare("float") == 0 || var_type.str.compare("float32") == 0){

	// }else if (){
		cout << "\tNot immplemented!" << endl;
		init_val = "not-implemented";

	}else if (var_type.str.compare("type32") == 0){
		cout << "\tNot immplemented!" << endl;
		init_val = "not-implemented";
	}else{
		log.lerror("Unrecognized type '" + var_type.str + "'", var_type.lnum);
		state = false;
		return state;
	}

	// Check for initial value size mismatch
	if (var_bytes.size() != cs.vars[cs.vars.size()-1].len){
		log.lerror("Error in 'quark_types.hpp', Statement::exec_declaration. Size of 'var_bytes' ("+to_string(var_bytes.size())+") does not match variable size ("+to_string(cs.vars[cs.vars.size()-1].len)+"). The variable's initial value is incorrectly initialized.", var_type.lnum);
		state = false;
		return state;
	}

	// Mark statement as executed, print via data_string
	was_executed = true;

	// If no initial value provided...
	if (!has_value){
		// Skip initialization instruction
		if (cs.skip_default_init){
			data_string = data_string + " ("+lc+"init-val"+gn+": "+dc+"none"+gn+") ("+lc+"addr"+gn+": "+dc+to_string(nv.addr)+gn+")";
			return state;
		}
	}

	data_string = data_string + " ("+lc+"init-val"+gn+": "+dc+init_val+gn+") ("+lc+"addr"+gn+": "+dc+to_string(nv.addr)+gn+")";

	// For each byte in variable, tell program to allocate space in RAM
	for (size_t b = 0 ; b < var_bytes.size() ; b++){

		// Add write write instruction
		cs.add(cs.lang.op_data_to_ram);

		// Add data value
		cs.add(to_string(var_bytes[b]));

		// Add addr value, byte 0
		// cs.add();

		// Add addr value, byte 8
		// cs.add();
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
	vector<qtoken> data_tokens(src.begin() + 1, src.end()-offset);

	//=============== Initialization Above, Exec Below =========================

	// First element of statement is machine code instruction
	cs.add(instruction.str);

	// Error check instruction - Instruction exists
	if (cs.is.ops.count(instruction.str) < 1){ // Check instruction exists
		log.lerror("Failed to find instruction '"+instruction.str+"'.", instruction.lnum);
		state = false;
	}

	// Next element of line is all data bits
	size_t num_bits = 0;
	for (size_t i = 0 ; i < data_tokens.size() ; i++){
		// Add token and specify expected size (ie. so address < 256 would be represented as 2 bytes)
		num_bits += cs.addToken(data_tokens[i], cs.is.ops[instruction.str].data_bits);
	}

	// Error check instruction - Correct number of bits
	if (cs.is.ops[instruction.str].data_bits != num_bits){
		log.lerror("Instruction '"+instruction.str+"' given incorrect number of data bytes (" + to_string(num_bits) + " instead of " + to_string(cs.is.ops[instruction.str].data_bits) + ").", instruction.lnum);
		state = false;
	}

	// Mark statement as executed, print via data_string
	was_executed = true;
	data_string = "("+lc+"inst"+gn+": " + dc + instruction.str + gn + ")";
	for (size_t i = 0 ; i < data_tokens.size() ; i++){
		data_string = data_string + " ("+lc+"data"+gn+": " + tokenstr(data_tokens[i]) + ")";
	}

	return state;
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

		// Update data string
		was_executed = true;
		data_string = "("+lc+"directive"+gn+": "+dc+"#ARCH"+gn+") ("+lc+"architecture"+gn+": "+dc+data_tokens[0].str+gn+")";

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

		// Update data string
		was_executed = true;
		data_string = "("+lc+"directive"+gn+": "+dc+"#SERIES"+gn+") ("+lc+"series"+gn+": "+dc+data_tokens[0].str+gn+")";

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

		// Update data string
		was_executed = true;
		data_string = "("+lc+"directive"+gn+": "+dc+"#ISV"+gn+") ("+lc+"version"+gn+": "+tokenstr(data_tokens[0])+gn+")";

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

		// Update data string
		was_executed = true;
		data_string = "("+lc+"directive"+gn+": "+dc+"#ISV_EXACT"+gn+") ("+lc+"version"+gn+": "+tokenstr(data_tokens[0])+gn+")";

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

		// Update data string
		was_executed = true;
		data_string = "("+lc+"directive"+gn+": "+dc+"#PMEM"+gn+") ("+lc+"memory_location"+gn+": "+dc+data_tokens[0].str+gn+")";

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

		// Save value
		cs.true_val = data_tokens[0].vali;

		// Update data string
		was_executed = true;
		data_string = "("+lc+"directive"+gn+": "+dc+"#TRUEVAL"+gn+") ("+lc+"data"+gn+": "+dc+to_string(data_tokens[0].vali)+gn+")";

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

		// Save value
		cs.false_val = data_tokens[0].vali;

		// Update data string
		was_executed = true;
		data_string = "("+lc+"directive"+gn+": "+dc+"#FALSEVAL"+gn+") ("+lc+"data"+gn+": "+dc+to_string(data_tokens[0].vali)+gn+")";

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

/*
Create a string representing the Statement object.
*/
std::string statementstr(Statement& s, long int start_idx=-1, size_t fold_level=0){

	std::string str;

	// Mark statement type
	switch(s.type){
		case StatementType::st_declaration:
			str = "[st_declaration:";
			break;
		case StatementType::st_machine_code:
			str = "[st_machine_code:";
			break;
		case StatementType::st_reassignment:
			str = "[st_reassignment:";
			break;
		case StatementType::st_directive:
			str = "[st_directive:";
			break;
		case StatementType::st_if:
			str = "[st_if:";
			break;
		case StatementType::st_while:
			str = "[st_while:";
			break;
		case StatementType::st_subroutine:
			str = "[st_subroutine:";
			break;
		case StatementType::st_macro:
			str = "[st_macro:";
			break;
		case StatementType::st_expansion:
			str = "[st_expansion:";
			break;
		case StatementType::st_subroutine_call:
			str = "[st_subroutine_call:";
			break;
	}

	// Add counter
	if (start_idx != -1){

		str = std::to_string(fold_level) + "," + std::to_string(start_idx) + " " + str;

		// Add indentations
		for (size_t i = 0 ; i < fold_level ; i++){
			str = "  " + str;
		}

	}

	// Add statement data
	if (!s.was_executed){
		for (size_t t = 0 ; t < s.src.size() ; t++){
			str = str + " " + tokenstr(s.src[t]);
		}
		str = str + "]";
	}else{
		str = str + " " + s.data_string + "]";
	}

	return str;

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
