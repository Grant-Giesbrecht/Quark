#ifndef LUTTYPES_HPP
#define LUTTYPES_HPP

#include <string>
#include <vector>
#include <iostream>
#include <map>
#include "GLogger.hpp"

using namespace std;

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

#endif
