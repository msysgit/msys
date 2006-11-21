; plugins required
; untgz     - http://nsis.sourceforge.net/wiki/UnTGZ
; inetc     - http://nsis.sourceforge.net/Inetc_plug-in
;             http://forums.winamp.com/showthread.php?s=&threadid=198596&perpage=40&highlight=&pagenumber=4
;             http://forums.winamp.com/attachment.php?s=&postid=1831346

; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "MinGW"
!define PRODUCT_VERSION "5.1.0"
!define PRODUCT_PUBLISHER "MinGW"
!define PRODUCT_WEB_SITE "http://www.mingw.org"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"
!define PRODUCT_STARTMENU_REGVAL "NSIS:StartMenuDir"
!define BUILD "7"

SetCompressor lzma

; MUI 1.67 compatible ------
!include "MUI.nsh"
!include "Sections.nsh"
!include "StrFunc.nsh"
${StrTok}

; MUI Settings
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "mingw.bmp" ; optional
!define MUI_ABORTWARNING "Are you sure you want to quit ${PRODUCT_NAME} ${PRODUCT_VERSION}?"
!define MUI_COMPONENTSPAGE_SMALLDESC

; Welcome page
!define MUI_WELCOMEPAGE_TITLE "Welcome to ${PRODUCT_NAME}\r\nVersion ${PRODUCT_VERSION}"
!define MUI_WELCOMEPAGE_TEXT "${PRODUCT_NAME} automates the process of downloading, installing, and uninstalling MinGW Components.\r\n\nClick Next to continue."
!insertmacro MUI_PAGE_WELCOME


Page custom DownLoadOrInstall

!insertmacro MUI_PAGE_LICENSE "MinGW_LICENSE.rtf"

Page custom ChoosePackage

var ChooseMessage

; Components page
!define MUI_PAGE_HEADER_SUBTEXT $ChooseMessage
!define MUI_PAGE_CUSTOMFUNCTION_PRE AbortComponents
!insertmacro MUI_PAGE_COMPONENTS

; Directory page
!define MUI_PAGE_HEADER_SUBTEXT "Choose the folder in which to install MinGW."
!define MUI_DIRECTORYPAGE_TEXT_TOP "${PRODUCT_NAME} will install MinGW components in the following directory. To install in a different folder click Browse and select another folder. Click Next to continue."
!define MUI_PAGE_CUSTOMFUNCTION_PRE AbortPage
!insertmacro MUI_PAGE_DIRECTORY

; Start menu page
var ICONS_GROUP
!define MUI_STARTMENUPAGE_NODISABLE
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "MinGW"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "${PRODUCT_UNINST_ROOT_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "${PRODUCT_STARTMENU_REGVAL}"
!define MUI_PAGE_CUSTOMFUNCTION_PRE AbortPage
!insertmacro MUI_PAGE_STARTMENU Application $ICONS_GROUP

var INSTALL_ACTION
; Instfiles page
!define MUI_PAGE_HEADER_SUBTEXT $INSTALL_ACTION
!define MUI_INSTFILESPAGE_ABORTHEADER_TEXT "Installation Aborted"
!define MUI_INSTFILESPAGE_ABORTHEADER_SUBTEXT "The installation was not completed successfully."
!insertmacro MUI_PAGE_INSTFILES


var FINISH_TITLE
var FINISH_TEXT

; Finish page
!define MUI_FINISHPAGE_TITLE $FINISH_TITLE
!define MUI_FINISHPAGE_TEXT $FINISH_TEXT
!define MUI_FINISHPAGE_TEXT_LARGE $INSTALLED_TEXT
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

; Reserve files
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

; MUI end ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
Caption "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "${PRODUCT_NAME}-${PRODUCT_VERSION}.exe"

ShowInstDetails show
ShowUnInstDetails show

var Install
var Package

var runtime
var W32API
var binutils
var Core
var GPP
var G77
var ADA
var GCJ
var OBJC
var Make

InstType "Minimal"
InstType "Full"

