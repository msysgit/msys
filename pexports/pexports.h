/*
 pexports - a program to extract exported symbols from a Portable
 Executable (PE) file.

 Copyright (C) 1998 Anders Norlander

 pexports is distributed under the GNU General Public License and
 has absolutely NO WARRANTY.

 pexports will work only on intel machines.
*/

#ifndef _pexports_h
#define _pexports_h

#include <stdio.h>

#include "str_tree.h"

#define VER_MAJOR 0
#define VER_MINOR 44

/* These are needed */
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned char BYTE;
typedef void* PVOID;
typedef long LONG;
#if defined(__MINGW32__) || defined(_MSC_VER)
typedef unsigned __int64 ULONGLONG;
#else
typedef unsigned long long ULONGLONG;
#endif
typedef int BOOL;
typedef void* HMODULE;
#ifdef _WIN64
typedef __int64 INT_PTR;
#define INT_PTR_FORMAT "I64d"
#else
typedef int INT_PTR;
#define INT_PTR_FORMAT "d"
#endif

#define FALSE 0
#define TRUE 1

/* PE structures */
typedef struct _IMAGE_DATA_DIRECTORY {
  DWORD   VirtualAddress;
  DWORD   Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

typedef struct _IMAGE_FILE_HEADER {
  WORD    Machine;
  WORD    NumberOfSections;
  DWORD   TimeDateStamp;
  DWORD   PointerToSymbolTable;
  DWORD   NumberOfSymbols;
  WORD    SizeOfOptionalHeader;
  WORD    Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

#define IMAGE_FILE_MACHINE_I386  0x014c
#define IMAGE_FILE_MACHINE_IA64  0x0200
#define IMAGE_FILE_MACHINE_AMD64 0x8664

typedef struct _IMAGE_OPTIONAL_HEADER32 {
  WORD    Magic;
  BYTE    MajorLinkerVersion;
  BYTE    MinorLinkerVersion;
  DWORD   SizeOfCode;
  DWORD   SizeOfInitializedData;
  DWORD   SizeOfUninitializedData;
  DWORD   AddressOfEntryPoint;
  DWORD   BaseOfCode;
  DWORD   BaseOfData;
  DWORD   ImageBase;
  DWORD   SectionAlignment;
  DWORD   FileAlignment;
  WORD    MajorOperatingSystemVersion;
  WORD    MinorOperatingSystemVersion;
  WORD    MajorImageVersion;
  WORD    MinorImageVersion;
  WORD    MajorSubsystemVersion;
  WORD    MinorSubsystemVersion;
  DWORD   Reserved1;
  DWORD   SizeOfImage;
  DWORD   SizeOfHeaders;
  DWORD   CheckSum;
  WORD    Subsystem;
  WORD    DllCharacteristics;
  DWORD   SizeOfStackReserve;
  DWORD   SizeOfStackCommit;
  DWORD   SizeOfHeapReserve;
  DWORD   SizeOfHeapCommit;
  DWORD   LoaderFlags;
  DWORD   NumberOfRvaAndSizes;
  IMAGE_DATA_DIRECTORY DataDirectory[16];
} IMAGE_OPTIONAL_HEADER32, *PIMAGE_OPTIONAL_HEADER32;

typedef struct _IMAGE_OPTIONAL_HEADER64 {
  WORD    Magic;
  BYTE    MajorLinkerVersion;
  BYTE    MinorLinkerVersion;
  DWORD   SizeOfCode;
  DWORD   SizeOfInitializedData;
  DWORD   SizeOfUninitializedData;
  DWORD   AddressOfEntryPoint;
  DWORD   BaseOfCode;
  ULONGLONG ImageBase;
  DWORD   SectionAlignment;
  DWORD   FileAlignment;
  WORD    MajorOperatingSystemVersion;
  WORD    MinorOperatingSystemVersion;
  WORD    MajorImageVersion;
  WORD    MinorImageVersion;
  WORD    MajorSubsystemVersion;
  WORD    MinorSubsystemVersion;
  DWORD   Win32VersionValue;
  DWORD   SizeOfImage;
  DWORD   SizeOfHeaders;
  DWORD   CheckSum;
  WORD    Subsystem;
  WORD    DllCharacteristics;
  ULONGLONG SizeOfStackReserve;
  ULONGLONG SizeOfStackCommit;
  ULONGLONG SizeOfHeapReserve;
  ULONGLONG SizeOfHeapCommit;
  DWORD   LoaderFlags;
  DWORD   NumberOfRvaAndSizes;
  IMAGE_DATA_DIRECTORY DataDirectory[16];
} IMAGE_OPTIONAL_HEADER64, *PIMAGE_OPTIONAL_HEADER64;

typedef struct IMAGE_NT_HEADERS32 {
  char Signature[4];
  IMAGE_FILE_HEADER FileHeader;
  IMAGE_OPTIONAL_HEADER32 OptionalHeader;
} IMAGE_NT_HEADERS32, *PIMAGE_NT_HEADERS32;

typedef struct _IMAGE_NT_HEADERS64 {
  char Signature[4];
  IMAGE_FILE_HEADER FileHeader;
  IMAGE_OPTIONAL_HEADER64 OptionalHeader;
} IMAGE_NT_HEADERS64, *PIMAGE_NT_HEADERS64;

typedef struct _IMAGE_SECTION_HEADER {
  BYTE    Name[8];
  union {
    DWORD   PhysicalAddress;
    DWORD   VirtualSize;
  } Misc;
  DWORD   VirtualAddress;
  DWORD   SizeOfRawData;
  DWORD   PointerToRawData;
  DWORD   PointerToRelocations;
  DWORD   PointerToLinenumbers;
  WORD    NumberOfRelocations;
  WORD    NumberOfLinenumbers;
  DWORD   Characteristics;
} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

#define IMAGE_SCN_CNT_CODE 0x00000020

typedef struct _IMAGE_EXPORT_DIRECTORY {
  DWORD   Characteristics;
  DWORD   TimeDateStamp;
  WORD    MajorVersion;
  WORD    MinorVersion;
  DWORD   Name;
  DWORD   Base;
  DWORD   NumberOfFunctions;
  DWORD   NumberOfNames;
  DWORD   AddressOfFunctions;
  DWORD   AddressOfNames;
  DWORD   AddressOfNameOrdinals;
} IMAGE_EXPORT_DIRECTORY, *PIMAGE_EXPORT_DIRECTORY;

typedef struct _IMAGE_DOS_HEADER {
  WORD   e_magic;
  WORD   e_cblp;
  WORD   e_cp;   
  WORD   e_crlc; 
  WORD   e_cparhdr;
  WORD   e_minalloc;
  WORD   e_maxalloc;
  WORD   e_ss;      
  WORD   e_sp;      
  WORD   e_csum;
  WORD   e_ip;      
  WORD   e_cs;      
  WORD   e_lfarlc;  
  WORD   e_ovno;    
  WORD   e_res[4];  
  WORD   e_oemid;   
  WORD   e_oeminfo; 
  WORD   e_res2[10];
  LONG   e_lfanew;  
} IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;

PIMAGE_SECTION_HEADER
find_section(DWORD rva);

PIMAGE_DOS_HEADER
load_pe_image(const char *filename);

void *
rva_to_ptr(DWORD rva);

void
dump_exports(DWORD exports_rva);

#define ADD_FUNCTION(nm,n) str_tree_add(&symbols, nm, (void*)(INT_PTR)n)
extern str_tree *symbols;

#endif /* _pexports_h */
