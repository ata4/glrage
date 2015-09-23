@echo off
set version_file=version.h
set version_var=GLRAGE_VERSION
for /f "tokens=*" %%i in ('git describe --tags') do set version_str=%%i
echo #define %version_var% "%version_str%" > %version_file%