SectionGroup /e "MinGW base tools" SecBase

  Section runtime SecRuntime
    SectionIn 1 2 3
  SectionEnd
  Section w32api SecW32API
    SectionIn 1 2
  SectionEnd
  Section binutils SecBinutils
    SectionIn 1 2
  SectionEnd
  Section "core compiler" SecCore
    SectionIn 1 2
  SectionEnd
SectionGroupEnd

Section "g++ compiler" SecGPP
    SectionIn 2
SectionEnd
Section "g77 compiler" SecG77
    SectionIn 2
SectionEnd
Section "Ada Compiler" SecADA
    SectionIn 2
SectionEnd
Section "Java Compiler" SecGCJ
    SectionIn 2
SectionEnd
Section "Objective C Compiler" SecOBJC
    SectionIn 2
SectionEnd
Section "MinGW Make" SecMake
    SectionIn 2
SectionEnd

var Updating

Section -installComponents

  SetAutoClose false

  push ${SecRuntime}
  push $runtime
  Call DownloadIfNeeded

  push ${SecW32API}
  push $W32API
  Call DownloadIfNeeded

  push ${SecBinutils}
  push $binutils
  Call DownloadIfNeeded

  push ${SecCore}
  push $Core
  Call DownloadIfNeeded

  push ${SecGPP}
  push $GPP
  Call DownloadIfNeeded

  push ${SecG77}
  push $G77
  Call DownloadIfNeeded

  push ${SecADA}
  push $ADA
  Call DownloadIfNeeded

  push ${SecGCJ}
  push $GCJ
  Call DownloadIfNeeded

  push ${SecOBJC}
  push $OBJC
  Call DownloadIfNeeded

  push ${SecMake}
  push $Make
  Call DownloadIfNeeded

  IntCmp $Install 1 +1 SkipInstall SkipInstall

  IntCmp $Updating 1 skipwrite +1
  
  CreateDirectory $INSTDIR
  File /oname=$INSTDIR\installed.ini INIfiles\installed.ini

  WriteINIStr $INSTDIR\installed.ini "settings"  "installtype" $Package

skipwrite:
  push ${SecRuntime}
  push $runtime
  push "runtime"
  Call ExtractTarball

  push ${SecW32API}
  push $W32API
  push "w32api"
  Call ExtractTarball

  push ${SecBinutils}
  push $binutils
  push "binutils"
  Call ExtractTarball

  push ${SecCore}
  push $Core
  push "core"
  Call ExtractTarball

  push ${SecGPP}
  push $GPP
  push "gpp"
  Call ExtractTarball

  push ${SecG77}
  push $G77
  push "g77"
  Call ExtractTarball

  push ${SecADA}
  push $ADA
  push "ada"
  Call ExtractTarball

  push ${SecGCJ}
  push $GCJ
  push "gcj"
  Call ExtractTarball

  push ${SecOBJC}
  push $OBJC
  push "objc"
  Call ExtractTarball

  push ${SecMake}
  push $Make
  push "make"
  Call ExtractTarball

  Strcpy $R1 "${PRODUCT_NAME}-${PRODUCT_VERSION}.exe"

  Delete "$INSTDIR\${PRODUCT_NAME}*.*"
  StrCmp $EXEDIR $INSTDIR skip_copy

  CopyFiles $EXEDIR\$R1 $INSTDIR\$R1
skip_copy:

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  SetOutPath $INSTDIR
  WriteIniStr "$INSTDIR\MinGW.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateDirectory "$SMPROGRAMS\$ICONS_GROUP"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\MinGW.lnk" "$INSTDIR\MinGW.url"
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Uninstall.lnk" "$INSTDIR\uninst.exe"
  SetOutPath $INSTDIR
  CreateShortCut "$SMPROGRAMS\$ICONS_GROUP\Update.lnk" "$INSTDIR\$R1"
  !insertmacro MUI_STARTMENU_WRITE_END
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"


SkipInstall:


SectionEnd

