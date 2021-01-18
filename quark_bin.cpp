/*
This program takes a Perihelion/Hacksembly assembly file and converts it to
binary. It requires a file to provide the codes to translate between assembly
and binary. See the comments for function 'load_keys()' to understand the formating
requirements of the code-containging file.

Pseudocode structure:
0.) (Get user options)
1.) Read in codes/keys
2.) Go line-by-line through assembly file - save data to vector of 'byte' objects
3.) Write bin file
*/

#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <fstream>
#include <gstd.hpp>
#include <ktable.hpp>
#include <sstream>
#include <bitset>

//CXCOMPILE g++ hacklink.cpp -o hacklink -lgstd -lktable -std=c++11
//CXCOMPILE ./hacklink example1.pas -v
//CXGENRUN FALSE

using namespace std;
using namespace gstd;

/*
Represents an assembly/binary translation code
*/
typedef struct{
	string assembly; //Assembly string
	string binary; //Binary string
	int ndb; //Number of bytes of data after instruction (eg. for storing address, numbers, etc)
}key;

/*
Represents a byte that will be written to homebrew memory.
*/
typedef struct{
	string data;
	size_t address;
}byte;

bool load_keys(string keyfile, vector<key>& keys);
bool place_byte(vector<byte>& bin, byte x, int& greatest_address, size_t line_num);
int hbdstring_int(string num);
string hbdstring_bin(string num);

