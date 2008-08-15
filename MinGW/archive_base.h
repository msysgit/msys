/** \file archive_base.h
 *
 * Created: JohnE, 2008-07-11
 */


/*
DISCLAIMER:
The author(s) of this file's contents release it into the public domain, without
express or implied warranty of any kind. You may use, modify, and redistribute
this file freely.
*/


#ifndef ARCHIVE_BASE_H_INC
#define ARCHIVE_BASE_H_INC


#include <time.h>


struct attr_item
{
  struct attr_item  *next;
  char              *fname;
  int                mode;
  time_t             time;
};


const char* archive_geterror();
void archive_seterror(const char* err_fmt, ...);
int makedir(const char* base, const char* newdir);
int setfiletime(const char* fname, time_t ftime);
int push_attr(struct attr_item** list, const char* fname, int mode, time_t time);
void restore_attr(struct attr_item** list);


#endif // ARCHIVE_BASE_H_INC
