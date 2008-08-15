/** \file nsis_interface.hpp
 *
 * Created: JohnE, 2008-07-26
 */


/*
DISCLAIMER:
The author(s) of this file's contents release it into the public domain, without
express or implied warranty of any kind. You may use, modify, and redistribute
this file freely.
*/


#ifndef NSIS_INTERFACE_HPP_INC
#define NSIS_INTERFACE_HPP_INC


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>


typedef std::string StringType;


#define NSDFUNC __stdcall
#define WM_NOTIFY_INIGO_MONTOYA (WM_USER+0xb)
#define WM_TREEVIEW_KEYHACK (WM_USER+0x13)


extern "C" {


typedef struct _stack_t {
  struct _stack_t *next;
  char text[1]; // this should be the length of string_size
} stack_t;


typedef struct {
  int autoclose;
  int all_user_var;
  int exec_error;
  int abort;
  int exec_reboot;
  int reboot_called;
  int XXX_cur_insttype; // deprecated
  int XXX_insttype_changed; // deprecated
  int silent;
  int instdir_error;
  int rtl;
  int errlvl;
  int alter_reg_view;
} exec_flags_type;


typedef struct {
  exec_flags_type *exec_flags;
  int (__stdcall *ExecuteCodeSegment)(int, HWND);
  void (__stdcall *validate_filename)(char *);
} extra_parameters;


typedef int nsFunction;


} //extern "C"


class NSIS
{
public:
	static void UpdateParams
	 (int string_size,
	  char* variables,
	  stack_t** stacktop,
	  extra_parameters* extra);

	static void pushstring(const char* str);
	static StringType popstring();
	static void pushint(int value);
	static int popint();

	static void ExecuteCallback(nsFunction func);

private:
	static int s_stringsize;
	static char* s_variables;
	static stack_t** s_stacktop;
	static extra_parameters* s_extra;
};


#endif // NSIS_INTERFACE_HPP_INC
