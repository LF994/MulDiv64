@echo OFF
:: compile to console executable, use 64 bits target command prompt
setlocal

call  :SubFind ML64.exe
if not exist "%__RUN%" (
  echo ML64.exe not found on path
  echo It may indicate that you NOT running 64 bits command prompt for VisualStudio
  echo Check CMD link properties, it should have VsDevCmd.bat -arch=amd64 
  goto :eof
)

:: compile asm, -c means no linking
ml64 -c muldiv64.asm

set CLARGS=/Os /W4 /EHsc /D_CRT_SECURE_NO_WARNINGS
set CLARGS=%CLARGS% tsmuldiv.cpp muldiv64.obj
set CLARGS=%CLARGS% /MD /D_AFXDLL /link /MAP

cl %CLARGS%
goto :eof


:SubFind
set __RUN=%~$PATH:1
echo %1 at: %__RUN%
goto :eof
:: eof
