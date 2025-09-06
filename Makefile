patchnvm: patchnvm.c
	cc -o patchnvm patchnvm.c
#	i686-w64-mingw32-gcc -static -o patchnvm.exe patchnvm.c
#	i686-w64-mingw32-strip -s patchnvm.exe

clean:
	rm -f patchnvm  patchnvm.exe
