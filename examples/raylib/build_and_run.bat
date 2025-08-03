@ECHO OFF

cl /nologo gdi32.lib msvcrt.lib raylib.lib winmm.lib user32.lib shell32.lib inferno.c main.c  -I.\include /link /libpath:.\lib /NODEFAULTLIB:libcmt

