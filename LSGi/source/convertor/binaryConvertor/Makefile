CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS =		binaryConvertor.o

LIBS =

TARGET =	binaryConvertor

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
