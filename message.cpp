#include "message.h"

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