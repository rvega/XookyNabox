CFLAGS =
INCL = 
LIBS = -lportaudio

all: echo sawtooth

echo: echo.c
	gcc echo.c $(CFLAGS) $(INCL) -o build/echo $(LIBS)

sawtooth:
	g++ sawtooth.cpp $(CFLAGS) $(INCL) -o build/sawtooth $(LIBS)

clean:
	-rm build/*

.PHONY: clean
