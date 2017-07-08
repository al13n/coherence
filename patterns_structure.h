#pragma once
#include "utility.h"
#include "config.h"
#include <vector>
#include <algorithm>
#include <map>
#include <limits>
UL getregion(const UL &address) {
	return (address/SIZE_OF_PATTERN);
}

int getoffset(const UL &address) {
	return address%SIZE_OF_PATTERN;
}

int getdenseindex(const UL &address) {
	const int _half = SIZE_OF_PATTERN >> 1;
	if (getoffset(address) < _half) {
		return 1;
	}
	return 2;
}

UL getpatternfromidx(const UL &idx) {
	UL pattern = ((UL)1 << SIZE_OF_PATTERN) - 1;
	if (idx != 3) {
		UL remove = ((UL)1 << (SIZE_OF_PATTERN >> 1)) - 1;
		if (idx == 1) {
			remove <<= (SIZE_OF_PATTERN >> 1);
		}
		pattern ^= remove;
	}
	return pattern;
}

class quadleveldata {
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
		UL total_cnt = patterns[1].size() + patterns[2].size() + patterns[3].size() + others.size();
		cout << "COMPLETELY DENSE:\t" << patterns[3].size() << " (" << (patterns[3].size()*100.0)/total_cnt << "%)" << endl;
		cout << "HALF - 1 DENSE:\t" << patterns[1].size() << " (" << (patterns[1].size()*100.0)/total_cnt << "%)" << endl;
		cout << "HALF - 2 DENSE:\t" << patterns[2].size() << " (" << (patterns[2].size()*100.0)/total_cnt << "%)" << endl;
		cout << "OTHERS:\t" << others.size() << " (" << (others.size()*100.0)/total_cnt << "%)" << endl;
	}
};

class bileveldata {
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
		UL total_cnt = patterns.size() + others.size();
		cout << "COMPLETELY DENSE:\t" << patterns.size() << " (" << (patterns.size()*100.0)/total_cnt << "%)" << endl;
		cout << "OTHERS:\t" << others.size() << " (" << (others.size()*100.0)/total_cnt << "%)" << endl;
	}
};

