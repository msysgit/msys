/********************************************************************
*	Class:	CList.cpp. This is part of WinUI.
*
*	Purpose:	A generic doubly linked list.
*
*	Authors:	Inspired of Listgen, by Claude Catonio.
*
*	License:	Public domain.
*			2002 Manu B.
*
*	Revisions:	
*			Manu B. 2001/10/05	Terms was translated to English.
*			Manu B. 2001/10/16	Add CList::Destroy(CNode *node) method.
*			Manu B. 2001/11/12	Add InsertBefore/InsertAfter methods.
*			Manu B. 2001/11/17	First() & Last() now returns an integer value.
*			Manu B. 2001/11/19	CNode::Destroy() returns next node by default.
*			Manu B. 2002/03/13	Suppress CNode methods.
*			Manu B. 2002/10/02	Add GetNext(), GetPrev().
*
********************************************************************/
#include "CList.h"
/* For MessageBox
#include <windows.h> */

/********************************************************************
*	Class:	CList.
*
*	Purpose:	Generic doubly linked list.
*
*	Revisions:	
*
********************************************************************/
CList::~CList(){
	//MessageBox (0, "CList", "destructor", MB_OK);
	while (_first != NULL){
		_current = _first;
		_first = _first->_next;
		delete _current;
	}
	_current = _last = _first;
	_count = 0;
}


/********************************************************************
*	Browse methods.
********************************************************************/
CNode *CList::First(){
	_current = _first;
return _current;
}

CNode *CList::Last(){
	_current = _last;
return _current;
}

CNode *CList::Prev(){
	if (_current != NULL){
		if(_current->_prev == NULL){
			// No previous node.
			return NULL;
		}else{
			// A previous one.
			_current = _current->_prev;
			return _current;
		}
	}
return NULL; // Empty list.
}

CNode *CList::Next(){
	if (_current != NULL){
		if(_current->_next == NULL){
			// No next node.
			return NULL;
		}else{
			// A next one.
			_current = _current->_next;
			return _current;
		}
	}
return NULL; // Empty list.
}


/********************************************************************
*	Insert methods.
********************************************************************/
void CList::InsertFirst(CNode *node){
	if(_first == NULL){
		// Empty list.
		_first = _last = node;
	}else{
		// Set node pointers.
		node->_prev	= NULL;
		node->_next 	= _first;
		// Insert in the list.
		_first->_prev	= node;
		_first 		= node;
	}
	// Set current node.
	_current = node;
	_count++;
}

void CList::InsertLast(CNode *node){
	if(_first == NULL){
		// Empty list.
		_first = _last = node;
	}else{
		// Set node pointers.
		node->_prev 	= _last;
		node->_next 	= NULL;
		// Insert in the list.
		_last->_next 	= node;
		_last 			= node;
	}
	// Set current node.
	_current = node;
	_count++;
}

void CList::InsertBefore(CNode *node){
	if(_first == NULL){
		// Empty list.
		_first = _last = node;
	}else{
		if (_current == _first)
			_first = node;
		// Set node pointers.
		node->_prev = _current->_prev;
		node->_next = _current;
		// Insert in the list.
		if (node->_prev)
			node->_prev->_next = node;
		_current->_prev = node;
	}
	// Set current node, increment the node counter.
	_current = node;
	_count++;
}

void CList::InsertAfter(CNode *node){
	if(_first == NULL){
		// Empty list.
		_first = _last = node;
	}else{
		if (_current == _last)
			_last = node;
		// Set node pointers.
		node->_prev = _current;
		node->_next = _current->_next;
		// Insert in the list.
		if (node->_next)
			node->_next->_prev = node;
		_current->_next = node;
	}
	// Set current node, increment the node counter.
	_current = node;
	_count++;
}


/********************************************************************
*	Destroy methods.
********************************************************************/
void CList::Detach(CNode *node){
	// Empty list ?
	if (_current != NULL){
		// Detach node from the list.
		if (node->_next != NULL)
			node->_next->_prev = node->_prev;
		if (node->_prev != NULL)
			node->_prev->_next = node->_next;
	
		// Set current node.
		if(node->_next != NULL)
			_current = node->_next;
		else
			_current = node->_prev;

		if (_current == NULL){
			// Now, the list is empty.
			_first = _last = NULL;

		}else if (_first == node){
			// Detached node was first.
			_first = _current;

		}else if (_last == node){
			// Detached node was last.
			_last = _current;
		}
		_count--;
	}
}

void CList::Destroy(CNode *node){
	// Empty list ?
	if (_current != NULL){
		// Detach node from the list.
		if (node->_next != NULL)
			node->_next->_prev = node->_prev;
		if (node->_prev != NULL)
			node->_prev->_next = node->_next;
	
		// Set current node.
		if(node->_next != NULL)
			_current = node->_next;
		else
			_current = node->_prev;

		if (_current == NULL){
			// Now, the list is empty.
			_first = _last = NULL;

		}else if (_first == node){
			// Detached node was first.
			_first = _current;

		}else if (_last == node){
			// Detached node was last.
			_last = _current;
		}
		_count--;
		delete node;
	}
}

void CList::DestroyCurrent(){
	Destroy(_current);
}

void CList::DestroyList(){
	while (_first != NULL){
		_current = _first;
		_first = _first->_next;
		delete _current;
	}
	_current = _last = _first;
	_count = 0;
}
