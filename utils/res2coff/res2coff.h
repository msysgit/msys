//  RES2COFF.h

#ifndef __RES2COFF_H__
# define __RES2COFF_H__

# define WIN32

# include <windows.h>
# include <stdio.h>
# include <stdarg.h>
# include <stdlib.h>
# include <time.h>

// This is an easy and clean way to define variables
// accessible from all source files. One of them
// defines __main__ and there's where they are
// placed. The rest of the source files see them
// as extern variables

# ifdef __main__
#  define PUBVAR
# else
#  define PUBVAR extern
# endif

/* *****************************************************************
 *
 * All structure definitions from this point on (paag)
 *
 * A 32 bit resource file header structure
 *
 */
 
typedef struct {
  DWORD DataSize          ;
  DWORD HeaderSize        ;
  DWORD Type              ;
  DWORD Name              ;
  DWORD DataVersion       ;
  WORD  MemoryFlag        ;
  WORD  LanguageID        ;
  WORD  Version           ;
  DWORD Characteristics   ;
}  RESOURCEHEAD,*PRESOURCEHEAD;


/*
 * This code added by Colin Peters <colin@bird.fu.is.saga-u.ac.jp>.
 * November 15, 1996. You should ifdef this out if you have complete
 * Windows 32 API files which include these structure definitions.
 */
struct _IMAGE_RESOURCE_DIRECTORY
{
  DWORD Characteristics;
  DWORD TimeStamp;
  WORD  MajorVersion;
  WORD  MinorVersion;
  WORD  NumberOfNamedEntries;
  WORD  NumberOfIdEntries;
};

typedef struct _IMAGE_RESOURCE_DIRECTORY   IMAGE_RESOURCE_DIRECTORY;
typedef struct _IMAGE_RESOURCE_DIRECTORY* PIMAGE_RESOURCE_DIRECTORY;

struct _IMAGE_RESOURCE_DIRECTORY_ENTRY
{
  DWORD Name;
  DWORD OffsetToData;
};

typedef struct _IMAGE_RESOURCE_DIRECTORY_ENTRY
			 IMAGE_RESOURCE_DIRECTORY_ENTRY;
typedef struct _IMAGE_RESOURCE_DIRECTORY_ENTRY*
			PIMAGE_RESOURCE_DIRECTORY_ENTRY;

struct _IMAGE_RESOURCE_DATA_ENTRY
{
  DWORD OffsetToData;
  DWORD Size;
  DWORD CodePage;
  DWORD Reserved;
};

typedef struct _IMAGE_RESOURCE_DATA_ENTRY   IMAGE_RESOURCE_DATA_ENTRY;
typedef struct _IMAGE_RESOURCE_DATA_ENTRY* PIMAGE_RESOURCE_DATA_ENTRY;

#define IMAGE_RESOURCE_NAME_IS_STRING		0x80000000
#define IMAGE_RESOURCE_DATA_IS_DIRECTORY	0x80000000

struct _IMAGE_RELOCATION
{
  DWORD VirtualAddress;
  DWORD SymbolTableIndex;
  WORD  Type;
};

typedef struct _IMAGE_RELOCATION   IMAGE_RELOCATION;
typedef struct _IMAGE_RELOCATION* PIMAGE_RELOCATION;

/* Image relocation types (i386) */
#define IMAGE_REL_I386_ABSOLUTE		0
#define IMAGE_REL_I386_DIR16		1
#define IMAGE_REL_I386_REL16		2
#define IMAGE_REL_I386_DIR32		6
#define IMAGE_REL_I386_DIR32NB		7
#define IMAGE_REL_I386_SEG12		9
#define IMAGE_REL_I386_SECTION		10
#define IMAGE_REL_I386_SECREL		11
#define IMAGE_REL_I386_REL32		14

#define IMAGE_SIZEOF_RELOCATION 10;

struct _IMAGE_SYMBOL
{
  union
  {
    BYTE	ShortName[8];
    struct
    {
      DWORD	Short;
      DWORD     Long;
    } Name;
    PBYTE	LongName[2];
  } N;
  DWORD	Value;
  SHORT	SectionNumber;
  WORD	Type;
  BYTE	StorageClass;
  BYTE  NumberOfAuxSymbols;
};

typedef struct _IMAGE_SYMBOL   IMAGE_SYMBOL;
typedef struct _IMAGE_SYMBOL* PIMAGE_SYMBOL;

#define IMAGE_SIZEOF_SYMBOL	18


