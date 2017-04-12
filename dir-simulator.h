#ifndef __DIR_SIMULTOR_H__
#define __DIR_SIMULATOR_H__
#include "utility.h"
#include <vector>
#include <algorithm>

/*
 *	Typename 'T' should have the following api's:
 *		- insert(unsigned long int key) -> pair<key, flag> : bool flag = success(?) 
 *		- find(unsigned long int key) -> iterator<T>
 *		- erase(unsigned long int key) -> bool/int
 *		- size() -> size of ds
 *		- end() -> iterator<T> to end of ds
 *	
 */
class dir_simulator {
private:
	struct rangedata {
		UL start;
		UL end;
		
		rangedata(UL s, UL e):start(s),end(e) {
		}
	
		bool findinrange(const UL find_val) {
			return find_val <= end && find_val >= start;
		}
	
		bool addtorange(const UL insert_val) {
			if ( !(	
				findinrange(insert_val+1) 
				|| findinrange(insert_val) 
				|| (insert_val != 0 && findinrange(insert_val-1))
			      ) 
			) {
				return false;
			}
			
			if (insert_val < start) {
				start = insert_val;
			}
			
			if (insert_val > end) {
				end = insert_val;
			}
			
			return true;
		}
	
		bool operator < (const rangedata x) {
			return end < x.start;
		}
		
		bool overlaps(const rangedata next) {
			return (start <= next.end) && (end >= next.start);
		}
	};

	struct line {
		std::vector<rangedata> address_load; // Keep them in order
		std::vector<rangedata> address_dirty; // Keep them in order
		std::vector<rangedata> false_positive_exist; // We said yes to exist earlier, but it does not - keep it accurate
		std::vector<rangedata> false_positive_dirty; // We said yes to dirty earlier, but it is not - keep it accurate
		
		bool existsinvector(vector<rangedata> & vec, const UL &check_address) {
			for (auto &idx: vec) {
				if (idx.findinrange(check_address)) {
					return true;
				}
			}
			return false;
		}
		
		bool insertinvector(vector<rangedata> &vec, const UL &address) {
			for (int i = 0; i < vec.size(); i++) {
				if (vec[i].addtorange(address)) {
					if (i > 0 && vec[i-1].end == vec[i].start) {
						vec[i-1].end = vec[i].end;
						vec.erase(vec.begin() + i);
					} else {
						if (i < vec.size() && vec[i+1].start == vec[i].end) {
							vec[i].end = vec[i+1].end;
							vec.erase(vec.begin() + i+1);
						}
					}
					return true;
				}
			}
			
			// add point range
			rangedata newrange(address, address);
			vec.push_back(newrange);
			std::sort(vec.begin(), vec.end());
			return true;
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
				if (vec[i].start == address) {
					vec[i].start = address+1;
					if (vec[i].start > vec[i].end) {
						vec.erase(vec.begin() + i);
					}
				}
				else if (vec[i].end == address) {
					vec[i].end = address - 1;
					if (vec[i].start > vec[i].end) {
						vec.erase(vec.begin() + i);
					}
				}
				else {
					int s = vec[i].start;
					int e = vec[i].end;
					vec.erase(vec.begin() + i);
					if (address > s) {
						rangedata newrange(s, address-1);
						vec.push_back(newrange);
					}
					
					if(address < e) {
						rangedata newrange(address+1, e);
						vec.push_back(newrange);
					}
					
					sort(vec.begin(), vec.end());
				}
				return true;
			}

			return false;
		}
		
		bool isdirty(const UL check_address) {
			if (existsinvector(false_positive_dirty, check_address))
				return false;
			
			if (existsinvector(address_dirty, check_address)) 
				return true;
			
			return false;
		}

		bool existsinline(const UL check_address) {
			if (existsinvector(false_positive_exist, check_address))
				return false;
			
			if (existsinvector(address_load, check_address)) 
				return true;
			
			return false;
		}

		bool removedirty(const UL address) {
			if (removefromvector(address_dirty, address)) {
				return true;
			}
			return false;
		}

		bool insertdirty(const UL address) {
			// is this dirty address existing in one of line's range?
			if (existsinvector(address_load, address)) {
				// was this flagged as false_positive_dirty earlier?
				if (existsinvector(false_positive_dirty, address)) {
					//yes, remove from false_positive_dirty
					removefromvector(false_positive_dirty, address);
				}
				// add to dirty vector
				insertinvector(address_dirty, address);
				return true;
			}
			return false;
		}
		
		
	}
	
	struct lrulines {
		int age;
		line data;
	};
	
	vector<lrulines> dir_mem;
public:
	dir_simulator(){}

	int size() { return dir_mem.size(); }

	bool exists(const UL cpu_address) {
	}

	bool insert(const UL cpu_address) {
	}

	bool remove(const UL address, int isgpuaddress) {
	}

	bool inform_falsepositive_dirty(UL address) {
	
	}
	
	bool inform_falsepositive_exists(UL address) {
	
	}

	bool isdirty(UL address) {
	}
	
	bool markdirty(UL address) {
	}
};

#endif
