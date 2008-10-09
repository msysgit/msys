/** \file pkg_index.cpp
 *
 * Created: JohnE, 2008-10-09
 */


#include "pkg_index.hpp"

#include <string>
#include <cstdarg>
#include "tinyxml/tinyxml.h"
#include "ui.hpp"
#include "package.hpp"


char PkgIndex::sm_lasterror[2048] = {0};
std::vector< std::string > PkgIndex::sm_index_categories;
PkgIndex::StringIntMap PkgIndex::sm_id_categories;
std::list< Package::Ref >PkgIndex::sm_packages;
PkgIndex::StringPackageMap PkgIndex::sm_id_packages;


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


PkgIndex::PackageIter PkgIndex::Packages_Begin()
{
	return sm_packages.begin();
}


PkgIndex::PackageIter PkgIndex::Packages_End()
{
	return sm_packages.end();
}


bool PkgIndex::LoadManifest(const std::string& mfile)
{
	TiXmlDocument doc(mfile.c_str());
	if (!doc.LoadFile())
	{
		SetError("Couldn't load '%s' as XML", mfile.c_str());
		return false;
	}
	for (TiXmlElement* cat_el =
	  TiXmlHandle(doc.RootElement()->FirstChildElement("package-categories")).
	  FirstChildElement("category").ToElement();
	 cat_el;
	 cat_el = cat_el->NextSiblingElement("category"))
	{
		const char* id = NonEmptyAttribute(cat_el, "id");
		if (!id)
			continue;
		const char* name = NonEmptyAttribute(cat_el, "name");
		if (!name)
			continue;
		sm_id_categories[id] = sm_index_categories.size();
		sm_index_categories.push_back(name);
		UI::NotifyNewCategory(name);
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
		UI::NotifyNewPackage(*newpkg);
	}
	return true;
}


void PkgIndex::ClearAll()
{
	sm_index_categories.clear();
	sm_id_categories.clear();
	sm_id_packages.clear();
}


void PkgIndex::InsertPackage(Package::Ref ins)
{
	sm_id_packages.insert(std::make_pair(ins->m_id, ins));
}


void PkgIndex::SetError(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(sm_lasterror, 2048, fmt, ap);
	va_end(ap);
}
