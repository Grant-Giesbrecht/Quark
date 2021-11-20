/*
Takes a .ISD and .qrk and creates a binary file

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
// namespace fs = std::filesystem;
// namespace gc = gstd::gcolor;

int main(int argc, char** argv){

	//Initialize default compiler parameters
	CompilerParams params;

	//Get input file's name
	if (argc < 2){

		params.error("Requires .qrk file's name (no spaces allowed) as input.");
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
	InstructionSet is;

	// Read ISV files

	// Read CW File
	if (!is.load_cw(cw_path)){
		cout << "Exiting" << endl;
		return -1;
	}

	// Read ISD File
	if (!is.load_isd(isd_path)){
		cout << "Exiting" << endl;
		return -1;
	}

	//-------------------- MCB 



	return 0;
}
