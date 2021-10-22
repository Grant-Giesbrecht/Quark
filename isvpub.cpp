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
	InstructionSet is_old;

	// Read new files

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

	// Read most recent archive files
	string cw_fn, isd_fn;
	if (!read_arch_version(lastver, is_old, cw_fn, isd_fn)){
		return -1;
	}

	//--------------------- Check for changes between versions -----------------

	// isv_delta changelog;
	//
	// changelog = get_version_diff(is_new, is_old);

	string usrin;
	int newmaj, newmin, newpatch;
	string newseries = lastver.series;
	do{

		// Print menu instructions
		cout << "Last version: " << gcolor::bb << lastver.series << " " << to_string(lastver.major) << "." << to_string(lastver.minor) << "." << to_string(lastver.patch) << gcolor::normal << endl;
		cout << "Enter new version number: " << endl;

		// Ask for major
		cout << "\tMajor: " << std::flush;
		std::getline(std::cin, usrin);
		try{
			newmaj = stoi(usrin);
		}catch(...){
			cout << "INVALID INPUT: " << usrin << endl;
			cout << endl << endl;
			continue;
		}

		// ASk for minor
		cout << "\tFS: " << flush;
		std::getline(std::cin, usrin);
		try{
			newmin = stoi(usrin);
		}catch(...){
			cout << "INVALID INPUT: " << usrin << endl;
			cout << endl << endl;
			continue;
		}

		// ASk for patch
		cout << "\tPatch: " << flush;
		std::getline(std::cin, usrin);
		try{
			newpatch = stoi(usrin);
		}catch(...){
			cout << "INVALID INPUT: " << usrin << endl;
			cout << endl << endl;
			continue;
		}

		// Ask for confirmation
		cout << endl << "Will archive new version as: " << gcolor::bb << lastver.series << " " << to_string(newmaj) << "." << to_string(newmin) << "." << to_string(newpatch) << gcolor::normal << endl;
		cout << "Confirm (y/n): " << flush;
		std::getline(std::cin, usrin);
		usrin = to_upper(usrin);
		if (usrin == "Y" || usrin == "YES"){
			break;
		}

	}while(true);

	//--------------------- Make new directory + copy files --------------------

	// Calculate new directory name in archive
	fs::path prev_path(lastver.path);
	fs::path new_path(prev_path.parent_path().concat("/"+newseries + "_" + to_string(newmaj) + "_" + to_string(newmin) + "_" + to_string(newpatch)));

	// Get new file names
	fs::path new_cw_path(new_path);
	new_cw_path.concat("/"+save_name+".cw");
	fs::path new_isd_path(new_path);
	new_isd_path.concat("/"+save_name+".isd");
	fs::path new_lut_path(new_path);
	new_lut_path.concat("/"+save_name+".lut");

	// Make new directory
	fs::create_directory(new_path);

	// Copy files
	fs::copy(cw_path, new_cw_path);
	fs::copy(isd_path, new_isd_path);
	fs::copy(lut_path, new_lut_path);

	cout << "======================= New Archive Entry Created =======================" << endl;
	cout << "New version: " << gcolor::bb << lastver.series << " " << to_string(newmaj) << "." << to_string(newmin) << "." << to_string(newpatch) << gcolor::normal << endl;
	cout << "Archive Location: " << gcolor::bb << new_path << gcolor::normal << endl;
	cout << "\tCopied: " << cw_path << " to " << new_cw_path << endl;
	cout << "\tCopied: " << isd_path << " to " << new_isd_path << endl;
	cout << "\tCopied: " << lut_path << " to " << new_lut_path << endl;


	return 0;
}
