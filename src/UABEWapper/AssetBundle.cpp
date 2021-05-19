#include <stdafx.h>

#include "../svcdef.h"


/**
 * 构造函数
 *
 * @param pClassDatabaseManager 类数据库管理器
 * @param pAssetsBundleFile 资源包对象
 * @param ppFile 资源包句柄地址
 */
CAssetBundle::CAssetBundle(CClassDatabaseManager * pClassDatabaseManager, AssetsBundleFile * pAssetsBundleFile, FILE ** ppFile)
{
	m_AssetsBundleFile = pAssetsBundleFile;
	m_File = ppFile;
	m_ClassDatabaseManager = pClassDatabaseManager;
	m_AssetsFileReader = NULL;
	m_nAssetsFileIndex = -1;
	m_nAssetsFileListIndex = -1;
}

CAssetBundle::~CAssetBundle()
{
	FreeAssetsFileReader();
}

/**
 * 获取AssetsFile的数量
 *
 * @param nAssetsFileListIndex 资源文件链表的索引
 */
/* virtual */ DWORD CAssetBundle::GetAssetsFileCount(int nAssetsFileListIndex)
{
	return ((AssetsList *)GetAssetsList(nAssetsFileListIndex))->count;
}

/**
 * 获取资源文件大小
 *
 * @param pAssetsFileInfo 资源文件信息
 */
/* virtual */ QWORD CAssetBundle::GetAssetsFileSize(void *pAssetsFileInfo)
{
	return ((AssetsBundleEntry *)pAssetsFileInfo)->length;
}

/**
 * 获取资源文件的偏移量
 *
 * @param pAssetsFileInfo 资源文件信息
 */
/* virtual */ QWORD CAssetBundle::GetAssetsFileOffset(void * pAssetsFileInfo)
{
	return ((AssetsBundleEntry *)pAssetsFileInfo)->offset;
}

/**
 * 获取资源文件的名字
 *
 * @param pAssetsFileInfo 资源文件信息
 */
/* virtual */ const char * CAssetBundle::GetAssetsFileName(void * pAssetsFileInfo)
{
	return ((AssetsBundleEntry *)pAssetsFileInfo)->name;
}

/**
 * 判断是否为资源文件
 *
 * @param pAssetsFileInfo 资源文件信息
 */
/* virtual */ bool CAssetBundle::IsAssetsFile(void * pAssetsFileInfo)
{
	return m_AssetsBundleFile->IsAssetsFile(AssetsReaderFromFile, (LPARAM)*m_File, (AssetsBundleEntry *)pAssetsFileInfo);
}

/**
 * 获取指定索引的资源文件列表
 *
 * @param index 资源文件列表索引
 */
/* virtual */ void * CAssetBundle::GetAssetsList(int index)
{
	return m_AssetsBundleFile->assetsLists3 + index;
}

/**
 * 获取资源文件链表中指定索引的资源文件信息
 *
 * @param pAssetsFileList 资源文件链表
 * @param index 资源文件索引
 */
/* virtual */ void * CAssetBundle::GetAssetsFileInfo(void * pAssetsFileList, int index)
{
	return ((AssetsList *)pAssetsFileList)->ppEntries[index];
}

/**
 * 判断当前AssetsBundle是否压缩过
 *
 * @return 压缩过返回true,否则false
 */
bool CAssetBundle::IsCompressed()
{
	return 0 == strcmp(m_AssetsBundleFile->bundleHeader3.signature, "UnityWeb");
}

/**
 * 创建AssetsFileReader
 *
 * @param pAssetsFileInfo AssetsFile文件信息
 */
/* virtual */ AssetsFileReader CAssetBundle::MakeAssetsFileReader(void * pAssetsFileInfo)
{
	return m_AssetsBundleFile->MakeAssetsFileReader(AssetsReaderFromFile, (LPARAM *)m_File, (AssetsBundleEntry *)pAssetsFileInfo);
}

/**
 * 获取版本号
 */
DWORD CAssetBundle::GetVersion()
{
	return m_AssetsBundleFile->bundleHeader3.fileVersion;
}

/**
 * 提取AssetsFile
 *
 * @param ppFile 资源包文件句柄
 * @param pAssetsFileInfo AssetsFile文件信息
 * @param pDirectoryPath 提取的资源文件所存放的目录
 * @param pAssetFileSuffix 资源文件后缀,默认为".assets"
 * @return 提取出来的文件名
 */
