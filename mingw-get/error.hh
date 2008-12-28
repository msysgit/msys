#ifndef ERROR_HH_INC
#define ERROR_HH_INC


#ifdef __cplusplus
extern "C" {
#endif


void MGError(const char* fmt, ...);
const char* const* MGGetErrors();
void MGClearErrors();


#ifdef __cplusplus
} // extern "C"
#endif


#endif // ERROR_HH_INC
