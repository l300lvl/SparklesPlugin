4hexchat sparkles.c hexsparkles.c
gcc -c -o hexsparkles.o hexsparkles.c -std=gnu99
gcc -o hexsparkles.dll -s -shared hexsparkles.o -Wl,--subsystem,windows
pause