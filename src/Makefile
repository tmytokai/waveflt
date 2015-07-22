OBJS=dcoffset.o track.o waveformat.o trackmanager.o waveflt-test.o io.o
CFLAGS=-g -Wall -O2 -MMD -D_DEBUG

waveflt : $(OBJS)
	g++ -o waveflt $(OBJS)

.cpp.o:
	g++ -c $(CFLAGS) $<

clean:
	rm -rf *.o *.d

-include *.d