#ifndef __DIR_SIMULTOR_H__
#define __DIR_SIMULATOR_H__
#include "utility.h"

/*
 *	Typename 'T' should have the following api's:
 *		- insert(unsigned long int key) -> pair<key, flag> : bool flag = success(?) 
 *		- find(unsigned long int key) -> iterator<T>
 *		- erase(unsigned long int key) -> bool/int
 *		- size() -> size of ds
 *		- end() -> iterator<T> to end of ds
 *	
 */
template <typename T>
class dir_simulator {
private:
	T dir_mem;
	UL consult;
	UL fp;
public:
	dir_simulator():fp(0){}

	UL get_fp() { return fp; }

	void false_positive() { /*fps.insert(cpu_address),*/ fp++; }
	int size() { return dir_mem.size(); }

	bool exists(const UL cpu_address)
	{	
		return dir_mem.find(__getaddress_cache__(cpu_address)) != dir_mem.end();
	}

	bool insert(const UL cpu_address)
	{
		return dir_mem.insert(__getaddress_cache__(cpu_address)).second;
	}

	bool remove(const UL cpu_address)
	{
		return dir_mem.erase(__getaddress_cache__(cpu_address));
	}
	void display_fps();
};

#endif
