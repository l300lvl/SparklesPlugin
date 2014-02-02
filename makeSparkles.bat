gcc -Wall -Os -DWIN32 -c sparkles.c
dllwrap --def XChat.def --dllname sparkles.dll sparkles.o
pause
