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
#include "LutTypes.hpp"

using namespace std;
usign namespace gstd;

int main(int argc, char** argv){

	map<string, string> settings = load_conf("quark.conf");

	show_conf(settings);

	return 0;
}
