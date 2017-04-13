CXXFLAGS =	-O2 -g -Wall -fmessage-length=0 -lpmem -lpthread 

OBJS =		MultipleWriterTest.o

LIBS =

TARGET =	MultipleWriterTest.exe

$(TARGET):	$(OBJS)
	$(CXX) -lm -lpthread -lpmem  -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
