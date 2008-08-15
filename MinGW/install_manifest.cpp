/** \file install_manifest.cpp
 *
 * Created: JohnE, 2008-07-11
 */


/*
DISCLAIMER:
The author(s) of this file's contents release it into the public domain, without
express or implied warranty of any kind. You may use, modify, and redistribute
this file freely.
*/


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "install_manifest.hpp"

#include <cstring>
#include <list>


InstallManifest::InstallManifest(const StringType& loadfile)
 : cur_comp(entry_setmap.end())
{
	TiXmlDocument doc(loadfile.c_str());
	if (doc.LoadFile())
	{
		std::list< TiXmlElement* > search_els;
		search_els.push_back(doc.RootElement());
		while (!search_els.empty())
		{
			TiXmlElement* el = search_els.front();
			search_els.pop_front();
			TiXmlElement* ent = el->FirstChildElement("Entry");
			if (ent)
			{
				const char* cid = el->Attribute("id");
				if (cid && strlen(cid) > 0)
				{
					SetComponent(cid);
					for (; ent; ent = ent->NextSiblingElement("Entry"))
						AddEntry(ent->FirstChild()->ToText()->Value());
				}
			}
			else
			{
				for (TiXmlElement* child = el->FirstChildElement();
				 child;
				 child = child->NextSiblingElement())
					search_els.push_back(child);
			}
		}
	}
}


const TiXmlElement* InstallManifest::GetComponent(const StringType& comp_id) const
{
	EntrySetMap::const_iterator found = entry_setmap.find(comp_id);
	return (found == entry_setmap.end()) ? 0 : found->second.first;
}


TiXmlElement* InstallManifest::SetComponent(const StringType& comp_id)
{
	EntrySetMap::iterator found = entry_setmap.find(comp_id);
	if (found != entry_setmap.end())
	{
		cur_comp = found;
		return found->second.first;
	}
	else
	{
		TiXmlElement* new_comp = new TiXmlElement("Component");
		new_comp->SetAttribute("id", comp_id.c_str());
		cur_comp = entry_setmap.insert(
		 std::make_pair(
		  std::string(comp_id),
		  std::make_pair(new_comp, std::set< std::string >())
		  )
		 ).first;
		return new_comp;
	}
}


void InstallManifest::AddEntry(const char* entry)
{
	if (cur_comp == entry_setmap.end())
		return;
	if (cur_comp->second.second.insert(entry).second)
	{
		TiXmlElement* ent_el = new TiXmlElement("Entry");
		TiXmlText* ent_txt = new TiXmlText(entry);
		ent_txt->SetCDATA(true);
		ent_el->LinkEndChild(ent_txt);
		cur_comp->second.first->LinkEndChild(ent_el);
	}
}

