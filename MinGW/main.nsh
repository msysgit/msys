; -- main.nsh --
; Created: JohnE, 2008-03-15


; DISCLAIMER:
; The author(s) of this file's contents release it into the public domain,
; without express or implied warranty of any kind. You may use, modify, and
; redistribute this file freely.


;;;;;
;; All the dirty work happens here. For the basic settings you're wanting to
;; change, look in "setup.nsi".
;;;;;


; Plugin settings
!addplugindir "plugins"


!ifdef INNER_COMPONENTS
SetCompressor zlib
SetCompress off
!else
SetCompressor /SOLID lzma
SetCompress auto
!endif

; Basic attributes
Name "${SHORTNAME}"
XPStyle On
BrandingText "${SHORTNAME} Setup ${SETUP_VER}"
CompletedText "$comp_text"


; Includes
!include LogicLib.nsh
!include MUI2.nsh
!include FileFunc.nsh
!insertmacro GetRoot
!insertmacro Locate
!insertmacro GetParameters
!insertmacro un.GetParameters
!include StrFunc.nsh
${StrRep}

!define MULTIUSER_EXECUTIONLEVEL Highest
!include MultiUser.nsh


;;; Globals
Var setup_type
Var stype_shortdesc
Var inst_dir
Var dlupdates
Var dl_mirror
Var comp_text
Var inst_ftext
Var inst_fsubtext
Var working_manifest
Var num_prev_insts
Var man_msg
Var uninst
Var have_allusers_mode


;;; UI settings
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "header1.bmp"
!define MUI_HEADERIMAGE_RIGHT
!define MUI_WELCOMEFINISHPAGE_BITMAP "wfpage1.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "wfpage1.bmp"
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_UNFINISHPAGE_NOAUTOCLOSE


;;; Installer Pages
Page custom WizardAction_Create WizardAction_Leave

!define MUI_DIRECTORYPAGE_VARIABLE $inst_dir
!define MUI_PAGE_CUSTOMFUNCTION_SHOW InstDir_Show
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE InstDir_Leave
!insertmacro MUI_PAGE_DIRECTORY

Page custom MirrorSelect_Create MirrorSelect_Leave

; !insertmacro MUI_PAGE_COMPONENTS
Page custom Components_Create

!define MUI_INSTFILESPAGE_FINISHHEADER_TEXT "$inst_ftext"
!define MUI_INSTFILESPAGE_FINISHHEADER_SUBTEXT "$inst_fsubtext"
!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_TEXT "Read '${READMEFILE}' (recommended)"
!define MUI_FINISHPAGE_RUN_FUNCTION "LaunchReadme"
!define MUI_PAGE_CUSTOMFUNCTION_SHOW "Finish_Show"
!insertmacro MUI_PAGE_FINISH


;;; Uninstaller pages
!define MUI_INSTFILESPAGE_FINISHHEADER_TEXT "$inst_ftext"
!define MUI_INSTFILESPAGE_FINISHHEADER_SUBTEXT "$inst_fsubtext"
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_UNPAGE_FINISH


;;; Languages
!insertmacro MUI_LANGUAGE "English"


;;; Reserved files
!insertmacro MUI_RESERVEFILE_LANGDLL ;Language selection dialog


;;; Sections
Section "Install Components" SEC_INSTALL_COMPONENTS
	Var /GLOBAL ar_dl_index

	RealProgress::SetProgress /NOUNLOAD 0

	SetOutPath "$inst_dir"
	CreateDirectory "$inst_dir\downloaded"
	tdminstall::AddManMiscFile /NOUNLOAD "downloaded/"

	; Unpack any inner archives
!ifdef INNER_COMPONENTS
!system 'ctemplate "${INNER_COMPONENTS}" arcout.template.txt > arcout.nsh' \
 = 0
