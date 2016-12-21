#ifndef __DIR_SIMULTOR_H__
#define __DIR_SIMULATOR_H__

class dir_simulator {
private:
	set<UL> dir_mem;
	UL consult;
	UL fp;
	set<UL> fps;
public:
	dir_simulator():consult(0), fp(0){}
	
	UL get_fp() { return fp; }
	UL get_consult() { return consult; }
	
	void false_positive(UL cpu_address) { fps.insert(cpu_address), fp++; }
	int size() { return dir_mem.size(); }

	bool exists(const UL cpu_address)
	{	
		consult++;
		return dir_mem.find(__getaddress_cache__(cpu_address)) != dir_mem.end();
	}
	
	bool insert(const UL cpu_address)
	{
		return dir_mem.insert(__getaddress_cache__(cpu_address)).second;
	}
	
	void display_fps();
};

void dir_simulator::display_fps()
{
	cout << "---------------------------\n";
	cout << "False positive addresses:\n";
	for(auto it: fps)
		cout << std::hex << it << " " << std::dec << __getaddress_gpu__(it) << endl;
	cout << "---------------------------\n";
}
#endif
