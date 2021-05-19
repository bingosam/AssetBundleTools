#include "stdafx.h"

#include "FileUtils.h"

/**
 * �г��ļ����µ��ļ�
 *
 * @param pDirectoryPath �ļ���·��
 * @param bRecursion �Ƿ�ݹ�
 * @param pFileNamePattern �ļ�����ʽ
 * @param filter �ļ������˻ص�����
 */
set<string> ListDirectory(const char * pDirectoryPath, bool bRecursion/* = false */, const char * pFileNamePattern/* = "*" */, FILE_NAME_FILTER filter/* = NULL */)
{
	set<string> result;
	if (NULL == pDirectoryPath) return result;

	string Pattern(pDirectoryPath);
	Pattern.append("\\").append(pFileNamePattern);
	WIN32_FIND_DATAA FindData;
	HANDLE hFind = FindFirstFileA(Pattern.c_str(), &FindData);
	if (INVALID_HANDLE_VALUE != hFind)
	{
		try
		{
			do
			{
				if (HAS_BITS(FindData.dwFileAttributes, FILE_ATTRIBUTE_HIDDEN)) continue;
				if (HAS_BITS(FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY))
				{
					if (!bRecursion || strcmp(FindData.cFileName, ".") == 0 || strcmp(FindData.cFileName, "..") == 0) continue;
					set<string> sub = ListDirectory(string(pDirectoryPath).append("\\").append(FindData.cFileName).c_str(), true, pFileNamePattern, filter);
					result.insert(sub.begin(), sub.end());
				}
				else if(NULL == filter || filter(FindData.cFileName))
				{
					result.insert(string(pDirectoryPath).append("\\").append(FindData.cFileName));
				}
			} while (FindNextFileA(hFind, &FindData));
			FindClose(hFind);
		}
		catch (...)
		{
			FindClose(hFind);
			throw;
		}
	}

	return result;
}

/**
 * �����ļ����µ��ļ�
 *
 * @param pDirectoryPath �ļ���·��
 * @param recursion �Ƿ�ݹ�
 * @param pFileNamePattern �ļ�����ʽ
 * @param filter �ļ������˻ص�����
 */
void ClearDirectory(const char * pDirectoryPath, bool recursion, const char * pFileNamePattern, FILE_NAME_FILTER filter)
{
	set<string> Files = ListDirectory(pDirectoryPath, recursion, pFileNamePattern, filter);
	for (set<string>::iterator It = Files.begin(); It != Files.end(); ++It)
	{
		DeleteFileA((*It).c_str());
	}
}

/**
 * �����ļ���
 *
 * @param pPath ·��
 * @param isDirectory pPath�Ƿ�Ϊ�ļ���·����Ĭ��Ϊ��
 * @return ���ش�����
 */
int CreateDirectoryRecursion(const char * pPath, bool isDirectory/* = true*/)
{
	if (NULL == pPath) return 0;
	if (ACCESS(pPath, 0) == 0) return 0;

	char * pszDir = strdup(pPath);
	size_t nLen = strlen(pszDir);
	int iError = 0;

	for (int i = 0; i < nLen; ++i)
	{
		if (pszDir[i] == '\\' || pszDir[i] == '/')
		{
			pszDir[i] = '\0';
			if (ACCESS(pszDir, 0) != 0)
			{
				iError = MKDIR(pszDir);
				int nError = GetOsError();
				if (0 != iError && nError != ERROR_ALREADY_EXISTS)
				{
					free(pszDir);
					return iError;
				}
			}
			pszDir[i] = '/';
		}
	}

	if (isDirectory)
	{
		if (ACCESS(pszDir, 0) != 0)
		{
			iError = MKDIR(pszDir);
			int nError = GetOsError();
			if (0 != iError && nError == ERROR_ALREADY_EXISTS)
			{
				iError = 0;
			}
		}
	}
	free(pszDir);
	return iError;
}

/**
 * ��ȡ�ļ���С
 *
 * @param pFilePath �ļ�·��
 * @return �ļ���С
 */
size_t GetFileSize(const char * pFilePath)
{
	HANDLE hFile = CreateFileA(pFilePath, FILE_READ_EA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) throw COsError();

	size_t nFileSize = GetFileSize(hFile, NULL); // ����ļ���С
	if ((DWORD)nFileSize == INVALID_FILE_SIZE) throw COsError();
	CloseHandle(hFile);
	return nFileSize;
}

/**
 * �����ļ�,��Ŀ���ļ�����Ŀ¼�������򴴽�
 *
 * @param pSrcFile �ļ�·��
 * @param pNewFile ���ļ�·��
 */
BOOL __CopyFile(const char * pSrcFile, const char * pNewFile, BOOL bFailIfExists)
{
	if (0 != CreateDirectoryRecursion(pNewFile, false)) return FALSE;
	return CopyFileA(pSrcFile, pNewFile, bFailIfExists);
}

/**
 * �����ļ���
 *
 * @param pSrcDir �ļ���·��
 * @param pNewDir ���ļ���·��
 * @param bRecursion �Ƿ�ݹ鿽��
 * @return �����Ƿ�ɹ�
 */
BOOL CopyDirectory(const char * pSrcDir, const char * pNewDir, BOOL bRecursion)
{
	string Pattern(pSrcDir);
	Pattern.append("\\*.*");
	WIN32_FIND_DATAA FindData;
	HANDLE hFind = FindFirstFileA(Pattern.c_str(), &FindData);
	if (INVALID_HANDLE_VALUE != hFind)
	{
		try
		{
			do
			{
				string srcPath(pSrcDir);
				srcPath.append("\\").append(FindData.cFileName);
				string newPath(pNewDir);
				newPath.append("\\").append(FindData.cFileName);
				if (HAS_BITS(FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY))
				{
					if (!bRecursion || strcmp(FindData.cFileName, ".") == 0 || strcmp(FindData.cFileName, "..") == 0) continue;
					if (!CopyDirectory(srcPath.c_str(), newPath.c_str(), TRUE)) return FALSE;
				}
				else
				{
					if (!__CopyFile(srcPath.c_str(), newPath.c_str(), false)) return FALSE;
				}
			} while (FindNextFileA(hFind, &FindData));
			FindClose(hFind);
		}
		catch (...)
		{
			FindClose(hFind);
			throw;
		}
		return TRUE;
	}
	return FALSE;
}
