IF NOT EXIST "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\amd64" GOTO Preview
set PATH=C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\amd64;%PATH%
Echo VS Professional Detected
GOTO Start

:Preview
IF NOT EXIST "C:\Program Files\Microsoft Visual Studio\2022\Preview\MSBuild\Current\Bin\amd64" GOTO Community
set PATH=C:\Program Files\Microsoft Visual Studio\2022\Preview\MSBuild\Current\Bin\amd64;%PATH%
Echo VS Preview Detected
GOTO Start

:Community
set PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\amd64;%PATH%
Echo VS Community Detected
:Start

cd ..
cd ..
cd ..

cd wxWidgets
msbuild.exe -m:10 .\build\msw\wx_vc16.sln -p:Configuration="Debug" -p:Platform="x86"
if %ERRORLEVEL% NEQ 0 goto error


msbuild.exe -m:10 .\build\msw\wx_vc16.sln -p:Configuration="Release" -p:Platform="x86"
if %ERRORLEVEL% NEQ 0 goto error


goto exit

:error

@echo Error compiling wxWidgets x86
pause
exit 1

:exit