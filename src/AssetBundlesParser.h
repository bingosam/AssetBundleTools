#pragma once

#include <fstream>

/**
 * 资源文件信息集合
 *
 * @key string 读取自AssetsBundleDirectoryInfo06->name "CAB-XXXX"
 * @value assets_file_info_ptr 资源文件信息
 */
typedef map<string, assets_file_info_ptr, str_less_incase> assets_file_info_map;

/**
 * 资源文件依赖
 *
 * @key 资源包名字 CAB-XXX
 * @value 资源包名集合 CAB-XXX
 */
typedef map<string, set<string>, str_less_incase> dependencies_map;

/**
 * 资源包集合
 *
 * @key 资源包文件名
 * @value 资源文件名集合 CAB-XXX
 */
typedef map<string, set<string>> asset_bundle_map;

/**
 * 重复的资源包集合
 *
 * @key 资源包名字 CAB-XXX
 * @value 资源包文件名集合
 */
typedef map<string, set<string>, str_less_incase> duplicate_asset_bundle_map;

struct ASSET_DETAIL_INFO {
	asset_info_ptr m_AssetInfo;
	string m_OwnerName;
	string m_OwnerFileName;
	string m_OwnerAssetBundleName;

	bool operator < (const ASSET_DETAIL_INFO &Other) const
	{
		return (*m_AssetInfo) < (*Other.m_AssetInfo);
	}
};
typedef shared_ptr<ASSET_DETAIL_INFO> asset_detail_info_ptr;

bool inline asset_detail_info_compare(asset_detail_info_ptr &Left, asset_detail_info_ptr &Right)
{
	return (*Left) < (*Right);
}

class CAssetBundlesParser
{
public:
	CAssetBundlesParser(const char * pOutputDirPath, bool bExportAssetFile = false);
	~CAssetBundlesParser();

	const TCHAR *GetName() const { return _T("Asset Bundle Parser"); }

	/**
	 * 解压指定文件夹下的所有资源包文件
	 *
	 * @param pDirectoryPath 存放资源包的文件夹路径
	 */
	void UnpackAssetsBundleFiles(const char *pDirectoryPath);

	/**
	 * 解压AssetBundle文件
	 *
	 * @param AssetBundle AssetBundle工具类
	 * @param AssetBundleFile AssetBundle文件路径
	 * @return 成功返回解压后的文件路径, 否则返回空
	 */
	string UnpackAssetBundleFile(const ab_file_ptr & AssetBundle, const string & AssetBundleFile);

	/**
	 * 加载所有资源包信息
	 */
	void LoadAssetBundles();

	/**
	 * 加载AssetBundle
	 *
	 * @param AssetBundle AssetBundle工具类
	 * @param AssetBundleFile AssetBundle文件路径
	 * @param DecompressedFile AssetBundle解压后的文件路径
	 */
	void LoadAssetBundle(const ab_file_ptr & AssetBundle, const string & AssetBundleFile, const string & DecompressedFile);

	/**
	 * 获取指定AssetBundle提取的资源的存放路径
	 */
	string GetAssetsDirPath(const string & AssetBundleFile);

	/**
	 * 将资源文件绝对路径转化成相对路径
	 *
	 * @param AbsAssetPath 资源绝对路径
	 * @return 相对路径
	 */
	string ConvertAssetPathToRelativePath(const string & AbsAssetPath);

	/**
	 * 释放资源
	 */
	void Release();

	/**
	 * 清理缓存
	 */
	void Clear();

	/**
	 * 资源解析
	 *
	 * @return 返回输出文件路径
	 */
	string Analyze();

	/**
	 * 生成报告页面
	 *
	 * @return 返回输出文件路径
	 */
	string GenerateReport();

	/**
	 * 解析目标文件夹下的AssetBundle
	 *
	 * @param pDirectoryPath AssetBundle所在目录
	 * @return 返回解析后的结果文件
	 */
	string Parse(const char *pDirectoryPath);

	/**
	 * 查找资源文件信息
	 *
	 * @param AssetBundleName 资源包名字
	 */
	assets_file_info_ptr LookupAssetsFileInfo(const string &AssetBundleName);

	/**
	 * 查找依赖信息
	 *
	 * @param AssetBundleName 资源包名字
	 */
	set<string> LookupDependencies(const string &AssetBundleName);
	
	/**
	 * 查找被依赖信息
	 *
	 * @param AssetBundleName 资源包名字
	 */
	set<string> LookupBeDepended(const string &AssetBundleName);

	/**
	 * 查找重复的资源包文件
	 *
	 * @param AssetBundleName 资源包名字
	 */
	set<string> LookupDuplicateAssetBundle(const string &AssetBundleName);

	/**
	 * 查找指定AssetBundle中的AssetsFile
	 *
	 * @param AssetBundleFile AssetBundle文件路径
	 */
	set<string> LookupAssetsFile(const string & AssetBundleFile);

