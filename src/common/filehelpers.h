
#ifndef __filehelpers_h__
#define __filehelpers_h__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include <string>
#include <vector>

using namespace std;

struct helpersDebugInfo {
	string msg;
	string file;
	string func;
	int line;
};

class CFileHelpers
{
	private:
		uint32_t FileBufMaxSize;
		int fd1, fd2;

		char* initFileBuf(char* buf, uint32_t size);
		char* deleteFileBuf(char* buf);
		bool ConsoleQuiet;
		helpersDebugInfo DebugInfo;
		void setDebugInfo(const char* msg, const char* file, const char* func, int line);
		void printDebugInfo();

	public:
		CFileHelpers();
		~CFileHelpers();
		static CFileHelpers* getInstance();
		bool doCopyFlag;

		void clearDebugInfo();
		void readDebugInfo(helpersDebugInfo* di);
		void setConsoleQuiet(bool q) { ConsoleQuiet = q; };
		bool getConsoleQuiet() { return ConsoleQuiet; };

		bool cp(const char *Src, const char *Dst, const char *Flags="");
		bool copyFile(const char *Src, const char *Dst, mode_t forceMode=0);
		bool copyDir(const char *Src, const char *Dst, bool backupMode=false);
		static bool createDir(string& Dir, mode_t mode = 0755);
		static bool createDir(const char *Dir, mode_t mode = 0755){string dir = string(Dir);return createDir(dir, mode);}
		static bool removeDir(const char *Dir);
		static uint64_t getDirSize(const char *dir);
		static uint64_t getDirSize(const string& dir){return getDirSize(dir.c_str());};
};

#endif // __filehelpers_h__
