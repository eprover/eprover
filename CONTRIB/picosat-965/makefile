CC=gcc
CFLAGS= -DTRACE -Wall -Wextra -DNDEBUG -O3

all: picosat picomcs picomus picogcnf libpicosat.a

clean:
	rm -f picosat picomcs picomus picogcnf
	rm -f *.exe *.s *.o *.a *.so *.plist
	rm -f makefile config.h
	rm -f gmon.out *~ 

analyze:
	clang --analyze $(CFLAGS) *.c *.h

picosat: libpicosat.a app.o main.o
	$(CC) $(CFLAGS) -o $@ main.o app.o -L. -lpicosat

picomcs: libpicosat.a picomcs.o
	$(CC) $(CFLAGS) -o $@ picomcs.o -L. -lpicosat

picomus: libpicosat.a picomus.o
	$(CC) $(CFLAGS) -o $@ picomus.o -L. -lpicosat

picogcnf: libpicosat.a picogcnf.o
	$(CC) $(CFLAGS) -o $@ picogcnf.o -L. -lpicosat

app.o: app.c picosat.h makefile
	$(CC) $(CFLAGS) -c $<

picomcs.o: picomcs.c picosat.h makefile
	$(CC) $(CFLAGS) -c $<

picomus.o: picomus.c picosat.h makefile
	$(CC) $(CFLAGS) -c $<

picogcnf.o: picogcnf.c picosat.h makefile
	$(CC) $(CFLAGS) -c $<

main.o: main.c picosat.h makefile
	$(CC) $(CFLAGS) -c $<

picosat.o: picosat.c picosat.h makefile
	$(CC) $(CFLAGS) -c $<

version.o: version.c config.h makefile
	$(CC) $(CFLAGS) -c $<

config.h: makefile VERSION mkconfig.sh # and actually picosat.c
	rm -f $@; ./mkconfig.sh > $@

libpicosat.a: picosat.o version.o
	ar rc $@ picosat.o version.o
	ranlib $@

SONAME=-Xlinker -soname -Xlinker libpicosat.so
libpicosat.so: picosat.o version.o
	$(CC) $(CFLAGS) -shared -o $@ picosat.o version.o $(SONAME)

.PHONY: all clean
