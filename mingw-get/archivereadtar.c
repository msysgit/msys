/** \file archivereadtar.c
 *
 * Created: JohnE, 2009-01-01
 * Based on contrib/untgz/untgz.c from the zlib distribution
 */


#include "archiveread_impl.h"

#include <malloc.h>
#include <stdio.h>
#include <limits.h>
#include <time.h>
#include <direct.h>
#include "zlib/zlib.h"
#include "bzip2/bzlib.h"


typedef struct
{
	ArchiveReaderStruct base;
	int compression;
	union
	{
		struct
		{
			gzFile infile;
		} gz;
		struct
		{
			FILE* infile;
			BZFILE* inbzfile;
		} bz2;
	} impl;
	char entry_path[PATH_MAX + 1];
	int entry_is_dir;
	int entry_mode;
	time_t entry_time;
	int entry_data;
} ArchiveReaderTar;
#define READERTAR(ptr) ((ArchiveReaderTar*)ptr)


/* values used in typeflag field */

#define REGTYPE  '0'            /* regular file */
#define AREGTYPE '\0'           /* regular file */
#define LNKTYPE  '1'            /* link */
#define SYMTYPE  '2'            /* reserved */
#define CHRTYPE  '3'            /* character special */
#define BLKTYPE  '4'            /* block special */
#define DIRTYPE  '5'            /* directory */
#define FIFOTYPE '6'            /* FIFO special */
#define CONTTYPE '7'            /* reserved */

/* GNU tar extensions */

#define GNUTYPE_DUMPDIR  'D'    /* file names from dumped directory */
#define GNUTYPE_LONGLINK 'K'    /* long link name */
#define GNUTYPE_LONGNAME 'L'    /* long file name */
#define GNUTYPE_MULTIVOL 'M'    /* continuation of file from another volume */
#define GNUTYPE_NAMES    'N'    /* file name that does not fit into main hdr */
#define GNUTYPE_SPARSE   'S'    /* sparse file */
#define GNUTYPE_VOLHDR   'V'    /* tape/volume header */

/* tar header */

#define BLOCKSIZE     512
#define SHORTNAMESIZE 100

struct tar_header
{                               /* byte offset */
  char name[100];               /*   0 */
  char mode[8];                 /* 100 */
  char uid[8];                  /* 108 */
  char gid[8];                  /* 116 */
  char size[12];                /* 124 */
  char mtime[12];               /* 136 */
  char chksum[8];               /* 148 */
  char typeflag;                /* 156 */
  char linkname[100];           /* 157 */
  char magic[6];                /* 257 */
  char version[2];              /* 263 */
  char uname[32];               /* 265 */
  char gname[32];               /* 297 */
  char devmajor[8];             /* 329 */
  char devminor[8];             /* 337 */
  char prefix[155];             /* 345 */
                                /* 500 */
};


static int DecompressOpen
 (ArchiveReaderTar* reader,
  const char* file,
  int compression)
{
	reader->compression = compression;
	if (compression == 1) //BZ2
	{
		reader->impl.bz2.infile = fopen(file, "rb");
		if (!reader->impl.bz2.infile)
		{
			ArchiveRead_SetError(&reader->base,
			 "Couldn't couldn't open file '%s' for reading", file);
			return 0;
		}
		int bzerror;
		reader->impl.bz2.inbzfile =
		 BZ2_bzReadOpen(&bzerror, reader->impl.bz2.infile, 0, 0, 0, 0);
		if (bzerror != BZ_OK)
		{
			ArchiveRead_SetError(&reader->base, "bzip2 couldn't open '%s'",
			 file);
			BZ2_bzReadClose(&bzerror, reader->impl.bz2.inbzfile);
			fclose(reader->impl.bz2.infile);
			reader->impl.bz2.infile = 0;
			return 0;
		}
	}
	else //GZ
	{
		reader->impl.gz.infile = gzopen(file, "rb");
		if (!reader->impl.gz.infile)
		{
			ArchiveRead_SetError(&reader->base,
			 "gzip couldn't open file '%s' for decompression", file);
			return 0;
		}
	}
	return 1;
}

