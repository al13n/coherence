#ifndef __GPU_SIMULATOR_H__
#define __GPU_SIMULATOR_H__
#include "utility.h"

class gpu_simulator {
	struct data {
		unsigned long int tag : 63;
		unsigned int isdirty : 1;
	};
private:
	vector<data> gpu_mem;
	UL mem_sz;
	bool is_tag_present(const UL gpu_address, const UL tag)
	{
		return gpu_mem[gpu_address].tag == tag;
	}

	UL insert_tag(const UL gpu_address, const UL tag)
	{
		if (!is_tag_present(gpu_address, tag))
			gpu_mem[gpu_address] = { tag, 0 };
		
		return tag;
	}

	bool mark_dirty(const UL gpu_address)
	{
		// Should it inform directory?
		return gpu_mem[gpu_address].isdirty = 1;	
	}
public:	
	gpu_simulator(){}
	gpu_simulator(UL size): mem_sz(size)
	{
		gpu_mem.resize(size, {0, 0});
	}

	bool address_exists(const UL);
	bool address_isdirty(const UL);
	void address_markclean(const UL);
	void address_remove(const UL);
	UL load(const UL);
	void store(const UL);
};

bool gpu_simulator::address_exists(const UL cpu_address)
{
	pair<UL, UL> _addresstag = __getaddresstagpair_gpu__(cpu_address);
	return is_tag_present(_addresstag.first, _addresstag.second);
}

bool gpu_simulator::address_isdirty(const UL cpu_address)
{
	pair<UL, UL> _addresstag = __getaddresstagpair_gpu__(cpu_address);
	if (address_exists(cpu_address))
		return gpu_mem[_addresstag.first].isdirty;
	return false;
}

void gpu_simulator::address_markclean(const UL cpu_address)
{
	pair<UL, UL> _addresstag = __getaddresstagpair_gpu__(cpu_address);
	if (address_exists(cpu_address) && address_isdirty(cpu_address))
		gpu_mem[_addresstag.first].isdirty = 0;
}

void gpu_simulator::address_remove(const UL cpu_address)
{
	pair<UL, UL> _addresstag = __getaddresstagpair_gpu__(cpu_address);
	if (address_exists(cpu_address))
		gpu_mem[_addresstag.first] = {0, 0};	
}

UL gpu_simulator::load(const UL cpu_address)
{
	pair<UL, UL> _addresstag = __getaddresstagpair_gpu__(cpu_address);
	return insert_tag(_addresstag.first, _addresstag.second);
}

void gpu_simulator::store(const UL cpu_address)
{
	pair<UL, UL> _addresstag = __getaddresstagpair_gpu__(cpu_address);
	insert_tag(_addresstag.first, _addresstag.second);
	mark_dirty(_addresstag.first);
}
#endif
