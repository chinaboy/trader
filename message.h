#pragma once

#include "header.h"

class Trade;
class Header;
class BytesReader;

class OrderEntryMessage: fix_size(37){
public:
	OrderEntryMessage(){}

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

class OrderAckMessage : fix_size(14){
public:
	OrderAckMessage(){}

	void init(Header * hdr);	

private:
	Header *hdr;
	uint32_t order_id;
	uint64_t client_id;
	uint8_t order_status;
	uint8_t reject_code;
	int fix_size;
};


class OrderFillMessage : fix_size(17){
public:
	OrderFillMessage(){}

	void init(Header* hdr);


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