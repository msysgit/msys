#include <limits.h>

#define LBLOCKSIZE (sizeof (long))
#define UNALIGNED(X) ((long)X & (long)(sizeof (long) -1))
#define ALIGNED(X) !UNALIGNED(X)
#define DETECTNULL(X) (((X) - LOW_BITS) & ~(X) & HIGH_BITS)
#define DETECTCHAR(X,MASK) (DETECTNULL(X ^ MASK))

#if LONG_MAX == 2147483647L
# define LBYTECOUNT 4
#elif LONG_MAX == 9223372036854775807L
# define LBYTECOUNT 8
#else
# error long int is not a 32bit or 64bit type.
#endif

#if LBYTECOUNT == 4
# define LOW_BITS  0x01010101
# define HIGH_BITS 0x80808080
#elif LBYTECOUNT == 8
# define LOW_BITS  0x0101010101010101
# define HIGH_BITS 0x8080808080808080
#elif LBYTECOUNT == 16
# define LOW_BITS  0x01010101010101010101010101010101
# define HIGH_BITS 0x80808080808080808080808080808080
#else
# error 4, 8 and 16 bytes only are supported.
#endif