!include arcout.nsh
!endif

	; Download archives
	StrCpy $ar_dl_index 0
	GetFunctionAddress $0 "ArchiveCallbackForDownload"
	tdminstall::EnumArchives /NOUNLOAD $0
	Pop $0
	${If} $0 != "OK"
		DetailPrint "$0"
		Goto onerror
	${EndIf}

	; Install archives
	GetFunctionAddress $0 "ItemBeforeActionCallback"
	tdminstall::RemoveAndAdd /NOUNLOAD \
	 "$inst_dir\downloaded|$EXEDIR\downloaded|$PLUGINSDIR" $0
	Pop $0
	${If} $0 != "OK"
		DetailPrint "$0"
		Goto onerror
	${EndIf}

	; MinGW environment vars batch file
	File "mingwvars.bat"
	tdminstall::AddManMiscFile /NOUNLOAD "mingwvars.bat"

	; This executable
	CopyFiles /SILENT "$EXEPATH" "$inst_dir"
	tdminstall::AddManMiscFile /NOUNLOAD "$EXEFILE"

	; Start menu shortcuts
	tdminstall::GetStartMenuSelected /NOUNLOAD
	Pop $0
	${If} $0 == 1
		CreateDirectory "$SMPROGRAMS\${STARTMENU_ENTRY}"
		CreateShortCut \
		 "$SMPROGRAMS\${STARTMENU_ENTRY}\MinGW Command Prompt.lnk" \
		 '%comspec%' '/k ""$inst_dir\mingwvars.bat""' \
		 "" "" "" "" "Open ${SHORTNAME} Command Prompt"
		CreateShortCut \
		 "$SMPROGRAMS\${STARTMENU_ENTRY}\Modify or Remove MinGW.lnk" \
		 "$inst_dir\$EXEFILE"
	${EndIf}

	; Add/Remove Programs entry
	WriteRegStr SHELL_CONTEXT \
	 "Software\Microsoft\Windows\CurrentVersion\Uninstall\${UNINSTKEY}" \
	 "DisplayName" "TDM/MinGW"
	WriteRegStr SHELL_CONTEXT \
	 "Software\Microsoft\Windows\CurrentVersion\Uninstall\${UNINSTKEY}" \
	 "UninstallString" '"$inst_dir\$EXEFILE" /tdmu'
	WriteRegDWORD SHELL_CONTEXT \
	 "Software\Microsoft\Windows\CurrentVersion\Uninstall\${UNINSTKEY}" \
	 "NoRepair" 1
	WriteRegStr SHELL_CONTEXT \
	 "Software\Microsoft\Windows\CurrentVersion\Uninstall\${UNINSTKEY}" \
	 "ModifyPath" '"$inst_dir\$EXEFILE"'
	WriteRegStr SHELL_CONTEXT \
	 "Software\Microsoft\Windows\CurrentVersion\Uninstall\${UNINSTKEY}" \
	 "InstallLocation" "$inst_dir"
	WriteRegStr SHELL_CONTEXT \
	 "Software\Microsoft\Windows\CurrentVersion\Uninstall\${UNINSTKEY}" \
	 "Publisher" "${PUBLISHER}"
	WriteRegStr SHELL_CONTEXT \
	 "Software\Microsoft\Windows\CurrentVersion\Uninstall\${UNINSTKEY}" \
	 "HelpLink" "${INFOURL}"
	WriteRegStr SHELL_CONTEXT \
	 "Software\Microsoft\Windows\CurrentVersion\Uninstall\${UNINSTKEY}" \
	 "URLUpdateInfo" "${UPDATEURL}"
	WriteRegStr SHELL_CONTEXT \
	 "Software\Microsoft\Windows\CurrentVersion\Uninstall\${UNINSTKEY}" \
	 "URLInfoAbout" "${INFOURL}"
	WriteRegStr SHELL_CONTEXT \
	 "Software\Microsoft\Windows\CurrentVersion\Uninstall\${UNINSTKEY}" \
	 "DisplayVersion" "${SETUP_VER}"

	; Write installation manifest
	DetailPrint "Writing installation manifest"
	tdminstall::WriteInstManifest /NOUNLOAD

	; Add installation to list
	DetailPrint "Updating list of ${SHORTNAME} installations"
	tdminstall::WriteInstList /NOUNLOAD "$APPDATA\${APPDATA_SUBFOLDER}" \
	 "installations.txt"

	Goto instend

onerror:
	StrCpy $inst_ftext "Installation Failed"
	StrCpy $inst_fsubtext "Setup was not completed successfully."
	StrCpy $comp_text "One or more errors occurred"

instend:
	RealProgress::SetProgress /NOUNLOAD 100
SectionEnd


