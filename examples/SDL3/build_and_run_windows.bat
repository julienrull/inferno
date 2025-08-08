@ECHO OFF

cl /nologo /Fe.\app.exe gdi32.lib msvcrt.lib raylib.lib winmm.lib user32.lib shell32.lib main.c  -I.\include /link /libpath:.\lib /NODEFAULTLIB:libcmt

