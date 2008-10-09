
#include "package.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "pkg_index.hpp"
#include "pkg_const.h"


extern "C" const char* Pkg_GetSubItemText(LPARAM lv_lparam, int index)
{
	static std::string vstr;

	Package* pkg = reinterpret_cast< Package* >(lv_lparam);
	switch (index)
	{
	case 1:
		return pkg->m_id.c_str();
	case 2:
		return pkg->m_installed_version.c_str();
	case 3:
		if (pkg->m_versions.size() > 0)
			return pkg->m_versions.front()->m_version.c_str();
		break;
	case 5:
		return pkg->m_title.c_str();
	default:
		break;
	}
	return "";
}


extern "C" const char* Pkg_GetInstalledVersion(LPARAM lv_lparam)
{
	Package* pkg = reinterpret_cast< Package* >(lv_lparam);
	if (pkg->m_installed_version.length() > 0)
		return pkg->m_installed_version.c_str();
	return 0;
}


extern "C" int Pkg_GetStateImage(LPARAM lv_lparam)
{
	return reinterpret_cast< Package* >(lv_lparam)->GetStateImage();
}


extern "C" int Pkg_GetSelectedAction(LPARAM lv_lparam)
{
	return reinterpret_cast< Package* >(lv_lparam)->m_selected_action;
}


extern "C" void Pkg_SelectAction(LPARAM lv_lparam, int action)
{
	reinterpret_cast< Package* >(lv_lparam)->m_selected_action = action;
}


PkgVersion::PkgVersion(const char* ver, int status)
 : m_version(ver),
 m_status(status)
{
}


extern "C" int VersionCompare(const char*, const char*);

static bool PkgVersionOrder
 (const PkgVersion::Ref& p1,
  const PkgVersion::Ref& p2)
{
	if (p1->m_status != p2->m_status)
		return (p1->m_status > p2->m_status);
	return (VersionCompare(p1->m_version.c_str(), p2->m_version.c_str()) > 0);
}


Package::Package(const char* id, const TiXmlElement* pack_el)
 : m_id(id),
 m_selected_action(ACT_NO_CHANGE),
 m_selected_version(-1)
{
	for (const TiXmlElement* aff_el = pack_el->FirstChildElement("affiliate");
	 aff_el;
	 aff_el = aff_el->NextSiblingElement("affiliate"))
	{
		const char* aff_id = aff_el->Attribute("id");
		if (aff_id && aff_id[0])
		{
			int cat = PkgIndex::CategoryIndex(aff_id);
			if (cat >= 0)
				m_categories.insert(cat);
		}
	}
	const TiXmlElement* desc_el = pack_el->FirstChildElement("description");
	if (desc_el)
	{
		const char* title = desc_el->Attribute("title");
		if (title && title[0])
			m_title = title;
		for (const TiXmlNode* node = desc_el->FirstChild();
		 node;
		 node = node->NextSibling())
		{
			if (node->ToText())
				m_description += node->Value();
			else if (node->ToElement() && strcmp(node->Value(), "p") == 0)
				m_description += "\r\n\r\n";
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
			m_versions.push_back(PkgVersion::Ref(new PkgVersion(ver, status)));
		}
	}
	if (m_versions.size() > 0)
		std::sort(m_versions.begin(), m_versions.end(), PkgVersionOrder);
}


int Package::GetStateImage() const
{
	const char* sel_ver = "";
	if (m_selected_version >= 0)
		sel_ver = m_versions[m_selected_version]->m_version.c_str();
	if (m_installed_version.length() <= 0)
	{
		if (m_selected_action == ACT_NO_CHANGE)
			return 1;
		else if (m_selected_action == ACT_INSTALL_VERSION)
			return 5;
	}
	else
	{
		if (m_selected_action == ACT_NO_CHANGE)
			return 2;
		else if (m_selected_action == ACT_INSTALL_VERSION)
			return (VersionCompare(m_installed_version.c_str(), sel_ver) < 0)
			 ? 7 : 8;
		else if (m_selected_action == ACT_REMOVE)
			return 9;
		else if (m_selected_action == ACT_REINSTALL)
			return 6;
	}
	return 0;
}
