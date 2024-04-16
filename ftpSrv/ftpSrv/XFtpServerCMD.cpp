#include <string.h>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/util.h>

#include <string>
using namespace std;

#include "XFtpServerCMD.h"
#include "testUtil.h"

#define BUFS 4096

void XFtpServerCMD::Reg(std::string cmd, XFtpTask *call) {
	testout("At XFtpServerCMD::Reg");
	if (!call) {
		cout << "XFtpServerCMD::Reg call is null " << endl;
		return;
	}
	if (cmd.empty()) {
		cout << "XFtpServerCMD::Reg cmd is null " << endl;
		return;
	}
	
	if (calls.find(cmd) != calls.end()) {
		cout << cmd << " is alredy register" << endl;
		return;
	}
	testout(cmd << " Reg success");
	call->base = base; //此base是从Xtask继承过来的
	call->cmdTask = this; //父类 XFtpTask* 指针指向子类XFtpServerCMD对象指针
	calls[cmd] = call;
	calls_del[call] = 0;
}

void XFtpServerCMD::Event(bufferevent *bev, short events) {
	testout("At XFtpServerCMD::Event");
	if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT)) {
		delete this;
	}
}

void XFtpServerCMD::Read(bufferevent *bev) {
	cout << endl;
	testout("At XFtpServerCMD::Read");
	char buf[BUFS] = { 0 };
	while (1) {
		int len = bufferevent_read(bev, buf, BUFS);
		if (len <= 0) break;
		cout << "Recv CMD(" << len << "):" << buf;

		string type = "";
		for (int i = 0; i < len; i++) {
			if (buf[i] == ' ' || buf[i] == '\r')
				break;
			type += buf[i];
		}


		cout << "type is [" << type << "]" << endl;
		if (calls.find(type) != calls.end()) {
			testout("begin to parse");
			XFtpTask *t = calls[type];
			t->Parse(type, buf);
			testout("curDir: [" << curDir << "]");
		}
		else {
			cout << "parse object not found" << endl;
			ResCMD("200 OK\r\n");
		}
	}
}

bool XFtpServerCMD::Init() {
	testout("At XFtpServerCMD::Init");


	bufferevent *bev = bufferevent_socket_new(base, sock, BEV_OPT_CLOSE_ON_FREE);
	if (!bev) {
		delete this;
		return false;
	}


	timeval t = {300, 0};
	bufferevent_set_timeouts(bev, &t, 0);

	string msg = "220 Welcome to XFtpServer\r\n";
	bufferevent_write(bev, msg.c_str(), msg.size());

	this->cmdTask = this;
	this->bev = bev;

	Setcb(bev);

	return true;
}

XFtpServerCMD::XFtpServerCMD() {
}

XFtpServerCMD::~XFtpServerCMD() {
	ClosePORT();
	for (auto i : calls_del) {
		delete i.first;
	}
}