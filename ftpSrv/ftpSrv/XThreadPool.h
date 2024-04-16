#pragma once
#include <vector>

class XThread;
class XTask;

//采用单例模式
class XThreadPool
{
public:

	static XThreadPool *Get() {
		static XThreadPool p;
		return &p;
	}

	void Init(int threadCount);

	void Dispatch(XTask*);
private:
	int threadCount;
	int lastThread = -1;
	std::vector<XThread *> threads;
	XThreadPool() {};
};

