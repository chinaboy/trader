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

	void printTrade(){
		cout << "firm_id is " << unsigned(firm_id) << ", trader_tag is " << trader_tag[0] << trader_tag[1] << trader_tag[2] << ", qty is " << qty << endl;
	}

	uint8_t firm_id;
	char trader_tag[3]; // char [3]
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
		char result = f.get();
		return (uint8_t)result;
	}

	uint16_t getUint16(){		 
		uint16_t result;
		char s[2];
		f.get( (char*) s, 3 );
		result = ((uint16_t) s[1] << 8 ) + (uint16_t) s[0];
		return result;
	}

	uint32_t getUint32(){	
		uint32_t result;
		char s[4];
		f.get( (char*) s, 5 );
		result = ((uint32_t) s[3] << 24 ) + ((uint32_t) s[2] << 16 ) + ((uint32_t) s[1] << 8 ) + (uint32_t) s[0];
		return result;
	}

	uint64_t getUint64(){
		uint64_t low = (uint64_t)getUint32();
		uint64_t high = (uint64_t)getUint32();
		return (low << 32 | high);
	}

	vector<char> getChars(size_t n){
		vector<char> v(n);
		for(int i=0; i < n; i++)
			v.push_back( f.get() );
		//reverse(v.begin(), v.end());
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
				if( termination.compare(0, string::npos, (char*)(v.data() + v.size() - termination.length()), termination.length() ) == 0 ){
					for(auto c:termination)
						v.pop_back();
					break;
				}
			}
			//reverse(v.begin(), v.end());
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

	void printHeader(){ 
		cout<< "message type is " << unsigned(msg_type) << ", sequence_id is " << sequence_id << ", timestamp is " << timestamp << ", direction is " << unsigned(msg_direction) << ", msg_len is " << msg_len << endl;
	}
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