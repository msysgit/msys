
#include "inst_manager.hpp"

#include <cstdarg>
#include "tracked_install.hpp"
#include "ref.hpp"
#include "package.hpp"
#include "ui.hpp"


char InstManager::sm_lasterror[2048] = {0};
std::string InstManager::sm_inst_loc;
std::vector< std::string > InstManager::sm_categories;
InstManager::IDPackageMap InstManager::sm_id_packages;


const char* InstManager::GetError()
{
	return sm_lasterror;
}


struct DirCreated
{
	std::string rdir;
	DirCreated(const std::string& dir)
	 : rdir(dir)
	{
	}
	bool Revert()
	{
		return RemoveDirectory(rdir.c_str());
	}
};
struct FileCreated
{
	std::string rfile;
	FileCreated(const std::string& file)
	 : rfile(file)
	{
	}
	bool Revert()
	{
		return DeleteFile(rfile.c_str());
	}
};

size_t DownloadFile
 (const char*,
  const char*,
  void (*)(size_t) = 0);

bool InstManager::Load(const std::string& inst_loc, bool create)
{
	sm_inst_loc = inst_loc;
	if (sm_inst_loc[sm_inst_loc.length() - 1] == '\\'
	 || sm_inst_loc[sm_inst_loc.length() - 1] == '/')
		sm_inst_loc.erase(sm_inst_loc.length() - 1);

	ClearPackages();

	ChangeSet::Ref changes(new ChangeSet);
	do
	{
		if (create)
		{
			if (!CreateDirectory(sm_inst_loc.c_str(), 0)
			 && ::GetLastError() != ERROR_ALREADY_EXISTS)
			{
				SetError("Couldn't create the directory '%s'",
				 sm_inst_loc.c_str());
				break;
			}
			changes->Push(DirCreated(sm_inst_loc));

			if (DownloadFile("http://localhost:1330/mingwinst/mingw_avail.mft",
			 (sm_inst_loc + "\\mingw_inst.mft").c_str(), 0) <= 0)
			{
				SetError("Couldn't download the following file:\r\n%s",
				 "http://localhost:1330/mingwinst/mingw_avail.mft");
				break;
			}
			changes->Push(FileCreated(sm_inst_loc + "\\mingw_inst.mft"));

			if (!LoadManifest(sm_inst_loc + "\\mingw_inst.mft"))
				break;

			return true;
		}
		else
		{
			if (!LoadManifest((sm_inst_loc + "\\mingw_inst.mft").c_str()))
				return false;
			return true;
		}
	} while (0);

	// Error
	changes->RevertAll();
	return false;
}


void InstManager::Save()
{
}


int InstManager::NumCategories()
{
	return sm_categories.size();
}


const char* InstManager::GetCategory(int cat)
{
	return sm_categories[cat].c_str();
}


InstManager::PackageIter InstManager::Packages_Begin()
{
	return sm_id_packages.begin();
}


InstManager::PackageIter InstManager::Packages_End()
{
	return sm_id_packages.end();
}


void InstManager::SetError(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(sm_lasterror, 2048, fmt, ap);
	va_end(ap);
}


static const char* NonEmptyAttribute(const TiXmlElement* el, const char* name)
{
	const char* attr = el->Attribute(name);
	if (attr && attr[0])
		return attr;
	return 0;
}

bool InstManager::LoadManifest(const std::string& mfile)
{
	TiXmlDocument doc(mfile.c_str());
	if (!doc.LoadFile())
	{
		SetError("Couldn't load '%s' as XML", mfile.c_str());
		return false;
	}
	int newpkgct = 0;
	for (TiXmlElement* cat_el =
	  doc.RootElement()->FirstChildElement("Category");
	 cat_el;
	 cat_el = cat_el->NextSiblingElement("Category"))
	{
		const char* name = NonEmptyAttribute(cat_el, "name");
		if (!name)
			continue;
		sm_categories.push_back(name);
		for (TiXmlElement* package_el =
		  cat_el->FirstChildElement("Package");
		 package_el;
		 package_el = package_el->NextSiblingElement("Package"))
		{
			const char* id = NonEmptyAttribute(package_el, "id");
			if (!id)
				continue;
			IDPackageMap::iterator found = sm_id_packages.find(id);
			if (found != sm_id_packages.end())
				continue;
			++newpkgct;
			Package::Ref newpkg(
			 new Package(id, sm_categories.size() - 1, package_el)
			 );
			InsertPackage(newpkg);
		}
	}
	if (newpkgct > 0)
		UI::NotifyNewPackages();
	return true;
}


void InstManager::ClearPackages()
{
	sm_categories.clear();
	sm_id_packages.clear();
}


void InstManager::InsertPackage(Package::Ref ins)
{
	sm_id_packages.insert(std::make_pair(ins->m_id, ins));
}
