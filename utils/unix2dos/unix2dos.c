/*
 *  Name: unix2dos
 *  Documentation:
 *    Convert lf ('\x0a') characters in a file to cr lf ('\x0d' '\x0a')
 *    combinations.
 *  Version: $$Id: unix2dos.c,v 1.2 2002-09-29 23:31:47 jrfonseca Exp $$
 *
 *  Copyright (c) 1994, 1995 Benjamin Lin.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice in the documentation and/or other materials provided with
 *     the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY
 *  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 *  OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 *  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 *  OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 *  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  == 1.0 == 1989.10.04 == John Birchfield (jb@koko.csustan.edu)
 *  == 1.1 == 1994.12.20 == Benjamin Lin (blin@socs.uts.edu.au)
 *     Cleaned up for Borland C/C++ 4.02
 *  == 1.2 == 1995.03.09 == Benjamin Lin (blin@socs.uts.edu.au)
 *     Fixed minor typo error
 *  == 1.3 == 1995.03.16 == Benjamin Lin (blin@socs.uts.edu.au)
 *     Modified to more conform to UNIX style.
 *  == 2.0 == 1995.03.19 == Benjamin Lin (blin@socs.uts.edu.au)
 *     Rewritten from scratch
 *  == 2.2 == 1995.03.30 == Benjamin Lin (blin@socs.uts.edu.au)
 *     Conversion from SunOS charset implemented.
 *
 *  == BUG ==
 *     stdio process under DOS not working
 */


#define RCS_AUTHOR   "$$Author: jrfonseca $$"
#define RCS_DATE     "$$Date: 2002-09-29 23:31:47 $$"
#define RCS_REVISION "$$Revision: 1.2 $$"
#define VER_AUTHOR   "Benjamin Lin"
#define VER_DATE     "1995.03.31"
#define VER_REVISION "2.2"

/* #define DEBUG */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/utime.h>
#include "unix2dos.h"


#define R_CNTRL   "rb"
#define W_CNTRL   "wb"


typedef struct
{
  int NewFile;                          /* is in new file mode? */
  int Quiet;                            /* is in quiet mode? */
  int KeepDate;                         /* should keep date stamp? */
  int ConvMode;                         /* 0 - ASCII, 1 - 7 bit, 2 - ISO */  
} CFlag;


void PrintUsage(void)
{
  fprintf(stderr, "unix2dos %s Copyright (c) 1994-1995 %s. (%s)\n", VER_REVISION, VER_AUTHOR, VER_DATE);
  fprintf(stderr, "Usage: unix2dos [-hkqV] [-o file ...] [-c convmode] [-n infile outfile ...]\n");
  fprintf(stderr, " -h --help        give this help\n");
  fprintf(stderr, " -k --keepdate    keep output file date\n");
  fprintf(stderr, " -q --quiet       quiet mode, suppress all warnings\n");
  fprintf(stderr, "                  always on in stdin->stdout mode\n");
  fprintf(stderr, " -V --version     display version number\n");
  fprintf(stderr, " -c --convmode    conversion mode\n");
  fprintf(stderr, " convmode         ASCII, 7bit, ISO, default to ASCII\n");
  fprintf(stderr, " -o --oldfile     write to old file\n");
  fprintf(stderr, " file ...         files to convert in old file mode\n");
  fprintf(stderr, " -n --newfile     write to new file\n");
  fprintf(stderr, " infile           original file in new file mode\n");
  fprintf(stderr, " outfile          output file in new file mode\n");
}


void PrintVersion(void)
{
  fprintf(stderr, "unix2dos %s (%s)\n", VER_REVISION, VER_DATE);
#ifdef DEBUG
  fprintf(stderr, "RCS_AUTHOR: %s\n", RCS_AUTHOR);
  fprintf(stderr, "RCS_DATE: %s\n", RCS_DATE);
  fprintf(stderr, "RCS_REVISION: %s\n", RCS_REVISION);
  fprintf(stderr, "VER_AUTHOR: %s\n", VER_AUTHOR);
  fprintf(stderr, "VER_DATE: %s\n", VER_DATE);
  fprintf(stderr, "VER_REVISION: %s\n", VER_REVISION);
#endif /* DEBUG */
}


/* opens file of name ipFN in read only mode
 * RetVal: NULL if failure
 *         file stream otherwise
 */
