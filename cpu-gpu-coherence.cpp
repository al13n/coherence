#include <bits/stdc++.h>
#include "gpu-simulator.h"
#include "dir-simulator.h"

int fps = 0;
int consult = 0;

int usage() {
	return ++consult;
}

int false_positive() {
	return ++fps;
}

int main() {
	string instruction;
	dir_simulator _dir;
	gpu_simulator _gpu;
	
	int cpu_loads = 0;
	int cpu_stores = 0;
	int gpu_loads = 0;
	int gpu_stores = 0;
	
	int errors = 0;
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
				cpu_loads++;
				usage();
				if (_dir.exists(address)) {
					// The address is in the GPU and it could either be clean or dirty
					
					// 1. - Is it dirty? Ask the directory (could be a false alarm) 
					//    - If yes, clean it up on the GPU, inform the GPU and load it to CPU
					if(_dir.isdirty(address)) {
						if (!_gpu.isdirty(address)) {
							false_positive();
							_dir.inform_falsepositive_dirty(address);
						} else {
							_gpu.resetdirtybit(address);
							_dir.removedirty(address);
						}
					} else {
						if (_gpu.isdirty(address)) {
							errors++;
						}
					}

					// 2. - So, it's clean. Doesnt matter
				} else {
					if (_gpu.exists(address)) {
						errors++;
					}
				}
			}
			// STORE 
			else if(type == "RDINV64B") {
				cpu_stores++;
				usage();
				// Is the address present in the GPU - dirty or clean?
				if (_dir.exists(address)) {
					if(!_gpu.exists(address)) {
						false_positive();
						_dir.inform_falsepositive_exists(address);
					} else {
						_gpu.remove(address);
						_dir.remove(address, false);
					}
				} else {
					if (_gpu.exists(address)) {
						errors++;
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
				gpu_loads++;	
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
				gpu_stores++;
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
	
	_dir.print();
	cout << "ERRORS: " << errors << endl;
	cout << "CPU LOADS: " << cpu_loads << " CPU STORES: " << cpu_stores << " GPU LOADS: " << gpu_loads << " GPU STORES: " << gpu_stores << endl;
	cout << "SIZE OF DIRECTORY: \t" << _dir.size()*8 << " bytes " << (_dir.size()*8)/1024 << " kilobytes" << endl;
	cout << consult<< " " << fps << " " << fps*1.0/consult << endl;	
	return 0;
}
