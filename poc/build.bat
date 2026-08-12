@echo off
REM ============================================================
REM  PidTest 编译脚本（VSCode / 纯命令行，不需要 Visual Studio IDE）
REM
REM  用前改两处：
REM    1. VCVARS  —— 你的 vcvars64.bat 路径（Build Tools 或 Community 都行）
REM    2. ZWINC / ZWLIB / ZWLIBNAME —— 装完 ZW3D 后照实填，见下方说明
REM ============================================================

setlocal

REM ---- 1. MSVC 环境 ----
set "VCVARS=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
REM  只装了 Build Tools 的话用这个：
REM set "VCVARS=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

if not exist "%VCVARS%" (
    echo [ERROR] 找不到 vcvars64.bat: %VCVARS%
    echo         请修改本脚本顶部的 VCVARS 路径
    exit /b 1
)
call "%VCVARS%" >nul

REM ---- 2. ZW3D SDK 路径 ----
REM  ZW3D_DIR 由安装程序设置，指向 ZW3D 安装根目录（末尾带反斜杠）
if "%ZW3D_DIR%"=="" (
    echo [ERROR] 环境变量 ZW3D_DIR 未设置。
    echo         请手工设置为 ZW3D 安装根目录，例如：
    echo         set ZW3D_DIR=C:\Program Files\ZWSOFT\ZW3D WuKong 2027\
    exit /b 1
)

REM  【装完后核对】进 "%ZW3D_DIR%api" 看实际目录名和 .lib 文件名，照实改这三行
set "ZWINC=%ZW3D_DIR%api\inc"
set "ZWLIB=%ZW3D_DIR%api\lib"
set "ZWLIBNAME=zw3dapi.lib"

if not exist "%ZWINC%" (
    echo [ERROR] 头文件目录不存在: %ZWINC%
    echo         注意：ZW3D 2024 之前的版本没有 inc 这一层，直接用 %%ZW3D_DIR%%api
    exit /b 1
)

REM ---- 3. 编译 ----
if not exist build mkdir build
pushd build

cl /nologo /c /EHsc /MD /O2 /D "NDEBUG" /D "_WINDOWS" /D "_USRDLL" ^
   /I "%ZWINC%" ^
   ..\PidTest.cpp
if errorlevel 1 goto :fail

link /nologo /DLL /OUT:PidTest.dll ^
     /DEF:..\PidTest.def ^
     /LIBPATH:"%ZWLIB%" "%ZWLIBNAME%" ^
     PidTest.obj
if errorlevel 1 goto :fail

popd
echo.
echo [OK] 生成 build\PidTest.dll
echo.
echo 下一步：把 build\PidTest.dll 拷到  %ZW3D_DIR%apilibs\
echo         （目录不存在就自己建），然后启动 ZW3D，命令行输入 ~PidMark
exit /b 0

:fail
popd
echo.
echo [FAILED] 编译或链接失败，见上方错误。
echo   常见原因：
echo     - .lib 文件名不对        -^> 进 %%ZW3D_DIR%%api 看实际文件名
echo     - 头文件目录层级不对      -^> 2024 之前的版本没有 inc 这层
echo     - ezwEntityType 枚举名不对 -^> 见 PidTest.cpp 里的 [TODO-VERIFY]
exit /b 1
