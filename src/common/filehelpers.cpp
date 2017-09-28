
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#include <algorithm>

#include "helpers.h"
#include "filehelpers.h"

#ifndef __path_file__
#define __path_file__ __FILE__
#endif

CFileHelpers::CFileHelpers()
{
	FileBufMaxSize	= 0xFFFF;
	doCopyFlag	= true;
	ConsoleQuiet	= false;
	clearDebugInfo();
}

CFileHelpers::~CFileHelpers()
{
}

char* CFileHelpers::initFileBuf(char* buf, uint32_t size)
{
	if (buf == NULL)
		buf = new char[size];
	return buf;
}

char* CFileHelpers::deleteFileBuf(char* buf)
{
	if (buf != NULL)
		delete [] buf;
	buf = NULL;
	return buf;
}

CFileHelpers* CFileHelpers::getInstance()
{
	static CFileHelpers* FileHelpers = NULL;
	if(!FileHelpers)
		FileHelpers = new CFileHelpers();
	return FileHelpers;
}

void CFileHelpers::clearDebugInfo()
{
	DebugInfo.msg.clear();
	DebugInfo.file.clear();
	DebugInfo.func.clear();
	DebugInfo.line = 0;
}

void CFileHelpers::setDebugInfo(const char* msg, const char* file, const char* func, int line)
{
	DebugInfo.msg  = msg;
	DebugInfo.file = file;
	DebugInfo.func = func;
	DebugInfo.line = line;
}

void CFileHelpers::readDebugInfo(helpersDebugInfo* di)
{
	di->msg  = DebugInfo.msg;
	di->file = DebugInfo.file;
	di->func = DebugInfo.func;
	di->line = DebugInfo.line;
}

void CFileHelpers::printDebugInfo()
{
	if (!ConsoleQuiet)
		printf(">>>> [%s:%d] %s\n", DebugInfo.func.c_str(), DebugInfo.line, DebugInfo.msg.c_str());
}