string CAssetBundle::ExtractAssetsFile(void *pAssetsFileInfo, const char *pDirectoryPath, const char *pAssetFileSuffix/* = ".assets"*/)
{
	AssetsFileReader reader = MakeAssetsFileReader(pAssetsFileInfo);
	if (NULL == reader) throw CUnityError(MAKE_ASSETS_FILE_READER_FAILED);

	QWORD ret = 0;
	vector<byte> vBuffer(GetAssetsFileSize(pAssetsFileInfo));
	//FIXME:  pos是0 还是GetAssetsFileOffset(pAssetsFileInfo)
	ret = reader(0, vBuffer.size(), &vBuffer[0], (LPARAM)*m_File);
	if (0 == ret)
	{
		FreeAssetsBundle_FileReader((LPARAM *)m_File, &reader);
		throw CUnityError(ASSETS_DATA_READ_FAILED, "Assets bundle directory name: %s", GetAssetsFileName(pAssetsFileInfo));
	}
	if(vBuffer.size() != ret)
	{
		vBuffer.resize(ret);
	}

	string assetFileName;
	string assetFilePath;

	assetFileName.assign(GetAssetsFileName(pAssetsFileInfo)).append(pAssetFileSuffix);
	assetFilePath.assign(NULL == pDirectoryPath ? "." : pDirectoryPath);
	assetFilePath.append("/").append(assetFileName);
	FILE *fp = fopen(assetFilePath.c_str(), "wb");
	if (NULL == fp)
	{
		FreeAssetsBundle_FileReader((LPARAM *)m_File, &reader);
		throw CUnityError(FILE_OPEN_FAILED);
	}
	ret = AssetsWriterToFile(0, vBuffer.size(), &vBuffer[0], (LPARAM)fp);
	if (vBuffer.size() != ret)
	{
		FreeAssetsBundle_FileReader((LPARAM *)m_File, &reader);
		FREE(fp, fclose);
		throw CUnityError(FILE_WRITE_FAILED, assetFilePath.c_str());
	}

	FreeAssetsBundle_FileReader((LPARAM *)m_File, &reader);
	FREE(fp, fclose);
	return assetFileName;
}

/**
 * 提取所有AssetsFile
 *
 * @param pAssetsFileList AssetsFile文件信息列表
 * @param pDirectoryPath 提取的资源文件所存放的目录
 * @param pAssetFileSuffix 资源文件后缀,默认为".assets"
 * @return 提取出来的文件名集合
 */
set<string> CAssetBundle::ExtractAssetsFiles(void * pAssetsFileList, const char * pDirectoryPath, const char * pAssetFileSuffix/* = ".assets"*/)
{
	set<string> Result;
	AssetsList *pList = (AssetsList *)pAssetsFileList;
	for (DWORD i = 0; i < pList->count; ++i)
	{
		if (!m_AssetsBundleFile->IsAssetsFile(AssetsReaderFromFile, (LPARAM)*m_File, pList->ppEntries[i])) continue;
		Result.insert(ExtractAssetsFile(pList->ppEntries[i], pDirectoryPath, pAssetFileSuffix));
	}
	return Result;
}

/**
 * 提取所有AssetsFile
 *
 * @param pDirectoryPath 提取的资源文件所存放的目录
 * @param pAssetFileSuffix 资源文件后缀,默认为".assets"
 * @return 提取出来的文件名集合
 */
set<string> CAssetBundle::ExtractAssetsFiles(const char * pDirectoryPath, const char * pAssetFileSuffix)
{
	set<string> Result;
	for (DWORD i = 0; i < m_AssetsBundleFile->listCount; ++i)
	{
		set<string> AssetsFiles = ExtractAssetsFiles(GetAssetsList(i), pDirectoryPath, pAssetFileSuffix);
		Result.insert(AssetsFiles.begin(), AssetsFiles.end());
	}
	return Result;
}


/**
 * 从资源包中加载资源文件
 *
 * @return 第一个资源文件
 */
