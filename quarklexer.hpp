#ifndef QUARKLEXER_HPP
#define QUARKLEXER_HPP

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

	//******************* Scan over Program, Break into Tokens *****************

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
				nt.type = TokenType::ins;
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

#endif
