/*
 * Added extra newline if ^M occurs
 * Christian Wurll, wurll@ira.uka.de
 * Thu Nov 19 1998 
 * 
 * Added Mac text file translation, i.e. \r to \n conversion
 * Bernd Johannes Wuebben, wuebben@kde.org
 * Wed Feb  4 19:12:58 EST 1998      
 *
 *  Name: dos2unix
 *  Documentation:
 *    Remove cr ('\x0d') characters from a file.
 *  Version: $$Id: dos2unix.c,v 1.1 2002-04-10 13:39:04 jrfonseca Exp $$
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
 *  == 1.2 == 1995.03.16 == Benjamin Lin (blin@socs.uts.edu.au)
 *     Modified to more conform to UNIX style.
 *  == 2.0 == 1995.03.19 == Benjamin Lin (blin@socs.uts.edu.au)
 *     Rewritten from scratch.
 *  == 2.1 == 1995.03.29 == Benjamin Lin (blin@socs.uts.edu.au)
 *     Conversion to SunOS charset implemented.
 *  == 2.2 == 1995.03.30 == Benjamin Lin (blin@socs.uts.edu.au)
 *     Fixed a bug in 2.1 where in new file mode, if outfile already exists
 *     conversion can not be completed properly.
 *
 *  == BUG ==
 *     stdio process under DOS not working
 */


#define RCS_AUTHOR   "$$Author: jrfonseca $$"
#define RCS_DATE     "$$Date: 2002-04-10 13:39:04 $$"
#define RCS_REVISION "$$Revision: 1.1 $$"
#define VER_AUTHOR   "Christian Wurll"
#define VER_DATE     "Thu Nov 19 1998"
#define VER_REVISION "3.1"

#define MACMODE  1
static int macmode = 0;

/* #define DEBUG */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/utime.h>
#include "dos2unix.h"


#define R_CNTRL   "rb"
#define W_CNTRL   "wb"


typedef struct
{
  int NewFile;                          /* is in new file mode? */
  int Quiet;                            /* is in quiet mode? */
  int KeepDate;                         /* should keep date stamp? */
  int ConvMode;                         /* 0 - ASCII, 1 - 7 bit, 2 - ISO, 3- Mac*/  
  int NewLine;                          /* if TRUE, then additional newline */
} CFlag;



void PrintUsage(void)
{
  fprintf(stderr, "dos2unix Copyright (c) 1994-1995 Benjamin Lin\n"\
       	          "         Copyright (c) 1998      Bernd Johannes Wuebben (Version 3.0)\n");
  fprintf(stderr, "         Copyright (c) 1998      Christian Wurll (Version 3.1)\n");
  fprintf(stderr, "Usage: dos2unix [-hkqV] [-c convmode] [-o file ...] [-n infile outfile ...]\n");
  fprintf(stderr, " -h --help        give this help\n");
  fprintf(stderr, " -k --keepdate    keep output file date\n");
  fprintf(stderr, " -q --quiet       quiet mode, suppress all warnings\n");
  fprintf(stderr, "                  always on in stdin->stdout mode\n");
  fprintf(stderr, " -V --version     display version number\n");
  fprintf(stderr, " -c --convmode    conversion mode\n");
  fprintf(stderr, " convmode         ASCII, 7bit, ISO, Mac, default to ASCII\n");
  fprintf(stderr, " -l --newline     add additional newline in all but Mac convmode\n");
  fprintf(stderr, " -o --oldfile     write to old file\n");
  fprintf(stderr, " file ...         files to convert in old file mode\n");
  fprintf(stderr, " -n --newfile     write to new file\n");
  fprintf(stderr, " infile           original file in new file mode\n");
  fprintf(stderr, " outfile          output file in new file mode\n");
}


