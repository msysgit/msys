#include <stddef.h>
#include <stdio.h>

/* we need integers of specific sizes */
#ifndef uint32
#define uint32 unsigned long
#endif

#ifndef uint16
#define uint16 unsigned short
#endif

/* defines for archive */
#ifndef ARMAG
#define ARMAG "!<arch>\n"
#endif

#ifndef SARMAG
#define SARMAG 8
#endif

#ifndef ARFMAG
#define ARFMAG "`\n"
#endif

/* archive header */
struct ar_hdr {
  char ar_name[16];
  char ar_date[12];
  char ar_uid[6];
  char ar_gid[6];
  char ar_mode[8];
  char ar_size[10];
  char ar_fmag[2];
};

int ar_read_header (struct ar_hdr *hdr, FILE *f);
char *get_ar_name (struct ar_hdr *hdr);

extern char *long_names;

/* import header */
struct imp_hdr {
  uint16 sig1;
  uint16 sig2;
  uint16 version;
  uint32 time;
  uint32 size;
  uint16 ord_or_hint;
  uint16 type;
};

/* sig1 */
#define IMAGE_FILE_MACHINE_UNKNOWN 0
/* sig2 */
#define SIG2	0xffff

/* values used in the type field */
#define IMPORT_CODE	0
#define IMPORT_DATA	1
#define IMPORT_CONST	2
#define TEST_IMPORT(t,v) (((t) & 3) == (v))
#define IMPNT(v)	(v << 2)
#define IMPORT_ORDINAL	IMPNT(0)
#define IMPORT_NAME	IMPNT(1)
#define IMPORT_NAME_NOPREFIX	IMPNT(2)
#define IMPORT_NAME_UNDECORATE	IMPNT(3)
#define TEST_IMPNT(t,v) (((t) & IMPNT(7)) == (v))

uint32 swap_endian (uint32);

#ifdef BIG_ENDIAN
#define btonat(u) u
#define ltonat(u) swap_endian (u)
#else
#define btonat(u) swap_endian (u)
#define ltonat(u) u
#endif /* BIG_ENDIAN */

int begins (char *, char *);

void *xmalloc ();
void warning (int , char *, ...);
void error (int , char *, ...);

extern char *program_name;

#if !defined(_WIN32) && !defined(__MSDOS__)
#define spawnvp do_spawn
int spawnvp (int, char *, char **);
#define P_WAIT 0
#define P_NOWAIT 1
#endif
