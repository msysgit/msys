// msys.h
// Copyright (C) 2002, 2003 Earnie Boyd <earnie@users.sf.net>
//
// This file is a part of MSYS.

#ifndef MSYS_H
#define MSYS_H

void __AbsDllPath( const char *, char *, int );
extern "C" void AbsDllPath( const char *, char *, int );
void __AbsExeModPath( char *, int );
extern "C" void AbsExeModPath( char *, int );

class auto_lock
{
  public:
    auto_lock (CRITICAL_SECTION &lock_param)
      : lock (lock_param)
    {
      EnterCriticalSection (&lock);
    }
    ~auto_lock ()
    {
      LeaveCriticalSection (&lock);
    }

  private:
    CRITICAL_SECTION &lock;
};

#endif