Section "un.Uninstall"
	RealProgress::SetProgress /NOUNLOAD 0

	StrCpy $comp_text "Completed successfully"
	StrCpy $inst_ftext "Uninstall Complete"
	StrCpy $inst_fsubtext "Uninstallation was completed successfully."

	${If} "$uninst" == ""
		DetailPrint "Error: Uninstaller called without location"
		Goto onerrorun
	${EndIf}

	UserInfo::GetName
	${IfNot} ${Errors}
		Pop $0
		tdminstall::CheckIfUserInstall /NOUNLOAD "$uninst" "$0"
		Pop $0
		${If} "$0" == 1
			Call un.MultiUser.InstallMode.CurrentUser
		${Else}
			Call un.MultiUser.InstallMode.AllUsers
		${EndIf}
		GetFunctionAddress $0 "un.ItemBeforeActionCallback"
		tdminstall::RemoveInst /NOUNLOAD "$uninst" \
		 "$APPDATA\${APPDATA_SUBFOLDER}\installations.txt" $0
		Pop $0
		${If} $0 != "OK"
			DetailPrint "$0"
			Goto onerrorun
		${EndIf}
		${If} ${FileExists} \
		 "$SMPROGRAMS\${STARTMENU_ENTRY}\MinGW Command Prompt.lnk"
			ShellLink::GetShortCutArgs \
			 "$SMPROGRAMS\${STARTMENU_ENTRY}\MinGW Command Prompt.lnk"
			Pop $0
			tdminstall::StringInString /NOUNLOAD "$uninst" "$0"
			Pop $0
			${If} "$0" == 1
				Delete \
				 "$SMPROGRAMS\${STARTMENU_ENTRY}\MinGW Command Prompt.lnk"
				Delete \
				 "$SMPROGRAMS\${STARTMENU_ENTRY}\Modify or Remove MinGW.lnk"
				RMDir "$SMPROGRAMS\${STARTMENU_ENTRY}"
				DetailPrint "Removed '${STARTMENU_ENTRY}' Start Menu entry"
			${EndIf}
		${EndIf}
		ReadRegStr $0 SHELL_CONTEXT \
		 "Software\Microsoft\Windows\CurrentVersion\Uninstall\${UNINSTKEY}" \
		 "UninstallString"
		tdminstall::StringInString /NOUNLOAD "$uninst" "$0"
		Pop $0
		${If} "$0" == 1
			DeleteRegKey SHELL_CONTEXT \
			 "Software\Microsoft\Windows\CurrentVersion\Uninstall\${UNINSTKEY}"
		${EndIf}
		SetDetailsPrint none
		SetOutPath "$TEMP"
		SetDetailsPrint both
		RMDir "$uninst"
		${IfNot} ${FileExists} "$uninst"
			DetailPrint "Removed '$uninst'"
		${Else}
			DetailPrint "(Some files left in '$uninst')"
		${EndIf}
	${EndIf}

	Goto uninstend

onerrorun:
	StrCpy $inst_ftext "Uninstall Failed"
	StrCpy $inst_fsubtext "Uninstallation was not completed successfully."
	StrCpy $comp_text "One or more errors occurred"

uninstend:
	RealProgress::SetProgress /NOUNLOAD 100
SectionEnd


;;; Special functions
Function .onInit
	InitPluginsDir
	!insertmacro MULTIUSER_INIT
	${GetParameters} $0
	tdminstall::GetUninst /NOUNLOAD "$0"
	Pop $uninst
	${If} "$uninst" != "true"
		StrCpy $uninst ""
	${EndIf}
	StrCpy $setup_type "undefined"
	StrCpy $inst_dir ""
	StrCpy $dlupdates "yes"
	StrCpy $dl_mirror ""
	StrCpy $comp_text "Completed successfully"
	StrCpy $inst_ftext "Installation Complete"
	StrCpy $inst_fsubtext "Setup was completed successfully."
	StrCpy $working_manifest "$PLUGINSDIR\${INNER_MANIFEST}"
	StrCpy $num_prev_insts 0
	StrCpy $man_msg ""
	tdminstall::BeginInstFindBanner /NOUNLOAD
	tdminstall::UpdateFoundInsts /NOUNLOAD \
	 "$APPDATA\${APPDATA_SUBFOLDER}\installations.txt"
	Pop $num_prev_insts
	${If} "$MultiUser.InstallMode" == "AllUsers"
		StrCpy $have_allusers_mode 1
		Call MultiUser.InstallMode.CurrentUser
		tdminstall::UpdateFoundInsts /NOUNLOAD \
		 "$APPDATA\${APPDATA_SUBFOLDER}\installations.txt"
		Pop $num_prev_insts
	${Else}
		StrCpy $have_allusers_mode 0
	${EndIf}
	tdminstall::EndInstFindBanner /NOUNLOAD
