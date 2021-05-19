#pragma once


class CAssetsFile
{
public:

	/**
	 * 构造函数
	 *
	 * @param pClassDatabaseManager 类数据库管理器
	 */
	CAssetsFile(CClassDatabaseManager *pClassDatabaseManager);
	~CAssetsFile();

	/**
	 * 获取资源文件实例
	 */
	AssetsFile * GetAssetsFile() { return m_AssetsFile; }

	/**
	 * 获取资源文件表的实例
	 */
	AssetsFileTable * GetAssetsFileTable() { return m_AssetsFileTable; }

	/**
	 * 获取资源文件信息的数量
	 */
	unsigned int GetAssetFileInfoCount() { return m_AssetsFileTable->assetFileInfoCount; }

	/**
	 * 通过索引获取资源文件信息
	 */
	AssetFileInfoEx *GetAssetFileInfoByIndex(unsigned int nIndex) { return m_AssetsFileTable->pAssetFileInfo + nIndex; }

	/**
	 * 获取资源集合
	 */
	asset_info_map GetAssets() { return m_Info->GetAssets(); }

	/**
	 * 根据pathID查找资源
	 */
	asset_info_ptr LookupAsset(QWORD uPathID)
	{
		m_Info->LookupAsset(uPathID);
	}

	/**
	 * 获取信息
	 */
	assets_file_info_ptr GetInformation() { return m_Info; }

	/**
	 * 获取名字
	 */
	string GetName() { return m_Info->GetName(); }

	/**
	 * 设置名字
	 */
	void SetName(const string &name) { m_Info->SetName(name); }

	/**
	 * 获取文件名
	 */
	string GetFileName() { return m_Info->GetFileName(); }

	/**
	 * 设置文件名
	 *
	 * @param FileName 文件名
	 */
	void SetFileName(string FileName) { m_Info->SetFileName(FileName); }

	/**
	 * 获取大小
	 */
	QWORD GetSize() { return m_Info->GetSize(); }

	/**
	 * 设置大小
	 *
	 * @param nSize 大小
	 */
	void SetSize(QWORD nSize) { m_Info->SetSize(nSize); }

	/**
	 * 加载资源文件
	 *
	 * @param pAssetsFilePath 资源文件路径
	 */
	bool Load(const char *pAssetsFilePath);

	/**
	 * 从内存中加载资源文件
	 *
	 * @param reader 资源文件读取器,通过MakeAssetsFileReader获得
	 * @param fpAssetsBundleFile 资源包文件实例
	 */
	bool LoadFromAssetsBundle(AssetsFileReader reader, FILE *fpAssetsBundleFile);

	/**
	 * 获取依赖列表
	 */
	set<string> GetDependencies() { return m_Dependencies; }

	/**
	 * 关闭资源文件并释放资源
	 */
	void Close();

	/**
	 * 加载资源类型模版
	 *
	 * @param pAssetTypeTemplateField 资源类型模版
	 * @param pAssetsFile 资源文件对象
	 * @param type 类型
	 * @return 加载成功与否
	 */
	bool LoadAssetTypeTemplateField(AssetTypeTemplateField * pAssetTypeTemplateField, DWORD type);

	/**
	 * 从类数据库中加载资源类型模版
	 *
	 * @param ppAssetTypeTemplateField 资源类型模版
	 * @param pAssetsFile 资源文件对象
	 * @param type 类型
	 * @return 加载成功与否
	 */
	bool LoadAssetTypeTemplateFieldFromClassDatabase(AssetTypeTemplateField * pAssetTypeTemplateField, DWORD type);

	/**
	 * 获取资源信息
	 *
	 * @param name 资源名
	 * @return 资源信息指针
	 */
	AssetFileInfoEx *GetAssetInfo(const char *name);

	/**
	 * 获取资源信息
	 *
	 * @param name 资源名
	 * @param type 资源类型
	 * @return 资源信息指针
	 */
	AssetFileInfoEx *GetAssetInfo(const char *name, DWORD type);

	/**
	 * 获取资源信息
	 *
	 * @param pathId 资源ID
	 * @return 资源信息指针
	 */
	AssetFileInfoEx *GetAssetInfo(QWORD pathId);

	/**
	 * 判断是否是manifest资源包
	 */
	bool IsAssetBundleManifest();

	/**
	 * 获取名为AssetBundle的资源
	 */
	AssetFileInfoEx *GetAssetBundleAsset();

	/**
	 * 加载资源信息
	 *
	 * @param 资源输出目录
	 */
	void LoadAssetInfos(const char * pOutputDirPath = NULL);

	/**
	 * 提取资源文件到指定目录
	 *
	 * @param uiAssetID 资源ID
	 * @param pOutputDirPath 输出文件夹路径
	 * @return 返回资源文件路径  若失败则为empty
	 */
	string ExportAssetFileTo(UINT64 uiAssetID, const char * pOutputDirPath);
private:

	/**
	 * 资源文件句柄
	 */
	FILE *m_File;

	/**
	 * 资源文件表
	 */
	AssetsFileTable * m_AssetsFileTable;

	/**
	 * 资源文件
	 */
	AssetsFile *m_AssetsFile;

	/**
	 * 是否从资源包中加载
	 */
	bool m_IsLoadFromAssetsBundle;

	/**
	 * 类数据库管理器
	 */
	CClassDatabaseManager *m_ClassDatabaseManager;

	/**
	 * 资源包名字
	 */
	string m_AssetBundleName;

	/**
	 * 依赖信息
	 */
	set<string> m_Dependencies;

	/**
	 * 加载过信息
	 */
	bool m_LoadInformation;

	/**
	 * 资源文件信息
	 */
	assets_file_info_ptr m_Info;

	/**
	 * 加载资源文件
	 *
	 * @param reader 资源文件读取器
	 * @param fpAssetsBundleFile 资源包文件实例
	 */
	bool Load(AssetsFileReader reader, FILE * fpAssetsBundleFile);

	/**
	 * 生成MD5
	 *
	 * @param pAssetFileInfo 资源文件信息
	 * @param pAssetTypeValueField 资源属性
	 */
	string MakeMD5(AssetFileInfoEx *pAssetFileInfo, AssetTypeValueField *pAssetTypeValueField);

	/**
	 * 解析依赖
	 */
	void ParseDependencies();

	/**
	 * 获取资源数据
	 *
	 * @param bFindName[in,out] 是否找到资源名字字段
	 * @param pAssetTypeValueField
	 * @return 资源数据
	 */
	string GetAssetData(bool & bFindName, AssetTypeValueField * pAssetTypeValueField);
	
	/**
	 * 提取资源
	 *
	 * @param AssetInfo 资源信息
	 * @param pBaseTypeValue 资源数据
	 * @param pOutputDirPath 输出目录
	 */
	void ExportAsset(const asset_info_ptr & AssetInfo, AssetTypeValueField *pBaseTypeValue, const char * pOutputDirPath);

	/**
	 * 提取资源
	 *
	 * @param AssetInfo 资源信息
	 * @param pBaseTypeValue 资源数据
	 * @param pOutputDirPath 输出目录
	 */
	void DoNotExportAsset(const asset_info_ptr & AssetInfo, AssetTypeValueField *pBaseTypeValue, const char * pOutputDirPath);
};
typedef shared_ptr<CAssetsFile> assets_file_ptr;

