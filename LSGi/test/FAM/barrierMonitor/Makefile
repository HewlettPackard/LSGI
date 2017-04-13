CXXFLAGS =	-O2 -g -Wall -fmessage-length=0 -lpmem -lpthread 

OBJS =		BarrierMonitor.o

LIBS =

TARGET =	BarrierMonitor

$(TARGET):	$(OBJS)
	$(CXX) -lm -lpthread -lpmem  -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
