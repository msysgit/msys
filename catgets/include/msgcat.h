#ifndef _MSGCAT_H_
/*
 * msgcat.h
 *
 */
#define _MSGCAT_H_

/* Message cataloge signature (magic number)...
 */
#define MINGW32_CATGETS_MAGIC_NUMBER   "MinGW-MC"

/* Package version number...
 */
#define MINGW32_CATGETS_VERSION_MAJOR  CATGETS_VERSION_MAJOR
#define MINGW32_CATGETS_VERSION_MINOR  CATGETS_VERSION_MINOR

/* Macros to encode a message catalogue signature for validation...
 */
#define mk_msgcat_ver( TAG ) TAG##_VERSION_MINOR, TAG##_VERSION_MAJOR
#define mk_msgcat_tag( TAG ) TAG##_MAGIC_NUMBER, { mk_msgcat_ver( TAG ) }

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#pragma pack(1)

struct ident
/*
 * Specifies the layout of the first part of the catalogue header,
 * which identifies a valid MinGW message catalogue, and the minimum
 * catgets version required to process it.
 */
{ unsigned char       magic_number[ sizeof( unsigned long long ) ];
  struct version
  { unsigned short    minor;    /* little endian layout needs minor ...       */
    unsigned short    major;    /* before major `catgets' version number      */
  } version;
};

struct key
/*
 * Defines the common key structure, for lookup of set and message
 * data offsets within the message catalogue.
 */
{ union
  { unsigned short    keyval;   /* generic reference for ...                  */
    unsigned short    setnum;   /* a message set number, within the set index */
    unsigned short    msgnum;   /* or a message number, in the message index  */
  };
  unsigned long       offset;   /* indexed entity location in catalogue file  */
};

struct index
/*
 * Remap the message catalogue header, extending to include the
 * set and message index arrays, and the message data space.
 */
{ unsigned long long  magic_number;     /* message catalogue signature        */
  unsigned long       version;          /* oldest compatible version number   */
  unsigned long       extent;           /* reserved: minimum memory required  */ 
  struct key          index[];          /* first message set index entry      */
};

typedef
union msgcat
/*
 * Merge the ident and index mappings, to create the type
 * definition for the message catalogue.
 */
{ struct ident id;
  struct index mc;
} MSGCAT;

#define MINGW32_CATGETS_ST_SIZE_MIN  sizeof(MSGCAT) + (sizeof(struct key) << 1)

extern int mc_validate( __const char * );

#endif /* _MSGCAT_H_: $RCSfile: msgcat.h,v $Revision: 1.1 $: end of file */
