@echo off

if not exist ..\build mkdir ..\build
pushd ..\build

set LibsDirectory=..\libs
set IncludeDirectories= ^
    /I%LibsDirectory%\SFML-2.4.0-windows-vc14-64-bit\include


set LibraryDirectory=%LibsDirectory%\SFML-2.4.0-windows-vc14-64-bit\lib

set Libraries= ^
    sfml-graphics-s.lib ^
    sfml-window-s.lib ^
    sfml-system-s.lib ^
    opengl32.lib ^
    winmm.lib ^
    gdi32.lib ^
    freetype.lib ^
    glew.lib ^
    jpeg.lib ^
    user32.lib ^
    Advapi32.lib

set Defines=/DSFML_STATIC

set FilesToCompile=..\code\main.cpp
set Defines=%Defines% /DUNITY_BUILD

set FilesToCompile= ^
    ..\src\main.cpp

set CompilerOptions=%Defines% /W3 /WX /FC /Zi /EHsc /MD /MP /wd4503 /nologo /FeKraad.exe %IncludeDirectories%

set LinkerOptions=/link /LIBPATH:%LibraryDirectory%

cl %CompilerOptions% %FilesToCompile% %Libraries% %LinkerOptions%

del *.obj
del *.ilk
del *.idb
del vc140.pdb
rmdir Kraad.tlog /s /q

popd
