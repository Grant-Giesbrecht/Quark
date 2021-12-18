#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <bitset>
#include <map>
#include <cmath>
#include <ctgmath>
#include "subatomic.hpp"

/*
KEY: (Standard) Keyword
ID: Identifier
NUM: Numeric value
OP: Operator
CD: Compiler Directive
SOP: Subroutine Operator
TYP: Type keyword
CMT: Comment
SEP: Seperator
LIT: Literal keyword
INS: Insruction
NL: Newline
*/
enum TokenType {key, id, num, op, cd, sop, typ, cmt, sep, lit, ins, nl}; // Token categories

typedef struct{
	// Type of token
	TokenType type; // Member of Cats

	bool use_float;

	// Token value
	std::string str;
	int vali;
	double valf;
	bool valb;
	int valv[3]; //For versions

	// Metadata
	size_t lnum; // Declaration line
}qtoken;

/*
Convert a qtoken to a string
*/
std::string tokenstr(qtoken tok){

	std::string str;

	switch(tok.type){
		case TokenType::key:
			str = "[key: \"" + tok.str + "\"]";
			break;
		case TokenType::id:
			str = "[id: \"" + tok.str + "\"]";
			break;
		case TokenType::num:
			(tok.use_float) ? str = "[num: \"" + to_string(tok.valf) + "\"]" : str = "[num: \"" + to_string(tok.vali) + "\"]";
			break;
		case TokenType::op:
			str = "[op: \"" + tok.str + "\"]";
			break;
		case TokenType::cd:
			str = "[cd: \"" + tok.str + "\"]";
			break;
		case TokenType::sop:
			str = "[sop: \"" + tok.str + "\"]";
			break;
		case TokenType::typ:
			str = "[typ: \"" + tok.str + "\"]";
			break;
		case TokenType::cmt:
			str = "[cmt: \"" + tok.str + "\"]";
			break;
		case TokenType::sep:
			str = "[sep: \"" + tok.str + "\"]";
			break;
		case TokenType::lit:
			str = "[lit: \"" + tok.str + "\"]";
			break;
		case TokenType::ins:
			str = "[ins: \"" + tok.str + "\"]";
			break;
		case TokenType::nl:
			str = "[newline]";
			break;
	}

	return str;
}