Section Uninstall
  !insertmacro MUI_STARTMENU_GETFOLDER "Application" $ICONS_GROUP

  RMDir /r "$SMPROGRAMS\$ICONS_GROUP"
  RMDir /r $INSTDIR

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"

  SetAutoClose true
SectionEnd

; Section descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecBase} "Base set of tools for compiling C programs"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecGPP} "C++ Compiler"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecG77} "Fortran Compiler"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecADA} "Ada Compiler"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecGCJ} "Java Compiler"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecOBJC} "Objective C Compiler"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecMake} "make for mingw"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

var mirrorINI

;-----------------------------------------------------------------------------
Function .onInit
;-----------------------------------------------------------------------------

; prevent multiple instances of installer
  System::Call "kernel32::CreateMutexA(i 0, i 0, t '$(^Name)') i .r0 ?e"
  Pop $0
  StrCmp $0 0 launch
  StrLen $0 "$(^Name)"
  IntOp $0 $0 + 1
loop:
  FindWindow $1 '#32770' '' 0 $1
  IntCmp $1 0 +4
  System::Call "user32::GetWindowText(i r1, t .r2, i r0) i."
  StrCmp $2 "$(^Name)" 0 loop
  System::Call "user32::SetForegroundWindow(i r1) i."
  System::Call "user32::ShowWindow(i r1,i 9) i."
  Abort
launch:

  StrCpy $R0 $WINDIR 1
  StrCpy $INSTDIR $R0:\MinGW


  ifFileExists $EXEDIR\mingw.ini +1 extractINI

  ; test existing ini file version
  ; if lower than build then use built in ini
  ReadINIStr $R1 "$EXEDIR\mingw.ini" "mingw" "Build"
  IntCmp ${BUILD} $R1 downloadINI downloadINI +1

extractINI:
  ; extract built in ini file
  File /oname=$EXEDIR\mingw.ini INIfiles\mingw.ini
  ReadINIStr $R1 "$EXEDIR\mingw.ini" "mingw" "Build"

downloadINI:
  ; save the current ini file in case download fails
  Rename $EXEDIR\mingw.ini $EXEDIR\mingw.ini.old
  ; Quietly download the latest mingw.ini file
  inetc::get  /SILENT "" "http://www.mingw.org/mingw.ini" "$EXEDIR\mingw.ini" /END

  Pop $R0
  StrCmp $R0 "OK" gotINI

  ; download failed so retrieve old file
  Rename $EXEDIR\mingw.ini.old $EXEDIR\mingw.ini

gotINI:
  ; Read mingw installer build info from INI file
  ReadINIStr $R0 "$EXEDIR\mingw.ini" "mingw" "Build"

  IntCmp ${BUILD} $R0 Finish newVersion +1

    ; downloaded ini older than current
    Delete $EXEDIR\mingw.ini
    Rename $EXEDIR\mingw.ini.old $EXEDIR\mingw.ini
    Goto Finish

  ; Read build info from INI file
  ReadINIStr $R0 "$EXEDIR\mingw.ini" "mingw" "Build"
  IntCmp ${BUILD} $R0 Finish newVersion Finish

newVersion:
    MessageBox MB_YESNO|MB_ICONINFORMATION|MB_DEFBUTTON1 "A newer version of the MinGW installer is available. Would you like to upgrade now?" IDYES upgradeMe IDNO Finish

upgradeMe:
    Call UpgradeMinGWUpdate
Finish:

  !insertmacro MUI_INSTALLOPTIONS_EXTRACT_AS "Dialogs\ChoosePackage.ini" "ChoosePackage.ini"

  StrCpy $Updating 0

  StrCpy $ChooseMessage "Choose the MinGW components you would like to install."

  ReadRegStr $1 ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "InstallLocation"
  StrCmp $1 "" first_install

  StrCpy $INSTDIR $1

  ifFileExists $INSTDIR\installed.ini +1 first_install

  StrCpy $Updating 1

  StrCpy $ChooseMessage "Choose the MinGW components you would like to update."

  ReadINIStr $Package $INSTDIR\installed.ini "settings"  "installtype"

  InstTypeSetText 0 ""
  InstTypeSetText 1 ""
  InstTypeSetText 2 ""

  goto installing

