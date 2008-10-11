/** \file pkg_index.hpp
 *
 * Created: JohnE, 2008-10-09
 */
#ifndef PKG_INDEX_HPP_INC
#define PKG_INDEX_HPP_INC


#include <list>
#include <map>
#include <vector>
#include "ref.hpp"


struct Package;


class PkgIndex
{
public:
	static void ClearAll();
	static bool LoadManifest(const std::string& mfile);

	static int NumCategories();
	static const char* GetCategory(int cat);
	static int CategoryIndex(const char* cat_id);

	typedef std::map< std::string, RefType< Package >::Ref > StringPackageMap;
	typedef StringPackageMap::const_iterator PackageIter;
	static PackageIter Packages_Begin();
	static PackageIter Packages_End();

private:
	static void SetError(const char* fmt, ...);
	static void InsertPackage(RefType< Package >::Ref ins);

	static char sm_lasterror[];
	static std::vector< std::string > sm_index_categories;
	typedef std::map< std::string, int > StringIntMap;
	static StringIntMap sm_id_categories;
	static StringPackageMap sm_id_packages;
};


#endif // PKG_INDEX_HPP_INC
