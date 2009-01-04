/** \file progressthread.hh
 *
 * Created: JohnE, 2009-01-03
 */
#ifndef PROGRESSTHREAD_HH_INC
#define PROGRESSTHREAD_HH_INC


#ifdef __cplusplus
extern "C" {
#endif


typedef struct ProgressThreadInfoStruct ProgressThreadInfo;


int ProgressThread
 (const char* dialog_title,
  int (*thread_func)(ProgressThreadInfo*, void*),
  void* thread_func_param);

void ProgressThread_NewStatus(ProgressThreadInfo* info, const char* status);
int ProgressThread_IsCancelled(ProgressThreadInfo* info);


#ifdef __cplusplus
} //extern "C"
#endif


#endif // PROGRESSTHREAD_HH_INC
