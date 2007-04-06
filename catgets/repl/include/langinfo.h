#ifndef _LANGINFO_H_
/* 
 * langinfo.h
 *
 * Defines the locale item identifiers and the prototype for a
 * rudimentary implementation of the nl_langinfo function.
 *
 * This file is part of the Mingw32 package.
 *
 * Contributors:
 *  Created by Keith Marshall <keithmarshall@users.sourceforge.net>
 *
 *  THIS SOFTWARE IS NOT COPYRIGHTED
 *
 *  This source code is offered for use in the public domain. You may
 *  use, modify or distribute it freely.
 *
 *  This code is distributed in the hope that it will be useful but
 *  WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
 *  DISCLAIMED. This includes but is not limited to warranties of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $Revision: 1.1 $
 * $Author: keithmarshall $
 * $Date: 2007-04-06 22:34:56 $
 *
 */
#define _LANGINFO_H_

/* All the headers include this file. */
#include <_mingw.h>

#include <nl_types.h>

#define __NL_ITEM( CATEGORY, INDEX )  ((CATEGORY << 16) | INDEX)

#define __NL_ITEM_CATEGORY( ITEM )    (ITEM >> 16)
#define __NL_ITEM_INDEX( ITEM )       (ITEM & 0xffff)

/*
 * Enumerate the item classification indices...
 *
 */
enum {
  /*
   * LC_CTYPE category...
   * Character set classification items.
   *
   */
  _NL_CTYPE_CODESET = __NL_ITEM( LC_CTYPE, 0 ),

  /*
   * Dummy entry, to terminate the list.
   *
   */
  _NL_ITEM_CLASSIFICATION_END
};

/*
 * Define the public aliases for the enumerated classification indices...
 *
 */
#define CODESET       _NL_CTYPE_CODESET

/*
 * Prototype the lookup function...
 *
 */
extern char __cdecl *nl_langinfo( nl_item );

#endif /* _LANGINFO_H_: $RCSfile: langinfo.h,v $Revision: 1.1 $: end of file */
