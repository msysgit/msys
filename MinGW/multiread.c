/** \file multiread.c
 *
 * Created: JohnE, 2008-07-11
 */


/*
DISCLAIMER:
The author(s) of this file's contents release it into the public domain, without
express or implied warranty of any kind. You may use, modify, and redistribute
this file freely.
*/


#include "multiread.h"

#include <malloc.h>
#include "zlib.h"
#include "bzlib.h"


enum
{
	MRT_UNDEF = 0,
	MRT_GZ = 1,
	MRT_BZ2 = 2
};


struct _MultiReader
{
	int type;
	union
	{
		gzFile gz;
		struct
		{
			FILE* filep;
			BZFILE* bzfilep;
		} bz2;
	} file;
};


MultiReader* CreateGZReader(const char *file)
{
	MultiReader* mr = malloc(sizeof(MultiReader));
	mr->type = MRT_GZ;
	mr->file.gz = gzopen(file, "rb");
	if (!mr->file.gz)
	{
		free(mr);
		return 0;
	}
	return mr;
}


MultiReader* CreateBZ2Reader(const char *file)
{
	MultiReader* mr = malloc(sizeof(MultiReader));
	mr->type = MRT_BZ2;
	mr->file.bz2.filep = fopen(file, "rb");
	if (!mr->file.bz2.filep)
	{
		free(mr);
		return 0;
	}
	int bzerror;
	mr->file.bz2.bzfilep = BZ2_bzReadOpen(&bzerror, mr->file.bz2.filep, 0, 0, 0, 0);
	if (bzerror != BZ_OK)
	{
		BZ2_bzReadClose(&bzerror, mr->file.bz2.bzfilep);
		fclose(mr->file.bz2.filep);
		free(mr);
		return 0;
	}
	return mr;
}


void DestroyMultiReader(MultiReader *mr)
{
	if (!mr)
		return;
	switch (mr->type)
	{
	case MRT_GZ:
		if (mr->file.gz)
			gzclose(mr->file.gz);
		break;
	case MRT_BZ2:
		{
			int bzerror;
			if (mr->file.bz2.bzfilep)
				BZ2_bzReadClose(&bzerror, mr->file.bz2.bzfilep);
			if (mr->file.bz2.filep)
				fclose(mr->file.bz2.filep);
		}
		break;
	default:
		break;
	}
	free(mr);
}


int MultiRead(MultiReader *mr, void *buf, unsigned len)
{
	if (!mr)
		return -1;
	switch (mr->type)
	{
	case MRT_GZ:
		return gzread(mr->file.gz, buf, len);
	case MRT_BZ2:
		{
			int bzerror;
			int ret = BZ2_bzRead(&bzerror, mr->file.bz2.bzfilep, buf, len);
			if (bzerror == BZ_OK)
				return ret;
			else if (bzerror == BZ_STREAM_END)
				return 0;
			return -1;
		}
	default:
		break;
	}
	return -1;
}


const char *MultiError(MultiReader *mr, int *errnum)
{
	if (!mr)
		return "";
	switch (mr->type)
	{
	case MRT_GZ:
		return gzerror(mr->file.gz, errnum);
	case MRT_BZ2:
		return BZ2_bzerror(mr->file.bz2.bzfilep, errnum);
	default:
		break;
	}
	return "";
}
