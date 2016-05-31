#include "header.h"

void Header::read(){
	this->br->setBufferSize(header_size);
	 
	this->marker = this->br->getUint16();		 
	this->msg_type = this->br->getUint8();			
	this->sequence_id = this->br->getUint64(); 
	this->timestamp = this->br->getUint64();
	this->msg_direction = this->br->getUint8();
	this->msg_len = this->br->getUint16();
	op();
}

void Header::op(){
	switch(this->msg_type){
		case 1:
			OrderEntryMessage oem(this);

			break;
		case 2:
			OrderAckMessage oam(this);
			break;
		case 3:
			OrderFillMessage ofm(this);
			break;
		default:
			throw runtime_error("unknown message type");
	}
}