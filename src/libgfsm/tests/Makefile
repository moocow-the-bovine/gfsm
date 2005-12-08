TARGETS = calctest

KNOWN_TARGETS = \
	calctest \
	pathtest \
	memtest \
	memtest-general \
	ftest \
	flextest


CC = gcc
#CC = ccmalloc --no-wrapper gcc
LD = $(CC)

CPPFLAGS = -I. -I.. -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include
CFLAGS   ?= -Wall -g

LDFLAGS  = -L../libgfsm/.libs -L/usr/local/lib -L/usr/lib
LIBS     = ../.libs/libgfsm.a -lglib-2.0 -lm

FLEX ?= flex
FLEXFLAGS ?=

BISON ?= bison
BISONFLAGS ?= --verbose

all: $(TARGETS)

##-- keep intermediate files
.SECONDARY:

##-- flex
%.lex.c %.lex.h: %.l
	$(FLEX) $(FLEXFLAGS) --outfile="$*.lex.c" --header-file="$*.lex.h" $^

##-- bison
%.tab.c %.tab.h: %.y
	$(BISON) $(BISONFLAGS) --defines --file-prefix="$*" --name-prefix="$*_yy" $^

##-- flex+bison
calctest: calctest.lex.o calctest.tab.o
	$(LD) $(LDFLAGS) -o $@ $^ $(GFSMLIBS) $(LIBS)

##-- .c -> .o
%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $< 

##-- clean
clean:
	rm -f *.o *.lo *.tab.[ch] *.lex.[ch] $(KNOWN_TARGETS)


##-- executables
%test: %test.o
	$(LD) $(LDFLAGS) -o $@ $^ $(GFSMLIBS) $(LIBS)

%-general: %-general.o
	$(LD) $(LDFLAGS) -o $@ $^ $(GFSMLIBS) $(LIBS)
