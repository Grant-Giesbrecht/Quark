#ifndef QUARKPARSER_HPP
#define QUARKPARSER_HPP

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
bool qparser(vector<qtoken> tokens, InstructionSet is, CompilerState& cs, GLogger& log ){

	bool parse_status = true;

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

	// Scan over token list, convert to statement classes. Almost all statement
	// types can be infered from looking at the first token, however more will
	// be scanned when required.
	size_t k, l, m;
	size_t i = 0;
	while (i < tokens.size()){

		if (tokens[i].type == typ && i < tokens.size()-2 && tokens[i+2].type == op && tokens[i+2].str == "="){ // Check if is a Declaration Statement
			log.info("Declaration Statement, line: " + to_string(tokens[i].lnum));

			// Find end
			k = i+2;
			while (k < tokens.size()){
				if (tokens[k].type == nl) break;
				k++;
			}


			i = k+1;
		}else if(tokens[i].type == ins){ // Check if is a Machine Code Statement
			log.info("Machine Code Statement, line: " + to_string(tokens[i].lnum));

			// Find end
			k = i+1;
			while (k < tokens.size()){
				if (tokens[k].type == nl) break;
				k++;
			}

			i = k+1;
		}else if (tokens[i].type == id && i < tokens.size()-1 && tokens[i+1].type == op && tokens[i+1].str == "="){ // Check if is a Reassignment Statement
			log.info(" Statement, line: " + to_string(tokens[i].lnum));

			// Find end
			k = i+1;
			while (k < tokens.size()){
				if (tokens[k].type == nl) break;
				k++;
			}

			i = k+1;
		}else if (tokens[i].type == cd){ // Directive Statement
			log.info("Compiler Directive Statement, line: " + to_string(tokens[i].lnum));

			// Find end
			k = i+1;
			while (k < tokens.size()){
				if (tokens[k].type == nl) break;
				k++;
			}

			i = k+1;
		}else if (tokens[i].type == key && tokens[i].str == "if"){ // If Statement
			// Find end of first closing bracket
			k = i+1;
			while (k < tokens.size()){
				if (tokens[k].type == sep && tokens[k].str == "}") break;
				k++;
			}

			// Check if statement is If-Else
			if (k < tokens.size()-1 && tokens[k+1].type == key && tokens[k+1].str == "else"){
				log.info("If-Else Statement, line: " + to_string(tokens[i].lnum));

				// Find next open bracket
				l = k+2;
				while (l < tokens.size()){
					if (tokens[l].type == sep && tokens[l].str == "{") break;
					l++;
				}
				m = l+1;
				while (m < tokens.size()){
					if (tokens[m].type == sep && tokens[m].str == "{") break;
					m++;
				}



				i = m+1;
			}else{
				log.info("If Statement, line: " + to_string(tokens[i].lnum));


				i = k+1;
			}
			cout << k << " " << l << " " << m << endl;
		}else if (tokens[i].type == key && tokens[i].str == "while"){
			log.info("While Statement, line: " + to_string(tokens[i].lnum));

			// Find end of first closing bracket
			k = i+1;
			while (k < tokens.size()){
				if (tokens[k].type == sep && tokens[k].str == "}") break;
				k++;
			}

			i = k+1;
		}else if (tokens[i].type == key && tokens[i].str == "subroutine"){
			log.info("Subroutine Statement, line: " + to_string(tokens[i].lnum));

			// Find end of closing bracket
			k = i+1;
			int indent = 0;
			while (k < tokens.size()){
				if (tokens[k].type == sep && tokens[k].str == "{") indent++;
				if (indent == 0){
					k++;
					continue; // Skip check for break until first indentation occurs
				}
				if (tokens[k].type == sep && tokens[k].str == "}") indent--;
				if (indent == 0) break;
				k++;
			}

			i = k+1;
		}else if (tokens[i].type == key && tokens[i].str == "macro"){
			log.info("Macro Statement, line: " + to_string(tokens[i].lnum));

			// Find end of closing bracket
			k = i+1;
			int indent = 0;
			while (k < tokens.size()){
				if (tokens[k].type == sep && tokens[k].str == "{") indent++;
				if (indent == 0){
					k++;
					continue; // Skip check for break until first indentation occurs
				}
				if (tokens[k].type == sep && tokens[k].str == "}") indent--;
				if (indent == 0) break;
				k++;
			}

			i = k+1;
		}else if (tokens[i].type == key && tokens[i].str == "expand"){
			log.info("Expand Statement, line: " + to_string(tokens[i].lnum));

			// Find end
			k = i+1;
			while (k < tokens.size()){
				if (tokens[k].type == nl) break;
				k++;
			}

			i = k+1;
		}else if (tokens[i].type == id && i < tokens.size()-1 && tokens[i+1].type == sep && tokens[i+1].str == "("){
			log.info("Subroutine Call Statement, line: " + to_string(tokens[i].lnum));

			// Find end
			k = i+1;
			while (k < tokens.size()){
				if (tokens[k].type == nl) break;
				k++;
			}

			i = k+1;
		}else if(tokens[i].type == nl){
			log.info("Empty Line, line: " + to_string(tokens[i].lnum));
			i++;
		}else{
			log.lerror("Failed to identify statement.", tokens[i].lnum);

			// Find end
			k = i+1;
			while (k < tokens.size()){
				if (tokens[k].type == nl) break;
				k++;
			}

			i = k+1;
		}
		// }else if (tokens[i].type == ){
		// 	log.info(" Statement, line: " + to_string(tokens[i].lnum));
		//
		// 	// Find end
		// 	k = i+1;
		// 	while (k < tokens.size()){
		// 		if (tokens[k].type == nl) break;
		// 		k++;
		// 	}
		//
		// 	i = k+1;
		// }


	}

	return parse_status;
}

#endif
