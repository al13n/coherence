#ifndef __DIR_SIMULTOR_H__
#define __DIR_SIMULATOR_H__
#include "utility.h"
#include "config.h"
#include <vector>
#include <algorithm>
#include <map>
#include <limits>
#include "patterns_structure.h"
#include "compartments_structure.h"

#ifndef DIR_SIM
#define DIR_SIM

class dir_simulator {
private:
	unordered_map< UL, shared_ptr<compartment> > c_p;
	shared_ptr<compartment> dir_mem;
	shared_ptr<compartment> tail_dir_mem;
	gpu_simulator *gpu;
	UL max_sz;
	UL sz;
	quadleveldata dirty_structure;

	UL getkey(const UL address) {
		return (address/SIZE_OF_COMPARTMENT);
	}

	bool deletefromlist(const UL &key) {
		if (c_p.find(key) != c_p.end()) {
			auto current = c_p[key];
			if (dir_mem == current) {
				dir_mem = current->previous;
			}
		
			if (tail_dir_mem == current) {
				tail_dir_mem = current->next;
			}
				
			if (current->previous != nullptr) {
				current->previous->next = current->next;
			}
			
			if (current->next != nullptr) {
				current->next->previous = current->previous;
			}
			
			current->previous = nullptr;
			current->next = nullptr;

			return true;
		}
		return false;
	}
	
	bool deletefrommap(const UL &key) {
		if (c_p.find(key) != c_p.end()) {
			c_p.erase(c_p.find(key));
			return true;
		}
		return false;
	}

	bool removefromlistandmovetohead(const UL &key) {
		if (c_p.find(key) != c_p.end()) {
			auto current = c_p[key];
			deletefromlist(key);
			if (dir_mem != nullptr && dir_mem != current) dir_mem->next = current;
			if (dir_mem != current) current->previous = dir_mem;
			current->next = nullptr;
			dir_mem = current;
			if (tail_dir_mem == nullptr) {
				tail_dir_mem = current;
			}
			return true;
		}
		return false;
	}
	
public:
	dir_simulator(gpu_simulator* _gpu): max_sz{0}, sz{0}, dir_mem{nullptr}, tail_dir_mem{nullptr} {
		gpu = _gpu;
	}
	
	UL numberoflines() {
		return c_p.size();
	}
	
	UL get_max_size() {
		return max_sz;
	}
	
	UL size() {
		return sz + c_p.size()*(sizeof(UL) + sizeof(compartment *)) + sizeof(tail_dir_mem) + sizeof(dir_mem);
	}
	
	bool exists(const UL &cpu_address) {
		UL gpu_address = __getaddress_cache__(cpu_address);
		bool flag = false;
		
		UL key = getkey(gpu_address);
		
		if (c_p.find(key) != c_p.end()) {
			flag = c_p[key]->exists(gpu_address);
			removefromlistandmovetohead(key);	
		}
		return flag;
	}

	bool insert(const UL &cpu_address) {
		UL gpu_address = __getaddress_cache__(cpu_address);
		bool flag = false;
		UL key = getkey(gpu_address);

		if (c_p.find(key) != c_p.end()) {
			UL prev_sz = c_p[key]->size();
			flag = c_p[key]->insert(gpu_address);
			sz += (c_p[key]->size() - prev_sz);
			removefromlistandmovetohead(key);
		} else {
			auto newcmp = std::make_shared<compartment> (gpu_address);
			c_p.insert(std::make_pair(key, newcmp));
			removefromlistandmovetohead(key);
			sz += newcmp->size();
			flag = true;
		}
		
		max_sz = max(size(), max_sz);
		return flag;
	}
	
	bool remove(const UL &address, isGpu_address isgpuaddress) {
		UL gpu_address = address;
		if (!bool(isgpuaddress)) {
			gpu_address = __getaddress_cache__(address);
		}
		bool flag = false;
		UL key = getkey(gpu_address);

		if (c_p.find(key) != c_p.end()) {
			UL prev_sz = c_p[key]->size();
			flag = c_p[key]->remove(gpu_address);
			sz += (c_p[key]->size() - prev_sz);
			if (c_p[key]->size() != 0) {
				removefromlistandmovetohead(key);
			} else {
				deletefromlist(key);
				deletefrommap(key);
			}
		}
		
		max_sz = max(size(), max_sz);
		return flag;
	}

	bool inform_falsepositive_exists(const UL &cpu_address) {
		const UL gpu_address = __getaddress_cache__(cpu_address);
		bool flag = false;
		const UL key = getkey(gpu_address);

		if (c_p.find(key) != c_p.end()) {
			UL prev_sz = c_p[key]->size();
			flag = c_p[key]->remove(gpu_address);
			sz += (c_p[key]->size() - prev_sz);
			if (c_p[key]->size() != 0) {
				removefromlistandmovetohead(key);
			} else {
				deletefromlist(key);
				deletefrommap(key);
			}
		}
		max_sz = max(size(), max_sz);
		return flag;
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
		UL total = 0;
		for (auto idx = tail_dir_mem; idx != nullptr; idx=idx->next) {
			num++;
			total += idx->getnumberofranges();
		}
		cout << "#################### ADDRESS EXISTS ###########################################\n";
		cout << "NUMBER OF LINES:\t" << num << endl;
		cout << "TOTAL NUMBER OF RANGES:\t" << total << endl;
		cout << "#################### ADDRESS DIRTY  ###########################################\n";
		dirty_structure.print();
		cout << "###############################################################################\n";
	}
};
#endif

#endif
