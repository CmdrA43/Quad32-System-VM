#ifndef PORT_H
#define PORT_H

#include <cstdint>
#include <functional>

struct IOPort {
	uint32_t rx_queue[256] = {0};
	int rx_read = 0;
	int rx_write = 0;
	int rx_count = 0;
	
	uint32_t tx_queue[256] = {0};
	int tx_read = 0;
	int tx_write = 0;
	int tx_count = 0;
	
	std::function<bool(uint32_t)> tickToHardware = nullptr;
	std::function<bool(uint32_t&)> tickFromHardware = nullptr;
	
	void pushFromHost(uint32_t value){
		tx_queue[tx_write] = value;
		tx_write = (tx_write + 1) & 255;
		if(tx_count == 256){
			tx_read = (tx_read + 1) & 255;
		} else {
			tx_count++;
		}
	};
	
	bool pollToHost(uint32_t& value){
		if(rx_count == 0) return false;
		uint32_t val = rx_queue[rx_read];
		rx_read = (rx_read + 1) & 255;
		rx_count--;
		value = val;
		return true;
	};
	
	void tick(){
		if(tx_count > 0 && tickToHardware){
			uint32_t value = tx_queue[tx_read];
			if(tickToHardware(value)){
				tx_read = (tx_read + 1) & 255;
				tx_count--;
			}
		}
		
		if(rx_count < 256 && tickFromHardware){
			uint32_t input;
			if(tickFromHardware(input)){
				rx_queue[rx_write] = input;
				rx_write = (rx_write + 1) & 255;
				rx_count++;
			}
		}
	}
};

#endif
