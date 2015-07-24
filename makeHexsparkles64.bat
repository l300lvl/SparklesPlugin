4hexchat sparkles.c hexsparkles.c
gcc -c -o hexsparkles64.o hexsparkles.c -std=gnu99 -m64
gcc -o hexsparkles64.dll -s -shared hexsparkles64.o -Wl,--subsystem,windows -m64
pause