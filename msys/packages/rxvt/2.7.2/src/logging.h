#ifdef UTMP_SUPPORT
# if ! defined(HAVE_STRUCT_UTMPX) && ! defined(HAVE_STRUCT_UTMP)
#  error cannot build with utmp support - no utmp or utmpx struct found
# endif

# if defined(RXVT_UTMPX_FILE) && defined(HAVE_STRUCT_UTMPX)
#   define RXVT_UTMP_AS_UTMPX
# else
#  if defined(RXVT_UTMP_FILE) && defined(HAVE_STRUCT_UTMP)
#   undef RXVT_UTMP_AS_UTMPX
#  endif
# endif
/* if you have both utmp and utmpx files lying around and are really
 * using utmp not utmpx, then uncomment the following line */
/* #undef RXVT_UTMP_AS_UTMPX */

# ifdef RXVT_UTMP_AS_UTMPX
#  define RXVT_REAL_UTMP_FILE	RXVT_UTMPX_FILE
# else
#  define RXVT_REAL_UTMP_FILE	RXVT_UTMP_FILE
# endif

# ifdef RXVT_UTMP_AS_UTMPX
#  define USE_SYSV_UTMP
#  include <utmpx.h>
# else
#  include <utmp.h>
#  ifdef HAVE_SETUTENT
#   define USE_SYSV_UTMP
#  else
#   undef USE_SYSV_UTMP
#  endif
# endif

# ifdef HAVE_LASTLOG_H
#  include <lastlog.h>
# endif
# include <pwd.h>

# undef UTMP
# ifdef USE_SYSV_UTMP
#  ifndef USER_PROCESS
#   define USER_PROCESS		7
#  endif
#  ifndef DEAD_PROCESS
#   define DEAD_PROCESS		8
#  endif
#  ifdef RXVT_UTMP_AS_UTMPX
#   define UTMP			struct utmpx
#   define setutent		setutxent
#   define getutent		getutxent
#   define getutid		getutxid
#   define endutent		endutxent
#   define pututline		pututxline
#  endif
# endif
# ifndef UTMP
#  define UTMP			struct utmp
# endif

# ifdef WTMP_SUPPORT
#  ifdef RXVT_UTMP_AS_UTMPX
#   define update_wtmp		updwtmpx
#   ifdef RXVT_WTMPX_FILE
#    define RXVT_REAL_WTMP_FILE	RXVT_WTMPX_FILE
#   else
#    error cannot build with wtmp support - no wtmpx file found
#   endif
#  else
#   define update_wtmp		rxvt_update_wtmp
#   ifdef RXVT_WTMP_FILE
#    define RXVT_REAL_WTMP_FILE	RXVT_WTMP_FILE
#   else
#    error cannot build with wtmp support - no wtmp file found
#   endif
#  endif
# endif

# ifdef __QNX__
#  include <sys/utsname.h>
#  undef USE_SYSV_UTMP
#  define ut_name		ut_user
# endif

#include "logging.intpro"		/* PROTOS for internal routines */
#endif
