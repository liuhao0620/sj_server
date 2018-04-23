CPP=g++
CPPFLAGS= -g -std=c++11
INCS= -I../common/net -I../common/include
TARGET=fight_gs
OBJS=fight_game_server.o
LIBS=/usr/local/lib/libuv.a -lpthread

$(TARGET):$(OBJS)
	$(CPP) $(CPPFLAGS) $(INCS) -o $(TARGET) $(OBJS) $(LIBS)

clean:
	rm -rf fight_gs fight_game_server.o

.cpp.o:
	$(CPP) $(CPPFLAGS) $(INCS) -o $@ -c $<
