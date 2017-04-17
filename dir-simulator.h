#ifndef __DIR_SIMULTOR_H__
#define __DIR_SIMULATOR_H__
#include "utility.h"
#include "config.h"
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
			//total size * 2 = size of rangedata * 8 bytes UL 
			return (total_size<<1)<<3;
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
	
		bool exceeds_fullrangelimit(vector<rangedata> &vec, const UL &address) {
			if (vec.size() != 0) {
				UL s = vec[0].start;
				if (s > address) {
					s = address;
				}
				UL e = vec[vec.size()-1].end;
				if(e < address) {
					e = address;
				}
				return ((UL)(e - s + (UL)1) >= MAX_FULLRANGE_LIMIT);
			}
			return false;
		}
			
		bool insertinvector(vector<rangedata> &vec, const UL &address, bool isaccurate) {
			UL closeness = CLOSEST_RANGE_INSERT_LIMIT + 1;
			int pos = -1;
			
			if (!exceeds_fullrangelimit(vec, address)) {
				for (int i = 0; i < vec.size(); i++) {
					if (vec[i].addtorange(address)) {
						if (i > 0 
						&& vec[i-1].end == vec[i].start 
						) {
							vec[i-1].end = vec[i].end;
							vec.erase(vec.begin() + i);
						} else {
							if (i < vec.size() 
							&& vec[i+1].start == vec[i].end
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
				insertinvector(address_dirty, address, false);
				return true;
			}
			return false;
		}
		
		bool insert(const UL address) {
			if (!existsinvector(address_load, address)) {
				return insertinvector(address_load, address, true);
			}
			return true;
		}
		
		bool remove(const UL address) {
			if (existsinvector(address_load, address)) {
				removefromvector(address_load, address);
				removefromvector(address_dirty, address);
				removefromvector(false_positive_dirty, address);
				removefromvector(false_positive_exist, address);
				return true;
			}
			return true;
		}

		bool inform_falsepositive_dirty(const UL address) {
			if (existsinvector(address_load, address)) {
				// Optional to remove from dirty vector
				// removedirty(address);
				insertinvector(false_positive_dirty, address, true);
				print();
				return true;
			}
			return false;
		}

		bool inform_falsepositive_exists(const UL address) {
			if (existsinvector(address_load, address)) {
				// Optional to remove from address_load vector
				// removefromvector(address_load, address);
				insertinvector(false_positive_exist, address, true);
				print();
				return true;
			}
			return false;
		}
	/*	
		UL closest(const UL address) {
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
			// #region: make it ready for new range insertion
			resetline();
			// #endregion
			return 0;
		}
	*/
		void print() {
			cout << "----------------------------------------------\n";
			cout << "address_load ranges: " << address_load.size() << "(size)\n";
			for(auto idx: address_load) {
				idx.print();
			}
			
			cout << "address_dirty ranges: " << address_dirty.size() << "(size)\n";
			for(auto idx: address_dirty) {
				idx.print();
			}
			cout << "false_positive_exist ranges: " << false_positive_exist.size() << "(size)\n";
			for(auto idx: false_positive_exist) {
				idx.print();
			}
			cout << "false_positive_dirty ranges: " << false_positive_dirty.size() << "(size)\n";
			for(auto idx: false_positive_dirty) {
				idx.print();
			}
			cout << "----------------------------------------------\n";
		}
		
	};
	
	struct lrulines {
		int age;
		line data;
		
		bool operator < (const lrulines& b) const {
			return age <= b.age;
		}
	};
	
	std::vector<lrulines> dir_mem;
	
	bool resetline() {
		int mx = -1;
		int pos = -1;
		for (int i = 0; i < dir_mem.size(); i++) {
			if (dir_mem[i].age > mx) {
				mx = dir_mem[i].age;
				pos = i;
			}
		}
		if ( pos != -1) {
			gpu->inform_clear(dir_mem[pos].data.address_load);
			dir_mem.erase(dir_mem.begin() + pos);	
			return true;
		}
		return false;
	}
	
	void shouldpurge() {
		if ( age_exceed || (size())/1024 > MAX_SIZE_DIRECTORY) {
			age_exceed = false;
			resetline();
		}
		std::sort(dir_mem.begin(), dir_mem.end());
	}
	
	gpu_simulator* gpu;
	bool age_exceed;
	UL max_sz;
public:
	dir_simulator(gpu_simulator* _gpu): age_exceed(false), max_sz(0){
		gpu = _gpu;
	}
	
	UL numberoflines() {
		return dir_mem.size();
	}
	
	UL get_max_size() {
		return max_sz;
	}
	
	UL size() { 
		//cout << "Number of directory lines: " << dir_mem.size() << endl;
		UL total_size = 0;
		for(auto idx: dir_mem) {
			total_size += idx.data.size() + 4;
		}
		if (total_size > max_sz) max_sz = total_size;
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
				if(idx.age > MAX_AGE_LIMIT)
					age_exceed = true;
			}
		}
		return flag;
	}

	bool insert(const UL cpu_address) {
		UL gpu_address = __getaddress_cache__(cpu_address);
		bool flag = false;
		/*
		for (int i = 0; i < dir_mem.size(); i++) {
			UL closeness = dir_mem[i].data.closest(gpu_address);
			if (closeness <= CLOSEST_RANGE_INSERT_LIMIT) {
				if (mn > closeness) {
					pos = i;
					mn = closeness;
				}
			}
			dir_mem[i].age++;
		}
		*/
		
		for (auto &idx: dir_mem) {
			if (!flag && idx.data.insert(gpu_address)) {
				idx.age = 0;
				flag = true;
			}
			else
				idx.age++;
		}
		
		if (!flag) {
			lrulines newlruline;
			newlruline.age = 0;
			flag = newlruline.data.insert(gpu_address);
			dir_mem.push_back(newlruline);
		}
		
		shouldpurge();
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
				if(idx.age > MAX_AGE_LIMIT)
					age_exceed = true;
			}
		}
		
		shouldpurge();
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
				if(idx.age > MAX_AGE_LIMIT)
					age_exceed = true;
			}
		}
		
		shouldpurge();
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
				if(idx.age > MAX_AGE_LIMIT)
					age_exceed = true;
			}
		}
		shouldpurge();
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
				if(idx.age > MAX_AGE_LIMIT)
					age_exceed = true;
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
				if(idx.age > MAX_AGE_LIMIT)
					age_exceed = true;
			}
		}
		if (!flag) {
			lrulines newlruline;
			newlruline.age = 0;
			newlruline.data.insert(gpu_address);
			newlruline.data.insertdirty(gpu_address);
			dir_mem.push_back(newlruline);
			flag = true;
		}
		shouldpurge();
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
				if(idx.age > MAX_AGE_LIMIT)
					age_exceed = true;
			}
		}
		shouldpurge();
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
