#include "StdAfx.h"

#include "../svcdef.h"

/**
 * ���캯��
 *
 * @param pClassDatabaseManager �����ݿ������
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
 * ��ȡAssetsBundle�ļ���
 *
 * @param pABFilePath AssetsBundle�ļ�·��
 * @param allowCompressed �Ƿ�����AssetsBundle�ļ�ѹ����
 * @return ��ȡ�ɹ�����true,����false
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
 * �жϵ�ǰAssetsBundle�Ƿ�ѹ����
 *
 * @return ѹ��������true,����false
 */
bool CAssetsBundleTools::IsCompressed()
{
	return m_AssetBundle->IsCompressed();
}

/**
 * ��ѹAssetsBundle�ļ�(��ѹ����)
 *
 * @param pABFilePath AssetsBundle�ļ�·��
 * @param pFileOutput ������ļ�·��
 * @return ��ѹ��� @see eERROR_CODE
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
 * �ر�
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
 * �ӽ�ѹ�����Դ������ȡ��Դ�ļ�
 *
 * @param pABFilePath ��Դ���ļ�·��
 * @param pDirectoryPath ��ȡ����Դ�ļ�����ŵ�Ŀ¼
 * @param pAssetFileSuffix ��Դ�ļ���׺,Ĭ��Ϊ".assets"
 * @return ��ȡ��������Դ�ļ����б�
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
 * ��ȡ��Դ�ļ���ָ��Ŀ¼
 *
 * @param pABFilePath AssetBundle�ļ�·��
 * @param pAssetsFileName AssetBundle�ļ��е�AssetsFile����
 * @param uiAssetID ��ԴID
 * @param pOutputDirPath ��Դ������ļ���·��
 * @return ������Դ�ļ�·��  ��ʧ����Ϊempty
 */
string CAssetsBundleTools::ExportAssetFileTo(const char * pABFilePath, const char * pAssetsFileName, UINT64 uiAssetID, const char * pOutputDirPath)
{
	assets_file_ptr AssetsFile = LoadAssetsFile(pABFilePath, pAssetsFileName);
	if (!AssetsFile.get()) throw CUnityError(ASSETS_FILE_NOT_FOUND);
	return AssetsFile->ExportAssetFileTo(uiAssetID, pOutputDirPath);
}

/**
 * ����Դ���м�����Դ�ļ�
 *
 * @param pABFilePath ��Դ���ļ�·��
 * @return ��һ����Դ�ļ�
 */
assets_file_ptr CAssetsBundleTools::LoadFirstAssetsFile(const char * pABFilePath)
{
	if (!Read(pABFilePath, false)) throw CUnityError(ASSETS_BUNDLE_FILE_READ_FAILED);
	return m_AssetBundle->LoadFirstAssetsFile();
}

/**
 * ������һ����Դ�ļ�
 * @return ��һ����Դ�ļ�
 */
assets_file_ptr CAssetsBundleTools::LoadNextAssetsFile()
{
	return m_AssetBundle->LoadNextAssetsFile();
}

/**
 * ����AssetsFile
 *
 * @param pABFilePath ��Դ���ļ�·��
 * @param pAssetsFileName AssetsFile����
 * @return AssetsFile����
 */
assets_file_ptr CAssetsBundleTools::LoadAssetsFile(const char * pABFilePath, const char * pAssetsFileName)
{
	if (!Read(pABFilePath, false)) throw CUnityError(ASSETS_BUNDLE_FILE_READ_FAILED);
	return m_AssetBundle->LoadAssetsFile(pAssetsFileName);
}
