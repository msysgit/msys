
CFLAGS=-Os -s

first_rule: all

all allbin: ../lib/libW11.dll

alldoc tags:

../lib/libW11.dll: xlib.o ntutil.o event.o
	dllwrap --export-all-symbols -o ../lib/libW11.dll xlib.o ntutil.o event.o -mwindows

xlib.o: xlib.c ntdef.h
	gcc -c xlib.c ${CFLAGS} -I.. 

ntutil.o: ntutil.c ntdef.h
	gcc -c ntutil.c ${CFLAGS} -I.. 

event.o: event.c ntdef.h
	gcc -c event.c ${CFLAGS} -I.. 

debug:
	gcc -c xlib.c -g -I.. 
	gcc -c event.c -g -I.. 
	gcc -c ntutil.c -g -I.. 

clean:
	-rm ../lib/libW11.dll xlib.o ntutil.o event.o
