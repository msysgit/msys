/** \file archiveread.hh
 *
 * Created: JohnE, 2008-12-28
 */
#ifndef ARCHIVEREAD_HH_INC
#define ARCHIVEREAD_HH_INC


#ifdef __cplusplus
extern "C" {
#endif


typedef void* ArchiveReader;


ArchiveReader* ArchiveRead_OpenFileAuto(char const* file);
void ArchiveRead_Close(ArchiveReader* reader);

int ArchiveRead_IsReadable(ArchiveReader* reader);

int ArchiveRead_NextEntry(ArchiveReader* reader);
char const* ArchiveRead_GetEntryPath(ArchiveReader* reader);
int ArchiveRead_EntryIsDirectory(ArchiveReader* reader);
int ArchiveRead_ExtractEntryToBase
 (ArchiveReader* reader,
  char const* base_path);

char const* ArchiveRead_GetError(ArchiveReader* reader);


#ifdef __cplusplus
} //extern "C"
#endif


#endif // ARCHIVEREAD_HH_INC
