Name "Festalon NSF Plugin 0.3.0"
OutFile "infestalon.exe"
InstallDir $PROGRAMFILES\Winamp
InstallDirRegKey HKLM \
                 "Software\Microsoft\Windows\CurrentVersion\Uninstall\Winamp" \
                 "UninstallString"

Function .onInit
  StrCpy $INSTDIR $INSTDIR\Plugins
FunctionEnd

DirText "Install the Festalon NSF Plugin.  Kayo boing?  Note that the installer will delete or overwrite an existing installation of the Festalon plugin, if a recognized filename is detected."
Section "blop"
  SetOutPath $INSTDIR
  MessageBox MB_YESNO|MB_ICONQUESTION \
        "Would you like to install the alternate, AMD K6-optimized version of Festalon?  This is highly recommended if you have such a processor whose clock speed is less than 500MHz." IDYES GoAMD
  Delete $INSTDIR\in_festalon_amd.dll
  File in_festalon.dll
  goto endish
  GoAMD:
  Delete $INSTDIR\in_festalon.dll
  File in_festalon_amd.dll
  endish:
SectionEnd ; end the section
; eof