FILE* OpenInFile(char *ipFN)
{
  return (fopen(ipFN, R_CNTRL));
}


/* opens file of name ipFN in write only mode
 * RetVal: NULL if failure
 *         file stream otherwise
 */
FILE* OpenOutFile(char *ipFN)
{
  return (fopen(ipFN, W_CNTRL));
}


/* converts stream ipInF to DOS format text and write to stream ipOutF
 * RetVal: 0  if success
 *         -1  otherwise
 */
int ConvertUnixToDos(FILE* ipInF, FILE* ipOutF, CFlag *ipFlag)
{
    int RetVal = 0;
    int TempChar;

    switch (ipFlag->ConvMode)
    {
        case 0: /* ASCII */
            while ((TempChar = getc(ipInF)) != EOF)
                if ((TempChar == '\x0a') && (putc('\x0d', ipOutF) == EOF) ||
                    (TempChar == '\x0d') && (((TempChar = getc(ipInF)) == EOF) || (putc('\x0d', ipOutF) == EOF)) ||
                    (putc(U2DAsciiTable[TempChar], ipOutF) == EOF))
                {
                    RetVal = -1;
                    if (!ipFlag->Quiet)
                        fprintf(stderr, "unix2dos: can not write to output file\n");
                    break;
                }
            break;
      case 1: /* 7Bit */
            while ((TempChar = getc(ipInF)) != EOF)
                if ((TempChar == '\x0a') && (putc('\x0d', ipOutF) == EOF) ||
                    (TempChar == '\x0d') && (((TempChar = getc(ipInF)) == EOF) || (putc('\x0d', ipOutF) == EOF)) ||
                    (putc(U2D7BitTable[TempChar], ipOutF) == EOF))
                {
                    RetVal = -1;
                    if (!ipFlag->Quiet)
                        fprintf(stderr, "unix2dos: can not write to output file\n");
                    break;
                }
            break;
      case 2: /* ISO */
            while ((TempChar = getc(ipInF)) != EOF)
                if ((TempChar == '\x0a') && (putc('\x0d', ipOutF) == EOF) ||
                    (TempChar == '\x0d') && (((TempChar = getc(ipInF)) == EOF) || (putc('\x0d', ipOutF) == EOF)) ||
                    (putc(U2DIsoTable[TempChar], ipOutF) == EOF))
                {
                    RetVal = -1;
                    if (!ipFlag->Quiet)
                        fprintf(stderr, "unix2dos: can not write to output file\n");
                    break;
                }
            break;
      default: /*unknown convmode */
          ;
#ifdef DEBUG
            fprintf(stderr, "unix2dos: program error, invalid conversion mode %d\n",ipFlag->ConvMode);
            exit(1);
#endif /* DEBUG */
  }
  return RetVal;
}


/* convert file ipInFN to DOS format text and write to file ipOutFN
 * RetVal: 0 if success
 *         -1 otherwise
 */
int ConvertUnixToDosNewFile(char *ipInFN, char *ipOutFN, CFlag *ipFlag)
{
  int RetVal = 0;
  FILE *InF = NULL;
  FILE *TempF = NULL;
  char TempPath[16];
  struct stat StatBuf;
  struct utimbuf UTimeBuf;

  /* retrieve ipInFN file date stamp */
  if ((ipFlag->KeepDate) && stat(ipInFN, &StatBuf))
    RetVal = -1;

  strcpy (TempPath, "./u2dtmp");
  strcat (TempPath, "XXXXXX");
  mktemp (TempPath);

#ifdef DEBUG
  fprintf(stderr, "unix2dos: using %s as temp file\n", TempPath);
#endif /* DEBUG */

  /* can open in file? */
  if ((!RetVal) && ((InF=OpenInFile(ipInFN)) == NULL))
    RetVal = -1;

  /* can open out file? */
  if ((!RetVal) && (InF) && ((TempF=OpenOutFile(TempPath)) == NULL))
  {
    fclose (InF);
    RetVal = -1;
  }

  /* conversion sucessful? */
  if ((!RetVal) && (ConvertUnixToDos(InF, TempF, ipFlag)))
    RetVal = -1;

   /* can close in file? */
  if ((InF) && (fclose(InF) == EOF))
    RetVal = -1;

  /* can close out file? */
  if ((TempF) && (fclose(TempF) == EOF))
    RetVal = -1;

  if ((!RetVal) && (ipFlag->KeepDate))
  {
    UTimeBuf.actime = StatBuf.st_atime;
    UTimeBuf.modtime = StatBuf.st_mtime;
    /* can change out file time to in file time? */
    if (utime(TempPath, &UTimeBuf) == -1)
      RetVal = -1;
  }

  /* any error? */
  if ((RetVal) && (unlink(TempPath)))
    RetVal = -1;

  /* can rename temp file to out file? */
  if (!RetVal)
  {
    if (stat(ipOutFN, &StatBuf) == 0)
      unlink(ipOutFN);

    if ((rename(TempPath, ipOutFN) == -1) && (!ipFlag->Quiet))
    {
      fprintf(stderr, "unix2dos: problems renaming '%s' to '%s'\n", TempPath, ipOutFN);
      fprintf(stderr, "          output file remains in '%s'\n", TempPath);
      RetVal = -1;
    }
  }

  return RetVal;
}


