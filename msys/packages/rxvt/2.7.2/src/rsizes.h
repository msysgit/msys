/*
 * If we haven't pulled in typedef's like int16_t , then do them ourself
 */

/* type of (normal and unsigned) basic sizes */
/* e.g. typedef short int16_t */

/* e.g. typedef unsigned short u_int16_t */

/* e.g. typedef int int32_t */

/* e.g. typedef unsigned int u_int32_t */

/* e.g. typedef long int64_t */

/* e.g. typedef unsigned long u_int64_t */


/* whatever normal size corresponds to a integer pointer */
#define intp_t int32_t
/* whatever normal size corresponds to a unsigned integer pointer */
#define u_intp_t u_int32_t
