;NSIS Modern User Interface
;Basic Example Script
;Written by Joost Verburg

;--------------------------------
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------
;General

  ;Name and file
  Name "monitord v2.0svn"
  OutFile "monitord-setup.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\monitord"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\monitord" ""

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages
  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "license.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "German"

;--------------------------------
;Installer Sections

Section "Grundprogramm" SecMonitord

  SetOutPath "$INSTDIR"
  
  ; Dateiliste
  File ..\monitord\monitord.exe
    File _start.bat
    File _stop.bat
    File _install-service.bat
    File _uninstall-service.bat
                
  
  ;Konfigfile nur erstellen, wenn es noch keins gibt. Sonst als
  ;monitord.xml.new speichern
  IfFileExists "$INSTDIR\monitord.xml" +1 +3
  File /oname=monitord.xml.new ..\sample-config\monitord.xml.win32
  goto +2
  File /oname=monitord.xml ..\sample-config\monitord.xml.win32

  ;Store installation folder
  WriteRegStr HKCU "Software\monitord" "" $INSTDIR
  
  ;Startmenu
  
  CreateDirectory "$SMPROGRAMS\monitord"
  CreateShortCut "$SMPROGRAMS\monitord\monitord.lnk" "$INSTDIR\monitord.exe"
  CreateShortCut "$SMPROGRAMS\monitord\Konfiguration bearbeiten.lnk" "notepad" "$INSTDIR\monitord.xml"
  CreateShortCut "$SMPROGRAMS\monitord\uninstall.lnk" "$INSTDIR\uninstall.exe"
  

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  
   ; In Systemsteuerung/Software eintragen mit "Nur Entfernen"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\monitord" \
				"DisplayName" "monitord"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\monitord" \
                 "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\monitord" \
                 "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\monitord" \
                 "NoRepair" 1

SectionEnd

Section "Beispielkonfiguration" SecSamples
  SetOutPath "$INSTDIR\sample-config"
  File "..\sample-config\*"
SectionEnd

Section "Plugins" SecPlugins
  SetOutPath "$INSTDIR\plugins"
  File ..\monitord\plugins\.libs\libmplugin_audiorecorder-0.dll
  File ..\monitord\plugins\.libs\libmplugin_mysql-0.dll
SectionEnd

Section "Dll" SecDLLs

  SetOutPath "$INSTDIR"
  File ..\monitord\libmysql.dll
  File ..\monitord\libmp3lame-0.dll

SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecMonitord ${LANG_GERMAN} "Das monitord Hauptprogramm"
  LangString DESC_SecPlugins ${LANG_GERMAN} "Plugins zum monitord, z.B. Audiorekorder, MySQL Plugin, ..."
  LangString DESC_SecDLLs ${LANG_GERMAN} "von Plugins benötigte DLLs (mysql.dll, mp3lame, ...)"
  LangString DESC_SecSamples ${LANG_GERMAN} "Beispielkonfigurationen"

  ;Assign language strings to sections
	  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMonitord} $(DESC_SecMonitord)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecPlugins} $(DESC_SecPlugins)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDLLs} $(DESC_SecDLLs)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecSamples} $(DESC_SecSamples)
	!insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ;ADD YOUR OWN FILES HERE...

  Delete "$INSTDIR\Uninstall.exe"
  
  RMDir /r "$INSTDIR"
  Delete "$SMPROGRAMS\monitord.lnk"
  Delete "$SMPROGRAMS\uninstall.lnk"
  RMDir /r "$SMPROGRAMS\monitord"

  DeleteRegKey /ifempty HKCU "Software\monitord"
  ; Aus Systemsteuerung/Software wieder löschen
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\monitord"

SectionEnd