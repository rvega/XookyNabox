CC = g++
CFLAGS = -g
INCL = `pkg-config --cflags alsa` `pkg-config --cflags sndfile` -Ivendor/zengarden/src -Ivendor/rtaudio
LIBS = `pkg-config --libs alsa` `pkg-config --libs sndfile` -Lvendor/zengarden/libs/Linux-i686 -lzengarden

all: zg_sample_3
#zg_sample_1 zg_sample_2 zg_sample_3 

zg_sample_3: zg_sample_3.cpp
	$(CC)	$(CFLAGS) $(INCL) -o build/zg_sample_3 zg_sample_3.cpp vendor/rtaudio/tests/Release/RtAudio.o $(LIBS)

zg_sample_1: zg_sample_1.cpp
	$(CC)	$(CFLAGS) $(INCL) -o build/zg_sample_1 zg_sample_1.cpp $(LIBS)

zg_sample_2: zg_sample_2.cpp
	$(CC)	$(CFLAGS) $(INCL) -o build/zg_sample_2 zg_sample_2.cpp $(LIBS)

clean:
	rm build/*

.PHONY: clean
