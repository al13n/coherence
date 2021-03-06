#ifndef __DIR_SIMULTOR_H__
#define __DIR_SIMULATOR_H__
#include "utility.h"
#include "config.h"
#include <vector>
#include <algorithm>
#include <map>
#include <limits>

vector < pair<UL, bitset<SIZE_OF_COMPARTMENT> > > patterns;
vector < pair<UL, bitset<2*SIZE_OF_COMPARTMENT> > > creation_eviction_pattern;


unordered_map< bitset<SIZE_OF_COMPARTMENT>, UL > pattern_exist;
unordered_map< bitset<SIZE_OF_COMPARTMENT>, UL > pattern_dirty;
unordered_map< bitset<2*SIZE_OF_COMPARTMENT>, UL > creation_eviction_exist;
unordered_map< bitset<2*SIZE_OF_COMPARTMENT>, UL > creation_eviction_dirty;

class dir_simulator {
private:
	class compartment {
	private:
		UL _rangecover;
	public:
		std::vector<rangedata> address_exist;
		std::vector<rangedata> address_dirty;
		
		shared_ptr<compartment> previous;
		shared_ptr<compartment> next;
		
		bool captured_creation_pattern_dirty;
		bitset<SIZE_OF_COMPARTMENT> creation_pattern_exists;
		bitset<SIZE_OF_COMPARTMENT> creation_pattern_dirty;
		bitset<SIZE_OF_COMPARTMENT> eviction_pattern_exists;
		bitset<SIZE_OF_COMPARTMENT> eviction_pattern_dirty;

		void getpattern(const vector<rangedata> &address_list, bitset<SIZE_OF_COMPARTMENT> &pattern) {
			for (int idx = 0; idx < address_list.size(); idx++) {
				const auto &r = address_list[idx];
				for (UL address = r.start; address <= r.end; address++) {
					UL offset = address%SIZE_OF_COMPARTMENT;
					pattern[offset] = 1;
				}
			}
		}
		
		~compartment() {
			previous = next = nullptr;
			if (GET_PATTERN_ON_EVICTION) {
				getpattern(address_exist, eviction_pattern_exists);
				getpattern(address_dirty, eviction_pattern_dirty);
			}
			address_exist.clear();
			address_dirty.clear();
			if (GET_PATTERN_ON_CREATION && GET_PATTERN_ON_EVICTION) {
				bitset <2*SIZE_OF_COMPARTMENT> exist_p;
				bitset <2*SIZE_OF_COMPARTMENT> dirty_p;
				
				for (int idx = 0; idx < SIZE_OF_COMPARTMENT; idx++) {
					exist_p[idx] = creation_pattern_exists[idx];
					dirty_p[idx] = creation_pattern_dirty[idx];
				}
				
				for (int idx = 0; idx < SIZE_OF_COMPARTMENT; idx++) {
					exist_p[idx + SIZE_OF_COMPARTMENT] = eviction_pattern_exists[idx];
					dirty_p[idx + SIZE_OF_COMPARTMENT] = eviction_pattern_dirty[idx];
				}
			
				if (creation_eviction_exist.find(exist_p) != creation_eviction_exist.end()) {
					creation_eviction_exist[exist_p]++;
				} else {
					creation_eviction_exist.insert(std::make_pair(exist_p, 1));
				}
				
				if (creation_eviction_dirty.find(dirty_p) != creation_eviction_dirty.end()) {
					creation_eviction_dirty[dirty_p]++;
				} else {
					creation_eviction_dirty.insert(std::make_pair(dirty_p, 1));
				}
			}
		}
			
		compartment(UL gpu_address): previous{nullptr}, next{nullptr}{
			address_exist.clear();
			address_dirty.clear();
			captured_creation_pattern_dirty = false;
			_rangecover = 0;
			insert(gpu_address);
			if (GET_PATTERN_ON_CREATION) {
				getpattern(address_exist, creation_pattern_exists);
			}
		}
		
		UL size() {
			if (address_exist.empty() && address_dirty.empty())	
				return 0;
			UL total_size = 0;
			total_size += address_exist.size() + address_dirty.size();
			return total_size*sizeof(rangedata) + sizeof(previous) + sizeof(next);
		}

