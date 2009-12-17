#ifndef PKGBASE_H
/*
 * pkgbase.h
 *
 * $Id: pkgbase.h,v 1.2 2009-12-17 17:35:12 keithmarshall Exp $
 *
 * Written by Keith Marshall <keithmarshall@users.sourceforge.net>
 * Copyright (C) 2009, MinGW Project
 *
 *
 * Public interface for the package directory management routines;
 * declares the XML data structures, and their associated class APIs,
 * which are used to describe packages and their interdependencies.
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
#define PKGBASE_H  1

#include <tinyxml.h>
#include <tinystr.h>

#ifndef EXTERN_C
# ifdef __cplusplus
#  define EXTERN_C extern "C"
# else
#  define EXTERN_C
# endif
#endif

class pkgXmlNode : public TiXmlElement
{
  /* A minimal emulation of the wxXmlNode class, founded on
   * the tinyxml implementation of the TiXmlElement class, and
   * subsequently extended by application specific features.
   */
  public:
    /* Constructors...
     */
    inline pkgXmlNode( const char* name ):TiXmlElement( name ){}
    inline pkgXmlNode( const pkgXmlNode& src ):TiXmlElement( src ){}

    /* Accessors...
     */
    inline const char* GetName()
    {
      /* Retrieve the identifying name of the XML tag;
       * tinyxml calls this the element "value"...
       */
      return Value();
    }
    inline pkgXmlNode* GetParent()
    {
      /* wxXmlNode provides this equivalant of tinyxml's
       * Parent() method.
       */
      return (pkgXmlNode*)(Parent());
    }
    inline pkgXmlNode* GetChildren()
    {
      /* wxXmlNode provides only this one method to access
       * the children of an element; it is equivalent to the
       * FirstChild() method in tinyxml's arsenal.
       */
      return (pkgXmlNode*)(FirstChild());
    }
    inline pkgXmlNode* GetNext()
    {
      /* This is wxXmlNode's method for visiting other children
       * of an element, after the first found by GetChildren();
       * it is equivalent to tinyxml's NextSibling().
       */
      return (pkgXmlNode*)(NextSibling());
    }
    inline const char* GetPropVal( const char* name, const char* subst )
    {
      /* tinyxml has no direct equivalent for this wxXmlNode method,
       * (which substitutes default "subst" text for an omitted property),
       * but it may be trivially emulated, using the Attribute() method.
       */
      const char* retval = Attribute( name );
      return retval ? retval : subst;
    }

    /* Additional methods specific to the application.
     */
    inline bool IsElementOfType( const char* tagname )
    {
      /* Confirm if the owner XML node represents a data element
       * with the specified "tagname".
       */
      return strcmp( GetName(), tagname ) == 0;
    }

    /* The following pair of methods provide an iterator
     * for enumerating the contained nodes, within the owner,
     * which themselves exhibit a specified tagname.
     */
    pkgXmlNode* FindFirstAssociate( const char* tagname );
    pkgXmlNode* FindNextAssociate( const char* tagname );

    /* Specific to XML node elements of type "release",
     * the following pair of methods retrieve the actual name of
     * the release tarball, and its associated source code tarball,
     * as they are named on the project download servers.
     */
    const char* ArchiveName();
    const char* SourceArchiveName();
};

class pkgActionItem
{
  /* A class implementing a bi-directionally linked list of
   * "action" descriptors, which is to be associated with the
   * pkgXmlDocument class, specifying actions to be performed
   * on the managed software installation.
   */
  private:
    /* Pointers to predecessor and successor in the linked list
     * comprising the schedule of action items.
     */
    pkgActionItem* prev;
    pkgActionItem* next;

    /* Flags define the specific action associated with this item.
     */
    unsigned long flags;

    /* Criteria for selection of package versions associated with
     * this action item.
     */
    const char* min_wanted;
    const char* max_wanted;

    /* Pointer to the XML database entry for the package selected
     * for processing by this action.
     */
    pkgXmlNode* selection;

    /* Method for retrieving packages from a distribution server.
     */
    void DownloadArchiveFiles( pkgActionItem* );

  public:
    /* Constructor...
     */
    pkgActionItem( pkgActionItem* = NULL, pkgActionItem* = NULL );

    /* Methods for assembling action items into a linked list.
     */
    pkgActionItem* Append( pkgActionItem* = NULL );
    pkgActionItem* Insert( pkgActionItem* = NULL );

    /* Methods for compiling the schedule of actions.
     */
    pkgActionItem* GetReference( pkgActionItem& );
    pkgActionItem* Schedule( unsigned long, pkgActionItem& );

    /* Methods for defining the selection criteria for
     * packages to be processed.
     */
    const char* SetRequirements( pkgXmlNode* );
    pkgXmlNode* SelectIfMostRecentFit( pkgXmlNode* );

    /* Method specifying where downloaded packages are stored.
     */
    const char* ArchivePath();

    /* Method for processing all scheduled actions.
     */
    void Execute();
};

class pkgXmlDocument : public TiXmlDocument
{
  /* Minimal emulation of the wxXmlDocument class, founded on
   * the tinyxml implementation of the TiXmlDocument class.
   */
  public:
    /* Constructors...
     */
    inline pkgXmlDocument( const char* name )
    {
      /* tinyxml has a similar constructor, but unlike wxXmlDocument,
       * it DOES NOT automatically load the document; force it.
       */
      LoadFile( name );

      /* Always begin with an empty actions list.
       */
      actions = NULL;
    }

    /* Accessors...
     */
    inline bool IsOk()
    {
      /* tinyxml doesn't have this, but instead provides a complementary
       * `Error()' indicator, so to simulate `IsOk()'...
       */
      return ! Error();
    }
    inline pkgXmlNode* GetRoot()
    {
      /* This is wxXmlDocument's method for locating the document root;
       * it is equivalent to tinyxml's RootElement() method.
       */
      return (pkgXmlNode *)(RootElement());
    }

  private:
    /* Properties specifying the schedule of actions.
     */
    unsigned long request;
    pkgActionItem* actions;

    /* Method to synchronise the state of the local package manifest
     * with the master copy held on the distribution server.
     */
    void SyncRepository( const char*, pkgXmlNode* );

  public:
    /* Method to merge content from repository-specific package lists
     * into the central XML package database.
     */
    pkgXmlNode* BindRepositories( bool );

    /* Method to locate the XML database entry for a named package.
     */
    pkgXmlNode* FindPackageByName( const char*, const char* = NULL );

    /* Method to resolve the dependencies of a specified package,
     * by walking the chain of references specified by "requires"
     * elements in the respective package database entries.
     */
    void ResolveDependencies( pkgXmlNode*, pkgActionItem* = NULL );

    /* Methods for compiling a schedule of actions.
     */
    void Schedule( unsigned long, const char* );
    pkgActionItem* Schedule( unsigned long, pkgActionItem&, pkgActionItem* = NULL );

    /* Method to execute a sequence of scheduled actions.
     */
    inline void ExecuteActions(){ actions->Execute(); }
};

EXTERN_C const char *xmlfile( const char*, const char* = NULL );
EXTERN_C int has_keyword( const char*, const char* );

static inline
bool match_if_explicit( const char *value, const char *proto )
{
  /* Helper to compare a pair of "C" strings for equality,
   * accepting NULL as a match for anything.
   */
  return (value == NULL) || (proto == NULL) || (strcmp( value, proto ) == 0);
}

#endif /* PKGBASE_H: $RCSfile: pkgbase.h,v $: end of file */
