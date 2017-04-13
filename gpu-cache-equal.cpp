#include <bits/stdc++.h>
#include "gpu-simulator.h"
#include "dir-simulator.h"

int usage(bool get = false) {
	static int consult = 0;
	if(get) return consult;
	return ++consult;
}

int false_positive(bool get = false) {
	static int fps = 0;
	if(get) return fps;
	return ++fps;
}

int main() {
	string instruction;
	dir_simulator _dir;
	gpu_simulator _gpu;

	while (getline(cin, instruction))
	{
		stringstream ss(instruction);
		string type;
		UL address;
		int thd;
		ss >> type;
		ss >> std::hex >> address;
		ss >> std::dec >> thd;
		
		//CPU
		if (thd < 12)
		{	//LOAD
			if (type == "MEMRD64B")
			{
				if (_dir.exists(address)) {
					// The address is in the GPU and it could either be clean or dirty
					
					// 1. - Is it dirty? Ask the directory (could be a false alarm) 
					//    - If yes, clean it up on the GPU, inform the GPU and load it to CPU
					
					if(_dir.isdirty(address)) {
						usage();
						if(!_gpu.isdirty(address)) {
							false_positive();
							_dir.inform_falsepositive_dirty(address);
						} else {
							_gpu.resetdirtybit(address);
							_dir.removedirty(address);
						}
					}

					// 2. - So, it's clean. Doesnt matter
				}
			}
			// STORE 
			else if(type == "RDINV64B") {
				
				// Is the address present in the GPU - dirty or clean?
				if (_dir.exists(address)) {
					usage();
					if(!_gpu.exists(address)) {
						false_positive();
						_dir.inform_falsepositive_exists(address);
					} else {
						_gpu.remove(address);
						_dir.remove(address);
					}
				}
			}
			//WRITEBACK
			else {
				//writeback
			}
		}
		//GPU
		else {
			// LOAD
			if (type == "MEMRD64B") {	
				// Is this address present on the GPU? - dont care
				
				// Is this address being replaced on the GPU? - update directory
				if (_gpu.isreplace(address)) {
					UL r_address = _gpu.getaddress_replace(address);
					_dir.remove(r_address, true);
				}
				
				_dir.insert(address);
				_gpu.insert(address);
			}
			// STORE
			else if (type == "RDINV64B") {
				// Is this address being replaced on the GPU?
				if (_gpu.isreplace(address)) {
					UL r_address = _gpu.getaddress_replace(address);
					_dir.remove(r_address, true);
				}
				_gpu.insert(address);
				_gpu.setdirtybit(address);
				_dir.insert(address);
				_dir.markdirty(address);
				
			}
			// WRITEBACK
			else {
				// writeback
			}
		}
	}
	
	return 0;
}