int main(int argc, char** argv){

	//************************************************************************//
	//******************* GET FILES, OPTIONS, ETC ****************************//

	string keyfile = "perihelion_assembly_codes.bcod"; //where to find assembly -> binary codes

	bool verbose = false; //Print verbose output
	bool create_bin = true; //Create binary file

	vector<byte> bin; //Binary out
	vector<key> keys;

	//Get input file's name
	if (argc < 2){
		cout << "ERROR: Requires .PAS file's name (no spaces allowed) as input." << endl;
		return -1;
	}
	string pas_file = argv[1];

	//If flags included...
	if (argc > 2){
		//Scan through flags
		for (int arg = 2 ; arg < argc ; arg++){
			if (string(argv[arg]) == "-v"){
				verbose = true;
			}else if(string(argv[arg]) == "-p"){ //"practice"
				create_bin = false;
			}else{
				cout << "WARNING: Ignoring unrecognized flag '" << argv[arg] << "'." << endl;
			}
		}

	}//end flags-if


	//***********************************************************************//
	//******************* READ IN ASSEMBLY-BINARY CONVERSION CODES **********//

	//Load keys
	if (!load_keys(keyfile, keys)){
		return -1;
	}
	if (verbose){

		KTable kt;
		kt.table_title("ASSEMBLY CODES");
		kt.row({"ASSEMBLY", "BINARY", "NO. AUX BYTES"});

		// cout << "ASSEMBLY CODES:" << endl;
		for (size_t i = 0 ; i < keys.size() ; i++){
			// cout << "\tASSEMBLY: " << keys[i].assembly << "\t" << " \tBINARY: " << keys[i].binary << " \tNUM DATA BYTES: " << to_string(keys[i].ndb) << endl;
			kt.row({keys[i].assembly, keys[i].binary, to_gstring(keys[i].ndb, 30, 5)});
		}

		kt.alignt('c');
		kt.alignh('l');
		kt.alignc('r');
		kt.alignc(0, 'l');

		cout << kt.str() << endl;

	}

	//********************************************************************//
	//************ READ .PAS FILE -> LINK WITH CODES, CREATE BIN *********//

	vector<string> words;

	//read through file
	size_t line_num = 0;
	ifstream file(pas_file.c_str());
	if (file.is_open()) {

		int greatest_address = -1; //Greatest address recorded in binary list
	 	string line;

	    while (getline(file, line)) { //For each line...
			line_num++;

			trim_whitespace(line); //Remove whitespace from line

			if (line.length() == 0) continue; //Continue if blank
			if (line.length() >= 2 && line.substr(0, 2) == "//") continue; //Skip comments
			ensure_whitespace(line, ":"); //Esure semicolons are recognized as a token

			//Parse words
			words = parse(line, " \t");
			if(words.size() == 0) continue; //Skip blank lines

			//Ensure enough tokens...
			if (words.size() < 3){
				cout << "ERROR: " << pas_file << ":Line " << to_string(line_num) << ": Invalid syntax - too few tokens" << endl;
				return -1;
			}

			string address_str = words[0];
			size_t address = hbdstring_int(address_str);


			//Ensure semicolon present
			if (words[1] != ":"){
				cout << "ERROR: " << pas_file << ":Line " << to_string(line_num) << ": Invalid syntax - missing colon" << endl;
				return -1;
			}

			//Find code...
			bool found = false;
			size_t idx;
			for (size_t i = 0 ; i < keys.size() ; i++){
				if (keys[i].assembly == words[2]){ //Code found...

					//*********************** CODE FOUND *********************//

					//Create new byte object
					byte temp_byte;
					temp_byte.address = address;
					temp_byte.data = keys[i].binary;

					//Insert new byte into bin vector
					if (!place_byte(bin, temp_byte, greatest_address, line_num)){
						return -1;
					}

					//Check that correct number of data bytes provided
					if (words.size()-3 != keys[i].ndb){
						cout << "ERROR: Wrong number of data bytes provided for instruction on line " << to_string(line_num) << endl;
						cout << "\tINSTR: " << words[2] << " \tExpected: " << keys[i].ndb << " \tProvided: " << words.size()-3 << endl;
						return -1;
					}

					//correct number of data bytes given - copy into binary object
					for (size_t ad = 3 ; ad < words.size() ; ad++){
						byte aux_byte;
						aux_byte.data = hbdstring_bin(words[ad]);
						aux_byte.address = address + ad-2;

						if (!place_byte(bin, aux_byte, greatest_address, line_num)){
							return -1;
						}
					}

					found = true;
					break;

					//******************** Continue... **********************//
				}
			}

			//Code not found - must be specifying byte explicitly
			if (!found){

				byte temp_byte;
				int val = hbdstring_int(words[2]);
				temp_byte.data = hbdstring_bin(words[2]);
				temp_byte.address = address;

				if (words.size() > 3){
					cout << "ERROR: More than one byte specified for address " << to_string(address) << " on line " << to_string(line_num) << "." << endl;
					return -1;
				}

				if (!place_byte(bin, temp_byte, greatest_address, line_num)){
					return -1;
				}

				if (val == -1){
					cout << "ERROR: Failed to process line" << to_string(line_num) << ".\n\tFailed to identify token '" << words[2] << "' as an instruction or numeric value." << endl;
					return -1;
				}
			}


	    }
	    file.close();
	}else{
		cout << "ERROR: Failed to read '" << pas_file << "'." << endl;
		return -1;
	}

	if (verbose){
		cout << "BINARY OUT:" << endl;
		for (size_t i = 0 ; i < bin.size() ; i++){
			cout << "\t" << to_string(bin[i].address) << ": " << bin[i].data << endl;
		}
		cout << endl;
	}

	//Create binary file
	if (create_bin){

		ofstream outfile(pas_file.substr(0, pas_file.length()-3) + "pbin");
	    if(outfile.is_open()){
	        string str;

			//For each byte...
			size_t line = -1;
			size_t filler_bytes = 0;
			for (size_t l = 0 ; l < bin.size() ; l++){

				line++;

				//Get to correct address...
				while (line < bin[l].address){
					outfile << "00000000\n";
					line++;
					filler_bytes++;
				}

				outfile << bin[l].data << "\n";

			}

	        outfile.close();

			cout << "Wrote binary file '" << pas_file.substr(0, pas_file.length()-3) + "pbin" << "':" << endl;
			cout << "\tTotal bytes written: " << to_string(line) << endl;
			cout << "\tDefined bytes: " << to_string(bin.size()) << endl;
			cout << "\tFiller bytes: " << to_string(filler_bytes) << endl;
	    }

	}


	return 0;
}

