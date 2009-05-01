/** \file pkgindex.cpp
 *
 * Created: JohnE, 2008-10-09
 */


#include "pkgindex.hh"

#include <vector>
#include <string>
#include <map>
#include <set>
#include <algorithm>
#include "tinyxml/tinyxml.h"
#include "getbindir.hh"
#include "error.hh"
#include "versioncompare.hh"
#include "pkg_const.h"


typedef std::map< std::string, int > StringIntMap;


struct PkgVersion
{
	std::string version;
	int status;

	PkgVersion(const char* ver, int stat)
	 : version(ver),
	 status(stat)
	{
	}
};

struct Package
{
	std::string id;
	std::set< int > categories;
	std::string installed_version;
	std::string title;
	std::string description;
	int selected_action;
	std::vector< PkgVersion > versions;
	int selected_version;

	Package(const char* pid, const TiXmlElement* pack_el);
};

static bool PkgVersionOrder
 (const PkgVersion& p1,
  const PkgVersion& p2)
{
	if (p1.status != p2.status)
		return (p1.status > p2.status);
	return (VersionCompare(p1.version.c_str(), p2.version.c_str()) > 0);
}


static struct PkgIndex
{
	std::vector< std::pair< std::string, std::vector< int > > > headings;
	std::vector< std::pair< std::string, std::string > > categories;
	StringIntMap category_ids;
	std::vector< Package > packages;
	StringIntMap package_ids;
} g_pkgindex;


static const char* NonEmptyAttribute(const TiXmlElement* el, const char* name)
{
	const char* attr = el->Attribute(name);
	return (attr && attr[0]) ? attr : 0;
}


Package::Package(const char* pid, const TiXmlElement* pack_el)
 : id(pid),
 selected_action(ACT_NO_CHANGE),
 selected_version(-1)
{
	for (const TiXmlElement* aff_el = pack_el->FirstChildElement("affiliate");
	 aff_el;
	 aff_el = aff_el->NextSiblingElement("affiliate"))
	{
		const char* aff_id = aff_el->Attribute("id");
		if (aff_id && aff_id[0])
		{
			StringIntMap::const_iterator found =
			 g_pkgindex.category_ids.find(aff_id);
			if (found != g_pkgindex.category_ids.end())
				categories.insert(found->second);
		}
	}
	const TiXmlElement* desc_el = pack_el->FirstChildElement("description");
	if (desc_el)
	{
		const char* ptitle = desc_el->Attribute("title");
		if (ptitle && ptitle[0])
			title = ptitle;
		for (const TiXmlNode* node = desc_el->FirstChild();
		 node;
		 node = node->NextSibling())
		{
			if (node->ToText())
				description += node->Value();
			else if (node->ToElement() && strcmp(node->Value(), "p") == 0)
				description += "\r\n\r\n";
		}
	}
	for (const TiXmlElement* rel_el = pack_el->FirstChildElement("release");
	 rel_el;
	 rel_el = rel_el->NextSiblingElement("release"))
	{
		const char* ver = rel_el->Attribute("version");
		if (ver && ver[0])
		{
			int status = PSTATUS_STABLE;
			const char* stat = rel_el->Attribute("status");
			if (stat)
			{
				if (strcmp(stat, "alpha") == 0)
					status = PSTATUS_ALPHA;
			}
			versions.push_back(PkgVersion(ver, status));
		}
	}
	if (versions.size() > 0)
		std::sort(versions.begin(), versions.end(), PkgVersionOrder);
}


extern "C" int PkgIndex_Load()
{
	PkgIndex_Clear();

	std::string mfile = std::string(GetBinDir()) + "\\mingw_avail.mft";
	TiXmlDocument doc(mfile.c_str());
	if (!doc.LoadFile())
	{
		MGError("Couldn't load '%s' as XML", mfile.c_str());
		return 0;
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
		g_pkgindex.headings.push_back(std::make_pair(std::string(name),
		 std::vector< int >()));
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
			StringIntMap::const_iterator found =
			 g_pkgindex.category_ids.find(id);
			if (found != g_pkgindex.category_ids.end())
				g_pkgindex.headings.back().second.push_back(found->second);
			else
			{
				g_pkgindex.headings.back().second.push_back(
				 g_pkgindex.categories.size()
				 );
				g_pkgindex.category_ids[id] = g_pkgindex.categories.size();
				g_pkgindex.categories.push_back(std::make_pair(std::string(id),
				 std::string(name)));
			}
		}
		g_pkgindex.headings.back().second.push_back(-1);
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
		StringIntMap::const_iterator found = g_pkgindex.package_ids.find(id);
		if (found != g_pkgindex.package_ids.end())
			continue;
		g_pkgindex.packages.push_back(Package(id, package_el));
	}

	return 1;
}


