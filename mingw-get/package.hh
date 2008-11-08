#ifndef PACKAGE_HH_INC
#define PACKAGE_HH_INC


#define WIN32_LEAN_AND_MEAN
#include <windows.h>


#ifdef __cplusplus

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


extern "C" {
#endif // defined __cplusplus


int Pkg_GetSelectedAction(LPARAM lv_lparam);
const char* Pkg_GetInstalledVersion(LPARAM lv_lparam);
const char* Pkg_GetSubItemText(LPARAM lv_lparam, int index);
void Pkg_SelectAction(LPARAM lv_lparam, int action);
int Pkg_GetStateImage(LPARAM lv_lparam);


#ifdef __cplusplus
} //extern "C"
#endif


#endif // PACKAGE_HH_INC
