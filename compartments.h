#ifndef __DIR_SIMULTOR_H__
#define __DIR_SIMULATOR_H__
#include "utility.h"
#include "config.h"
#include <vector>
#include <algorithm>
#include <map>
#include <limits>

class dir_simulator {
private:
	class compartment {
	public:
		std::vector<rangedata> address_exist;
		std::vector<rangedata> address_dirty;
		
		shared_ptr<compartment> previous;
		shared_ptr<compartment> next;
	
		~compartment() {
			previous = next = nullptr;
			address_exist.clear();
			address_dirty.clear();
		}
			
		UL size() {
			if (address_exist.empty() && address_dirty.empty())	
				return 0;
			UL total_size = 0;
			total_size += address_exist.size() + address_dirty.size();
			return total_size*sizeof(rangedata) + sizeof(previous) + sizeof(next);
		}

		bool existsinvector(vector<rangedata> & vec, const UL &check_address) {
                        for (auto &idx: vec) {
                                if (idx.findinrange(check_address)) {
                                        return true;
                                }
                        }
                        return false;
                }
		
		bool insertinvector(vector<rangedata> &vec, const UL &address, bool isaccurate) {
			UL closeness = CLOSEST_RANGE_INSERT_LIMIT + 1;
			int pos = -1;
			
			for (int i = 0; i < vec.size(); i++) {
				if (vec[i].addtorange(address)) {
					if (i > 0 
					&& ( (vec[i-1].end == vec[i].start) || ( (vec[i-1].end+1) == vec[i].start ) )
					) {
						vec[i-1].end = vec[i].end;
						vec.erase(vec.begin() + i);
					} else {
						if (i+1 < vec.size() 
						&& ( (vec[i+1].start == vec[i].end) || ( vec[i+1].start == (vec[i].end+1) ) )
						) {
							vec[i].end = vec[i+1].end;
							vec.erase(vec.begin() + i+1);
						}
					}
					return true;
				} else {
					if (!isaccurate) {
						UL dist1 = abs(vec[i].start - address);
						UL dist2 = abs(vec[i].end - address);
						UL mn = min(mn,  min(dist1, dist2));
						if (closeness > mn) {
							closeness = mn;
							pos = i;
						}
					}
				}
			}
		
			if (!isaccurate && pos != -1 && closeness <= CLOSEST_RANGE_INSERT_LIMIT) {
				vec[pos].addoutlier(address);
				return true;
			} else {
				// add point range
				rangedata newrange(address, address);
				vec.push_back(newrange);
				std::sort(vec.begin(), vec.end());
				return true;
			}
			return false;
		}

		bool removefromvector(vector<rangedata> &vec, const UL &address) {
			int pos = -1;
			for (int i = 0; i < vec.size(); i++) {
				if (vec[i].findinrange(address)) {
					pos = i;
					break;
				}
			}	

			if (pos != -1) {
				if (vec[pos].start == address) {
					vec[pos].start = address+1;
					if (vec[pos].start > vec[pos].end) {
						vec.erase(vec.begin() + pos);
					}
				}
				else if (vec[pos].end == address) {
					vec[pos].end = address - 1;
					if (vec[pos].start > vec[pos].end) {
						vec.erase(vec.begin() + pos);
					}
				}
				else {
					UL s = vec[pos].start;
					UL e = vec[pos].end;
					vec.erase(vec.begin() + pos);
					if (address > s) {
						rangedata newrange(s, address-1);
						vec.push_back(newrange);
					}

					if(address < e) {
						rangedata newrange(address+1, e);
						vec.push_back(newrange);
					}	
					std::sort(vec.begin(), vec.end());
				}
				return true;
			}
			return false;
		}

		compartment(UL gpu_address): previous{nullptr}, next{nullptr} {
			address_exist.clear();
			address_dirty.clear();
			insertinvector(address_exist, gpu_address, IS_ACCURATE_ADDRESS_EXIST);
		}
		
		bool exists(const UL &address) {
			return existsinvector(address_exist, address);
		}
			
