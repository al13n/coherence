#include "utility.h"

struct statistic {
	void false_positive()
	{
		fp++;
	}
	
	void usage()
	{
		used++;
	}
	
	pair<int, int> get_fp()
	{
		return make_pair(fp, used);
	}
private:
	int fp = 0;
	int used = 0;
} _stats;


int main(){
	string instruction;
	vector<UL> current_access;
	gpu_simulator _gpu(1<<GPU_ADDRESS_LEN);
	cache_simulator _cache;

	while(getline(cin, instruction)) {
		stringstream ss(instruction);
		string type;
		UL address;
		int thd;
		ss >> type;
		ss >> std::hex >> address;
		ss >> thd;

		if (thd < 12)
		{
			// The cpu needs to access some memory, ask the coherence directory about it
			//Consult cache
			_stats.usage();
			if (_cache.exists(address))
			{
				if (!_gpu.address_exists(address))
					_stats.false_positive();
			}
			
		} else {
			if (type == "MEMRD64B")
			{	
				_cache.insert(address);
				UL gpu_address = __getaddress_gpu__(address);
				UL tag = __gettag_gpu__(address);
				
				if(!_gpu.is_tag_present(gpu_address, tag)){
					_gpu.insert_tag(gpu_address, tag);
				}
			} else {
				// just confirm that it's the same address.
			}
		}
	}
	
	cout << "False positives: " << _stats.get_fp().first << endl;
	cout << "Cache usage: " << _stats.get_fp().second << endl;

	return 0;
}
