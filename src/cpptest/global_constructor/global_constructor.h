#include <list>

class CClient;
class CServer {
public:
	CServer();
	void Append(CClient* p);
	std::list<CClient*> mList;
};

class CClient {
public:
	CClient(const char* name);
	const char* mName;
};

