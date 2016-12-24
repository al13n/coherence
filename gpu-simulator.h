#ifndef __GPU_SIMULATOR_H__
#define __GPU_SIMULATOR_H__
#include "utility.h"

class gpu_simulator {
private:
	vector<UL> gpu_mem;
	set<UL> access;
	int mem_sz;
	int inserts;
	bool is_tag_present(const UL gpu_address, const UL tag)
	{
		return gpu_mem[gpu_address] == tag;
	}

	UL insert_tag(const UL gpu_address, const UL tag)
	{
		access.insert(__makeaddress_cache__(gpu_address, tag));
		inserts++;
		return gpu_mem[gpu_address] = tag;
	}
public:	
	gpu_simulator(){}
	gpu_simulator(int size): mem_sz(size), inserts(0)
	{
		gpu_mem.resize(size, 0);
	}
	
	void print()
	{	
		cout << "GPU total inserts: " << inserts << endl;
		cout << "GPU inserts(unique): " << access.size() << endl;
	}
	
	void _print_gpu_address_ranges()
	{
		cout << "Accessed address ranges(just TAG|GPU)\n-----------------------------------" << endl;
		UL start = *access.begin();
		UL end = start;
		int count = 0;
		for(auto it: access)
		{
			if (it != end+1)
			{
				cout << std::hex << start << " " << end << std::dec << endl;
				start = it;
				count++;
			}
			end = it;
		}
		cout << "Lines: " << count << endl;
	}

	bool address_exists(const UL);
	UL insert(const UL);
};

bool gpu_simulator::address_exists(const UL cpu_address)
{
	UL gpu_address = __getaddress_gpu__(cpu_address);
	UL gpu_tag = __gettag_gpu__(cpu_address);
	return is_tag_present(gpu_address, gpu_tag);
}

UL gpu_simulator::insert(const UL cpu_address)
{
	UL gpu_address = __getaddress_gpu__(cpu_address);
	UL gpu_tag = __gettag_gpu__(cpu_address);
	if(!is_tag_present(gpu_address, gpu_tag)){
		return insert_tag(gpu_address, gpu_tag);
	}
	return 0;
}
#endif
