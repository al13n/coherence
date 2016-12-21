#ifndef __UTILITY_H_
#define __UTILITY_H_

#include <bits/stdc++.h>
using namespace std;

typedef unsigned long int UL;
static const int GPU_OFFSET_LEN = 6;
static const int GPU_ADDRESS_LEN = 20;

inline UL __getaddress_gpu__(const UL cpu_address)
{
	return (cpu_address & (1<<(GPU_ADDRESS_LEN + GPU_OFFSET_LEN) - 1)) >> GPU_OFFSET_LEN;
}

inline UL __gettag_gpu__(const UL cpu_address)
{
	return (cpu_address >> (GPU_ADDRESS_LEN + GPU_OFFSET_LEN));
}

inline UL __getaddress_cache__(const UL cpu_address)
{
	return (cpu_address >> GPU_OFFSET_LEN);
}

class gpu_simulator {
private:
	vector<UL> gpu_mem;
	int mem_sz;
public:
	gpu_simulator(){}
	gpu_simulator(int size): mem_sz(size)
	{
		gpu_mem.resize(size, 0);
	}

	bool is_tag_present(const UL gpu_address, const UL tag)
	{
		return gpu_mem[gpu_address] == tag;
	}

	bool insert_tag(const UL gpu_address, const UL tag)
	{
		return gpu_mem[gpu_address] = tag;
	}
	
	bool address_exists(const UL);
};

bool gpu_simulator::address_exists(const UL cpu_address)
{
	UL gpu_address = __getaddress_gpu__(cpu_address);
	UL gpu_tag = __gettag_gpu__(cpu_address);
	return is_tag_present(gpu_address, gpu_tag);
}


class cache_simulator {
private:
	set<UL> cache_mem;
public:
	cache_simulator(){}
	
	bool exists(const UL cpu_address)
	{	
		return cache_mem.find(__getaddress_cache__(cpu_address)) != cache_mem.end();
	}
	
	bool insert(const UL cpu_address)
	{
		return cache_mem.insert(__getaddress_cache__(cpu_address)).second;
	}
};
#endif
