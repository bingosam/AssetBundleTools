#pragma once


typedef enum eASSET_COMPARE_RESULT
{
	ASSET_DIFFERENCE = 0,	//��ͬ��Դ
	ASSET_THE_SAME,			//ͬһ����Դ
	ASSET_EQUAL,			//������Դ������ͬ
	ASSET_DIFFERENT_NAME,	//����Դ������ ������ͬ
}ASSET_COMPARE_RESULT;

/**
 * ��ȡ�ȽϽ�����ַ�����Ϣ
 *
 * @return result �ȽϽ��
 */
const char * GetAssetCompareResultString(ASSET_COMPARE_RESULT result);

/**
 * ��ȡ�ȽϽ��������
 *
 * return �ȽϽ����������Ϣ
 */
const char * GetAssetCompareResultDesc(ASSET_COMPARE_RESULT result);

/**
 * ��ȡ�ȽϽ��������
 *
 * return �ȽϽ����������Ϣ
 */
const wchar_t * GetAssetCompareResultDescW(ASSET_COMPARE_RESULT result);

/**
 * ��Դ��Ϣ
 */
class CAssetInfo
{
public:

	/**
	 * ���캯��
	 *
	 * @param pName ��Դ����
	 * @param uPathID ��Դ·��ID
	 * @param nType ��Դ����
	 * @param type ��Դ�����ַ���
	 * @param nSize ��Դ��С
	 */
	CAssetInfo(const char *pName, unsigned __int64 uPathID, DWORD nType, string type, DWORD nSize);
	~CAssetInfo();

	/**
	 * ��ȡ·��ID
	 */
	unsigned __int64 GetPathID() const { return m_uPathID; }
	
	/**
	 * ����·��ID
	 */
	void SetPathID(unsigned __int64 uPathID) { m_uPathID = uPathID; }

	/**
	 * ��ȡ����
	 */
	string GetName() const { return m_Name; }

	/**
	 * ��������
	 */
	void SetName(const string & Name) { m_Name = Name; }

	/**
	 * ��ȡ�����ַ���
	 */
	string GetTypeString() const { return m_Type; }

	/**
	 * ���������ַ���
	 */
	void SetTypeString(const string & Type) { m_Type = Type; }

	/**
	 * ��ȡMD5ֵ
	 */
	string GetMD5() const { return m_MD5; }

	/**
	 * ����MD5ֵ
	 */
	void SetMD5(const string & MD5) { m_MD5 = MD5; }

	/**
	 * ��ȡ����
	 */
	DWORD GetType() const { return m_nType; }

	/**
	 * ��������
	 */
	void SetType(DWORD nType) { m_nType = nType; }

	/**
	 * ��ȡ��С
	 */
	DWORD GetSize() const { return m_nSize; }

	/**
	 * ���ô�С
	 */
	void SetSize(DWORD nSize) { m_nSize = nSize; }

	/**
	 * ��ȡ��ʵ��С
	 */
	QWORD GetActualSize() { return GetResourceSize() + GetSize(); }

	/**
	 * ��ȡ��Դ��С
	 */
	QWORD GetResourceSize() { return m_nResourceSize; }

	/**
	 * ������Դ��С
	 */
	void SetResourceSize(QWORD nResourceSize) { m_nResourceSize = nResourceSize; }

	/**
	 * ��ȡ��Դ�ļ�·��
	 */
	string GetFilePath() { return m_FilePath; }

	/**
	 * ������Դ�ļ�·��
	 */
	void SetFilePath(const string & FilePath) { m_FilePath = FilePath; }

	/**
	 * ��ȡ��Դ�ļ���
	 */
	string GetFileName() { return m_FileName; }

	/**
	 * ������Դ�ļ���
	 */
	void SetFileName(const string & FileName) { m_FileName = FileName; }

	/**
	 * �ж���Դ�Ƿ����
	 *
	 * @param Other ������Դ
	 */
	bool EqualTo(const CAssetInfo &Other);

