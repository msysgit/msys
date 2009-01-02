/** \file archivereadzip.c
 *
 * Created: JohnE, 2008-12-28
 */


#include "archiveread_impl.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <malloc.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <io.h>
#include <sys/stat.h>
#include "zlib/contrib/minizip/unzip.h"


typedef struct
{
	ArchiveReaderStruct base;
	unzFile zf;
	unz_file_info entry_info;
	char entry_path[PATH_MAX + 1];
} ArchiveReaderZip;
#define READERZIP(ptr) ((ArchiveReaderZip*)ptr)


void ArchiveReadZip_Close(void* reader)
{
	if (READERZIP(reader)->zf)
		unzClose(READERZIP(reader)->zf);
	free(reader);
}


int ArchiveReadZip_NextEntry(void* reader)
{
	if (unzGoToNextFile(READERZIP(reader)->zf) != UNZ_OK)
		return 0;
	if (unzGetCurrentFileInfo(READERZIP(reader)->zf,
	 &(READERZIP(reader)->entry_info),
	 READERZIP(reader)->entry_path, PATH_MAX, 0, 0, 0, 0) != UNZ_OK)
		return 0;
	return 1;
}


char const* ArchiveReadZip_GetEntryPath(void* reader)
{
	return READERZIP(reader)->entry_path;
}


int ArchiveReadZip_EntryIsDirectory(void* reader)
{
	return ((READERZIP(reader)->entry_info.external_fa &
	  FILE_ATTRIBUTE_DIRECTORY)
	 || READERZIP(reader)->entry_path[
	   strlen(READERZIP(reader)->entry_path) - 1
	  ] == '/');
}


static int setfiletime (const char *fname,time_t ftime)
{
#ifdef WIN32
  static int isWinNT = -1;
  SYSTEMTIME st;
  FILETIME locft, modft;
  struct tm *loctm;
  HANDLE hFile;
  int result;

  loctm = localtime(&ftime);
  if (loctm == NULL)
    return -1;

  st.wYear         = (WORD)loctm->tm_year + 1900;
  st.wMonth        = (WORD)loctm->tm_mon + 1;
  st.wDayOfWeek    = (WORD)loctm->tm_wday;
  st.wDay          = (WORD)loctm->tm_mday;
  st.wHour         = (WORD)loctm->tm_hour;
  st.wMinute       = (WORD)loctm->tm_min;
  st.wSecond       = (WORD)loctm->tm_sec;
  st.wMilliseconds = 0;
  if (!SystemTimeToFileTime(&st, &locft) ||
      !LocalFileTimeToFileTime(&locft, &modft))
    return -1;

  if (isWinNT < 0)
    isWinNT = (GetVersion() < 0x80000000) ? 1 : 0;
  hFile = CreateFile(fname, GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                     (isWinNT ? FILE_FLAG_BACKUP_SEMANTICS : 0),
                     NULL);
  if (hFile == INVALID_HANDLE_VALUE)
    return -1;
  result = SetFileTime(hFile, NULL, NULL, &modft) ? 0 : -1;
  CloseHandle(hFile);
  return result;
#else
  struct utimbuf settime;

  settime.actime = settime.modtime = ftime;
  return utime(fname,&settime);
#endif
}

