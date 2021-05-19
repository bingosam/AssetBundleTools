#pragma once

/**
 * ����Դ��������ص�����
 *
 * @author ������
 */

/**
 * ��Դ������
 * ֧��Unity 5.3���°汾
 */
class CAssetBundle
{
public:
	/**
	 * ���캯��
	 *
	 * @param pClassDatabaseManager �����ݿ������
	 * @param pAssetsBundleFile ��Դ������
	 * @param ppFile ��Դ�������ַ
	 */
	CAssetBundle(CClassDatabaseManager * pClassDatabaseManager, AssetsBundleFile * pAssetsBundleFile, FILE ** ppFile);
	~CAssetBundle();

	/**
	 * ��ȡAssetsFile������
	 *
	 * @param nAssetsFileListIndex ��Դ�ļ����������
	 */
	virtual DWORD GetAssetsFileCount(int nAssetsFileListIndex);

	/**
	 * ��ȡ��Դ�ļ���С
	 *
	 * @param pAssetsFileInfo ��Դ�ļ���Ϣ
	 */
	virtual QWORD GetAssetsFileSize(void * pAssetsFileInfo);

	/**
	 * ��ȡ��Դ�ļ���ƫ����
	 *
	 * @param pAssetsFileInfo ��Դ�ļ���Ϣ
	 */
	virtual QWORD GetAssetsFileOffset(void * pAssetsFileInfo);

	/**
	 * ��ȡ��Դ�ļ�������
	 *
	 * @param pAssetsFileInfo ��Դ�ļ���Ϣ
	 */
	virtual const char * GetAssetsFileName(void * pAssetsFileInfo);

	/**
	 * �ж��Ƿ�Ϊ��Դ�ļ�
	 *
	 * @param pAssetsFileInfo ��Դ�ļ���Ϣ
	 */
	virtual bool IsAssetsFile(void * pAssetsFileInfo);

	/**
	 * ��ȡָ����������Դ�ļ��б�
	 *
	 * @param index ��Դ�ļ��б�����
	 */
	virtual void * GetAssetsList(int index);

	/**
	 * ��ȡ��Դ�ļ�������ָ����������Դ�ļ���Ϣ
	 *
	 * @param pAssetsFileList ��Դ�ļ�����
	 * @param index ��Դ�ļ�����
	 */
	virtual void * GetAssetsFileInfo(void * pAssetsFileList, int index);

	/**
	 * �жϵ�ǰAssetsBundle�Ƿ�ѹ����
	 *
	 * @return ѹ��������true,����false
	 */
	virtual bool IsCompressed();

	/**
	 * ����AssetsFileReader
	 *
	 * @param pAssetsFileInfo AssetsFile�ļ���Ϣ
	 */
	virtual AssetsFileReader MakeAssetsFileReader(void * pAssetsFileInfo);

	/**
	 * ��ȡ�汾��
	 */
	DWORD GetVersion();

	/**
	 * ��ȡAssetsFile
	 *
	 * @param pAssetsFileInfo AssetsFile�ļ���Ϣ
	 * @param pDirectoryPath ��ȡ����Դ�ļ�����ŵ�Ŀ¼
	 * @param pAssetFileSuffix ��Դ�ļ���׺,Ĭ��Ϊ".assets"
	 * @return ��ȡ�������ļ���
	 */
	string ExtractAssetsFile(void * pAssetsFileInfo, const char * pDirectoryPath, const char * pAssetFileSuffix = ".assets");
	
	/**
	 * ��ȡ����AssetsFile
	 *
	 * @param pAssetsFileList AssetsFile�ļ���Ϣ�б�
	 * @param pDirectoryPath ��ȡ����Դ�ļ�����ŵ�Ŀ¼
	 * @param pAssetFileSuffix ��Դ�ļ���׺,Ĭ��Ϊ".assets"
	 * @return ��ȡ�������ļ�������
	 */
	virtual set<string> ExtractAssetsFiles(void * pAssetsFileList, const char * pDirectoryPath, const char * pAssetFileSuffix = ".assets");

	/**
	 * ��ȡ����AssetsFile
	 *
	 * @param pDirectoryPath ��ȡ����Դ�ļ�����ŵ�Ŀ¼
	 * @param pAssetFileSuffix ��Դ�ļ���׺,Ĭ��Ϊ".assets"
	 * @return ��ȡ�������ļ�������
	 */
	set<string> ExtractAssetsFiles(const char * pDirectoryPath, const char * pAssetFileSuffix = ".assets");

	/**
	 * ����Դ���м�����Դ�ļ�
	 *
	 * @return ��һ����Դ�ļ�
	 */
	assets_file_ptr LoadFirstAssetsFile();

	/**
	 * ������һ����Դ�ļ�
	 * @return ��һ����Դ�ļ�
	 */
	assets_file_ptr LoadNextAssetsFile();

	/**
	 * ����AssetsFile
	 *
	 * @param pAssetsFileName Ҫ���ص�AssetsFile����
	 * @return AssetsFile����
	 */
	assets_file_ptr LoadAssetsFile(const char * pAssetsFileName);

protected:
	/**
	 * ������Դ�ļ�
	 *
	 * @param pAssetsFileInfo AssetsFile�ļ���Ϣ
	 * @return AssetsFile����
	 */
	assets_file_ptr LoadAssetsFile(void * pAssetsFileInfo);

protected:
	/**
	 * ��Դ������
	 */
	AssetsBundleFile * m_AssetsBundleFile;

	/**
	 * ��Դ�ļ������ַ
	 */
	FILE ** m_File;

	/**
	 * �����ݿ������
	 */
	CClassDatabaseManager * m_ClassDatabaseManager;

	/**
	 * ��ǰ��Դ�ļ�����
	 */
	DWORD m_nAssetsFileIndex;

	/**
	 * ��ǰ��Դ�ļ���������
	 */
	DWORD m_nAssetsFileListIndex;

private:

	/**
	 * ��Դ�ļ��Ķ���
	 */
	AssetsFileReader m_AssetsFileReader;

	/**
	 * �ͷ���Դ�ļ��Ķ���
	 */
	void FreeAssetsFileReader();
};
typedef shared_ptr<CAssetBundle> asset_bundle_ptr;