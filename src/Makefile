OBJS=dbgmsg.o dbgout.o waveformat.o doublebuffer.o storageio.o host.o source.o output.o resampler.o waveflt-test.o
CFLAGS=-g -Wall -O2 -MMD -D_DEBUG

waveflt : $(OBJS)
	g++ -o waveflt $(OBJS)

.cpp.o:
	g++ -c $(CFLAGS) $<

clean:
	rm -rf *.o *.d waveflt

-include *.d
