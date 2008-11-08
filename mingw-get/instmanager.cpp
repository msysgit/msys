/** \file instmanager.cpp
 *
 * Created: JohnE, 2008-11-01
 */


#include "instmanager.hpp"

#include <sys/stat.h>


std::string InstManager::sm_inst_path;


extern "C" int InstManager_ApplyChanges()
{
	return 1;
}


bool InstManager::InstExistsAt(const std::string& path)
{
	struct _stat st;
	return (_stat((std::string(path) + "\\mingwinst.mft").c_str(), &st) == 0);
}


bool InstManager::Load(const char* inst_path)
{
	sm_inst_path = inst_path;
	return true;
}
