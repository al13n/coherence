#ifndef __UTILITY_H_
#define __UTILITY_H_

using namespace std;

typedef unsigned long int UL;
static const int GPU_OFFSET_LEN = 6;
static const int GPU_ADDRESS_LEN = 20;

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

struct rangedata {
	UL start;
	UL end;
	
	rangedata(UL s, UL e):start(s),end(e) {
	}
	
	UL getcoverage() {
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
		
		if (insert_val < start) {
			start = insert_val;
		}
		
		if (insert_val > end) {
			end = insert_val;
		}
		
		return true;
	}

	bool addoutlier(const UL insert_val) {
		if (start > insert_val) {
			start = insert_val;
			return true;
		}

		if (end < insert_val) {
			end = insert_val;
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
		
#endif
