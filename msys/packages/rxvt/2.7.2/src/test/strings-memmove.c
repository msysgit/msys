#include "../../config.h"
#include "../rxvt.h"

# /* hack! */	define memset		rmemset
# /* hack! */	define memmove		rmemmove
# /* hack! */	include "../strings.extpro"
# /* hack! */	include "../strings.c"
# /* hack! */	undef memmove
# /* hack! */	undef memset

#define OFF	16
#define BUFSZ	300

unsigned char  buf1[BUFSZ], buf2[BUFSZ], buf3[BUFSZ];

main()
{
    int i, j, k, a, b;
    int res;

    res = 0;
    for (i = 0; i < OFF; i++)
	for (j = 0; j < OFF; j++)
	    for (k = 0; k < OFF; k++) {

		memset(buf2, 127, BUFSZ);
		memset(buf3, 127, BUFSZ);
		for (a = 1; a < 256; a++)
		    buf1[OFF + i + a - 1] = a;
		rmemmove(buf2 + OFF + k, buf1 + OFF + i, 256 - OFF + j);
		memmove(buf3 + OFF + k, buf1 + OFF + i, 256 - OFF + j);
		for (a = 0; a < sizeof buf2; a++)
		    if (buf2[a] != buf3[a]) {
			res = 1;
			printf("Test BAD, Start = +%d, Size = +%d, Dest =+ %d: ",
			       i, j, k);
			printf("    -1=%02x, 0=%02x, %d=%02x, %d=%02x\n",
			       buf2[OFF + k - 1], buf2[OFF + k],
			       255 - OFF + j, buf2[k + 255 + j],
			       256 - OFF + j, buf2[k + 256 + j]);
			break;
		    }
	    }
    if (res == 0)
	printf("OK\n");
    exit(res);
}