void PrintVersion(void)
{
  fprintf(stderr, "dos2unix %s (%s)\n", VER_REVISION, VER_DATE);
#ifdef DEBUG
  fprintf(stderr, "RCS_AUTHOR: %s\n", RCS_AUTHOR);
  fprintf(stderr, "RCS_DATE: %s\n", RCS_DATE);
  fprintf(stderr, "RCS_REVISION: %s\n", RCS_REVISION);
  fprintf(stderr, "VER_AUTHOR: %s\n", VER_AUTHOR);
  fprintf(stderr, "VER_DATE: %s\n", VER_DATE);
  fprintf(stderr, "VER_REVISION: %s\n", VER_REVISION);
#endif DEBUG
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


/* converts stream ipInF to UNIX format text and write to stream ipOutF
 * RetVal: 0  if success
 *         -1  otherwise
 */
int ConvertDosToUnix(FILE* ipInF, FILE* ipOutF, CFlag *ipFlag)
{
    int RetVal = 0;
    int TempChar;
    
    if ( macmode )
      ipFlag->ConvMode = 3;

    switch (ipFlag->ConvMode)
    {
        case 0: /* ASCII */
	  while ((TempChar = getc(ipInF)) != EOF) {
	    if (TempChar != '\x0d') {
	      if (putc(D2UAsciiTable[TempChar], ipOutF) == EOF) {
		RetVal = -1;
		if (!ipFlag->Quiet)
		  fprintf(stderr, "dos2unix: can not write to out file\n");
		break;
	      } 
	    } else {
	      if (ipFlag->NewLine) {
		putc('\n', ipOutF);
	      }
	    }
	  }
	  break;
        case 1: /* 7Bit */
	  while ((TempChar = getc(ipInF)) != EOF) {
	    if (TempChar != '\x0d') {
	      if (putc(D2U7BitTable[TempChar], ipOutF) == EOF) {
		RetVal = -1;
		if (!ipFlag->Quiet)
		  fprintf(stderr, "dos2unix: can not write to out file\n");
		break;
	      }
	    } else {
	      if (ipFlag->NewLine) {
		putc('\n', ipOutF);
	      }
	    }
	  }
	  break;
        case 2: /* ISO */
	  while ((TempChar = getc(ipInF)) != EOF) {
	    if (TempChar != '\x0d') {
	      if (putc(D2UIsoTable[TempChar], ipOutF) == EOF) {
		RetVal = -1;
		if (!ipFlag->Quiet)
		  fprintf(stderr, "dos2unix: can not write to out file\n");
		break;
	      }
	    } else {
	      if (ipFlag->NewLine) {
		putc('\n', ipOutF);
	      }
	    }
	  }
	  break;
    case 3: /* Mac */
	  while ((TempChar = getc(ipInF)) != EOF)
	    if ((TempChar != '\x0d'))
	      {
		if(putc(D2UAsciiTable[TempChar], ipOutF) == EOF){
		  RetVal = -1;
		  if (!ipFlag->Quiet)
		    fprintf(stderr, "dos2unix: can not write to out file\n");
		  break;
		}
	      }
	    else{
	      if (putc('\x0a', ipOutF) == EOF)
		{
		  RetVal = -1;
		  if (!ipFlag->Quiet)
		    fprintf(stderr, "dos2unix: can not write to out file\n");
		  break;
		}
	    }
	  break;
    default: /* unknown convmode */
      ;
#ifdef DEBUG
      fprintf(stderr, "dos2unix: program error, invalid conversion mode %d\n",ipFlag->ConvMode);
      exit(1);
#endif DEBUG
    }
    
    return RetVal;
}


/* convert file ipInFN to UNIX format text and write to file ipOutFN
 * RetVal: 0 if success
 *         -1 otherwise
 */
int ConvertDosToUnixNewFile(char *ipInFN, char *ipOutFN, CFlag *ipFlag)
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

  strcpy (TempPath, "./d2utmp");
  strcat (TempPath, "XXXXXX");
  mktemp (TempPath);

#ifdef DEBUG
  fprintf(stderr, "dos2unix: using %s as temp file\n", TempPath);
#endif DEBUG

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
  if ((!RetVal) && (ConvertDosToUnix(InF, TempF, ipFlag)))
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
      fprintf(stderr, "dos2unix: problems renaming '%s' to '%s'\n", TempPath, ipOutFN);
      fprintf(stderr, "          output file remains in '%s'\n", TempPath);
      RetVal = -1;
    }
  }
  return RetVal;
}




