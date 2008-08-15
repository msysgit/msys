/** \file archive_zip.cpp
 *
 * Created: JohnE, 2008-07-11
 */


/*
DISCLAIMER:
The author(s) of this file's contents release it into the public domain, without
express or implied warranty of any kind. You may use, modify, and redistribute
this file freely.
*/


extern "C"{
#include <io.h>
#include <sys/stat.h>
#include "archive_base.h"
#include "contrib/minizip/unzip.h"
}
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstring>
#include <string>
#include "ref.hpp"


extern int num_files_in_cur_op;
extern int cur_file_in_op_index;


static void CharArrayDeleter(char* ar)
{
	delete[] ar;
}


int unzip
 (const char* file,
  const char* base,
  int (*before_entry_callback)(const char*, int),
  void (*create_callback)(const char*, int))
{
	unzFile zf = unzOpen(file);
	if (!zf)
	{
		archive_seterror("minizip failed to open file '%s'", file);
		return -1;
	}
	RefType< void >::Ref fcloser(zf, unzClose);
	num_files_in_cur_op = 0;
	unz_global_info ginfo;
	if (unzGetGlobalInfo(zf, &ginfo) == UNZ_OK)
		num_files_in_cur_op = ginfo.number_entry;
	if (unzGoToFirstFile(zf) != UNZ_OK)
	{
		archive_seterror("minizip couldn't seek to the first entry in archive "
		 "'%s'", file);
		return -1;
	}
	const int buf_size = 1024 * 1024;
	RefType< char >::Ref buf(new char[buf_size], CharArrayDeleter);
	struct attr_item *attributes = NULL;
	cur_file_in_op_index = 0;
	while (true)
	{
		unz_file_info info;
		char fname[2048];
		if (unzGetCurrentFileInfo(zf, &info, fname, 2047, 0, 0, 0, 0) != UNZ_OK)
		{
			archive_seterror("minizip couldn't retrieve entry information from "
			 "'%s'", file);
			return -1;
		}
		fname[2047] = '\0';
		bool is_dir = false;
		if (info.external_fa & FILE_ATTRIBUTE_DIRECTORY)
			is_dir = true;
		else if (fname[strlen(fname) - 1] == '/')
			is_dir = true;
		if (before_entry_callback)
		{
			int cbres = before_entry_callback(fname, is_dir);
			if (cbres != 0)
			{
				archive_seterror("minizip failed to unzip '%s': cancelled from "
				 "callback", fname);
				return cbres;
			}
		}
		std::string out_path = std::string(base) + "/" + fname;
		if (is_dir)
		{
			if (!makedir(base, fname))
			{
				archive_seterror("couldn't create directory '%s'",
				 out_path.c_str());
				return -1;
			}
		}
		else
		{
			if (unzOpenCurrentFile(zf) != UNZ_OK)
			{
				archive_seterror("minizip couldn't open '%s' for extraction in "
				 "'%s'", fname, file);
				return -1;
			}
			bool was_created = false;
			if (chmod(out_path.c_str(), _S_IREAD|_S_IWRITE) != 0)
				was_created = true;
			FILE* outfile = fopen(out_path.c_str(), "wb");
			if (!outfile)
			{
				size_t p = std::string(fname).find_last_of("/\\", std::string::npos);
				if (p != std::string::npos)
				{
					makedir(base, std::string(fname).substr(0, p).c_str());
					outfile = fopen(out_path.c_str(), "wb");
				}
			}
			if (!outfile)
			{
				unzCloseCurrentFile(zf);
				archive_seterror("couldn't open '%s' for writing",
				 out_path.c_str());
				return -1;
			}
			//if (was_created)
			int read;
			while ((read = unzReadCurrentFile(zf, RefGetPtr(buf), buf_size)) > 0)
			{
				if (read <= 0
				 || fwrite(RefGetPtr(buf), 1, read, outfile)
				  != static_cast< size_t >(read))
				{
					fclose(outfile);
					remove(out_path.c_str());
					archive_seterror("failed to write %d bytes to '%s'",
					 read, out_path.c_str());
					return -1;
				}
			}
			fclose(outfile);
			int close_result = unzCloseCurrentFile(zf);
			if (read < 0)
			{
				remove(out_path.c_str());
				archive_seterror("minizip error reading '%s' from '%s'", fname,
				 file);
				return -1;
			}
			if (close_result == UNZ_CRCERROR)
			{
				remove(out_path.c_str());
				archive_seterror("bad CRC for file '%s' in '%s'", fname, file);
				return -1;
			}
			if (info.external_fa & 0x01)
				chmod(out_path.c_str(), _S_IREAD);
		}
		int mode = _S_IREAD | _S_IWRITE;
		if (info.external_fa & FILE_ATTRIBUTE_READONLY)
			mode &= ~_S_IWRITE;
		push_attr(&attributes, out_path.c_str(), mode, info.dosDate);
		create_callback(fname, is_dir ? 1 : 0);
		int res = unzGoToNextFile(zf);
		if (res == UNZ_END_OF_LIST_OF_FILE)
			break;
		else if (res != UNZ_OK)
		{
			archive_seterror("minizip couldn't seek to the next entry in '%s'",
			 file);
			return -1;
		}
		++cur_file_in_op_index;
	}
	restore_attr(&attributes);
	return 0;
}