!ifdef INNER_COMPONENTS
!system 'ctemplate "${INNER_COMPONENTS}" arcreg.template.txt > arcreg.nsh' \
 = 0
!include arcreg.nsh
!endif
FunctionEnd

Function un.onInit
	InitPluginsDir
	!insertmacro MULTIUSER_UNINIT
	${un.GetParameters} $0
	tdminstall::GetUninst /NOUNLOAD "$0"
	Pop $uninst
	StrCpy $comp_text "Completed successfully"
	StrCpy $inst_ftext "Uninstall Complete"
	StrCpy $inst_fsubtext "Uninstallation was completed successfully."
FunctionEnd

Function .onGUIEnd
	tdminstall::Unload
	RealProgress::Unload
FunctionEnd

Function un.onGUIEnd
	tdminstall::Unload
	RealProgress::Unload
FunctionEnd

Function .onVerifyInstDir
	${If} "$setup_type" != "create"
		${IfNot} ${FileExists} "$inst_dir\installed_man.txt"
			${If} $man_msg != ""
				SendMessage $man_msg ${WM_SETTEXT} 0 \
				 'STR:(No installation manifest present in this directory)'
			${EndIf}
			Abort
		${Else}
			${If} $man_msg != ""
				SendMessage $man_msg ${WM_SETTEXT} 0 'STR:'
			${EndIf}
		${EndIf}
	${EndIf}
FunctionEnd


;;; Utility functions
Function ArchiveCallbackForDownload
	Var /GLOBAL apc_path
	Var /GLOBAL apc_file
	Pop $apc_path
	Pop $apc_file
	Push $0

	; Check if already available
	${If} ${FileExists} "$EXEDIR\downloaded\$apc_file"
		DetailPrint "Using local archive '$EXEDIR\downloaded\$apc_file'"
		Push "OK"
		Return
	${ElseIf} ${FileExists} "$inst_dir\downloaded\$apc_file"
		DetailPrint "Using local archive '$inst_dir\downloaded\$apc_file'"
		Push "OK"
		Return
	${ElseIf} ${FileExists} "$PLUGINSDIR\$apc_file"
		DetailPrint "Using inner archive '$apc_file'"
		Push "OK"
		Return
	${EndIf}

dlfile:
	DetailPrint "Downloading '$apc_file'"
	tdminstall::GetDownloadProgress /NOUNLOAD $ar_dl_index ; Pushes...
	RealProgress::SetProgress /NOUNLOAD ; ...then pops.
	${StrRep} $0 "$apc_path$apc_file" "+" "%2B"
	Push "$dl_mirror$0"
	Push "$apc_file"
	Call DownloadArchive
	Pop $0
	${If} $0 != "OK"
		MessageBox MB_RETRYCANCEL|MB_ICONEXCLAMATION|MB_DEFBUTTON1 \
		 "Couldn't download \
		 '$dl_mirror$apc_path$apc_file':$\r$\n$0$\r$\n$\r$\nWould you like to \
		 try again?" \
		 IDRETRY dlfile
		DetailPrint "$0"
		StrCpy $0 "Couldn't download '$dl_mirror$apc_path$apc_file'"
	${EndIf}
	IntOp $ar_dl_index $ar_dl_index + 1
	tdminstall::AddManMiscFile /NOUNLOAD "downloaded/$apc_file"

	Exch $0
FunctionEnd

Function ItemBeforeActionCallback
	Var /GLOBAL ibec_path
	Var /GLOBAL ibec_isdir
	Var /GLOBAL ibec_isdel
	Pop $ibec_path
	Pop $ibec_isdir
	Pop $ibec_isdel

	${If} "$ibec_isdel" == 1
		DetailPrint "Removing '$ibec_path'"
	${Else}
		${If} "$ibec_isdir" == 1
			DetailPrint "Creating '$ibec_path'"
		${Else}
			DetailPrint "Extracting '$ibec_path'"
		${EndIf}
	${EndIf}

	tdminstall::GetInstallProgress /NOUNLOAD ; Pushes...
	RealProgress::SetProgress /NOUNLOAD ; ...then pops.
FunctionEnd

