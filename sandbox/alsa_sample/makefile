CC = gcc
CFLAGS =
INCL = `pkg-config --cflags alsa`
LIBS = `pkg-config --libs alsa`

all: examples

examples: examples/ex1.c examples/ex2.c examples/ex3.c examples/ex4.c
	$(CC)	$(CFLAGS) $(INCL) -o build/ex1 examples/ex1.c $(LIBS)
	$(CC)	$(CFLAGS) $(INCL) -o build/ex2 examples/ex2.c $(LIBS)
	$(CC)	$(CFLAGS) $(INCL) -o build/ex3 examples/ex3.c $(LIBS)
	$(CC)	$(CFLAGS) $(INCL) -o build/ex4 examples/ex4.c $(LIBS)	

#ex3_debug: examples/ex3.c
	#$(CC)	$(CFLAGS) -g $(INCL) -o build/ex3 examples/ex3.c $(LIBS)

clean:
	-rm build/*

.PHONY: clean