		bool existsinvector(vector<rangedata> & vec, const UL &check_address) {
			int start = 0;
			int end = vec.size() - 1;
			while (start <= end) {
				int mid = (start+end)/2;
				if (vec[mid].findinrange(check_address)) {
					return true;
				}
				if (vec[mid].end > check_address) {
					end = mid - 1;
				} else {
					start = mid + 1;
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

		bool exists(const UL &address) {
			return existsinvector(address_exist, address);
		}
			
		bool insert(const UL &address) {
			if (!exists(address)) _rangecover++;
			return insertinvector(address_exist, address, IS_ACCURATE_ADDRESS_EXIST);
		}
	
		bool remove(const UL &address) {
			if (exists(address)) _rangecover--;
			removefromvector(address_exist, address);
			removefromvector(address_dirty, address);
			return true;
		}
			
		bool removedirty(const UL &address) {
			return removefromvector(address_dirty, address);
		}
		
		bool markdirty(const UL &address) {
			bool flag = false;
			if (existsinvector(address_exist, address)) {
				flag = insertinvector(address_dirty, address, IS_ACCURATE_ADDRESS_DIRTY); 
			}
			
			if (flag && GET_PATTERN_ON_CREATION && !captured_creation_pattern_dirty) {
				captured_creation_pattern_dirty = true;
				getpattern(address_dirty, creation_pattern_dirty);
			}
			
			return flag;
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
			/*
			struct coverage {
				UL sum = 0;
				coverage(): sum{0} {}
				void operator() (rangedata r) { sum += r.getcoverage(); }
			};
			
			coverage res = for_each (address_exist.begin(), address_exist.end(), coverage());
			return res.sum;
			*/
			return _rangecover;
		}
		
		UL reduce(const UL &req_reduction_size) {
			if (address_exist.size() <= 1 && address_dirty.size() <= 1) {
				return 0;
			}
			UL end;
			UL size_reduced = 0;
			while (req_reduction_size > size_reduced && address_exist.size() > 1) {
				_rangecover -= address_exist[ address_exist.size() - 1 ].getcoverage();
				_rangecover -= address_exist[ address_exist.size() - 2 ].getcoverage();
				end = address_exist[ address_exist.size() - 1 ].end;
				address_exist.pop_back();
				size_reduced += sizeof(rangedata);
				address_exist[ address_exist.size() - 1 ].end = end;
				_rangecover += address_exist[ address_exist.size() - 1 ].getcoverage();
			}
			
			while (req_reduction_size > size_reduced && address_dirty.size() > 1) {
				end = address_dirty[ address_dirty.size() - 1 ].end;
				address_dirty.pop_back();
				size_reduced += sizeof(rangedata);
				address_dirty[ address_dirty.size() - 1 ].end = end;
			}

			return size_reduced;
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
	
	unordered_map< UL, shared_ptr<compartment> > c_p;
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
	
	bool shouldpurge() {
		while (size() > (UL)(MAX_SIZE_DIRECTORY*1024)) {
			UL req_size_reduction = size() - (UL)(MAX_SIZE_DIRECTORY*1024);

			bool should_evict = true;
			auto tmp_ptr = tail_dir_mem;
			UL prev_sz;
			UL total_decrement_size = 0;

			for (int idx = 0; idx < ((c_p.size()*MAX_PERCENT_REDUCE_BEFORE_EVICT)/100) && tmp_ptr != nullptr; idx++) {
				UL reduced_size = tmp_ptr->reduce(req_size_reduction);
				req_size_reduction -= reduced_size;
				total_decrement_size += reduced_size;
				if (req_size_reduction <= 0) {
					should_evict = false;
					break;
				}
				tmp_ptr = tmp_ptr->next;
			}
			
			sz -= total_decrement_size;
			
			if (should_evict && tail_dir_mem != nullptr) {
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
	
	void getpattern(const vector<rangedata> &address_list, bitset<SIZE_OF_COMPARTMENT> &pattern) {
		for (int idx = 0; idx < address_list.size(); idx++) {
			const auto &r = address_list[idx];
			for (UL address = r.start; address <= r.end; address++) {
				UL offset = address%SIZE_OF_COMPARTMENT;
				pattern[offset] = 1;
			}
		}
	}
	
	void getexistpattern(const UL &key) {
		bitset<SIZE_OF_COMPARTMENT> pattern;

		if (c_p.find(key) != c_p.end()) {
			for (int idx = 0; idx < c_p[key]->address_exist.size(); idx++) {
				const auto &r = c_p[key]->address_exist[idx];
				for (UL address = r.start; address <= r.end; address++) {
					UL offset = address%SIZE_OF_COMPARTMENT;
					pattern[offset] = 1;
				}
			}
			
			if (pattern_exist.find(pattern) != pattern_exist.end()) {
				pattern_exist[pattern]++;
			} else {
				pattern_exist.insert(std::make_pair(pattern, 1));
			}
		
		}
	}
	
	void getdirtypattern(const UL& key) {
		bitset<SIZE_OF_COMPARTMENT> pattern;

		if (c_p.find(key) != c_p.end()) {
			for (int idx = 0; idx < c_p[key]->address_dirty.size(); idx++) {
				const auto &r = c_p[key]->address_dirty[idx];
				for (UL address = r.start; address <= r.end; address++) {
					UL offset = address%SIZE_OF_COMPARTMENT;
					pattern[offset] = 1;
				}
			}
		
			if (pattern_dirty.find(pattern) != pattern_dirty.end()) {
				pattern_dirty[pattern]++;
			} else {
				pattern_dirty.insert(std::make_pair(pattern, 1));
			}
			
		}
	}
	
	bool exists(const UL &cpu_address) {
		UL gpu_address = __getaddress_cache__(cpu_address);
		bool flag = false;
		
		UL key = getkey(gpu_address);
		
		if (c_p.find(key) != c_p.end()) {
			flag = c_p[key]->exists(gpu_address);
			removefromlistandmovetohead(key);	
		}
		if (GET_PATTERN_ON_QUERY) getexistpattern(key);	
		rangecoverage();
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
		if (GET_PATTERN_ON_UPDATE) {
			getexistpattern(key);
		}
		
		shouldpurge();
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
		
		if (GET_PATTERN_ON_UPDATE) {
			getexistpattern(key);
			getdirtypattern(key);
		}
		
		shouldpurge();
		return flag;
	}

	bool inform_falsepositive_dirty(const UL &cpu_address) {
		const UL gpu_address = __getaddress_cache__(cpu_address);
		bool flag = false;
		const UL key = getkey(gpu_address);

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
		
		if (GET_PATTERN_ON_UPDATE) {
			getdirtypattern(key);
		}
		
		shouldpurge();
		
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
		
		if (GET_PATTERN_ON_UPDATE) {
			getexistpattern(key);
		}
		
		shouldpurge();
		
		return flag;
	}

	bool isdirty(const UL cpu_address) {
		const UL gpu_address = __getaddress_cache__(cpu_address);
		bool flag = false;
		const UL key = getkey(gpu_address);

		if (c_p.find(key) != c_p.end()) {
			flag = c_p[key]->isdirty(gpu_address);
			removefromlistandmovetohead(key);
		}
		
		if (GET_PATTERN_ON_QUERY) {
			getdirtypattern(key);
		}
			
		rangecoverage();
		return flag;
	}
	
	bool markdirty(const UL cpu_address) {
		const UL gpu_address = __getaddress_cache__(cpu_address);
		bool flag = false;	
		const UL key = getkey(gpu_address);

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
		if (GET_PATTERN_ON_UPDATE) {
			getdirtypattern(key);
		}
		shouldpurge();
		return flag;
	}
	
	bool removedirty(const UL cpu_address) {
		const UL gpu_address = __getaddress_cache__(cpu_address);
		bool flag = false;	
		const UL key = getkey(gpu_address);

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
		if (GET_PATTERN_ON_UPDATE) {
			getdirtypattern(key);
		}
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
	
	void printpatterns() {
		struct {	
		bool operator ()(const pair<UL, bitset<SIZE_OF_COMPARTMENT> > &a, const pair<UL, bitset<SIZE_OF_COMPARTMENT> > &b) {
			return a.first <= b.first; 
		}
		} p_comparator;
		UL total_count = 0;
		cout << "###############################################################\n";
		cout << "ADDRESS EXISTS PATTERNS:\n";
		for (auto idx: pattern_exist) {
			total_count += idx.second;
		}
	
		for (auto idx: pattern_exist) {
			if ((double)((double)idx.second*100.0) >= (double)((double)total_count*(double)GREATER_THAN_PERCENT_PATTERNS))
				patterns.push_back( std::make_pair(idx.second, idx.first) );
		}
		
		sort(patterns.begin(), patterns.end(), p_comparator);
		for(auto idx: patterns) {
			cout << (idx.first*100.0)/total_count << "\t" << idx.first << "\t" << idx.second << endl;
		}
		cout << "TOTAL COUNT: " << total_count << endl;
		cout << "###############################################################\n";
		cout << endl << endl << endl;
		total_count = 0;
		patterns.clear();
		cout << "###############################################################\n";
		cout << "ADDRESS DIRTY PATTERNS:\n";
		
		for (auto idx: pattern_dirty) {
			total_count += idx.second;
		}
		
		for (auto idx: pattern_dirty) {
			if (((double)idx.second*100.0) >= (double)((double)total_count*(double)GREATER_THAN_PERCENT_PATTERNS))
				patterns.push_back( std::make_pair(idx.second, idx.first) );
		}

		sort(patterns.begin(), patterns.end(), p_comparator);
		for(auto idx: patterns) {
			cout << (idx.first*100.0)/total_count << "\t" << idx.first << "\t" << idx.second << endl;
		}
		cout << "TOTAL COUNT: " << total_count << endl;
		cout << "###############################################################\n";
	}


	void print_creationeviction_patterns() {
		struct {	
		bool operator ()(const pair<UL, bitset<2*SIZE_OF_COMPARTMENT> > &a, const pair<UL, bitset<2*SIZE_OF_COMPARTMENT> > &b) {
			return a.first <= b.first; 
		}
		} p_comparator;
		UL total_count = 0;
		cout << "###############################################################\n";
		cout << "ADDRESS EXISTS PATTERNS - EVICTION::CREATION :\n";
		for (auto idx: creation_eviction_exist) {
			total_count += idx.second;
		}
	
		for (auto idx: creation_eviction_exist) {
			if ((double)((double)idx.second*100.0) >= (double)((double)total_count*(double)GREATER_THAN_PERCENT_PATTERNS))
				creation_eviction_pattern.push_back( std::make_pair(idx.second, idx.first) );
		}
		
		sort(creation_eviction_pattern.begin(), creation_eviction_pattern.end(), p_comparator);
		for(auto idx: creation_eviction_pattern) {
			cout << (idx.first*100.0)/total_count << "\t" << idx.first << "\t\t";
			for(int bit = 0; bit < SIZE_OF_COMPARTMENT; bit++)
				cout << idx.second[bit];
			cout << "\t";
			for(int bit = 0; bit < SIZE_OF_COMPARTMENT; bit++)
				cout << idx.second[bit+SIZE_OF_COMPARTMENT];
			cout << endl;
		}
		cout << "TOTAL COUNT: " << total_count << endl;
		cout << "###############################################################\n";
		cout << endl << endl << endl;
		total_count = 0;
		creation_eviction_pattern.clear();
		cout << "###############################################################\n";
		cout << "ADDRESS DIRTY PATTERNS - EVICTION::CREATION :\n";
		
		for (auto idx: creation_eviction_dirty) {
			total_count += idx.second;
		}
		
		for (auto idx: creation_eviction_dirty) {
			if ((double)((double)idx.second*100.0) >= (double)((double)total_count*(double)GREATER_THAN_PERCENT_PATTERNS))
				creation_eviction_pattern.push_back( std::make_pair(idx.second, idx.first) );
		}

		sort(creation_eviction_pattern.begin(), creation_eviction_pattern.end(), p_comparator);
		for(auto idx: creation_eviction_pattern) {
			cout << (idx.first*100.0)/total_count << "\t" << idx.first << "\t\t";
			for(int bit = 0; bit < SIZE_OF_COMPARTMENT; bit++)
				cout << idx.second[bit];
			cout << "\t";
			for(int bit = 0; bit < SIZE_OF_COMPARTMENT; bit++)
				cout << idx.second[bit+SIZE_OF_COMPARTMENT];
			cout << endl;
		}
		cout << "TOTAL COUNT: " << total_count << endl;
		cout << "###############################################################\n";
	}
	
};

#endif
