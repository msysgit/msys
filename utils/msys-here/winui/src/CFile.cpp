/********************************************************************
*	Module:	CFile.cpp. This is part of WinUI.
*
*	Purpose:	
*
*	Authors:	Manu B.
*
*	License:	Public domain.
*			2002 Manu B.
*
*	Revisions:	
*
********************************************************************/
#include <ctype.h>
#include "CFile.h"


bool Winify(char *src){
	while (*src){
		if (*src == '/')
			*src = '\\';
		src++;
	}
return true;
}

// FIXME: We should be able to choose between '\\' or '/' as separator.
// TODO: should be defined as macros.
bool IsRelative(const char *path){
	return (path && path[0] == '.' && (path[1] == '\0' || (path[1] == '\\' && path[2] != '\0')));
}

bool IsAbsolute(const char *path){
	return (path && isalpha(*path) && path[1] == ':' && path[2] == '\\' && path[3] != '\0');
}

// TODO: to be added to CFile.
size_t RelativeToAbsolute(char *dest, const char *dir, const char *fileName);


/********************************************************************
*	Class:	CPath.
*
*	Purpose:	Manages paths.
*
*	Revision:	1.0
*
********************************************************************/
CPath::CPath(){
	*_path = '\0'; // Test value.
	_offset = 0;
	_dirLevel = 0;
	_ftype = 0;
	_untitled = true;
}

CPath::~CPath(){
}

bool CPath::SetPath(CPath *path){
	if (path)
		return SetPath(path->GetPath());
return false;
}

bool CPath::SetPath(const char *fullpath){
	/* 	Accepts '.\file' or 'c:\file', initializes _path,
		_dirLevel, _offset and _ftype.			*/

	// Check fullpath and copy it to path.
	if (IsRelative(fullpath) || IsAbsolute(fullpath)){
		if (strlen(fullpath)+1 > _MAX_PATH)
			return false;
		char *dest = _path;
		const char *src = fullpath;
		const char *sep = NULL;
	
		_dirLevel = (WORD) -1;
		while (*src){
			if (*src == '\\'){
				sep = src;
				_dirLevel++;
			}
			*dest++ = *src++;
		}
		*dest++ = '\0';

		_offset = (sep - fullpath) +1;
		SetFileType(); // Virtual function.
		_untitled = false;
		return true;
        }
	*_path = '\0';
	_offset = 0;
	_dirLevel = 0;
	_ftype = 0;
	_untitled = false;
return false;
}

bool CPath::SetPath(const char *directory, const char *filename){
	// Check directory and copy it to path.
	if ((IsRelative(directory) || IsAbsolute(directory)) && filename && *filename){
		if ((strlen(directory) + strlen(filename) +1) > _MAX_PATH)
			return false;
		char fullpath[_MAX_PATH];
		sprintf(fullpath, "%s\\%s", directory, filename);
		return SetPath(fullpath);
	}
return false;
}

bool CPath::SetUntitled(const char *label){
	if (label && *label && _untitled){
		strcpy(_path, label);
		_offset = 0;
		_dirLevel = 0;
		_untitled = true;
		return true;
	}
return false;
}

bool CPath::GetDirectory(char *directory){
	if (_offset){
		strncpy(directory, _path, _offset);
		directory[_offset-1] = '\0';
		return true;
	}
return false;
}

const char *CPath::GetFileName(void){
	if (*_path)
		return (_path + _offset);
	else
		return NULL;
}

const char *CPath::GetFileExt(void){
	const char *p = _path + strlen(_path);
	while (p != _path){
		if (*--p == '.')
			return (p+1);
		else if (*p == '\\' || *p == '/')
			return NULL;
	}
return NULL;
}

bool CPath::RelativeToAbsolute(char *dest, const char *rootDirectory){
	return ::RelativeToAbsolute(dest, rootDirectory, _path);
}

bool CPath::AbsoluteToRelative(char *dest, const char *rootDirectory){
	return ::AbsoluteToRelative(dest, rootDirectory, _path);
}

bool CPath::ThisDirectory(const char *directory){
	if (directory && _offset)
		return (0 == strnicmp(directory, _path, _offset));
return false;
}

bool CPath::Rename(const char *name, const char *directory){
	/*	Rename doesn't check if "name" already exists in
		the owner list.							*/

	if (name && *name){
		char oldname[_MAX_PATH];
		char newname[_MAX_PATH];
		RelativeToAbsolute(oldname, directory);
		strcpy(newname, oldname);
		char *filename = strrchr(newname, '\\');
		if (filename){
			strcpy(filename+1, name);
	
			if (rename(oldname, newname) == 0){
				strcpy(_path + _offset, name);
				return true;
			}
		}
	}
return false;
}

