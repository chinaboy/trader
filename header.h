#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector> 
#include <stdexcept> 
#include <algorithm> 
#include <assert.h> 

using namespace std;

class OrderEntryMessage;
class OrderAckMessage;
class OrderFillMessage;

class Stats{
public:
	Stats(){
		oem_count = oam_count = ofm_count = packet = 0;
	}

	void incrementOEM(){
		oem_count ++;
	}

	void incrementOAM(){
		oam_count ++;
	}

	void incrementOFM(){
		ofm_count ++;
	}

	void incrementPacket(){
		packet ++;
	}

	void print(){

	}

	int oem_count, oam_count, packet, ofm_count ;

};

class Trade{
public:
	/*
	Trade(char *tmp_buffer){
		this->firm_id = tmp_buffer[0];
		this->trader_tag[2] = tmp_buffer[1];
		this->trader_tag[1] = tmp_buffer[2];
		this->trader_tag[0] = tmp_buffer[3];
		this->qty = ((uint32_t) tmp_buffer[7] << 24 ) + ((uint32_t) tmp_buffer[6] << 16 ) + ((uint32_t) tmp_buffer[5] << 8 ) + (uint32_t) tmp_buffer[4];
	}
	*/

	void printTrade(){
		cout << "firm_id is " << unsigned(firm_id) << ", trader_tag is " << trader_tag[0] << trader_tag[1] << trader_tag[2] << ", qty is " << qty << endl;
	}

	uint8_t firm_id;
	vector<char> trader_tag; // char [3]
	uint32_t qty;
};

class BytesReader{
public:
	BytesReader(string stream){
		this->f.open(stream.c_str(), ios::in | ifstream::binary);
		termination.assign("DBDBDBDB"); 
		capacity = 1000;
		buffer = new char[capacity + 1];
	}

	~BytesReader(){
		this->f.close();
		if( buffer ){
			delete[] buffer;
		}
	}

	bool good(){
		if(f)
			return true;
		return false;
	}

	bool readBuffer(int n){
		buffer_size = n;
		if( n > capacity){
			delete[] buffer;
			buffer = new char[n+1];
			capacity = n;
		}
		pos = 0;

		f.read( buffer, n );
		if(!f){
			//delete[] buffer;
			cout<< "error: only " << f.gcount() << " could be read";
			//throw runtime_error("can't read anything");
			return false;
		}
		return true;
	}


	uint8_t getUint8(){
		assert( pos + 1 <= buffer_size );
		unsigned char result = buffer[pos++];
		return (uint8_t)result;
	}

	uint16_t getUint16(){	
		assert( pos + 2 <= buffer_size );	 
		uint16_t result;
		unsigned char s[3];
		s[0] = buffer[pos];
		s[1] = buffer[pos+1];
		pos++, pos++;
		result = ((uint16_t) s[1] << 8 ) | (uint16_t) s[0];
		return result;
	}

	uint32_t getUint32(){	
		assert( pos + 4 <= buffer_size );
		uint32_t result;
		unsigned char s[5];
		//f.get( (char*) s, 5 );
		s[0] = buffer[pos];
		s[1] = buffer[pos+1];
		s[2] = buffer[pos+2];
		s[3] = buffer[pos+3];
		pos += 4;
		result = ((uint32_t) s[3] << 24 ) | ((uint32_t) s[2] << 16 ) | ((uint32_t) s[1] << 8 ) | (uint32_t) s[0];
		return result;
	}

	uint64_t getUint64(){
		assert( pos + 8 <= buffer_size );
		unsigned char s[9];
		//f.get( (char*) s, 9 );
		s[0] = buffer[pos];
		s[1] = buffer[pos+1];
		s[2] = buffer[pos+2];
		s[3] = buffer[pos+3];
		s[4] = buffer[pos+4];
		s[5] = buffer[pos+5];
		s[6] = buffer[pos+6];
		s[7] = buffer[pos+7];	
		pos+= 8;	
		uint64_t result = ((uint64_t) s[7] << 56 ) | ((uint64_t) s[6] << 48 ) | ((uint64_t) s[5] << 40 ) | ((uint64_t) s[4] << 32 ) | ((uint64_t) s[3] << 24 ) | ((uint64_t) s[2] << 16 ) | ((uint64_t) s[1] << 8 ) | (uint64_t) s[0];
		return result;
	}

