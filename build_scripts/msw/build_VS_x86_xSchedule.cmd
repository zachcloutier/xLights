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

cd xSchedule
msbuild.exe -m:10 xSchedule.sln -p:Configuration="Release" -p:Platform="x86"
if %ERRORLEVEL% NEQ 0 goto error
cd ..

cd xSchedule
cd xSMSDaemon
msbuild.exe -m:10 xSMSDaemon.sln -p:Configuration="Release" -p:Platform="x86"
if %ERRORLEVEL% NEQ 0 goto error
cd ..
cd ..

cd build_scripts
cd msw

goto exit

:error

@echo Error compiling x86
pause
exit 1

:exit