/** \file archive_7z.cpp
 *
 * Unpack .7z (7-Zip) archives. Important: based on 7-zip unpacker which is
 * very-slightly-modified from the original LZMA distribution in that it also
 * reads file attributes.
 */

/* Created: JohnE, 2008-07-13
 */


/*
DISCLAIMER:
The author(s) of this file's contents release it into the public domain, without
express or implied warranty of any kind. You may use, modify, and redistribute
this file freely.
*/


#include <string>
#include <cstdio>
#include "ref.hpp"
extern "C" {
#include <sys/stat.h>
#include "7z/7zCrc.h"
#include "7z/Archive/7z/7zIn.h"
#include "7z/Archive/7z/7zExtract.h"
#include "archive_base.h"
}
#define WIN32_LEAN_AND_MEAN
#include <windows.h>


extern "C"
{

/* 7-zip wants use to define a structure that it can pun to an ISzInStream and
 * then pass back to us in a callback, so that we can pun it back to what it
 * really is. Yay, C polymorphism!
 */
typedef struct
{
	ISzInStream sz_stream;
	FILE* filep;
} FileInStream;


/* 7-zip needs functions that it can call on the object we provide to read and
 * seek through data.
 */
static SZ_RESULT SzFileReadImp
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
static SZ_RESULT SzFileSeekImp(void *object, CFileSize pos)
{
	FileInStream* f = (FileInStream*)object;
	int res = fseek(f->filep, (long)pos, SEEK_SET);
	if (res == 0)
		return SZ_OK;
	return SZE_FAIL;
}

} // extern "C"


// Progress counters; FIXME: don't use globals...
extern int num_files_in_cur_op;
extern int cur_file_in_op_index;


/* Scoped deleter function for the 7-zip archive database */
static void SzArDbExDeleter(CArchiveDatabaseEx* db)
{
	SzArDbExFree(db, SzFree);
}

/* Scoped deleter function for 7-zip's memory buffer */
static void OutBufferDeleter(Byte* ob)
{
	if (ob)
		SzFree(ob);
}


/* Extract all the files from a 7-zip archive to the specified location, with
 * callbacks. "file" is the path to the file to unpack, "base" is the location
 * to extract to, "before_entry_callback" is a function to call right before
 * each entry is unpacked, and "create_callback" is a function to call right
 * after each entry has been unpacked. Both callback functions' arguments are as
 * follows: the path (relative to base) of the entry, and an int that is nonzero
 * if the entry is a directory.
 */
int un7z
 (const char* file,
  const char* base,
  int (*before_entry_callback)(const char*, int),
  void (*create_callback)(const char*, int))
{
	// Create and fill out our reader object
	FileInStream instream;
	instream.sz_stream.Read = SzFileReadImp;
	instream.sz_stream.Seek = SzFileSeekImp;
	instream.filep = fopen(file, "rb");
	if (!instream.filep)
	{
		archive_seterror("failed to open file '%s' for reading", file);
		return -1;
	}
	// Close the FILE* when we leave this scope
	RefType< FILE >::Ref fcloser(instream.filep, fclose);

	CrcGenerateTable();

	CArchiveDatabaseEx db;
	SzArDbExInit(&db);
	// Free up the database when we leave this scope
	RefType< CArchiveDatabaseEx >::Ref db_deleter(&db, SzArDbExDeleter);

	// Set up 7-zip's memory allocation scheme (uses the default allocators)
	ISzAlloc allocImp, allocTempImp;
	allocImp.Alloc = SzAlloc;
	allocImp.Free = SzFree;
	allocTempImp.Alloc = SzAllocTemp;
	allocTempImp.Free = SzFreeTemp;

	// Open the archive
	if (SzArchiveOpen(&instream.sz_stream, &db, &allocImp, &allocTempImp)
	 != SZ_OK)
	{
		archive_seterror("7zlib couldn't open '%s' as a 7z archive", file);
		return -1;
	}

	// Update progress total
	num_files_in_cur_op = db.Database.NumFiles;

	// Set up 7-zip's "cache" variables
	UInt32 blockIndex = 0xFFFFFFFF;
	Byte *outBuffer = 0;
	size_t outBufferSize = 0;
	// Delete the buffer when we leave this scope
	RefType< Byte >::Ref outbuf_deleter(outBuffer, OutBufferDeleter);

	// Iterate and extract entries
	for (unsigned i = 0; i < db.Database.NumFiles; i++)
	{
		// Update progress
		cur_file_in_op_index = i;

		CFileItem* cur_file = &db.Database.Files[i];
		
		// Call before-extraction callback
		if (before_entry_callback)
		{
			int cbres = before_entry_callback(cur_file->Name, cur_file->IsDirectory);
			if (cbres != 0)
			{
				archive_seterror("7zlib failed to unzip '%s': cancelled from "
				 "callback", cur_file->Name);
				return cbres;
			}
		}

		// Get the full path to output
		std::string out_path = std::string(base) + "/" + cur_file->Name;

		// If entry is a directory, merely create the directory
		if (cur_file->IsDirectory)
		{
			if (!makedir(base, cur_file->Name))
			{
				archive_seterror("couldn't create directory '%s'",
				 out_path.c_str());
				return -1;
			}
		}
		// If entry is a file, extract it
		else
		{
			size_t offset, outSizeProcessed;
			// 7-zip seems to encourage getting the whole file in one blow
			SZ_RESULT res = SzExtract(&instream.sz_stream, &db, i, &blockIndex,
			 &outBuffer, &outBufferSize, &offset, &outSizeProcessed, &allocImp,
			 &allocTempImp);
			if (res == SZE_CRC_ERROR)
			{
				archive_seterror("bad CRC for file '%s' in '%s'",
				 cur_file->Name, file);
				return -1;
			}
			else if (res != SZ_OK)
			{
				archive_seterror("7zlib failed to unpack file '%s' in '%s'",
				 cur_file->Name, file);
				return -1;
			}

			bool was_created = false;
			// If the output file already exists, make sure we can write over it
			if (chmod(out_path.c_str(), _S_IREAD|_S_IWRITE) != 0)
				was_created = true;

			// Open the output file for writing
			FILE* outfile = fopen(out_path.c_str(), "wb");
			if (!outfile)
			{
				// Couldn't open it -- make sure its parent directory exists
				size_t p = std::string(cur_file->Name).find_last_of("/\\", std::string::npos);
				if (p != std::string::npos)
				{
					makedir(base, std::string(cur_file->Name).substr(0, p).c_str());
					outfile = fopen(out_path.c_str(), "wb");
				}
			}
			if (!outfile)
			{
				// Still couldn't open it; error out
				archive_seterror("couldn't open '%s' for writing",
				 out_path.c_str());
				return -1;
			}

			// Write out the whole file and then close it
			int written = fwrite(outBuffer + offset, 1, outSizeProcessed,
			 outfile);
			fclose(outfile);
			if (written <= 0
			 || static_cast< size_t >(written) != outSizeProcessed)
			{
				// At this point we have an empty output file; remove it
				remove(out_path.c_str());
				archive_seterror("failed to write %d bytes to '%s'",
				 outSizeProcessed, out_path.c_str());
				return -1;
			}
		}

		// Set stored attributes
		if (cur_file->AreAttributesDefined)
			SetFileAttributes(out_path.c_str(), cur_file->Attributes);

		// Call after-creation callback
		create_callback(cur_file->Name, cur_file->IsDirectory ? 1 : 0);
	}

	return 0;
}

