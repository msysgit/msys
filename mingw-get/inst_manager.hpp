#ifndef INST_MANAGER_HPP_INC
#define INST_MANAGER_HPP_INC


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <map>
#include <vector>
#include "tinyxml/tinyxml.h"
#include "ref.hpp"


struct Package;


class InstManager
{
public:
	static const char* GetError();

	static bool Load
	 (const std::string& inst_loc,
	  bool create = false);
	static void Save();

private:
	static void SetError(const char* fmt, ...);

	static char sm_lasterror[];
	static std::string sm_inst_loc;
};


#endif // INST_MANAGER_HPP_INC
