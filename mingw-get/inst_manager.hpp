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

	static int NumCategories();
	static const char* GetCategory(int cat);
	static int CategoryIndex(const char* cat_id);

	typedef std::map< std::string, RefType< Package >::Ref >::const_iterator
	 PackageIter;
	static PackageIter Packages_Begin();
	static PackageIter Packages_End();

private:
	static void SetError(const char* fmt, ...);
	static bool LoadManifest(const std::string& mfile);
	static void ClearPackages();
	static void InsertPackage(RefType< Package >::Ref ins);

	static char sm_lasterror[];
	static std::string sm_inst_loc;
	static std::vector< std::string > sm_index_categories;
	typedef std::map< std::string, int > StringIntMap;
	static StringIntMap sm_id_categories;
	typedef std::map< std::string, RefType< Package >::Ref > StringPackageMap;
	static StringPackageMap sm_id_packages;
};


#endif // INST_MANAGER_HPP_INC
