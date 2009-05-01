/** \file archiveread7z.c
 *
 * Created: JohnE, 2008-12-28
 */


#include "archiveread_impl.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <malloc.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "7z/7zCrc.h"
#include "7z/Archive/7z/7zIn.h"
#include "7z/Archive/7z/7zExtract.h"


typedef struct
{
	ISzInStream sz_stream;
	FILE* filep;
} FileInStream;

typedef struct
{
	ArchiveReaderStruct base;
	char* path;
	FileInStream instream;
	CArchiveDatabaseEx db;
	unsigned entry;
	UInt32 blockIndex; /* it can have any value before first call (if outBuffer = 0) */
	Byte* outBuffer; /* it must be 0 before first call for each new archive. */
	size_t outBufferSize;  /* it can have any value before first call (if outBuffer = 0) */
} ArchiveReader7z;
#define READER7Z(ptr) ((ArchiveReader7z*)ptr)


static int g_global_inits = 0;
ISzAlloc g_allocImp, g_allocTempImp;


SZ_RESULT SzFileReadImp
 (void *object,
  void *buffer,
  size_t size,
  size_t *processedSize)
{
	FileInStream* f = (FileInStream*)object;
	int read = fread(buffer, 1, size, f->filep);
	if (processedSize)
		*processedSize = read;
	else if (read <= 0)
		return SZE_FAIL;
	return SZ_OK;
}

SZ_RESULT SzFileSeekImp(void *object, CFileSize pos)
{
	FileInStream* f = (FileInStream*)object;
	int res = fseek(f->filep, (long)pos, SEEK_SET);
	if (res == 0)
		return SZ_OK;
	return SZE_FAIL;
}


void ArchiveRead7z_Close(void* reader)
{
	if (READER7Z(reader)->outBuffer)
		SzFree(READER7Z(reader)->outBuffer);
	if (READER7Z(reader)->instream.filep)
		fclose(READER7Z(reader)->instream.filep);
	if (READER7Z(reader)->path)
		free(READER7Z(reader)->path);
	SzArDbExFree(&(READER7Z(reader)->db), SzFree);
	free(reader);
}


int ArchiveRead7z_NextEntry(void* reader)
{
	if (READER7Z(reader)->entry >= READER7Z(reader)->db.Database.NumFiles)
		return 0;
	++(READER7Z(reader)->entry);
	return 1;
}


char const* ArchiveRead7z_GetEntryPath(void* reader)
{
	return READER7Z(reader)->db.Database.Files[READER7Z(reader)->entry].Name;
}


int ArchiveRead7z_EntryIsDirectory(void* reader)
{
	return READER7Z(reader)->db.Database.Files[READER7Z(reader)->entry].IsDirectory;
}


int ArchiveRead7z_ExtractEntryToBase
 (void* reader,
  char const* base_path)
{
	char fullpath[PATH_MAX + 1];
	snprintf(fullpath, PATH_MAX + 1, "%s/%s", base_path,
	 READER7Z(reader)->db.Database.Files[READER7Z(reader)->entry].Name);

	if (ArchiveRead7z_EntryIsDirectory(reader))
	{
		if (!ArchiveRead_EnsureDirectory(fullpath, strlen(base_path)))
			return 0;
	}
	else
	{
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

		if (!READER7Z(reader)->instream.filep)
		{
			READER7Z(reader)->instream.filep =
			 fopen(READER7Z(reader)->path, "rb");
			if (!READER7Z(reader)->instream.filep)
			{
				ArchiveRead_SetError((ArchiveReaderStruct*)reader,
				 "Failed to reopen file '%s' for entry extraction",
				 READER7Z(reader)->path);
				return 0;
			}
		}

		size_t offset, outSizeProcessed;
		SZ_RESULT res = SzExtract(&(READER7Z(reader)->instream.sz_stream),
		 &(READER7Z(reader)->db), READER7Z(reader)->entry,
		 &(READER7Z(reader)->blockIndex),
		 &(READER7Z(reader)->outBuffer),
		 &(READER7Z(reader)->outBufferSize), &offset, &outSizeProcessed,
		 &g_allocImp, &g_allocTempImp);
		if (res == SZE_CRC_ERROR)
		{
			ArchiveRead_SetError((ArchiveReaderStruct*)reader,
			 "Bad CRC for entry '%s'",
			 READER7Z(reader)->db.Database.Files[READER7Z(reader)->entry].Name);
			return 0;
		}
		else if (res != SZ_OK)
		{
			ArchiveRead_SetError((ArchiveReaderStruct*)reader,
			 "7zlib failed to unpack entry '%s'",
			 READER7Z(reader)->db.Database.Files[READER7Z(reader)->entry].Name);
			return 0;
		}

		char tmppath[PATH_MAX + 1];
		snprintf(tmppath, PATH_MAX + 1, "%s.tmp", fullpath);

		FILE* outfile = fopen(tmppath, "wb");
		if (!outfile)
		{
			ArchiveRead_SetError((ArchiveReaderStruct*)reader,
			 "Failed to open output file '%s'", tmppath);
			return 0;
		}
		fwrite(READER7Z(reader)->outBuffer + offset, 1, outSizeProcessed,
		 outfile);
		fclose(outfile);

		remove(fullpath);
		rename(tmppath, fullpath);
	}

	if (READER7Z(reader)->db.Database.Files[READER7Z(reader)->entry].AreAttributesDefined)
		SetFileAttributes(fullpath, READER7Z(reader)->db.Database.Files[READER7Z(reader)->entry].Attributes);

	return 1;
}


void* ArchiveRead7z_OpenFile(char const* file)
{
	if (!g_global_inits)
	{
		CrcGenerateTable();
		g_allocImp.Alloc = SzAlloc;
		g_allocImp.Free = SzFree;
		g_allocTempImp.Alloc = SzAllocTemp;
		g_allocTempImp.Free = SzFreeTemp;
		g_global_inits = 1;
	}

	ArchiveReader7z* reader = malloc(sizeof(ArchiveReader7z));
	ArchiveRead_InitBase(&reader->base);
	reader->base.is_readable = 0;
	reader->path = 0;
	reader->instream.filep = 0;
	reader->outBuffer = 0;

	SzArDbExInit(&reader->db);

	FileInStream instream;
	instream.sz_stream.Read = SzFileReadImp;
	instream.sz_stream.Seek = SzFileSeekImp;
	instream.filep = fopen(file, "rb");
	if (!instream.filep)
	{
		ArchiveRead_SetError(&reader->base,
		 "Failed to open file '%s' for reading", file);
		return reader;
	}

	SZ_RESULT ret = SzArchiveOpen(&instream.sz_stream, &reader->db, &g_allocImp,
	 &g_allocTempImp);
	fclose(instream.filep);
	instream.filep = 0;
	if (ret != SZ_OK)
	{
		ArchiveRead_SetError(&reader->base,
		 "7zlib refused to open file '%s' as an archive", file);
		return reader;
	}

	reader->entry = 0;
	reader->path = malloc(strlen(file) + 1);
	strcpy(reader->path, file);

	reader->base.internal_close = ArchiveRead7z_Close;
	reader->base.internal_next_entry = ArchiveRead7z_NextEntry;
	reader->base.internal_get_entry_path = ArchiveRead7z_GetEntryPath;
	reader->base.internal_entry_is_directory = ArchiveRead7z_EntryIsDirectory;
	reader->base.internal_extract_entry_to_base =
	 ArchiveRead7z_ExtractEntryToBase;

	reader->base.is_readable = 1;
	return reader;
}
