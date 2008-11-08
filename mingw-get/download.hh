/** \file download.hh
 *
 * Created: JohnE, 2008-11-08
 */
#ifndef DOWNLOAD_HH_INC
#define DOWNLOAD_HH_INC


#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>


int DownloadFile
 (const char* url,
  const char* local,
  int (*prog_callback)(size_t, size_t));


#ifdef __cplusplus
} // extern "C"
#endif


#endif // DOWNLOAD_HH_INC
