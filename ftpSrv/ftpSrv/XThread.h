#include <event2/util.h>
#include <list>
#include <mutex>

class XTask;
struct event_base;

class XThread
{
public:
	void Start();

	void Main();

	bool Setup();

	void Notify(evutil_socket_t, short);

	void Activate();

	void AddTack(XTask *);

	XThread();
	~XThread();

	void setId(int id){
		m_id = id;
	}

	int getId(){
		return m_id;
	}

private:
	int m_id = 0;
	int notify_send_fd = 0;
	event_base *base = 0;
	std::list<XTask*> tasks;
	std::mutex tasks_mutex;
};