first_install:
  SectionSetText ${SecRuntime} ""
  SectionSetText ${SecW32API} ""
  SectionSetText ${SecBinutils} ""
  SectionSetText ${SecCore} ""

installing:
  GetTempFileName $mirrorINI $PLUGINSDIR
  File /oname=$mirrorINI "Dialogs\ShowMirror.ini"

FunctionEnd

;-----------------------------------------------------------------------------
Function .onVerifyInstDir
;-----------------------------------------------------------------------------
FunctionEnd

;-----------------------------------------------------------------------------
Function un.onUninstSuccess
;-----------------------------------------------------------------------------
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "MinGW was successfully removed from your computer."
FunctionEnd

;-----------------------------------------------------------------------------
Function un.onInit
;-----------------------------------------------------------------------------
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove MinGW and all of its components?" IDYES +2
  Abort
FunctionEnd


;-----------------------------------------------------------------------------
Function UpgradeMinGWUpdate
;-----------------------------------------------------------------------------
  ReadINIStr $R1 "$EXEDIR\mingw.ini" "mingw.ini" "Filename"

  DetailPrint "Downloading new version of MinGWUpdater..."
  inetc::get "http://prdownloads.sourceforge.net/mingw/$R1" "$EXEDIR\$R1" /END
  Pop $R0
  StrCmp $R0 "OK" success
    ; Failure
    SetDetailsView show
    DetailPrint "Download failed: $R0"
    Abort

  success:
    MessageBox MB_YESNO|MB_ICONQUESTION "Would you like to run the new version of MinGWUpdater now?" IDYES runNew
    return

  runNew:
    Exec "$EXEDIR\$R1"
    Quit
FunctionEnd

var Updates

;-----------------------------------------------------------------------------
Function AbortComponents
;-----------------------------------------------------------------------------

  IntCmp $Updating 1 +1 ShowPage ShowPage

  IntCmp $Updates 0 +1 Showpage Showpage

  StrCpy $FINISH_TEXT "${PRODUCT_NAME} found no updates to install."
  Abort

ShowPage:

FunctionEnd

;-----------------------------------------------------------------------------
Function AbortPage
;-----------------------------------------------------------------------------
  IntCmp $Updating 1 +1 TestInstall TestInstall
    Abort

TestInstall:
  IntCmp $Install 1 ShowPage +1 +1
    Abort

ShowPage:
FunctionEnd

var tarball
var component
var section
;-----------------------------------------------------------------------------
Function ExtractTarball
;-----------------------------------------------------------------------------
  pop $component  ; name
  pop $tarball  ; tarball
  pop $section  ; section

  SectionGetFlags $section $0
  IntOp $0 $0 & ${SF_SELECTED}
  IntCmp $0 ${SF_SELECTED} +1 SkipExtract

  DetailPrint "Extracting $tarball"
  untgz::extract -d "$INSTDIR" -z "$EXEDIR\$tarball"

  StrCmp $R0 "success" succeeded

  abort
  goto SkipExtract

succeeded:
  WriteINIStr $INSTDIR\installed.ini "components"  $component $tarball

SkipExtract:

FunctionEnd

var CurrentVer
var InstalledVer
var PackageSection
var PackageFlags

;-----------------------------------------------------------------------------
Function checkVersion
;-----------------------------------------------------------------------------
  pop $PackageSection
  pop $CurrentVer
  pop $InstalledVer

  SectionGetFlags $PackageSection $PackageFlags
  IntOp $PackageFlags $PackageFlags & ${SECTION_OFF}

  StrCmp $CurrentVer $InstalledVer noupdate


  Intop $Updates $Updates + 1

  ; don't select if not installed
  StrCmp $InstalledVer "" done

  IntOp $PackageFlags $PackageFlags | ${SF_SELECTED}

  Goto done

