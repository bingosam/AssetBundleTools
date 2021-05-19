#pragma once

/**
 * 文件名过滤回调函数
 *
 * @param pFileName 文件名
 * @return 是否所需
 */
typedef bool (*FILE_NAME_FILTER)(const char *pFileName);

/**
 * 列出文件夹下的文件
 *
 * @param pDirectoryPath 文件夹路径
 * @param bRecursion 是否递归
 * @param pFileNamePattern 文件名格式
 * @param filter 文件名过滤回调函数
 */
set<string> ListDirectory(const char *pDirectoryPath, bool bRecursion = false, const char *pFileNamePattern = "*", FILE_NAME_FILTER filter = NULL);

/**
 * 清理文件夹下的文件
 *
 * @param pDirectoryPath 文件夹路径
 * @param recursion 是否递归
 * @param pFileNamePattern 文件名格式
 * @param filter 文件名过滤回调函数
 */
void ClearDirectory(const char *pDirectoryPath, bool recursion = false, const char *pFileNamePattern = "*", FILE_NAME_FILTER filter = NULL);

/**
 * 创建文件夹
 *
 * @param pPath 路径
 * @param isDirectory pPath是否为文件夹路径，如默认为真
 */
int CreateDirectoryRecursion(const char *pDirectoryPath, bool isDirectory = true);

/**
 * 获取文件大小
 *
 * @param pFilePath 文件路径
 * @return 文件大小
 */
size_t GetFileSize(const char * pFilePath);

/**
 * 拷贝文件,若目标文件所在目录不存在则创建
 *
 * @param pSrcFile 文件路径
 * @param pNewFile 新文件路径
 * @param bFailIfExists 若存在是否返回失败, false时覆盖
 * @return 拷贝是否成功
 */
BOOL __CopyFile(const char * pSrcFile, const char * pNewFile, BOOL bFailIfExists);

/**
 * 拷贝文件夹
 *
 * @param pSrcDir 文件夹路径
 * @param pNewDir 新文件夹路径
 * @param bRecursion 是否递归拷贝
 * @return 拷贝是否成功
 */
BOOL CopyDirectory(const char * pSrcDir, const char * pNewDir, BOOL bRecursion);