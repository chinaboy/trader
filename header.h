#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector> 
#include <stdexcept> 
#include <algorithm> 

using namespace std;

class OrderEntryMessage;
class OrderAckMessage;
class OrderFillMessage;

class Trade{
public:
	Trade(char *tmp_buffer){
		this->firm_id = tmp_buffer[0];
		this->trader_tag[2] = tmp_buffer[1];
		this->trader_tag[1] = tmp_buffer[2];
		this->trader_tag[0] = tmp_buffer[3];
		this->qty = ((uint32_t) tmp_buffer[7] << 24 ) + ((uint32_t) tmp_buffer[6] << 16 ) + ((uint32_t) tmp_buffer[5] << 8 ) + (uint32_t) tmp_buffer[4];
	}
	uint8_t firm_id;
	char trader_tag[3]; // char [3]
	uint32_t qty;
};

class BytesReader{
public:
	BytesReader(string stream){
		this->f.open(stream.c_str(), ifstream::binary);
		termination.assign("BDBDBDBD"); // in reverse
		 
	}

	~BytesReader(){
		this->f.close();
	}

	uint8_t getUint8(){
		uint8_t result = f.get();
		return result;
	}

	uint16_t getUint16(){		 
		uint16_t result;
		char s[2];
		f.get( (char*) s, 2 );
		result = ((uint16_t) s[0] << 8 ) + (uint16_t) s[1];
		return result;
	}

	uint32_t getUint32(){	
		uint32_t result;
		char s[4];
		f.get( (char*) s, 4 );
		result = ((uint32_t) s[0] << 24 ) + ((uint32_t) s[1] << 16 ) + ((uint32_t) s[2] << 8 ) + (uint32_t) s[3];
		return result;
	}

	uint64_t getUint64(){
		uint64_t low = (uint64_t)getUint32();
		uint64_t high = (uint64_t)getUint32();
		return (low << 32 | high);
	}

	vector<char> getChars(size_t n){
		vector<char> v(n);
		for(int i=0; i<n; i++)
			v.push_back( f.get() );
		reverse(v.begin(), v.end());
		return std::move(v);
	}	

	// read til termination characters
	/// {{{
		vector<char> getMaxChars(int limit){
			vector<char> v;
			int readChars = 0;
			while( readChars < limit ){
				v.push_back( f.get() );
				readChars ++;
				if( v.size() < termination.length() )
					continue;
				if( termination.compare(0, string::npos, (char*)(v.data() + v.size() - termination.length()), termination.length() ) == 0 )
					break;
			}
			return std::move(v);
		}

		vector<Trade> getTrades(){
			vector<Trade> v;
			char tmp_buffer[8];
			for(;;){
				f.read((char*)tmp_buffer, 8);

				// Is it end of repeated group of trades?
				if( termination.compare( 0, string::npos, (char*)tmp_buffer, 8 ) == 0 ){
					break;
				}
				
				Trade t(tmp_buffer);
				v.push_back(t);
			}
			return std::move(v);
		}
	/// }}}

		void consumeTermination(){
			getChars(8);
		}
private:
	ifstream f;
	string termination;
};

class Header{
public:
	Header(string filename) : header_size(22){		 
		this->br = new BytesReader(filename);		
	}

	~Header(){
		delete this->br;
	}

	void read();
	void op();
	

	bool next(){
		return ! this->f.eof();
	}	

	BytesReader * getBytesReader(){return br;}

	void printHeader(){ }
private:
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
	OrderEntryMessage(): fix_size(37){}

	void init(Header* hdr);

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
private:
	Header *hdr;
	uint32_t order_id;
	uint64_t fill_price;
	uint32_t fill_qty;
	uint8_t no_of_contras;
	vector<Trade> trades;
	int fix_size;
};