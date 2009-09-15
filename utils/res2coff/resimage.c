/*
 *  RESimage.c 
 *
 *  This module is devoted to treating the .RES files
 */
 
#include "res2coff.h"

// Initialise the Resource Information Tree

void InitResourceInfo(void)
{
  int i;

  for (i=0;i<MAX_TYPES;i++) {
    ResInfo[i].count = ResInfo[i].named = ResInfo[i].unnamed = 0;
    ResInfo[i].RName = ResInfo[i].RID = NULL;
  }
}

// Free the Resource Information Tree

void FreeResourceInfo(void)
{
  int i;

#ifdef NDEBUG
  if (verbose)
    printf("Resources found in file\n Named Unnamed Type\n");
#endif
  
  for (i=0;i<MAX_TYPES;i++)
    {
#ifdef NDEBUG
      if ((verbose) && (ResInfo[i].count))
	printf(" %4d %6d   %s\n",
	       ResInfo[i].named,
	       ResInfo[i].unnamed,
	       szResourceTypes[i]);
#endif
      if (ResInfo[i].RName)
	free(ResInfo[i].RName);
      if (ResInfo[i].RID)
	free(ResInfo[i].RID);
    }
}

// Add a named resource of a given type to the Resource Information Tree

int AddNamedResource(int type,PRESOURCEHEAD pNewResource)
{
  PRESOURCEHEAD *pTemp;
  int           result;
  int           last = ResInfo[type].named;

  pTemp = (PRESOURCEHEAD *)realloc(ResInfo[type].RName,
				   sizeof(PRESOURCEHEAD)*(last+1));
  ResInfo[type].RName = pTemp;
  if (result = (pTemp) ? TRUE : FALSE)
    {
      ResInfo[type].RName = pTemp;
      ResInfo[type].count++;
      ResInfo[type].named++;
      pTemp[last] = pNewResource;
    }

  return result;
}

// Add a numbered resource of a given type to the Resource Information Tree

int AddIDResource(int type,PRESOURCEHEAD pNewResource)
{
  PRESOURCEHEAD *pTemp;
  int           result;
  int           last = ResInfo[type].unnamed;

  pTemp = (PRESOURCEHEAD *)realloc(ResInfo[type].RID,
				   sizeof(PRESOURCEHEAD)*(last+1));
  ResInfo[type].RID = pTemp;
  if (result = (pTemp) ? TRUE : FALSE)
    {
      ResInfo[type].RID = pTemp;
      ResInfo[type].count++;
      ResInfo[type].unnamed++;
      pTemp[last] = pNewResource;
    }
  return result;
}

// Functions to analyse a resource header structure in a RES32 file

int IsTypeCoded(PRESOURCEHEAD this)
{
  WORD *pTemp;

  pTemp = (WORD *)&(this->Type);
  return (*pTemp == 0xffff) ? TRUE : FALSE;
}

int TypeCode(PRESOURCEHEAD this)
{
  WORD *pTemp;

  pTemp = (WORD *)&(this->Type);
  if (*pTemp != 0xffff)
    return 0x7fff;		/* RT_ERROR */
  return *(++pTemp);
}

int IsNameCoded(PRESOURCEHEAD this)
{
  WORD *pTemp;

  pTemp = (WORD *)&(this->Type);
  if (*pTemp++ == 0xffff)
    pTemp++;
  else
    while (*pTemp++)
      ;
  return (*pTemp == 0xffff) ? TRUE : FALSE;
}

int IDCode(PRESOURCEHEAD this)
{
  WORD *pTemp;

  pTemp = (WORD *)&(this->Type);
  if (*pTemp++ == 0xffff)
    pTemp++;
  else
    while (*pTemp++)
      ;
  if (*pTemp++ == 0xffff)
    return *pTemp;
  else
    return 0x7fff;		/* RT_ERROR */
}

// This function returns the pointer to the start of the
// name in a named resource

wchar_t *ResName(PRESOURCEHEAD this)
{
  wchar_t *result;

  result = (wchar_t *)&this->Type;
  if (*result == 0xffff)
    return result + 2;

  while (*result)
    result++;

  return result;
}

// This function returns the Codepage information

WORD LanguageID(PRESOURCEHEAD this)
{
  return 0x434;
}

// This function parses a resource, including it in the
// Resource Information Tree and returns a pointer to the
// next resource in the RES32 file image. 
// Returns NULL on error