/* convert file ipInFN to DOS format text
 * RetVal: 0 if success
 *         -1 otherwise
 */
int ConvertUnixToDosOldFile(char* ipInFN, CFlag *ipFlag)
{
  int RetVal = 0;
  FILE *InF = NULL;
  FILE *TempF = NULL;
  char TempPath[16];
  struct stat StatBuf;
  struct utimbuf UTimeBuf;

  /* retrieve ipInFN file date stamp */
  if ((ipFlag->KeepDate) && stat(ipInFN, &StatBuf))
    RetVal = -1;

  strcpy (TempPath, "./u2dtmp");
  strcat (TempPath, "XXXXXX");
  mktemp (TempPath);

#ifdef DEBUG
  fprintf(stderr, "unix2dos: using %s as temp file\n", TempPath);
#endif /* DEBUG */

  /* can open in file? */
  if ((!RetVal) && ((InF=OpenInFile(ipInFN)) == NULL))
    RetVal = -1;

  /* can open out file? */
  if ((!RetVal) && (InF) && ((TempF=OpenOutFile(TempPath)) == NULL))
  {
    fclose (InF);
    RetVal = -1;
  }

  /* conversion sucessful? */
  if ((!RetVal) && (ConvertUnixToDos(InF, TempF, ipFlag)))
    RetVal = -1;

   /* can close in file? */
  if ((InF) && (fclose(InF) == EOF))
    RetVal = -1;

  /* can close out file? */
  if ((TempF) && (fclose(TempF) == EOF))
    RetVal = -1;

  if ((!RetVal) && (ipFlag->KeepDate))
  {
    UTimeBuf.actime = StatBuf.st_atime;
    UTimeBuf.modtime = StatBuf.st_mtime;
    /* can change out file time to in file time? */
    if (utime(TempPath, &UTimeBuf) == -1)
      RetVal = -1;
  }

  /* can delete in file? */
  if ((!RetVal) && (unlink(ipInFN) == -1))
    RetVal = -1;

  /* any error? */
  if ((RetVal) && (unlink(TempPath)))
    RetVal = -1;

  /* can rename out file to in file? */
  if ((!RetVal) && (rename(TempPath, ipInFN) == -1))
  {
    if (!ipFlag->Quiet)
    {
      fprintf(stderr, "unix2dos: problems renaming '%s' to '%s'\n", TempPath, ipInFN);
      fprintf(stderr, "          output file remains in '%s'\n", TempPath);
    }
    RetVal = -1;
  }
  return RetVal;
}


/* convert stdin to DOS format text and write to stdout
 * RetVal: 0 if success
 *         -1 otherwise
 */
int ConvertUnixToDosStdio(CFlag *ipFlag)
{
    ipFlag->NewFile = 1;
    ipFlag->Quiet = 1;
    ipFlag->KeepDate = 0;
    return (ConvertUnixToDos(stdin, stdout, ipFlag));
}