bool CFileHelpers::cp(const char *Src, const char *Dst, const char *Flags/*=""*/)
{
	clearDebugInfo();
	if ((Src == NULL) || (Dst == NULL)) {
		setDebugInfo("One or more parameters are NULL", __path_file__, __func__, __LINE__);
		printDebugInfo();
		return false;
	}

	string src = Src;
	src = trim(src);
	if (src.find_first_of("/") != 0)
		src = "./" + src;
	size_t pos = src.find_last_of("/");
	if (pos == src.length()-1)
		src = src.substr(0, pos);

	string dst = Dst;
	dst = trim(dst);
	if (dst.find_first_of("/") != 0)
		dst = "./" + dst;
	pos = dst.find_last_of("/");
	if (pos == dst.length()-1)
		dst = dst.substr(0, pos);

	bool wildcards      = (src.find("*") != string::npos);
	bool recursive      = ((strchr(Flags, 'r') != NULL) || (strchr(Flags, 'a') != NULL));
	bool no_dereference = ((strchr(Flags, 'd') != NULL) || (strchr(Flags, 'a') != NULL));

	static struct stat FileInfo;
	char buf[PATH_MAX];
	if (wildcards == false) {
		if (!file_exists(src.c_str())) {
			setDebugInfo("Source file not exist", __path_file__, __func__, __LINE__);
			printDebugInfo();
			return false;
		}
		if (lstat(src.c_str(), &FileInfo) == -1) {
			setDebugInfo("lstat error", __path_file__, __func__, __LINE__);
			printDebugInfo();
			return false;
		}

		pos = src.find_last_of("/");
		string fname = src.substr(pos);

		static struct stat FileInfo2;
		// is symlink
		if (S_ISLNK(FileInfo.st_mode)) {
			int len = readlink(src.c_str(), buf, sizeof(buf)-1);
			if (len != -1) {
				buf[len] = '\0';
				if (!no_dereference) { /* copy */
					string buf_ = (string)buf;
					char buf2[PATH_MAX + 1];
					if (buf[0] != '/')
						buf_ = getPathName(src) + "/" + buf_;
					buf_ = (string)realpath(buf_.c_str(), buf2);
					//printf("\n>>>> RealPath: %s\n \n", buf_.c_str());
					if (file_exists(dst.c_str()) && (lstat(dst.c_str(), &FileInfo2) != -1)){
						if (S_ISDIR(FileInfo2.st_mode))
							copyFile(buf_.c_str(), (dst + fname).c_str());
						else {
							unlink(dst.c_str());
							copyFile(buf_.c_str(), dst.c_str());
						}
					}
					else
						copyFile(buf_.c_str(), dst.c_str());
				}
				else { /* link */
					if (file_exists(dst.c_str()) && (lstat(dst.c_str(), &FileInfo2) != -1)){
						if (S_ISDIR(FileInfo2.st_mode))
							symlink(buf, (dst + fname).c_str());
						else {
							unlink(dst.c_str());
							symlink(buf, dst.c_str());
						}
					}
					else
						symlink(buf, dst.c_str());
				}
			}
		}
		// is directory
		else if (S_ISDIR(FileInfo.st_mode)) {
			if (recursive)
				copyDir(src.c_str(), dst.c_str());
			else {
				setDebugInfo("'recursive flag' must be set to copy dir.", __path_file__, __func__, __LINE__);
				printDebugInfo();
				return false;
			}
		}
		// is file
		else if (S_ISREG(FileInfo.st_mode)) {
			if (file_exists(dst.c_str()) && (lstat(dst.c_str(), &FileInfo2) != -1)){
				if (S_ISDIR(FileInfo2.st_mode))
					copyFile(src.c_str(), (dst + fname).c_str());
				else {
					unlink(dst.c_str());
					copyFile(src.c_str(), dst.c_str());
				}
			}
			else
				copyFile(src.c_str(), dst.c_str());
		}
		else {
			setDebugInfo("Currently unsupported st_mode.", __path_file__, __func__, __LINE__);
			printDebugInfo();
			return false;
		}
	}
	else {
		setDebugInfo("Wildcard feature not yet realized.", __path_file__, __func__, __LINE__);
		printDebugInfo();
		return false;
	}

	return true;
}

