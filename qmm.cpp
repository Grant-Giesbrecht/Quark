/*
Takes a .ISD and .qrk and creates a binary file

Quark-- Compiler

--------------------------------------------------------------------------------
Created by G. Giesbrecht
7-11-2021
*/

//CXCOMPILE make isvpub

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

	//Initialize default compiler parameters
	CompilerParams params;

	//Get input file's name
	if (argc < 2){
		params.error("Requires .q-- file's name (no spaces allowed) as input.");
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
		cout << "Failed to open file" << endl;
		return -1;
	}

	std::string line;

	while (getline(file, line)) {

		line_num++;
		qmm_line nextqmm;
		qmm_line.lum = line_num;

		trim_whitespace(line); //Remove whitespace from line

		// Find comment_spec occurance (find first occurance)
		size_t idx = line.find("//");
		if (idx != std::string::npos){
			line = line.substr(0, idx);
			qmm_line.comment = line.substr(idx);
		}

		if (line.length() == 0) continue; //Continue if blank

		//Parse words
		words = gstd::parseIdx(line, " \t");

		//Ensure words exist
		if (words.size() < 1){
			continue;
		}

		// Read compiler directives
		signed int dirs_end = words.size()-1;
		for (signed int i = words.size() ; i >= 0 ; i--){

			//Skip blank words
			if (words[i].str.length() < 1) continue;

			// Check for directive
			if (words[i].str[0] == '#'){
				compdir cd;
				cd.directive = words[i].str;
				cd.argument = words[i+1].str;
				qmm_line.dirs.push_back(cd);
				dirs_end = i-1;
			}
		}

		// Read instruction
		if (dirs_end >= 0){
			qmm_line.instruction = words[0].str;
			for (size_t i = 1; i <= dirs_end ; i++){
				qmm_line.data_bytes.push_back(gstd::fstoi(words[i].str));
			}

		}
    }


	return 0;
}
