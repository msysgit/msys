/********************************************************************
*	Module:	CFile.h. This is part of WinUI.
*
*	License:	Public domain.
*			2002 Manu B.
*
********************************************************************/
#ifndef CFILE_H
#define CFILE_H

#include "winui.h"

bool Winify(char *src);

// TODO: to be added to CPath.
bool AbsoluteToRelative(char *dest, const char *rootDir, const char *fileDir);

class CPath {
	public:
	CPath();
	virtual ~CPath();

	// Initializes.
	bool SetPath(CPath *path);
	bool SetPath(const char *fullpath = NULL);
	bool SetPath(const char *directory, const char *filename);
	virtual bool SetFileType(void) {return true;}
	bool SetUntitled(const char *label);
	// Get file properties.
	const char *GetPath(void) {return _path;}
	unsigned int GetFileType(void) {return _ftype;}
	bool IsUntitled(void) {return _untitled;}
	bool GetDirectory(char *directory);
	const char *GetFileName(void);
	const char *GetFileExt(void);
	WORD GetDirLevel(void) {return _dirLevel;}
	// Comparison.
	bool ThisDirectory(const char *directory);
	// Conversions.
	bool RelativeToAbsolute(char *dest, const char *rootDirectory);
	bool AbsoluteToRelative(char *dest, const char *rootDirectory);
	bool ChangeFileExt(char *ext);
	// File I/O.
	bool Rename(const char *name, const char *directory);
	bool Remove(const char *directory = NULL);

	protected:
	char _path[MAX_PATH]; 
	unsigned int _ftype;

	private:
	WORD _offset; 
	WORD _dirLevel; 
	// A particular kind of file .
	bool _untitled;
};


class CFile : public CPath {
	public:
	CFile();
	virtual ~CFile();

	FILE *GetFile(void) {return _file;}
	bool FOpen(const char *filename, const char *mode);
	bool FClose(void);

	protected:
	FILE *_file;
};


/********************************************************************
*	Section:
*		A ini-file parser.
*
*	Content:
*		CIniFile
*
*	Revisions:	
*
********************************************************************/
#define MAX_BLOC_SIZE 16384

class CIniFile : public CFile {
	public:
	CIniFile();
	~CIniFile();

	bool	Open(const char *filename, const char *mode);
	bool	Close(void);
	bool	GetSection(char *sectionName);
	bool	GetString(char *destData, char *keyName, char *defData, size_t maxLen);
	int	GetInt(char *key, int defVal);

	private:   
	char	*FindSection(char *sectionName, char *curr);
	bool	FindData(char *destData, char *keyName, char *defData, size_t maxLen, char *curr);
	size_t CpyString(char *destData, const char *curr, size_t maxLen);
	char	*SkipUnwanted(char *curr);

	// Pointers to the file buffer, the current char and
	// the current section.
	char *_buff;
	char *_curr;
	char *_sect;
};

#endif
