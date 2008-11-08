/** \file previnstalls.hh
 *
 * Created: JohnE, 2008-11-08
 */
#ifndef PREVINSTALLS_HH_INC
#define PREVINSTALLS_HH_INC


#ifdef __cplusplus
extern "C" {
#endif


int GetPrevInstalls(void (*callback)(const char*));


#ifdef __cplusplus
} //extern "C"
#endif


#endif // PREVINSTALLS_HH_INC
