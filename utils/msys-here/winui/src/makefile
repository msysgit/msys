# Generated automatically by Visual-MinGW.
# http://visual-mingw.sourceforge.net/


# Standard Variables

CC = g++
WRES = windres
RANLIB = ranlib
DLLWRAP = dllwrap
CPPFLAGS = 

LIBS = -lcomctl32
INCDIRS = -I ../include
OPTIMIZ = -O2
STRIP = -s
LIBDIRS = -L../lib


# Compilation flags

ifeq ($(MAKECMDGOALS),debug)
CFLAGS = -W -Wall -pedantic -g -fvtable-thunks -fno-exceptions -fno-rtti
LDFLAGS = 
else
CFLAGS = -W -Wall -pedantic $(OPTIMIZ) -fvtable-thunks -fno-exceptions -fno-rtti
LDFLAGS = $(STRIP) 
endif


# Main target

BINDIR = ../lib

TARGET = $(BINDIR)/libwinui.a

SRCS = \
	CFile.cpp \
	CList.cpp \
	winui.cpp

RES =

OBJS = \
	CFile.o \
	CList.o \
	winui.o

# Explicit rules
MAKEOBJ = $(CC) $(CFLAGS) $(CPPFLAGS) $(INCDIRS) -c -o $@ $<

%.o : %.rc
	$(WRES) $(CPPFLAGS) $< $@

%.o : %.c
	$(MAKEOBJ)

%.o : %.cpp
	$(MAKEOBJ)

$(TARGET): deps $(OBJS)
	rm -f $@
	$(AR) -cru $@ $(OBJS)
	$(RANLIB) $@
# Targets

all:	$(TARGET)

debug:	$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)


# Automatic Dependencies

deps: $(SRCS)
	$(CC) -MM $(CFLAGS) $(INCDIRS) $(SRCS) > $@

-include deps