bool CFileHelpers::copyFile(const char *Src, const char *Dst, mode_t forceMode/*=0*/)
{
	doCopyFlag = true;

	/*
	set mode for Dst
	----------------
	when forceMode==0 (default) then
	    when Dst exists
	        mode = mode from Dst
	    else
	        mode = mode from Src
	else
	    mode = forceMode
	*/
	mode_t mode = forceMode & 0x0FFF;
	if (mode == 0) {
		static struct stat FileInfo;
		const char *f = Dst;
		if (!file_exists(Dst))
			f = Src;
		if (lstat(f, &FileInfo) == -1)
			return false;
		mode = FileInfo.st_mode & 0x0FFF;
	}

	if ((fd1 = open(Src, O_RDONLY)) < 0)
		return false;
	if (file_exists(Dst))
		unlink(Dst);
	if ((fd2 = open(Dst, O_WRONLY | O_CREAT, mode)) < 0) {
		close(fd1);
		return false;
	}

	char* FileBuf = NULL;
	uint32_t block;
	off64_t fsizeSrc64 = lseek64(fd1, 0, SEEK_END);
	lseek64(fd1, 0, SEEK_SET);
	if (fsizeSrc64 > 0x7FFFFFF0) { // > 2GB
		uint32_t FileBufSize = FileBufMaxSize;
		FileBuf = initFileBuf(FileBuf, FileBufSize);
		off64_t fsize64 = fsizeSrc64;
		block = FileBufSize;
		//printf("#####[%s] fsizeSrc64: %lld 0x%010llX - large file\n", __FUNCTION__, fsizeSrc64, fsizeSrc64);
		while(fsize64 > 0) {
			if(fsize64 < (off64_t)FileBufSize)
				block = (uint32_t)fsize64;
			read(fd1, FileBuf, block);
			write(fd2, FileBuf, block);
			fsize64 -= block;
			if (!doCopyFlag)
				break;
		}
		if (doCopyFlag) {
			lseek64(fd2, 0, SEEK_SET);
			off64_t fsizeDst64 = lseek64(fd2, 0, SEEK_END);
			if (fsizeSrc64 != fsizeDst64){
				close(fd1);
				close(fd2);
				FileBuf = deleteFileBuf(FileBuf);
				return false;
			}
		}
	}
	else { // < 2GB
		off_t fsizeSrc = lseek(fd1, 0, SEEK_END);
		uint32_t FileBufSize = (fsizeSrc < (off_t)FileBufMaxSize) ? fsizeSrc : FileBufMaxSize;
		FileBuf = initFileBuf(FileBuf, FileBufSize);
		lseek(fd1, 0, SEEK_SET);
		off_t fsize = fsizeSrc;
		block = FileBufSize;
		//printf("#####[%s] fsizeSrc: %ld 0x%08lX - normal file\n", __FUNCTION__, fsizeSrc, fsizeSrc);
		while(fsize > 0) {
			if(fsize < (off_t)FileBufSize)
				block = (uint32_t)fsize;
			read(fd1, FileBuf, block);
			write(fd2, FileBuf, block);
			fsize -= block;
			if (!doCopyFlag)
				break;
		}
		if (doCopyFlag) {
			lseek(fd2, 0, SEEK_SET);
			off_t fsizeDst = lseek(fd2, 0, SEEK_END);
			if (fsizeSrc != fsizeDst){
				close(fd1);
				close(fd2);
				FileBuf = deleteFileBuf(FileBuf);
				return false;
			}
		}
	}
	close(fd1);
	close(fd2);

	if (!doCopyFlag) {
		sync();
		unlink(Dst);
		FileBuf = deleteFileBuf(FileBuf);
		return false;
	}

	FileBuf = deleteFileBuf(FileBuf);
	return true;
}

bool CFileHelpers::copyDir(const char *Src, const char *Dst, bool /*backupMode*/)
{
	DIR *Directory;
	struct dirent *CurrentFile;
	static struct stat FileInfo;
	char srcPath[PATH_MAX];
	char dstPath[PATH_MAX];
	char buf[PATH_MAX];

	//open directory
	if ((Directory = opendir(Src)) == NULL)
		return false;
	if (lstat(Src, &FileInfo) == -1) {
		closedir(Directory);
		return false;
	}
	// create directory
		// is symlink
	if (S_ISLNK(FileInfo.st_mode)) {
		int len = readlink(Src, buf, sizeof(buf)-1);
		if (len != -1) {
			buf[len] = '\0';
			symlink(buf, Dst);
		}
	}
	else {
		// directory
		if (!createDir(Dst, FileInfo.st_mode & 0x0FFF)) {
			closedir(Directory);
			return false;
		}
	}

	// read directory
	while ((CurrentFile = readdir(Directory)) != NULL) {
		// ignore '.' and '..'
		if (strcmp(CurrentFile->d_name, ".") && strcmp(CurrentFile->d_name, "..")) {
			strcpy(srcPath, Src);
			strcat(srcPath, "/");
			strcat(srcPath, CurrentFile->d_name);
			if (lstat(srcPath, &FileInfo) == -1) {
				closedir(Directory);
				return false;
			}
			strcpy(dstPath, Dst);
			strcat(dstPath, "/");
			strcat(dstPath, CurrentFile->d_name);
			// is symlink
			if (S_ISLNK(FileInfo.st_mode)) {
				int len = readlink(srcPath, buf, sizeof(buf)-1);
				if (len != -1) {
					buf[len] = '\0';
					symlink(buf, dstPath);
				}
			}
			// is directory
			else if (S_ISDIR(FileInfo.st_mode)) {
				copyDir(srcPath, dstPath);
			}
			// is file
			else if (S_ISREG(FileInfo.st_mode)) {
				string save = "";
				copyFile(srcPath, (dstPath + save).c_str()); /* mode is set by copyFile */
			}
		}
	}
	closedir(Directory);
	return true;
}

