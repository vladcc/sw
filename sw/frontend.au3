#pragma compile(FileDescription, Make your script an .exe)
#pragma compile(ProductName, Script Wrap)
#pragma compile(ProductVersion, 1.0.0.0)
#pragma compile(FileVersion, 1.0.0.0)
#pragma compile(LegalCopyright, Vladimir Dinev)

#NoTrayIcon

Opt("GUIOnEventMode", 1)
Opt("GUICloseOnESC", 1)
Opt("MustDeclareVars", 1)

Const $TITLE = "Script Wrap v1.0"
Const $SW = "sw"
Const $FD_FILEMUSTEXIST = 1, $GUI_EVENT_CLOSE = -3, $MB_OK = 0, $STDOUT_CHILD = 0x2

Main()

Func Main()
   #cs
   GUI and wait loop below
   #ce
   Local $hMainGUI
   Global $txtScript, $txtRun, $txtIcon
   Global $btnOK, $btnCancel, $btnSB, $btnRB, $btnIB

   $hMainGUI = GUICreate($TITLE, 390, 190)
   GUISetOnEvent($GUI_EVENT_CLOSE, "Close", $hMainGUI)

   GUICtrlCreateLabel("Script file:", 5, 5)
   $txtScript =  GUICtrlCreateInput("", 5, 25, 320, -1)
   $btnSB = GUICtrlCreateButton("&Browse", 330, 23, 50, 23)
   GUICtrlSetOnEvent($btnSB, "OpenDialog")

   GUICtrlCreateLabel("Interpreter/run line:", 5, 55)
   $txtRun =  GUICtrlCreateInput("", 5, 75, 320, -1)
   $btnRB = GUICtrlCreateButton("B&rowse", 330, 73, 50, 23)
   GUICtrlSetOnEvent($btnRB, "OpenDialog")

   GUICtrlCreateLabel("Icon:", 5, 105)
   $txtIcon =  GUICtrlCreateInput("", 5, 125, 320, -1)
   $btnIB = GUICtrlCreateButton("Bro&wse", 330, 123, 50, 23)
   GUICtrlSetOnEvent($btnIB, "OpenDialog")

   $btnOK = GUICtrlCreateButton("&OK", 5, 155, 50, -1)
   GUICtrlSetOnEvent($btnOK, "ScriptWrap")

   $btnCancel = GUICtrlCreateButton("&Cancel", 60, 155, 50, -1)
   GUICtrlSetOnEvent($btnCancel, "Close")

   GUISetState(@SW_SHOW, $hMainGUI)

   FileChangeDir(@ScriptDir)

   While 1
	  Sleep(100)
   WEnd

EndFunc

Func OpenDialog()
   #cs
   invokes the file open dialog window
   for file selection
   #ce
   Local $hFileOpenDialog, $hOut

   $hFileOpenDialog = FileOpenDialog("Select a file", @ScriptDir & "\", "All (*.*)", $FD_FILEMUSTEXIST)

   Switch @GUI_CtrlId
   Case $btnSB
	  $hOut = $txtScript
   Case $btnRB
	  $hOut = $txtRun
   Case $btnIB
	  $hOut = $txtIcon
   EndSwitch

   If $hFileOpenDialog <> "" Then GUICtrlSetData($hOut, $hFileOpenDialog)

   FileChangeDir(@ScriptDir); change back the working dir to the script dir

EndFunc

Func ScriptWrap()
   #cs
   calls sw.exe with the proper arguments
   #ce
   Local $sRunLine, $iPID
   Local $sScript, $sRun, $sIcon, $sOut

   $sRunLine = ""
   $sScript = ""
   $sRun = ""
   $sIcon = ""
   $sOut = ""

   $sScript = GetFileNameOnly(GUICtrlRead($txtScript))
   $sRun = GUICtrlRead($txtRun)
   $sIcon = GUICtrlRead($txtIcon)

   If $sScript <> "" Then $sRunLine &= $SW & " -f " & '"' & $sScript & '"'
   If $sRun <> "" Then $sRunLine &= " -e " & '"' & $sRun & '"'
   If $sIcon <> "" Then $sRunLine &= " -ic " & '"' & $sIcon & '"'

   If StringInStr($sRunLine, $SW) <> 0 Then
	  $iPID = Run($sRunLine, @ScriptDir, @SW_HIDE, $STDOUT_CHILD)
	  ProcessWaitClose($iPID)
	  $sOut = StdoutRead($iPID) ; get sw.exe console output
   Else
	  $sOut = "No script file."
   EndIf

   MsgBox($MB_OK, $TITLE, $sOut)

EndFunc

Func GetFileNameOnly($sFullName)
   #cs
   extracts the file name of the script from
   it's full path
   #ce
   Local $sFileName, $sSplitChar, $iFileNamePos

   If StringMid($sFullName, 2, 1) <> ":" Then Return $sFullName ; if not full path

   $sSplitChar = StringMid($sFullName, 3, 1) ; get separator
   $iFileNamePos = StringInStr($sFullName, $sSplitChar, 0, -1)

   ; get only file name
   $sFileName = StringRight($sFullName, StringLen($sFullName) - $iFileNamePos)

   Return $sFileName

EndFunc

Func Close()
   #cs
   goes home
   #ce
   Exit
EndFunc