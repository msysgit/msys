/*
 * pkgstrm.cpp
 *
 * $Id: pkgstrm.cpp,v 1.1 2009-11-23 20:44:25 keithmarshall Exp $
 *
 * Written by Keith Marshall <keithmarshall@users.sourceforge.net>
 * Copyright (C) 2009, MinGW Project
 *
 *
 * Implementation of the streaming data filters, which will be used
 * for reading package archives in any supported compression format;
 * currently supported formats are:--
 *
 *   raw    (uncompressed)
 *   gzip   (compressed)
 *   bzip2  (compressed)
 *   lzma   (compressed)
 *   xz     (compressed)
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
#include <unistd.h>
#include <fcntl.h>

#ifndef O_BINARY
/*
 * MS-Windows nuisances...
 * Files are expected to be either explicitly text or binary;
 * (UNIX makes no such specific distinction).  We want to force
 * treatment of all files as binary; define a "no-op" substitute
 * for the appropriate MS-Windows attribute, for when we compile
 * on UNIX, so we may henceforth just use it unconditionally.
 */
# ifdef _O_BINARY
#  define O_BINARY _O_BINARY
# else
#  define O_BINARY  0
# endif
#endif

/* We need to enable PKGSTRM_H_SPECIAL awareness, when we compile this...
 */
#define  PKGSTRM_H_SPECIAL  1
#include "pkgstrm.h"

/*****
 *
 * Class Implementation: pkgArchiveStream
 *
 * This class uses a default constructor and default virtual destructor.
 * We never instantiate objects of this class directly; all derived classes
 * provide their own specialised constructors and destructors, together with
 * a mandatory specialised "Read" method.
 *
 * We do, however, provide one generic "GetRawData" method, which derived
 * classes may adopt, or may override, as necessary...
 *
 */
int pkgArchiveStream::GetRawData( int fd, uint8_t *buf, size_t max )
{
  /* Generic helper function for reading a compressed data stream into
   * its decompressing filter's input buffer.  The default implementation
   * assumes a file stream, and simply invokes a read() request; however,
   * we segregate this function, to facilitate an override to handle
   * other input streaming capabilities.
   */
  return read( fd, buf, max );
}

/*****
 *
 * Class Implementation: pkgRawArchiveStream
 *
 * This is the simplest archive stream class, suitable for archives
 * which have been stored WITHOUT compression...
 *
 */
pkgRawArchiveStream::pkgRawArchiveStream( const char *filename )
{
  /* The constructor has little to to, but to open the archive file
   * and associate a file descriptor with the resultant data stream.
   */
  fd = open( filename, O_RDONLY | O_BINARY );
}

pkgRawArchiveStream::~pkgRawArchiveStream()
{
  /* The destructor needs only to close the data stream.
   */
  close( fd );
}

int pkgRawArchiveStream::Read( char *buf, size_t max )
{
  /* While the stream reader simply transfers the requested number
   * of bytes from the stream, to the caller's buffer.
   */
  return read( fd, buf, max );
}

/*****
 *
 * Class Implementation: pkgGzipArchiveStream
 *
 * This class creates an input streaming interface, suitable for
 * reading archives which have been stored with gzip compression.
 * The implementation is based on the use of libz.a, which allows
 * for a similar implementation to that of pkgRawArchiveStream.
 *
 */
pkgGzipArchiveStream::pkgGzipArchiveStream( const char *filename )
{
  /* Once more, the constructor has little to do but open the stream;
   * in this case, the method is analogous to C's fopen().
   */
  stream = gzopen( filename, "rb" );
}

pkgGzipArchiveStream::~pkgGzipArchiveStream()
{
  /* Another destructor, with little to do but close the stream; the
   * gzclose() call suffices for the purpose.
   */
  gzclose( stream );
}

int pkgGzipArchiveStream::Read( char *buf, size_t max )
{
  /* The reader is again served by a single function call, to transfer
   * the requested volume of decompressed data from the raw input file
   * to the caller's buffer.
   */
  return gzread( stream, buf, max );
}

/*****
 *
 * Class Implementation: pkgBzipArchiveStream
 *
 * This class creates an input streaming interface, suitable for
 * reading archives which have been stored with bzip2 compression.
 * The implementation is based on the use of libbz2.a, which again
 * allows for a fairly simple implementation, which is also quite
 * analogous to that of pkgRawArchiveStream.
 *
 */
pkgBzipArchiveStream::pkgBzipArchiveStream( const char *filename )
{
  /* The constructor carries a marginal additional overhead, in
   * that it must first open a regular file, before associating
   * a bzip2 control structure with it; subsequent stream access
   * is directed exclusively through that control structure.
   */
  FILE *streamfile = fopen( filename, "rb" );
  stream = BZ2_bzReadOpen( &bzerror, streamfile, 0, 0, 0, 0 );
}

pkgBzipArchiveStream::~pkgBzipArchiveStream()
{
  /* For the destructor, it is again just a matter of closing
   * the bzip2 stream; (this also takes care of closing the
   * associated file stream).
   */
  BZ2_bzReadClose( &bzerror, stream );
}

