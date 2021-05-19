#include "stdafx.h"

#include "FileUtils.h"

/**
 * 列出文件夹下的文件
 *
 * @param pDirectoryPath 文件夹路径
 * @param bRecursion 是否递归
 * @param pFileNamePattern 文件名格式
 * @param filter 文件名过滤回调函数
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
 * 清理文件夹下的文件
 *
 * @param pDirectoryPath 文件夹路径
 * @param recursion 是否递归
 * @param pFileNamePattern 文件名格式
 * @param filter 文件名过滤回调函数
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
 * 创建文件夹
 *
 * @param pPath 路径
 * @param isDirectory pPath是否为文件夹路径，默认为真
 * @return 返回错误码
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
 * 获取文件大小
 *
 * @param pFilePath 文件路径
 * @return 文件大小
 */
size_t GetFileSize(const char * pFilePath)
{
	HANDLE hFile = CreateFileA(pFilePath, FILE_READ_EA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) throw COsError();

	size_t nFileSize = GetFileSize(hFile, NULL); // 获得文件大小
	if ((DWORD)nFileSize == INVALID_FILE_SIZE) throw COsError();
	CloseHandle(hFile);
	return nFileSize;
}

/**
 * 拷贝文件,若目标文件所在目录不存在则创建
 *
 * @param pSrcFile 文件路径
 * @param pNewFile 新文件路径
 */
BOOL __CopyFile(const char * pSrcFile, const char * pNewFile, BOOL bFailIfExists)
{
	if (0 != CreateDirectoryRecursion(pNewFile, false)) return FALSE;
	return CopyFileA(pSrcFile, pNewFile, bFailIfExists);
}

/**
 * 拷贝文件夹
 *
 * @param pSrcDir 文件夹路径
 * @param pNewDir 新文件夹路径
 * @param bRecursion 是否递归拷贝
 * @return 拷贝是否成功
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
