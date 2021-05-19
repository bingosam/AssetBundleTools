#include "StdAfx.h"

#include "../svcdef.h"


/**
 * 构造函数
 *
 * @param pClassDatabaseManager 类数据库管理器
 * @param pAssetsBundleFile 资源包对象
 * @param ppFile 资源包句柄地址
 */
CAssetBundle5::CAssetBundle5(CClassDatabaseManager * pClassDatabaseManager, AssetsBundleFile * pAssetsBundleFile, FILE ** ppFile)
	: CAssetBundle(pClassDatabaseManager, pAssetsBundleFile, ppFile)
{
}

CAssetBundle5::~CAssetBundle5()
{
	m_AssetsBundleFile;
}

/**
 * 获取AssetsFile的数量
 *
 * @param nAssetsFileListIndex 资源文件链表的索引
 */
DWORD CAssetBundle5::GetAssetsFileCount(int nAssetsFileListIndex)
{
	return ((AssetsBundleBlockAndDirectoryList06 *)GetAssetsList(nAssetsFileListIndex))->directoryCount;
}

/**
 * 获取资源文件大小
 *
 * @param pAssetsFileInfo 资源文件信息
 */
QWORD CAssetBundle5::GetAssetsFileSize(void *pAssetsFileInfo)
{
	return ((AssetsBundleDirectoryInfo06 *)pAssetsFileInfo)->decompressedSize;
}

/**
 * 获取资源文件的偏移量
 *
 * @param pAssetsFileInfo 资源文件信息
 */
QWORD CAssetBundle5::GetAssetsFileOffset(void * pAssetsFileInfo)
{
	return ((AssetsBundleDirectoryInfo06 *)pAssetsFileInfo)->offset;
}

/**
 * 获取资源文件的名字
 *
 * @param pAssetsFileInfo 资源文件信息
 */
const char * CAssetBundle5::GetAssetsFileName(void * pAssetsFileInfo)
{
	return ((AssetsBundleDirectoryInfo06 *)pAssetsFileInfo)->name;
}

/**
 * 判断是否为资源文件
 *
 * @param pAssetsFileInfo 资源文件信息
 */
bool CAssetBundle5::IsAssetsFile(void * pAssetsFileInfo)
{
	return m_AssetsBundleFile->IsAssetsFile(AssetsReaderFromFile, (LPARAM)*m_File, (AssetsBundleDirectoryInfo06 *)pAssetsFileInfo);
}

/**
 * 获取指定索引的资源文件列表
 *
 * @param index 资源文件列表索引
 */
void * CAssetBundle5::GetAssetsList(int index)
{
	return m_AssetsBundleFile->bundleInf6 + index;
}

/**
 * 获取资源文件链表中指定索引的资源文件信息
 *
 * @param pAssetsFileList 资源文件链表
 * @param index 资源文件索引
 */
void * CAssetBundle5::GetAssetsFileInfo(void * pAssetsFileList, int index)
{
	return ((AssetsBundleBlockAndDirectoryList06 *)pAssetsFileList)->dirInf + index;
}

/**
 * 判断当前AssetsBundle是否压缩过
 *
 * @return 压缩过返回true,否则false
 */
bool CAssetBundle5::IsCompressed()
{
	bool bResult = (m_AssetsBundleFile->bundleHeader6.flags & 0x3F) != 0;
	if (!bResult)
	{
		for (DWORD i = 0; i < m_AssetsBundleFile->listCount; ++i)
		{
			for (DWORD k = 0; k < m_AssetsBundleFile->bundleInf6[i].blockCount; ++k)
			{
				if ((m_AssetsBundleFile->bundleInf6[i].blockInf[k].flags & 0x3F) != 0)
				{
					bResult = true;
					break;
				}
			}
		}
	}

	return bResult;
}

/**
 * 创建AssetsFileReader
 *
 * @param pAssetsFileInfo AssetsFile文件信息
 */
AssetsFileReader CAssetBundle5::MakeAssetsFileReader(void * pAssetsFileInfo)
{
	return m_AssetsBundleFile->MakeAssetsFileReader(AssetsReaderFromFile, (LPARAM *)m_File, (AssetsBundleDirectoryInfo06 *)pAssetsFileInfo);
}

/**
 * 提取所有AssetsFile
 *
 * @param pAssetsFileList AssetsFile文件信息列表
 * @param pDirectoryPath 提取的资源文件所存放的目录
 * @param pAssetFileSuffix 资源文件后缀,默认为".assets"
 * @return 提取出来的文件名集合
 */
set<string> CAssetBundle5::ExtractAssetsFiles(void * pAssetsFileList, const char * pDirectoryPath, const char * pAssetFileSuffix/* = ".assets"*/)
{
	set<string> Result;
	AssetsBundleBlockAndDirectoryList06 *pList = (AssetsBundleBlockAndDirectoryList06 *)pAssetsFileList;
	for (DWORD i = 0; i < pList->directoryCount; ++i)
	{
		if (!m_AssetsBundleFile->IsAssetsFile(AssetsReaderFromFile, (LPARAM)*m_File, pList->dirInf + i)) continue;
		Result.insert(ExtractAssetsFile(pList->dirInf + i, pDirectoryPath, pAssetFileSuffix));
	}
	return Result;
}
