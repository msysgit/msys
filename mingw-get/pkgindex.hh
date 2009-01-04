/** \file pkgindex.hh
 *
 * Created: JohnE, 2009-01-03
 */
#ifndef PKGINDEX_HH_INC
#define PKGINDEX_HH_INC


#ifdef __cplusplus
extern "C" {
#endif


int PkgIndex_Load();
void PkgIndex_Clear();

int PkgIndex_NumHeadings();
const char* PkgIndex_GetHeading(int heading);
const int* PkgIndex_GetHeadingChildren(int heading);

const char* PkgIndex_GetCategory(int category);

int PkgIndex_NumPackages();
int PkgIndex_PackageInAnyOf(int package, const int* categories);
const char* PkgIndex_PackageGetID(int package);
const char* PkgIndex_PackageGetSubItemText(int package, int subitem);
const char* PkgIndex_PackageGetTitle(int package);
const char* PkgIndex_PackageGetDescription(int package);
const char* PkgIndex_PackageGetInstalledVersion(int package);
const char* PkgIndex_PackageGetLatestVersion(int package);
int PkgIndex_PackageNumVersions(int package);
const char* PkgIndex_PackageVersionGetString(int package, int version);
void PkgIndex_PackageSetSelectedVersion(int package, int version);
int PkgIndex_PackageVersionGetStatus(int package, int version);
int PkgIndex_PackageGetSelectedAction(int package);
void PkgIndex_PackageSelectAction(int package, int action);
int PkgIndex_PackageGetStateImage(int package);


#ifdef __cplusplus
} //extern "C"
#endif


#endif // PKGINDEX_HH_INC
