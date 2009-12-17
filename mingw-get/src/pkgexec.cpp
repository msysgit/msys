/*
 * pkgexec.cpp
 *
 * $Id: pkgexec.cpp,v 1.2 2009-12-17 17:35:12 keithmarshall Exp $
 *
 * Written by Keith Marshall <keithmarshall@users.sourceforge.net>
 * Copyright (C) 2009, MinGW Project
 *
 *
 * Implementation of package management task scheduler and executive.
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
#include "dmh.h"
#include "mkpath.h"

#include "pkgbase.h"
#include "pkginfo.h"
#include "pkgtask.h"

EXTERN_C const char *action_name( unsigned long index )
{
  /* Define the keywords used on the mingw-get command line,
   * to specify the package management actions to be performed,
   * mapping each to a unique action code index.
   */
  static const char* action_id[] =
  {
    "no change",	/* unused; zero cannot test true in a bitwise test  */
    "remove",		/* remove a previously installed package	    */
    "install",		/* install a new package			    */
    "upgrade",		/* upgrade previously installed packages	    */

    "update"		/* update local copy of repository catalogues	    */
  };

  /* For specified "index", return a pointer to the associated keyword,
   * or NULL, if "index" is outside the defined action code range.
   */
  return ((index >= 0) && (index < end_of_actions))
    ? action_id[ index ]
    : NULL;
}

EXTERN_C int action_code( const char* request )
{
  /* Match an action keyword specified on the command line
   * to an entry from the above list...
   */
  int lencode = strlen( request );

  int index;
  for( index = 0; index < end_of_actions; index++ )
  {
    /* Try all defined keywords in turn, until we find a match
     * or we run out of definitions...
     */
    if( strncmp( request, action_name( index ), lencode ) == 0 )
      /*
       * for a successful match...
       * immediately return the associated action code index.
       */
      return index;
  }

  /* If we get to here, the specified keyword was not matched;
   * signal this, by returning -1.
   */
  return -1;
}

pkgActionItem::pkgActionItem( pkgActionItem *after, pkgActionItem *before )
{
  /* Construct an appropriately initialised non-specific pkgActionItem...
   */
  flags = 0;		/* no specific action yet assigned */

  min_wanted = NULL;	/* no minimum package version constraint... */
  max_wanted = NULL;	/* nor any maximum version */

  selection = NULL;	/* no package selection yet, for this item */

  /* Insert this item at a specified location in the actions list.
   */
  prev = after;
  next = before;
}

pkgActionItem*
pkgActionItem::Append( pkgActionItem *item )
{
  /* Add an "item" to an ActionItems list, attaching it immediately
   * after the item referenced by the "this" pointer; nominally "this"
   * refers to the last entry in the list, resulting in a new item
   * being appended to the list, but the implementation preserves
   * integrity of any following list items, thus also fulfilling
   * an "insert after this" function.
   */
  if( this == NULL )
    /*
     * No list exists yet;
     * return "item" as first and only entry in new list.
     */
    return item;

  /* Ensure "item" physically exists, or if not, create a generic
   * placeholder in which to construct it...
   */
  if( (item == NULL) && ((item = new pkgActionItem()) == NULL) )
    /*
     * ...bailing out if no such placeholder can be created.
     */
    return NULL;

  /* Maintain list integrity...
   */
  if( (item->next = next) != NULL )
    /*
     * ...moving any existing items which already follow the insertion
     * point in the list structure, to follow the newly added "item".
     */
    next->prev = item;

  /* Set the new item's own reference pointer, to establish its list
   * attachment point...
   */
  item->prev = this;

  /* ...and attach it immediately after that point.
   */
  return next = item;
}

pkgActionItem*
pkgActionItem::Insert( pkgActionItem *item )
{
  /* Add an "item" to an ActionItems list, inserting it immediately
   * before the item referenced by the "this" pointer.
   */
  if( this == NULL )
    /*
     * No list exists yet;
     * return "item" as first and only entry in new list.
     */
    return item;

  /* Ensure "item" physically exists, or if not, create a generic
   * placeholder in which to construct it...
   */
  if( (item == NULL) && ((item = new pkgActionItem()) == NULL) )
    /*
     * ...bailing out if no such placeholder can be created.
     */
    return NULL;

  /* Maintain list integrity...
   */
  if( (item->prev = prev) != NULL )
    /*
     * ...moving any existing items which already precede the insertion
     * point in the list structure, to precede the newly added "item".
     */
    prev->next = item;

  /* Set the new item's own reference pointer, to establish the item
   * currently at the attachment point, as its immediate successor...
   */
  item->next = this;

  /* ...and attach it, immediately preceding that point.
   */
  return prev = item;
}

pkgActionItem*
pkgActionItem::Schedule( unsigned long action, pkgActionItem& item )
{
  /* Make a copy of an action item template (which may exist in
   * a volatile scope) on the heap, assign the requested action,
   * and return it for inclusion in the task schedule.
   */
  pkgActionItem *rtn = new pkgActionItem(); *rtn = item;
  rtn->flags = action | (rtn->flags & ~ACTION_MASK);
  return rtn;
}

