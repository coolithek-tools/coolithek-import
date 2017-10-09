
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <expat.h>

#include "mv2mariadb.h"
#include "configfile.h"
#include "common/helpers.h"
#include "curl.h"
#include "serverlist.h"

extern CMV2Mysql*	g_mainInstance;
extern GSettings	g_settings;
extern const char*	g_progName;
extern bool		g_debugPrint;

extern void myExit(int val);

CServerlist::CServerlist(string ue)
{
	Init(ue);
}

void CServerlist::Init(string ue)
{
	userAgent = ue;
	serverList_v.clear();
}

CServerlist::~CServerlist()
{
	serverList_v.clear();
}

void CServerlist::xmlEndElement(void* userData, const char *element)
{
	CServerlist* inst = static_cast<CServerlist*>(userData);

	if ((string)element == "URL")
		inst->serverList_v.push_back(inst->xmlLastContent);
}

void CServerlist::xmlHandleData(void* userData, const char *content, int length)
{
	CServerlist* inst = static_cast<CServerlist*>(userData);

	char* tmp = new char[length+1];
	strncpy(tmp, content, length);
	tmp[length] = '\0';
	inst->xmlLastContent = (string)tmp;
	delete [] tmp;
}

bool CServerlist::xmlParse(string xmlData)
{
	int ret = true;
	XML_Parser parser = XML_ParserCreate("UTF-8");
	XML_SetUserData(parser, (void*)this);
	XML_SetElementHandler(parser, NULL, xmlEndElement);
	XML_SetCharacterDataHandler(parser, xmlHandleData);

	int xmlRet = XML_Parse(parser, xmlData.c_str(), xmlData.length(), XML_TRUE);
	if (xmlRet == XML_STATUS_ERROR) {
		printf("[%s] Error parsing xml data: %s\n", g_progName, XML_ErrorString(XML_GetErrorCode(parser)));
		ret = false;
	}
	XML_ParserFree(parser);

	return ret;
}

void CServerlist::clearConfig()
{
	char cfg_key[256];
	for (int i = 1; i <= g_settings.downloadServerCount; i++) {
		memset(cfg_key, 0, sizeof(cfg_key));
		snprintf(cfg_key, sizeof(cfg_key), "downloadServer_%02d", i);
		g_mainInstance->getConfig()->deleteKey(cfg_key);
		memset(cfg_key, 0, sizeof(cfg_key));
		snprintf(cfg_key, sizeof(cfg_key), "downloadServerConnectFail_%02d", i);
		g_mainInstance->getConfig()->deleteKey(cfg_key);
	}
	for (int i = 0; i < static_cast<int>(maxDownloadServerCount); i++) {
		g_settings.downloadServer[i] = "";
		g_settings.downloadServerConnectFail[i] = 0;
	}
	g_settings.downloadServerCount = 0;
	g_settings.lastDownloadServer = 1;
}

void CServerlist::getServerList()
{
	if (g_settings.serverListUrl.empty())
		return;
	bool toFile		= false;
	string serverListXml	= "";
	CCurl* curl		= new CCurl();
	int ret = curl->CurlDownload(g_settings.serverListUrl, serverListXml, toFile, userAgent, true, false, NULL, true);
	delete curl;
	if (ret != 0)
		return;

	bool result = xmlParse(serverListXml);
	if (result) {
		if (!serverList_v.empty())
			clearConfig();
		for (size_t i = 0; i < serverList_v.size(); i++) {
			if (i >= static_cast<int>(maxDownloadServerCount))
				break;
			g_settings.downloadServerCount++;
			g_settings.downloadServer[i+1] = serverList_v[i];
		}
	}
	g_mainInstance->saveDownloadServerSetup();
}
