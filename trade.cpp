#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector> 
#include <stdexcept> 

using namespace std;

class Trader;
class BytesReader;
class Header;
class OrderEntryMessage;
class OrderAckMessage;
class OrderFillMessage;

class Trader{
public:
	Trader(uint8_t firm_id, string trader_tag, uint32_t qty){
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
		termination.assign("BDBDBDBD");
		this->pos = 0;
	}

	~BytesReader(){
		delete[] this->buffer;
	}

	void setBufferSize(int fix_size){
		if(this->buffer){
			delete[] this->buffer;
		}
		this->buffer = new uint8_t[fix_size];
		this->f.read((char*)buffer, fix_size);
	}

	uint8_t getUint8(){
		uint8_t result = this->buffer[this->pos];
		this->pos++;
		return result;
	}

	uint16_t getUint16(){
		int pos = this->pos;
		uint16_t result = ((uint16_t)buffer[pos+1] << 8) | buffer[pos];
		this->pos += 2;
		return result;
	}

	uint32_t getUint32(){	
		uint8_t* buffer = this->buffer + this->pos;
		uint32_t result = ((uint32_t)buffer[3] << 24) | ((uint32_t)buffer[2] << 16) | ((uint32_t)buffer[1] << 8) | buffer[0];
		this->pos += 4;
		return result;
	}

	uint64_t getUint64(){
		int pos = this->pos;
		uint8_t* buffer = this->buffer + this->pos;
		uint64_t result = ((uint64_t)buffer[7] << 56) | ((uint64_t)buffer[6] << 48) | ((uint64_t)buffer[5] << 40) |((uint64_t)buffer[4] << 32 ) |
					((uint64_t)buffer[3] << 24) | ((uint64_t)buffer[2] << 16) | ((uint64_t)buffer[1] << 8) | buffer[0];
		this->pos += 8;
		return result;
	}

	string getChars(size_t n){
		string s( (const char*)this->buffer, n);
		this->pos += n;
		return s;
	}	

private:
	ifstream f;
	uint8_t* buffer;
	int pos;
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

	void read(){
		this->br->setBufferSize(header_size);
		 
		this->marker = this->br->getUint16();		 
		this->msg_type = this->br->getUint8();			
		this->sequence_id = this->br->getUint64(); 
		this->timestamp = this->br->getUint64();
		this->msg_direction = this->br->getUint8();
		this->msg_len = this->br->getUint16();
		op();
	}

	void op(){
			switch(this->msg_type){
				case 1:
					OrderEntryMessage oem(this);

					break;
				case 2:
					OrderAckMessage oam(this);
					break;
				case 3;
					OrderFillMessage ofm(this);
					break;
				default:
					throw runtime_error("unknown message type");
			}
	}

	bool next(){
		return ! this->f.eof();
	}

	string getMaxChars(ifstream &f){
		string s;
		return s;
	}

	vector<Trader> getTraders(ifstream &f){
		vector<Trader> v;
		uint8_t buffer[8];
		for(;;){
			f.read((char*)buffer, 8);
			string s((const char*)buffer, 8);
			if( s.compare(termination) == 0 ){
				break;
			}
			this->buffer = buffer;
			this->pos = 0;
			uint8_t firm_id = getUint8();
			string trader_tag = getChars(3);
			uint32_t qty = getUint32();
			Trader t(firm_id, trader_tag, qty);
			v.push_back(t);
		}
		return std::move(v);
	}

	BytesReader * getBytesReader(){return br;}
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
	OrderEntryMessage(Header * hdr): fix_size(37){   // exclude variable firm string and termination; max 255 and 8
		//this.hdr = std::move(hdr);
		this->hdr = hdr;
		BytesReader *br = hdr->getBytesReader();
		br->setBufferSize(fix_size);
		
		this->price = br->getUint64();
		this->qty = br->getUint32();
		// copy 10 chars to instrument
		this->instrument = br->getChars(10);

		this->getUint8 = br->getUint8();
		this->client_assigned_id = br->getUint64();
		this->time_in_force = br->getUint8();

		// copy 3 chars to trader tag
		this->trader_tag = br->getChars(3);
		this->firm_id = br->getUint8();

		// read until termination string
		this->firm = br->getMaxChars();
	}

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
	OrderAckMessage(Header * hdr, ifstream &f) : fix_size(14){
		this->hdr = hdr;
		BytesReader *br = hdr->getBytesReader();
		br->setBufferSize(fix_size);
		
		this->order_id = br->getUint32();
		this->client_id = br->getUint64();
		this->order_status = br->getUint8();
		this->reject_code = br->getUint8();
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
	OrderFillMessage(Header * hdr, ifstream &f) : fix_size(17){
		this->hdr = hdr;
		BytesReader *br = hdr->getBytesReader();
		br->setBufferSize(fix_size);

		this->order_id = br->getUint32();
		this->fill_price = br->getUint64();
		this->fill_qty = br->getUint32();
		this->no_of_contras = br->getUint8();
		this->traders = br->getTraders(f);
	}

	~OrderFillMessage(){
		
	}
private:
	Header *hdr;
	uint32_t order_id;
	uint64_t fill_price;
	uint32_t fill_qty;
	uint8_t no_of_contras;
	vector<Trader> traders;
	int fix_size;
};

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