Function un.ItemBeforeActionCallback
	Var /GLOBAL unibec_path
	Var /GLOBAL unibec_isdir
	Var /GLOBAL unibec_isdel
	Pop $unibec_path
	Pop $unibec_isdir
	Pop $unibec_isdel

	${If} "$unibec_isdel" == 1
		DetailPrint "Removing '$unibec_path'"
	${Else}
		${If} "$unibec_isdir" == 1
			DetailPrint "Creating '$unibec_path'"
		${Else}
			DetailPrint "Extracting '$unibec_path'"
		${EndIf}
	${EndIf}

	tdminstall::GetInstallProgress /NOUNLOAD ; Pushes...
	RealProgress::SetProgress /NOUNLOAD ; ...then pops.
FunctionEnd

Function DownloadArchive
	Var /GLOBAL da_file
	Var /GLOBAL da_url
	Pop $da_file
	Pop $da_url
	inetc::get \
	 /CAPTION "Download '$da_file'" \
	 /QUESTION "Are you sure you want to cancel the download?" \
	 "$da_url" "$inst_dir\downloaded\$da_file" \
	 /END
	; Leave result on stack for caller to use
FunctionEnd


;;; Page functions
Function WizardAction_Create
	${If} "$uninst" == "true"
		StrCpy $setup_type "remove"
		StrCpy $stype_shortdesc "Uninstall"
		tdminstall::GetFirstPrevInst /NOUNLOAD
		Pop $inst_dir
		${If} "$inst_dir" != ""
			tdminstall::SetInstLocation /NOUNLOAD "$inst_dir"
		${EndIf}
		Abort
	${EndIf}

	!insertmacro MUI_HEADER_TEXT "Wizard Action" "Choose which action you want \
	  the setup wizard to perform."

	nsDialogs::Create /NOUNLOAD 1018
	Pop $2

	IntOp $0 $(^FontSize) + 2
	CreateFont $0 "$(^Font)" $0 "700"

	; Action buttons
	${NSD_CreateButton} 0u 0u 150u 17u "Create"
	Pop $1
	SendMessage $1 ${WM_SETFONT} $0 1
	${NSD_OnClick} $1 WizardAction_OnClickCreate
	${NSD_CreateLabel} 15u 19u 150u 20u \
	 ": Create a new ${SHORTNAME} installation"
	Pop $1
	${NSD_CreateButton} 0u 41u 150u 17u "Manage"
	Pop $1
	SendMessage $1 ${WM_SETFONT} $0 1
	${NSD_OnClick} $1 WizardAction_OnClickManage
	${NSD_CreateLabel} 15u 60u 150u 20u \
	 ": Manage an existing ${SHORTNAME} installation"
	Pop $1
	${NSD_CreateButton} 0u 82u 150u 17u "Remove"
	Pop $1
	SendMessage $1 ${WM_SETFONT} $0 1
	${NSD_OnClick} $1 WizardAction_OnClickRemove
	${NSD_CreateLabel} 15u 101u 150u 20u \
	 ": Remove a ${SHORTNAME} installation"
	Pop $1

	; Previous installations
	${If} "$num_prev_insts" > 0
		tdminstall::ReplaceDirProc /NOUNLOAD $2
		${NSD_CreateGroupBox} 170u 0u 129u 126u "Previous Installations"
		Pop $1
		GetFunctionAddress $1 "WizardAction_OnInstSel"
		tdminstall::CreateInstDirPrevList /NOUNLOAD $2 1 $1
	${EndIf}

	; Download updates checkbox
!ifdef INNER_COMPONENTS
	Var /GLOBAL WizardAction.DLUpdates
	${NSD_CreateCheckBox} 0u 128u 290u 12u \
	 "Check for updated files on the ${SHORTNAME} server"
	Pop $WizardAction.DLUpdates
	${If} $dlupdates == "yes"
		SendMessage $WizardAction.DLUpdates ${BM_SETCHECK} ${BST_CHECKED} 0
	${Else}
		SendMessage $WizardAction.DLUpdates ${BM_SETCHECK} ${BST_UNCHECKED} 0
	${EndIf}
!endif

	; Hide Next button
	GetDlgItem $0 $HWNDPARENT 1
	EnableWindow $0 0
	ShowWindow $0 ${SW_HIDE}

	nsDialogs::Show
FunctionEnd

