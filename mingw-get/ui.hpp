#ifndef UI_HPP_INC
#define UI_HPP_INC


#include <string>


class Package;


class UI
{
public:
	static void ResetLists();
	static void NotifyNewCategory(const char* name);
	static void NotifyNewPackage(const Package& pkg);
};


#endif // UI_HPP_INC
