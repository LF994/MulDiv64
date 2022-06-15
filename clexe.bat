@echo OFF
:: compile to console executable, 32 or 62 bits:
:: call it from proper command prompt in VisualStudio
setlocal
set CLARGS=/Os /W4 /EHsc /D_CRT_SECURE_NO_WARNINGS
::set CLARGS=%CLARGS% /I..\h
set CLARGS=%CLARGS% %*
set CLARGS=%CLARGS% /MD /D_AFXDLL /link /MAP
::: 32 bit, /MD: exe, link with C-runtime from nondebug DLL
:: (MSVCRT.DLL fro VC6):

cl %CLARGS% >%tmp%\err
type %tmp%\err

:: manifest is mandatory only for VC2008, not for VC6 or VC2010 and above
:: mt.exe -manifest %~n1.exe.manifest -outputresource:%~n1.exe;1
:: del  %~n1.exe.manifest 1>nul >2:nul

:: eof
