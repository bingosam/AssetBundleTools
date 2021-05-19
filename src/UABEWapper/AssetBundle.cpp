#include <stdafx.h>

#include "../svcdef.h"


/**
 * ���캯��
 *
 * @param pClassDatabaseManager �����ݿ������
 * @param pAssetsBundleFile ��Դ������
 * @param ppFile ��Դ�������ַ
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
 * ��ȡAssetsFile������
 *
 * @param nAssetsFileListIndex ��Դ�ļ����������
 */
/* virtual */ DWORD CAssetBundle::GetAssetsFileCount(int nAssetsFileListIndex)
{
	return ((AssetsList *)GetAssetsList(nAssetsFileListIndex))->count;
}

/**
 * ��ȡ��Դ�ļ���С
 *
 * @param pAssetsFileInfo ��Դ�ļ���Ϣ
 */
/* virtual */ QWORD CAssetBundle::GetAssetsFileSize(void *pAssetsFileInfo)
{
	return ((AssetsBundleEntry *)pAssetsFileInfo)->length;
}

/**
 * ��ȡ��Դ�ļ���ƫ����
 *
 * @param pAssetsFileInfo ��Դ�ļ���Ϣ
 */
/* virtual */ QWORD CAssetBundle::GetAssetsFileOffset(void * pAssetsFileInfo)
{
	return ((AssetsBundleEntry *)pAssetsFileInfo)->offset;
}

/**
 * ��ȡ��Դ�ļ�������
 *
 * @param pAssetsFileInfo ��Դ�ļ���Ϣ
 */
/* virtual */ const char * CAssetBundle::GetAssetsFileName(void * pAssetsFileInfo)
{
	return ((AssetsBundleEntry *)pAssetsFileInfo)->name;
}

/**
 * �ж��Ƿ�Ϊ��Դ�ļ�
 *
 * @param pAssetsFileInfo ��Դ�ļ���Ϣ
 */
/* virtual */ bool CAssetBundle::IsAssetsFile(void * pAssetsFileInfo)
{
	return m_AssetsBundleFile->IsAssetsFile(AssetsReaderFromFile, (LPARAM)*m_File, (AssetsBundleEntry *)pAssetsFileInfo);
}

/**
 * ��ȡָ����������Դ�ļ��б�
 *
 * @param index ��Դ�ļ��б�����
 */
/* virtual */ void * CAssetBundle::GetAssetsList(int index)
{
	return m_AssetsBundleFile->assetsLists3 + index;
}

/**
 * ��ȡ��Դ�ļ�������ָ����������Դ�ļ���Ϣ
 *
 * @param pAssetsFileList ��Դ�ļ�����
 * @param index ��Դ�ļ�����
 */
/* virtual */ void * CAssetBundle::GetAssetsFileInfo(void * pAssetsFileList, int index)
{
	return ((AssetsList *)pAssetsFileList)->ppEntries[index];
}

/**
 * �жϵ�ǰAssetsBundle�Ƿ�ѹ����
 *
 * @return ѹ��������true,����false
 */
bool CAssetBundle::IsCompressed()
{
	return 0 == strcmp(m_AssetsBundleFile->bundleHeader3.signature, "UnityWeb");
}

/**
 * ����AssetsFileReader
 *
 * @param pAssetsFileInfo AssetsFile�ļ���Ϣ
 */
/* virtual */ AssetsFileReader CAssetBundle::MakeAssetsFileReader(void * pAssetsFileInfo)
{
	return m_AssetsBundleFile->MakeAssetsFileReader(AssetsReaderFromFile, (LPARAM *)m_File, (AssetsBundleEntry *)pAssetsFileInfo);
}

/**
 * ��ȡ�汾��
 */
DWORD CAssetBundle::GetVersion()
{
	return m_AssetsBundleFile->bundleHeader3.fileVersion;
}

/**
 * ��ȡAssetsFile
 *
 * @param ppFile ��Դ���ļ����
 * @param pAssetsFileInfo AssetsFile�ļ���Ϣ
 * @param pDirectoryPath ��ȡ����Դ�ļ�����ŵ�Ŀ¼
 * @param pAssetFileSuffix ��Դ�ļ���׺,Ĭ��Ϊ".assets"
 * @return ��ȡ�������ļ���
 */
string CAssetBundle::ExtractAssetsFile(void *pAssetsFileInfo, const char *pDirectoryPath, const char *pAssetFileSuffix/* = ".assets"*/)
{
	AssetsFileReader reader = MakeAssetsFileReader(pAssetsFileInfo);
	if (NULL == reader) throw CUnityError(MAKE_ASSETS_FILE_READER_FAILED);

	QWORD ret = 0;
	vector<byte> vBuffer(GetAssetsFileSize(pAssetsFileInfo));
	//FIXME:  pos��0 ����GetAssetsFileOffset(pAssetsFileInfo)
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
 * ��ȡ����AssetsFile
 *
 * @param pAssetsFileList AssetsFile�ļ���Ϣ�б�
 * @param pDirectoryPath ��ȡ����Դ�ļ�����ŵ�Ŀ¼
 * @param pAssetFileSuffix ��Դ�ļ���׺,Ĭ��Ϊ".assets"
 * @return ��ȡ�������ļ�������
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
 * ��ȡ����AssetsFile
 *
 * @param pDirectoryPath ��ȡ����Դ�ļ�����ŵ�Ŀ¼
 * @param pAssetFileSuffix ��Դ�ļ���׺,Ĭ��Ϊ".assets"
 * @return ��ȡ�������ļ�������
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
 * ����Դ���м�����Դ�ļ�
 *
 * @return ��һ����Դ�ļ�
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
 * ������һ����Դ�ļ�
 * @return ��һ����Դ�ļ�
 */
assets_file_ptr CAssetBundle::LoadNextAssetsFile()
{
	if (0 > m_nAssetsFileIndex) throw runtime_error("You need to load the first assets file first!");
	FreeAssetsFileReader();//�����ڼ����¸���Դ֮ǰ�ͷţ����������������LoadAssetsFile���ͷ�

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
 * ����AssetsFile
 *
 * @param pAssetsFileName Ҫ���ص�AssetsFile����
 * @return AssetsFile����
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
 * ������Դ�ļ�
 *
 * @param pAssetsFileInfo AssetsFile�ļ���Ϣ
 * @return AssetsFile����
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
 * �ͷ���Դ�ļ��Ķ���
 */
void CAssetBundle::FreeAssetsFileReader()
{
	if (NULL == m_AssetsFileReader) return;

	FreeAssetsBundle_FileReader((LPARAM *)m_File, &m_AssetsFileReader);
	m_AssetsFileReader = NULL;
}
