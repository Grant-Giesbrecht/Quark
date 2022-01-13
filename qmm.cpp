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
#include "GLogger.hpp"
#include "gcolors.hpp"

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

	GLogger lgr;
	lgr.setLevel(LOGGER_MSG);

	//Get input file's name
	if (argc < 2){
		lgr.error("Requires .q-- file's name (no spaces allowed) as input. Use -h for help.", true);
		return -1;
	}
	string filename = argv[1];

	// Read output flags
	string flag;
	string outfile = "out.bpi";
	string outfile_r = "out.bpir";
	bool save_bpir = false;
	bool save_bpi = true;
	bool verbose = false;
	for (int argi = 1 ; argi < argc ; argi++){

		// Get flag
		flag = argv[argi];

		// Only help flag can appear without file name
		if (strcmp(flag.c_str(), "-h") == 0){
			cout << endl <<  "*************************************************************************************************************************" << endl;
			cout << "* Q-- Compiler                                                                                                          *" << endl;
			cout << "*                                                                                                                       *" << endl;
			cout << "* Usage                                                                                                                 *" << endl;


			cout << "*  qmm ./path/to/sourcefile.q--    " << gcolor::blue << "Saves output file to working directory as 'out.bpi'." << gcolor::normal << "                                 *" << endl;

			cout << "*  qmm ./path/to/sourcefile.q-- " << gcolor::red << "..." << gcolor::normal << " -o ./path/to/output_file.bpi    " << gcolor::blue << "Saves output as specified bpi file." << gcolor::normal << "                 *" << endl;

			cout << "*  qmm ./path/to/sourcefile.q-- " << gcolor::red << "..." << gcolor::normal << " -or ./path/to/bpir_output_file.bpir    " << gcolor::blue << "Saves output as bpi and specified bpir file." << gcolor::normal << " *" << endl;

			cout << "*  qmm ./path/to/sourcefile.q-- " << gcolor::red << "..." << gcolor::normal << " -v    " << gcolor::blue << "Use verbose output." << gcolor::normal << "                                                           *" << endl;

			cout << "*  qmm ./path/to/sourcefile.q-- " << gcolor::red << "..." << gcolor::normal << " -dummy    " << gcolor::blue << "Do not save result to file." << gcolor::normal << "                                               *" << endl;

			cout << "*                                                                                                                       *" << endl;
			cout << "* Written by Grant Giesbrecht                                                                                           *" << endl;
			cout << "*************************************************************************************************************************" << endl << endl;
			return 0;
		}

		// Do not check for remaining flags for argument 1 (arg 1 must be filename)
		if (argi < 2) continue;

		// Get value
		if (strcmp(flag.c_str(), "-o") == 0){
			if (argi+1 >= argc){
				lgr.warning("Incorrect number of args for flag: " + flag, true);
				break;
			}
			outfile = argv[argi+1];
			lgr.msg("Outfile: " + outfile, true);
			argi++;
		}else if (strcmp(flag.c_str(), "-or") == 0){
			if (argi+1 >= argc){
				lgr.warning("Incorrect number of args for flag: " + flag, true);
				break;
			}
			outfile_r = argv[argi+1];
			save_bpir = true;
			lgr.msg("BPIR Outfile: " + outfile_r, true);
			argi++;
		}else if (strcmp(flag.c_str(), "-v") == 0){
			verbose = true;
		}else if (strcmp(flag.c_str(), "-dummy") == 0){
			save_bpir = false;
			save_bpi = false;
		}else{
			lgr.warning("Unrecognized flag: " + flag, true);
		}
	}



	//---------------------- Read configuration file ---------------------------
	map<string, string> settings;
	if (!load_conf("/Users/grantgiesbrecht/Documents/GitHub/Quark/quark.conf", settings)){
		lgr.error("Failed to read configuration file", true);
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
		lgr.error("Failed to read CW File ("+cw_path+")", true);
		return -1;
	}

	// Read ISD File
	if (!is.load_isd(isd_path)){
		lgr.error("Failed to read ISD File ("+isd_path+")", true);
		return -1;
	}

    //-------------------- Read program into vector ----------------------------

    //read through file
	ifstream file(filename.c_str());
	if (!file.is_open()) {
		lgr.error("Failed to read program source ("+filename+")", true);
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
					lgr.error("Failed to convert "+words[i].str+" to a number.", true);
					return -1;
				}

			}

		}

		// Add to program
		program.push_back(nextqmm);
    }

	//--------------------- Print Program --------------------------------------

	if (verbose){
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
	}

	//------------------ Convert Program vector to BPIR (string vector) --------

	// List of addresses assigned by 'here'
	map<string, size_t> here_directives;

	// List of lines for BPIR file (Blinkenrechner Program Image - Readable)
	vector<string> bpir;

	// Scan over all program elements
	string nl;
	size_t lnum_bpir = 0;
	for (size_t pi = 0 ; pi < program.size() ; pi++){

		// Verify instruction is not blank
		if (program[pi].instruction == ""){
			lgr.warning("Case not handled for instruction-free lines (Line " +to_string(program[pi].lnum)+")", true);
		}else{

			// Add instruction to program
			nl = to_string(lnum_bpir) + ": " + program[pi].instruction;
			if (program[pi].comment.length() > 0){
				nl = nl + " " + program[pi].comment;
			}
			bpir.push_back(nl);
			lnum_bpir++;

			// Check number of expected data bytes
			int num_bytes;
			if (is.ops.count(program[pi].instruction)){
				num_bytes = is.ops[program[pi].instruction].data_bits;
			}else{
				lgr.error("Failed to find instruction '" + program[pi].instruction + "' (Line "+to_string(program[pi].lnum)+")", true);
				return -1;
			}
			// try{
			//
			// 	cout << program[pi].instruction << " " << is.ops[program[pi].instruction].data_bits << endl;
			// }catch(...){
			// 	lgr.error("Failed to find instruction '" + program[pi].instruction + "' (Line "+to_string(program[pi].lnum)+")", true);
			// 	return -1;
			// }

			if (strcmp(program[pi].instruction.c_str(), "RAM_REGB") == 0){
				cout << "\t" << num_bytes << " " << program[pi].data_bytes.size() << endl;
			}

			// Add data bytes to program
			if (num_bytes == 1){ // If one data byte... (8 bit)
				for (size_t k = 0 ; k < program[pi].data_bytes.size() ; k++){
					nl = to_string(lnum_bpir) + ": " + to_string(program[pi].data_bytes[k]);
					bpir.push_back(nl);
					lnum_bpir++;
				}
			}else if(num_bytes == 2){ // If two data bytes (16 bit)

				if (program[pi].data_bytes.size() >= 1){

					uint16_t addr = program[pi].data_bytes[0];

					uint16_t mask_byte0 = 255;
					uint16_t mask_byte1 = 65280;

					uint16_t byte0 = (addr & mask_byte0);
					uint16_t byte1 = ((addr & mask_byte1) >> 8);

					nl = to_string(lnum_bpir) + ": " + to_string(byte0);
					bpir.push_back(nl);
					lnum_bpir++;

					nl = to_string(lnum_bpir) + ": " + to_string(byte1);
					bpir.push_back(nl);
					lnum_bpir++;

				}else if(program[pi].dirs.size() < 1){
					lgr.error("Line " + to_string(program[pi].lnum) + " is missing a data byte", true);
					return -1;
				}


			}else if(num_bytes != 0){
				lgr.warning("Cannot handle " + to_string(num_bytes)+ "-byte data in current configuration (Line "+to_string(program[pi].lnum)+")", true);
				return -1;
			}

			// Process directives
			for (size_t k = 0 ; k < program[pi].dirs.size() ; k++){
				if (program[pi].dirs[k].directive.compare("#HERE") == 0){

					// Check if key exists
					if (here_directives.count(program[pi].dirs[k].argument)){
						lgr.warning("Key already exists. Overwriting", true);
					}

					// Save line number
					here_directives[program[pi].dirs[k].argument] = lnum_bpir-1;

				}else if(program[pi].dirs[k].directive.compare("#DEREF") == 0){
					// Check if key exists...
					if (here_directives.count(program[pi].dirs[k].argument)){

						// Verify number of data bytes
						if (is.ops[program[pi].instruction].data_bits != 2){
							lgr.error("Cannot dereference address for instrucion taking other than 2 data bytes", true);
							return -1;
						}

						uint16_t addr;
						try{
							addr = here_directives[program[pi].dirs[k].argument];
						}catch(...){
							lgr.error("Failed to dereference abstract address '" + program[pi].dirs[k].argument +"' (Line " + to_string(program[pi].lnum) + "). Perhaps it is defined on a later line?", true);
							return -1;
						}

						uint16_t mask_byte0 = 255;
						uint16_t mask_byte1 = 65280;

						uint16_t byte0 = (addr & mask_byte0);
						uint16_t byte1 = ((addr & mask_byte1) >> 8);

						nl = to_string(lnum_bpir) + ": " + to_string(byte0);
						bpir.push_back(nl);
						lnum_bpir++;

						nl = to_string(lnum_bpir) + ": " + to_string(byte1);
						bpir.push_back(nl);
						lnum_bpir++;
					}else{
						lgr.error("Failed to dereference key '" + program[pi].dirs[k].argument + "'", true);
						return -1;
					}
				}
			}
		}

	}

	//------------------------------- Print BPIR -------------------------------

	if (verbose){
		cout << "BPIR Contents:" << endl;
		for (size_t i = 0 ; i < bpir.size() ; i++){

			cout << bpir[i] << endl;

		}
	}

	//------------------------- Create BPI from BPIR ---------------------------

	vector<string> bpi;
	string msg;
	if (!processBPIR(bpir, is, bpi, msg)){
		lgr.error(msg, true);
		return -1;
	}

	//------------------------------- Print BPI --------------------------------

	if (verbose){
		cout << "BPI Contents:" << endl;
		for (size_t i = 0 ; i < bpi.size() ; i++){

			cout << bpi[i] << endl;

		}
	}

	//------------------------------- Save files -------------------------------

	if (save_bpi){
		ofstream bpi_out;
		bpi_out.open (outfile);
		for (size_t pl = 0 ; pl < bpi.size() ; pl++){
			bpi_out << bpi[pl] << "\n";
		}
		bpi_out.close();
	}

	if (save_bpir){
		ofstream bpir_out;
		bpir_out.open (outfile_r);
		for (size_t pl = 0 ; pl < bpir.size() ; pl++){
			bpir_out << bpir[pl] << "\n";
		}
		bpir_out.close();
	}


	return 0;
}
