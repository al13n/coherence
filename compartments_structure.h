#pragma once
#include "utility.h"
#include "config.h"
#include <vector>
#include <algorithm>
#include <map>
#include <limits>
class compartment {
private:
	UL _rangecover;
public:
	std::vector<rangedata> address_exist;
	
	shared_ptr<compartment> previous;
	shared_ptr<compartment> next;
	
	~compartment() {
		previous = next = nullptr;
		address_exist.clear();
	}
		
	compartment(UL gpu_address): previous{nullptr}, next{nullptr}{
		address_exist.clear();
		_rangecover = 0;
		insert(gpu_address);
	}
	
	UL size() {
		if (address_exist.empty())	
			return 0;
		UL total_size = 0;
		total_size += address_exist.size();
		return total_size*sizeof(rangedata) + (COUNT_LINK_SIZE_COMPARTMENT ? (sizeof(previous) + sizeof(next)) : 0);
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
					vec[i-1].setEnd(vec[i].end);
					vec.erase(vec.begin() + i);
				} else {
					if (i+1 < vec.size() 
					&& ( (vec[i+1].start == vec[i].end) || ( vec[i+1].start == (vec[i].end+1) ) )
					) {
						vec[i].setEnd(vec[i+1].end);
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
				vec[pos].setStart(address+1);
				if (vec[pos].start > vec[pos].end) {
					vec.erase(vec.begin() + pos);
				}
			}
			else if (vec[pos].end == address) {
				vec[pos].setEnd(address - 1);
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
		return true;
	}
		
	UL getkey() {
		if (!address_exist.empty()) {
			return (address_exist[0].start)/SIZE_OF_COMPARTMENT;
		}
		return NAN;
	}
	
	UL getcoverage() {
		return _rangecover;
	}

	void print() {
		cout << "---------------------------------------------------------\n";
		cout << "Number of ranges:\t" << address_exist.size() << endl;
		cout << "---------------------------------------------------------\n";
	}
	
	UL getnumberofranges() {
		return address_exist.size();
	}
};

