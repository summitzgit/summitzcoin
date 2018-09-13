
rem <delete folder>
rd .\build /s /q

rem <delete files>
del Makefile
del qrc_bitcoin.cpp
del *-qt
del .\src\obj\build.h
del .\src\*coind

rem <delete files>
del *.o *.p *.a *.qm *.mk /s
