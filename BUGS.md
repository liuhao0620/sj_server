## 综述
	
	本文件记录在使用libuv过程中遇到的问题

## 2018/05/09

	经过进一步测试，AllocCb与RecvCb必然是成对调用的，但是被async打断的时候，RecvCb的参数nread为0

## 2018/05/05
	
	使用uv_async_send在uv_udp_recv_start接收线程中调用uv_udp_send时，会导致AllocCb被调用后，没有调用RecvCb
	不要在AllocCb中申请堆内存，想当然的在RecvCb中释放，因为上述情况会导致两个函数不成对调用