Function WizardAction_Leave
	; Set $inst_dir
	${GetRoot} "$PROGRAMFILES" $0
	StrCpy $0 "$0${DEF_INST_DIR}"
	${If} $setup_type == "create"
		tdminstall::IsPrevInst /NOUNLOAD "$0"
		Pop $1
		${If} "$1" == 1
			StrCpy $inst_dir ""
		${Else}
			StrCpy $inst_dir "$0"
		${EndIf}
	${ElseIf} "$inst_dir" == ""
		tdminstall::GetFirstPrevInst /NOUNLOAD
		Pop $inst_dir
	${EndIf}
	${If} "$inst_dir" != ""
		tdminstall::SetInstLocation /NOUNLOAD "$inst_dir"
	${EndIf}

	; Check Download updates checkbox state
!ifdef INNER_COMPONENTS
	${NSD_GetState} $WizardAction.DLUpdates $0
	${If} $0 == ${BST_CHECKED}
		StrCpy $dlupdates "yes"
	${Else}
		StrCpy $dlupdates "no"
	${EndIf}
!endif

	; Download updated manifest
	${If} "$setup_type" != "remove"
		${If} $dlupdates == "yes"
			${IfNot} ${FileExists} "$PLUGINSDIR\${LOCAL_NET_MANIFEST}"
				Push /END
				Push "$PLUGINSDIR\${LOCAL_NET_MANIFEST}"
				Push "${NET_MANIFEST_URL}"
				Push "Are you sure you want to cancel the download?"
				Push /QUESTION
				Push "Downloading '${NET_MANIFEST_URL}'"
				Push /BANNER
				Push \
				 "An error occurred while downloading '${NET_MANIFEST_URL}'. Would \
				 you like to try again?"
				Push /RESUME
				Push "Download updated manifest"
				Push /CAPTION
				inetc::get
				Pop $0
				${If} "$0" != "OK"
					MessageBox MB_OK|MB_ICONEXCLAMATION \
					 "An updated manifest file could not be \
					 downloaded:$\r$\n$0$\r$\n(${NET_MANIFEST_URL})"
					Abort
				${EndIf}
			${EndIf}
			StrCpy $working_manifest "$PLUGINSDIR\${LOCAL_NET_MANIFEST}"
			tdminstall::SetManifest /NOUNLOAD "$working_manifest" $HWNDPARENT
			Pop $0
			${If} "$0" == "update"
				Abort
			${ElseIf} "$0" != "OK"
				MessageBox MB_OK|MB_ICONEXCLAMATION \
				 "The downloaded manifest file could not be loaded."
				Delete "$working_manifest"
				Abort
			${EndIf}
!ifdef INNER_COMPONENTS
		${Else}
			${IfNot} ${FileExists} "$PLUGINSDIR\${INNER_COMPONENTS}"
				File "/oname=$PLUGINSDIR\${INNER_COMPONENTS}" \
				 "${INNER_COMPONENTS}"
			${EndIf}
			StrCpy $working_manifest "$PLUGINSDIR\${INNER_COMPONENTS}"
			tdminstall::SetManifest /NOUNLOAD "$working_manifest" $HWNDPARENT
			Pop $0
			${If} "$0" != "OK"
				MessageBox MB_OK|MB_ICONEXCLAMATION \
				 "The inner manifest file could not be loaded."
				Abort
			${EndIf}
!endif
		${EndIf}
	${EndIf}
FunctionEnd

Function WizardAction_OnClickCreate
	Pop $0
	StrCpy $setup_type "create"
	StrCpy $stype_shortdesc "New Installation"
	Call WizardActionNext
FunctionEnd

Function WizardAction_OnClickManage
	Pop $0
	StrCpy $setup_type "manage"
	StrCpy $stype_shortdesc "Existing Installation"
	Call WizardActionNext
FunctionEnd

Function WizardAction_OnClickRemove
	Pop $0
	StrCpy $setup_type "remove"
	StrCpy $stype_shortdesc "Uninstall"
	Call WizardActionNext
FunctionEnd

Function WizardActionNext
	; Show Next button
	GetDlgItem $0 $HWNDPARENT 1
	EnableWindow $0 1
	ShowWindow $0 ${SW_SHOW}
	; "Click" Next button
	SendMessage $0 ${BM_CLICK} 0 0
FunctionEnd

Function WizardAction_OnInstSel
	Pop $inst_dir
	Push 0
	Call WizardAction_OnClickManage
FunctionEnd


