#ifndef __GPU_SIMULATOR_H__
#define __GPU_SIMULATOR_H__
#include "utility.h"


class gpu_simulator {
private:	
	struct data {
		unsigned long int tag : 62;
		unsigned int present : 1;
		unsigned int isdirty : 1;
	};
	vector<data> gpu_mem;
	UL mem_sz;
public:	
	gpu_simulator(){
		gpu_mem.resize((1<<(GPU_ADDRESS_LEN+1)), {0, 0, 0});	
	}

	bool exists(const UL);
	bool isdirty(const UL);
	bool isreplace(const UL);
	bool resetdirtybit(const UL);
	bool setdirtybit(const UL);
	void remove(const UL);
	void insert(const UL);
	UL getaddress_replace(const UL);
	
};

bool gpu_simulator::exists(const UL cpu_address) {
	pair<UL, UL> _addresstag = __getaddresstagpair_gpu__(cpu_address);
	return gpu_mem[_addresstag.first].present && (gpu_mem[_addresstag.first].tag == _addresstag.second);
}

bool gpu_simulator::isdirty(const UL cpu_address) {
	pair<UL, UL> _addresstag = __getaddresstagpair_gpu__(cpu_address);
	return exists(cpu_address) && gpu_mem[_addresstag.first].isdirty;
}

bool gpu_simulator::isreplace(const UL cpu_address) {
	pair<UL, UL> _addresstag = __getaddresstagpair_gpu__(cpu_address);
	return (gpu_mem[_addresstag.first].present) && (gpu_mem[_addresstag.first].tag != _addresstag.second);
}

bool gpu_simulator::resetdirtybit(const UL cpu_address) {
	if (exists(cpu_address)) {
		pair<UL, UL> _addresstag = __getaddresstagpair_gpu__(cpu_address);
		gpu_mem[_addresstag.first].isdirty = 0;
		return true;
	}
	return false;
}

bool gpu_simulator::setdirtybit(const UL cpu_address) {
	if (exists(cpu_address)) {
		pair<UL, UL> _addresstag = __getaddresstagpair_gpu__(cpu_address);
		gpu_mem[_addresstag.first].isdirty = 1;
		return true;
	}
	return false;
}

void gpu_simulator::remove(const UL cpu_address) {
	pair<UL, UL> _addresstag = __getaddresstagpair_gpu__(cpu_address);
	gpu_mem[_addresstag.first] = {0, 0, 0};
}

void gpu_simulator::insert(const UL cpu_address) {
	if (exists(cpu_address)) {
		return;
	}
	pair<UL, UL> _addresstag = __getaddresstagpair_gpu__(cpu_address);
	gpu_mem[_addresstag.first] = {0, 0, 0};
	gpu_mem[_addresstag.first].tag = _addresstag.second;
	gpu_mem[_addresstag.first].present = 1;	
}

UL gpu_simulator::getaddress_replace(const UL cpu_address) {
	pair<UL, UL> _addresstag = __getaddresstagpair_gpu__(cpu_address);
	return __makeaddress_cache__(_addresstag.first, gpu_mem[_addresstag.first].tag);
}
#endif
