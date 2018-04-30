CPP=g++
CPPFLAGS= -g -std=c++11
INCS= -I../common/net -I../common/include -I../common/log -I../common/protoc
TARGET=test_client
OBJS=test_client.o
LIBS=/usr/local/lib/libuv.a /usr/local/lib/libprotobuf.a -lpthread
PBOBJS=../common/protoc/gprotoc/internal_protocol.pb.o ../common/protoc/gprotoc/external_protocol.pb.o

all:$(TARGET)

clean:
	rm -rf $(TARGET) $(OBJS)

$(TARGET):$(OBJS) $(PBOBJS)
	$(CPP) $(CPPFLAGS) $(INCS) -o $(TARGET) $(OBJS) $(PBOBJS) $(LIBS)

.cpp.o:
	$(CPP) $(CPPFLAGS) $(INCS) -o $@ -c $<

../common/protoc/gprotoc/internal_protocol.pb.o:../common/protoc/gprotoc/internal_protocol.pb.cc
	$(CPP) $(CPPFLAGS) $(INCS) -o $@ -c $<

../common/protoc/gprotoc/external_protocol.pb.o:../common/protoc/gprotoc/external_protocol.pb.cc
	$(CPP) $(CPPFLAGS) $(INCS) -o $@ -c $<