		bool insert(const UL &address) {
			return insertinvector(address_exist, address, IS_ACCURATE_ADDRESS_EXIST);
		}
	
		bool remove(const UL &address) {
			removefromvector(address_exist, address);
			removefromvector(address_dirty, address);
			return true;
		}
			
		bool removedirty(const UL &address) {
			return removefromvector(address_dirty, address);
		}
		
		bool markdirty(const UL &address) {
			if (existsinvector(address_exist, address)) {
				return insertinvector(address_dirty, address, IS_ACCURATE_ADDRESS_DIRTY); 
			}
			return false;
		}
		
		bool isdirty(const UL &address) {
			if (existsinvector(address_exist, address)) {
				return existsinvector(address_dirty, address);
			}
			return false;
		}
		
		UL getkey() {
			if (!address_exist.empty()) {
				return (address_exist[0].start)/SIZE_OF_COMPARTMENT;
			}
			return NAN;
		}
		
		UL getcoverage() {
			struct coverage {
				UL sum = 0;
				coverage(): sum{0} {}
				void operator() (rangedata r) { sum += r.getcoverage(); }
			};
			
			coverage res = for_each (address_exist.begin(), address_exist.end(), coverage());
			return res.sum;
		}
		
		void print() {
			cout << "---------------------------------------------------------\n";
			cout << "ADDRESS EXISTS:\n";
			for (auto idx: address_exist) {
				idx.print();
			}
			cout << "ADDRESS DIRTY:\n";
			for (auto idx: address_dirty) {
				idx.print();
			}
			cout << "---------------------------------------------------------\n";
		}
	};
	
	map<UL, shared_ptr<compartment> > c_p;
	shared_ptr<compartment> dir_mem;
	shared_ptr<compartment> tail_dir_mem;
	gpu_simulator *gpu;
	UL max_sz;
	UL sz;
	UL total_range_coverage;
	UL max_range_coverage;
	UL entry_max_range_coverage;
	UL exist_access_count;
	
	UL getkey(const UL address) {
		return (address/SIZE_OF_COMPARTMENT);
	}

