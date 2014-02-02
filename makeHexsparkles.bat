4hexchat sparkles.c hexsparkles.c
gcc -Wall -Os -DWIN32 -c hexsparkles.c
dllwrap --def HexChat.def --dllname hexsparkles.dll hexsparkles.o
pause