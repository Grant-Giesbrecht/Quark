/*
Takes a .ISD and .qrk and creates a binary file

Quark-- Compiler

--------------------------------------------------------------------------------
Created by G. Giesbrecht
7-11-2021
*/

//CXCOMPILE make qmm
//CXCOMPILE ./qmm ../Quark-Programs/BrPgm-3/BrPgm3.q--
//CXGENRUN FALSE

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

using namespace std;
using namespace gstd;

// Compiler directive
typedef struct{
    std::string directive;
    std::string argument;
}compdir;

// QMM Line
typedef struct{
    std::string instruction; //Instruction
    std::vector<int> data_bytes; //Instruction data bytes
    std::vector<compdir> dirs; //Compiler Directives
    std::string comment; // Comment
    size_t lnum; //Line number
}qmm_line;

// namespace fs = std::filesystem;
// namespace gc = gstd::gcolor;

int main(int argc, char** argv){

	//Get input file's name
	if (argc < 2){
		cout << ("Requires .q-- file's name (no spaces allowed) as input.") << endl;
		return -1;
	}
	string filename = argv[1];

	bool verbose = false;

	//---------------------- Read configuration file ---------------------------
	map<string, string> settings;
	if (!load_conf("quark.conf", settings)){
		return -1;
	}
	string isd_path = settings["isd_file_path"];
	string cw_path = settings["cw_file_path"];
	string lut_path = settings["lut_file_path"];
	string arch_path = settings["Archive_Dir"];
	string save_name = settings["save_name"];

	string series = settings["Series"];
	string architecture_name = settings["architecture_name"];

	// Show configuration if requested
	if (verbose){
		show_conf(settings);
	}

	//----------------- Determine last archived version of ISV -----------------

	// Get directories
    if (verbose){
		show_dir_contents(arch_path);
	}

	// Filter based on series name
	vector<isv_data> sarc;
	vector<isv_data> arc;
	arc = get_arch_titles(arch_path, architecture_name);
	for (size_t i = 0 ; i < arc.size() ; i++){
		if (arc[i].series == series){
			sarc.push_back(arc[i]);
		}
	}

	// Get newest version
	isv_data lastver;
	lastver = newest_version(sarc);

	cout << "NEWEST VERSION FOUND: " << endl;
	cout << "\tSeries:       " << gc::bb << lastver.series << gc::normal << endl;
	cout << "\tMajor:        " << gc::bb << lastver.major << gc::normal << endl;
	cout << "\tFeature Set:  " << gc::bb << lastver.minor << gc::normal << endl;
	cout << "\tPatch:        " << gc::bb << lastver.patch << gc::normal << endl;
	cout << "\tArchitecture: " << gc::bb << lastver.arch << gc::normal << endl;

	//-------------------- Load current and last ISV version -------------------

	// Create InstructionSet objects
	InstructionSet is_new;

	// Read ISV files

	// Read CW File
	if (!is_new.load_cw(cw_path)){
		cout << "Exiting" << endl;
		return -1;
	}

	// Read ISD File
	if (!is_new.load_isd(isd_path)){
		cout << "Exiting" << endl;
		return -1;
	}

    //-------------------- Read program into vector ----------------------------

    //read through file
	ifstream file(filename.c_str());
	if (!file.is_open()) {
		cout << "Failed to open file '" << filename << "'"  << endl;
		return -1;
	}

	std::string line;
	size_t line_num = 0;
	vector<string_idx> words;
	vector<qmm_line> program;
	while (getline(file, line)) {

		line_num++;
		qmm_line nextqmm;
		nextqmm.lnum = line_num;

		trim_whitespace(line); //Remove whitespace from line

		// Find comment_spec occurance (find first occurance)
		size_t idx = line.find("//");
		if (idx != std::string::npos){

			nextqmm.comment = line.substr(idx);
			line = line.substr(0, idx);
		}

		if (line.length() == 0) continue; //Continue if blank

		//Parse words
		words = gstd::parseIdx(line, " \t");

		//Ensure words exist
		if (words.size() < 1){
			continue;
		}

		// Read compiler directives
		// signed int dirs_end = words.size()-1;
		// for (signed int i = words.size()-1 ; i >= 0 ; i--){
		//
		// 	cout << i << endl;
		//
		// 	//Skip blank words
		// 	if (words[i].str.length() < 1) continue;
		//
		// 	cout << "\t"<< i << endl;
		//
		// 	// Check for directive
		// 	if (words[i].str[0] == '#'){
		// 		compdir cd;
		// 		cd.directive = words[i].str;
		// 		if (i+1 < words.size()){
		// 			cd.argument = words[i+1].str;
		// 		}
		// 		nextqmm.dirs.push_back(cd);
		// 		dirs_end = i-1;
		// 	}
		// 	cout << "\t\t"<< i << endl;
		// }

		int dirs_end = words.size()-1;
		for (size_t j = 0 ; j < words.size() ; j++){

			//Skip blank words
			if (words[j].str.length() < 1) continue;

			// Check for directive
			if (words[j].str[0] == '#'){
				compdir cd;
				cd.directive = words[j].str;
				if (j+1 < words.size()){
					cd.argument = words[j+1].str;
				}
				nextqmm.dirs.push_back(cd);
				dirs_end = j-1;
				j++;
			}

		}

		// Read instruction
		if (dirs_end >= 0){

			nextqmm.instruction = words[0].str;
			for (size_t i = 1; i <= dirs_end && i < words.size() ; i++){

				try{
					nextqmm.data_bytes.push_back(gstd::fstoi(words[i].str));
				}catch(...){
					cout << "Failed to convert " << words[i].str << " to a number" << endl;
				}

			}

		}

		// Add to program
		program.push_back(nextqmm);
    }

	//--------------------- Print Program --------------------------------------

	for (size_t i = 0 ; i < program.size() ; i++){

		// Print instruction and line number
		cout << "[" << program[i].lnum << "] " << program[i].instruction << endl;

		// Print data bytes
		if (program[i].data_bytes.size() > 0) cout << "\t [";
		for (size_t k = 0 ; k < program[i].data_bytes.size() ; k++){
			if (k != 0){
				cout << ", ";
			}
			cout << to_string(program[i].data_bytes[k]);
		}
		if (program[i].data_bytes.size() > 0) cout << "]" << endl;

		// Print directives
		for (size_t k = 0 ; k < program[i].dirs.size() ; k++){
			cout << "\tD" << k << ": " << program[i].dirs[k].directive << ", " << program[i].dirs[k].argument << endl;
		}

		if (program[i].comment.length() > 0){
			cout << "\tC: " << program[i].comment << endl;
		}
	}

	return 0;
}