	/**
	 * 添加AssetsFile
	 *
	 * @param AssetBundleFile AssetBundle文件路径
	 * @param AssetsFiles AssesFile集合
	 */
	void PutAssetsFile(const string & AssetBundleFile, const set<string> & AssetsFiles);

	/**
	 * 查找解压后的AssetBundle文件路径
	 */
	string LookupDecompressedAssetBundleFile(const string & AssetBundleFile);

	/**
	 * 添加解压后的AssetBundle文件路径
	 */
	void PutDecompressedAssetBundleFile(const string & AssetBundleFile, const string & DecompressedAssetBundle);

	/**
	 * 添加大小
	 *
	 * @param nDecompressedSize 解压后大小
	 * @param nCompressedSize 压缩后大小                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          
	 */
	void AddSize(size_t nDecompressedSize, size_t nCompressedSize);

private:
	class CWorkThread : public CThread
	{
	public:
		CWorkThread(CAssetBundlesParser * pOwner);
		virtual ~CWorkThread();
		
		/**
		 * 添加AssetBundle文件路径
		 *
		 * @param AssetBundleFile AssetBundle文件路径
		 */
		void AddAssetBundleFile(const string & AssetBundleFile);

		/**
		 * @copydoc CThread::Run
		 */
		virtual void Run();
	private:
		ab_file_ptr m_AssetsBundle;

		CAssetBundlesParser * m_pOwner;

		vector<string> m_AssetBundleFiles;
	};
	typedef shared_ptr<CWorkThread> work_thread_ptr;

	/**
	 * html文件生成器
	 */
	class CHtmlGenerator{
	public:
		/**
		 * 构造函数
		 *
		 * @param pOwner 所属的解析器
		 */
		CHtmlGenerator(CAssetBundlesParser * pOwner);

		/**
		 * 析构函数
		 */
		~CHtmlGenerator();
		
		/**
		 * 添加冗余资源
		 *
		 * @param AssetType 资源类型
		 * @param ReduAssets 冗余资源队列
		 */
		void PutReduAssets(string AssetType, const list<asset_detail_info_ptr> & ReduAssets);

		/**
		 * 添加依赖
		 *
		 * @param Name 资源包名（可能是ABName、FileName、Name）
		 * @param AssetsFileInfo AssetsFile信息
		 */
		void PutDependency(string Name, const assets_file_info_ptr & AssetsFileInfo);

		/**
		 * 添加资源
		 *
		 * @param AssetBundleName  资源所属的AssetBundle名
		 * @param AssetInfo 资源信息
		 */
		void PutAsset(string AssetBundleName, const asset_info_ptr & AssetInfo);

		/**
		 * 生成html文件
		 *
		 * @param FilePath 文件路径
		 */
		void Generate(const string & FilePath);

	private:
		typedef multimap<wstring, list<asset_detail_info_ptr>> redu_assets_map;
		typedef map<string, list<assets_file_info_ptr>> deps_map;
		typedef map<string, multimap<string, asset_info_ptr>> assets_map;

		CAssetBundlesParser * m_pOwner;

		wofstream m_Stream;

		/**
		 * 存放冗余资源的集合
		 *
		 * @key 资源类型
		 * @value 冗余资源队列
		 */
		redu_assets_map m_ReduAssets;

		/**
		 * 默认选择的冗余资源
		 */
		wstring m_DefaultSelected;

		set<wstring> m_ReduAssetTypes;

		/**
		 * 缺少的依赖包
		 */
		set<string> m_MissingDependencies;

		/**
		 * 依赖集合
		 */
		deps_map m_Dependencies;

		/**
		 * 冗余资源大小
		 */
		UINT64 m_uiReduAssetsSize;

		/**
		 * 资源集合
		 */
		assets_map m_Assets;

		/**
		 * 初始化文件头
		 */
		void InitHead();

		/**
		 * 添加html body
		 */
		void AppendBody();

		/**
		 * 添加摘要
		 */
		void AppendDigest();

		/**
		 * 添加冗余信息
		 */
		void AppendRedus();

		/**
		 * 添加依赖信息
		 */
		void AppendDependencies();

		/**
		 * 添加资源信息
		 */
		void AppendAssetsInfo();

		/**
		 * 添加脚本
		 */
		void AppendScript();

		/**
		 * 添加冗余资源选择脚本
		 */
		void AppendReduAssetsSelectScript();

		/**
		 * 添加冗余资源队列到文件流
		 *
		 * @param Assets 资源队列
		 */
		void AppendReduAssets(list<asset_detail_info_ptr> Asset);

		/**
		 * 添加冗余资源到文件流
		 *
		 * @param Asset 资源信息
		 */
		void AppendReduAsset(const asset_detail_info_ptr & Asset, int nRowSpan = 0);

		/**
		 * 单位换算
		 *
		 * @param uiByteSize 字节数
		 * @return 换算后的数据
		 */
		string ConvertUnitA(UINT64 uiByteSize);

