''
''  Handle information related to Compile process 
''
Type COMPILE_TYPE
   FileToCompile   As WString * MAX_PATH
   OutputFilename  As WString * MAX_PATH   ' resulting exe name 
   CompilerPath    As WString * MAX_PATH 
   CurrentFolder   As WString * MAX_PATH
   OutputFolder    As WString * MAX_PATH
   CompileMode     As WString * MAX_PATH 
   CompileFlags    As WString * MAX_PATH
   BatchFile       As WString * MAX_PATH
   LogFile         As WString * MAX_PATH  
   RunAfterCompile As BOOLEAN
   StartTime       As Double
   EndTime         As Double
End Type
Dim Shared gCompile As COMPILE_TYPE


''
''  Main module that handles the entire compile process
''
'------------------------------------------------------------------------------------------------------------------------
Function code_Compile( ByVal wID As Long ) As BOOLEAN

   Dim ShExecInfo As SHELLEXECUTEINFOW  
    
   Dim As Long Parenthesis_Start, Parenthesis_End, Error_Start       
   Dim As Long f, i, NumLines, NextLine, idx, r
   Dim As Long NumWarnings, NumErrors, IsError, IsWarning
   Dim As Long nFirstErrorLine = -1
   
   Dim wszTemp      As WString * MAX_PATH 
   Dim wszFileExe   As WString * MAX_PATH
   Dim wszParam     As WString * MAX_PATH  
   
   Dim As String execmd, sDQ, sLogSt, st, st_ucase, sTemp
   Dim As String sOutputMsg, sFileExe, sParam
   Dim As String st1, st2, st3        
   Dim As HWnd hLV
         
   ' Can only continue to compile if the primary source code file
   ' is not dirty and needs to be saved.
   If gConfig.CompileAutosave Then
      If OnCommand_FileSaveAll(HWND_FRMMAIN) Then Return True  ' user cancelled save
   End If
   

   ' Perform some pre-compile checks to see if we should continue.
   ProcessFromCurdrive(@gConfig.FBWINCompiler32)
   ProcessFromCurdrive(@gConfig.FBWINCompiler64)
   Select Case Ucase(gConfig.DefaultCompiler)
      Case "FBC 32BIT":   gCompile.CompilerPath = gConfig.FBWINcompiler32
      Case "FBC 64BIT":   gCompile.CompilerPath = gConfig.FBWINcompiler64
   End Select
   
   
   ' Check to see if the compiler exists
   If Dir(gCompile.CompilerPath) = "" Then 
      MessageBoxW( HWND_FRMMAIN, WStr("Invalid defined compiler path."), WStr("Error"), _
                        MB_OK Or MB_ICONINFORMATION Or MB_DEFBUTTON1 Or MB_APPLMODAL )
      Function = False: Exit Function
   End If
   
   
   ' Set some compile flags depending on the type of compile requested.
   Select Case wID
      Case IDM_BUILDEXECUTE:  gCompile.RunAfterCompile = True
      Case IDM_COMPILE:       gCompile.RunAfterCompile = False
   End Select
   
   Dim pDoc As clsDocument Ptr = gTTabCtl.GetActiveDocumentPtr()
   If pDoc = 0 Then Return 0
   
   gCompile.FileToCompile = *pDoc->DiskFilename
   gCompile.CurrentFolder = Curdir                        
   gCompile.CompileFlags  = gConfig.CompilerSwitches   
   
   Select Case gConfig.DefaultCompileMode   
      Case "GUI":     gCompile.CompileMode = " -s gui "
      Case "CONSOLE": gCompile.CompileMode = " -s console "
   End Select   
  
   FF_Pathname(@WStr("PATH"), pDoc->DiskFilename, @gCompile.OutputFolder)

   ' Change to the code output directory
   ChDir gCompile.OutputFolder
   
   
   ' Create a temporary batch that will contain the compile commands. We shell
   ' to and run this batch file in order to do the compile.
   ' Replace any embedded spaces with underscores
   FF_Pathname(@WStr("NAME"), @gCompile.FileToCompile, @wszTemp)
   sTemp = "_" & Str(wszTemp)
   sTemp = FF_Replace(sTemp, " ", "_" )
   gCompile.BatchFile = sTemp & "_compile.bat" 
   gCompile.LogFile   = sTemp & "_compile.log"
   sDQ = Chr(34)   ' double quotes
   
   execmd = sDQ & gCompile.CompilerPath & sDQ & " " & _
            sDQ & gCompile.FileToCompile & sDQ & _
            " -v " & gCompile.CompileFlags & gCompile.CompileMode & _
            " > " & gCompile.LogFile & " 2>&1"   ' ensure stderr is redirected to log file as well

   f = Freefile
   Open gCompile.BatchFile For Output as #f
   Print #f, execmd
   Close #f

         
   ' Shell to the command interpreter and run the batch file
   gCompile.StartTime = Timer

   wszFileExe = Environ("COMSPEC")
   wszParam   = " /C " & gCompile.BatchFile
   With ShExecInfo
      .cbSize       = Len(SHELLEXECUTEINFOW)
      .fMask        = SEE_MASK_NOCLOSEPROCESS
      .HWnd         = 0
      .lpVerb       = Null
      .lpFile       = @wszFileExe
      .lpParameters = @wszParam   
      .lpDirectory  = 0
      .nShow        = SW_HIDE
      .hInstApp     = 0 
   End With
   ShellExecuteEx(@ShExecInfo)
   WaitForSingleObject(ShExecInfo.hProcess,INFINITE)

   gCompile.EndTime = Timer


   ' Get the log file contents into a string in order to parse it for errors, etc
   If Dir(gCompile.LogFile) > "" Then
      f = Freefile
      Open gCompile.LogFile For Binary as #f 
      sLogSt = Space(LOF(f))
      Get #f, , sLogSt
      Close #f
   End If   
   
   ' Delete the temporary batch file
   Kill gCompile.BatchFile

   
   ' Clear the Compiler Log listview
   If IsWindow(HWND_FRMCOMPILERESULTS) = 0 Then
      ' Create the frmCompileResults form (initially not visible). This is a non-modal
      ' popup form that displays compile results.
      frmCompileResults_Show(HWND_FRMMAIN, 0)
   End If
   hLV = GetDlgItem(HWND_FRMCOMPILERESULTS, IDC_FRMCOMPILERESULTS_LISTVIEW)
   ListView_DeleteAllItems( hLV )
   
   ' Parse the log string.
   ' The compile was done using the "verbose" parameter. Save the first
   ' two lines because they contain the compiler version and copyright info.
   NumLines = FF_ParseCount( sLogSt, vbCrLf)

   For NextLine = 1 To NumLines                   
   
      st = Trim(FF_Parse(sLogSt, vbCrlf, NextLine))                 
      st_ucase = Ucase(st)
      
      If NextLine <= 2 Then  ' Save the FB version and copyright information
         sOutputMsg = sOutputMsg & st & vbCrLf 
         Continue For
      End If        
      
      ' Check to see if an error occurred in compiling a resource script.
      ' If there was a bad source name passed to the compiler (for 
      ' example, missing .rc file), then the output at the end of the
      ' log file is like the following:
      '
      ' Error!
      ' Could Not Open source file (p.RC)
      ' OBJ file Not made
      ' compiling resource FAILED: Error Code 1
      
      ' Check to see if linking failed
      If (Left(st_ucase, 6) = "ERROR!") Then
         FF_ListView_InsertItem( hLV, NumWarnings, 0, WStr("0") )
         FF_ListView_InsertItem( hLV, NumWarnings, 1, WStr("") ) 
         FF_ListView_InsertItem( hLV, NumWarnings, 2, WStr("compiling resource FAILED: Error Code 1") ) 
         NumErrors = NumErrors + 1
      End If
      
      If Instr(st_ucase, "LINKING FAILED:") Then 
         FF_ListView_InsertItem( hLV, NumWarnings, 0, WStr("0") ) 
         FF_ListView_InsertItem( hLV, NumWarnings, 1, WStr("") ) 
         FF_ListView_InsertItem( hLV, NumWarnings, 2, WStr(st) ) 
         NumErrors = NumErrors + 1
      End If


      ' Check for the "linking: " line because that line identifies the actual output filename being created
      If Left(st_ucase, 9) = "LINKING: " Then 
         ' -o "WinFBE.exe" 
         i = Instr(st, " -o ")
         If i Then
            sTemp = Mid(st, i+4)
            i = Instr(sTemp, sDQ & " ")
            If i Then gCompile.OutputFilename = FF_Remove(Left(sTemp, i), sDQ)
         End If      
      End If
      
      ' Check for any compiler warnings
      IsWarning = Instr(st_ucase, ") WARNING ")
      If IsWarning Then Error_Start = IsWarning
      IsError   = Instr(st_ucase, ") ERROR ") 
      If IsError Then Error_Start = IsError
      
      If (Error_Start > 0) Then 
         'sample warning message
         'c:\freebasic\test.bas(1394) warning 1(1): Passing scalar as pointer, at parameter 2 (hwndOldFocus) of ONSETFOCUS()
         'sample error message
         'c:\freebasic\test.bas(17) error 41: Variable not declared, kjljjada in 'kjljjada Error'
         Parenthesis_Start = Instr(1, st, "(")
         Parenthesis_End   = Instr(Parenthesis_Start, st, ")")
         If (Parenthesis_Start < Parenthesis_End) And _
            (Parenthesis_End <= Error_Start)       And _
            (Error_Start > 0) Then
   
            st1 = FF_Parse(st, "(", 1)   ' filename
            st2 = Mid( st, Parenthesis_Start + 1, Parenthesis_End - Parenthesis_Start - 1)  ' line# 
            st3 = Mid(st, Error_Start+1)  ' error message

            FF_ListView_InsertItem( hLV, NumWarnings+NumErrors, 0, WStr(st2) )  ' line#
            FF_ListView_InsertItem( hLV, NumWarnings+NumErrors, 1, WStr(st1) )  ' filename
            FF_ListView_InsertItem( hLV, NumWarnings+NumErrors, 2, WStr(st3) )  ' error message
            
            If IsWarning Then 
               NumWarnings = NumWarnings + 1
            ElseIf IsError Then
               If nFirstErrorLine = -1 Then
                  nFirstErrorLine = NumWarnings + NumErrors
               End If
               NumErrors = NumErrors + 1
            End If            
         End If
      End If

   Next


   '
   ' (1) No errors - successful compile. Show success messagebox. A successful compile
   '     will even occur if a number of warnings are present. If warnings then show Compiler Log.
   ' (2) Errors - unsuccessful compile. Show frmCompileRsults form and open the Compiler Log
   '     and position it to the error location.
   '
   If (NumWarnings = 0) AndAlso (NumErrors = 0) Then
      Kill gCompile.LogFile
   Else                        
      ' There were warnings and/or errors so display the compile results form
      wszTemp = "Compile Results :: [Warnings: " & NumWarnings & "  Errors: " & NumErrors & "]"
      SetWindowTextW( HWND_FRMCOMPILERESULTS, @wszTemp )
      ShowWindow HWND_FRMCOMPILERESULTS, SW_SHOW
      MessageBeep(MB_ICONWARNING)
   End If
   
   If (NumErrors = 0) Then           
      DestroyWindow(HWND_FRMCOMPILERESULTS)
      st = sOutputMsg & vbCrLf & _
            "Primary Source: "     & gCompile.FileToCompile & vbCrLf & _
            "Target Compilation: " & gCompile.OutputFilename & vbCrLf & _
            "Compile Time: "       & Format(gCompile.EndTime-gCompile.StartTime, "###0.0") & " seconds." & vbCrLf & _
            "File Size: "          & FileLen(gCompile.OutputFilename) & " bytes."
      MessageBoxW HWND_FRMMAIN, WStr(st), WStr("Compiler Results"), MB_ICONINFORMATION Or MB_OK Or MB_TASKMODAL
   Else
      gCompile.RunAfterCompile = False 
      
      ' Position the Compiler Log to the first error
      nFirstErrorLine = Iif(nFirstErrorLine > -1, nFirstErrorLine, 0) 
      ListView_SelectItem( hLV, nFirstErrorLine )
      SetDocumentErrorPosition(hLV)
   End If
  
   
   If gCompile.RunAfterCompile Then 
      Exec gCompile.OutputFilename, gConfig.CommandLine
   End If

   ' Ensure that we switch back to the Project's directory
   ChDir gCompile.CurrentFolder
   
End Function





