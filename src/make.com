$ gcc /warn/nocase  /object=mgetasc.obj mgetasc.c
$ gcc /warn/nocase  /object=sci.obj sci.c
$ gcc /warn/nocase  /object=serial.obj serial.c
$ gcc /warn/nocase  /object=mtools.obj mtools.c
make: `mgetasc.exe' is up to date
