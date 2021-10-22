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
#include "LutTypes.hpp"

using namespace std;
using namespace gstd;
// namespace fs = std::filesystem;
// namespace gc = gstd::gcolor;

int main(int argc, char** argv){

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


	InstructionSet is;

	// Read new files

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

	// Read most recent archive files


	//--------------------- Check for changes between versions -----------------

	//--------------------- Make new directory + copy files --------------------

	// Make new directory

	// Copy files



	return 0;
}
