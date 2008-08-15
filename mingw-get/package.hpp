#ifndef PACKAGE_HPP_INC
#define PACKAGE_HPP_INC


#include <string>
#include "ref.hpp"
#include "tinyxml/tinyxml.h"


struct Package
{
	typedef RefType< Package >::Ref Ref;

	std::string m_id;
	int m_category;
	std::string m_latest_version;

	Package(const char* id, int cat, const TiXmlElement* pack_el);
};


#endif // PACKAGE_HPP_INC
