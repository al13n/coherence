#include <bits/stdc++.h>
using namespace std;

typedef unsigned long long int ull;

ull gpu_mem[1<<20];

int main(){
	string instruction;
	//set<ull> gpu_mem;
	vector<ull> current_access; 
	while(getline(cin, instruction)) {
		stringstream ss(instruction);
		string type;
		ull address;
		int thd;
		ss >> type;
		ss >> std::hex >> address;
		ss >> thd;

		if(thd < 12){
			/*
			cout << "GPU size: " << std::dec << gpu_mem.size() << endl;
			cout << "Current accessed: \n";
			for(int i = 0; i < current_access.size(); i++){
				cout << std::hex << current_access[i] << " ";
			}
			current_access.clear();
			*/
		} else {
			/*
				if(gpu_mem.find(address) == gpu_mem.end()){
					gpu_mem.insert(address);
					current_access.push_back(address);
				}
			*/
			
			if(type == "MEMRD64B"){
				ull lsb = (address & ((1<<26)-1)) >> 6;
				ull msb = (address >> 26);
				cout << std::hex << address << " " << lsb << " " << msb << endl;
				//cout << std::hex << msb << endl;
				// msb acts as tag
				if(gpu_mem[lsb] != msb){
					gpu_mem[lsb] = msb;
				} 
			} else {
				// just confirm that it's the same address.
			}
		}
	}

	//cout << "GPU size: " << gpu_mem.size() << endl;

	return 0;
}
