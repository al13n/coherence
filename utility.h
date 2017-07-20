#ifndef __UTILITY_H_
#define __UTILITY_H_

#ifndef NDEBUG
#   define ASSERT(condition, message) \
    do { \
        if (! (condition)) { \
            std::cerr << "Assertion `" #condition "` failed in " << __FILE__ \
                      << " line " << __LINE__ << ": " << message << std::endl; \
            std::terminate(); \
        } \
    } while (false)
#else
#   define ASSERT(condition, message) do { } while (false)
#endif

//#include "config.h"
using namespace std;

typedef unsigned long int UL;
static const int GPU_OFFSET_LEN = 6;
static const int GPU_ADDRESS_LEN = GPU_CACHE_SIZE - GPU_OFFSET_LEN;

inline UL __getaddress_gpu__(const UL cpu_address)
{
	return (cpu_address >> GPU_OFFSET_LEN) & (((UL)1<<GPU_ADDRESS_LEN) - 1);
}

inline UL __gettag_gpu__(const UL cpu_address)
{
	return (cpu_address >> (GPU_ADDRESS_LEN + GPU_OFFSET_LEN));
}

inline UL __getaddress_gpu_informclear__(const UL dir_address) {
	return (dir_address) & (((UL)1<<GPU_ADDRESS_LEN) - 1);
}

inline UL __getaddress_cache__(const UL cpu_address)
{
	return (cpu_address >> GPU_OFFSET_LEN);
}

inline UL __makeaddress_cache__(const UL gpu_address, const UL tag)
{
	return (tag << (GPU_ADDRESS_LEN)) | (gpu_address);
}

inline pair<UL, UL> __getaddresstagpair_gpu__(const UL cpu_address)
{
	return make_pair( __getaddress_gpu__(cpu_address), __gettag_gpu__(cpu_address) );
}
	
enum class isGpu_address : bool { False, True };

class rangedata {
public:
	static unordered_map<UL, UL> coverage;
	UL start;
	UL end;

	static void addcover(const UL cover) {
		if (cover == 0) return;
		if (coverage.find(cover) != coverage.end()) {
			coverage[cover]++;
		} else {
			coverage.insert(make_pair(cover, (UL)1));
		}
	}
	
	static void removecover(const UL cover) {
		if(coverage.find(cover) != coverage.end()) {
			coverage[cover]--;
			if (coverage[cover] == 0) {
				coverage.erase(coverage.find(cover));
			}
			return;
		}
	}

	~rangedata() {
		removecover(getcoverage());
	}
		
	rangedata(UL s, UL e):start(s),end(e) {
		addcover(getcoverage());
	}
	
	bool setStart(UL address) {
		removecover(getcoverage());
		start = address;
		addcover(getcoverage());
		return true;
	}
	
	bool setEnd(UL address) {
		removecover(getcoverage());
		end = address;
		addcover(getcoverage());
		return true;
	}
	
	UL getcoverage() {
		if (start > end) return 0;
		return (UL)(end - start + (UL)1);
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
		
		UL prev_cover = getcoverage();
		
		if (insert_val < start) {
			start = insert_val;
		}
		
		if (insert_val > end) {
			end = insert_val;
		}
		
		removecover(prev_cover);
		addcover(getcoverage());
		return true;
	}

	bool addoutlier(const UL insert_val) {
		UL prev_cover = getcoverage();
		if (start > insert_val) {
			start = insert_val;
			removecover(prev_cover);
			addcover(getcoverage());
			return true;
		}

		if (end < insert_val) {
			end = insert_val;
			removecover(prev_cover);
			addcover(getcoverage());
			return true;
		}

		return false;
	}

	bool operator < (const rangedata x) const {
		return end < x.start;
	}
	
	void print() {
		cout << "START:\t" << start << " END:\t" << end << endl;
	}
};
unordered_map<UL, UL> rangedata::coverage;
#endif
