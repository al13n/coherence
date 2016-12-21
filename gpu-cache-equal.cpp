#include <bits/stdc++.h>
#include "utility.h"
#include "dir-simulator.h"

int main(){
	string instruction;
	gpu_simulator _gpu(1<<GPU_ADDRESS_LEN);
	dir_simulator _dir;

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
			if (_dir.exists(address))
			{
				if (!_gpu.address_exists(address))
					_dir.false_positive(address);
			}
			
		} else {
			if (type == "MEMRD64B")
			{	
				_dir.insert(address);
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
	
	cout << "Directory usage/consult: " << _dir.get_consult() << endl;
	cout << "Directory size: " << _dir.size() << endl;
	cout << "False positives: " << _dir.get_fp() << endl;
	if (_dir.get_fp())	_dir.display_fps();	
	return 0;
}
