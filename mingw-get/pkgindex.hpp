/** \file pkgindex.hpp
 *
 * Created: JohnE, 2008-10-09
 */
#ifndef PKGINDEX_HPP_INC
#define PKGINDEX_HPP_INC


#include <list>
#include <map>
#include <vector>
#include "ref.hpp"


struct Package;


class PkgIndex
{
public:
	static void Clear();
	static bool LoadIndex();

	static int NumCategories();
	static const char* GetCategory(int cat);
	static int CategoryIndex(const char* cat_id);

	static int NumHeadings();
	static char const* GetHeading(int heading);
	static std::list< int >::const_iterator HeadingChildren_Begin(int heading);
	static std::list< int >::const_iterator HeadingChildren_End(int heading);

	typedef std::map< std::string, RefType< Package >::Ref > StringPackageMap;
	typedef StringPackageMap::const_iterator PackageIter;
	static PackageIter Packages_Begin();
	static PackageIter Packages_End();

private:
	static void InsertPackage(RefType< Package >::Ref ins);

	static std::vector< std::string > sm_index_categories;
	typedef std::map< std::string, int > StringIntMap;
	static StringIntMap sm_id_categories;
	static StringPackageMap sm_id_packages;

	static std::vector< std::pair< std::string, std::list< int > > >
	 sm_headings;
};


#endif // PKGINDEX_HPP_INC
