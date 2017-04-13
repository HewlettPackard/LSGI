CXXFLAGS =      -O2 -g -Wall -fmessage-length=0 -lpthread -lpmem

OBJS1 =         test_pullUpdatesLoop.o

OBJS2 =         test_createUpdatePersistLoop.o

TARGET1 =       test_pullUpdatesLoop

TARGET2 =       test_createUpdatePersistLoop

$(TARGET1):     $(OBJS1) $(OBJS2)
	$(CXX) -o $(TARGET1) $(OBJS1) $(CXXFLAGS)
	$(CXX) -o $(TARGET2) $(OBJS2) $(CXXFLAGS)

all:    $(TARGET1)

clean:
	rm -f $(OBJS1) $(TARGET1) $(OBJS2) $(TARGET2)
