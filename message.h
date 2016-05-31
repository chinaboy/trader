#pragma once

#include "trade.h"

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

		this->side = br->getUint8();
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
	OrderAckMessage(Header * hdr) : fix_size(14){
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
	OrderFillMessage(Header * hdr) : fix_size(17){
		this->hdr = hdr;
		BytesReader *br = hdr->getBytesReader();
		br->setBufferSize(fix_size);

		this->order_id = br->getUint32();
		this->fill_price = br->getUint64();
		this->fill_qty = br->getUint32();
		this->no_of_contras = br->getUint8();
		this->trades = br->getTrades();
	}

	~OrderFillMessage(){
		
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