bool CPath::Remove(const char *directory){
	// Delete the file.
	if (IsRelative(_path) && directory){
		char fullpath[_MAX_PATH];
		*fullpath = '\0';
		if (RelativeToAbsolute(fullpath, directory))
			return (remove(fullpath) == 0);
	}else if (IsAbsolute(_path)){
		return (remove(_path) == 0);
	}
return false;
}

bool CPath::ChangeFileExt(char *ext){
	// TODO: to be revised.
	int len = strlen(_path);
	for (int n=len; n > 0; n--){
		if (_path[n] == '.'){
			_path[n+1] = 0;
			strcat(_path, ext);
			return true;
		}
	}
return false;	// No file extension.
}

size_t RelativeToAbsolute(char *dest, const char *currentDir, const char *fileName){
	if (*fileName == '.'){
		if (*++fileName == '\\' && currentDir[1] == ':'){
			char *p = dest;
			while (*currentDir)
				*p++ = *currentDir++;
			*p++ = '\\';
			fileName++;
			while (*fileName)
				*p++ = *fileName++;
			*p++ = '\0';
			return strlen(dest);
		}else if (*fileName == '\0'){
			strcpy(dest, currentDir);
			return strlen(dest);
		}
	}
return 0;
}

bool AbsoluteToRelative(char *dest, const char *rootDir, const char *fileDir){
	/* Converts an absolute directory string returned to a project relative directory.
		Ex:
			Root directory:	C:\MyProjects\Project
			File directory:	C:\MyProjects\Project\SubDir1\SubDir2
			Returned string:	.\SubDir1\SubDir2				
	Used in (CProject::AddFiles)							*/

	/* Check if fileDir contains the project directory */
	int len = strlen(rootDir);
	int dirLen = strlen(fileDir)-len;
	if (strnicmp(rootDir, fileDir, len) == 0){
		/* Format a relative directory */
		strcpy(dest, ".");
		fileDir += len;

		if (*fileDir == '\0'){
			return true;
		}else if (*fileDir == '\\'){
			fileDir++;
			if (dirLen > 0 && dirLen < _MAX_DIR){
				strcat(dest, "\\");
				strncat(dest, fileDir, dirLen);
				return true;
			}
		}
	}
return false;
}


/********************************************************************
*	Class:	CFile.
*
*	Purpose:	File I/O.
*
*	Revision:	1.0
*
********************************************************************/
CFile::CFile(){
	_file = NULL;
}

CFile::~CFile(){
	if (_file)
		fclose(_file);
}

bool CFile::FOpen(const char *filename, const char *mode){
	/*	If filename is NULL, we fopen(_path).	*/
	if (!filename)
		filename = _path;
	else if (!SetPath(filename))
		return false;
	if (!_file && filename && mode)
		return ((_file = fopen(filename, mode)) != NULL);
return false;
}

bool CFile::FClose(void){
	if (_file && (fclose(_file) == 0)){
		_file = NULL;
		return true;
	}
return false;
}


/********************************************************************
*	Section:
*		Inifile parser.
*
*	Content:
*		CIniFile
*
*	Revisions:	
*
********************************************************************/
/********************************************************************
*	Class:	CIniFile.
*
*	Purpose:	"GetPrivateProfileString()" like procedures.
*
*	Revisions:	
*
********************************************************************/
CIniFile::CIniFile(){
	_buff	= NULL;
	_curr = NULL;
	_sect = NULL;
}

CIniFile::~CIniFile(){
	Close();
}

bool CIniFile::Open(const char *filename, const char *mode){
	/*	Allocates a buffer of MAX_BLOC_SIZE and load "fileName".
		First frees previous file buffer if there's one */

	if (FOpen(filename, mode)){
		_buff = (char *) malloc(MAX_BLOC_SIZE);
		if (!_buff)
			return false;

		/* FIXME: 	ZeroMemory because open/close projects can cause
				GetString to found data from previous projects. */
		::ZeroMemory(_buff, MAX_BLOC_SIZE);

		size_t bitesRead = fread(_buff, 1, MAX_BLOC_SIZE, _file);
		if (bitesRead > 0){
			// TODO: Allow to load files of any size.
			if (0 < fread(_buff, 1, MAX_BLOC_SIZE, _file)){
				::MessageBox(0, "CIniFile ERROR: OVERFOW", "CIniFile", MB_OK | MB_ICONERROR);
				Close();
				return false;
			}
		}

		// Initialize the pointer to current char.
		_curr = _buff;
		return true;
	}
return false;
}

bool CIniFile::Close(void){
	if (_buff)
		free(_buff);
	_buff	= NULL;
	_curr = NULL;
	_sect = NULL;
return FClose();
}

bool CIniFile::GetSection(char *sectionName){
	/*	Resets section pointer and search "sectionName".	*/
	_sect = NULL;

	if (sectionName && _curr && _buff){
		// Parse once from current char until the end of buffer.
		char *stopPos = _curr;
		while (*_curr){
			_sect = FindSection(sectionName, _curr);
			if (_sect)
				return true; // Found !
			_curr++;
		}
		// If not found, parse again from the beginning to stopPos.
		if (!_sect){
			_curr = _buff;
			while (_curr < stopPos){
				_sect = FindSection(sectionName, _curr);
				if (_sect)
					return true; // Found !
				_curr++;
			}
		}
	}
return false;
}

