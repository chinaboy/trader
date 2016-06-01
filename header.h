#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector> 
#include <stdexcept> 

using namespace std;

class OrderEntryMessage;
class OrderAckMessage;
class OrderFillMessage;

class Trade{
public:
	Trade(uint8_t firm_id, string trader_tag, uint32_t qty){
		this->firm_id = firm_id;
		this->trader_tag = trader_tag;
		this->qty = qty;
	}
	uint8_t firm_id;
	string trader_tag; // char [3]
	uint32_t qty;
};

class BytesReader{
public:
	BytesReader(string stream){
		this->f.open(stream.c_str(), ifstream::binary);
		termination.assign("DBDBDBDB");
		 
	}

	~BytesReader(){
		this->f.close();
	}

	uint8_t getUint8(){
		uint8_t result;
		f.get((char)result);
		return result;
	}

	uint16_t getUint16(){		 
		uint16_t result;
		char s[2];
		f.get( (char*) s, 2 );
		result = ((uint16_t) s[1] << 8 ) + (uint16_t) s[0];
		return result;
	}

	uint32_t getUint32(){	
		uint32_t result;
		char s[4];
		f.get( (char*) s, 4 );
		result = ((uint32_t) s[3] << 24 ) + ((uint32_t) s[2] << 16 ) + ((uint32_t) s[1] ) << 8 + (uint32_t) s[0];
		return result;
	}

	uint64_t getUint64(){
		uint64_t low = (uint64_t)getUint32();
		uint64_t high = (uint64_t)getUint32();
		return (high << 32 | low);
	}

	string getChars(size_t n){
		string s( (const char*)this->buffer, n);
		 
		return s;
	}	

	// read til termination characters
	/// {{{
		string getMaxChars(){
			string s;
			return s;
		}

		vector<Trade> getTrades(){
			vector<Trade> v;
			uint8_t tmp_buffer[8];
			for(;;){
				f.read((char*)tmp_buffer, 8);
				string s((const char*)tmp_buffer, 8);

				// Is it end of repeated group of trades?
				if( s.compare(termination) == 0 ){
					break;
				}
				
				uint8_t firm_id = getUint8();
				string trader_tag = getChars(3);
				uint32_t qty = getUint32();
				Trade t(firm_id, trader_tag, qty);
				v.push_back(t);
			}
			return std::move(v);
		}
	/// }}}
private:
	ifstream f;
	vector<unsigned char> buffer; //uint8_t* buffer;
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

	void printHeader(){ this->br->printHeader();}
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
	string instrument ; // char instrument[10];
	uint8_t side;
	uint64_t client_assigned_id;
	uint8_t time_in_force;
	string trader_tag ; // char trader_tag[3];
	uint8_t firm_id;
	string firm; // max 255 chars
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