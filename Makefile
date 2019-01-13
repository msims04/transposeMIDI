gcc: main.c
	g++ -O2 -static -o transposeMIDI.exe main.c -lwinmm
	strip --strip-debug transposeMIDI.exe
	upx -9 transposeMIDI.exe > /dev/null

win: main.c
	cl /MT /O2 /Fe:transposeMIDI.exe main.c winmm.lib
	upx -9 transposeMIDI.exe > NUL
