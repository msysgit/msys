/*
 * pkginet.cpp
 *
 * $Id: pkginet.cpp,v 1.2 2009-12-16 20:09:00 keithmarshall Exp $
 *
 * Written by Keith Marshall <keithmarshall@users.sourceforge.net>
 * Copyright (C) 2009, MinGW Project
 *
 *
 * Implementation of the package download machinery for mingw-get.
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
#define WIN32_LEAN_AND_MEAN

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <wininet.h>
#include <errno.h>

#include "dmh.h"
#include "mkpath.h"

#include "pkgbase.h"
#include "pkgtask.h"

class pkgInternetAgent
{
  /* A minimal, locally implemented class, instantiated ONCE as a
   * global object, to ensure that wininet's global initialisation is
   * completed at the proper time, without us doing it explicitly.
   */
  private:
    HINTERNET SessionHandle;

  public:
    inline pkgInternetAgent():SessionHandle( NULL )
    {
      /* Constructor...
       */
      if( InternetAttemptConnect( 0 ) == ERROR_SUCCESS )
	SessionHandle = InternetOpen
	  ( "MinGW Installer", INTERNET_OPEN_TYPE_PRECONFIG,
	     NULL, NULL, 0
	  );
    }
    inline ~pkgInternetAgent()
    {
      /* Destructor...
       */
      if( SessionHandle != NULL )
	Close( SessionHandle );
    }

    /* Remaining methods are simple inline wrappers for the
     * wininet functions we plan to use...
     */
    inline HINTERNET OpenURL( const char *URL )
    {
      return InternetOpenUrl( SessionHandle, URL, NULL, 0, 0, 0 );
    }
    inline DWORD QueryStatus( HINTERNET id )
    {
      DWORD ok, idx = 0, len = sizeof( ok );
      if( HttpQueryInfo( id, HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE, &ok, &len, &idx ) )
	return ok;
      return 0;
    }
    inline int Read( HINTERNET dl, char *buf, size_t max, DWORD *count )
    {
      return InternetReadFile( dl, buf, max, count );
    }
    inline int Close( HINTERNET id )
    {
      return InternetCloseHandle( id );
    }
};

/* This is the one and only instantiation of an object of this class.
 */
static pkgInternetAgent pkgDownloadAgent;

const char *pkgActionItem::ArchivePath()
{
  /* Specify where downloaded packages are cached,
   * within the local file system.
   */
  return "%R" "var/cache/mingw-get/packages" "%/M/%F";
}

class pkgInternetStreamingAgent
{
  /* Another locally implemented class; each individual file download
   * gets its own instance of this, either as-is for basic data transfer,
   * or as a specialised derivative of this base class.
   */
  protected:
    const char *filename;
    const char *dest_template;

    char *dest_file;
    HINTERNET dl_host;
    int dl_status;

  private:
    virtual int TransferData( int );

  public:
    pkgInternetStreamingAgent( const char*, const char* );
    virtual ~pkgInternetStreamingAgent();

    virtual int Get( const char* );
    inline const char *DestFile(){ return dest_file; }
};

pkgInternetStreamingAgent::pkgInternetStreamingAgent
( const char *local_name, const char *dest_specification )
{
  /* Constructor for the pkgInternetStreamingAgent class.
   */
  filename = local_name;
  dest_template = dest_specification;
  dest_file = (char *)(malloc( mkpath( NULL, dest_template, filename, NULL ) ));
  if( dest_file != NULL )
    mkpath( dest_file, dest_template, filename, NULL );
}

pkgInternetStreamingAgent::~pkgInternetStreamingAgent()
{
  /* Destructor needs to free the heap memory allocated by the
   * constructor, for storage of "dest_file" name.
   */
  free( (void *)(dest_file) );
}

int pkgInternetStreamingAgent::TransferData( int fd )
{
  /* In the case of this base class implementation,
   * we simply read the file's data from the Internet source,
   * and write a verbatim copy to the destination file.
   */
  char buf[8192]; DWORD count, tally = 0;
  do { dl_status = pkgDownloadAgent.Read( dl_host, buf, sizeof( buf ), &count );
       dmh_printf( "\rdownloading: %s: %I32d b", filename, tally += count );
       write( fd, buf, count );
     } while( dl_status && (count > 0) );
  dmh_printf( "\rdownloading: %s: %I32d b\n", filename, tally );
  return dl_status;
}