	bool deletefromlist(UL &key) {
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
	
	bool deletefrommap(UL &key) {
		if (c_p.find(key) != c_p.end()) {
			c_p.erase(c_p.find(key));
			return true;
		}
		return false;
	}

	bool removefromlistandmovetohead(UL &key) {
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
	
	bool shouldpurge() {
		while (size() > (UL)(MAX_SIZE_DIRECTORY*1024)) {
			if (tail_dir_mem != nullptr) {
				if (dir_mem == tail_dir_mem) dir_mem = nullptr;
				if (!tail_dir_mem->address_exist.empty()) {
					gpu->inform_clear(tail_dir_mem->address_exist);
					UL key = tail_dir_mem->getkey();
					auto current = c_p[key];
					deletefromlist(key);
					deletefrommap(key);
					sz -= current->size();
					current.reset();
				} else {
					auto current = tail_dir_mem;
					sz -= current->size();
					tail_dir_mem = current->next;
					current.reset();
				}
			}
			return true;
		}
		return false;
	}
	
public:
	dir_simulator(gpu_simulator* _gpu): max_sz{0}, sz{0}, dir_mem{nullptr}, tail_dir_mem{nullptr}, total_range_coverage{0}, max_range_coverage{0}, entry_max_range_coverage{0}, exist_access_count{0} {
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
	
	void rangecoverage() {
		exist_access_count++;
		UL range_coverage = 0;
                for (auto idx = tail_dir_mem; idx != nullptr; idx=idx->next) {
               		UL temp_range_coverage = idx->getcoverage();
			entry_max_range_coverage = max(entry_max_range_coverage, temp_range_coverage);
			range_coverage += temp_range_coverage;
                }
		max_range_coverage = max(max_range_coverage, range_coverage);
		total_range_coverage += range_coverage;
	}

	double get_avg_rangecoverage() {
		return (double)((double)(total_range_coverage*1.0)/exist_access_count);
	}
	
	UL get_max_rangecoverage() {
		return max_range_coverage;
	}
	
	UL get_entry_max_rangecoverage() {
		return entry_max_range_coverage;
	}

	bool exists(const UL cpu_address) {
		UL gpu_address = __getaddress_cache__(cpu_address);
		bool flag = false;
		
		UL key = getkey(gpu_address);
		
		if (c_p.find(key) != c_p.end()) {
			flag = c_p[key]->exists(gpu_address);
			removefromlistandmovetohead(key);	
		}
		
		rangecoverage();
		return flag;
	}

	bool insert(const UL cpu_address) {
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
		shouldpurge();
		return flag;
	}

	bool remove(const UL address, bool isgpuaddress) {
		UL gpu_address = address;
		if (!isgpuaddress) {
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
		shouldpurge();
		return flag;
	}

	bool inform_falsepositive_dirty(UL cpu_address) {
		UL gpu_address = __getaddress_cache__(cpu_address);
		bool flag = false;
		UL key = getkey(gpu_address);

		if (c_p.find(key) != c_p.end()) {
			UL prev_sz = c_p[key]->size();
			flag = c_p[key]->removedirty(gpu_address);
			sz += (c_p[key]->size() - prev_sz);
			if (c_p[key]->size() != 0) {
				removefromlistandmovetohead(key);
			} else {
				deletefromlist(key);
				deletefrommap(key);
			}
		}
		max_sz = max(size(), max_sz);
		shouldpurge();
		
		return flag;
	}
	
	bool inform_falsepositive_exists(UL cpu_address) {
		UL gpu_address = __getaddress_cache__(cpu_address);
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
		shouldpurge();
		
		return flag;
	}

	bool isdirty(UL cpu_address) {
		UL gpu_address = __getaddress_cache__(cpu_address);
		bool flag = false;
		UL key = getkey(gpu_address);

		if (c_p.find(key) != c_p.end()) {
			flag = c_p[key]->isdirty(gpu_address);
			removefromlistandmovetohead(key);
		}
		rangecoverage();
		return flag;
	}
	
	bool markdirty(UL cpu_address) {
		UL gpu_address = __getaddress_cache__(cpu_address);
		bool flag = false;	
		UL key = getkey(gpu_address);

		if (c_p.find(key) != c_p.end()) {
			UL prev_sz = c_p[key]->size();
			flag = c_p[key]->markdirty(gpu_address);
			sz += (c_p[key]->size() - prev_sz);
			removefromlistandmovetohead(key);
		} else {
			auto newcmp = std::make_shared<compartment> (gpu_address);
			newcmp->markdirty(gpu_address);
			c_p.insert(std::make_pair(key, newcmp));
			sz += newcmp->size();
			removefromlistandmovetohead(key);
			flag = true;
		}
		max_sz = max(size(), max_sz);
		shouldpurge();
	
		return flag;
	}
	
	bool removedirty(UL cpu_address) {
		UL gpu_address = __getaddress_cache__(cpu_address);
		bool flag = false;	
		UL key = getkey(gpu_address);

		if (c_p.find(key) != c_p.end()) {
			UL prev_sz = c_p[key]->size();
			flag = c_p[key]->removedirty(gpu_address);
			sz += (c_p[key]->size() - prev_sz);
			if (c_p[key]->size() != 0) {
				removefromlistandmovetohead(key);
			} else {
				deletefromlist(key);
				deletefrommap(key);
			}
		}
		max_sz = max(size(), max_sz);
		shouldpurge();
		return flag;
	}
	
	void print() {
		UL num = 0;
		for (auto idx = tail_dir_mem; idx != nullptr; idx=idx->next) {
			cout << "###############################################################\n";
			cout << "POSITION:\t" << num++ << endl;
			idx->print();
			cout << "RANGE COVERAGE: " << idx->getcoverage() << endl;
			cout << "###############################################################\n";
		}
	}
};

#endif