	/**
	 * �ж��Ƿ�ΪAssetBundle��Դ
	 */
	bool IsAssetBundleAsset();

	/**
	 * ����С�ں�
	 *
	 * @param Other ��һ������
	 * @return �Ƿ�С��Other
	 */
	bool operator<(const CAssetInfo &Other) const;

	/**
	 * ����һ����Դ���бȽ�
	 *
	 * @param Other ������Դ
	 * @return �ȽϽ�� @see ASSET_COMPARE_RESULT
	 */
	ASSET_COMPARE_RESULT Compare(const CAssetInfo &Other);

private:
	/**
	 * ·��ID
	 */
	unsigned __int64 m_uPathID;

	/**
	 * ��Դ����
	 */
	string m_Name;

	/**
	 * ����
	 */
	DWORD m_nType;

	/**
	 * �����ַ���
	 */
	string m_Type;

	/**
	 * ��Դ��Ϣ��С
	 */
	DWORD m_nSize;

	/**
	 * ��Դ��С
	 */
	QWORD m_nResourceSize;

	/**
	 * md5ֵ
	 */
	string m_MD5;

	/**
	 * �ļ�·��
	 */
	string m_FilePath;

	/**
	 * �ļ���
	 */
	string m_FileName;
};
typedef shared_ptr<CAssetInfo> asset_info_ptr;

/**
 * asset_info_ptr�ıȽϺ���
 */
struct asset_info_less : public binary_function<asset_info_ptr, asset_info_ptr, bool>
{
	bool operator()(asset_info_ptr const &left, asset_info_ptr const &right) const
	{
		return (*left) < (*right);
	}
};

typedef map<UINT64, asset_info_ptr> asset_info_map;

/**
 * ��Դ�ļ���Ϣ
 */
class CAssetsFileInfo
{
public:
	CAssetsFileInfo();
	~CAssetsFileInfo();

	string GetFileName() { return m_FileName; }
	void SetFileName(const string &FileName) { m_FileName = FileName; }

	string GetName() { return m_Name; }
	void SetName(const string &Name) { m_Name = Name; }

	string GetAssetBundleName() { return m_AssetBundleName; }
	void SetAssetBundleName(const string &AssetBundleName) { m_AssetBundleName = AssetBundleName; }

	QWORD GetSize() { return m_Size; }
	void SetSize(QWORD nSize) { m_Size = nSize; }

	/**
	 * �����Դ��Ϣ
	 *
	 * @param uPathID ��ԴID
	 * @param Asset ��Դ��Ϣ
	 */
	void PutAsset(UINT64 uPathID, const asset_info_ptr &Asset)
	{
		m_Assets[uPathID] = Asset;
	}

	/**
	 * ������Դ��Ϣ
	 */
	asset_info_ptr LookupAsset(UINT64 uPathID)
	{
		asset_info_map::iterator It = m_Assets.find(uPathID);
		return It == m_Assets.end() ? asset_info_ptr() : It->second;
	}

	/**
	 * ��ȡ��Դ��Ϣ����
	 */
	asset_info_map GetAssets() { return m_Assets; }

private:
	/**
	 * ����
	 *
	 * @note AssetsBundleDirectoryInfo06.name
	 */
	string m_Name;

	/**
	 * �ļ���
	 *
	 * @note �ļ���
	 */
	string m_FileName;

	/**
	 * AssetBundle��
	 *
	 * @note ����Դ���е�AssetBundle��Դ��������m_AssetBundleName
	 */
	string m_AssetBundleName;

	/**
	 * ��С
	 */
	QWORD m_Size;

	/**
	 * ��Դ�ļ���Ϣ����
	 */
	asset_info_map m_Assets;
};
typedef shared_ptr<CAssetsFileInfo> assets_file_info_ptr;

class CAssetBundleInfo
{
public:
	CAssetBundleInfo();
	~CAssetBundleInfo();
private:

};

