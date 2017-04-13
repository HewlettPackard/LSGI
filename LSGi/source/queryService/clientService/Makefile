CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS =		QueryClient.o

LIBS =

TARGET =	QueryClient

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)
	cp QueryClient ../../../demo/graphQuery
all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
