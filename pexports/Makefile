CC=gcc -Wall

DISTFILES=README COPYING ChangeLog Makefile hlex.l hparse.y pexports.h pexports.c str_tree.c str_tree.h

OBJS=hlex.o hparse.o pexports.o str_tree.o

all: pexports.exe

pexports.exe: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) $(OBJS)

pexports.o: pexports.c pexports.h str_tree.h
str_tree.o: str_tree.c str_tree.h
hlex.o: hlex.c hparse.h
hlex.c: hlex.l 
hparse.o: hparse.c str_tree.h
hparse.h: hparse.c
hparse.c: hparse.y
	bison -d $< -o $@

pexports.zip: $(DISTFILES)
	zip $@ $(DISTFILES)

clean:
	$(RM) *.o *.exe hlex.c hparse.c hparse.h pexports.zip