/*
Filters through 'bin' and places 'byte' where it belongs, according to the address.
Greatest address is used to accelerate the process - just initialize it with -1
before calling 'place_byte' the first time, then don't touch it so different calls
of this function can remember it. line_num is just used for giving nice error
messages.
*/
bool place_byte(vector<byte>& bin, byte x, int& greatest_address, size_t line_num){

	//If address > last address append
	if ((int)(x.address) > greatest_address){
		greatest_address = x.address;
		bin.push_back(x);
	}else{

		//Else find location
		size_t insert_idx = -1;
		for (size_t b = 0 ; b < bin.size() ; b++){ //Scan through array of binary data
			if (bin[b].address == x.address){
				cout << "ERROR: Address " << to_string(x.address) << " assigned twice, 2nd time with line " << to_string(line_num) << endl;
				return false;
			}
			if ((b == 0 || bin[b-1].address < x.address) && (bin[b].address > x.address) ){
				bin.insert(bin.begin()+b, x);
				break;
			}
		}
	}

	return true;
}

/*
Reads a file (keyfile) which contains comments (//) and assembly/binary conversion
information. All lines that aren't blank or comments need to have three tokens:
first the assembly name for the instruction, then the binary code for the instruction,
then the number of bytes used by data for the instruction (ex. halt uses 0 extra,
RAM_REGA uses two extra for the two-byte address, RAM_FLASH uses four extra for
the two two-byte addresses).

Saves results into 'keys' struct.
*/
bool load_keys(string keyfile, vector<key>& keys){

	vector<string> words;

	size_t line_num = 0;

	//read through file
	ifstream file(keyfile.c_str());
	if (file.is_open()) {
	 	string line;

	    while (getline(file, line)) {
			line_num++;

			trim_whitespace(line); //Remove whitespace from line

			if (line.length() == 0) continue; //Continue if blank
			if (line.length() >= 2 && line.substr(0, 2) == "//") continue; //Skip comments

			//Parse words
			words = parse(line, " \t");

			//Ensure exactly 3 tokens
			if (words.size() == 3){

				key tempkey;
				tempkey.assembly = words[0];
				tempkey.binary = words[1];
				tempkey.ndb = stoi(words[2]);

				keys.push_back(tempkey);

			}else{
				cout << "ERROR: " << keyfile << ":Line " << to_string(line_num) << " contains more/less than 3 tokens" << endl;
				return false;
			}


	    }
	    file.close();
	}else{
		cout << "ERROR: Failed to read '" << keyfile << "'." << endl;
		return false;
	}

	return true;
}

/*
Accepts an string representing an integer 'num', and converts it to an int. The
cool thing about this function though is that the input num can be of decimal,
hexadecimal, or binary format. Just write the number for decimal or append an 'x'
or 'b' for hex and bin formats, respectively!
*/
int hbdstring_int(string num){

	if (num.length() < 2){
		return stoi(num);
	}

	if (num.at(0) == 'x'){
		int x;
		std::stringstream ss;
		ss << std::hex << num.substr(1);
		ss >> x;
		return x;
	}else if(num.at(0) == 'b'){
		return std::stoi(num.substr(1), nullptr, 2);
	}else{ //decimal
		return stoi(num);
	}

}

/*
Same idea as hbdstring_int(), except instead of returning an integer type, it
returns a string of the integer data represented as an 8-bit binary byte.
*/
string hbdstring_bin(string num){

	//Get numeric value
	int val = hbdstring_int(num);

	//Process errors
	if (val == -1) return "ERROR";

	//Convert int to binary string
	std::string r;
    while (val!=0) { //Algorithm borrowed from GitHub (https://stackoverflow.com/questions/22746429/c-decimal-to-binary-converting)
		r = ( val % 2 == 0 ? "0" : "1" ) + r;
		val /= 2;
	}

	//Pad with leading zeros (takes care of zero-case)
	while (r.length() < 8) r = "0" + r;

	return r;

}




















//
