//CXCOMPILE make quark
//CXCOMPILE ./quark quark_test1.qrk -tok -state
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

void print_quark_help();

int main(int argc, char** argv){

	GLogger log;
	log.setLevel(LOGGER_MSG);

	//Get input file's name
	if (argc < 2){
		log.error("Requires .qrk file's name (no spaces allowed) as input.", true);
		return -1;
	}
	string filename = argv[1];

	// Check for help prompt
	if (strcmp(filename.c_str(), "-h") == 0 || strcmp(filename.c_str(), "-help") == 0){
		print_quark_help();
		return -1;
	}

	// Read output flags
	string flag;
	string outfile = "out.bpi";
	string outfile_r = "out.bpir";
	bool save_bpir = false;
	bool save_bpi = true;
	bool verbose = false;
	bool show_tokens = false;
	bool show_statements = false;

	for (int argi = 2 ; argi < argc ; argi++){

		// Get flag
		flag = argv[argi];

		// Get value
		if (strcmp(flag.c_str(), "-o") == 0){
			if (argi+1 >= argc){
				log.warning("Incorrect number of args for flag: " + flag, true);
				break;
			}
			outfile = argv[argi+1];
			log.msg("Outfile: " + outfile, true);
			argi++;
		}else if (strcmp(flag.c_str(), "-or") == 0){
			if (argi+1 >= argc){
				log.warning("Incorrect number of args for flag: " + flag, true);
				break;
			}
			outfile_r = argv[argi+1];
			save_bpir = true;
			log.msg("BPIR Outfile: " + outfile_r, true);
			argi++;
		}else if (strcmp(flag.c_str(), "-v") == 0){
			verbose = true;
		}else if (strcmp(flag.c_str(), "-dummy") == 0){
			save_bpir = false;
			save_bpi = false;
		}else if (strcmp(flag.c_str(), "-tok") == 0 || strcmp(flag.c_str(), "-token") == 0 || strcmp(flag.c_str(), "-t") == 0){
			show_tokens = true;
		}else if (strcmp(flag.c_str(), "-state") ==0 || strcmp(flag.c_str(), "-s") == 0){
			show_statements = true;
		}else{
			log.warning("Unrecognized flag: " + flag, true);
		}
	}



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
		log.error("Failed to read CW File ("+cw_path+")", true);
		return -1;
	}

	// Read ISD File
	if (!is.load_isd(isd_path)){
		log.error("Failed to read ISD File ("+isd_path+")", true);
		return -1;
	}

    //-------------------- Read program into vector ----------------------------

    //read through file
	ifstream file(filename.c_str());
	if (!file.is_open()) {
		log.error("Failed to read program source ("+filename+")", true);
		return -1;
	}

	std::string line;
	size_t line_num = 0;
	vector<string_idx> plain_text;
	while (getline(file, line)) {

		line_num++;

		trim_whitespace(line); //Remove whitespace from line

		// Add line to plain text vector
		string_idx nsi;
		nsi.str = line;
		nsi.idx = line_num;
		plain_text.push_back(nsi);

    }

	//----------------------- Create Language Spec -----------------------------

	language_specs lang;
	lang.op_data_to_ram = "PDATA_RAM0";


	//----------------------------- Run Lexer ----------------------------------

	vector<qtoken> token_list;
	if (!qlexer(plain_text, is, token_list, log)){
		cout << "Lexer Returned with Errors:" << endl;
		cout << log.all() << endl;
		return -1;
	}

	// Print token list
	if (show_tokens){
		cout << "Token List:" << endl;
		for (size_t t = 0 ; t < token_list.size() ; t++){
			cout << tokenstr(token_list[t]) << " ";
			if (token_list[t].type == TokenType::nl) cout << endl;
		}
	}

	// Create CompilerStatus object
	CompilerState cs(is, log, lang);
	cs.log = log;

	vector<Statement> tree;

	if (!qparser(token_list, is, cs, tree, log)){
		cout << "Parser Returned with Errors:" << endl;
		cout << log.all() << endl;
		return -1;
	}

	cout << "Lexer and Parser returned with no errors." << endl;
	cout << log.all() << endl;

	// Print statement list
	if (show_statements){
		cout << "Statement List:" << endl;
		for (size_t t = 0 ; t < tree.size() ; t++){
			cout << statementstr(tree[t], t) << endl;
			// if (token_list[t].type == TokenType::nl) cout << endl;
		}
	}

	cout << "Tree size: " << tree.size() << endl;

	// Run through tree and execute each statement
	for (size_t i = 0 ; i < tree.size() ; i++){
		tree[i].exec(cs, log);
	}

	cout << "\nBPIR: " << endl;
	cout << cs.str() << endl;

	return 0;
}

void print_quark_help(){
	cout << "Quark help" << endl;
}
