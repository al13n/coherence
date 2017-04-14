#ifndef __DIR_SIMULTOR_H__
#define __DIR_SIMULATOR_H__
#include "utility.h"
#include <vector>
#include <algorithm>

#define CLOSEST_RANGE_INSERT_LIMIT 16

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
				findinrange(insert_val) 
				|| ((insert_val+1) != 0	&&	findinrange(insert_val+1)) 
				|| (insert_val 	   != 0	&&	findinrange(insert_val-1))
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
	
		bool operator < (const rangedata x) const {
			return end < x.start;
		}
		
		void print() {
			cout << "START:\t" << start << " " << "END:\t" << end << endl;
		}
	};

	struct line {
		std::vector<rangedata> address_load; // Keep them in order
		std::vector<rangedata> address_dirty; // Keep them in order
		std::vector<rangedata> false_positive_exist; // We said yes to exist earlier, but it does not - keep it accurate
		std::vector<rangedata> false_positive_dirty; // We said yes to dirty earlier, but it is not - keep it accurate
		
		UL size() {
			UL total_size = 0;
			total_size += address_load.size();
			total_size += address_dirty.size();
			total_size += false_positive_exist.size();
			total_size += false_positive_dirty.size();
			//total size * 2 = size of rangedata
			return (total_size<<1);
		}
		
		void resetline() {
			/* #region: make it ready for new range insertion */
			address_load.clear();
			address_dirty.clear();
			false_positive_exist.clear();
			false_positive_dirty.clear();
			/* #endregion	*/
		}
		
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
					int s = vec[pos].start;
					int e = vec[pos].end;
					vec.erase(vec.begin() + pos);
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
		
		bool insert(const UL address) {
			if (!existsinvector(address_load, address)) {
				return insertinvector(address_load, address);
			}
			return false;
		}
		
		bool remove(const UL address) {
			if (existsinvector(address_load, address)) {
				removefromvector(address_load, address);
				removefromvector(address_dirty, address);
				removefromvector(false_positive_dirty, address);
				removefromvector(false_positive_exist, address);
				return true;
			}
			return false;
		}

		bool inform_falsepositive_dirty(const UL address) {
			if (existsinvector(address_load, address)) {
				// Optional to remove from dirty vector
				// removedirty(address);
				insertinvector(false_positive_dirty, address);
				print();
				return true;
			}
			return false;
		}

		bool inform_falsepositive_exists(const UL address) {
			if (existsinvector(address_load, address)) {
				// Optional to remove from address_load vector
				// removefromvector(address_load, address);
				insertinvector(false_positive_exist, address);
				print();
				return true;
			}
			return false;
		}
		
		int closest(const UL address) {
			if (address_load.size() != 0) {
				UL mn = CLOSEST_RANGE_INSERT_LIMIT + 1;
				for (auto idx: address_load) {
					if (idx.findinrange(address)) {
						return 0;
					} else {
						UL dist1 = abs(idx.start - address);
						UL dist2 = abs(idx.end - address);
						mn = min(mn,  min(dist1, dist2));
					}
				}
				return mn;
			}		
			/* #region: make it ready for new range insertion */
			resetline();
			/* #endregion	*/
			return 0;
		}

		void print() {
			cout << "----------------------------------------------\n";
			cout << "address_load ranges:\n";
			for(auto idx: address_load) {
				idx.print();
			}
			
			cout << "address_dirty ranges:\n";
			for(auto idx: address_dirty) {
				idx.print();
			}
			cout << "false_positive_exist ranges:\n";
			for(auto idx: false_positive_exist) {
				idx.print();
			}
			cout << "false_positive_dirty ranges:\n";
			for(auto idx: false_positive_dirty) {
				idx.print();
			}
			cout << "----------------------------------------------\n";
		}
		
	};
	
	struct lrulines {
		int age;
		line data;
	};
	
	std::vector<lrulines> dir_mem;

