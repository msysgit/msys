#include "rxvtlib.h"

/*----------------------------------------------------------------------*/
/* main() */
/* INTPROTO */
int
main(int argc, const char *const *argv)
{
    rxvt_t         *rxvt_vars;

    if ((rxvt_vars = rxvt_init(argc, argv)) == NULL)
	return EXIT_FAILURE;
    rxvt_main_loop(rxvt_vars);	/* main processing loop */
    return EXIT_SUCCESS;
}
