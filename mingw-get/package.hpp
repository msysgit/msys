#ifndef PACKAGE_HPP_INC
#define PACKAGE_HPP_INC


#include <string>
#include <set>
#include <vector>
#include "ref.hpp"
#include "tinyxml/tinyxml.h"


struct PkgVersion
{
	typedef RefType< PkgVersion >::Ref Ref;

	std::string m_version;
	int m_status;

	PkgVersion(const char* ver, int status);
};


struct Package
{
	typedef RefType< Package >::Ref Ref;

	std::string m_id;
	std::set< int > m_categories;
	std::string m_installed_version;
	std::string m_title;
	std::string m_description;
	int m_selected_action;
	std::vector< PkgVersion::Ref > m_versions;
	int m_selected_version;

	Package(const char* id, const TiXmlElement* pack_el);

	int GetStateImage() const;
};


#endif // PACKAGE_HPP_INC
