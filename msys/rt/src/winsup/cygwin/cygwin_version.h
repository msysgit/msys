#ifndef CYGWIN_VERSION_H
#define CYGWIN_VERSION_H 1
/* cygwin_version.h: shared info for cygwin

   Copyright 2000, 2001 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#include <cygwin/version.h>

#ifdef __cplusplus
extern "C" {
/* This is for programs that want to access the shared data. */
class shared_info *cygwin_getshared (void);
#endif

struct dll_version_info
{
  unsigned short api_major;
  unsigned short api_minor;
  unsigned short dll_major;
  unsigned short dll_minor;
  unsigned short shared_data;
  const char *dll_build_date;
  char shared_id[sizeof (DLL_VERSION_IDENTIFIER) + 64];
};

#ifndef __cplusplus
typedef struct dll_version_info dll_version_info;
#endif

extern dll_version_info cygwin_version;
extern const char *dll_version_strings;
#ifdef __cplusplus
}
#endif

#endif
