#ifndef __DIR_SIMULTOR_H__
#define __DIR_SIMULATOR_H__
#include "utility.h"
#include "config.h"
#include <vector>
#include <algorithm>
#include <map>
#include <limits>
#include "patterns_structure.h"

#ifndef DIR_SIM
#define DIR_SIM

class dir_simulator {
private:
	quadleveldata exist_structure;
	quadleveldata dirty_structure;
	gpu_simulator *gpu;
	UL max_sz;
	UL sz;
	
public:
	dir_simulator(gpu_simulator* _gpu): max_sz{0}, sz{0} {
		gpu = _gpu;
	}
	
	UL get_max_size() {
		return max_sz;
	}
	
	UL size() {
		return exist_structure.size() + dirty_structure.size(); 
	}
	
	bool exists(const UL &cpu_address) {
		UL gpu_address = __getaddress_cache__(cpu_address);
		return exist_structure.find(gpu_address);
	}

	bool insert(const UL &cpu_address) {
		UL gpu_address = __getaddress_cache__(cpu_address);
		exist_structure.insert(gpu_address);	
		max_sz = max(size(), max_sz);
		return true;
	}
	
	bool remove(const UL &address, isGpu_address isgpuaddress) {
		UL gpu_address = address;
		if (!bool(isgpuaddress)) {
			gpu_address = __getaddress_cache__(address);
		}
		exist_structure.remove(gpu_address);
		dirty_structure.remove(gpu_address);	
		max_sz = max(size(), max_sz);
		return true;
	}
	
	bool inform_falsepositive_exists(const UL &cpu_address) {
		const UL gpu_address = __getaddress_cache__(cpu_address);
		exist_structure.false_positive(gpu_address);	
		max_sz = max(size(), max_sz);
		return true;
	}

	bool inform_falsepositive_dirty(const UL &cpu_address) {
		const UL gpu_address = __getaddress_cache__(cpu_address);
		dirty_structure.false_positive(gpu_address);	
		max_sz = max(size(), max_sz);
		return true;
	}
	
	bool isdirty(const UL cpu_address) {
		const UL gpu_address = __getaddress_cache__(cpu_address);
		return dirty_structure.find(gpu_address);
	}
	
	bool markdirty(const UL cpu_address) {
		const UL gpu_address = __getaddress_cache__(cpu_address);
		dirty_structure.insert(gpu_address);
		max_sz = max(size(), max_sz);
		return true;
	}
	
	bool removedirty(const UL cpu_address) {
		const UL gpu_address = __getaddress_cache__(cpu_address);
		dirty_structure.remove(gpu_address);
		max_sz = max(size(), max_sz);
		return true;
	}
	
	void print() {
		UL num = 0;
		cout << "###############################################################\n";
		exist_structure.print();
		cout << "###############################################################\n";
		dirty_structure.print();
		cout << "###############################################################\n";
	}
};
#endif

#endif
