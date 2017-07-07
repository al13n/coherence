#ifndef __DIR_SIMULTOR_H__
#define __DIR_SIMULATOR_H__
#include "utility.h"
#include "config.h"
#include <vector>
#include <algorithm>
#include <map>
#include <limits>
UL getregion(const UL &address) {
	return (address/SIZE_OF_COMPARTMENT);
}

int getoffset(const UL &address) {
	return address%SIZE_OF_COMPARTMENT;
}

int getdenseindex(const UL &address) {
	const int _half = SIZE_OF_COMPARTMENT >> 1;
	if (getoffset(address) < _half) {
		return 1;
	}
	return 2;
}

UL getpatternfromidx(const UL &idx) {
	UL pattern = ((UL)1 << SIZE_OF_COMPARTMENT) - 1;
	if (idx != 3) {
		UL remove = ((UL)1 << (SIZE_OF_COMPARTMENT >> 1)) - 1;
		if (idx == 1) {
			remove <<= (SIZE_OF_COMPARTMENT >> 1);
		}
		pattern ^= remove;
	}
	return pattern;
}

class existdata {
private:
	set <UL> patterns[4];
	unordered_map <UL, UL> others;
public:
	bool find(UL address) {
		UL key = getregion(address);
		int idx = getdenseindex(address);
		if ( (patterns[idx].find(key) != patterns[idx].end()) || (patterns[3].find(key) != patterns[3].end())) {
			return true;
		}
		if (others.find(key) != others.end()) {
			return others[key] & ((UL)1 << (getoffset(address)));
		}
		return false;
	}

	bool insert(UL address) {
		if (find(address)) {
			return true;
		}
		UL key = getregion(address);
		int idx = getdenseindex(address);
		int _idx = (idx == 1 ? 2 : 1);
		if (patterns[_idx].find(key) != patterns[_idx].end()) {
			patterns[_idx].erase(patterns[_idx].find(key));
			patterns[3].insert(key);
			return true;
		}
	
		if (others.find(key) != others.end()) {
			others[key] |= ((UL)1 << (getoffset(address)));
			return true;	
		}
		
		patterns[idx].insert(key);
		return true;
	}

	bool remove(UL address) {
		UL key = getregion(address);
		int idx = getdenseindex(address);
		
		if (others.find(key) != others.end()) {
			if (others[key] & ((UL)1 << (getoffset(address)))) {
				others[key] ^= ((UL)1 << (getoffset(address)));
				if (others[key] == 0) {
					others.erase(others.find(key));
				}
			}
			return true;
		}
	
		if ((patterns[idx].find(key) != patterns[idx].end()) || (patterns[3].find(key) != patterns[3].end())) {
			int foundidx = (patterns[idx].find(key) != patterns[idx].end()) ? idx : 3;
			patterns[foundidx].erase(patterns[foundidx].find(key));
			UL value = getpatternfromidx(foundidx);
			value ^= ((UL)1 << (getoffset(address)));
			others.insert(make_pair(key, value));
			return true;
		}
		
		return false;
	}
	
	bool false_positive(UL address) {
		return remove(address);
	}

	UL size() {
		UL total_sz = 0;
		for(int i = 1; i < 4; i++) {
			total_sz += patterns[i].size()*sizeof(UL);
		}
		return total_sz + others.size()*(sizeof(UL) + sizeof(UL));
	}
	
	void print() {
	}
};

class dirtydata {
private:
	set <UL> patterns;
	unordered_map <UL, UL> others;
public:
	bool find(UL address) {
		UL key = getregion(address);
		if ( (patterns.find(key) != patterns.end()) ) {
			return true;
		}
		if (others.find(key) != others.end()) {
			return others[key] & ((UL)1 << (getoffset(address)));
		}
		return false;
	}

	bool insert(UL address) {
		if (find(address)) {
			return true;
		}
		UL key = getregion(address);
		if (others.find(key) != others.end()) {
			others[key] |= ((UL)1 << (getoffset(address)));
			return true;
		}
		
		patterns.insert(key);
		return true;
	}

	bool remove(UL address) {
		UL key = getregion(address);
		int idx = getdenseindex(address);
		
		if (others.find(key) != others.end()) {
			if (others[key] & ((UL)1 << (getoffset(address)))) {
				others[key] ^= ((UL)1 << (getoffset(address)));
				if (others[key] == 0) {
					others.erase(others.find(key));
				}
			}
			return true;
		}
	
		if (patterns.find(key) != patterns.end()) {
			patterns.erase(patterns.find(key));
			UL value = getpatternfromidx(3);
			value ^= ((UL)1 << (getoffset(address)));
			others.insert(make_pair(key, value));
			return true;
		}
		
		return false;
	}
	
	bool false_positive(UL address) {
		return remove(address);
	}

	UL size() {
		return patterns.size()*sizeof(UL) + others.size()*(sizeof(UL) + sizeof(UL));
	}
	
	void print() {
	}
};

class dir_simulator {
private:
	existdata exist_structure;
	dirtydata dirty_structure;
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
