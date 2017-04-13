CXXFLAGS =	-Wall -Wno-sign-compare  -lpthread -lpmem

SOURCES =	QueryService.cpp

OBJS =		QueryService.o

LIBS =

TARGET =	QueryService

$(TARGET):	$(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES) $(LIBS)
	cp QueryService ../../../demo/inference/
all:	$(TARGET)
     
clean:
	rm -f $(OBJS) $(TARGET)
