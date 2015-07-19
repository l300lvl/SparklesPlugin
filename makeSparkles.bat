gcc -c -o sparkles.o sparkles.c -std=gnu99
gcc -o sparkles.dll -s -shared sparkles.o -Wl,--subsystem,windows
pause