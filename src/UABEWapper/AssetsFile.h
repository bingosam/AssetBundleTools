#pragma once


class CAssetsFile
{
public:

	/**
	 * ���캯��
	 *
	 * @param pClassDatabaseManager �����ݿ������
	 */
	CAssetsFile(CClassDatabaseManager *pClassDatabaseManager);
	~CAssetsFile();

	/**
	 * ��ȡ��Դ�ļ�ʵ��
	 */
	AssetsFile * GetAssetsFile() { return m_AssetsFile; }

	/**
	 * ��ȡ��Դ�ļ����ʵ��
	 */
	AssetsFileTable * GetAssetsFileTable() { return m_AssetsFileTable; }

	/**
	 * ��ȡ��Դ�ļ���Ϣ������
	 */
	unsigned int GetAssetFileInfoCount() { return m_AssetsFileTable->assetFileInfoCount; }

	/**
	 * ͨ��������ȡ��Դ�ļ���Ϣ
	 */
	AssetFileInfoEx *GetAssetFileInfoByIndex(unsigned int nIndex) { return m_AssetsFileTable->pAssetFileInfo + nIndex; }

	/**
	 * ��ȡ��Դ����
	 */
	asset_info_map GetAssets() { return m_Info->GetAssets(); }

	/**
	 * ����pathID������Դ
	 */
	asset_info_ptr LookupAsset(QWORD uPathID)
	{
		m_Info->LookupAsset(uPathID);
	}

	/**
	 * ��ȡ��Ϣ
	 */
	assets_file_info_ptr GetInformation() { return m_Info; }

	/**
	 * ��ȡ����
	 */
	string GetName() { return m_Info->GetName(); }

	/**
	 * ��������
	 */
	void SetName(const string &name) { m_Info->SetName(name); }

	/**
	 * ��ȡ�ļ���
	 */
	string GetFileName() { return m_Info->GetFileName(); }

	/**
	 * �����ļ���
	 *
	 * @param FileName �ļ���
	 */
	void SetFileName(string FileName) { m_Info->SetFileName(FileName); }

	/**
	 * ��ȡ��С
	 */
	QWORD GetSize() { return m_Info->GetSize(); }

	/**
	 * ���ô�С
	 *
	 * @param nSize ��С
	 */
	void SetSize(QWORD nSize) { m_Info->SetSize(nSize); }

	/**
	 * ������Դ�ļ�
	 *
	 * @param pAssetsFilePath ��Դ�ļ�·��
	 */
	bool Load(const char *pAssetsFilePath);

	/**
	 * ���ڴ��м�����Դ�ļ�
	 *
	 * @param reader ��Դ�ļ���ȡ��,ͨ��MakeAssetsFileReader���
	 * @param fpAssetsBundleFile ��Դ���ļ�ʵ��
	 */
	bool LoadFromAssetsBundle(AssetsFileReader reader, FILE *fpAssetsBundleFile);

	/**
	 * ��ȡ�����б�
	 */
	set<string> GetDependencies() { return m_Dependencies; }

	/**
	 * �ر���Դ�ļ����ͷ���Դ
	 */
	void Close();

	/**
	 * ������Դ����ģ��
	 *
	 * @param pAssetTypeTemplateField ��Դ����ģ��
	 * @param pAssetsFile ��Դ�ļ�����
	 * @param type ����
	 * @return ���سɹ����
	 */
	bool LoadAssetTypeTemplateField(AssetTypeTemplateField * pAssetTypeTemplateField, DWORD type);

	/**
	 * �������ݿ��м�����Դ����ģ��
	 *
	 * @param ppAssetTypeTemplateField ��Դ����ģ��
	 * @param pAssetsFile ��Դ�ļ�����
	 * @param type ����
	 * @return ���سɹ����
	 */
	bool LoadAssetTypeTemplateFieldFromClassDatabase(AssetTypeTemplateField * pAssetTypeTemplateField, DWORD type);

	/**
	 * ��ȡ��Դ��Ϣ
	 *
	 * @param name ��Դ��
	 * @return ��Դ��Ϣָ��
	 */
	AssetFileInfoEx *GetAssetInfo(const char *name);