extern "C" void PkgIndex_Clear()
{
	g_pkgindex.headings.clear();
	g_pkgindex.categories.clear();
	g_pkgindex.packages.clear();
	g_pkgindex.package_ids.clear();
}


extern "C" int PkgIndex_NumHeadings()
{
	return g_pkgindex.headings.size();
}


extern "C" const char* PkgIndex_GetHeading(int heading)
{
	return g_pkgindex.headings[heading].first.c_str();
}


extern "C" const int* PkgIndex_GetHeadingChildren(int heading)
{
	return &g_pkgindex.headings[heading].second.front();
}


extern "C" const char* PkgIndex_GetCategory(int category)
{
	return g_pkgindex.categories[category].second.c_str();
}


extern "C" int PkgIndex_NumPackages()
{
	return g_pkgindex.packages.size();
}


extern "C" int PkgIndex_PackageInAnyOf(int package, const int* categories)
{
	for (const int* cat_it = categories; *cat_it != -1; ++cat_it)
	{
		if (g_pkgindex.packages[package].categories.count(*cat_it) > 0)
			return 1;
	}
	return 0;
}


extern "C" const char* PkgIndex_PackageGetID(int package)
{
	return g_pkgindex.packages[package].id.c_str();
}


extern "C" const char* PkgIndex_PackageGetSubItemText(int package, int subitem)
{
	switch (subitem)
	{
	case 1:
		return g_pkgindex.packages[package].id.c_str();
	case 2:
		return g_pkgindex.packages[package].installed_version.c_str();
	case 3:
		if (g_pkgindex.packages[package].versions.size() > 0)
			return g_pkgindex.packages[package].versions.front().version.c_str();
		break;
	case 5:
		return g_pkgindex.packages[package].title.c_str();
	}
	return "";
}


extern "C" const char* PkgIndex_PackageGetTitle(int package)
{
	return g_pkgindex.packages[package].title.c_str();
}


extern "C" const char* PkgIndex_PackageGetDescription(int package)
{
	return g_pkgindex.packages[package].description.c_str();
}


extern "C" const char* PkgIndex_PackageGetInstalledVersion(int package)
{
	return g_pkgindex.packages[package].installed_version.c_str();
}


extern "C" const char* PkgIndex_PackageGetLatestVersion(int package)
{
	return (g_pkgindex.packages[package].versions.empty()) ?
	 "" : g_pkgindex.packages[package].versions.front().version.c_str();
}


extern "C" int PkgIndex_PackageNumVersions(int package)
{
	return g_pkgindex.packages[package].versions.size();
}


extern "C" const char* PkgIndex_PackageVersionGetString
 (int package,
  int version)
{
	return g_pkgindex.packages[package].versions[version].version.c_str();
}


extern "C" void PkgIndex_PackageSetSelectedVersion(int package, int version)
{
	g_pkgindex.packages[package].selected_version = version;
}


extern "C" int PkgIndex_PackageVersionGetStatus(int package, int version)
{
	return g_pkgindex.packages[package].versions[version].status;
}


extern "C" int PkgIndex_PackageGetSelectedAction(int package)
{
	return g_pkgindex.packages[package].selected_action;
}


extern "C" void PkgIndex_PackageSelectAction(int package, int action)
{
	g_pkgindex.packages[package].selected_action = action;
}


extern "C" int PkgIndex_PackageGetStateImage(int package)
{
	const Package& pkg = g_pkgindex.packages[package];
	const char* sel_ver = "";
	if (pkg.selected_version >= 0)
		sel_ver = pkg.versions[pkg.selected_version].version.c_str();
	if (pkg.installed_version.length() <= 0)
	{
		if (pkg.selected_action == ACT_NO_CHANGE)
			return 1;
		else if (pkg.selected_action == ACT_INSTALL_VERSION)
			return 5;
	}
	else
	{
		if (pkg.selected_action == ACT_NO_CHANGE)
			return 2;
		else if (pkg.selected_action == ACT_INSTALL_VERSION)
			return (VersionCompare(pkg.installed_version.c_str(), sel_ver) < 0)
			 ? 7 : 8;
		else if (pkg.selected_action == ACT_REMOVE)
			return 9;
		else if (pkg.selected_action == ACT_REINSTALL)
			return 6;
	}
	return 0;
}
