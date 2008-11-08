#ifndef MULTIUSER_HH_INC
#define MULTIUSER_HH_INC


#define WIN32_LEAN_AND_MEAN
#include <windows.h>


#ifdef __cplusplus
extern "C" {
#endif


enum
{
	MU_USER = 0,
	MU_GUEST,
	MU_POWER,
	MU_ADMIN
};


int GetAccountTypeHelper();


#ifdef __cplusplus
} //extern "C"
#endif


#endif // MULTIUSER_HH_INC
