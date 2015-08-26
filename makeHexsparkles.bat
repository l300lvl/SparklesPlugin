gcc -c -o hexsparkles.o sparkles.c -D HEXCHAT -std=gnu99
gcc -o hexsparkles.dll -s -shared hexsparkles.o -Wl,--subsystem,windows
pause