noupdate:

  SectionSetText $PackageSection ""

done:
  SectionSetFlags $PackageSection $PackageFlags

FunctionEnd

;-----------------------------------------------------------------------------
var Name

;-----------------------------------------------------------------------------
Function DownloadIfNeeded
;-----------------------------------------------------------------------------
  pop $Name  ; Filename
  pop $section  ; section

  
  SectionGetFlags $section $0
  IntOp $0 $0 & ${SF_SELECTED}
  IntCmp $0 ${SF_SELECTED} +1 SkipDL

  inetc::get /RESUME "" "http://downloads.sourceforge.net/mingw/$Name" "$EXEDIR\$Name" /END
  Pop $0
  StrCmp $0 "OK" DownLoadOK

  ; can we recover from this now?

  detailprint $0
  Abort "Could not download $Name!"

DownLoadOK:

SkipDL:
FunctionEnd

!define packages "previous|current|candidate"
;-----------------------------------------------------------------------------
Function ChoosePackage
;-----------------------------------------------------------------------------
  IntCmp $Updating 1 updating +1

  !insertmacro MUI_HEADER_TEXT "Choose Package" "Please select the MinGW package you wish to install."
  ; Display the page.
  !insertmacro MUI_INSTALLOPTIONS_DISPLAY "ChoosePackage.ini"
  ; Get the user entered values.

  StrCpy $0 1
loop:
  StrCpy $R0 "Field $0"
  !insertmacro MUI_INSTALLOPTIONS_READ $1 "ChoosePackage.ini" $R0 "State"
  IntCmp $1 1 found
  IntOp $0 $0 + 1
  goto loop

found:
  IntOp $0 $0 - 1
  ${StrTok} $Package ${packages} "|" $0 0

updating:
  ReadINIStr $R0 "$EXEDIR\mingw.ini" $Package "runtime"
  ${StrTok} $runtime $R0 "|" 0 0
  ${StrTok} $R1 $R0 "|" 1 0
  SectionSetSize ${SecRuntime} $R1

  ReadINIStr $R0 "$EXEDIR\mingw.ini" $Package "w32api"
  ${StrTok} $W32API $R0 "|" 0 0
  ${StrTok} $R1 $R0 "|" 1 0
  SectionSetSize ${SecW32API} $R1

  ReadINIStr $R0 "$EXEDIR\mingw.ini" $Package "binutils"
  ${StrTok} $binutils $R0 "|" 0 0
  ${StrTok} $R1 $R0 "|" 1 0
  SectionSetSize ${SecBinutils} $R1

  ReadINIStr $R0 "$EXEDIR\mingw.ini" $Package "core"
  ${StrTok} $Core $R0 "|" 0 0
  ${StrTok} $R1 $R0 "|" 1 0
  SectionSetSize ${SecCore} $R1

  ReadINIStr $R0 "$EXEDIR\mingw.ini" $Package "gpp"
  ${StrTok} $GPP $R0 "|" 0 0
  ${StrTok} $R1 $R0 "|" 1 0
  SectionSetSize ${SecGPP} $R1

  ReadINIStr $R0 "$EXEDIR\mingw.ini" $Package "g77"
  ${StrTok} $G77 $R0 "|" 0 0
  ${StrTok} $R1 $R0 "|" 1 0
  SectionSetSize ${SecG77} $R1

  ReadINIStr $R0 "$EXEDIR\mingw.ini" $Package "ada"
  ${StrTok} $ADA $R0 "|" 0 0
  ${StrTok} $R1 $R0 "|" 1 0
  SectionSetSize ${SecADA} $R1

  ReadINIStr $R0 "$EXEDIR\mingw.ini" $Package "java"
  ${StrTok} $GCJ $R0 "|" 0 0
  ${StrTok} $R1 $R0 "|" 1 0
  SectionSetSize ${SecGCJ} $R1

  ReadINIStr $R0 "$EXEDIR\mingw.ini" $Package "objc"
  ${StrTok} $OBJC $R0 "|" 0 0
  ${StrTok} $R1 $R0 "|" 1 0
  SectionSetSize ${SecOBJC} $R1

  ReadINIStr $R0 "$EXEDIR\mingw.ini" $Package "make"
  ${StrTok} $Make $R0 "|" 0 0
  ${StrTok} $R1 $R0 "|" 1 0
  SectionSetSize ${SecMake} $R1

  IntCmp $Updating 1 +1 installing
  
  SectionGetFlags ${SecBase} $0
  IntOp $0 $0 & ${SECTION_OFF}
  SectionSetFlags ${SecBase} $0

  StrCpy $Updates 0

  ReadINIStr $R0 "$INSTDIR\installed.ini" "components" "runtime"
  push $R0
  push $runtime
  push ${SecRuntime}
  call CheckVersion
  
  ReadINIStr $R0 "$INSTDIR\installed.ini" "components" "w32api"
  push $R0
  push $W32API
  push ${SecW32API}
  call CheckVersion

  ReadINIStr $R0 "$INSTDIR\installed.ini" "components" "binutils"
  push $R0
  push $binutils
  push ${SecBinutils}
  call CheckVersion

  ReadINIStr $R0 "$INSTDIR\installed.ini" "components" "core"
  push $R0
  push $Core
  push ${SecCore}
  call CheckVersion

  IntCmp $Updates 0 +1 baseupdates

  SectionSetText ${SecBase} ""

