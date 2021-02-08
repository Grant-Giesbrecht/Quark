#include <cstdlib>
#include <iostream>
#include <fstream>
#include <time.h>

using namespace std;

int main () {

	srand(time(NULL));

	ofstream file;
	file.open ("exmat_rev.pcm");

	for (int i = 0 ; i < 2048 ; i++){
		// file << i << ":" << rand()%256 << endl;
		file << i << ":" << (2047-i)%256 << endl;
	}

	file.close();
	return 0;

}
