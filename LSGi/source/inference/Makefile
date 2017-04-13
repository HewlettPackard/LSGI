CFLAGS=-w -Wno-sign-compare  -lm -lpthread -lpmem

SOURCES=  utils.cpp inferenceConfig.cpp \
	inMemoryGraph.cpp globalStates.cpp \
	processSync.cpp binaryGraphReader.cpp \
	 inference.cpp engineMain.cpp

OBJECTS= utils.o inferenceConfig.o \
	inMemoryGraph.o globalStates.o	\
	processSync.o binaryGraphReader.o \
	inference.o engineMain.o

EXECUTABLE=gibbsDNSApp

all: $(EXECUTABLE)
	cp gibbsDNSApp  ../../demo/inference/

$(EXECUTABLE): $(SOURCES)
	g++ $(CFLAGS)  -o $@ $(SOURCES)

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
