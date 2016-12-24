#include <bits/stdc++.h>
#include "dir-simulator.h"
#include "gpu-simulator.h"

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
		ss >> std::dec >> thd;
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
				_gpu.insert(address);
			} else {
				// just confirm that it's the same address.
			}
		}
	}
	
	cout << "Directory usage/consult: " << _dir.get_consult() << endl;
	cout << "Directory size: " << _dir.size() << endl;
	cout << "False positives: " << _dir.get_fp() << endl;
	//if (_dir.get_fp())	_dir.display_fps();	
	_gpu.print();
	_gpu._print_gpu_address_ranges();
	return 0;
}
