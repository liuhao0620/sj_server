CPP=g++
CPPFLAGS= -g -std=c++11
INCS= -I../common/net -I../common/include
TARGET=test_client
OBJS=test_client.o
LIBS=/usr/local/lib/libuv.a -lpthread

$(TARGET):$(OBJS)
	$(CPP) $(CPPFLAGS) $(INCS) -o $(TARGET) $(OBJS) $(LIBS)

clean:
	rm -rf $(TARGET) $(OBJS)

.cpp.o:
	$(CPP) $(CPPFLAGS) $(INCS) -o $@ -c $<
