/********************************************************************
*	Module:	CList.h. This is part of WinUI.
*
*	License:	Public domain.
*			2002 Manu B.
*
********************************************************************/
#ifndef CLIST_H
#define CLIST_H

#include <stdlib.h>

class CList;

class CNode {
	public:
	CNode() {_next = _prev = NULL; _type = 0;};
	~CNode() {};

	CNode *GetNext(void) {return _next;}
	CNode *GetPrev(void) {return _prev;}
	unsigned int GetType(void) {return _type;}

	protected:
	CNode *_prev;
	CNode *_next;

	friend class CList;
	// The type must be set in the constructor of a derived class.
	unsigned int _type;
};

class CList {
	public:
	CList() {_first = _last = _current = NULL; _count = 0;};
	~CList();

	bool IsEmpty(void) {return (_count != 0);}
	unsigned int GetCount() {return _count;};
	CNode *GetCurrent() {return _current;};
	CNode *First();
	CNode *Last();
	CNode *Prev();
	CNode *Next();

	void	InsertFirst(CNode *node);
	void	InsertLast(CNode *node);
	void	InsertBefore(CNode *node);
	void	InsertAfter(CNode *node);

	void	Detach(CNode *node);
	void	Destroy(CNode *node);
	void	DestroyCurrent();
	void	DestroyList();

	protected:
	CNode *_first;
	CNode *_last;
	CNode *_current;
	unsigned int _count;
};

#endif
