#define  __main__
#include "res2coff.h"

/*
  res2coff.c
 
  Convert a 32 bit resource file into a COFF file
  to integrate the resources into an application
  at link time

  v1.00 : First official release, seems to be quite
          debugged

  v1.01 : stdargs for AppExit()
          standalone CleanUp()
	  Compilable with mingw32 and Jacob Navia's WIN32 headers
	  
  Most of the credit for getting me doing this thing is
  for Matt Pietrek's and his Windows95 Programming Secrets.
  I got the book before I even thought of doing this, but
  without it, I wouldn't have even dared to start.

  * Contributors:
  *  Created by Pedro A. Aranda <paag@tid.es>
  *
  *  THIS SOFTWARE IS NOT COPYRIGHTED
  *
  *  This source code is offered for use in the public domain. You may
  *  use, modify or distribute it freely.
  *
  *  This code is distributed in the hope that it will be useful but
  *  WITHOUT ANY WARRANTY. ALL WARRENTIES, EXPRESS OR IMPLIED ARE HEREBY
  *  DISCLAMED. This includes but is not limited to warrenties of
  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
  Madrid, 06-06-96

  */

/*
 * Exit the program in an orderly way
 * Any open handles are closed, 
 * All dynamic memory is freed
 * Error messages are printed out through stderr
 */

void CleanUp(void)
{

  if (Directory.Image)
    free(Directory.Image);

  if (Strings.Image)
    free(Strings.Image);

  if (ResourceHeads.Image)
    free(ResourceHeads.Image);

  if (RelocTable.pElems)
    free(RelocTable.pElems);
  
  FreeResourceInfo();

  if (RESview)
    UnmapViewOfFile(RESview);
  if (RESmapping)
    CloseHandle(RESmapping);
  if ((RESfile) && (RESfile != (HANDLE)INVALID_HANDLE_VALUE))
    CloseHandle(RESfile);
  if ((OBJFile) && (OBJFile != (HANDLE)INVALID_HANDLE_VALUE))
    CloseHandle(OBJFile);
}

void AppExit(int ErrorLevel,char *msg,...)
{
  va_list args;

  va_start(args,msg);
  if ((ErrorLevel) && (msg))
    {
      fprintf(stderr,"%s :",ProgName);
      vfprintf(stderr,msg,args);
    }
  va_end(args);
  
  CleanUp();
  exit(ErrorLevel);
}

void CheckedWrite(HANDLE of,LPVOID buffer,DWORD size,char *sectmsg)
{
  DWORD written;

  WriteFile(of,buffer,size,&written,NULL);
  if (written == size)
    return;
  else
    AppExit(1,"Error writing %s , disk full(?)\n",sectmsg);
}

void usage(char wrong)
{
  printf("\nUsage: %s [-v] -i infile -o outfile\n\n"
	 " -v             verbose mode\n"
	 " -i <file>.res  RES32 file (input)\n"
	 " -o <file>.o    COFF object file (output)\n",ProgName);

  if (wrong)
    printf("\n -%c is not defined as a valid option \n",wrong);

  exit(-1);
}

int main(int argc,char **argv)
{
  int    arg,result;
  char  *src,*dst;
  DWORD  cbFileSize;

  ProgName = argv[0];
  
  if (argc == 1) {
    usage(0);
    exit(-1);
  }
  
  ResourceTime = (unsigned long)time(NULL);  

  verbose = 0;
  src = dst = NULL;

  InitResourceInfo();

  for (arg = 1;arg < argc;arg++) {
    if (argv[arg][0] == '-')
      switch(argv[arg][1]) {
        case 'v':
        case 'V': verbose = 1;
                  break;

        case 'i':
        case 'I': if (argv[arg][2])
                    src = &argv[arg][2];
                  else
                    src = argv[++arg];
                  break;
        case 'o':
        case 'O': if (argv[arg][2])
                    dst = &argv[arg][2];
                  else
                    dst = argv[++arg];
                  break;

        default : usage(argv[arg][1]);
                  break;
      } else
        usage(0);
  }

  if (src == NULL) {
    usage(0);
    printf("No input file\n");
  }
  
  if (dst == NULL) {
    usage(0);
    printf("No output file\n");
  }

  if (verbose)
    printf("%s version %x.%02x\n\n",ProgName,__MAVERSION__,__MIVERSION__);

/*
 * Now we enter a very orderly procedure:
 *  1.- Open the resource file  
 *  2.- Create a maping for it
 *  3.- Map it into virtual memory
 *  4.- Check it is indeed a RES32 file
 *  5.- Sort the unnamed resources (v1.2)
 *  6.- Create the COFF object file
 *
 * Should any of these steps fail, the application is
 * exited in an orderly way
 */

  RESfile = CreateFileA(src, GENERIC_READ,
			FILE_SHARE_READ, NULL, OPEN_EXISTING,
			FILE_FLAG_SEQUENTIAL_SCAN, NULL);

  if ((!RESfile) || (RESfile == (HANDLE)INVALID_HANDLE_VALUE))
    AppExit(1,"Couldn't open '%s'\n",src);

  RESmapping = CreateFileMappingA(RESfile,NULL,PAGE_READONLY,0,0,NULL);

  if (RESmapping == NULL)
    AppExit(1,"Couldn't create file mapping for '%s'\n",src);

  RESview = MapViewOfFile(RESmapping, FILE_MAP_READ, 0, 0, 0);

  if (RESview == NULL)
    AppExit(1,"Couldn't map view of '%s'\n",src);

  cbFileSize = GetFileSize(RESfile, NULL);

  if ((cbFileSize == -1) && (GetLastError() != NO_ERROR)) 
    AppExit(1,"Error getting size of '%s'",src);

  RESviewEnd = (void *)((BYTE *)RESview + cbFileSize);

  if (!Check32bitResourceFile((PRESOURCEHEAD)RESview))
    AppExit(1,"Error in RES32 file format in '%s'\n",src);

  SortUnnamedResources();

  OBJFile = CreateFileA(dst,GENERIC_WRITE,0,NULL,
			CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
  
  if ((!OBJFile) || (OBJFile == (HANDLE)INVALID_HANDLE_VALUE))
    AppExit(1,"Couldn't create '%s'\n",dst);
   
  /*
   * Now we do the real processing. All functions have
   * autodocumenting names, just follow the code.
   * When all is done, we exit in an ordely way, 
   * cleaning up any kind off mess we could have
   * stored in memory and flagging success (error level = 0)
   */
  MakeDirectoryTree();
  MakeCOFFSections(OBJFile);
  DumpDirectoryImage(OBJFile);
  DumpResources(OBJFile);
  DumpRelocations(OBJFile);
  DumpSymbols(OBJFile);
  CleanUp();
  return 0;
}

