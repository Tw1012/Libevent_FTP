#include <iostream>
using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::flush;
// #include <accctrl.h>  Windows平台上的一个头文件，通常用于访问控制列表（Access Control List，ACL）相关的功能
#include <stdlib.h>
#include <signal.h> //C标准库

#define errmsg(msg)          \
	do                       \
	{                        \
		cout << msg << endl; \
		exit(1);             \
	} while (0)
/**************basic include***************/
#include <string.h>
#include <string>
using std::string;
#include <fstream>

#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>
#include <event2/buffer.h>
#include <event2/util.h>

#include "testUtil.h"
#include "XThreadPool.h"
#include "XThread.h"
#include "XTask.h"
#include "XFtpFactory.h"
#include <unistd.h>

#define SPORT 21
#define BUFS 1024

#define XThreadPoolGet XThreadPool::Get()

/*
 *  接待连接的回调函数
 */
void listen_cb(struct evconnlistener *ev, evutil_socket_t fd, struct sockaddr *addr, int socklen, void *arg)
{
	testout("main thread At listen_cb");
	sockaddr_in *sin = (sockaddr_in *)addr;
	/*
	 *  创建任务
	 */
	XTask *task = XFtpFactory::Get()->CreateTask();
	task->sock = fd;
	/*
	 *  分配任务
	 */
	XThreadPoolGet->Dispatch(task);
}

int main()
{
	//调用signal(SIGPIPE, SIG_IGN)函数，将SIGPIPE信号的处理方式设置为忽略（SIG_IGN表示忽略该信号）
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) // 检查在尝试将SIGPIPE信号的动作设置为忽略时是否发生了错误
		errmsg("signal(SIGPIPE, SIG_IGN) error!");

	/*
	 *  初始化线程池
	 */
	XThreadPoolGet->Init(10);

	// 初始化libevent的事件集合
	event_base *base = event_base_new();
	if (!base)
		errmsg("main thread event_base_new error");

	/*
	 *  创建libevent上下文
	 */
	sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));			 
	sin.sin_family = AF_INET;				 // 网际协议IPv4
	sin.sin_port = htons(SPORT);			 // 端口号21
	sin.sin_addr.s_addr = htonl(INADDR_ANY); 

	// 创建监听事件
	//通过这段代码，可以创建一个监听器，用于在指定地址和端口上接收连接请求，并在有连接到来时调用指定的回调函数进行处理。
	evconnlistener *ev = evconnlistener_new_bind(
		base,									   // libevent的上下文
		listen_cb,								   // 接收到连接的回调函数
		base,									   // 回调函数获取的参数 arg
		LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, // 地址重用，evconnlistener关闭同时关闭socket
		10,										   // 连接队列大小，对应listen函数
		(sockaddr *)&sin,						   // 绑定的地址和端口
		sizeof(sin));

	cout << "ev:" << ev << endl;
	cout << "err:" << errno << endl;
	if (base)
	{
		cout << "begin to listen..." << endl;
		cout << "pid = " << getpid() << endl;	   // 用来取得目前进程的进程识别码
		//从这里开始进入事件的循环
		event_base_dispatch(base); // return 0：成功,  -1 ：发生错误， 1 已退出循环，没有待监测或则激活的事件
	}

	cout << "event_base_dispatch end..." << endl;
	
	if (ev)
		evconnlistener_free(ev);
	if (base)
		event_base_free(base);
	testout("server end");
	return 0;
}