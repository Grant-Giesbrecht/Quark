#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>

using namespace std;

int main(int argc, char** argv){

	bool save_apas = false; //Tell compiler to save .APAS file

	//If flags included...
	if (argc > 1){

		//Scan through flags
		for (int arg = 1 ; arg < argc ; arg++){
			if (argv[arg] == "-apas"){
				save_apas = true;
			}
		}

	}


	return 0;
}
