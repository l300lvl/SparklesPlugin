gcc -c -o hexsparkles64.o sparkles.c -D HEXCHAT -std=gnu99 -m64
gcc -o hexsparkles64.dll -s -shared hexsparkles64.o -Wl,--subsystem,windows -m64
pause