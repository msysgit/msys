/* *********************************************
 *
 * res2coff : Functions dealing with the creation
 *            of a .o file image in memory before
 *            writing it out
 *
 */

#include "res2coff.h"
#include <stdlib.h>


/*
 *  return the offset of the string we just added 
 *  into the string table or -1 if error 
 *  strings are DWORD aligned 
 */

int Add2StringTable(wchar_t *NewString)
{
  int result  = Strings.Size;
  int strlen  = lstrlenW(NewString);
  int i,addsize;
  wchar_t *p1,*p2;

  addsize = (strlen+2) * sizeof(wchar_t);
  if (addsize % sizeof(DWORD)) 
    addsize += sizeof(wchar_t);

  Strings.Image = realloc(Strings.Image,result+addsize);
  if (Strings.Image == NULL)
    AppExit(1,"Out of memory for the string table\n",NULL);

  p1 = NewString;
  p2 = (wchar_t *)((BYTE *)Strings.Image+result);
  *p2++ = (wchar_t)strlen;

  for (i=0;i < strlen;i++)
    *p2++ = *p1++;
  *p2 = L'\0';
  Strings.Size +=addsize;
  return result;
}

// Append a MemImage to another MemImage
int AppendMemImage(MemImage *membase,MemImage *memappend)
{
  char *p1,*p2;
  int i;
  
  membase->Image = realloc(membase->Image,membase->Size+memappend->Size);

  if (membase->Image == NULL) 
    AppExit(1,"Error reallocating MemImage!\n",NULL);
 
  p1 = (char *)memappend->Image;
  p2 = (char *)(membase->Image)+membase->Size;

  for(i=0;i<memappend->Size;i++) 
    *p2++ = *p1++;

  membase->Size += memappend->Size;
  return TRUE;
}

// Add an entry to the relocation table
int AddRelocation(int RVA)
{
  int       last,newsize,*pReloc;

  last = RelocTable.nElems++;
  newsize = RelocTable.nElems * sizeof(int);
  pReloc = RelocTable.pElems = realloc(RelocTable.pElems,newsize);
  if (pReloc == NULL)
    AppExit(1,"Out of memory for relocations\n",NULL);
  pReloc[last]  = RVA ;
  return TRUE;
}

// Create the directory tree and other related chunks

int MakeDirectoryTree(void)
{
  PIMAGE_RESOURCE_DIRECTORY       RD;
  PIMAGE_RESOURCE_DIRECTORY_ENTRY RDE;
  PIMAGE_RESOURCE_DATA_ENTRY      RPR;

  int i,j,ThisOffset,ResSize;

  int types        = 0;
  int ResPointOffs = 0;
  int ResourceOffs = 0;
  int PhantomID    = 100;
  
// we start by calculating the amount of memory we are
// going to need in the directory image and the resource pointers

  ResourceHeads.Size = 0;
  Directory.Size = sizeof(IMAGE_RESOURCE_DIRECTORY); // in the root
  for (i=1;i<17;i++)
    if (ResInfo[i].count) {
      types++;
      Directory.Size += sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY); // in the root
// resource types
      Directory.Size += sizeof(IMAGE_RESOURCE_DIRECTORY);      
      Directory.Size += ResInfo[i].count * 
                        sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY);
// individual resources
      Directory.Size += ResInfo[i].count * 
                        sizeof(IMAGE_RESOURCE_DIRECTORY);
      Directory.Size += ResInfo[i].count * 
	                sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY);
// 
// reserve also for the Resource Pointer Records
//
      ResourceHeads.Size += ResInfo[i].count * 
                            sizeof(IMAGE_RESOURCE_DATA_ENTRY);
    }
    
  if (types == 0) 
    AppExit(1,"ERROR: no resources in RES file ?!?!\n",NULL);

  Directory.Image = malloc(Directory.Size);
  if (Directory.Image == NULL)
    AppExit(1,"Out of memory for the directory\n",NULL);

  ResourceHeads.Image = malloc(ResourceHeads.Size);
  if (ResourceHeads.Image == NULL) 
    AppExit(1,"Out of memory for the resource headers\n",NULL);

  RPR = (PIMAGE_RESOURCE_DATA_ENTRY)ResourceHeads.Image;

// Root Directory

  RD = (PIMAGE_RESOURCE_DIRECTORY)Directory.Image;
  RD -> Characteristics      = 0;
  RD -> TimeDateStamp        = ResourceTime;
  RD -> MajorVersion         = DECLARED_MAV;
  RD -> MinorVersion         = DECLARED_MIV;
  RD -> NumberOfNamedEntries = 0;
  RD -> NumberOfIdEntries    = types;

  RD++;
  RDE = (PIMAGE_RESOURCE_DIRECTORY_ENTRY) RD;
  ThisOffset = sizeof(IMAGE_RESOURCE_DIRECTORY) +
               types * sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY);

