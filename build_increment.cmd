@echo off
setlocal

set VERSION_FILE=build.num
set HEADER_FILE=src\version.h

:: Читаем текущий номер сборки, если файл существует
if exist %VERSION_FILE% (
    set /p BUILD_NUM=<%VERSION_FILE%
    set /a BUILD_NUM+=1
) else (
    :: Если файла нет, начинаем с 1
    set BUILD_NUM=1
)

:: Сохраняем новый номер обратно в файл
echo %BUILD_NUM% > %VERSION_FILE%

:: Генерируем заголовочный файл для C/C++
echo #ifndef VERSION_H > %HEADER_FILE%
echo #define VERSION_H >> %HEADER_FILE%
echo #define BUILD_NUMBER %BUILD_NUM% >> %HEADER_FILE%
echo #define VERSION_STRING "1.0.%BUILD_NUM%" >> %HEADER_FILE%
echo #endif >> %HEADER_FILE%

echo Build number incremented to: %BUILD_NUM%