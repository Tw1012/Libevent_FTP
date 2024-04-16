#pragma once
#include "XTask.h"

//单例模式
class XFtpFactory
{
public:
	static XFtpFactory* Get() {
		static XFtpFactory f;
		return &f;
	}
	XTask *CreateTask();
private:
	XFtpFactory();
};