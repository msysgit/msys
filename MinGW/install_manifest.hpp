/** \file install_manifest.hpp
 *
 * Created: JohnE, 2008-07-11
 */


/*
DISCLAIMER:
The author(s) of this file's contents release it into the public domain, without
express or implied warranty of any kind. You may use, modify, and redistribute
this file freely.
*/


#ifndef INSTALL_MANIFEST_HPP_INC
#define INSTALL_MANIFEST_HPP_INC


#include <string>
#include <set>
#include <map>
#include "tinyxml/tinyxml.h"


typedef std::string StringType;


class InstallManifest
{
public:
	InstallManifest(const StringType& loadfile = StringType());

	const TiXmlElement* GetComponent(const StringType& comp_id) const;
	TiXmlElement* SetComponent(const StringType& comp_id);
	void AddEntry(const char* entry);

private:
	typedef std::map< std::string, std::pair< TiXmlElement*, std::set< std::string > > > EntrySetMap;
	EntrySetMap entry_setmap;
	EntrySetMap::iterator cur_comp;
};


#endif // INSTALL_MANIFEST_HPP_INC
