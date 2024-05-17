@echo off
cd /d "%~dp0"
set CURRENTDIR=%cd%
cd ../../../../utils/board_gen
for /f "tokens=1,2 delims==" %%i in (prebuild.cfg) do (
if "%%i"=="board_gen_mode" set board_gen_mode=%%j
)
if %board_gen_mode% NEQ 1 (
    if "%2%" == "scpu" (
        python board_gen.py -w 1
    )
    @exit /b 0 
)

copy ..\FLM\KL520-W25Q256JV-scpu-auto.FLM %1%
copy ..\FLM\KL520-W25Q256JV-scpu-auto-slave.FLM %1%
cd %CURRENTDIR%

@echo off
set have_python=0
for /f "tokens=* USEBACKQ" %%g in ('python --version') do (set have_python=1)

rem echo have_python=%have_python%
@if %have_python% EQU 1 (
    goto with_python_label
) else (
    echo '--- [Error] Please download and add the python environment.'
    echo 'Remember to select "Add python 3.x to PATH" and "install for all users" checkbox during installation ---'
    start iexplore https://www.python.org/downloads/
    goto end
)

:try_and_install_python_module_func
@python -c "import sys, pkgutil; sys.exit(0 if pkgutil.find_loader(%~1) else 1)"
if %errorlevel% EQU 1 (
    echo "--- [before_build] install python module %~2 ---"
    python -m pip install %~2
) else (
    echo "--- [before_build]python module %~2 has been installed---"
)
echo "---[before build]done for python module %~2 installation----"	
@exit /b 0

:with_python_label
rem python -m pip uninstall crcmod
call :try_and_install_python_module_func 'numpy' numpy
call :try_and_install_python_module_func 'cv2' opencv-python
call :try_and_install_python_module_func 'serial' pyserial
call :try_and_install_python_module_func 'crcmod' crcmod
call :try_and_install_python_module_func 'xmodem' xmodem
call :try_and_install_python_module_func 'pyprind' pyprind
call :try_and_install_python_module_func 'usb' pyusb
goto board_gen_label


:board_gen_label
cd ../..
set system_config= 
for %%i in (.) do set system_config=%%~nxi
rem echo system_config=%system_config%

cd ../../utils/board_gen
python board_gen.py -s %system_config% -c %2%

:end
echo "--- [before_build] done ---"