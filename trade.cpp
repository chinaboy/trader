#include "trade.h"

class Header;

int parseStream(string stream){
		
	Header hdr(stream);
	while( hdr.next() ){
		hdr.read();
		break;
	}
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
