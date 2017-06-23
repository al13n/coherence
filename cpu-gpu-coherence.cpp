#include <bits/stdc++.h>
#include "config.h"
#include "gpu-simulator.h"
//#include "dir-simulator.h"
#include "compartments.h"

int fps = 0;
int consult = 0;
int nofps = 0;

int usage() {
	return ++consult;
}

int false_positive() {
	return ++fps;
}

int no_fps() {
	return ++nofps;
}

int main() {
	srand(time(NULL));
	string instruction;
	gpu_simulator _gpu;
	dir_simulator _dir(&_gpu);
	
	UL cpu_loads = 0;
	UL cpu_stores = 0;
	UL gpu_loads = 0;
	UL gpu_stores = 0;
	UL ask_dir_exists = 0;
	UL ask_dir_dirty = 0;
	UL dir_dirty[2] = {0, 0};
	UL dir_exists[2] = {0, 0};
	UL errors = 0;
	UL gpu_misses = 0;
	
	ifstream configfile("config.h");
	//ofstream missfile("gpumisses_1");
	string configcontent = "";
	string tmp;
	while (!configfile.eof()) {
		getline(configfile, tmp);
		configcontent = configcontent + tmp  + "\n";
	}

	/*
	vector<string> instructions;
	while (getline(cin, instruction)) {
		instructions.push_back(instruction);
	}
	
	cout << "COMPLETED READING INSTRUCTIONS" << endl;
	*/
	UL pc = 0;
	//while (pc < instructions.size())
	while (getline(cin, instruction))
	{
		
		//instruction = instructions[pc++];
		pc++;
		if (pc%1000000 == 0) cout << "PROCESSING INSTRUCTION: " << pc << endl;
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
				//usage();
				//ask_dir_exists++;
				//if (_dir.exists(address)) {
					// The address is in the GPU and it could either be clean or dirty
					
					// 1. - Is it dirty? Ask the directory (could be a false alarm) 
					//    - If yes, clean it up on the GPU, inform the GPU and load it to CPU
					//if(_gpu.exists(address)) {
					//	no_fps();
					//	dir_exists[1]++;
						ask_dir_dirty++;
						usage();
						if(_dir.isdirty(address)) {
							dir_dirty[1]++;
							if (!_gpu.isdirty(address)) {
								false_positive();
								_dir.inform_falsepositive_dirty(address);
							} else {
								no_fps();
								_gpu.resetdirtybit(address);
								_dir.removedirty(address);
							}
						} else {
							dir_dirty[0]++;
							if (_gpu.isdirty(address)) {
								errors++;
							} else {
								no_fps();
							}
						}
					//} else {
					//	false_positive();
					//}

					// 2. - So, it's clean. Doesnt matter
				//} else {
				//	dir_exists[0]++;
				//	if (_gpu.exists(address)) {
				//		errors++;
				//	} else {
				//		no_fps();
				//	}
				//}
			}
			// STORE 
			else if(type == "RDINV64B") {
				cpu_stores++;
				usage();
				// Is the address present in the GPU - dirty or clean?
				ask_dir_exists++;
				if (_dir.exists(address)) {
					dir_exists[1]++;
					if(!_gpu.exists(address)) {
						false_positive();
						_dir.inform_falsepositive_exists(address);
					} else {
						no_fps();
						_gpu.remove(address);
						_dir.remove(address, isGpu_address::False);
					}
				} else {
					dir_exists[0]++;
					if (_gpu.exists(address)) {
						errors++;
					} else {
						no_fps();
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
					_dir.remove(r_address, isGpu_address::True);
				}
				
				_dir.insert(address);
				if (!_gpu.exists(address)) {
					//missfile << address << endl;
					gpu_misses++;
				}
				_gpu.insert(address);
			}
			// STORE
			else if (type == "RDINV64B") {
				gpu_stores++;
				// Is this address being replaced on the GPU?
				if (_gpu.isreplace(address)) {
					UL r_address = _gpu.getaddress_replace(address);
					_dir.remove(r_address, isGpu_address::True);
				}
				if (!_gpu.exists(address)) {
					//missfile << address << endl;
					gpu_misses++;
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
		
		assert(errors == 0);
	}
	
	//_dir.print();
	_dir.printpatterns();
	
	cout << "\n------------------------CONFIG-----------------------------------------------\n";
	cout << configcontent << endl;
	cout << "-----------------------------------------------------------------------------\n\n";
	
	cout << "ERRORS: " << errors << endl;
	cout << "CPU LOADS: " << cpu_loads << " CPU STORES: " << cpu_stores << " GPU LOADS: " << gpu_loads << " GPU STORES: " << gpu_stores << endl;
	cout << "DIR ASK DIRTY: " << ask_dir_dirty << " ";
	cout << "DIR DIRTY YES: " << dir_dirty[1] << " DIR DIRTY NO: " << dir_dirty[0] << endl;
	cout << "DIR ASK EXISTS: " << ask_dir_exists << " ";
	cout << "DIR EXISTS YES: " << dir_exists[1] << " DIR EXISTS NO: " << dir_exists[0] << endl;
	cout << "NUMBER OF DIR LINES: " << _dir.numberoflines() << endl;
	cout << "CURRENT SIZE OF DIRECTORY: " << _dir.size() << " bytes " << (_dir.size())/1024 << " kilobytes" << endl;
	cout << "MAX SIZE OF DIRECTORY: " << _dir.get_max_size() << " bytes " << (_dir.get_max_size())/1024 << " kilobytes" << endl;
	cout << "TIMES DIR ASKED: " << consult << " FALSE POSITIVE: " << fps << " (%)false_positives: " << (fps*100.0/consult) << endl;
	cout << "NO FALSE POSITIVE: " << nofps << endl;
	cout << "TOTAL DATA CLEARED DUE TO DIR CLEARANCE: " << _gpu.gettotal_data_cleared() << " addresses " << _gpu.gettotal_data_cleared()*64 << " (bytes) " << (_gpu.gettotal_data_cleared()*64)/1024 << " (kb)" << endl;
	cout << "GPU MISSES: " << gpu_misses << endl;
	cout << "AVG RANGE COVERAGE: " << _dir.get_avg_rangecoverage() << endl;
	cout << "MAX RANGE COVERAGE: " << _dir.get_max_rangecoverage() << endl;
	cout << "ENTRY MAX RANGE COVERAGE: " << _dir.get_entry_max_rangecoverage() << endl;
	cout << "NUMBER OF GPU EVICTIONS: " << _gpu.getnum_evictions() << endl;
	
	return 0;
}
