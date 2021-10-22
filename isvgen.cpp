/*
Takes a .ISD and .CW and creates an instruction set version in a .LUT file.

TODO:
* Fix: if two instructions have the same instruction number, this will not be detected, no error will be given

--------------------------------------------------------------------------------
Created by G. Giesbrecht
14-8-2021
*/

//CXCOMPILE make isvgen

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
// #include "LutTypes.hpp"

int main(int argc, char** argv){

	InstructionSet is;

	// std::vector<control_line> controls;
	// map<string, operation> ops;
	// isv_data isv;
	// isd_internal isdi;

	// Get control wiring filename
	std::string cw_filename;
	if (argc < 3){
		cw_filename = "./Source Files/blinkenrechner.cw";
	}else{
		cw_filename = string(argv[2]);
	}

	// Read Control Wiring file
	cout << "Reading (CW): " << argv[2] << endl;
	if (!is.load_cw(cw_filename)){
		cout << "Exiting" << endl;
		return -1;
	}

	// Print control wiring
	is.print_cw();


	// Get ISD filename
	std::string isd_filename;
	if (argc < 2){
		isd_filename = "./Source Files/blinkenrechner.isd";
	}else{
		isd_filename = argv[1];
	}

	// Read ISD file
	cout << "Reading (ISD): " << isd_filename << endl;
	if (!is.load_isd(isd_filename)){
		cout << "Exiting" << endl;
		return -1;
	}

	// Print Instruction Set Description
	is.print_isd();

	// Print summary
	is.print_operation_summary();

	// Generate LUT
	is.generate_LUT(false);

	// Display lookup table
	is.print_lut();

	// Get Look-Up Table Filename
	string lut_filename = "./ISVs/blinkenrechner.lut";

	// Save LUT file
	if (is.save_lut(lut_filename)){
		is.log.msg("Successfully saved LUT data to file '" + lut_filename);
	}else{
		is.log.msg("ERROR: Failed to write file '" + lut_filename);
	}

	// Print messages
	cout << "\nTarget Architecture: " << is.isv.arch << endl;
	cout << "\nISV Series: " << is.isv.series << endl;

	cout << is.log.all() << endl;

	return 0;
}
