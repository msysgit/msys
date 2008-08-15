/*
 * untgz.c -- Display contents and extract files from a gzip'd TAR file
 *
 * written by Pedro A. Aranda Gutierrez <paag@tid.es>
 * adaptation to Unix by Jean-loup Gailly <jloup@gzip.org>
 * various fixes by Cosmin Truta <cosmint@cs.ubbcluj.ro>
 * modified for extracting only with custom MultiReader by John E.
 *   <tdragon _at_ tdragon.net>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>

#include "multiread.h"
#include "archive_base.h"

#ifdef unix
#  include <unistd.h>
#else
#  include <direct.h>
#  include <io.h>
#endif

#ifdef WIN32
#include <windows.h>
#  ifndef F_OK
#    define F_OK  0
#  endif
#  define mkdir(dirname,mode)   _mkdir(dirname)
#  ifdef _MSC_VER
#    define access(path,mode)   _access(path,mode)
#    define chmod(path,mode)    _chmod(path,mode)
#    define strdup(str)         _strdup(str)
#  endif
#else
#  include <utime.h>
#endif


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

union tar_buffer
{
  char               buffer[BLOCKSIZE];
  struct tar_header  header;
};

enum { TGZ_EXTRACT, TGZ_INVALID };

#ifndef OF /* function prototypes */
#  define OF(args)  args
#endif


int getoct              OF((char *, int));
char *strtime           OF((time_t *));

const char *TGZsuffix[] = { "\0", ".tar", ".tar.gz", ".taz", ".tgz", NULL };

extern int num_files_in_cur_op;
extern int cur_file_in_op_index;


/* convert octal digits to int */
/* on error return -1 */

int getoct (char *p,int width)
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


/* convert time_t to string */
/* use the "YYYY/MM/DD hh:mm:ss" format */

char *strtime (time_t *t)
{
  struct tm   *local;
  static char result[32];

  local = localtime(t);
  sprintf(result,"%4d/%02d/%02d %02d:%02d:%02d",
          local->tm_year+1900, local->tm_mon+1, local->tm_mday,
          local->tm_hour, local->tm_min, local->tm_sec);
  return result;
}


/* tar file extract */

int tar
 (MultiReader *in,
  const char *base,
  int (*before_entry_callback)(const char*, int),
  void (*create_callback)(const char*, int))
{
  union  tar_buffer buffer;
  int    len;
  int    err;
  int    getheader = 1;
  int    remaining = 0;
  FILE   *outfile = NULL;
  char   fname_full[BLOCKSIZE + 2048];
  char   *fname;
  int    tarmode;
  time_t tartime;
  struct attr_item *attributes = NULL;
  int    action = TGZ_EXTRACT;

  if (!base || strlen(base) < 3)
	{
      archive_seterror("No base path given");
      return -1;
    }
  strncpy(fname_full, base, BLOCKSIZE + 2047);
  fname_full[strlen(base)] = '/';
  fname = fname_full + strlen(base) + 1;

  num_files_in_cur_op = 0;
  cur_file_in_op_index = 0;

  while (1)
    {
      len = MultiRead(in, &buffer, BLOCKSIZE);
      if (len < 0)
        {
          archive_seterror(MultiError(in, &err));
          return -1;
        }
      /*
       * Always expect complete blocks to process
       * the tar information.
       */
      if (len != BLOCKSIZE)
        {
          action = TGZ_INVALID; /* force error exit */
          remaining = 0;        /* force I/O cleanup */
        }

      /*
       * If we have to get a tar header
       */
      if (getheader >= 1)
        {
          /*
           * if we met the end of the tar
           * or the end-of-tar block,
           * we are done
           */
          if (len == 0 || buffer.header.name[0] == 0)
            break;

          tarmode = getoct(buffer.header.mode,8);
          tartime = (time_t)getoct(buffer.header.mtime,12);
          if (tarmode == -1 || tartime == (time_t)-1)
            {
              buffer.header.name[0] = 0;
              action = TGZ_INVALID;
            }

          if (getheader == 1)
            {
              strncpy(fname,buffer.header.name,SHORTNAMESIZE);
              if (fname[SHORTNAMESIZE-1] != 0)
                  fname[SHORTNAMESIZE] = 0;
            }
          else
            {
              /*
               * The file name is longer than SHORTNAMESIZE
               */
              if (strncmp(fname,buffer.header.name,SHORTNAMESIZE-1) != 0)
                {
                  archive_seterror("bad long name");
                  return -1;
                }
              getheader = 1;
            }

          /*
           * Act according to the type flag
           */
          switch (buffer.header.typeflag)
            {
            case DIRTYPE:
              err = before_entry_callback(fname, 1);
              if (err != 0)
				return err;
              makedir(base, fname);
              if (push_attr(&attributes,fname_full,tarmode,tartime) != 0)
				return -1;
              create_callback(fname, 1);
              ++cur_file_in_op_index;
              break;
            case REGTYPE:
            case AREGTYPE:
              remaining = getoct(buffer.header.size,12);
              if (remaining == -1)
                {
                  action = TGZ_INVALID;
                  break;
                }
              err = before_entry_callback(fname, 0);
              if (err != 0)
				return err;
              int was_created = 0;
              if (chmod(fname_full, _S_IREAD|_S_IWRITE) != 0)
                was_created = 1;
              outfile = fopen(fname_full,"wb");
              if (outfile == NULL) {
                /* try creating directory */
                char *p = strrchr(fname_full, '/');
                if (p != NULL) {
                  *p = '\0';
                  makedir(base, fname);
                  *p = '/';
                  outfile = fopen(fname_full,"wb");
                }
              }
              if (outfile == NULL)
                {
                  archive_seterror("Couldn't create %s",fname_full);
                  return -1;
                }
              create_callback(fname, 0);
              ++cur_file_in_op_index;
              getheader = 0;
              break;
            case GNUTYPE_LONGLINK:
            case GNUTYPE_LONGNAME:
              remaining = getoct(buffer.header.size,12);
              if (remaining < 0 || remaining >= BLOCKSIZE)
                {
                  action = TGZ_INVALID;
                  break;
                }
              len = MultiRead(in, fname, BLOCKSIZE);
              if (len < 0)
                {
                  archive_seterror(MultiError(in, &err));
                  return -1;
                }
              if (fname[BLOCKSIZE-1] != 0 || (int)strlen(fname) > remaining)
                {
                  action = TGZ_INVALID;
                  break;
                }
              getheader = 2;
              break;
            default:
              break;
            }
        }
      else
        {
          unsigned int bytes = (remaining > BLOCKSIZE) ? BLOCKSIZE : remaining;

          if (outfile != NULL)
            {
              if (fwrite(&buffer,sizeof(char),bytes,outfile) != bytes)
                {
                  archive_seterror("Error writing %s",fname_full);
                  fclose(outfile);
                  outfile = NULL;
                  remove(fname_full);
                  return -1;
                }
            }
          remaining -= bytes;
        }

      if (remaining == 0)
        {
          getheader = 1;
          if (outfile != NULL)
            {
              fclose(outfile);
              outfile = NULL;
              if (action != TGZ_INVALID)
                push_attr(&attributes,fname_full,tarmode,tartime);
            }
        }

      /*
       * Abandon if errors are found
       */
      if (action == TGZ_INVALID)
        {
          archive_seterror("broken archive");
          break;
        }
    }

  /*
   * Restore file modes and time stamps
   */
  restore_attr(&attributes);

  return (action == TGZ_INVALID) ? -1 : 0;
}

