#ifndef PACKAGE_HPP_INC
#define PACKAGE_HPP_INC


#include <string>
#include <set>
#include "ref.hpp"
#include "tinyxml/tinyxml.h"


struct Package
{
	typedef RefType< Package >::Ref Ref;

	std::string m_id;
	std::set< int > m_categories;
	std::string m_installed_version;
	std::string m_stable_version;
	std::string m_unstable_version;
	std::string m_description;
	bool m_show_unstable;

	Package(const char* id, const TiXmlElement* pack_el);
};


#endif // PACKAGE_HPP_INC