static void DecompressClose(ArchiveReaderTar* reader)
{
	if (reader->compression == 1) //BZ2
	{
		if (reader->impl.bz2.infile)
		{
			int bzerror;
			BZ2_bzReadClose(&bzerror, reader->impl.bz2.inbzfile);
			fclose(reader->impl.bz2.infile);
		}
	}
	else //GZ
	{
		if (reader->impl.gz.infile)
			gzclose(reader->impl.gz.infile);
	}
}

static int DecompressRead(ArchiveReaderTar* reader, char* buffer, int count)
{
	if (reader->compression == 1) //BZ2
	{
		int bzerror;
		int ret = BZ2_bzRead(&bzerror, reader->impl.bz2.inbzfile, buffer,
		 count);
		if (bzerror == BZ_OK)
			return ret;
		else if (bzerror == BZ_STREAM_END)
			return 0;
		return -1;
	}
	return gzread(reader->impl.gz.infile, buffer, count);
}

static int DecompressSeekAhead(ArchiveReaderTar* reader, int dist)
{
	if (reader->compression == 1) //BZ2
	{
		char buf[1024];
		int bzerror;
		while (dist >= 1024)
		{
			if (BZ2_bzRead(&bzerror, reader->impl.bz2.inbzfile, buf, 1024)
			 != 1024)
				return 0;
			dist -= 1024;
		}
		if (BZ2_bzRead(&bzerror, reader->impl.bz2.inbzfile, buf, dist) != dist)
			return 0;
		return 1;
	}
	else //GZ
		return (gzseek(reader->impl.gz.infile, dist, SEEK_CUR) >= 0);
}


static int getoct (char *p,int width)
{
  int result = 0;
  char c;

  while (width--)
    {
      c = *p++;
      if (c == 0)
        break;
      if (c == ' ')
        continue;
      if (c < '0' || c > '7')
        return -1;
      result = result * 8 + (c - '0');
    }
  return result;
}


static int ReadHeaders(ArchiveReaderTar* reader)
{
	int longname = 0;
	char buf[BLOCKSIZE];
	while (1)
	{
		// Read a block
		int read = DecompressRead(reader, buf, BLOCKSIZE);
		if (read != BLOCKSIZE)
			return 0;
		struct tar_header* hdr = (struct tar_header*)buf;
		if (hdr->name[0] == 0)
			return 0;

		// Read a long path
		if (hdr->typeflag == GNUTYPE_LONGLINK
		 || hdr->typeflag == GNUTYPE_LONGNAME)
		{
			int len = getoct(hdr->size, 12);
			if (len < 0 || len >= BLOCKSIZE)
			{
				ArchiveRead_SetError(&reader->base,
				 "Bad length indicator for long name in tar entry");
				return 0;
			}
			if (DecompressRead(reader, reader->entry_path, BLOCKSIZE)
			 != BLOCKSIZE)
			{
				ArchiveRead_SetError(&reader->base,
				 "EOF reading long name block");
				return 0;
			}
			reader->entry_path[len] = 0;
			longname = 1;
			continue;
		}

		if (longname)
		{
			if (strncmp(reader->entry_path, hdr->name, SHORTNAMESIZE - 1) != 0)
			{
				ArchiveRead_SetError(&reader->base,
				 "Mismatched long name in tar entry");
				return 0;
			}
		}
		else
		{
			strncpy(reader->entry_path, hdr->name, SHORTNAMESIZE);
			reader->entry_path[SHORTNAMESIZE - 1] = 0;
		}

		// Get file mode & time
		reader->entry_mode = getoct(hdr->mode, 8);
		if (reader->entry_mode == -1)
		{
			ArchiveRead_SetError(&reader->base,
			 "Bad file mode indicator in tar entry");
			return 0;
		}
		reader->entry_time = (time_t)getoct(hdr->mtime, 12);
		if (reader->entry_time == -1)
		{
			ArchiveRead_SetError(&reader->base,
			 "Bad file time indicator in tar entry");
			return 0;
		}

		if (hdr->typeflag == DIRTYPE)
		{
			reader->entry_is_dir = 1;
			reader->entry_data = 0;
			break;
		}
		if (hdr->typeflag == REGTYPE
		 || hdr->typeflag == AREGTYPE)
		{
			reader->entry_is_dir = 0;
			reader->entry_data = getoct(hdr->size, 12);
			if (reader->entry_data == -1)
			{
				ArchiveRead_SetError(&reader->base,
				 "Bad entry size indicator in tar entry");
				return 0;
			}
			break;
		}
	}
	return 1;
}