public:
	dir_simulator(){}

	UL size() { 
		cout << "Number of directory lines: " << dir_mem.size() << endl;
		UL total_size = 0;
		for(auto idx: dir_mem) {
			total_size += idx.data.size() + 1;
		}
		return total_size;
	}

	bool exists(const UL cpu_address) {
		UL gpu_address = __getaddress_cache__(cpu_address);
		bool flag = false;
		for (auto &idx: dir_mem) {
			if (!flag && idx.data.existsinline(gpu_address)) {
				flag = true;
				idx.age = 0;
			} else {
				idx.age++;
			}
		}
		return flag;
	}

	bool insert(const UL cpu_address) {
		UL gpu_address = __getaddress_cache__(cpu_address);
		bool flag = false;
		for (auto &idx: dir_mem) {
			if (!flag && idx.data.closest(gpu_address) <= CLOSEST_RANGE_INSERT_LIMIT) {
				idx.data.insert(gpu_address);
				idx.age = 0;
				flag = true;
			} else {
				idx.age++;	
			}
		}

		if (!flag) {
			lrulines newlruline;
			newlruline.age = 0;
			newlruline.data.insert(gpu_address);
			dir_mem.push_back(newlruline);
			flag = true;
		}
		return flag;
	}

	bool remove(const UL address, bool isgpuaddress) {
		UL cpu_address = address;
		if (!isgpuaddress) {
			cpu_address = __getaddress_cache__(address);
		}
		bool flag = false;
		for (auto &idx: dir_mem) {
			if (!flag && idx.data.remove(cpu_address)) {
				flag = true;
				idx.age = 0;
			} else {
				idx.age++;
			}
		}
		
		return flag;
	}

	bool inform_falsepositive_dirty(UL cpu_address) {
		UL gpu_address = __getaddress_cache__(cpu_address);
		bool flag = false;
		for (auto &idx: dir_mem) {
			if (!flag && idx.data.inform_falsepositive_dirty(gpu_address)) {
				flag = true;
				idx.age = 0;
			} else {
				idx.age++;
			}
		}
		return flag;
	}
	
	bool inform_falsepositive_exists(UL cpu_address) {
		UL gpu_address = __getaddress_cache__(cpu_address);
		bool flag = false;
		for (auto &idx: dir_mem) {
			if (!flag && idx.data.inform_falsepositive_exists(gpu_address)) {
				idx.age = 0;
				flag = true;
			} else {
				idx.age++;
			}
		}
		return flag;
	}

	bool isdirty(UL cpu_address) {
		UL gpu_address = __getaddress_cache__(cpu_address);
		bool flag = false;
		bool once = false;
		for (auto &idx: dir_mem) {
			if (!once && idx.data.existsinline(gpu_address)) {
				flag = idx.data.isdirty(gpu_address);
				idx.age = 0;
				once = true;
			} else {
				idx.age++;
			}
		}
		return flag;
	}
	
	bool markdirty(UL cpu_address) {
		UL gpu_address = __getaddress_cache__(cpu_address);
		bool flag = false;	
		for (auto &idx: dir_mem) {
			if(!flag && idx.data.insertdirty(gpu_address)) {
				idx.age = 0;
				flag = true;
			} else {
				idx.age++;
			}
		}
		return flag;
	}
	
	bool removedirty(UL cpu_address) {
		UL gpu_address = __getaddress_cache__(cpu_address);
		bool flag = false;	
		for (auto &idx: dir_mem) {
			if(!flag && idx.data.removedirty(gpu_address)) {
				idx.age = 0;
				flag = true;
			} else {
				idx.age++;
			}
		}
		return flag;
	}
	
	void print() {
		for (auto &idx: dir_mem) {
			cout << "###############################################################\n";
			cout << "AGE:\t" << idx.age << endl;
			idx.data.print();
			cout << "###############################################################\n";
		}
	}
};

#endif