// returns:	 true - success.
//		 false - errno is set
bool CFileHelpers::createDir(string& Dir, mode_t mode)
{
	CFileHelpers* fh = CFileHelpers::getInstance();
	fh->clearDebugInfo();
	int res = 0;
	for(string::iterator iter = Dir.begin() ; iter != Dir.end();) {
		string::iterator newIter = find(iter, Dir.end(), '/' );
		string newPath = string( Dir.begin(), newIter );
		if(!newPath.empty() && !file_exists(newPath.c_str())) {
			res = mkdir( newPath.c_str(), mode);
			if (res == -1) {
				if (errno == EEXIST) {
					res = 0;
				} else {
					// We can assume that if an error
					// occured, following will fail too,
					// so break here.
					if (!fh->getConsoleQuiet())
						printf("[CFileHelpers %s] creating directory %s: %s\n", __func__, newPath.c_str(), strerror(errno));
					char buf[1024];
					memset(buf, '\0', sizeof(buf));
					snprintf(buf, sizeof(buf)-1, "creating directory %s: %s", newPath.c_str(), strerror(errno));
					fh->setDebugInfo(buf, __path_file__, __func__, __LINE__);
					break;
				}
			}
		}
		iter = newIter;
		if(newIter != Dir.end())
			++ iter;
	}

	return (res == 0 ? true : false);
}

bool CFileHelpers::removeDir(const char *Dir)
{
	CFileHelpers* fh = CFileHelpers::getInstance();
	fh->clearDebugInfo();
	DIR *dir;
	struct dirent *entry;
	char path[PATH_MAX];

	dir = opendir(Dir);
	if (dir == NULL) {
		if (errno == ENOENT)
			return true;
		if (!fh->getConsoleQuiet())
			printf("[CFileHelpers %s] remove directory %s: %s\n", __func__, Dir, strerror(errno));
		char buf[1024];
		memset(buf, '\0', sizeof(buf));
		snprintf(buf, sizeof(buf)-1, "remove directory %s: %s", Dir, strerror(errno));
		fh->setDebugInfo(buf, __path_file__, __func__, __LINE__);
		return false;
	}
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
			snprintf(path, (size_t) PATH_MAX, "%s/%s", Dir, entry->d_name);
			if (entry->d_type == DT_DIR)
				removeDir(path);
			else
				unlink(path);
		}
	}
	closedir(dir);
	rmdir(Dir);

	errno = 0;
	return true;
}

u_int64_t CFileHelpers::getDirSize(const char *dirname)
{
	DIR *dir;
	char fullDirName[500];
	struct dirent *dirPnt;
	struct stat cur_file;
	uint64_t total_size = 0;

	//open current dir
	sprintf(fullDirName, "%s/", dirname);
	if((dir = opendir(fullDirName)) == NULL) {
		fprintf(stderr, "Couldn't open %s\n", fullDirName);
		return 0;
	}

	//go through the directory
	while( (dirPnt = readdir(dir)) != NULL ) {
		if(strcmp((*dirPnt).d_name, "..") == 0 || strcmp((*dirPnt).d_name, ".") == 0)
			continue;

		//create current filepath
		sprintf(fullDirName, "%s/%s", dirname, (*dirPnt).d_name);
		if(stat(fullDirName, &cur_file) == -1)
			continue;

		if(cur_file.st_mode & S_IFREG) //file...
			total_size += cur_file.st_size;
		else if(cur_file.st_mode & S_IFDIR) //dir...
			total_size += getDirSize(fullDirName);
	}
	closedir(dir);

	return total_size;
}

