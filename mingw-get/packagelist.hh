/** \file packagelist.hh
 *
 * Created: JohnE, 2009-01-03
 */
#ifndef PACKAGELIST_HH_INC
#define PACKAGELIST_HH_INC


#ifdef __cplusplus
extern "C" {
#endif


void PackageList_Create();
void PackageList_Sort(int column);
void PackageList_OnSelect(int sel);
void PackageList_OnStateCycle(int sel);
void PackageList_SetCategories(const int* categories);


#ifdef __cplusplus
} //extern "C"
#endif


#endif // PACKAGELIST_HH_INC
