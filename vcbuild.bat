@echo off

set arch=%1
set mode=%2
set workspaceFolder=%3
set targetName=%4

set compilerFlags=
set linkFlags=
set buildDir=build\%arch%\%mode%
set files=

set preDefine=/D "_MBCS" /D "_CRT_SECURE_NO_DEPRECATE"

set commonCompilerFlags=/permissive- /GS /W3 /Zc:wchar_t /Gm- /Zc:inline /sdl /Fd"%buildDir%\vc141.pdb" /fp:precise %preDefine% /errorReport:prompt /WX- /Zc:forScope /Gd /FC /Fa"%buildDir%/" /EHsc /nologo /Fo"%buildDir%/" /Fp"%buildDir%\%targetName%.pch" /diagnostics:classic

set linkLib="kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib"

set commonLinkFlags=/OUT:"%workspaceFolder%\%buildDir%\%targetName%.exe" /MANIFEST /NXCOMPAT /PDB:"%workspaceFolder%\%buildDir%\%targetName%.pdb" /DYNAMICBASE

set commonLinkFlags2=/MANIFESTUAC:"level='asInvoker' uiAccess='false'" /ManifestFile:"%buildDir%\%targetName%.exe.intermediate.manifest" /ERRORREPORT:PROMPT /NOLOGO /TLBID:1

if /i "%arch%"=="x64" (
    call "%VS_ROOT%\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
    if /i "%mode%"=="Debug" goto debug-x64
    if /i "%mode%"=="Release" goto release-x64
)

if /i "%arch%"=="x86" (
    call "%VS_ROOT%\2017\Community\VC\Auxiliary\Build\vcvars32.bat"
    if /i "%mode%"=="Debug" goto debug-x86
    if /i "%mode%"=="Release" goto release-x86
)

:debug-x64
set compilerFlags=%commonCompilerFlags% /JMC /ZI /Od /RTC1 /MDd
set linkFlags=%commonLinkFlags% %linkLib% %commonLinkFlags2% /DEBUG:FASTLINK /INCREMENTAL /MACHINE:X64
goto var-done

:release-x64
set compilerFlags=%commonCompilerFlags% /GL /Gy /Zi /O2 /Oi /MT 
set linkFlags=%commonLinkFlags% /LTCG:incremental %linkLib% %commonLinkFlags2% /DEBUG:FULL /OPT:REF /OPT:ICF /MACHINE:X64
goto var-done

:debug-x86
set compilerFlags=%commonCompilerFlags% /JMC /analyze- /ZI /Od /RTC1 /Oy- /MDd
set linkFlags=%commonLinkFlags% %linkLib% %commonLinkFlags2% /DEBUG:FASTLINK /INCREMENTAL /MACHINE:X86
goto var-done

:release-x86
set compilerFlags=%commonCompilerFlags% /GL /analyze- /Gy /Zi /O2 /Oy- /Oi /MT
set linkFlags=%commonLinkFlags% /LTCG:incremental %linkLib% %commonLinkFlags2% /DEBUG:FULL /OPT:REF /SAFESEH /OPT:ICF /MACHINE:X86
goto var-done

:var-done
goto next-arg

:next-arg
if /i "%5"=="" (
    goto args-done
) else (
    set files=%5 %files%
    goto arg-ok
)

:arg-ok
shift
goto next-arg

:args-done
REM if exist %buildDir% rd /S /Q %buildDir%
if not exist %buildDir% mkdir %buildDir%
echo %buildDir%
echo cl %compilerFlags% %files% /link %linkFlags%
echo=
cl %compilerFlags% %files% /link %linkFlags%