int main (int argc, char *argv[])
{
  /* variable declarations */
  int ArgIdx;
  int CanSwitchFileMode;
  int ShouldExit;
  CFlag *pFlag;

  /* variable initialisations */
  ArgIdx = 0;
  CanSwitchFileMode = 1;
  ShouldExit = 0;
  pFlag = (CFlag*)malloc(sizeof(CFlag));  
  pFlag->NewFile = 0;
  pFlag->Quiet = 0;
  pFlag->KeepDate = 0;
  pFlag->ConvMode = 0;
  
  /* no option, use stdin and stdout */
  if (argc == 1)
  {
    exit(ConvertUnixToDosStdio(pFlag));
  }

  while ((++ArgIdx < argc) && (!ShouldExit))
  {
    /* is it an option? */
    if (argv[ArgIdx][0] == '-')
    {
      /* an option */
      if ((strcmp(argv[ArgIdx],"-h") == 0) || (strcmp(argv[ArgIdx],"--help") == 0))
        PrintUsage();
      if ((strcmp(argv[ArgIdx],"-k") == 0) || (strcmp(argv[ArgIdx],"--keepdate") == 0))
        pFlag->KeepDate = 1;
      if ((strcmp(argv[ArgIdx],"-q") == 0) || (strcmp(argv[ArgIdx],"--quiet") == 0))
        pFlag->Quiet = 1;
      if ((strcmp(argv[ArgIdx],"-V") == 0) || (strcmp(argv[ArgIdx],"--version") == 0))
        PrintVersion();

      if ((strcmp(argv[ArgIdx],"-c") == 0) || (strcmp(argv[ArgIdx],"--convmode") == 0))
      {
        ArgIdx++;
        if (strcmpi(argv[ArgIdx],"ASCII") == 0)
          pFlag->ConvMode = 0;
        else if (strcmpi(argv[ArgIdx], "7Bit") == 0)
          pFlag->ConvMode = 1;
        else if (strcmpi(argv[ArgIdx], "ISO") == 0)
          pFlag->ConvMode = 2;
        else
        {
          if (!pFlag->Quiet)
            fprintf(stderr, "unix2dos: invalid %s conversion mode specified\n",argv[ArgIdx]);
          ShouldExit = 1;
        }
      }

      if ((strcmp(argv[ArgIdx],"-o") == 0) || (strcmp(argv[ArgIdx],"--oldfile") == 0))
      {
        /* last convert not paired */
        if (!CanSwitchFileMode)
        {
          if (!pFlag->Quiet)
            fprintf(stderr, "unix2dos: target of file %s not specified in new file mode\n", argv[ArgIdx-1]);
          ShouldExit = 1;
        }
        pFlag->NewFile = 0;
      }
      if ((strcmp(argv[ArgIdx],"-n") == 0) || (strcmp(argv[ArgIdx],"--newfile") == 0))
      {
        /* last convert not paired */
        if (!CanSwitchFileMode)
        {
          if (!pFlag->Quiet)
            fprintf(stderr, "unix2dos: target of file %s not specified in new file mode\n", argv[ArgIdx-1]);
          ShouldExit = 1;
        }
        pFlag->NewFile = 1;
      }
    }
    else
    {
      /* not an option */
      if (pFlag->NewFile)
      {
        if (CanSwitchFileMode)
          CanSwitchFileMode = 0;
        else
        {
          if (!pFlag->Quiet)
            fprintf(stderr, "unix2dos: converting file %s to file %s in DOS format ...\n", argv[ArgIdx-1], argv[ArgIdx]);
          if (ConvertUnixToDosNewFile(argv[ArgIdx-1], argv[ArgIdx], pFlag))
          {
            if (!pFlag->Quiet)
              fprintf(stderr, "unix2dos: problems converting file %s to file %s\n", argv[ArgIdx-1], argv[ArgIdx]);
            ShouldExit = 1;
          }
          CanSwitchFileMode = 1;
        }
      }
      else
      {
        if (!pFlag->Quiet)
          fprintf(stderr, "unix2dos: converting file %s to DOS format ...\n", argv[ArgIdx]);
        if (ConvertUnixToDosOldFile(argv[ArgIdx], pFlag))
        {
          if (!pFlag->Quiet)
            fprintf(stderr, "unix2dos: problems converting file %s\n", argv[ArgIdx]);
          ShouldExit = 1;
        }
      }
    }
  }

  if ((!pFlag->Quiet) && (!CanSwitchFileMode))
  {
    fprintf(stderr, "unix2dos: target of file %s not specified in new file mode\n", argv[ArgIdx-1]);
    ShouldExit = 1;
  }
  free(pFlag);
  return (ShouldExit);
}

