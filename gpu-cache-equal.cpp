#include <bits/stdc++.h>
#include "dir-simulator.h"
#include "gpu-simulator.h"

int main(){
	string instruction;
	dir_simulator< set<UL> > _dir;
	gpu_simulator _gpu(1<<GPU_ADDRESS_LEN);

	while (getline(cin, instruction))
	{
		stringstream ss(instruction);
		string type;
		UL address;
		int thd;
		ss >> type;
		ss >> std::hex >> address;
		ss >> std::dec >> thd;
		if (thd < 12)
		{
			if (type == "MEMRD64B")
			{
				if (_dir.exists(address))
				{
					if (!_gpu.address_exists(address) || !_gpu.address_isdirty(address))
						_dir.false_positive();
					_dir.remove(address);
				}
				if (_gpu.address_exists(address))
					_gpu.address_markclean(address);
			
			} else if(type == "RDINV64B") {
				if (_dir.exists(address))
					_dir.remove(address);
				if (_gpu.address_exists(address))
					_gpu.address_remove(address);
			} else {
				//writeback
			}
		} else {
			if (type == "MEMRD64B")
			{	
				_gpu.load(address);
			} else if (type == "RDINV64B") {
				// Should there be a way in which the gpu(internally) informs the directory, when it needs to mark some address dirty?
				_dir.insert(address);
				_gpu.store(address);
			} else {
				// writeback
			}
		}
	}
	
	cout << "Directory size: " << _dir.size() << endl;
	cout << "False positives: " << _dir.get_fp() << endl;
	return 0;
}
