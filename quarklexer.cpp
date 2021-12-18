//CXCOMPILE make qlexer
//CXCOMPILE ./qlexer quark_test1.qrk
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

using namespace std;
using namespace gstd;

/*
Accepts a plain text input, performs lexical analysis and returns a list of
tokens.
*/
bool qlexer(vector<string_idx> txt, InstructionSet is, vector<qtoken>& tokens, GLogger& log ){

	// Reset token list to be clean
	tokens.clear();

	bool lex_status = true;

	//********************* Generate Lists of Keywords *************************

	// Create list of (standard) keywords
	vector<std::string> keyword_list;
	keyword_list.push_back("subroutine");
	keyword_list.push_back("macro");
	keyword_list.push_back("expand");
	keyword_list.push_back("if");
	keyword_list.push_back("else");
	keyword_list.push_back("while");

	// Create list of literal-keywords
	vector<string> lit_keyword_list;
	lit_keyword_list.push_back("FLASH0");
	lit_keyword_list.push_back("FLASH1");
	lit_keyword_list.push_back("RAM0");
	lit_keyword_list.push_back("zero");
	lit_keyword_list.push_back("nzero");
	lit_keyword_list.push_back("carry");
	lit_keyword_list.push_back("ncarry");
	lit_keyword_list.push_back("true");
	lit_keyword_list.push_back("false");

	// Create list of type-keywords
	vector<string> type_keyword_list;
	type_keyword_list.push_back("int");
	type_keyword_list.push_back("int8");
	type_keyword_list.push_back("float");
	type_keyword_list.push_back("float8");
	type_keyword_list.push_back("addr");
	type_keyword_list.push_back("addr16");
	type_keyword_list.push_back("int32");
	type_keyword_list.push_back("float32");
	type_keyword_list.push_back("type8");
	type_keyword_list.push_back("type16");
	type_keyword_list.push_back("type32");

	// Create list of compiler directives
	vector<string> comp_directives;
	comp_directives.push_back("#ARCH");
	comp_directives.push_back("#SERIES");
	comp_directives.push_back("#ISV");
	comp_directives.push_back("#ISV_EXACT");
	comp_directives.push_back("#PMEM");
	comp_directives.push_back("#TRUEVAL");
	comp_directives.push_back("#FALSEVAL");
	comp_directives.push_back("#INCLUDE");
	comp_directives.push_back("#SUBOPERATOR");
	comp_directives.push_back("#HERE");

	// Create list of (standard) operators
	vector<string> op_list;
	op_list.push_back(":");
	op_list.push_back("=");
	op_list.push_back("&");

	// Create list of subroutine Operators
	vector<string> sub_op_list;
	sub_op_list.push_back("+");
	sub_op_list.push_back("-");
	sub_op_list.push_back("*");
	sub_op_list.push_back("/");
	sub_op_list.push_back("^");
	sub_op_list.push_back("%%");
	sub_op_list.push_back("|");
	sub_op_list.push_back("&");

	// Create list of separators
	vector<string> sep_list;
	sep_list.push_back(";");
	sep_list.push_back("(");
	sep_list.push_back(")");
	sep_list.push_back("{");
	sep_list.push_back("}");
	sep_list.push_back("@");

	//*********************

	qtoken cmt_tok;
	string line;
	vector<string_idx> words;
	string word;

	qtoken newline_tok;
	newline_tok.type = nl;

	// Scan over entire file
	for (size_t i = 0 ; i < txt.size() ; i++){

		// Get line from file
		line = txt[i].str;

		// Remove comment from line
		size_t cmt_idx = line.find("//"); // Find comment_spec occurance (find first occurance)
		if (cmt_idx != std::string::npos){ // Remove everything after comment
			cmt_tok.type = TokenType::key;
			cmt_tok.str = line.substr(cmt_idx);
			cmt_tok.lnum = txt[i].idx;
			line = line.substr(0, cmt_idx); // Trim line
		}

		//Put whitespace around symbols
		string all_syms = "";
		for (size_t i = 0 ; i < sep_list.size() ; i++) all_syms = all_syms + sep_list[i];
		for (size_t i = 0 ; i < sub_op_list.size() ; i++) all_syms = all_syms + sub_op_list[i];
		for (size_t i = 0 ; i < op_list.size() ; i++) all_syms = all_syms + op_list[i];
		ensure_whitespace(line, all_syms);

		// Parse line at whitespace
		words = parseIdx(line, " \t");

		// Scan over all words
		for (size_t w = 0 ; w < words.size() ; w++){

			// define word
			word = words[w].str;

			// Check if word is member of standard keyword list
			if (inVector(word, keyword_list)){
				qtoken nt;
				nt.type = TokenType::key;
				nt.str = word;
				nt.lnum = txt[i].idx;
				tokens.push_back(nt);
			}else if (inVector(word, lit_keyword_list)){ // Check if word is member of literals keyword
				qtoken nt;
				nt.type = TokenType::lit;
				nt.str = word;
				nt.lnum = txt[i].idx;
				tokens.push_back(nt);
			}else if (inVector(word, type_keyword_list)){ // Check if word is member of literals keyword
				qtoken nt;
				nt.type = TokenType::typ;
				nt.str = word;
				nt.lnum = txt[i].idx;
				tokens.push_back(nt);
			}else if (inVector(word, comp_directives)){ // Check if word is member of literals keyword
				qtoken nt;
				nt.type = TokenType::cd;
				nt.str = word;
				nt.lnum = txt[i].idx;
				tokens.push_back(nt);
			}else if (inVector(word, op_list)){ // Check if word is member of literals keyword
				qtoken nt;
				nt.type = TokenType::op;
				nt.str = word;
				nt.lnum = txt[i].idx;
				tokens.push_back(nt);
			}else if (inVector(word, sub_op_list)){ // Check if word is member of literals keyword
				qtoken nt;
				nt.type = TokenType::sop;
				nt.str = word;
				nt.lnum = txt[i].idx;
				tokens.push_back(nt);
			}else if (inVector(word, sep_list)){ // Check if word is member of literals keyword
				qtoken nt;
				nt.type = TokenType::sep;
				nt.str = word;
				nt.lnum = txt[i].idx;
				tokens.push_back(nt);
			}else if(isNumeric(word)){ // Check if word is numeric literal
				qtoken nt;
				nt.type = TokenType::num;
				size_t idx_e = word.find("e");
				size_t idx_E = word.find("E");
				size_t idx_pkt = word.find(".");
				try{
					if (idx_e == std::string::npos && idx_E == std::string::npos && idx_pkt == std::string::npos){
						nt.use_float = false;
						nt.vali = fstoi(word);
					}else{
						nt.use_float = true;
						nt.valf = stod(word);
					}
				}catch(...){
					log.lerror("Failed to convert numeric literal '" + word +"'.", txt[i].idx);
					lex_status = false;
				}

				nt.str = word;
				nt.lnum = txt[i].idx;
				tokens.push_back(nt);
			}else if(isInstruction(word, is)){
				qtoken nt;
				nt.type = TokenType::sep;
				nt.str = word;
				nt.lnum = txt[i].idx;
				tokens.push_back(nt);
			}else if(isIdentifier(word)){
				qtoken nt;
				nt.type = TokenType::id;
				nt.str = word;
				nt.lnum = txt[i].idx;
				tokens.push_back(nt);
			}else{
				log.lerror("Failed to identify token '" + word + "'.", txt[i].idx);
				lex_status = false;
			}
		}

		// Save comment token if found
		if (cmt_idx != std::string::npos){ // Remove everything after comment
			tokens.push_back(cmt_tok);
		}

		// Add newline token
		newline_tok.lnum = txt[i].idx;
		tokens.push_back(newline_tok);
	}


	return lex_status;
}


int main(int argc, char** argv){

	bool show_tokens = true;

	GLogger lgr;
	lgr.setLevel(LOGGER_MSG);

	//Get input file's name
	if (argc < 2){
		lgr.error("Requires .qrk file's name (no spaces allowed) as input.", true);
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
	for (int argi = 2 ; argi < argc ; argi++){

		// Get flag
		flag = argv[argi];

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
	if (!load_conf("quark.conf", settings)){
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

	//----------------------------- Run Lexer ----------------------------------

	vector<qtoken> token_list;
	if (!qlexer(plain_text, is, token_list, lgr)){
		cout << lgr.all() << endl;
		return -1;
	}

	// Print token list
	if (show_tokens){
		for (size_t t = 0 ; t < token_list.size() ; t++){
			cout << tokenstr(token_list[t]) << " ";
			if (token_list[t].type == TokenType::nl) cout << endl;
		}
	}

	return 0;
}
