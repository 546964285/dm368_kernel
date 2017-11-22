@echo off
pushd ..\..\
setlocal

:process_arg
if "%1"=="" goto end_process_arg
set name=%1
set value=

:process_arg_value
if NOT "%value%"=="" set value=%value% %2
if "%value%"=="" set value=%2
shift
if "%2"=="!" goto set_arg
if "%2"=="" goto set_arg
goto process_arg_value

:set_arg
set %name%=%value%
shift
shift
goto process_arg
:end_process_arg

echo. > temp_postBuildStep_AM1808.bat

echo HexAIS.exe -ini ..\..\GNU\AISUtils\OMAP-L138-uart.ini -o %PROJECT_ROOT%\ubl_AM1808_SDMMC.ais %PROJECT_ROOT%\..\SDMMC\UBL_AM1808_SDMMC.out >> temp_postBuildStep_AM1808.bat

call temp_postBuildStep_AM1808.bat
del temp_postBuildStep_AM1808.bat

endlocal
popd