void ArchiveReadTar_Close(void* reader)
{
	DecompressClose(READERTAR(reader));
	free(reader);
}


int ArchiveReadTar_NextEntry(void* reader)
{
	if (READERTAR(reader)->entry_data > 0)
	{
		if (!DecompressSeekAhead(reader,
		 ((int)(READERTAR(reader)->entry_data / BLOCKSIZE) + 1) * BLOCKSIZE))
			return 0;
	}
	if (!ReadHeaders(READERTAR(reader)))
		return 0;
	return 1;
}


const char* ArchiveReadTar_GetEntryPath(void* reader)
{
	return READERTAR(reader)->entry_path;
}


int ArchiveReadTar_EntryIsDirectory(void* reader)
{
	return READERTAR(reader)->entry_is_dir;
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

int ArchiveReadTar_ExtractEntryToBase
 (void* reader,
  char const* base_path)
{
	char fullpath[PATH_MAX + 1];
	snprintf(fullpath, PATH_MAX + 1, "%s/%s", base_path,
	 READERTAR(reader)->entry_path);

	if (ArchiveReadTar_EntryIsDirectory(reader))
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

	char tmppath[PATH_MAX + 1];
	snprintf(tmppath, PATH_MAX + 1, "%s.tmp", fullpath);

	FILE* outfile = fopen(tmppath, "wb");
	if (!outfile)
	{
		ArchiveRead_SetError((ArchiveReaderStruct*)reader,
		 "Failed to open file '%s' for output", tmppath);
		return 0;
	}

	char buf[BLOCKSIZE];
	while (READERTAR(reader)->entry_data > 0)
	{
		if (DecompressRead(READERTAR(reader), buf, BLOCKSIZE) != BLOCKSIZE)
		{
			ArchiveRead_SetError((ArchiveReaderStruct*)reader,
			 "Failed to read %d bytes from decompression stream", BLOCKSIZE);
			fclose(outfile);
			remove(tmppath);
			return 0;
		}
		int write = (READERTAR(reader)->entry_data < BLOCKSIZE) ?
		 READERTAR(reader)->entry_data : BLOCKSIZE;
		if (fwrite(buf, 1, write, outfile) != write)
		{
			ArchiveRead_SetError((ArchiveReaderStruct*)reader,
			 "Failed to write %d bytes to '%s'", write, tmppath);
			fclose(outfile);
			remove(tmppath);
			return 0;
		}
		READERTAR(reader)->entry_data -= write;
	}
	fclose(outfile);

	remove(fullpath);
	rename(tmppath, fullpath);

	setfiletime(fullpath, READERTAR(reader)->entry_time);
	chmod(fullpath, READERTAR(reader)->entry_mode);

	return 1;
}


void* ArchiveReadTar_OpenFile(int compression, char const* file)
{
	ArchiveReaderTar* reader = malloc(sizeof(ArchiveReaderTar));
	ArchiveRead_InitBase(&reader->base);
	reader->base.is_readable = 0;

	if (!DecompressOpen(reader, file, compression))
		return reader;

	if (!ReadHeaders(READERTAR(reader)))
		return reader;

	reader->base.internal_close = ArchiveReadTar_Close;
	reader->base.internal_next_entry = ArchiveReadTar_NextEntry;
	reader->base.internal_get_entry_path = ArchiveReadTar_GetEntryPath;
	reader->base.internal_entry_is_directory = ArchiveReadTar_EntryIsDirectory;
	reader->base.internal_extract_entry_to_base =
	 ArchiveReadTar_ExtractEntryToBase;

	reader->base.is_readable = 1;
	return reader;
}