		/**
		 * 单位换算
		 *
		 * @param uiByteSize 字节数
		 * @return 换算后的数据
		 */
		wstring ConvertUnitW(UINT64 uiByteSize);
	};
	typedef shared_ptr<CHtmlGenerator> html_generator_ptr;

	/**
	 * 资源包
	 */
	ab_file_ptr m_AssetsBundle;

	/**
	 * 存放资源包的文件夹路径
	 */
	string m_DirPath;

	/**
	 * 资源包文件路径集合
	 *
	 * @key 解压前的AssetBundle文件路径
	 * @param 解压后的AssetBundle文件路径
	 */
	map<string, string> m_AssetsBundleFilePathes;

	/**
	 * 操作资源包文件路径集合的互斥对象
	 */
	CMutex m_ABPathesMutex;

	/**
	 * 资源包集合
	 *
	 * @key AssetBundle文件名
	 * @value AssetBundle中解析出来的资源包集合
	 */
	asset_bundle_map m_AssetBundles;

	/**
	 * 重复的资源包集合
	 *
	 * @key 资源包名
	 * @value 包含有该资源包名的AssetBundle文件名集合
	 */
	duplicate_asset_bundle_map m_DuplicateAssetbundles;

	assets_file_info_map m_AssetsFileInfos;

	/**
	 * 依赖集合
	 *
	 * @key 资源包名
	 * @value 依赖的资源包集合
	 */
	dependencies_map m_Dependencies;

	/**
	 * 输出目录
	 */
	string m_OutputDirPath;

	/**
	 * 解压后的大小
	 */
	size_t m_nDecompressedSize;

	/**
	 * 冗余资源大小
	 */
	size_t m_nReduAssetsSize;

	/**
	 * 解压前的大小
	 */
	size_t m_nCompressedSize;

	/**
	 * 资源导出目录
	 */
	string m_AssetExportDir;
	
	/**
	 * 操作资源文件信息的互斥对象
	 */
	CMutex m_AssetsFileInfosMutex;

	/**
	 * 操作资源包集合的互斥对象
	 */
	CMutex m_AssetBundlesMutex;

	/**
	 * 操作依赖集合的互斥对象
	 */
	CMutex m_DepsMutex;

	/**
	 * AssetBundle大小的互斥锁
	 */
	CMutex m_AssetBundleSizeMutex;

	vector<work_thread_ptr> m_WorkThreads;

	CMutex m_DirMutex;

	/**
	 * 被依赖集合
	 *
	 * @key 资源包名
	 * @value 被依赖的资源包集合
	 */
	dependencies_map m_BeDepended;

	html_generator_ptr m_HtmlGenerator;
	
	/**
	 * 添加AssetsFile信息
	 *
	 * @param AssetBundleName AssetBundle名字
	 * @param AssetsFileInfo AssetsFile信息
	 */
	void PutAssetsFileInfo(const string &AssetBundleName, const assets_file_info_ptr &AssetsFileInfo);

	/**
	 * 新增依赖
	 *
	 * @param AssetBundleName 资源包名
	 * @param Dependencies 依赖集合
	 */
	void PutDependencies(const string &AssetBundleName, const set<string> &Dependencies);

	/**
	 * 添加重复的资源包文件
	 *
	 * @param AssetBundleName 资源包名
	 * @param FileName 文件名
	 */
	void PutDuplicateAssetBundle(const string &AssetBundleName, const string &FileName);

	/**
	 * 在文件流中追加资源文件信息
	 *
	 * @param Stream 文件流
	 * @param Name AssetBundle名
	 * @param AssetsFileInfo 资源文件信息
	 * @param Prefix 每一行的前缀
	 */
	void AppendAssetBundle(ofstream &Stream, const string &Name, const assets_file_info_ptr &AssetsFileInfo, const string &Prefix);

	/**
	 * 在文件流中追加资源信息
	 *
	 * @param Stream 文件流
	 * @param AssetsDetail 资源信息列表
	 */
	void AppendAssets(ofstream &Stream, list<asset_detail_info_ptr> &AssetsDetail);

	/**
	 * 在文件流中追加资源信息
	 * @param Stream 文件流
	 * @param AssetDetailInfo 资源信息
	 * @param Prefix 每一行的前缀
	 */
	void AppendAsset(ofstream &Stream, const asset_detail_info_ptr &AssetDetailInfo, const string &Prefix);

	/**
	 * 导出资源文件
	 *
	 * @param AssetBundleFile 解压前的AssetBundle文件路径
	 * @param AssetsFileName 要导出的资源所属的AssetsFile名字
	 * @param uiAssetID 资源ID
	 * @return 返回资源文件路径
	 */
	string ExportAssetFile(const string & AssetBundleFile, const string & AssetsFileName, UINT64 uiAssetID);

	/**
	 * 导出队列中的所有资源文件
	 *
	 * @param AssetsDetail 资源详情队列
	 */
	void ExportAssetFiles(const list<asset_detail_info_ptr> &AssetsDetail);
};

typedef shared_ptr<CAssetBundlesParser> ab_parser_ptr;