// Root directory entries are pointers to directories
// of types

  for (i=1;i<17;i++)
    if (ResInfo[i].count) {
      RDE->Name         = i;
      RDE->OffsetToData = ThisOffset | IMAGE_RESOURCE_DATA_IS_DIRECTORY ;
      RDE++;
      ThisOffset += sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY) * ResInfo[i].count +
                    sizeof(IMAGE_RESOURCE_DIRECTORY);
    }

// Now the directories of types
// ThisOffset is pointing at the storage area for the last directory level
// each Directory there will point to one and only one resource

  RD = (PIMAGE_RESOURCE_DIRECTORY)Directory.Image;
  RD++;
  RDE = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)RD;
  for (i=0;i<types;i++) {
    int                              RDNum = RDE->Name & 0x7fffffff;
    int                              RDOfs = RDE->OffsetToData & 0x7fffffff;
    PIMAGE_RESOURCE_DIRECTORY_ENTRY  RDENext;

    RD = (PIMAGE_RESOURCE_DIRECTORY)((BYTE *)Directory.Image + RDOfs);
    
    RD -> Characteristics      = 0;
    RD -> TimeDateStamp        = ResourceTime;
    RD -> MajorVersion         = DECLARED_MAV;
    RD -> MinorVersion         = DECLARED_MIV;
    RD -> NumberOfNamedEntries = ResInfo[RDNum].named;
    RD -> NumberOfIdEntries    = ResInfo[RDNum].unnamed;
    RD++;

    RDENext = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)RD;
    for (j=0;j<ResInfo[RDNum].count;j++) {
      PIMAGE_RESOURCE_DIRECTORY       RDLast;
      PIMAGE_RESOURCE_DIRECTORY_ENTRY RDELast;
      PRESOURCEHEAD                   *pThisResource;

      if (j==0)
	pThisResource = ResInfo[RDNum].RName;
      if (j == ResInfo[RDNum].named) 
	pThisResource = ResInfo[RDNum].RID;

      if (j < ResInfo[RDNum].named) {
	wchar_t *NameInRES;
        int NameOfs;

	NameInRES = ResName(*pThisResource);
	NameOfs   = Add2StringTable(NameInRES);
	NameOfs += Directory.Size+ResourceHeads.Size;
        RDENext->Name = NameOfs | IMAGE_RESOURCE_NAME_IS_STRING;
      } else {
	wchar_t *NameID = ResName(*pThisResource);
	if (*NameID == 0xffff) {
	  PhantomID = (int)*(++NameID);
	  RDENext->Name = PhantomID++;
	} else {
	  AppExit(1,"Unnamed resource has no ID field",NULL);
	}
      }
      
      RDENext->OffsetToData = ThisOffset | IMAGE_RESOURCE_DATA_IS_DIRECTORY;
      
      RDLast      = (PIMAGE_RESOURCE_DIRECTORY)
                    ((BYTE *)Directory.Image+ThisOffset);

      ThisOffset += sizeof(IMAGE_RESOURCE_DIRECTORY);

      RDLast -> Characteristics      = 0;
      RDLast -> TimeDateStamp        = ResourceTime;
      RDLast -> MajorVersion         = DECLARED_MAV;
      RDLast -> MinorVersion         = DECLARED_MIV;
      RDLast -> NumberOfNamedEntries = 0;
      RDLast -> NumberOfIdEntries    = 1;

      RDELast = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)
                ((BYTE *)Directory.Image+ThisOffset);

      ThisOffset              += sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY);
      RDELast->Name            = PhantomID++;
      RDELast->OffsetToData    = Directory.Size+ResPointOffs;
      ResSize                  = (*pThisResource)->DataSize;

      RPR -> OffsetToData = ResourceOffs;
      RPR -> Size         = ResSize;
      RPR -> CodePage     = LanguageID(*pThisResource);
      RPR -> Reserved     = 0;

      RPR++;
      RDENext++;
      ResPointOffs += sizeof(IMAGE_RESOURCE_DATA_ENTRY);
      ResourceOffs += ResSize;
      pThisResource++;
    }
    RDE++;    
  }
  
  ResourceSize = ResourceOffs;
  
  RPR = (PIMAGE_RESOURCE_DATA_ENTRY)ResourceHeads.Image;
  for (i=0;i<17;i++) {
    for( j=0;j<ResInfo[i].count;j++) {
      LPVOID RelocPos;

      /*
       * The data is situated after the directory, the resourceheads
       * and the strings
       */
      RPR->OffsetToData += Directory.Size+ResourceHeads.Size+Strings.Size;
      /*
       * the resource heads, where the relocations have to take place,
       * are just after the directory
       */
      RelocPos = (LPVOID)(&RPR->OffsetToData);
      AddRelocation((int)((BYTE *)Directory.Size - 
			  (BYTE *)ResourceHeads.Image +
			  (BYTE *)RelocPos));

      RPR++;
    }
  }

