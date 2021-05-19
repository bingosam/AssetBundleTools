#pragma once
/**
 * ����Դ��������ص�����
 */

class CAssetsBundleTools
{
public:
	/**
	 * ���캯��
	 *
	 * @param pClassDatabaseManager �����ݿ������
	 */
	CAssetsBundleTools(CClassDatabaseManager * pClassDatabaseManager);

	/**
	 * ��������
	 */
	~CAssetsBundleTools();

	/**
	 * ��ȡAssetsBundleFile
	 */
	AssetsBundleFile * GetAssetsBundleFile() { return m_AssetsBundleFile; }

	/**
	 * ��ȡ�ļ����
	 */
	FILE * GetFile() { return m_ABFile; }

	/**
	 * ��ȡ��Դ���汾
	 */
	DWORD GetVersion() { return GetAssetsBundleFile()->bundleHeader3.fileVersion; }

	/**
	 * ����UABE��־��¼����
	 *
	 * @param logger ��־����ص�����
	 */
	void SetUABELogger(AssetsFileVerifyLogger logger) { this->m_UABELogger = logger; }

	/**
	 * ��ȡAssetsBundle�ļ���
	 *
	 * @param pABFilePath AssetsBundle�ļ�·��
	 * @param allowCompressed �Ƿ�����AssetsBundle�ļ�ѹ����
	 * @return ��ȡ�ɹ�����true,����false
	 */
	bool Read(const char *pABFilePath, bool allowCompressed = true);
	
	/**
	 * �жϵ�ǰAssetsBundle�Ƿ�ѹ����
	 *
	 * @return ѹ��������true,����false
	 */
	bool IsCompressed();

	/**
	 * ��ѹAssetsBundle�ļ�(��ѹ����)
	 *
	 * @param pABFilePath AssetsBundle�ļ�·��
	 * @param pFileOutput ������ļ�·��
	 * @return ��ѹ��� @see eERROR_CODE
	 */
	int UnpackIfCompressed(const char *pABFilePath, const char *pFileOutput);

	/**
	 * �ر�
	 */
	void Close();

	/**
	 * �ӽ�ѹ�����Դ������ȡ��Դ�ļ�
	 *
	 * @param pABFilePath ��Դ���ļ�·��
	 * @param pDirectoryPath ��ȡ����Դ�ļ�����ŵ�Ŀ¼
	 * @param pAssetFileSuffix ��Դ�ļ���׺,Ĭ��Ϊ".assets"
	 * @return ��ȡ��������Դ�ļ����б�
	 */
	set<string> ExtractAssetsFile(const char *pABFilePath, const char *pDirectoryPath, const char *pAssetFileSuffix = ".assets");

	/**
	 * ��ȡ��Դ�ļ���ָ��Ŀ¼
	 *
	 * @param pABFilePath AssetBundle�ļ�·��
	 * @param pAssetsFileName AssetBundle�ļ��е�AssetsFile����
	 * @param uiAssetID ��ԴID
	 * @param pOutputDirPath ��Դ������ļ���·��
	 * @return ������Դ�ļ�·��  ��ʧ����Ϊempty
	 */
	string ExportAssetFileTo(const char * pABFilePath, const char * pAssetsFileName, UINT64 uiAssetID, const char * pOutputDirPath);

	/**
	 * ����Դ���м�����Դ�ļ�
	 *
	 * @param pABFilePath ��Դ���ļ�·��
	 * @return ��һ����Դ�ļ�
	 */
	assets_file_ptr LoadFirstAssetsFile(const char *pABFilePath);

	/**
	 * ������һ����Դ�ļ�
	 * @return ��һ����Դ�ļ�
	 */
	assets_file_ptr LoadNextAssetsFile();

	/**
	 * ����AssetsFile
	 *
	 * @param pABFilePath ��Դ���ļ�·��
	 * @param pAssetsFileName AssetsFile����
	 * @return AssetsFile����
	 */
	assets_file_ptr LoadAssetsFile(const char * pABFilePath, const char * pAssetsFileName);

private:
	/**
	 * ��Դ���ļ�ʵ��
	 */
	AssetsBundleFile * m_AssetsBundleFile;

	/**
	 * AssetsBundle�ļ����
	 */
	FILE *m_ABFile;

	/**
	 * �ṩ��UABE����־��¼��
	 */
	AssetsFileVerifyLogger m_UABELogger;

	/**
	 * �����ݿ������
	 */
	CClassDatabaseManager *m_ClassDatabaseManager;

	/**
	 * AssetBundle����
	 */
	asset_bundle_ptr m_AssetBundle;
};

typedef shared_ptr<CAssetsBundleTools> ab_file_ptr;