pkgActionItem*
pkgActionItem::GetReference( pkgActionItem& item )
{
  /* Check for a prior reference, within the task schedule,
   * for the package specified for processing by "item".
   */
  pkgXmlNode* pkg;
  if( (pkg = item.selection->GetParent()) != NULL )
  {
    /* We have a pointer to the XML database entry which identifies
     * the package containing the release specified as the selection
     * associated with "item"; walk the chain of prior entries in
     * the schedule...
     */
    for( pkgActionItem* item = this; item != NULL; item = item->prev )
    {
      /* ...and if we find another item holding an identical pointer,
       * (i.e. to the same package), we return it...
       */
      if( item->selection->GetParent() == pkg )
	return item;
    }
  }

  /* If we get to here, there is no prior action scheduled for the
   * specified package, so we return a NULL pointer...
   */
  return NULL;
}

const char * pkgActionItem::SetRequirements( pkgXmlNode *req )
{
  /* Establish the selection criteria, for association of any
   * particular package release with an action item.
   */
  flags &= ACTION_MASK;

  /* First check for a strict equality requirement...
   */
  if( (min_wanted = req->GetPropVal( "eq", NULL )) != NULL )
    /*
     * ...and if specified, set the selection range such that
     * only the one specific release can match.
     */
    max_wanted = min_wanted;

  else
  { /* Check for either an inclusive, or a strictly exclusive,
     * minimum requirement (release "greater" than) specification,
     * setting the minimum release selector...
     */
    if( ((min_wanted = req->GetPropVal( "ge", NULL )) == NULL)
    &&  ((min_wanted = req->GetPropVal( "gt", NULL )) != NULL)  )
      /*
       * ...and its selection mode flag accordingly.
       */
      flags |= STRICTLY_GT;

    /* Similarly, check for an inclusive, or a strictly exclusive,
     * maximum requirement (release "less" than) specification,
     * setting the maximum release selector...
     */
    if( ((max_wanted = req->GetPropVal( "le", NULL )) == NULL)
    &&  ((max_wanted = req->GetPropVal( "lt", NULL )) != NULL)  )
      /*
       * ...and its selection mode flag accordingly.
       */
      flags |= STRICTLY_LT;
  }

  /* Return a canonical representation of the requirements spec.
   */
  return (min_wanted == NULL) ? max_wanted : min_wanted;
}

pkgXmlNode *pkgActionItem::SelectIfMostRecentFit( pkgXmlNode *package )
{
  /* Assign "package" as the "selection" for the referring action item,
   * provided it matches the specified selection criteria and it represents
   * a more recent release than any current selection.
   */
  pkgSpecs test( package );

  /* Establish the selection criteria...
   */
  pkgSpecs min_fit( min_wanted );
  pkgSpecs max_fit( max_wanted );

  /* Choose one of the above, as a basis for identification of
   * a correct package-component match...
   */
  pkgSpecs& fit = min_wanted ? min_fit : max_fit;

  /* Verify that "package" fulfills the selection criteria...
   */
  if(  match_if_explicit( test.GetComponentClass(), fit.GetComponentClass() )
  &&   match_if_explicit( test.GetComponentVersion(), fit.GetComponentVersion() )
  && ((max_wanted == NULL) || ((flags & STRICTLY_LT) ? (test < max_fit) : (test <= max_fit)))
  && ((flags & STRICTLY_GT) ? (test > min_fit) : (test >= min_fit))  )
  {
    /* We have the correct package component, and it fits within
     * the allowed range of release versions...
     */
    pkgSpecs last( selection );
    if( test > last )
      /*
       * It is also more recent than the current selection,
       * so we now replace that...
       */
      selection = package;
  }

  /* Whatever choice we make, we return the resultant selection...
   */
  return selection;
}

pkgActionItem* pkgXmlDocument::Schedule
( unsigned long action, pkgActionItem& item, pkgActionItem* rank )
{
  /* Schedule an action item with a specified ranking order in
   * the action list, (or at the end of the list if no ranking
   * position is specified)...
   */
  pkgActionItem *ref = rank ? rank : actions;

  /* Record the requested action code...
   */
  request = action;

  /* Don't reschedule, if we already have a prior matching item...
   */
  if(  (ref->GetReference( item ) == NULL)
  /*
   * ...but, when we don't, we raise a new scheduling request...
   */
  &&  ((ref = ref->Schedule( action, item )) != NULL)  )
  {
    /* ...and, when successfully raised, add it to the task list...
     */
    if( rank )
      /*
       * ...at the specified ranking position, if any...
       */
      return rank->Insert( ref );
    else
      /* ...otherwise, at the end of the list.
       */
      return actions = actions->Append( ref );
  }

  /* If we get to here, then no new action was scheduled; we simply
   * return the current insertion point in the task list.
   */
  return rank;
}

/* $RCSfile: pkgexec.cpp,v $: end of file */
