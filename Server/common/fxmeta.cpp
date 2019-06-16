#include "fxmeta.h"
#include <set>
#include <list>
#include <vector>
#include <string.h>
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include "defines.h"

#ifdef WIN32
#include <Windows.h>
#include <process.h>

#include <psapi.h>
#include <cstddef>
#include <dbghelp.h>
#else
#include <pthread.h>
#include <signal.h>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <netinet/in.h>
#include <execinfo.h>
#include <sys/stat.h>
#endif // WIN32


void ListDir(const char* pDirName, ListDirAndLoadFile* pListDirAndLoadFile)
{
#ifdef WIN32
	char szFile[MAX_PATH] = { 0 };
	char szFind[MAX_PATH];
	char strFileName[MAX_PATH];

	WIN32_FIND_DATA FindFileData;
	string_cpy_s(szFind, MAX_PATH, pDirName);

	string_cat_s(szFind, MAX_PATH - 1, "*.*");
	HANDLE hFind = ::FindFirstFile(szFind, &FindFileData);

	if (INVALID_HANDLE_VALUE == hFind) return;
	while (1)
	{
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (FindFileData.cFileName[0] != '.')
			{
				string_cpy_s(szFile, MAX_PATH, pDirName);
				//strcat(szFile,"//");  
				string_cat_s(szFile, MAX_PATH, FindFileData.cFileName);
				string_cat_s(szFile, MAX_PATH, "//");
				ListDir(szFile, pListDirAndLoadFile);
			}
		}
		else
		{

			string_cpy_s(strFileName, MAX_PATH, pDirName);
			string_cat_s(strFileName, MAX_PATH, FindFileData.cFileName);
			pListDirAndLoadFile->LoadFile(strFileName);
		}


		if (!FindNextFile(hFind, &FindFileData)) break;
	}
	FindClose(hFind);
#else
	DIR* dp;
	struct dirent* pDirp;

	/* open dirent directory */
	if ((dp = opendir(pDirName)) == NULL)
	{
		perror(pDirName);
		return;
	}

	/**
	 * read all files in this dir
	 **/
	while ((pDirp = readdir(dp)) != NULL)
	{
		if (strcmp(pDirp->d_name, ".") == 0 || strcmp(pDirp->d_name, "..") == 0)
		{
			continue;
		}
#if 1
		//有的系统不能用readdir 要用stat
		struct stat buf;
		std::string strPath(pDirName);
		std::string strFileName(pDirp->d_name);
		std::string strFullFilePath = strPath + "/" + strFileName;
		stat(strFullFilePath.c_str(), &buf);
		if (buf.st_mode & S_IFREG)
		{
			std::string strPath(pDirName);
			std::string strFileName(pDirp->d_name);
			std::string strFullFilePath = strPath + "/" + strFileName;
			pListDirAndLoadFile->LoadFile(strFullFilePath.c_str());
		}
		else if (buf.st_mode & S_IFDIR)
		{
			//directory
			std::string strPath(pDirName);
			std::string strDirName(pDirp->d_name);
			std::string strFullDirPath = strPath + "/" + strDirName;
			ListDir(strFullDirPath.c_str(), pListDirAndLoadFile);
		}
		else
		{
			continue;
		}
#else
		switch (pDirp->d_type)
		{
			case DT_REG:
			{
				std::string strPath(pDirName);
				std::string strFileName(pDirp->d_name);
				std::string strFullFilePath = strPath + "/" + strFileName;
				pListDirAndLoadFile->LoadFile(strFullFilePath.c_str());
			}
			break;
			case DT_DIR:
			{
				//directory
				std::string strPath(pDirName);
				std::string strDirName(pDirp->d_name);
				std::string strFullDirPath = strPath + "/" + strDirName;
				ListDir(strFullDirPath.c_str(), pListDirAndLoadFile);
			}
			break;
			default:
			{
				return;
			}
			break;
		}
#endif
	}
#endif
}