static const char *get_host_info
( pkgXmlNode *ref, const char *property, const char *fallback = NULL )
{
  /* Helper function to retrieve host information from the XML catalogue.
   *
   * Call with property = "url", to retrieve the URL template to pass as
   * "fmt" argument to mkpath(), or with property = "mirror", to retrieve
   * the substitution text for the "modifier" argument.
   */
  const char *uri = NULL;
  while( ref != NULL )
  {
    /* Starting from the "ref" package entry in the catalogue...
     */
    pkgXmlNode *host = ref->FindFirstAssociate( "download-host" );
    while( host != NULL )
    {
      /* Examine its associate tags; if we find one of type
       * "download-host", with the requisite property, then we
       * immediately return that property value...
       */
      if( (uri = host->GetPropVal( property, NULL )) != NULL )
	return uri;

      /* Otherwise, we look for any other candidate tags
       * associated with the same catalogue entry...
       */
      host = host->FindNextAssociate( "download-host" );
    }
    /* Failing an immediate match, extend the search to the
     * ancestors of the initial reference entry...
     */
    ref = ref->GetParent();
  }
  /* ...and ultimately, if no match is found, we return the
   * specified "fallback" property value.
   */
  return fallback;
}

static inline
int set_transit_path( const char *path, const char *file, char *buf = NULL )
{
  /* Helper to define the transitional path name for downloaded files,
   * used to save the file data while the download is in progress.
   */
  static const char *transit_dir = "/.in-transit";
  return mkpath( buf, path, file, transit_dir );
}

int pkgInternetStreamingAgent::Get( const char *from_url )
{
  /* Download a file from the specified internet URL.
   *
   * Before download commences, we accept that this may fail...
   */
  dl_status = 0;

  /* Set up a "transit-file" to receive the downloaded content.
   */
  char transit_file[set_transit_path( dest_template, filename )];
  int fd; set_transit_path( dest_template, filename, transit_file );

  if( (fd = set_output_stream( transit_file, 0644 )) >= 0 )
  {
    /* The "transit-file" is ready to receive incoming data...
     * Configure and invoke the download handler to copy the data
     * from the appropriate host URL, to this "transit-file".
     */
    if( (dl_host = pkgDownloadAgent.OpenURL( from_url )) != NULL )
    {
      if( pkgDownloadAgent.QueryStatus( dl_host ) == HTTP_STATUS_OK )
      {
	/* With the download transaction fully specified, we may
	 * request processing of the file transfer...
	 */
	dl_status = TransferData( fd );
      }

      /* We are done with the URL handle; close it.
       */
      pkgDownloadAgent.Close( dl_host );
    }

    /* Always close the "transit-file", whether the download
     * was successful, or not...
     */
    close( fd );
    if( dl_status )
      /*
       * When successful, we move the "transit-file" to its
       * final downloaded location...
       */
      rename( transit_file, dest_file );
    else
      /* ...otherwise, we discard the incomplete "transit-file",
       * leaving the caller to diagnose the failure.
       */
      unlink( transit_file );
  }

  /* Report success or failure to the caller...
   */
  return dl_status;
}

