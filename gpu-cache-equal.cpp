#include "utility.h"

int main(){
	string instruction;
	vector<UL> current_access;
	gpu_simulator _gpu(1<<GPU_ADDRESS_LEN);

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
			UL gpu_adress = __getaddress_gpu__(address);
			//Consult cache
			
		} else {
			if (type == "MEMRD64B")
			{
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
	return 0;
}