int pkgBzipArchiveStream::Read( char *buf, size_t max )
{
  /* Once again, reading is a simple matter of transferring
   * the requisite number of bytes to the caller's buffer.
   */
  return BZ2_bzRead( &bzerror, stream, buf, max );
}

/*****
 *
 * Class Implementation: pkgLzmaArchiveStream
 *
 * This class creates an input streaming interface, suitable for
 * reading archives which have been stored with lzma compression;
 * based on the use of liblzma.a, this implements an adaptation of
 * Lasse Collin's "xzdec" code, as configured for use as an lzma
 * decompressor.
 *
 */
static inline
uint64_t memlimit()
{
  /* Naively cap the memory available to lzma and xz decoders.
   *
   * FIXME: libarchive appears to use this; however, Lasse Collin
   * provides a more sophisticated method for xz, based on actual
   * physical memory footprint; we should adopt it.
   */
  return 1ULL << 23 + 1ULL << 21;
}

static
void lzma_stream_initialise( lzma_stream *stream )
{
  /* This simple helper provides a static template, which is
   * used to define initial state for lzma and xz decoders.
   */
  static const lzma_stream stream_template = LZMA_STREAM_INIT;
  *stream = stream_template;
  /*
   * ...mark the input buffer as initially empty.
   */
  stream->avail_in = 0;
}

pkgLzmaArchiveStream::pkgLzmaArchiveStream( const char *filename )
{
  /* The constructor must first open a file stream...
   */
  if( (fd = open( filename, O_RDONLY | O_BINARY )) >= 0 )
  {
    /* ...then set up the lzma decoder, in appropriately
     * initialised state...
     */
    lzma_stream_initialise( &stream );
    status = lzma_alone_decoder( &stream, memlimit() );
  }
}

pkgLzmaArchiveStream::pkgLzmaArchiveStream( int fileno ):fd( fileno )
{
  /* ...then set up the lzma decoder, in appropriately
   * initialised state...
   */
  lzma_stream_initialise( &stream );
  status = lzma_alone_decoder( &stream, memlimit() );
}

pkgLzmaArchiveStream::~pkgLzmaArchiveStream()
{
  /* The destructor frees memory resources allocated to the decoder,
   * and closes the input stream file descriptor.
   *
   * FIXME: The lzma_alone_decoder may indicate end-of-stream, before
   * the physical input data stream is exhausted.  For now, we silently
   * ignore any such residual data; (it is likely to be garbage anyway).
   * Should we handle it any more explicitly?
   */
  lzma_end( &stream );
  close( fd );
}

int pkgLzmaArchiveStream::Read( char *buf, size_t max )
{
  /* Read an lzma compressed data stream; store up to "max" bytes of
   * decompressed data into "buf".
   * 
   * Start by directing the decoder to use "buf", initially marking it 
   * as "empty".
   */
  stream.next_out = (uint8_t *)(buf);
  stream.avail_out = max;

  while( (stream.avail_out > 0) && (status == LZMA_OK) )
  {
    /* "buf" hasn't been filled yet, and the decoder continues to say
     * that more data may be available.
     */
    if( stream.avail_in == 0 )
    {
      /* We exhausted the current content of the raw input buffer;
       * top it up again.
       */
      stream.next_in = streambuf;
      if( (stream.avail_in = GetRawData( fd, streambuf, BUFSIZ )) < 0 )
      {
	/* FIXME: an I/O error occurred here: need to handle it!!!
	 */
      }
    }

    /* Run the decoder, to decompress as much as possible of the data
     * currently in the raw input buffer, filling available space in
     * "buf"; go round again, in case we exhausted the raw input data
     * before we ran out of available space in "buf".
     */
    status = lzma_code( &stream, LZMA_RUN );
  }

  /* When we get to here, we either filled "buf" completely, or we
   * completely exhausted the raw input stream; in either case, we
   * return the actual number of bytes stored in "buf", (i.e. its
   * total size, less any residual free space).
   */
  return max - stream.avail_out;
}

/*****
 *
 * Class Implementation: pkgXzArchiveStream
 *
 * This class creates an input streaming interface, suitable for
 * reading archives which have been stored with xz compression;
 * again based on the use of liblzma.a, this implements a further
 * adaptation of Lasse Collin's "xzdec" code, as configured for
 * use as an xz decompressor.
 *
 */
pkgXzArchiveStream::pkgXzArchiveStream( const char *filename )
{
  /* The constructor must first open a file stream...
   */
  if( (fd = open( filename, O_RDONLY | O_BINARY )) >= 0 )
  {
    /* ...then set up the lzma decoder, in appropriately
     * initialised state...
     */
    lzma_stream_initialise( &stream );
    status = lzma_stream_decoder( &stream, memlimit(), LZMA_CONCATENATED );

    /* Finally, recognising that with LZMA_CONCATENATED data,
     * we will eventually need to switch the decoder from its
     * initial LZMA_RUN state to LZMA_FINISH, we must provide
     * a variable to specify the active state, (which we may
     * initialise for the LZMA_RUN state).
     */
    opmode = LZMA_RUN;
  }
}