void pkgActionItem::DownloadArchiveFiles( pkgActionItem *current )
{
  /* Update the local package cache, to ensure that all packages needed
   * to complete the current set of scheduled actions are present; if any
   * are missing, invoke an Internet download agent to fetch them.  This
   * requires us to walk the action list...
   */
  while( current != NULL )
  {
    /* ...while we haven't run off the end...
     */
    if( (current->flags & ACTION_INSTALL) == ACTION_INSTALL )
    {
      /* For all packages specified in the current action list,
       * for which an "install" action is scheduled, and for which
       * no associated archive file is present in the local archive
       * cache, place an Internet download agent on standby to fetch
       * the required archive from a suitable internet mirror host.
       */
      const char *package_name = current->selection->ArchiveName();
      pkgInternetStreamingAgent download( package_name, current->ArchivePath() );

      /* Check if the required archive is already available locally...
       */
      if( (access( download.DestFile(), R_OK ) != 0) && (errno == ENOENT) )
      {
	/* ...if not, ask the download agent to fetch it...
	 */
	const char *url_template = get_host_info( current->selection, "uri" );
	if( url_template != NULL )
	{
	  /* ...from the URL constructed from the template specified in
	   * the package repository catalogue (configuration database)...
	   */
	  const char *mirror = get_host_info( current->selection, "mirror" );
	  char package_url[mkpath( NULL, url_template, package_name, mirror )];
	  mkpath( package_url, url_template, package_name, mirror );
	  if( ! (download.Get( package_url ) > 0) )
	    dmh_notify( DMH_ERROR,
		"Get package: %s: download failed\n", package_url
	      );
	}
	else
	  /* Cannot download; the repository catalogue didn't specify a
	   * template, from which to construct a download URL...
	   */
	  dmh_notify( DMH_ERROR,
	      "Get package: %s: no URL specified for download\n", package_name
	    );
      }
    }
    /* Repeat download action, for any additional packages specified
     * in the current "actions" list.
     */
    current = current->next;
  }
}

#define DATA_CACHE_PATH		"%R" "var/cache/mingw-get/data"
#define WORKING_DATA_PATH	"%R" "var/lib/mingw-get/data"

/* Internet servers host package catalogues in lzma compressed format;
 * we will decompress them "on the fly", as we download them.  To achieve
 * this, we will use a variant of the pkgInternetStreamingAgent, using a
 * specialised TransferData method; additionally, this will incorporate
 * a special derivative of a pkgLzmaArchiveStream, with its GetRawData
 * method adapted to stream data from an internet URI, instead of
 * reading from a local file.
 *
 * To derive the pkgInternetLzmaStreamingAgent, we need to include the
 * specialised declarations of a pkgArchiveStream, in order to make the
 * declaration of pkgLzmaArchiveStream available as our base class.
 */
#define  PKGSTRM_H_SPECIAL  1
#include "pkgstrm.h"

class pkgInternetLzmaStreamingAgent :
public pkgInternetStreamingAgent, public pkgLzmaArchiveStream
{
  /* Specialisation of the pkgInternetStreamingAgent base class,
   * providing decompressed copies of LZMA encoded files downloaded
   * from the Internet; (the LZMA decompression capability is derived
   * from the pkgLzmaArchiveStream base class).
   */
  public:
    /* We need a specialised constructor...
     */
    pkgInternetLzmaStreamingAgent( const char*, const char* );

  private:
    /* Specialisation requires overrides for each of this pair of
     * methods, (the first from the pkgLzmaArchiveStream base class;
     * the second from pkgInternetStreamingAgent).
     */
    virtual int GetRawData( int, uint8_t*, size_t );
    virtual int TransferData( int );
};

/* This specialisation of the pkgInternetStreamingAgent class needs its
 * own constructor, simply to invoke the constructors for the base classes,
 * (since neither is instantiated by a default constructor).
 */
pkgInternetLzmaStreamingAgent::pkgInternetLzmaStreamingAgent
( const char *local_name, const char *dest_specification ):
pkgInternetStreamingAgent( local_name, dest_specification ),
pkgLzmaArchiveStream( -1 ){}

int pkgInternetLzmaStreamingAgent::GetRawData( int fd, uint8_t *buf, size_t max )
{
  /* Fetch raw (compressed) data from the Internet host, and load it into
   * the decompression filter's input buffer, whence the TransferData routine
   * may retrieve it, via the filter, as an uncompressed stream.
   */
  DWORD count;
  dl_status = pkgDownloadAgent.Read( dl_host, (char *)(buf), max, &count );
  return (int)(count);
}

int pkgInternetLzmaStreamingAgent::TransferData( int fd )
{
  /* In this case, we read the file's data from the Internet source,
   * stream it through the lzma decompression filter, and write a copy
   * of the resultant decompressed data to the destination file.
   */
  char buf[8192]; DWORD count;
  do { count = pkgLzmaArchiveStream::Read( buf, sizeof( buf ) );
       write( fd, buf, count );
     } while( dl_status && (count > 0) );
  return dl_status;
}