int ArchiveReadZip_ExtractEntryToBase
 (void* reader,
  char const* base_path)
{
	char fullpath[PATH_MAX + 1];
	snprintf(fullpath, PATH_MAX + 1, "%s/%s", base_path,
	 READERZIP(reader)->entry_path);

	if (ArchiveReadZip_EntryIsDirectory(reader))
		return ArchiveRead_EnsureDirectory(fullpath, strlen(base_path));

	int rmostsep = strlen(fullpath) - 1;
	while (rmostsep > 0
	 && fullpath[rmostsep] != '/' && fullpath[rmostsep] != '\\')
		--rmostsep;
	if (rmostsep > 0)
	{
		char save = fullpath[rmostsep];
		fullpath[rmostsep] = 0;
		if (!ArchiveRead_EnsureDirectory(fullpath, strlen(base_path)))
		{
			ArchiveRead_SetError((ArchiveReaderStruct*)reader,
			 "Failed to create directory '%s' for extraction", fullpath);
			return 0;
		}
		fullpath[rmostsep] = save;
	}

	if (unzOpenCurrentFile(READERZIP(reader)->zf) != UNZ_OK)
	{
		ArchiveRead_SetError((ArchiveReaderStruct*)reader,
		 "minizip couldn't open entry '%s' for extraction",
		 READERZIP(reader)->entry_path);
		return 0;
	}

	char tmppath[PATH_MAX + 1];
	snprintf(tmppath, PATH_MAX + 1, "%s.tmp", fullpath);

	FILE* outfile = fopen(tmppath, "wb");
	if (!outfile)
	{
		ArchiveRead_SetError((ArchiveReaderStruct*)reader,
		 "Failed to open file '%s' for output", tmppath);
		unzCloseCurrentFile(READERZIP(reader)->zf);
		return 0;
	}

	int const buf_size = 1024 * 1024;
	char* buf = malloc(buf_size);
	int read;
	while ((read = unzReadCurrentFile(READERZIP(reader)->zf, buf, buf_size)) > 0)
	{
		if (fwrite(buf, 1, read, outfile) != read)
		{
			ArchiveRead_SetError((ArchiveReaderStruct*)reader,
			 "Failed to write %d bytes to '%s'", read, tmppath);
			free(buf);
			fclose(outfile);
			remove(tmppath);
			unzCloseCurrentFile(READERZIP(reader)->zf);
			return 0;
		}
	}
	free(buf);
	fclose(outfile);

	int close_result = unzCloseCurrentFile(READERZIP(reader)->zf);
	if (read < 0)
	{
		ArchiveRead_SetError((ArchiveReaderStruct*)reader,
		 "minizip error extracting entry '%s'", READERZIP(reader)->entry_path);
		remove(tmppath);
		return 0;
	}
	if (close_result == UNZ_CRCERROR)
	{
		ArchiveRead_SetError((ArchiveReaderStruct*)reader,
		 "Bad CRC extracting entry '%s'", READERZIP(reader)->entry_path);
		remove(tmppath);
		return 0;
	}

	remove(fullpath);
	rename(tmppath, fullpath);

	setfiletime(fullpath, READERZIP(reader)->entry_info.dosDate);
	SetFileAttributes(fullpath, READERZIP(reader)->entry_info.external_fa);

	return 1;
}


void* ArchiveReadZip_OpenFile(char const* file)
{
	ArchiveReaderZip* reader = malloc(sizeof(ArchiveReaderZip));
	ArchiveRead_InitBase(&reader->base);
	reader->base.is_readable = 0;

	reader->zf = unzOpen(file);
	if (!reader->zf)
	{
		ArchiveRead_SetError(&reader->base, "minizip couldn't open file '%s'",
		 file);
		return reader;
	}

	if (unzGoToFirstFile(reader->zf) != UNZ_OK)
	{
		ArchiveRead_SetError(&reader->base,
		 "minizip couldn't seek to the first entry in archive '%s'", file);
		unzClose(reader->zf);
		reader->zf = 0;
		return reader;
	}
	if (unzGetCurrentFileInfo(reader->zf, &reader->entry_info,
	 reader->entry_path, PATH_MAX, 0, 0, 0, 0) != UNZ_OK)
	{
		ArchiveRead_SetError(&reader->base,
		 "minizip couldn't retrieve entry information from '%s'", file);
		unzClose(reader->zf);
		reader->zf = 0;
		return reader;
	}

	reader->base.internal_close = ArchiveReadZip_Close;
	reader->base.internal_next_entry = ArchiveReadZip_NextEntry;
	reader->base.internal_get_entry_path = ArchiveReadZip_GetEntryPath;
	reader->base.internal_entry_is_directory = ArchiveReadZip_EntryIsDirectory;
	reader->base.internal_extract_entry_to_base =
	 ArchiveReadZip_ExtractEntryToBase;

	reader->base.is_readable = 1;
	return reader;
}