/* convert file ipInFN to UNIX format text
 * RetVal: 0 if success
 *         -1 otherwise
 */
int ConvertDosToUnixOldFile(char* ipInFN, CFlag *ipFlag)
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
  fprintf(stderr, "dos2unix: using %s as temp file\n", TempPath);
#endif DEBUG

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
  if ((!RetVal) && (ConvertDosToUnix(InF, TempF, ipFlag)))
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
      fprintf(stderr, "dos2unix: problems renaming '%s' to '%s'\n", TempPath, ipInFN);
      fprintf(stderr, "          output file remains in '%s'\n", TempPath);
    }
    RetVal = -1;
  }
  return RetVal;
}


/* convert stdin to UNIX format text and write to stdout
 * RetVal: 0 if success
 *         -1 otherwise
 */
int ConvertDosToUnixStdio(CFlag *ipFlag)
{
    ipFlag->NewFile = 1;
    ipFlag->Quiet = 1;
    ipFlag->KeepDate = 0;
    return (ConvertDosToUnix(stdin, stdout, ipFlag));
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
  pFlag->NewLine = 0;

  if( strcmp(argv[0],"mac2unix") == 0 )
    macmode = MACMODE;

  /* no option, use stdin and stdout */
  if (argc == 1)
  {
    exit(ConvertDosToUnixStdio(pFlag));
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
      if ((strcmp(argv[ArgIdx],"-l") == 0) || (strcmp(argv[ArgIdx],"--newline") == 0))
        pFlag->NewLine = 1;
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
        else if (strcmpi(argv[ArgIdx], "Mac") == 0)
          pFlag->ConvMode = 3;
        else
        {
          if (!pFlag->Quiet)
            fprintf(stderr, "dos2unix: invalid %s conversion mode specified\n",argv[ArgIdx]);
          ShouldExit = 1;
        }
      }

      if ((strcmp(argv[ArgIdx],"-o") == 0) || (strcmp(argv[ArgIdx],"--oldfile") == 0))
      {
        /* last convert not paired */
        if (!CanSwitchFileMode)
        {
          if (!pFlag->Quiet)
            fprintf(stderr, "dos2unix: target of file %s not specified in new file mode\n", argv[ArgIdx-1]);
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
            fprintf(stderr, "dos2unix: target of file %s not specified in new file mode\n", argv[ArgIdx-1]);
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
            fprintf(stderr, "dos2unix: converting file %s to file %s in UNIX format ...\n", argv[ArgIdx-1], argv[ArgIdx]);
          if (ConvertDosToUnixNewFile(argv[ArgIdx-1], argv[ArgIdx], pFlag))
          {
            if (!pFlag->Quiet)
              fprintf(stderr, "dos2unix: problems converting file %s to file %s\n", argv[ArgIdx-1], argv[ArgIdx]);
            ShouldExit = 1;
          }
          CanSwitchFileMode = 1;
        }
      }
      else
      {
        if (!pFlag->Quiet)
          fprintf(stderr, "dos2unix: converting file %s to UNIX format ...\n", argv[ArgIdx]);
        if (ConvertDosToUnixOldFile(argv[ArgIdx], pFlag))
        {
          if (!pFlag->Quiet)
            fprintf(stderr, "dos2unix: problems converting file %s\n", argv[ArgIdx]);
          ShouldExit = 1;
        }
      }
    }
  }
  
  if ((!pFlag->Quiet) && (!CanSwitchFileMode))
  {
    fprintf(stderr, "dos2unix: target of file %s not specified in new file mode\n", argv[ArgIdx-1]);
    ShouldExit = 1;
  }
  free(pFlag);
  return (ShouldExit);
}

