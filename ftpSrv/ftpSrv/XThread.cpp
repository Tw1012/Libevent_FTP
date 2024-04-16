#include <thread> //c++11的线程库
#include <iostream>
using namespace std;
 
#include <unistd.h> //linux的系统库
 
#include <event2/event.h>

#include "testUtil.h"
#include "XThread.h"
#include "XTask.h"

/*
evutil_socket_t fd 是文件描述符，绑定注册事件的fds[0]
short which 是在该fd上关注的事件类型，可以是EV_READ, EV_WRITE, EV_SIGNAL；这里绑定注册事件的EV_READ
void *arg 是注册事件最后一个参数传递过来的
*/
static void Notify_cb(evutil_socket_t fd, short which, void *arg) {
	XThread *t = (XThread*)arg;
	t->Notify(fd, which);
}

void XThread::Notify(evutil_socket_t fd, short which) {
	testout(XThread::getId() << " thread At Notify()");
	char buf[2] = { 0 };
 
	//fd为文件描述符；buf表示读出数据缓冲区地址；1表示读出的字节数
	int re = read(fd, buf, 1);
	if (re < 0)
		return;
	cout << XThread::getId() << " thread receive" << buf << endl;
}


void XThread::Start() {
	testout(XThread::getId() << " thread At Start()");

	Setup(); //对每个线程进行配置

	thread th(&XThread::Main, this); //c++11的线程构造函数

	//使用了 deach 后，分离的线程可以与 原父亲线程共同访问原子量
	th.detach(); //将对象表示的线程与调用线程分离，允许它们彼此独立执行
}

void XThread::Main() {
	cout << XThread::getId() << " thread::Main() begin" << endl;
	event_base_dispatch(base); //进入事件循环，linux下一般调用的io方法是epoll
	event_base_free(base);
	cout << XThread::getId() << " thread::Main() end" << endl;
	
}

bool XThread::Setup() {
	testout(XThread::getId() << " thread At Setup");

	int fds[2];

	/*
	pipe函数定义中的fd参数是一个大小为2的一个数组类型的指针，
	通过pipe函数创建的这两个文件描述符 fd[0] 和 fd[1] 分别构成管道的两端，
	往 fd[1] 写入的数据可以从 fd[0] 读出，
	并且 fd[1] 一端只能进行写操作，fd[0] 一端只能进行读操作，不能反过来使用。
	*/
	if (pipe(fds)) { 
		cerr << "pipe failed" << endl;
		return false;
	}

	notify_send_fd = fds[1];

	event_config *ev_conf = event_config_new();
	//EVENT_BASE_FLAG_NOLOCK不要为event_base分配锁，设置这个选项在于多进程或者单进程
	event_config_set_flag(ev_conf, EVENT_BASE_FLAG_NOLOCK); 
	this->base = event_base_new_with_config(ev_conf);
	event_config_free(ev_conf);
	if (!base) {
		cout << "event_base_new_with_config error!" << endl;
		return false;
	}
	//创建一个事件
	//Linux的文件描述符(这里是fds[0])是一个非负整数，用来唯一标识一个正在被进程访问的文件或其他I/O资源。
	//文件描述符在进程中被用来进行读取、写入、关闭等操作。
	//通常情况下，标准输入、标准输出和标准错误分别对应文件描述符0、1和2。
	event *ev = event_new(base, fds[0], EV_READ | EV_PERSIST, Notify_cb, this);
	event_add(ev, 0);

	return true;
}

void XThread::Activate() {
	testout(XThread::getId() << " thread At Activate()");

	//notify_send_fd为文件描述符,就是fds[1]；"c" 表示写入的内容；1表示写入内容的大小
	int re = write(notify_send_fd, "c", 1);

	if (re <= 0) {
		cerr << "XThread::Activate() fail" << endl;
	}

	XTask *t = NULL;
	tasks_mutex.lock();
	if (tasks.empty()) {
		tasks_mutex.unlock();
		return;
	}
	t = tasks.front();
	tasks.pop_front();
	tasks_mutex.unlock();
	t->Init();
}

void XThread::AddTack(XTask *t) {
	if (!t) return;

	t->base = this->base; //初始化Xtask

	tasks_mutex.lock();
	tasks.push_back(t);
	tasks_mutex.unlock();
}

XThread::XThread() {

}
XThread::~XThread() {

}