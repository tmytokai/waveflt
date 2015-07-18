OBJS=dcoffset.o track.o waveformat.o
CFLAGS=-g -Wall -O2 -MMD -D_DEBUG

waveflt : $(OBJS)
	ls -al $(OBJS)

.cpp.o:
	g++ -c $(CFLAGS) $<

clean:
	rm -rf *.o *.d

-include *.d