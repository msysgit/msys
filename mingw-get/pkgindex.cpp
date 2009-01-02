/** \file pkgindex.cpp
 *
 * Created: JohnE, 2008-10-09
 */


#include "pkgindex.hpp"

#include <string>
#include <cstdarg>
#include "tinyxml/tinyxml.h"
#include "package.hh"
#include "error.hh"
#include "getbindir.hh"


std::vector< std::string > PkgIndex::sm_index_categories;
PkgIndex::StringIntMap PkgIndex::sm_id_categories;
PkgIndex::StringPackageMap PkgIndex::sm_id_packages;
std::vector< std::pair< std::string, std::list< int > > > PkgIndex::sm_headings;


static const char* NonEmptyAttribute(const TiXmlElement* el, const char* name)
{
	const char* attr = el->Attribute(name);
	if (attr && attr[0])
		return attr;
	return 0;
}


int PkgIndex::NumCategories()
{
	return sm_index_categories.size();
}


const char* PkgIndex::GetCategory(int cat)
{
	return sm_index_categories[cat].c_str();
}


int PkgIndex::CategoryIndex(const char* cat_id)
{
	StringIntMap::iterator found = sm_id_categories.find(cat_id);
	if (found == sm_id_categories.end())
		return -1;
	return found->second;
}


int PkgIndex::NumHeadings()
{
	return sm_headings.size();
}


char const* PkgIndex::GetHeading(int heading)
{
	return sm_headings[heading].first.c_str();
}


std::list< int >::const_iterator PkgIndex::HeadingChildren_Begin(int heading)
{
	return sm_headings[heading].second.begin();
}


std::list< int >::const_iterator PkgIndex::HeadingChildren_End(int heading)
{
	return sm_headings[heading].second.end();
}


PkgIndex::PackageIter PkgIndex::Packages_Begin()
{
	return sm_id_packages.begin();
}


PkgIndex::PackageIter PkgIndex::Packages_End()
{
	return sm_id_packages.end();
}


bool PkgIndex::LoadIndex()
{
	Clear();
	std::string mfile = std::string(GetBinDir()) + "\\mingw_avail.mft";
	TiXmlDocument doc(mfile.c_str());
	if (!doc.LoadFile())
	{
		MGError("Couldn't load '%s' as XML", mfile.c_str());
		return false;
	}

	for (TiXmlElement* heading_el =
	  TiXmlHandle(doc.RootElement()->FirstChildElement("package-categories")).
	   FirstChildElement("heading").ToElement();
	 heading_el;
	 heading_el = heading_el->NextSiblingElement("heading"))
	{
		char const* name = NonEmptyAttribute(heading_el, "name");
		if (!name)
			continue;
		sm_headings.push_back(std::make_pair(std::string(name),
		 std::list< int >()));
		for (TiXmlElement* cat_el = heading_el->FirstChildElement("category");
		 cat_el;
		 cat_el = cat_el->NextSiblingElement("category"))
		{
			const char* id = NonEmptyAttribute(cat_el, "id");
			if (!id)
				continue;
			const char* name = NonEmptyAttribute(cat_el, "name");
			if (!name)
				continue;
			sm_headings.back().second.push_back(sm_index_categories.size());
			sm_id_categories[id] = sm_index_categories.size();
			sm_index_categories.push_back(name);
		}
	}

	for (TiXmlElement* package_el =
	  TiXmlHandle(doc.RootElement()->FirstChildElement("package-collection")).
	  FirstChildElement("package").ToElement();
	 package_el;
	 package_el = package_el->NextSiblingElement("package"))
	{
		const char* id = NonEmptyAttribute(package_el, "id");
		if (!id)
			continue;
		StringPackageMap::iterator found = sm_id_packages.find(id);
		if (found != sm_id_packages.end())
			continue;
		Package::Ref newpkg(new Package(id, package_el));
		InsertPackage(newpkg);
	}
	return true;
}


void PkgIndex::Clear()
{
	sm_index_categories.clear();
	sm_id_categories.clear();
	sm_id_packages.clear();
	sm_headings.clear();
}


void PkgIndex::InsertPackage(Package::Ref ins)
{
	sm_id_packages.insert(std::make_pair(ins->m_id, ins));
}
