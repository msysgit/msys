
#include "package.hpp"

#include "inst_manager.hpp"


extern "C" const char* Pkg_GetSubItemText(LPARAM lv_lparam, int index)
{
	Package* pkg = reinterpret_cast< Package* >(lv_lparam);
	switch (index)
	{
	case 1:
		return pkg->m_id.c_str();
	case 2:
		return pkg->m_installed_version.c_str();
	case 3:
		return pkg->m_stable_version.c_str();
	case 5:
		return pkg->m_description.c_str();
	default:
		break;
	}
	return "";
}


extern "C" int Pkg_UnstableShown(LPARAM lv_lparam)
{
	return (reinterpret_cast< Package* >(lv_lparam)->m_show_unstable) ? 1 : 0;
}


extern "C" void Pkg_SetUnstableShown(LPARAM lv_lparam, int shown)
{
	reinterpret_cast< Package* >(lv_lparam)->m_show_unstable = shown;
}


Package::Package(const char* id, const TiXmlElement* pack_el)
 : m_id(id),
 m_show_unstable(false)
{
	const TiXmlElement* release_el =
	 pack_el->FirstChildElement("stable-release");
	if (release_el)
	{
		const char* rel_ver = release_el->Attribute("version");
		if (rel_ver && rel_ver[0])
			m_stable_version = rel_ver;
	}
	for (const TiXmlElement* aff_el = pack_el->FirstChildElement("affiliate");
	 aff_el;
	 aff_el = aff_el->NextSiblingElement("affiliate"))
	{
		const char* aff_id = aff_el->Attribute("id");
		if (aff_id && aff_id[0])
		{
			int cat = InstManager::CategoryIndex(aff_id);
			if (cat >= 0)
			{
				m_categories.insert(cat);
			}
		}
	}
	const TiXmlElement* desc_el = pack_el->FirstChildElement("description");
	if (desc_el)
	{
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
}