PRESOURCEHEAD ParseResource(PRESOURCEHEAD this)
{
#ifndef NDEBUG
  char *MkAString(wchar_t *lstr)
    {
      char *result,*p;
      int Alen = lstrlenW(lstr) + 1;

      result = malloc(Alen);
      if (result)
	{
	  p = result;
	  do
	    {
	      *(p++) = (char)(*(lstr++));
	    }
	  while (p[-1]);
	}
      return result;
    }
  
  char *PascalWString(wchar_t *PWStr)
    {
      char *result,*p;
      int len;

      len = *PWStr;
      PWStr++;
      
      if (len == 0)
	return "";

      result = malloc(len+1);
      if (result == NULL)
	return "Alloc error!!!";

      p=result;
      while (len--)
	*p++ = (char) *PWStr++;

      *p = 0;
      return result;
    }
#endif
  
  BYTE *pNext;
  int rest,Offset2Next;
  int typecode = TypeCode(this);

  Offset2Next = this->DataSize + this->HeaderSize;
  if (rest = Offset2Next % sizeof(DWORD))
    Offset2Next += sizeof(DWORD) - rest;

  pNext = (BYTE *) this + Offset2Next;

  if (!IsTypeCoded(this)) {
    printf(" ERROR: Named resource types not yet supported!\n");
    return NULL;
  }

  if (IsNameCoded(this))
    {      
#ifndef NDEBUG
      printf("Unnamed resource of type %d (%s) : [%04x]\n",
	     typecode,
	     szResourceTypes[typecode],
	     IDCode(this));
#endif
      AddIDResource(typecode,(PRESOURCEHEAD)this);
    }
  else
    {      
#ifndef NDEBUG
      printf("Named resource of type %d (%s) : [%s]\n",
	     typecode,
	     szResourceTypes[typecode],
	     MkAString(ResName(this)));
#endif
      AddNamedResource(TypeCode(this),(PRESOURCEHEAD)this);
    }

#ifndef NDEBUG
  if (typecode == 6)		/* String table */
    {
      int code,firstcode;
      wchar_t *StringFromTable;
	
      if (!IsNameCoded(this))
	printf("Named string block !!!???\n");
      else
	{
	  StringFromTable = (wchar_t *)this;
	  StringFromTable += 16;
	  
	  code = (IDCode(this)-1) * 16;
	  do
	    {
	      printf(" %6d %s\n",code++,PascalWString(StringFromTable));
	      StringFromTable += (*StringFromTable) + 1;
	    }
	  while (code & 0x000f);
	}
    }
#endif
  return (PRESOURCEHEAD) pNext;
}

// This function checks the whole resource file image
// for conformancy to the RES32 file format and creates
// the Resource Information Tree. Returns TRUE on compliance
// or FALSE if error detected

int Check32bitResourceFile(PRESOURCEHEAD FileBuf)
{
  LPVOID pTemp;

// The first record is an invalid record with
// predefined contents.

  int result = ((FileBuf->DataSize == 0) &&
		(FileBuf->HeaderSize == 32) &&
		(FileBuf->Type == 0xffff) &&
		(FileBuf->Name == 0xffff) &&
		(FileBuf->DataVersion == 0) &&
		(FileBuf->MemoryFlag == 0) &&
		(FileBuf->LanguageID == 0) &&
		(FileBuf->Version  == 0) &&
		(FileBuf->Characteristics == 0)) ? TRUE : FALSE;

// the '32' in 'FileBuf + 32' is the size of the
// Resource32Header aligned to DWORD boundary

  if (result)
    {
      pTemp = (BYTE *)FileBuf + 32;
      while ( (pTemp) && (pTemp < RESviewEnd) )
	pTemp = ParseResource((PRESOURCEHEAD) pTemp);
      
      result = (pTemp) ? TRUE : FALSE;
    }

  return result;
}

/* This procedure sorts the resource tree in order to have */
/* the unnamed resources sorted in ascending order */

void SortUnnamedResources(void)
{
  int type;

  int ResourceCmp(const void *a,const void *b)
    {
      int AName = ((*(PRESOURCEHEAD*)a)->Name) >> 16;
      int BName = ((*(PRESOURCEHEAD*)b)->Name) >> 16;

      return AName-BName;
    }

  for (type = 1;type < 17;type++)
    if (ResInfo[type].unnamed)
      qsort(ResInfo[type].RID,
	    ResInfo[type].unnamed,
	    sizeof(PRESOURCEHEAD),
	    ResourceCmp);
}
