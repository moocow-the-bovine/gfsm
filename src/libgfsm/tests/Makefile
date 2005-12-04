TARGETS = ftest

KNOWN_TARGETS = \
	pathtest \
	memtest \
	memtest-general \
	ftest


CC = gcc
#CC = ccmalloc --no-wrapper gcc
LD = $(CC)

CPPFLAGS = -I. -I.. -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include
CFLAGS   ?= -Wall -g

LDFLAGS  = -L../libgfsm/.libs -L/usr/local/lib -L/usr/lib
LIBS     = ../.libs/libgfsm.a -lglib-2.0 -lm


all: $(TARGETS)


%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $< 

##-- clean
clean:
	rm -f *.o *.lo $(KNOWN_TARGETS)


##-- executables
%test: %test.o
	$(LD) $(LDFLAGS) -o $@ $^ $(GFSMLIBS) $(LIBS)

%-general: %-general.o
	$(LD) $(LDFLAGS) -o $@ $^ $(GFSMLIBS) $(LIBS)