//******************************************************
//	Reminder :
//	GetPrivateProfileString(sectionName, keyName, defData, 
//						destData, maxLen, fileName);
//******************************************************
bool CIniFile::GetString(char *destData, char *keyName, char *defData, size_t maxLen){
	/*	Search "keyName" and copies the value to "destData".
		Returns true if successfull, false otherwise. 			*/

	// Is there a section ?
	if (_sect){
		// Parse once from current char until the end of section.
		char *stopPos = _curr;
		while (*_curr != '\0' && *_curr != '['){
			if (FindData(destData, keyName, defData, maxLen, _curr))
				return true; // Found !
			_curr++;
		}
		// If not found, parse again from the beginning to stopPos.
		_curr = _sect;
		while (_curr < stopPos){
			if (FindData(destData, keyName, defData, maxLen, _curr))
				return true; // Found !
			_curr++;
		}
	}else{
		CpyString(destData, defData, maxLen);
	}
return false;
}

int CIniFile::GetInt(char *key, int defVal){
	char integer[12];
	if (GetString(integer, key, "", sizeof(integer)))
		return atoi(integer);
	else
		return defVal;
}


// Private members.
char *CIniFile::FindSection(char *sectionName, char *curr){
	/*	Search the section through one line of text.
		Returns a pointer to the section if successful, NULL otherwise.
		This procedure increments current pointer to the end of line.	*/

	if (!sectionName || !curr)
		return NULL;

	bool sectionFound = false;

	/* Skip spaces and comments */
	curr = SkipUnwanted(curr);

	/* End of buffer ? */
	if (*curr == '\0'){
		_curr = curr;
		return NULL;

	/* A section ? */
	}else if (*curr == '['){
		curr++;
		if (*curr == '\0'){
			_curr = curr;
			return NULL;
		}

		/* Parse the section name */
		int len = strlen(sectionName);
		if (!strncmp(curr, sectionName, len)){
			curr+=len;
			if (*curr == ']'){
				/* Section found ! */
				sectionFound = true;
			}
		}
	}

	/* Increment current pointer until the end of current line */
	while (*curr != '\0' && *curr != '\n')
		curr++;
	_curr = curr;

	if (sectionFound)
		return curr;
return NULL;
}

bool CIniFile::FindData(char *destData, char *keyName, char *defData, size_t maxLen, char *curr){
	/*	Search "keyName" through one line of text.
		Returns true if successful, false otherwise.
		This procedure increments current pointer to the end of line.	*/

	bool keyFound = false;
	// Skip spaces and comments.
	curr = SkipUnwanted(curr);

	// End of buffer ? End of section ?
	if (*curr == '\0' || *curr == '['){
		CpyString(destData, defData, maxLen);
		_curr = curr;
		return false;

	/* Search the key */
	}else{
		int len = strlen(keyName);
		/* Compare key and curr */
		if (!strncmp(curr, keyName, len)){
			curr+=len;
			/* Search an '=' sign */
			while (isspace(*curr)){
				if (*curr == '\n')
					break;
				curr++;
			}

			if (*curr == '='){
				/* Key found ! */
				keyFound = true;
				curr++;
				while (isspace(*curr)){
					if (*curr == '\n')
						break;
					curr++;
				}

				/* Copy data ! */
				curr += CpyString(destData, curr, maxLen);
			}
		}
	}

	/* Increment current pointer until the end of current line */
	while (*curr != '\0' && *curr != '\n')
		curr++;
	_curr = curr;

	if (!keyFound)
		CpyString(destData, defData, maxLen);
return keyFound;
}

size_t CIniFile::CpyString(char *destData, const char *curr, size_t maxLen){
	/*	Returns the number of characters copied not including
		the terminating null char.						*/

	char *ch = destData;
	while (*curr != '\0' && *curr != '\r' && *curr != '\n'){
		// Truncates the string.
		if ((size_t) (ch-destData+1) == maxLen)
			break;
		*ch++ = *curr++;
	}
	*ch = '\0';
return (ch-destData);
}

char * CIniFile::SkipUnwanted(char *curr){
	for ( ; ; ){		
		/* End of buffer ? */
		if (*curr == '\0')
			return curr;
		if (isspace(*curr)){
			/* Skip spaces */
			curr++;
		}else if (*curr == ';'){
			/* Skip comments */
			char *end_of_line = strchr(curr, '\n');
			if (end_of_line){
				/* End of comment */
				curr = end_of_line+1;
				continue;
			}else{
				/* End of buffer */
				return end_of_line;
			}
		/* Done */
		}else{
			return curr;
		}
	}
}