	vector<char> getChars(int n){
		if( n + pos > buffer_size ){

			cout << "before assert exception: n = " << n << " pos = " << pos << " buffer_size = " << buffer_size << endl; 
		}
		assert( n + pos <= buffer_size );
		vector<char> v;
		v.reserve(n);
		for(int i=0; i < n; i++)
			v.push_back( buffer[pos++] );
		//reverse(v.begin(), v.end());
		return std::move(v);
	}	

	// read til termination characters
	/// {{{
	/*
		vector<char> getMaxChars(int size){
			vector<char> v;
			v.reserve(size);
			for(int i=0; i<size; i++)
				v[i] = f.get();			 
			return std::move(v);
		}*/

		vector<Trade> getTrades(int no_of_contras){
			vector<Trade> v;
			for(int i=0; i<no_of_contras; ++i){	
				Trade t;
				t.firm_id = getUint8();
				t.trader_tag = getChars(3);
				t.qty = getUint32();

				v.push_back(t);
			}
			return std::move(v);
		}
	/// }}}

		void consumeTermination(){
			vector<char> v = getChars(8);
			string end = string(v.begin(), v.end());
			assert( end.compare(termination) == 0  );
			 
		}
private:
	int capacity;
	int buffer_size;
	int pos;
	char * buffer;
	ifstream f;
	string termination;
};

class Header{
public:
	Header(string filename) : header_size(22){		 
		this->br = new BytesReader(filename);	
		eof = false;	
	}

	~Header(){
		delete this->br;
	}

	void read();
	void op();
	
	void setEof(){ eof = true; }

	bool next(){
		if(eof)
			return false;
		return this->f.good();
	}	

	BytesReader * getBytesReader(){return br;}

	void printHeader(){ 
		cout<< "message type is " << unsigned(msg_type) << ", sequence_id is " << sequence_id << ", timestamp is " << hex << timestamp << ", direction is " << unsigned(msg_direction) << ", msg_len is " << msg_len << endl;
	}

	void printStats(){
		stats.print();
	}

	int getMsgLen(){return (int)msg_len;}


private:
	bool eof;
	Stats stats;
	ifstream f;
	int header_size;
	BytesReader *br;
	uint16_t marker;
	uint8_t msg_type;
	uint64_t sequence_id;
	uint64_t timestamp;
	uint8_t msg_direction;
	uint16_t msg_len;
};

class OrderEntryMessage{
public:
	OrderEntryMessage(): fix_size(36){}

	void init(Header* hdr);

	void printOEM(){
		cout << "price is " << price << ", qty is " << qty << ", instrument is " << string(instrument.begin(), instrument.end()) << ", side is " << unsigned(side) << ", client_assigned_id is " << client_assigned_id
			<< ", time_in_force is " << unsigned(time_in_force) << ", trader_tag is " << string(trader_tag.begin(), trader_tag.end() ) << ", firm_id is " << unsigned(firm_id) << ", firm is " << string(firm.begin(), firm.end()) << endl;
	}
private:
	Header *hdr;
	
	uint64_t price;
	uint32_t qty;
	vector<char> instrument ; // char instrument[10];
	uint8_t side;
	uint64_t client_assigned_id;
	uint8_t time_in_force;
	vector<char> trader_tag ; // char trader_tag[3];
	uint8_t firm_id;
	vector<char> firm; // max 255 chars
	int fix_size;
};

class OrderAckMessage{
public:
	OrderAckMessage() : fix_size(14){}

	void init(Header * hdr);	

	void printOAM(){
		cout << "order_id is " << order_id << ", client_id is " << client_id << ", order_status is " << unsigned(order_status) << ", reject_code is " << unsigned(reject_code) << endl;
	}

private:
	Header *hdr;
	uint32_t order_id;
	uint64_t client_id;
	uint8_t order_status;
	uint8_t reject_code;
	int fix_size;
};


class OrderFillMessage{
public:
	OrderFillMessage() : fix_size(17){}

	void init(Header* hdr);

	~OrderFillMessage(){}

	void printOFM(){
		cout << "order_id is "<< order_id << ", fill_price is " << fill_price << ", fill_qty is " << fill_qty << ", no_of_contras is " << unsigned(no_of_contras) << endl;
		for(auto t:trades)
			t.printTrade();
	}
private:
	Header *hdr;
	uint32_t order_id;
	uint64_t fill_price;
	uint32_t fill_qty;
	uint8_t no_of_contras;
	vector<Trade> trades;
	int fix_size;
};