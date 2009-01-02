/** \file archiveread_impl.h
 *
 * Created: JohnE, 2008-12-29
 */
#ifndef ARCHIVEREAD_IMPL_H_INC
#define ARCHIVEREAD_IMPL_H_INC


#include "archiveread.hh"


typedef struct
{
	char* error;
	int is_readable;

	void (*internal_close)(void*);
	int (*internal_next_entry)(void*);
	char const* (*internal_get_entry_path)(void*);
	int (*internal_entry_is_directory)(void*);
	int (*internal_extract_entry_to_base)(void*, char const*);
} ArchiveReaderStruct;


void ArchiveRead_InitBase(ArchiveReaderStruct* base);
void ArchiveRead_SetError
 (ArchiveReaderStruct* reader,
  char const* fmt,
  ...);

int ArchiveRead_EnsureDirectory(char const* path, int base_len);


#endif // ARCHIVEREAD_IMPL_H_INC
