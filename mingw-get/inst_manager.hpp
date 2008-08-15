#ifndef INST_MANAGER_HPP_INC
#define INST_MANAGER_HPP_INC


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>
#include <map>
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
	static bool LoadManifest(const std::string& mfile);

	static char sm_lasterror[];
	static std::string sm_inst_loc;
	typedef std::map< std::string, RefType< Package >::Ref > IDPackageMap;
	static IDPackageMap sm_all_packages;
};


#endif // INST_MANAGER_HPP_INC
