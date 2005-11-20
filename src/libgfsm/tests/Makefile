TARGETS = pathtest

CPPFLAGS = -I. -I.. -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include
CFLAGS   = -g

LDFLAGS  = -L../libgfsm/.libs -L/usr/local/lib -L/usr/lib
LIBS     = ../.libs/libgfsm.a -lglib-2.0 -lm


all: $(TARGETS)


%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $< 

pathtest: pathtest.o ../.libs/libgfsm.a
	$(CC) $(LDFLAGS) -o $@ pathtest.o $(GFSMLIBS) $(LIBS)

clean:
	rm -f *.o *.lo $(TARGETS)
