/** \file archive.cpp
 *
 * Format-agnostic functions for unpacking archives. Called from tdminstall.cpp;
 * calls various functions in archive_7z.cpp, archive_base.c, archive_zip.cpp,
 * and untgz.c.
 */

/* Created: JohnE, 2008-07-11
 */


/*
DISCLAIMER:
The author(s) of this file's contents release it into the public domain, without
express or implied warranty of any kind. You may use, modify, and redistribute
this file freely.
*/


#include <cstring>
#include <string>
extern "C" {
#include "multiread.h"
#include "archive_base.h"
}
#include "install_manifest.hpp"
#include "ref.hpp"


enum ArcType
{
	ARCT_UNKNOWN = 0,
	ARCT_TAR_GZ,
	ARCT_TAR_BZ2,
	ARCT_ZIP,
	ARCT_7Z
};


// Set when InstallArchive is called; called from ArcBeforeCallback to indicate
// that an entry is about to be unpacked
int (*ia_callback)(const char*, bool, bool) = 0;

// Set when InstallArchive is called; entries are added to it in
// ArcCreateCallback
InstallManifest* inst_man = 0;


/* Returns true if file name "file" ends with "ext" */
static bool HasExtension(const std::string& file, const char* ext)
{
	size_t elen = strlen(ext);
	if (file.length() > elen
	 && stricmp(file.c_str() + file.length() - elen, ext) == 0)
		return true;
	return false;
}


/* Called from the various format-specific unpacking functions to indicate that
 * an entry is about to be unpacked. "fpath" is the entry's path (relative to
 * base); is_dir is nonzero if the entry is a directory rather than a file. */
extern "C" int ArcBeforeCallback(const char* fpath, int is_dir)
{
	if (ia_callback)
		return ia_callback(fpath, is_dir, false);
	return 0;
}


/* Called from the various format-specific unpacking functions to indicate that
 * an entry has been unpacked to disk. "fpath" is the entry's path (relative to
 * base); is_dir is nonzero if the entry is a directory rather than a file. */
extern "C" void ArcCreateCallback(const char* fpath, int is_dir)
{
	if (inst_man)
	{
		if (is_dir)
		{
			// Ensure that the path ends in a slash if it is a directory
			int plen = strlen(fpath);
			if (fpath[plen - 1] == '/' || fpath[plen - 1] == '\\')
				inst_man->AddEntry(fpath);
			else
				inst_man->AddEntry((std::string(fpath) + "/").c_str());
		}
		else
			inst_man->AddEntry(fpath);
	}
}


// defined in untgz.c
extern "C" int tar
 (MultiReader*,
  const char*,
  int (*)(const char*, int),
  void (*)(const char*, int));

// defined in archive_zip.cpp
int unzip
 (const char* file,
  const char* base,
  int (*before_entry_callback)(const char*, int),
  void (*create_callback)(const char*, int));

// defined in archive_7z.cpp
int un7z
 (const char* file,
  const char* base,
  int (*before_entry_callback)(const char*, int),
  void (*create_callback)(const char*, int));


/* Unpack an archive to the specified location, with callbacks. "base" is the
 * location to unpack to, "archive" is the file to unpack, "man" is the
 * installation manifest to add unpacked entries to, and "callback" is a
 * function to call right before each entry is unpacked. "callback"'s arguments
 * are the path (relative to base) of the entry, a boolean that is true if the
 * entry is a directory, and a boolean that is false if the entry is being
 * created (in other words, always false for InstallArchive) */
std::string InstallArchive
 (const char* base,
  const std::string& archive,
  InstallManifest& man,
  int (*callback)(const char*, bool, bool))
{
	ia_callback = callback;
	inst_man = &man;

	if (archive.length() <= 0)
		return "No archive given";
	else if (archive.length() < 5)
		return std::string("Archive name '") + archive + "' is too short.";

	// Determine the archive type by the file's extension
	ArcType type;
	if (HasExtension(archive, ".tar.gz")
	 || HasExtension(archive, ".tgz"))
		type = ARCT_TAR_GZ;
	else if (HasExtension(archive, ".tar.bz2")
	 || HasExtension(archive, ".tbz")
	 || HasExtension(archive, ".tbz2"))
		type = ARCT_TAR_BZ2;
	else if (HasExtension(archive, ".zip"))
		type = ARCT_ZIP;
	else if (HasExtension(archive, ".7z"))
		type = ARCT_7Z;
	else
	{
		return std::string("Archive '") + archive +
		 "' doesn't have a recognized extension";
	}

	switch (type)
	{
	case ARCT_TAR_GZ:
		{
			// Create gzip reader and use the "tar" function to unpack
			RefType< MultiReader >::Ref mr(CreateGZReader(archive.c_str()),
			 DestroyMultiReader);
			if (!mr)
			{
				return std::string("zlib failed to open file '") + archive +
				 "'";
			}
			int res = tar(RefGetPtr(mr), base, ArcBeforeCallback,
			 ArcCreateCallback);
			if (res != 0)
			{
				const char* emsg = "cancelled from callback";
				if (res == -1)
					emsg = archive_geterror();
				return std::string("tar failed to untar file '") + archive +
				 "': " + emsg;
			}
		}
		break;
	case ARCT_TAR_BZ2:
		{
			// Create bzip2 reader and use the tar function to unpack
			RefType< MultiReader >::Ref mr(CreateBZ2Reader(archive.c_str()),
			 DestroyMultiReader);
			if (!mr)
			{
				return std::string("bzip2 failed to open file '") + archive +
				 "'";
			}
			int res = tar(RefGetPtr(mr), base, ArcBeforeCallback,
			 ArcCreateCallback);
			if (res != 0)
			{
				const char* emsg = "cancelled from callback";
				if (res == -1)
					emsg = archive_geterror();
				return std::string("tar failed to untar file '") + archive +
				 "': " + emsg;
			}
		}
		break;
	case ARCT_ZIP:
		{
			// Use unzip function to unpack
			int res = unzip(archive.c_str(), base, ArcBeforeCallback,
			 ArcCreateCallback);
			if (res != 0)
			{
				const char* emsg = "cancelled from callback";
				if (res == -1)
					emsg = archive_geterror();
				return std::string("minizip failed to unzip file '") + archive +
				 "': " + emsg;
			}
		}
		break;
	case ARCT_7Z:
		{
			// Use un7z function to unpack
			int res = un7z(archive.c_str(), base, ArcBeforeCallback,
			 ArcCreateCallback);
			if (res != 0)
			{
				const char* emsg = "cancelled from callback";
				if (res == -1)
					emsg = archive_geterror();
				return std::string("7zlib failed to unpack '") + archive +
				 "': " + emsg;
			}
		}
		break;
	default:
		break;
	}
	return "OK";
}

