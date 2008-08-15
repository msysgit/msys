/** \file multiread.h
 *
 * Created: JohnE, 2008-07-11
 */


/*
DISCLAIMER:
The author(s) of this file's contents release it into the public domain, without
express or implied warranty of any kind. You may use, modify, and redistribute
this file freely.
*/


#ifndef MULTIREAD_H_INC
#define MULTIREAD_H_INC


typedef struct _MultiReader MultiReader;


MultiReader* CreateGZReader(const char *file);
MultiReader* CreateBZ2Reader(const char *file);
void DestroyMultiReader(MultiReader *mr);
int MultiRead(MultiReader *mr, void *buf, unsigned len);
const char *MultiError(MultiReader *mr, int *errnum);


#endif // MULTIREAD_H_INC
