#pragma once


typedef enum eASSET_COMPARE_RESULT
{
	ASSET_DIFFERENCE = 0,	//不同资源
	ASSET_THE_SAME,			//同一个资源
	ASSET_EQUAL,			//两个资源数据相同
	ASSET_DIFFERENT_NAME,	//除资源名字外 其他相同
}ASSET_COMPARE_RESULT;

/**
 * 获取比较结果的字符串信息
 *
 * @return result 比较结果
 */
const char * GetAssetCompareResultString(ASSET_COMPARE_RESULT result);

/**
 * 获取比较结果的描述
 *
 * return 比较结果的描述信息
 */
const char * GetAssetCompareResultDesc(ASSET_COMPARE_RESULT result);

/**
 * 获取比较结果的描述
 *
 * return 比较结果的描述信息
 */
const wchar_t * GetAssetCompareResultDescW(ASSET_COMPARE_RESULT result);

/**
 * 资源信息
 */
class CAssetInfo
{
public:

	/**
	 * 构造函数
	 *
	 * @param pName 资源名字
	 * @param uPathID 资源路径ID
	 * @param nType 资源类型
	 * @param type 资源类型字符串
	 * @param nSize 资源大小
	 */
	CAssetInfo(const char *pName, unsigned __int64 uPathID, DWORD nType, string type, DWORD nSize);
	~CAssetInfo();

	/**
	 * 获取路径ID
	 */
	unsigned __int64 GetPathID() const { return m_uPathID; }
	
	/**
	 * 设置路径ID
	 */
	void SetPathID(unsigned __int64 uPathID) { m_uPathID = uPathID; }

	/**
	 * 获取名字
	 */
	string GetName() const { return m_Name; }

	/**
	 * 设置名字
	 */
	void SetName(const string & Name) { m_Name = Name; }

	/**
	 * 获取类型字符串
	 */
	string GetTypeString() const { return m_Type; }

	/**
	 * 设置类型字符串
	 */
	void SetTypeString(const string & Type) { m_Type = Type; }

	/**
	 * 获取MD5值
	 */
	string GetMD5() const { return m_MD5; }

	/**
	 * 设置MD5值
	 */
	void SetMD5(const string & MD5) { m_MD5 = MD5; }

	/**
	 * 获取类型
	 */
	DWORD GetType() const { return m_nType; }

	/**
	 * 设置类型
	 */
	void SetType(DWORD nType) { m_nType = nType; }

	/**
	 * 获取大小
	 */
	DWORD GetSize() const { return m_nSize; }

	/**
	 * 设置大小
	 */
	void SetSize(DWORD nSize) { m_nSize = nSize; }

	/**
	 * 获取真实大小
	 */
	QWORD GetActualSize() { return GetResourceSize() + GetSize(); }

	/**
	 * 获取资源大小
	 */
	QWORD GetResourceSize() { return m_nResourceSize; }

	/**
	 * 设置资源大小
	 */
	void SetResourceSize(QWORD nResourceSize) { m_nResourceSize = nResourceSize; }

	/**
	 * 获取资源文件路径
	 */
	string GetFilePath() { return m_FilePath; }

	/**
	 * 设置资源文件路径
	 */
	void SetFilePath(const string & FilePath) { m_FilePath = FilePath; }

	/**
	 * 获取资源文件名
	 */
	string GetFileName() { return m_FileName; }

	/**
	 * 设置资源文件名
	 */
	void SetFileName(const string & FileName) { m_FileName = FileName; }

	/**
	 * 判断资源是否相等
	 *
	 * @param Other 其他资源
	 */
	bool EqualTo(const CAssetInfo &Other);

	/**
	 * 判断是否为AssetBundle资源
	 */
	bool IsAssetBundleAsset();

	/**
	 * 重载小于号
	 *
	 * @param Other 另一个对象
	 * @return 是否小于Other
	 */
	bool operator<(const CAssetInfo &Other) const;

	/**
	 * 与另一个资源进行比较
	 *
	 * @param Other 其他资源
	 * @return 比较结果 @see ASSET_COMPARE_RESULT
	 */
	ASSET_COMPARE_RESULT Compare(const CAssetInfo &Other);

private:
	/**
	 * 路径ID
	 */
	unsigned __int64 m_uPathID;

	/**
	 * 资源名字
	 */
	string m_Name;

	/**
	 * 类型
	 */
	DWORD m_nType;

	/**
	 * 类型字符串
	 */
	string m_Type;

	/**
	 * 资源信息大小
	 */
	DWORD m_nSize;

	/**
	 * 资源大小
	 */
	QWORD m_nResourceSize;

	/**
	 * md5值
	 */
	string m_MD5;

	/**
	 * 文件路径
	 */
	string m_FilePath;

	/**
	 * 文件名
	 */
	string m_FileName;
};
typedef shared_ptr<CAssetInfo> asset_info_ptr;

/**
 * asset_info_ptr的比较函数
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
 * 资源文件信息
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
	 * 添加资源信息
	 *
	 * @param uPathID 资源ID
	 * @param Asset 资源信息
	 */
	void PutAsset(UINT64 uPathID, const asset_info_ptr &Asset)
	{
		m_Assets[uPathID] = Asset;
	}

	/**
	 * 查找资源信息
	 */
	asset_info_ptr LookupAsset(UINT64 uPathID)
	{
		asset_info_map::iterator It = m_Assets.find(uPathID);
		return It == m_Assets.end() ? asset_info_ptr() : It->second;
	}

	/**
	 * 获取资源信息集合
	 */
	asset_info_map GetAssets() { return m_Assets; }

private:
	/**
	 * 名字
	 *
	 * @note AssetsBundleDirectoryInfo06.name
	 */
	string m_Name;

	/**
	 * 文件名
	 *
	 * @note 文件名
	 */
	string m_FileName;

	/**
	 * AssetBundle名
	 *
	 * @note 从资源包中的AssetBundle资源解析出的m_AssetBundleName
	 */
	string m_AssetBundleName;

	/**
	 * 大小
	 */
	QWORD m_Size;

	/**
	 * 资源文件信息集合
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

