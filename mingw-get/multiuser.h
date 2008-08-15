#ifndef MULTIUSER_H_INC
#define MULTIUSER_H_INC


#define WIN32_LEAN_AND_MEAN
#include <windows.h>


enum
{
	MU_USER = 0,
	MU_GUEST,
	MU_POWER,
	MU_ADMIN
};


int GetAccountTypeHelper();


#endif // MULTIUSER_H_INC
