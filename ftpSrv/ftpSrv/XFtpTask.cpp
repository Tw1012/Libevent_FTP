#include "XFtpTask.h"
#include "testUtil.h"

#include <event2/bufferevent.h>
#include <event2/event.h>

#include <string.h>
#include <iostream>
using namespace std;

void XFtpTask::ConnectoPORT() {
	testout("At XFtpTask::ConnectoPORT");
	if (cmdTask->ip.empty() || cmdTask->port <= 0 || !cmdTask->base) {
		cout << "ConnectPORT failed" << endl;
		return;
	}
	if (bev) {
		bufferevent_free(bev);
		bev = 0;
	}

	//创建基于套接字的buffevent
	bev = bufferevent_socket_new(cmdTask->base, -1, BEV_OPT_CLOSE_ON_FREE);
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(cmdTask->port);

	//evutil_inet_pton(int af, const char *src, void *dst)
	//evutil_inet_pton提供以标准文本表示形式将 Internet 网络地址转换为数字二进制形式的与协议无关的转换
	//af分别设置为AF_INET或者AF_INET6，
	//src是要解析的地址字符串，dst则适当的指向一个in_addr或者in_addr6结构
    evutil_inet_pton(AF_INET, cmdTask->ip.c_str(), &sin.sin_addr.s_addr);

	
	Setcb(bev);

	//定时器 60秒，0微秒
	timeval t = {60, 0};

	/*
	int bufferevent_set_timeouts(struct bufferevent *bufev,
			 const struct timeval *tv_read,
			 const struct timeval *tv_write);
	读写超时
	*/
	bufferevent_set_timeouts(bev, &t, 0);

	//在基于套接字的 bufferevent 上启动连接
	bufferevent_socket_connect(bev, (sockaddr*)&sin, sizeof(sin));
}

void XFtpTask::ClosePORT() {
	if (bev) {
		bufferevent_free(bev);
		bev = 0;
	}
	if (fp) {
		fclose(fp);
		fp = 0;
	}
}

void XFtpTask::Setcb(bufferevent *bev) {
	/*
	操作回调、水位和启用/禁用
	void bufferevent_setcb(struct bufferevent *bufev,//发生了事件的bufferevent
    bufferevent_data_cb readcb,		//读取足够多数据时的回调 
    bufferevent_data_cb writecb,	//写入足够多数据时的回调
    bufferevent_event_cb eventcb,	//发生错误时的回调
    void *cbarg);
	*/
	bufferevent_setcb(bev, ReadCB, WriteCB, EventCB, this);
	
	//启用
	bufferevent_enable(bev, EV_READ | EV_WRITE);

}

void XFtpTask::Send(const string &data) {
	testout("At XFtpTask::Send");
	Send(data.c_str(), data.size());
}
void XFtpTask::Send(const char *data, size_t datasize) {
	testout("At XFtpTask::Send");
	cout << data;
	cout << datasize << endl;
	if (datasize == 0) return;
	if (bev) {
		bufferevent_write(bev, data, datasize);
	}
}

void XFtpTask::ResCMD(string msg) {
	testout("At XFtpTask::ResCMD");
	if (!cmdTask || !cmdTask->bev) return;
	// cout << "ResCMD: " << msg << endl << flush;
	cout << "ResCMD: " << msg << endl;
	if (msg[msg.size() - 1] != '\n') {
		msg += "\r\n";
	}
	bufferevent_write(cmdTask->bev, msg.c_str(), msg.size());
}


void XFtpTask::EventCB(bufferevent *bev, short events, void *arg) {
	XFtpTask *t = (XFtpTask*)arg;
	t->Event(bev, events);
}
void XFtpTask::ReadCB(bufferevent *bev, void *arg) {
	XFtpTask *t = (XFtpTask*)arg;
	t->Read(bev);
}
void XFtpTask::WriteCB(bufferevent *bev, void *arg) {
	XFtpTask *t = (XFtpTask*)arg;
	t->Write(bev);
}


XFtpTask::~XFtpTask() {
	ClosePORT();
}