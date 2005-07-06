INC=$(DINCLUDE)

first_rule: all

all allbin: ../lib/libW11.dll

alldoc tags:

../lib/libW11.dll: xlib.o ntutil.o event.o
	dllwrap --export-all-symbols -o ../lib/libW11.dll xlib.o ntutil.o event.o -mwindows

xlib.o: xlib.c ntdef.h
	gcc -c xlib.c -O $(INC)

ntutil.o: ntutil.c ntdef.h
	gcc -c ntutil.c -O $(INC) 

event.o: event.c ntdef.h
	gcc -c event.c -O $(INC) 

debug:
	gcc -c xlib.c -g $(INC) 

clean distclean:
	rm -f ../lib/libW11.dll xlib.o ntutil.o event.o
