# Generated automatically by Visual-MinGW.
# http://visual-mingw.sourceforge.net/


# Standard Variables

CC = g++
WRES = windres
RANLIB = ranlib
DLLWRAP = dllwrap
CPPFLAGS = 

LIBS = -lwinui -lcomctl32 -lgetopt
INCDIRS = -I../lib -I../winui/include
OPTIMIZ = -O2
STRIP = -s
LIBDIRS = -L ../lib -L ../winui/lib


# Compilation flags

ifeq ($(MAKECMDGOALS),debug)
CFLAGS = -W -Wall -pedantic -g -fvtable-thunks
LDFLAGS = 
else
CFLAGS = -W -Wall -pedantic $(OPTIMIZ) -fvtable-thunks
LDFLAGS = $(STRIP) 
endif


# Main target

BINDIR = ../1.0

TARGET = $(BINDIR)/msys.exe

SRCS = \
	main.cpp

RES = \
	rsrc.rc

OBJS = \
	main.o \
	rsrc.o

# Explicit rules
MAKEOBJ = $(CC) $(CFLAGS) $(CPPFLAGS) $(INCDIRS) -c -o $@ $<

%.o : %.rc
	$(WRES) $(CPPFLAGS) $< $@

%.o : %.c
	$(MAKEOBJ)

%.o : %.cpp
	$(MAKEOBJ)

$(TARGET): deps $(OBJS)
	$(CC) -mwindows $(LDFLAGS) $(INCDIRS) $(LIBDIRS) -o $@ $(OBJS) $(LIBS)


# Targets

all:	$(TARGET)

debug:	$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)


# Automatic Dependencies

deps: $(SRCS)
	$(CC) -MM $(CFLAGS) $(INCDIRS) $(SRCS) > $@

-include deps
