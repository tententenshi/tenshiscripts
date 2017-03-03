#include <stdio.h>
#include "global_constructor.h"

CServer::CServer()
{
	printf("CServer Constructor called\n");
}
void CServer::Append(CClient* p)
{
	mList.push_back(p);
	printf("CServer Append called\n");
}

CClient::CClient(const char* name)
{
	mName = name;
	extern CServer gServer;
	gServer.Append(this);
	printf("CClient Constructor called\n");
}

CServer gServer;	// OK
CClient gClient1("gClient1");
CClient gClient2("gClient2");
//CServer gServer;	// list may corrupted!!!


int main()
{
	for (std::list<CClient*>::iterator i = gServer.mList.begin(); i != gServer.mList.end(); i++) {
		CClient* pCurClient = *i;
		printf("cur client is %s\n", pCurClient->mName);
	}
	printf("program end\n");
}
