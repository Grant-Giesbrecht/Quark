//CXCOMPILE make alloc_test
//CXCOMPILE ./alloc_test
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
#include "GLogger.hpp"

#include "subatomic.hpp"
#include "quark_types.hpp"
#include "quarklexer.hpp"
#include "quarkparser.hpp"

using namespace std;
using namespace gstd;

int main(){

	GLogger log;
	log.setLevel(LOGGER_MSG);

	//---------------------- Read configuration file ---------------------------
	map<string, string> settings;
	if (!load_conf("quark.conf", settings)){
		log.error("Failed to read configuration file", true);
		return -1;
	}
	string isd_path = settings["isd_file_path"];
	string cw_path = settings["cw_file_path"];
	string lut_path = settings["lut_file_path"];
	string arch_path = settings["Archive_Dir"];
	string save_name = settings["save_name"];

	string series = settings["Series"];
	string architecture_name = settings["architecture_name"];

	//----------------- Determine last archived version of ISV -----------------

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
		log.error("Failed to read CW File ("+cw_path+")", true);
		return -1;
	}

	// Read ISD File
	if (!is.load_isd(isd_path)){
		log.error("Failed to read ISD File ("+isd_path+")", true);
		return -1;
	}

	language_specs lang;
	lang.op_data_to_ram = "PDATA_RAM0";

	CompilerState cs(is, log, lang);
	cs.show_ram_usage();
	cs.mark_ram_used(10, 4);
	cs.show_ram_usage();
	cs.mark_ram_used(20, 4);
	cs.show_ram_usage();
	cs.mark_ram_used(24, 2);
	cs.show_ram_usage();
	cs.mark_ram_used(18, 2);
	cs.show_ram_usage();
	cs.mark_ram_used(15, 1);
	cs.show_ram_usage();
	if (cs.mark_ram_used(20,2)){
		cout << "pass" << endl;
	}else{
		cout << "fail" << endl;
	}
	if (cs.mark_ram_used(25,2)){
		cout << "pass" << endl;
	}else{
		cout << "fail" << endl;
	}
	if (cs.mark_ram_used(26,2)){
		cout << "pass" << endl;
	}else{
		cout << "fail" << endl;
	}
	cs.show_ram_usage();


	cout << "20:21 " << cs.check_ram_avail(20, 2) << endl;
	cout << "11:13 " << cs.check_ram_avail(11, 2) << endl;
	cout << "28:29 " << cs.check_ram_avail(28, 2) << endl;


	return 0;
}
