@echo off
chcp 65001 >nul
:: 飞秋企业微信通知插件 - Release 编译脚本
:: Visual Studio Build Tools 2022 (安装在 D:\VS2022)

echo ========================================
echo  WeComNotify 插件编译脚本 (Release)
echo ========================================
echo.

:: Step 1: 设置 MSVC 环境变量 (x86 = Win32)
echo [1/3] 初始化编译器环境...
call "D:\VS2022\VC\Auxiliary\Build\vcvarsall.bat" x86
if errorlevel 1 (
    echo 错误: 无法初始化编译器环境！
    pause
    exit /b 1
)

:: Step 2: 切换到项目目录
cd /d "%~dp0"
echo.
echo [2/3] 编译项目...

:: Step 3: 执行 MSBuild 编译 Release Win32
msbuild WeComNotify.vcxproj /p:Configuration=Release /p:Platform=Win32 /m /nologo

if errorlevel 1 (
    echo.
    echo ========================================
    echo  编译失败！请检查错误信息。
    echo ========================================
    pause
    exit /b 1
)

echo.
echo ========================================
echo  编译成功！
echo ========================================
echo.
echo 输出文件: Release\WeComNotify.dll
echo.
echo 部署步骤:
echo   1. 将 Release\WeComNotify.dll 复制到飞秋 Plugins\ 目录
echo   2. 修改 WeComNotifyModule.cpp 中的 Webhook URL 并重新编译
echo   3. 重启飞秋
echo.
pause