Function InstDir_Show
	!insertmacro MUI_HEADER_TEXT "$stype_shortdesc: Installation Directory" \
	  "Choose the installation directory to use."
	tdminstall::ReplaceDirProc /NOUNLOAD $mui.DirectoryPage
	ShowWindow $mui.DirectoryPage.SpaceRequired ${SW_HIDE}
	SendMessage $mui.DirectoryPage.DirectoryBox ${WM_SETTEXT} 0 'STR:Installation Directory'
	${If} $setup_type == "manage"
		SendMessage $mui.DirectoryPage.Text ${WM_SETTEXT} 0 \
		 "STR:Setup will operate on the installation of ${SHORTNAME} in the \
		  following folder. To use a different installation, click Browse and \
		  select its location. $_CLICK"
	${ElseIf} $setup_type == "remove"
		SendMessage $mui.DirectoryPage.Text ${WM_SETTEXT} 0 \
		 "STR:Setup will remove the installation of ${SHORTNAME} in the \
		  following folder. To remove a different installation, click Browse \
		  and select its location. Click Uninstall to begin removal."
		ShowWindow $mui.DirectoryPage.SpaceAvailable ${SW_HIDE}
		tdminstall::CreateInstDirUninstNote /NOUNLOAD $mui.DirectoryPage
		Pop $0
		GetDlgItem $0 $HWNDPARENT 1
		SendMessage $0 ${WM_SETTEXT} 0 'STR:Uninstall'
	${EndIf}
	${If} "$setup_type" == "manage"
	${OrIf} "$setup_type" == "remove"
		tdminstall::CreateInstDirManMsg /NOUNLOAD $mui.DirectoryPage
		Pop $man_msg
		${If} "$num_prev_insts" > 0
			GetFunctionAddress $0 "InstDir_OnInstSel"
			tdminstall::CreateInstDirPrevList /NOUNLOAD $mui.DirectoryPage 2 $0
		${EndIf}
	${EndIf}
FunctionEnd

Function InstDir_Leave
	Var /GLOBAL idl_empty

	StrLen $0 "$inst_dir"
	IntOp $0 $0 - 1
	StrCpy $1 "$inst_dir" 1 $0
	${If} "$1" == "\"
	${OrIf} "$1" == "/"
		StrCpy $inst_dir "$inst_dir" $0
	${EndIf}

	${If} "$setup_type" == "remove"
		GetFunctionAddress $0 "WriteUninstCallback"
		tdminstall::RunUninstall /NOUNLOAD "$inst_dir" "$TEMP" "$EXEPATH" \
		 "$EXEFILE" $0
		Pop $0
		${If} "$0" != "OK"
			MessageBox MB_OK|MB_ICONEXCLAMATION "$0"
		${EndIf}
		Quit
	${EndIf}

	StrCpy $0 1
	UserInfo::GetName
	${IfNot} ${Errors}
		Pop $0
		tdminstall::CheckIfUserInstall /NOUNLOAD "$inst_dir" "$0"
		Pop $0
	${EndIf}
	${If} "$0" == 1
		Call MultiUser.InstallMode.CurrentUser
	${Else}
		Call MultiUser.InstallMode.AllUsers
	${EndIf}

	${If} "$setup_type" == "create"
		StrCpy $idl_empty "true"
		${Locate} "$inst_dir" "/L=FD /G=0" "InstDir_NotEmpty"
		${If} $idl_empty != "true"
			MessageBox MB_YESNO|MB_ICONEXCLAMATION|MB_DEFBUTTON2 \
			 "The directory '$inst_dir' is not empty!$\r$\nAre you sure you \
			  want to install here?" IDYES noabort
			Abort
noabort:
		${EndIf}
		tdminstall::SetPrevInstMan /NOUNLOAD ""
		Pop $0
	${Else}
		tdminstall::SetPrevInstMan /NOUNLOAD "$inst_dir\installed_man.txt"
		Pop $0
		${If} "$0" != 1
			MessageBox MB_OK|MB_ICONEXCLAMATION \
			 "The installation manifest file for '$inst_dir' could not be \
			  loaded."
			Abort
		${EndIf}
	${EndIf}
	tdminstall::SetInstLocation /NOUNLOAD "$inst_dir"
FunctionEnd

Function InstDir_NotEmpty
	StrCpy $idl_empty "false"
	Push "StopLocate"
FunctionEnd

Function InstDir_OnInstSel
	Exch $0

	; SendMessage $mui.DirectoryPage.Directory ${WM_SETTEXT} 0 'STR:$0'

	; "Click" Next button
	GetDlgItem $0 $HWNDPARENT 1
	SendMessage $0 ${BM_CLICK} 0 0

	Pop $0
