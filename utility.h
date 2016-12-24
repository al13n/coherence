#ifndef __UTILITY_H_
#define __UTILITY_H_

using namespace std;

typedef unsigned long int UL;
static const int GPU_OFFSET_LEN = 6;
static const int GPU_ADDRESS_LEN = 20;

inline UL __getaddress_gpu__(const UL cpu_address)
{
	return (cpu_address >> GPU_OFFSET_LEN) & ((UL)(1<<GPU_ADDRESS_LEN) - 1);
}

inline UL __gettag_gpu__(const UL cpu_address)
{
	return (cpu_address >> (GPU_ADDRESS_LEN + GPU_OFFSET_LEN));
}

inline UL __getaddress_cache__(const UL cpu_address)
{
	return (cpu_address >> GPU_OFFSET_LEN);
}

inline UL __makeaddress_cache__(const UL gpu_address, const UL tag)
{
	return (tag << (GPU_ADDRESS_LEN + GPU_OFFSET_LEN)) | (gpu_address);
}

#endif
