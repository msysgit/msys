/* Include prototypes for all files */
/*
 * $Id: protos.h,v 1.1 2002/12/06 23:08:03 earnie Exp $
 */
#include "command.extpro"

#ifdef RXVT_GRAPHICS
# include "graphics.extpro"
#endif
#ifdef GREEK_SUPPORT
# include "grkelot.extpro"
#endif

#ifdef UTMP_SUPPORT
# include "logging.extpro"
#endif

#include "main.extpro"

#ifdef MENUBAR
# include "menubar.extpro"
#endif

#include "misc.extpro"

#ifdef DISPLAY_IS_IP
# include "netdisp.extpro"
#endif
#ifndef NO_STRINGS
# include "strings.extpro"
#endif

#include "screen.extpro"

#include "scrollbar.extpro"

#include "xdefaults.extpro"

#ifdef XPM_BACKGROUND
# include "xpm.extpro"
#endif
