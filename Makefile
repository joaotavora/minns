minns: PrethreadedConnectionManager.o EchoHandler.o



EchoHandler.o: EchoHandler.h common.h
PrethreadedConnectionManager.o: EchoHandler.h common.h

clean:
	rm -rf *.o minns *.dSYM