static const char *serial_number( const char *catalogue )
{
  /* Local helper function to retrieve issue numbers from any repository
   * package catalogue; returns the result as a duplicate of the internal
   * string, allocated on the heap (courtesy of the strdup() function).
   */
  const char *issue;
  pkgXmlDocument src( catalogue );

  if(   src.IsOk()
  &&  ((issue = src.GetRoot()->GetPropVal( "issue", NULL )) != NULL)  )
    /*
     * Found an issue number; return a copy...
     */
    return strdup( issue );

  /* If we get to here, we couldn't get a valid issue number;
   * whatever the reason, return NULL to indicate failure.
   */
  return NULL;
}

void pkgXmlDocument::SyncRepository( const char *name, pkgXmlNode *repository )
{
  /* Fetch a named package catalogue from a specified Internet repository.
   *
   * Package catalogues are XML files; the master copy on the Internet host
   * must be stored in lzma compressed format, and named to comply with the
   * convention "%F.xml.lzma", in which "%F" represents the value of the
   * "name" argument passed to this pkgXmlDocument class method.
   */ 
  const char *url_template;
  if( (url_template = repository->GetPropVal( "uri", NULL )) != NULL )
  {
    /* Initialise a streaming agent, to manage the catalogue download;
     * (note that we must include the "%/M" placeholder in the template
     * for the local name, to accommodate the name of the intermediate
     * "in-transit" directory used by the streaming agent).
     */
    pkgInternetLzmaStreamingAgent download( name, DATA_CACHE_PATH "%/M/%F.xml" );
    {
      /* Construct the full URI for the master catalogue, and stream it to
       * a locally cached, decompressed copy of the XML file.
       */
      const char *mirror = repository->GetPropVal( "mirror", NULL );
      char catalogue_url[mkpath( NULL, url_template, name, mirror )];
      mkpath( catalogue_url, url_template, name, mirror );
      if( download.Get( catalogue_url ) <= 0 )
	dmh_notify( DMH_ERROR,
	    "Sync Repository: %s: download failed\n", catalogue_url
	  );
    }

    /* We will only replace our current working copy of this catalogue,
     * (if one already exists), with the copy we just downloaded, if this
     * downloaded copy bears an issue number indicating that it is more
     * recent than the working copy.
     */
    const char *repository_version, *working_version;
    if( (repository_version = serial_number( download.DestFile() )) != NULL )
    {
      /* Identify the location for the working copy, (if it exists).
       */
      const char *working_copy_path_name = WORKING_DATA_PATH "/%F.xml";
      char working_copy[mkpath( NULL, working_copy_path_name, name, NULL )];
      mkpath( working_copy, working_copy_path_name, name, NULL );

      /* Compare issue serial numbers...
       */
      if( ((working_version = serial_number( working_copy )) == NULL)
      ||  ((strcmp( repository_version, working_version )) > 0)        )
      {
	/* In these circumstances, we couldn't identify an issue number
	 * for the working copy of the catalogue; (maybe there is no such
	 * catalogue, or maybe it doesn't specify a valid issue number);
	 * in either case, we promote the downloaded copy in its place.
	 *
	 * FIXME: we assume that the working file and the downloaded copy
	 * are stored on the same physical file system device, so we may
	 * replace the former by simply deleting it, and renaming the
	 * latter with its original path name; we make no provision for
	 * replacing the working version by physical data copying.
	 */
	unlink( working_copy );
	rename( download.DestFile(), working_copy );
      }

      /* The issue numbers, returned by the serial_number() function, were
       * allocated on the heap; free them to avoid leaking memory!
       */
      free( (void *)(repository_version) );
      /*
       * The working copy issue number may be represented by a NULL pointer;
       * while it may be safe to call free on this, it just *seems* wrong, so
       * we check it first, to be certain.
       */
      if( working_version != NULL )
	free( (void *)(working_version) );
    }

    /* If the downloaded copy of the catalogue is still in the download cache,
     * we have chosen to keep a previous working copy, so we have no further
     * use for the downloaded copy; discard it, noting that we don't need to
     * confirm its existence because this will fail silently, if it is no
     * longer present.
     */
    unlink( download.DestFile() );
  }
}

/* $RCSfile: pkginet.cpp,v $: end of file */
