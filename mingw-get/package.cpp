
#include "package.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


extern "C" const char* PackageGetSubItemText(LPARAM lv_lparam, int index)
{
	Package* pkg = reinterpret_cast< Package* >(lv_lparam);
	switch (index)
	{
	case 1:
		return pkg->m_id.c_str();
	case 3:
		return pkg->m_latest_version.c_str();
	default:
		break;
	}
	return "";
}


Package::Package(const char* id, int cat, const TiXmlElement* pack_el)
 : m_id(id),
 m_category(cat)
{
	const TiXmlElement* latest_ver_el =
	 pack_el->FirstChildElement("LatestVersion");
	if (latest_ver_el)
	{
		const char* latest_ver = latest_ver_el->Attribute("id");
		if (latest_ver && latest_ver[0])
			m_latest_version = latest_ver;
	}
}
