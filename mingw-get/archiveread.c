/** \file archiveread.c
 *
 * Created: JohnE, 2008-12-28
 */


#include "archiveread_impl.h"

#include <limits.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <malloc.h>
#include <errno.h>
#include <direct.h>


void* ArchiveRead7z_OpenFile(char const*);
void* ArchiveReadZip_OpenFile(char const*);
void* ArchiveReadTar_OpenFile(int, char const*);

ArchiveReader* ArchiveRead_OpenFileAuto(char const* file)
{
	ArchiveReaderStruct* reader = 0;
	int flen = strlen(file);
	if (stricmp(file + flen - 3, ".7z") == 0)
		reader = ArchiveRead7z_OpenFile(file);
	else if (stricmp(file + flen - 4, ".zip") == 0)
		reader = ArchiveReadZip_OpenFile(file);
	else if (stricmp(file + flen - 7, ".tar.gz") == 0
	 || stricmp(file + flen - 4, ".tgz") == 0)
		reader = ArchiveReadTar_OpenFile(0, file);
	else if (stricmp(file + flen - 8, ".tar.bz2") == 0
	 || stricmp(file + flen - 5, ".tbz2") == 0)
		reader = ArchiveReadTar_OpenFile(1, file);
	return (void*)reader;
}


void ArchiveRead_Close(ArchiveReader* reader)
{
	if (((ArchiveReaderStruct*)reader)->error)
		free(((ArchiveReaderStruct*)reader)->error);
	((ArchiveReaderStruct*)reader)->internal_close(reader);
}


int ArchiveRead_IsReadable(ArchiveReader* reader)
{
	return ((ArchiveReaderStruct*)reader)->is_readable;
}


int ArchiveRead_NextEntry(ArchiveReader* reader)
{
	return ((ArchiveReaderStruct*)reader)->internal_next_entry(reader);
}


char const* ArchiveRead_GetEntryPath(ArchiveReader* reader)
{
	return ((ArchiveReaderStruct*)reader)->internal_get_entry_path(reader);
}


int ArchiveRead_EntryIsDirectory(ArchiveReader* reader)
{
	return ((ArchiveReaderStruct*)reader)->internal_entry_is_directory(reader);
}


int ArchiveRead_ExtractEntryToBase
 (ArchiveReader* reader,
  char const* base_path)
{
	return ((ArchiveReaderStruct*)reader)->internal_extract_entry_to_base(reader, base_path);
}


char const* ArchiveRead_GetError(ArchiveReader* reader)
{
	return ((ArchiveReaderStruct*)reader)->error ?
	 ((ArchiveReaderStruct*)reader)->error : "";
}


void ArchiveRead_InitBase(ArchiveReaderStruct* base)
{
	base->error = 0;
}


void ArchiveRead_SetError
 (ArchiveReaderStruct* reader,
  char const* fmt,
  ...)
{
	if (!reader->error)
		reader->error = malloc(PATH_MAX * 2);
	va_list args;
	va_start(args, fmt);
	vsnprintf(reader->error, PATH_MAX * 2 - 1, fmt, args);
	va_end(args);
}


int ArchiveRead_EnsureDirectory(char const* path, int base_len)
{
	char expath[PATH_MAX + 1];
	snprintf(expath, PATH_MAX + 1, "%s/", path);
	int plen = strlen(expath);
	int i = base_len + 1;
	for (; i < plen; ++i)
	{
		if (expath[i] == '/' || expath[i] == '\\')
		{
			char save = expath[i];
			expath[i] = 0;
			if (_mkdir(expath) != 0 && errno != EEXIST)
				return 0;
			expath[i] = save;
		}
	}
	return 1;
}