typedef union _IMAGE_AUX_SYMBOL
{
  struct	/* Sym */
  {
    DWORD	TagIndex;

    union	/* Misc */
    {
      struct
      {
        WORD	Linenumber;
	WORD	Size;
      } LnSz;
      DWORD	TotalSize;
    } Misc;

    union	/* FcnAry */
    {
      struct
      {
        DWORD	PointerToLinenumber;
	DWORD	PointerToNextFunction;
      } Function;

      struct
      {
	WORD	Dimension[4];
      } Array;

    } FcnAry;

    WORD	TvIndex;

  } Sym;

  struct	/* File */
  {
    BYTE	Name[IMAGE_SIZEOF_SYMBOL];
  } File;

  struct	/* Section */
  {
    DWORD	Length;
    WORD	NumberOfRelocations;
    WORD	NumberOfLinenumbers;
    DWORD	CheckSum;
    SHORT	Number;
    BYTE	Selection;
  } Section;

} IMAGE_AUX_SYMBOL;


struct _IMAGE_FILE_HEADER
{
  WORD	Machine;
  WORD	NumberOfSections;
  DWORD	TimeDateStamp;
  DWORD PointerToSymbolTable;
  DWORD NumberOfSymbols;
  WORD	SizeOfOptionalHeader;
  WORD	Characteristics;
};

typedef struct _IMAGE_FILE_HEADER   IMAGE_FILE_HEADER;
typedef struct _IMAGE_FILE_HEADER* PIMAGE_FILE_HEADER;

#define IMAGE_SIZEOF_FILE_HEADER 20

#define IMAGE_FILE_LINE_NUMS_STRIPPED	0x0004
#define IMAGE_FILE_32BIT_MACHINE	0x0100

#define	IMAGE_FILE_MACHINE_I386		0x014c


#define IMAGE_SIZEOF_SHORT_NAME	8

struct _IMAGE_SECTION_HEADER
{
  BYTE	Name[IMAGE_SIZEOF_SHORT_NAME];
  union
  {
    DWORD	PhysicalAddress;
    DWORD	VirtualSize;
  } Misc;
  DWORD	VirtualAddress;
  DWORD	SizeOfRawData;
  DWORD	PointerToRawData;
  DWORD	PointerToRelocations;
  DWORD	PointerToLinenumbers;
  WORD	NumberOfRelocations;
  WORD	NumberOfLinenumbers;
  DWORD	Characteristics;
};

typedef struct _IMAGE_SECTION_HEADER   IMAGE_SECTION_HEADER;
typedef struct _IMAGE_SECTION_HEADER* PIMAGE_SECTION_HEADER;

#define IMAGE_SIZEOF_SECTION_HEADER	40

#define IMAGE_SCN_MEM_READ	0x40000000
#define IMAGE_SCN_MEM_WRITE	0x80000000

/*
 * End of modification by Colin Peters.
 */ 


// This is the storage for the COFF image chunks
// before they are transfered to the disk

typedef struct { 
  int     Size   ;
  void    *Image ;
}   MemImage;

// This holds the relocation and symbol tables

typedef struct {
  int 	  nElems;
  LPVOID  pElems;
} Table ;

// Make a similar tree to the original one, but
// taking into account that data are already in 
// memory. 


typedef struct {
  int count,named,unnamed;
  PRESOURCEHEAD *RName,*RID;
} RESOURCEINFO;


// ********************************************************

#define MAX_TYPES   17 
#define CCHMAXPATH  260

// Program version

#define __MAVERSION__ 1
#define __MIVERSION__ 0x20

// Declared version in the resources

#define DECLARED_MAV 4
#define DECLARED_MIV 0


/* **************************************************
 *
 * Function prototypes
 */
 
# include "protos.h"


/* **************************************************
 *
 * Variables 
 */
 
PUBVAR RESOURCEINFO ResInfo[MAX_TYPES];

/* Option flags */

PUBVAR int  verbose;

PUBVAR int  ResourceTime;
PUBVAR int  ResourceSize;
PUBVAR char *ProgName;
//

PUBVAR HANDLE OBJFile;
PUBVAR HANDLE RESfile;
PUBVAR HANDLE RESmapping;
PUBVAR LPVOID RESview;
PUBVAR LPVOID RESviewEnd;

// *******************************************
// Initialised variables
//

PUBVAR MemImage  Directory
#ifdef __main__
   =     { 0,NULL }
#endif
;

PUBVAR MemImage  Strings
#ifdef __main__
= { 0,NULL }
#endif
;

PUBVAR MemImage  ResourceHeads
#ifdef __main__
= { 0,NULL }
#endif
;

PUBVAR Table RelocTable
#ifdef __main__
= { 0,NULL }
#endif
;

// Constants :
// The predefined resource types

PUBVAR char *szResourceTypes[]
# ifdef __main__
 = {
    "???_0",        "CURSOR",       "BITMAP",     "ICON",
    "MENU",         "DIALOG",       "STRING",     "FONTDIR",
    "FONT",         "ACCELERATORS", "RCDATA",     "MESSAGETABLE",
    "GROUP_CURSOR", "???_13",       "GROUP_ICON", "???_15",
    "VERSION"
   }
# endif
;

#endif // defined(__RES2COFF_H__)