assets_file_ptr CAssetBundle::LoadFirstAssetsFile()
{
	for (m_nAssetsFileListIndex = 0; m_nAssetsFileListIndex < m_AssetsBundleFile->listCount; ++m_nAssetsFileListIndex)
	{
		void *pAssetsFileList = GetAssetsList(m_nAssetsFileListIndex);
		for (m_nAssetsFileIndex = 0; m_nAssetsFileIndex < GetAssetsFileCount(m_nAssetsFileListIndex); ++m_nAssetsFileIndex)
		{
			void *pAssetsFileInfo = GetAssetsFileInfo(pAssetsFileList, m_nAssetsFileIndex);
			if (!IsAssetsFile(pAssetsFileInfo)) continue;
			return LoadAssetsFile(pAssetsFileInfo);
		}
	}
	return assets_file_ptr();
}

/**
 * 加载下一个资源文件
 * @return 下一个资源文件
 */
assets_file_ptr CAssetBundle::LoadNextAssetsFile()
{
	if (0 > m_nAssetsFileIndex) throw runtime_error("You need to load the first assets file first!");
	FreeAssetsFileReader();//必须在加载下个资源之前释放，否则崩溃，不能在LoadAssetsFile下释放

	for (; m_nAssetsFileListIndex < m_AssetsBundleFile->listCount; ++m_nAssetsFileListIndex)
	{
		void *pAssetsFileList = GetAssetsList(m_nAssetsFileListIndex);
		m_nAssetsFileIndex++;
		for (; m_nAssetsFileIndex < GetAssetsFileCount(m_nAssetsFileListIndex); ++m_nAssetsFileIndex)
		{
			void *pAssetsFileInfo = GetAssetsFileInfo(pAssetsFileList, m_nAssetsFileIndex);
			if (!IsAssetsFile(pAssetsFileInfo)) continue;
			return LoadAssetsFile(pAssetsFileInfo);
		}
		m_nAssetsFileIndex = -1;
	}
	return assets_file_ptr();
}

/**
 * 加载AssetsFile
 *
 * @param pAssetsFileName 要加载的AssetsFile名字
 * @return AssetsFile对象
 */
assets_file_ptr CAssetBundle::LoadAssetsFile(const char * pAssetsFileName)
{
	for (m_nAssetsFileListIndex = 0; m_nAssetsFileListIndex < m_AssetsBundleFile->listCount; ++m_nAssetsFileListIndex)
	{
		void *pAssetsFileList = GetAssetsList(m_nAssetsFileListIndex);
		for (m_nAssetsFileIndex = 0; m_nAssetsFileIndex < GetAssetsFileCount(m_nAssetsFileListIndex); ++m_nAssetsFileIndex)
		{
			void *pAssetsFileInfo = GetAssetsFileInfo(pAssetsFileList, m_nAssetsFileIndex);
			if (!IsAssetsFile(pAssetsFileInfo)) continue;
			if (strcmp(GetAssetsFileName(pAssetsFileInfo), pAssetsFileName) != 0) continue;
			return LoadAssetsFile(pAssetsFileInfo);
		}
	}
	return assets_file_ptr();
}

/**
 * 加载资源文件
 *
 * @param pAssetsFileInfo AssetsFile文件信息
 * @return AssetsFile对象
 */
assets_file_ptr CAssetBundle::LoadAssetsFile(void * pAssetsFileInfo)
{
	m_AssetsFileReader = MakeAssetsFileReader(pAssetsFileInfo);
	if (NULL == m_AssetsFileReader) throw CUnityError(MAKE_ASSETS_FILE_READER_FAILED);

	assets_file_ptr AssetsFile(new CAssetsFile(m_ClassDatabaseManager));
	if (!AssetsFile->LoadFromAssetsBundle(m_AssetsFileReader, *m_File))
	{
		FreeAssetsFileReader();
		return assets_file_ptr();
	}
	AssetsFile->SetName(GetAssetsFileName(pAssetsFileInfo));
	AssetsFile->SetSize(GetAssetsFileSize(pAssetsFileInfo));
	return AssetsFile;
}

/**
 * 释放资源文件阅读器
 */
void CAssetBundle::FreeAssetsFileReader()
{
	if (NULL == m_AssetsFileReader) return;

	FreeAssetsBundle_FileReader((LPARAM *)m_File, &m_AssetsFileReader);
	m_AssetsFileReader = NULL;
}
