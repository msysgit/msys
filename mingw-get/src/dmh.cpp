/*
 * dmh.cpp
 *
 * $Id: dmh.cpp,v 1.1 2009-11-11 21:59:43 keithmarshall Exp $
 *
 * Written by Keith Marshall <keithmarshall@users.sourceforge.net>
 * Copyright (C) 2009, MinGW Project
 *
 *
 * Implementation of the diagnostic message handling subsystem.
 *
 *
 * This is free software.  Permission is granted to copy, modify and
 * redistribute this software, under the provisions of the GNU General
 * Public License, Version 3, (or, at your option, any later version),
 * as published by the Free Software Foundation; see the file COPYING
 * for licensing details.
 *
 * Note, in particular, that this software is provided "as is", in the
 * hope that it may prove useful, but WITHOUT WARRANTY OF ANY KIND; not
 * even an implied WARRANTY OF MERCHANTABILITY, nor of FITNESS FOR ANY
 * PARTICULAR PURPOSE.  Under no circumstances will the author, or the
 * MinGW Project, accept liability for any damages, however caused,
 * arising from the use of this software.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "dmh.h"

class dmhTypeGeneric
{
  /* Abstract base class, from which message handlers are derived.
   */
  public:
    dmhTypeGeneric( const char* );
    virtual uint16_t control( const uint16_t, const uint16_t ) = 0;
    virtual int notify( const dmh_severity, const char*, va_list ) = 0;
    virtual int printf( const char*, va_list ) = 0;

  protected:
    const char *progname;
};

class dmhTypeTTY : public dmhTypeGeneric
{
  /* Diagnostic message handler for use in console applications.
   */
  public:
    dmhTypeTTY( const char *progname );
    virtual uint16_t control( const uint16_t, const uint16_t );
    virtual int notify( const dmh_severity, const char*, va_list );
    virtual int printf( const char*, va_list );
};

class dmhTypeGUI : public dmhTypeGeneric
{
  /* Diagnostic message handler for use in window applications.
   */
  public:
    dmhTypeGUI( const char *progname );
    virtual uint16_t control( const uint16_t, const uint16_t );
    virtual int notify( const dmh_severity, const char*, va_list );
    virtual int printf( const char*, va_list );
};

/* Constructors serve to initialise the message handler,
 * simply creating the class instance, and storing the specified
 * program name within it.
 */
dmhTypeGeneric::dmhTypeGeneric( const char* name ):progname( name ){}
dmhTypeTTY::dmhTypeTTY( const char* name ):dmhTypeGeneric( name ){}
dmhTypeGUI::dmhTypeGUI( const char* name ):dmhTypeGeneric( name ){}

/* This pointer stores the address of the message handler
 * class instance, after initialisation.
 */
static dmhTypeGeneric *dmh = NULL;

EXTERN_C void dmh_init( const dmh_class subsystem, const char *progname )
{
  /* Public entry point for message handler initialisation...
   *
   * We only do it once, silently ignoring any attempt to
   * establish a second handler.
   */
  if( dmh == NULL )
  {
    /* No message handler has yet been initialised;
     * passing the specified program name, select...
     */
    if( subsystem == DMH_SUBSYSTEM_GUI )
      /*
       * ...a GUI class handler on demand...
       */
      dmh = new dmhTypeGUI( progname );

    else
      /* ...otherwise, a console class handler by default.
       */
      dmh = new dmhTypeTTY( progname );
  }
}

static inline
int abort_if_fatal( const dmh_severity code, int status )
{
  /* Helper function to abort an application, on notification
   * of a DMH_FATAL exception.
   */
  if( code == DMH_FATAL )
    exit( EXIT_FAILURE );

  /* If the exception wasn't DMH_FATAL, then fall through to
   * return the specified status code.
   */
  return status;
}

uint16_t dmhTypeGUI::control( const uint16_t request, const uint16_t mask )
{
  /* Select optional features of the GUI class message handler.
   *
   * FIXME: this is a stub; implementation to be provided.
   */
  return 0;
}

int dmhTypeGUI::notify( const dmh_severity code, const char *fmt, va_list argv )
{
  /* Message dispatcher for GUI applications.
   *
   * FIXME: this is a stub; implementation to be provided.
   */
  return
  fprintf( stderr, "%s: *** ERROR *** DMH_SUBSYSTEM_GUI not yet implemented\n", progname );
}

int dmhTypeGUI::printf( const char *fmt, va_list argv )
{
  /* Display arbitrary text messages via the diagnostic message handler.
   *
   * FIXME: this is a stub; implementation to be provided.
   */
  return notify( DMH_ERROR, fmt, argv );
}

uint16_t dmhTypeTTY::control( const uint16_t request, const uint16_t mask )
{
  /* Select optional features of the console class message handler.
   * This message handler provides no optional features; we make this
   * a "no-op".
   */
  return 0;
}

int dmhTypeTTY::notify( const dmh_severity code, const char *fmt, va_list argv )
{
  /* Message dispatcher for console class applications.
   */
  static const char *severity[] =
  {
    /* Labels to identify message severity...
     */
    "INFO",		/* DMH_INFO */
    "WARNING",		/* DMH_WARNING */
    "ERROR",		/* DMH_ERROR */
    "FATAL"		/* DMH_FATAL */
  };

  /* Dispatch the message to standard error, terminate application
   * if DMH_FATAL, else continue, returning the message length.
   */
  int retcode = fprintf( stderr, "%s: *** %s *** ", progname, severity[code] );
  return abort_if_fatal( code, retcode + vfprintf( stderr, fmt, argv ) );
}

int dmhTypeTTY::printf( const char *fmt, va_list argv )
{
  /* Display arbitrary text messages via the diagnostic message handler;
   * for the TTY subsystem, this is equivalent to printf() on stderr.
   */
  return vfprintf( stderr, fmt, argv );
}

EXTERN_C uint16_t dmh_control( const uint16_t request, const uint16_t mask )
{
  return dmh->control( request, mask );
}

EXTERN_C int dmh_notify( const dmh_severity code, const char *fmt, ... )
{
  /* Public entry point for diagnostic message dispatcher.
   */
  if( dmh == NULL )
  {
    /* The message handler has been called before initialising it;
     * this is an internal program error -- treat it as fatal!
     */
    dmh_init( DMH_SUBSYSTEM_TTY, "dmh" );
    dmh_notify( DMH_FATAL, "message handler was not initialised\n" );
  }

  /* Normal operation; pass the message on to the active handler.
   */
  va_list argv;
  va_start( argv, fmt );
  int retcode = dmh->notify( code, fmt, argv );
  va_end( argv );
  return retcode;
}

EXTERN_C int dmh_printf( const char *fmt, ... )
{
  /* Simulate standard printf() function calls, redirecting the display
   * of formatted output through the diagnostic message handler.
   */
  va_list argv;
  va_start( argv, fmt );
  int retcode = dmh->printf( fmt, argv );
  va_end( argv );
  return retcode;
}

/* $RCSfile: dmh.cpp,v $: end of file */
