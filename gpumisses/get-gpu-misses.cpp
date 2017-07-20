#include <bits/stdc++.h>
#include "common.h"
#include "gpu-simulator.h"

int main() {
	string instruction;
	gpu_simulator _gpu;
	UL gpu_misses = 0;
	
	ifstream configfile("config.h");
	string configcontent = "";
	string tmp;
	
	while (!configfile.eof()) {
		getline(configfile, tmp);
		configcontent = configcontent + tmp  + "\n";
	}

	UL pc = 0;
	while (getline(cin, instruction))
	{
		
		pc++;
		if (pc%1000000 == 0) cout << "PROCESSING INSTRUCTION: " << pc << endl;
		stringstream ss(instruction);
		string type;
		UL address;
		int thd;
		ss >> type;
		ss >> std::hex >> address;
		ss >> std::dec >> thd;
		
	/*	//CPU
		if (thd < 12)
		{	//LOAD
			if (type == "MEMRD64B")
			{
				_gpu.resetdirtybit(address);
			}
			// STORE 
			else if(type == "RDINV64B") {
				_gpu.remove(address);
			}
			//WRITEBACK
			else {
				//writeback
			}
		}
		//GPU
		else {
			// LOAD
	*/		if (type == "MEMRD64B") {
				if (!_gpu.exists(address)) {
					gpu_misses++;
				}
				_gpu.insert(address);
			}
			// STORE
			else if (type == "RDINV64B") {
				if (!_gpu.exists(address)) {
					gpu_misses++;
				}
				_gpu.insert(address);
				_gpu.setdirtybit(address);
			}
			// WRITEBACK
			else {
				// writeback
			}
	//	}
		
	}

	cout << configcontent << endl;	
	cout << "GPU MISSES: " << gpu_misses << endl;
	
	return 0;
}
