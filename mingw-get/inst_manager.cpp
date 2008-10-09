
#include "inst_manager.hpp"

#include <cstdarg>
#include "tracked_install.hpp"
#include "ref.hpp"
#include "package.hpp"
#include "ui.hpp"


char InstManager::sm_lasterror[2048] = {0};
std::string InstManager::sm_inst_loc;


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
std::string GetBinDir();

bool InstManager::Load(const std::string& inst_loc, bool create)
{
	sm_inst_loc = inst_loc;
	if (sm_inst_loc[sm_inst_loc.length() - 1] == '\\'
	 || sm_inst_loc[sm_inst_loc.length() - 1] == '/')
		sm_inst_loc.erase(sm_inst_loc.length() - 1);

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

#if 0
			if (DownloadFile("http://localhost:1330/mingwinst/mingw_avail.mft",
			 (GetBinDir() + "\\mingw_avail.mft").c_str(), 0) <= 0)
			{
				SetError("Couldn't download the following file:\r\n%s",
				 "http://localhost:1330/mingwinst/mingw_avail.mft");
				break;
			}
#endif

			return true;
		}
		else
		{
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


void InstManager::SetError(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(sm_lasterror, 2048, fmt, ap);
	va_end(ap);
}
