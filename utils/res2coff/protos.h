// res2coff.c

void WriteError(LPCSTR);
void AppExit(int,char *,...);
void CleanUp(void);
void CheckedWrite(HANDLE,LPVOID,DWORD,char *);
void usage(char);

// OBJimage.c

int  Add2StringTable(wchar_t *);
int  AppendMemImage(MemImage *,MemImage *);
int  AddRelocation(int);
int  MakeDirectoryTree(void);
int  MakeSymbols(char *);

void DumpResources(HANDLE);
void DumpDirectoryImage(HANDLE);
void DumpRelocations(HANDLE);
void DumpSymbols(HANDLE);
void MakeCOFFSections(HANDLE);

// RESimage.c

void InitResourceInfo(void);
void FreeResourceInfo(void);
int AddNamedResource(int,PRESOURCEHEAD);
int AddIDResource(int,PRESOURCEHEAD);
int IsTypeCoded(PRESOURCEHEAD);
int TypeCode(PRESOURCEHEAD);
int IsNameCoded(PRESOURCEHEAD);
int IDCode(PRESOURCEHEAD);
wchar_t *ResName(PRESOURCEHEAD);
WORD LanguageID(PRESOURCEHEAD);
PRESOURCEHEAD ParseResource(PRESOURCEHEAD);
int Check32bitResourceFile(PRESOURCEHEAD);
void SortUnnamedResources(void);