FunctionEnd

Function WriteUninstCallback
	Exch $0
	WriteUninstaller "$0"
	Pop $0
FunctionEnd


Function MirrorSelect_Create
	${If} $dlupdates != "yes"
		Abort
	${EndIf}

	!insertmacro MUI_HEADER_TEXT "$stype_shortdesc: Download Mirror" \
	 "Choose the location of a download server."

	nsDialogs::Create /NOUNLOAD 1018
	Pop $0

	${NSD_CreateLabel} 0 5u 100% 36u \
	 "Please select a download mirror from the following list. For a quicker \
	  download, choose a mirror that is geographically close to you."
	Pop $0

	${NSD_CreateGroupBox} 10u 40u 280u 90u "Select a Mirror"
	Pop $0

	Var /GLOBAL mirror_lb
	${NSD_CreateListBox} 18u 51u 264u 72u ""
	Pop $mirror_lb
	tdminstall::PopulateMirrorList /NOUNLOAD $mirror_lb

	nsDialogs::Show
FunctionEnd

Function MirrorSelect_Leave
	tdminstall::GetSelMirror /NOUNLOAD $mirror_lb
	Pop $dl_mirror
FunctionEnd


Function Components_Create
	!insertmacro MUI_HEADER_TEXT "$stype_shortdesc: Choose Components" \
	 "Choose which features of ${SHORTNAME} you want installed."

	nsDialogs::Create /NOUNLOAD 1018
	Pop $0

	${NSD_CreateLabel} 0u 0u 300u 18u \
	 "Check the components you want installed and uncheck the components you \
	 don't want installed. $_CLICK"
	Pop $1

	${NSD_CreateLabel} 0u 23u 90u 9u \
	 "Select the type of install:"
	Pop $1

	${NSD_CreateDropList} 92u 21u 206u 75u ""
	Pop $1
	${If} "$setup_type" == "manage"
		tdminstall::PopulateInstallTypeList /NOUNLOAD $1 1
	${Else}
		tdminstall::PopulateInstallTypeList /NOUNLOAD $1 0
	${EndIf}

	${NSD_CreateLabel} 0u 38u 300u 9u \
	 "Or, select the optional components you wish to have installed:"
	Pop $1

	tdminstall::CreateComponentsTree /NOUNLOAD $0

	${NSD_CreateGroupBox} 210u 45u 89u 94u "Description"
	Pop $1

	${NSD_CreateLabel} 215u 55u 79u 79u ""
	Pop $1
	tdminstall::SetDescLabel /NOUNLOAD $1

	${NSD_CreateLabel} 0u 128u 200u 12u ""
	Pop $1
	tdminstall::SetSpaceReqLabel /NOUNLOAD $1

	nsDialogs::Show
FunctionEnd


Function Finish_Show
	${IfNot} ${FileExists} "$inst_dir\${READMEFILE}"
		SendMessage $mui.FinishPage.Run ${BM_SETCHECK} 0 ${BST_UNCHECKED}
		ShowWindow $mui.FinishPage.Run ${SW_HIDE}
	${EndIf}
FunctionEnd

Function LaunchReadme
	Push "$inst_dir\${READMEFILE}"
	Call ConvertUnixNewLines
	ExecShell "open" "$inst_dir\${READMEFILE}"
FunctionEnd

Function ConvertUnixNewLines
Exch $R0 ;file #1 path
Push $R1 ;file #1 handle
Push $R2 ;file #2 path
Push $R3 ;file #2 handle
Push $R4 ;data
Push $R5
 
 FileOpen $R1 $R0 r
 GetTempFileName $R2
 FileOpen $R3 $R2 w
 
 loopRead:
  ClearErrors
  FileRead $R1 $R4 
  IfErrors doneRead
 
   StrCpy $R5 $R4 1 -1
   StrCmp $R5 $\n 0 +4
   StrCpy $R5 $R4 1 -2
   StrCmp $R5 $\r +3
   StrCpy $R4 $R4 -1
   StrCpy $R4 "$R4$\r$\n"
 
  FileWrite $R3 $R4
 
 Goto loopRead
 doneRead:
 
 FileClose $R3
 FileClose $R1
 
 SetDetailsPrint none
 Delete $R0
 Rename $R2 $R0
 SetDetailsPrint both
 
Pop $R5
Pop $R4
Pop $R3
Pop $R2
Pop $R1
Pop $R0
FunctionEnd

; end main.nsh
