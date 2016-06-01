#include "header.h"


void Header::read(){
	this->br->setBufferSize(header_size);
	 
	//this->marker = this->br->getUint16();
	uint8_t first = this->br->getUint8();
	uint8_t second = this->br->getUint8();
	this->msg_type = this->br->getUint8();			
	cout << std::oct << unsigned( first ) << "\t" << std::oct << unsigned( second ) << endl << std::oct << "S" << "\t" << std::oct << "T" << endl << unsigned( this->msg_type ) << endl;
	return;
	this->sequence_id = this->br->getUint64(); 
	this->timestamp = this->br->getUint64();
	this->msg_direction = this->br->getUint8();
	this->msg_len = this->br->getUint16();
	op();
}

void Header::op(){
	switch(this->msg_type){
		case 1:
			{
				OrderEntryMessage oem;
				oem.init(this);
			}
			break;
		case 2:
			{
				OrderAckMessage oam;
				oam.init(this);
				break;
			}
		case 3:
			{
				OrderFillMessage ofm;
				ofm.init(this);
				break;
			}
		default:
			throw runtime_error("unknown message type");
	}
}

void OrderEntryMessage::init(Header * hdr){   // exclude variable firm string and termination; max 255 and 8
	//this.hdr = std::move(hdr);
	this->hdr = hdr;
	BytesReader *br = hdr->getBytesReader();
	br->setBufferSize(fix_size);
	
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
	this->firm = br->getMaxChars();
}

void OrderAckMessage::init(Header * hdr){
	this->hdr = hdr;
	BytesReader *br = hdr->getBytesReader();
	br->setBufferSize(fix_size);
	
	this->order_id = br->getUint32();
	this->client_id = br->getUint64();
	this->order_status = br->getUint8();
	this->reject_code = br->getUint8();
}

void OrderFillMessage::init(Header * hdr){
	this->hdr = hdr;
	BytesReader *br = hdr->getBytesReader();
	br->setBufferSize(fix_size);

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