baseupdates:
  ReadINIStr $R0 "$INSTDIR\installed.ini" "components" "gpp"
  push $R0
  push $GPP
  push ${SecGPP}
  call CheckVersion

  ReadINIStr $R0 "$INSTDIR\installed.ini" "components" "g77"
  push $R0
  push $G77
  push ${SecG77}
  call CheckVersion

  ReadINIStr $R0 "$INSTDIR\installed.ini" "components" "ada"
  push $R0
  push $ADA
  push ${SecADA}
  call CheckVersion

  ReadINIStr $R0 "$INSTDIR\installed.ini" "components" "gcj"
  push $R0
  push $GCJ
  push ${SecGCJ}
  call CheckVersion

  ReadINIStr $R0 "$INSTDIR\installed.ini" "components" "objc"
  push $R0
  push $OBJC
  push ${SecOBJC}
  call CheckVersion

  ReadINIStr $R0 "$INSTDIR\installed.ini" "components" "make"
  push $R0
  push $Make
  push ${SecMake}
  call CheckVersion

installing:

FunctionEnd

;-----------------------------------------------------------------------------

Function DownloadOrInstall
;-----------------------------------------------------------------------------

  IntCmp $Updating 1 update +1

  FlushINI $mirrorINI

  InstallOptions::initDialog /NOUNLOAD "$mirrorINI"
  InstallOptions::show

  ReadINIStr $Install $mirrorINI "Field 2" "State"

  IntCmp $Install 1 install +1 +1

  StrCpy $INSTALL_ACTION "Please wait while ${PRODUCT_NAME} downloads the components you selected."
  StrCpy $FINISH_TITLE "Download complete."
  StrCpy $FINISH_TEXT "${PRODUCT_NAME} has finished downloading the components you selected. To install the package please run the installer again and select the download and install option."

  Goto done

install:
  StrCpy $INSTALL_ACTION "Please wait while ${PRODUCT_NAME} downloads and installs the components you selected."
  StrCpy $FINISH_TITLE "Installation complete."
  StrCpy $FINISH_TEXT "${PRODUCT_NAME} has finished installing the components you selected."

  Goto done

update:
  StrCpy $INSTALL_ACTION "Please wait while ${PRODUCT_NAME} downloads and installs the components you selected."
  StrCpy $FINISH_TITLE "Update complete."
  StrCpy $FINISH_TEXT "${PRODUCT_NAME} has finished updating the installed components."
  StrCpy $Install 1
done:

FunctionEnd
