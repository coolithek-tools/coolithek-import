
#ifndef __SERVERLIST_H__
#define __SERVERLIST_H__

#include <stdint.h>
#include <string>

using namespace std;

class CServerlist
{
	private:
		string xmlLastContent;
		string userAgent;
		vector<string> serverList_v;

		void Init(string ue);
		static void xmlEndElement(void* userData, const char *element);
		static void xmlHandleData(void* userData, const char *content, int length);
		bool xmlParse(string xmlData);
		void clearConfig();

	public:
		CServerlist(string ue="");
		~CServerlist();
		void getServerList();

};

#endif // __SERVERLIST_H__
