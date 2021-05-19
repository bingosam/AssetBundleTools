#include "StdAfx.h"

#include "../svcdef.h"


/**
 * ���캯��
 *
 * @param pClassDatabaseManager �����ݿ������
 * @param pAssetsBundleFile ��Դ������
 * @param ppFile ��Դ�������ַ
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
 * ��ȡAssetsFile������
 *
 * @param nAssetsFileListIndex ��Դ�ļ����������
 */
DWORD CAssetBundle5::GetAssetsFileCount(int nAssetsFileListIndex)
{
	return ((AssetsBundleBlockAndDirectoryList06 *)GetAssetsList(nAssetsFileListIndex))->directoryCount;
}

/**
 * ��ȡ��Դ�ļ���С
 *
 * @param pAssetsFileInfo ��Դ�ļ���Ϣ
 */
QWORD CAssetBundle5::GetAssetsFileSize(void *pAssetsFileInfo)
{
	return ((AssetsBundleDirectoryInfo06 *)pAssetsFileInfo)->decompressedSize;
}

/**
 * ��ȡ��Դ�ļ���ƫ����
 *
 * @param pAssetsFileInfo ��Դ�ļ���Ϣ
 */
QWORD CAssetBundle5::GetAssetsFileOffset(void * pAssetsFileInfo)
{
	return ((AssetsBundleDirectoryInfo06 *)pAssetsFileInfo)->offset;
}

/**
 * ��ȡ��Դ�ļ�������
 *
 * @param pAssetsFileInfo ��Դ�ļ���Ϣ
 */
const char * CAssetBundle5::GetAssetsFileName(void * pAssetsFileInfo)
{
	return ((AssetsBundleDirectoryInfo06 *)pAssetsFileInfo)->name;
}

/**
 * �ж��Ƿ�Ϊ��Դ�ļ�
 *
 * @param pAssetsFileInfo ��Դ�ļ���Ϣ
 */
bool CAssetBundle5::IsAssetsFile(void * pAssetsFileInfo)
{
	return m_AssetsBundleFile->IsAssetsFile(AssetsReaderFromFile, (LPARAM)*m_File, (AssetsBundleDirectoryInfo06 *)pAssetsFileInfo);
}

/**
 * ��ȡָ����������Դ�ļ��б�
 *
 * @param index ��Դ�ļ��б�����
 */
void * CAssetBundle5::GetAssetsList(int index)
{
	return m_AssetsBundleFile->bundleInf6 + index;
}

/**
 * ��ȡ��Դ�ļ�������ָ����������Դ�ļ���Ϣ
 *
 * @param pAssetsFileList ��Դ�ļ�����
 * @param index ��Դ�ļ�����
 */
void * CAssetBundle5::GetAssetsFileInfo(void * pAssetsFileList, int index)
{
	return ((AssetsBundleBlockAndDirectoryList06 *)pAssetsFileList)->dirInf + index;
}

/**
 * �жϵ�ǰAssetsBundle�Ƿ�ѹ����
 *
 * @return ѹ��������true,����false
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
 * ����AssetsFileReader
 *
 * @param pAssetsFileInfo AssetsFile�ļ���Ϣ
 */
AssetsFileReader CAssetBundle5::MakeAssetsFileReader(void * pAssetsFileInfo)
{
	return m_AssetsBundleFile->MakeAssetsFileReader(AssetsReaderFromFile, (LPARAM *)m_File, (AssetsBundleDirectoryInfo06 *)pAssetsFileInfo);
}

/**
 * ��ȡ����AssetsFile
 *
 * @param pAssetsFileList AssetsFile�ļ���Ϣ�б�
 * @param pDirectoryPath ��ȡ����Դ�ļ�����ŵ�Ŀ¼
 * @param pAssetFileSuffix ��Դ�ļ���׺,Ĭ��Ϊ".assets"
 * @return ��ȡ�������ļ�������
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
