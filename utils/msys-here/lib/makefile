# Generated automatically by Visual-MinGW.
# http://visual-mingw.sourceforge.net/


# Standard Variables

CC = gcc
WRES = windres
RANLIB = ranlib
DLLWRAP = dllwrap
CPPFLAGS = -DHAVE_STRING_H

LIBS = 
INCDIRS = -I../include
OPTIMIZ = -O2
STRIP = -s
LIBDIRS = -L../lib


# Compilation flags

ifeq ($(MAKECMDGOALS),debug)
CFLAGS = -W -Wall -pedantic -g 
LDFLAGS = 
else
CFLAGS = -W -Wall -pedantic $(OPTIMIZ) 
LDFLAGS = $(STRIP) 
endif


# Main target

BINDIR = .

TARGET = $(BINDIR)/libgetopt.a

SRCS = \
	getopt.c

RES =

OBJS = \
	getopt.o

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
