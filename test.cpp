#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector> 
#include <stdexcept> 
#include <algorithm> 
#include <assert.h> 

using namespace std;

int parseStream(string stream){	
	ifstream f;
	f.open(stream.c_str(), ifstream::binary);

	while( !f.eof()){
		cout << hex << f.get() << ' ' ;
	}
	cout << endl;
	return 1;
}

int main(int argc, char* argv[]){
	if(argc > 1){
		cout << argv[1] << endl;
		string file_stream(argv[1]);
		parseStream(file_stream);

	}
	return 0;
}