pkgXzArchiveStream::~pkgXzArchiveStream()
{
  /* This destructor frees memory resources allocated to the decoder,
   * and closes the input stream file descriptor; unlike the preceding
   * case of the lzma_alone_decoder, the lzma_stream_decoder guarantees
   * that there is no trailing garbage remaining from the input stream.
   */
  lzma_end( &stream );
  close( fd );
}

int pkgXzArchiveStream::Read( char *buf, size_t max )
{
  /* Read an xz compressed data stream; store up to "max" bytes of
   * decompressed data into "buf".
   * 
   * Start by directing the decoder to use "buf", initially marking it 
   * as "empty".
   */
  stream.next_out = (uint8_t *)(buf);
  stream.avail_out = max;

  while( (stream.avail_out > 0) && (status == LZMA_OK) )
  {
    /* "buf" hasn't been filled yet, and the decoder continues to say
     * that more data may be available.
     */
    if( stream.avail_in == 0 )
    {
      /* We exhausted the current content of the raw input buffer;
       * top it up again.
       */
      stream.next_in = streambuf;
      if( (stream.avail_in = GetRawData( fd, streambuf, BUFSIZ )) < 0 )
      {
	/* FIXME: an I/O error occurred here: need to handle it!!!
	 */
      }

      else if( stream.avail_in < BUFSIZ )
      {
	/* A short read indicates end-of-input...
	 * Unlike the case of the lzma_alone_decoder, (as used for
	 * decompressing lzma streams), the lzma_stream_decoder, (when
	 * initialised for LZMA_CONCATENATED data, as we use here), may
	 * run lzma_code in either LZMA_RUN or LZMA_FINISH mode; the
	 * normal mode is LZMA_RUN, but we switch to LZMA_FINISH
	 * when we have exhausted the input stream.
	 */
	opmode = LZMA_FINISH;
      }
    }

    /* Run the decoder, to decompress as much as possible of the data
     * currently in the raw input buffer, filling available space in
     * "buf"; as noted above, "opmode" will be LZMA_RUN, until we have
     * exhausted the input stream, when it becomes LZMA_FINISH.
     */
    status = lzma_code( &stream, opmode );

    /* We need to go round again, in case we exhausted the raw input
     * data before we ran out of available space in "buf", except...
     */
    if( (status == LZMA_OK) && (opmode == LZMA_FINISH) )
      /*
       * ...when we've already achieved the LZMA_FINISH state,
       * this becomes unnecessary, so we break the cycle.
       */
      break;
  }

  /* When we get to here, we either filled "buf" completely, or we
   * completely exhausted the raw input stream; in either case, we
   * return the actual number of bytes stored in "buf", (i.e. its
   * total size, less any residual free space).
   */
  return max - stream.avail_out;
}

/*****
 *
 * Auxiliary function: pkgOpenArchiveStream()
 *
 * NOTE: Keep this AFTER the class specialisations, so that their derived
 * class declarations are visible for object instantiation here!
 *
 */
#include <string.h>
#include <strings.h>

extern "C" pkgArchiveStream* pkgOpenArchiveStream( const char* filename )
{
  /* Naive decompression filter selection, based on file name extension.
   *
   * FIXME: adopt more proactive selection method, (similar to that used
   * by libarchive, perhaps), based on magic patterns within the file.
   *
   * NOTE: MS-Windows may use UNICODE file names, but distributed package
   * archives almost certainly do not.  For our purposes, use of the POSIX
   * Portable Character Set should suffice; we offer no concessions for
   * any usage beyond this.
   */
  char *ext = strrchr( filename, '.' );
  if( ext != NULL )
  {
    if( strcasecmp( ext, ".gz" ) == 0 )
      /*
       * We expect this input stream to be "gzip" compressed,
       * so we return the appropriate decompressor.
       */
      return new pkgGzipArchiveStream( filename );

    else if( strcasecmp( ext, ".bz2" ) == 0 )
      /*
       * We expect this input stream to be "bzip2" compressed,
       * so again, we return the appropriate decompressor.
       */
      return new pkgBzipArchiveStream( filename );

    else if( strcasecmp( ext, ".lzma" ) == 0 )
      /*
       * We expect this input stream to be "lzma" compressed,
       * so again, we return the appropriate decompressor.
       */
      return new pkgLzmaArchiveStream( filename );

    else if( strcasecmp( ext, ".xz" ) == 0 )
      /*
       * We expect this input stream to be "xz" compressed,
       * so again, we return the appropriate decompressor.
       */
      return new pkgXzArchiveStream( filename );
  }

  /* If we get to here, then we didn't recognise any of the standard
   * compression indicating file name extensions; fall through, to
   * process the stream as raw (uncompressed) data.
   */
  return new pkgRawArchiveStream( filename );
}

/* $RCSfile: pkgstrm.cpp,v $: end of file */
