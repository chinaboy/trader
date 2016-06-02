#include "header.h"

void Stats::creditTraders( OrderFillMessage &ofm){
	for(auto t:ofm.trades ){
		addQty(t.trader_tag, t.fill_qty);
	}
	addQty( getOriginalTrader(  ofm.getOrderId() ), ofm.fill_qty;);
}

string Stats::getOriginalTrader( uint32_t order_id){
	string s("");
	unordered_map< uint64_t, vector<OrderEntry> >::const_iterator oe = orders_map.find(order_id);
	if( oe != orders_map.end() ){
		s = oe.trader_tag;
	}
	return s;
}

string Stats::calculateActiveTrader(){
	string trader ;
	uint32_t max = 0;

	for ( auto it = trader_fills_map.begin(); it != trader_fills_map.end(); ++it ){
		if( max < it->second){
			max = it->second;
			trader = it->first;
		}
	}
	return trader;
}

void Stats::addQty(string trader, int fill_qty){
	if( trader.compare("") == 0){
		return;
	}
	unordered_map<string, int>::const_iterator got = trader_fills_map.find( trader );

	if( got == trader_fills_map.end() ){
		trader_fills_map[t] = fill_qty;
	}else{
		trader_fills_map[t] += fill_qty;
	}
}

void Header::read(){
	//this->marker = this->br->getUint16();

	if( ! this->br->readBuffer( header_size ) ){
		setEof();
		return ;
	}

	char s = this->br->getUint8();
	char t = this->br->getUint8();
	assert( s=='S' && t=='T' );
		
	this->msg_type = this->br->getUint8();			
	this->sequence_id = this->br->getUint64(); 
	this->timestamp = this->br->getUint64();
	this->msg_direction = this->br->getUint8();

	assert( this->msg_direction == 0 || this->msg_direction == 1);
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

				OrderEntry oe(sequence_id + 1, this->client_assigned_id, this->trader_tag, this->instrument, this->qty, 0);
				stats.filled_orders_map[ oem.client_assigned_id ] = std::move(oe) ;
			}
			break;
		case 2:
			{
				OrderAckMessage oam;
				oam.init(this);
				//oam.printOAM();
				stats.incrementOAM();
				// look into 
				unordered_map<string, vector<OrderEntry>>::const_iterator got = stats.filled_orders_map.find( oam.client_id ) ;

				if( got != stats.filled_order_map.end() ){
					vector<OrderEntry> oe = stats.filled_order_map[oam.client_id];
					for( auto entry:oe ){
						if( entry.sequence_id == sequence_id ){
							entry.order_id = oam.order_id;
							entry.setAcked();
							OrderEntry goodOrder = entry;
							stats.orders_map[oam.order_id] = std::move( goodOrder );
							break;
						}
					}
				}
				break;
			}
		case 3:
			{
				OrderFillMessage ofm;
				ofm.init(this);
				//ofm.printOFM();
				stats.incrementOFM();
				stats.creditTraders(ofm);
				break;
			}
		default:
			string error_str = "unknown message type " + ( this->msg_type );
			cout << error_str << endl;
			throw runtime_error(error_str);
			break;
	}
}

void OrderEntryMessage::init(Header * hdr){   // exclude variable firm string and termination; max 255 and 8
	//this.hdr = std::move(hdr);
	this->hdr = hdr;
	BytesReader *br = hdr->getBytesReader();

 
	if( !br->readBuffer( hdr->getMsgLen() ) ){
		hdr->setEof();
		return;
	}
	
	this->price = br->getUint64();
	this->qty = br->getUint32();
	// copy 10 chars to instrument
	this->instrument = br->getChars(10);

	this->side = br->getUint8();
	this->client_assigned_id = br->getUint64();
	this->time_in_force = br->getUint8();

	// copy 3 chars to trader tag
	vector<char> cv = br->getChars(3);
	this->trader_tag = string(cv.begin(), cv.end());
	this->firm_id = br->getUint8();

	// read until termination string
	int remain = (int)(hdr->getMsgLen() ) - fix_size - 8; // minus fix size fields and termination string
	this->firm = br->getChars(remain);


	br->consumeTermination();
}

void OrderAckMessage::init(Header * hdr){
	this->hdr = hdr;
	BytesReader *br = hdr->getBytesReader();

	 
	if( !br->readBuffer( hdr->getMsgLen() )){
		hdr->setEof();
		return;
	}
	
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

 
	if( !br->readBuffer( hdr->getMsgLen() ) ){
		hdr->setEof();
		return ;
	}

	this->order_id = br->getUint32();
	this->fill_price = br->getUint64();
	this->fill_qty = br->getUint32();
	this->no_of_contras = br->getUint8();
	this->trades = br->getTrades( this->no_of_contras );
	br->consumeTermination();
}

int parseStream(string stream){	
	Header hdr(stream);
	while( hdr.next() ){
		hdr.read();
		//break;
	}
	hdr.printStats();
	return 1;
}

int main(int argc, char* argv[]){
	if(argc > 1){
		//cout << argv[1] << endl;
		string file_stream(argv[1]);
		parseStream(file_stream);

	}
	return 0;
}
