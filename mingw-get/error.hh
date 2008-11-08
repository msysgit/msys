#ifndef ERROR_HH_INC
#define ERROR_HH_INC


extern char mg_last_error[];


#ifdef __cplusplus
extern "C" {
#endif


void MGSetError(const char* fmt, ...);


static inline const char* MGLastError()
{
	return mg_last_error;
}


#ifdef __cplusplus
} // extern "C"
#endif


#endif // ERROR_HH_INC
