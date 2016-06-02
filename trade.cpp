#include "header.h"

void Header::read(){
	this->marker = this->br->getUint16();
	this->msg_type = this->br->getUint8();			
	this->sequence_id = this->br->getUint64(); 
	this->timestamp = this->br->getUint64();
	this->msg_direction = this->br->getUint8();
	this->msg_len = this->br->getUint16();
	stats.incrementPacket();
	op();
}

void Header::op(){
	//printHeader();
	switch(unsigned(this->msg_type)){
		case 1:
			{
				OrderEntryMessage oem;
				oem.init(this);
				//oem.printOEM();
				stats.incrementOEM();
			}
			break;
		case 2:
			{
				OrderAckMessage oam;
				oam.init(this);
				//oam.printOAM();
				stats.incrementOAM();
				break;
			}
		case 3:
			{
				OrderFillMessage ofm;
				ofm.init(this);
				//ofm.printOFM();
				stats.incrementOFM();
				break;
			}
		default:
			throw runtime_error(string( this->msg_type ) + "unknown message type " );
	}
}

void OrderEntryMessage::init(Header * hdr){   // exclude variable firm string and termination; max 255 and 8
	//this.hdr = std::move(hdr);
	this->hdr = hdr;
	BytesReader *br = hdr->getBytesReader();
	 
	
	this->price = br->getUint64();
	this->qty = br->getUint32();
	// copy 10 chars to instrument
	this->instrument = br->getChars(10);

	this->side = br->getUint8();
	this->client_assigned_id = br->getUint64();
	this->time_in_force = br->getUint8();

	// copy 3 chars to trader tag
	this->trader_tag = br->getChars(3);
	this->firm_id = br->getUint8();

	// read until termination string
	this->firm = br->getMaxChars(255);
}

void OrderAckMessage::init(Header * hdr){
	this->hdr = hdr;
	BytesReader *br = hdr->getBytesReader();
	
	this->order_id = br->getUint32();
	this->client_id = br->getUint64();
	this->order_status = br->getUint8();
	this->reject_code = br->getUint8();
	// consume termination characters
	br->consumeTermination();
}

void OrderFillMessage::init(Header * hdr){
	this->hdr = hdr;
	BytesReader *br = hdr->getBytesReader();

	this->order_id = br->getUint32();
	this->fill_price = br->getUint64();
	this->fill_qty = br->getUint32();
	this->no_of_contras = br->getUint8();
	this->trades = br->getTrades();
}


int parseStream(string stream){	
	Header hdr(stream);
	while( hdr.next() ){
		hdr.read();
		//break;
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