	/**
	 * ��ȡ��Դ��Ϣ
	 *
	 * @param name ��Դ��
	 * @param type ��Դ����
	 * @return ��Դ��Ϣָ��
	 */
	AssetFileInfoEx *GetAssetInfo(const char *name, DWORD type);

	/**
	 * ��ȡ��Դ��Ϣ
	 *
	 * @param pathId ��ԴID
	 * @return ��Դ��Ϣָ��
	 */
	AssetFileInfoEx *GetAssetInfo(QWORD pathId);

	/**
	 * �ж��Ƿ���manifest��Դ��
	 */
	bool IsAssetBundleManifest();

	/**
	 * ��ȡ��ΪAssetBundle����Դ
	 */
	AssetFileInfoEx *GetAssetBundleAsset();

	/**
	 * ������Դ��Ϣ
	 *
	 * @param ��Դ���Ŀ¼
	 */
	void LoadAssetInfos(const char * pOutputDirPath = NULL);

	/**
	 * ��ȡ��Դ�ļ���ָ��Ŀ¼
	 *
	 * @param uiAssetID ��ԴID
	 * @param pOutputDirPath ����ļ���·��
	 * @return ������Դ�ļ�·��  ��ʧ����Ϊempty
	 */
	string ExportAssetFileTo(UINT64 uiAssetID, const char * pOutputDirPath);
private:

	/**
	 * ��Դ�ļ����
	 */
	FILE *m_File;

	/**
	 * ��Դ�ļ���
	 */
	AssetsFileTable * m_AssetsFileTable;

	/**
	 * ��Դ�ļ�
	 */
	AssetsFile *m_AssetsFile;

	/**
	 * �Ƿ����Դ���м���
	 */
	bool m_IsLoadFromAssetsBundle;

	/**
	 * �����ݿ������
	 */
	CClassDatabaseManager *m_ClassDatabaseManager;

	/**
	 * ��Դ������
	 */
	string m_AssetBundleName;

	/**
	 * ������Ϣ
	 */
	set<string> m_Dependencies;

	/**
	 * ���ع���Ϣ
	 */
	bool m_LoadInformation;

	/**
	 * ��Դ�ļ���Ϣ
	 */
	assets_file_info_ptr m_Info;

	/**
	 * ������Դ�ļ�
	 *
	 * @param reader ��Դ�ļ���ȡ��
	 * @param fpAssetsBundleFile ��Դ���ļ�ʵ��
	 */
	bool Load(AssetsFileReader reader, FILE * fpAssetsBundleFile);

	/**
	 * ����MD5
	 *
	 * @param pAssetFileInfo ��Դ�ļ���Ϣ
	 * @param pAssetTypeValueField ��Դ����
	 */
	string MakeMD5(AssetFileInfoEx *pAssetFileInfo, AssetTypeValueField *pAssetTypeValueField);

	/**
	 * ��������
	 */
	void ParseDependencies();

	/**
	 * ��ȡ��Դ����
	 *
	 * @param bFindName[in,out] �Ƿ��ҵ���Դ�����ֶ�
	 * @param pAssetTypeValueField
	 * @return ��Դ����
	 */
	string GetAssetData(bool & bFindName, AssetTypeValueField * pAssetTypeValueField);
	
	/**
	 * ��ȡ��Դ
	 *
	 * @param AssetInfo ��Դ��Ϣ
	 * @param pBaseTypeValue ��Դ����
	 * @param pOutputDirPath ���Ŀ¼
	 */
	void ExportAsset(const asset_info_ptr & AssetInfo, AssetTypeValueField *pBaseTypeValue, const char * pOutputDirPath);

	/**
	 * ��ȡ��Դ
	 *
	 * @param AssetInfo ��Դ��Ϣ
	 * @param pBaseTypeValue ��Դ����
	 * @param pOutputDirPath ���Ŀ¼
	 */
	void DoNotExportAsset(const asset_info_ptr & AssetInfo, AssetTypeValueField *pBaseTypeValue, const char * pOutputDirPath);
};
typedef shared_ptr<CAssetsFile> assets_file_ptr;

