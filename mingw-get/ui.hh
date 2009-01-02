/** \file ui.hh
 *
 * Created: JohnE, 2008-11-08
 */
#ifndef UI_HH_INC
#define UI_HH_INC


#ifdef __cplusplus
extern "C" {
#endif


void UI_UpdateLists();
void UI_OnCategoryChange(int const* categories);
void UI_SortListView(int column);
void UI_OnListViewSelect(int sel);
void UI_OnStateCycle(int sel);
void LastError_MsgBox(const char* title);
void UI_RefreshCategoryList();


#ifdef __cplusplus
} //extern "C"
#endif


#endif // UI_HH_INC