// Finally we put all 'subsections' or chunks together

  if (AppendMemImage(&Directory,&ResourceHeads) == FALSE)
    return FALSE;

  if (AppendMemImage(&Directory,&Strings) == FALSE)
    return FALSE;
    
  return TRUE;
}

void DumpResource(HANDLE ofile,PRESOURCEHEAD this)
{
  BYTE *buffer = (BYTE *)this;
  
  buffer += this->HeaderSize;
  CheckedWrite(ofile,buffer,this->DataSize,"resources");
}

void DumpResources(HANDLE ofile)
{
  int i,j;

  for (i=1;i<17;i++) {
    for (j=0;j < ResInfo[i].named;j++) 
      DumpResource(ofile,ResInfo[i].RName[j]);
    
    for (j=0;j < ResInfo[i].unnamed;j++)
      DumpResource(ofile,ResInfo[i].RID[j]);    
  }
}

void DumpDirectoryImage(HANDLE ofile)
{
  CheckedWrite(ofile,Directory.Image,Directory.Size,"resource directory");
}

/*  Dump all relocations */

void DumpRelocations(HANDLE ofile)
{

  /* 
   * The relocation type is 32 bits absolute without
   * base  reference. The  smbols  referenced is the
   * section  start,  which is  the first element of 
   * the symbols table.
   */

  IMAGE_RELOCATION RelocBuffer = { 0,0,IMAGE_REL_I386_DIR32NB};
  int              i,*relocs;

  relocs = (int *)RelocTable.pElems;
  for (i=0;i<RelocTable.nElems;i++) {
    RelocBuffer.VirtualAddress  = relocs[i];
    CheckedWrite(ofile,(LPVOID)(&RelocBuffer),10,"relocations");
  }
} 

void DumpSymbols(HANDLE ofile)
{
  IMAGE_SYMBOL     NewSym;
  IMAGE_AUX_SYMBOL SecSym;

  memset(&NewSym,0,IMAGE_SIZEOF_SYMBOL);
  memset(&SecSym,0,IMAGE_SIZEOF_SYMBOL);

  lstrcpyA(NewSym.N.ShortName,".rsrc");
  NewSym.Value              = 0;
  NewSym.SectionNumber      = 1;
  NewSym.Type               = 0;
  NewSym.StorageClass       = 3;
  NewSym.NumberOfAuxSymbols = 1;

  SecSym.Section.Length              = ResourceSize+Directory.Size;
  SecSym.Section.NumberOfRelocations = RelocTable.nElems;

  CheckedWrite(ofile,&NewSym,IMAGE_SIZEOF_SYMBOL,"symbols");
  CheckedWrite(ofile,&SecSym,IMAGE_SIZEOF_SYMBOL,"symbols");
}

void MakeCOFFSections(HANDLE ofile)
{
  IMAGE_FILE_HEADER    fileheader;
  IMAGE_SECTION_HEADER RSRChdr;
  
  lstrcpyA(RSRChdr.Name,".rsrc");
  RSRChdr.Misc.PhysicalAddress   = 0;
  RSRChdr.VirtualAddress         = 0;
  RSRChdr.SizeOfRawData          = ResourceSize+Directory.Size;
  RSRChdr.PointerToRawData       = IMAGE_SIZEOF_FILE_HEADER +
                                   IMAGE_SIZEOF_SECTION_HEADER;
  RSRChdr.PointerToRelocations   = RSRChdr.SizeOfRawData +
                                   RSRChdr.PointerToRawData;
  RSRChdr.PointerToLinenumbers   = 0;
  RSRChdr.NumberOfRelocations    = RelocTable.nElems;
  RSRChdr.NumberOfLinenumbers    = 0;
  RSRChdr.Characteristics        = IMAGE_SCN_MEM_READ
					/* | IMAGE_SCN_MEM_WRITE */;

  fileheader.Machine              = IMAGE_FILE_MACHINE_I386;
  fileheader.NumberOfSections     = 1;
  fileheader.TimeDateStamp        = ResourceTime;
  fileheader.PointerToSymbolTable = RSRChdr.PointerToRelocations+
                                    RelocTable.nElems * 10;
  fileheader.NumberOfSymbols      = 2;
  fileheader.SizeOfOptionalHeader = 0;
  fileheader.Characteristics      = IMAGE_FILE_LINE_NUMS_STRIPPED | 
                                    IMAGE_FILE_32BIT_MACHINE;

  CheckedWrite(ofile,&fileheader,IMAGE_SIZEOF_FILE_HEADER,"COFF file header");
  CheckedWrite(ofile,&RSRChdr,IMAGE_SIZEOF_SECTION_HEADER,".rsrc section header");
}

