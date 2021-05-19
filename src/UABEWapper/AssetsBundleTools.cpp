#include "StdAfx.h"

#include "../svcdef.h"

/**
 * 构造函数
 *
 * @param pClassDatabaseManager 类数据库管理器
 */
CAssetsBundleTools::CAssetsBundleTools(CClassDatabaseManager * pClassDatabaseManager)
{
	m_AssetsBundleFile = NULL;
	m_UABELogger = NULL;
	m_ABFile = NULL;
	m_ClassDatabaseManager = pClassDatabaseManager;
}

CAssetsBundleTools::~CAssetsBundleTools()
{
	Close();
	m_UABELogger = NULL;
}

/**
 * 读取AssetsBundle文件。
 *
 * @param pABFilePath AssetsBundle文件路径
 * @param allowCompressed 是否允许AssetsBundle文件压缩过
 * @return 读取成功返回true,否则false
 */
bool CAssetsBundleTools::Read(const char * pABFilePath, bool allowCompressed)
{
	Close();
	m_AssetsBundleFile = new AssetsBundleFile();

	m_ABFile = fopen(pABFilePath, "rb");
	if (NULL == m_ABFile)
	{
		return false;
	}

	if (!m_AssetsBundleFile->Read(AssetsReaderFromFile, (LPARAM)m_ABFile, m_UABELogger, allowCompressed))
	{
		fclose(m_ABFile);
		m_ABFile = NULL;
		return false;
	}

	m_AssetBundle = CAssetBundleFactory::CreateAssetBundle(m_ClassDatabaseManager, m_AssetsBundleFile, &m_ABFile);
	return NULL != m_AssetBundle.get();
}

/**
 * 判断当前AssetsBundle是否压缩过
 *
 * @return 压缩过返回true,否则false
 */
bool CAssetsBundleTools::IsCompressed()
{
	return m_AssetBundle->IsCompressed();
}

/**
 * 解压AssetsBundle文件(若压缩过)
 *
 * @param pABFilePath AssetsBundle文件路径
 * @param pFileOutput 输出的文件路径
 * @return 解压结果 @see eERROR_CODE
 */
int CAssetsBundleTools::UnpackIfCompressed(const char * pABFilePath, const char * pFileOutput)
{
	int ret = SUCCESSED;
	do {
		if (!Read(pABFilePath))
		{
			ret = ASSETS_BUNDLE_FILE_READ_FAILED;
			break;
		}

		if (!IsCompressed()) {
			ret = ASSETS_BUNDLE_FILE_NOT_COMPRESSED;
			break;
		}

		FILE *pOutput = fopen(pFileOutput, "wb");
		if (NULL == pOutput) {
			ret = FILE_OPEN_FAILED;
			break;
		}

		if (!m_AssetsBundleFile->Unpack(AssetsReaderFromFile, (LPARAM)m_ABFile, AssetsWriterToFile, (LPARAM)pOutput))
		{
			fclose(pOutput);
			ret = ASSETS_BUNDLE_FILE_UNPACK_FAILED;
			break;
		}
		fclose(pOutput);
	} while (0);
	return ret;
}

/**
 * 关闭
 */
void CAssetsBundleTools::Close()
{
	m_AssetBundle.reset();
	if (NULL != m_AssetsBundleFile)
	{
		m_AssetsBundleFile->Close();
	}
	FREE(m_AssetsBundleFile, delete);
	FREE(m_ABFile, fclose);
}

/**
 * 从解压后的资源包中提取资源文件
 *
 * @param pABFilePath 资源包文件路径
 * @param pDirectoryPath 提取的资源文件所存放的目录
 * @param pAssetFileSuffix 资源文件后缀,默认为".assets"
 * @return 提取出来的资源文件名列表
 */
set<string> CAssetsBundleTools::ExtractAssetsFile(const char *pABFilePath, const char *pDirectoryPath, const char *pAssetFileSuffix/* = ".assets"*/)
{
	set<string> AssetsFileNames;
	if (!Read(pABFilePath, false)) throw CUnityError(ASSETS_BUNDLE_FILE_READ_FAILED);

	if (!CreateDirectoryA(pDirectoryPath, NULL) && ERROR_ALREADY_EXISTS != GetLastError())
	{
		throw COsError("Failed to create directory!");
	}

	return m_AssetBundle->ExtractAssetsFiles(pDirectoryPath, pAssetFileSuffix);
}

/**
 * 提取资源文件到指定目录
 *
 * @param pABFilePath AssetBundle文件路径
 * @param pAssetsFileName AssetBundle文件中的AssetsFile名字
 * @param uiAssetID 资源ID
 * @param pOutputDirPath 资源输出的文件夹路径
 * @return 返回资源文件路径  若失败则为empty
 */
string CAssetsBundleTools::ExportAssetFileTo(const char * pABFilePath, const char * pAssetsFileName, UINT64 uiAssetID, const char * pOutputDirPath)
{
	assets_file_ptr AssetsFile = LoadAssetsFile(pABFilePath, pAssetsFileName);
	if (!AssetsFile.get()) throw CUnityError(ASSETS_FILE_NOT_FOUND);
	return AssetsFile->ExportAssetFileTo(uiAssetID, pOutputDirPath);
}

/**
 * 从资源包中加载资源文件
 *
 * @param pABFilePath 资源包文件路径
 * @return 第一个资源文件
 */
assets_file_ptr CAssetsBundleTools::LoadFirstAssetsFile(const char * pABFilePath)
{
	if (!Read(pABFilePath, false)) throw CUnityError(ASSETS_BUNDLE_FILE_READ_FAILED);
	return m_AssetBundle->LoadFirstAssetsFile();
}

/**
 * 加载下一个资源文件
 * @return 下一个资源文件
 */
assets_file_ptr CAssetsBundleTools::LoadNextAssetsFile()
{
	return m_AssetBundle->LoadNextAssetsFile();
}

/**
 * 加载AssetsFile
 *
 * @param pABFilePath 资源包文件路径
 * @param pAssetsFileName AssetsFile名字
 * @return AssetsFile对象
 */
assets_file_ptr CAssetsBundleTools::LoadAssetsFile(const char * pABFilePath, const char * pAssetsFileName)
{
	if (!Read(pABFilePath, false)) throw CUnityError(ASSETS_BUNDLE_FILE_READ_FAILED);
	return m_AssetBundle->LoadAssetsFile(pAssetsFileName);
}
