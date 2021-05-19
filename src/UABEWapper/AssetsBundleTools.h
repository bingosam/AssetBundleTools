#pragma once
/**
 * 与资源包工具相关的声明
 */

class CAssetsBundleTools
{
public:
	/**
	 * 构造函数
	 *
	 * @param pClassDatabaseManager 类数据库管理器
	 */
	CAssetsBundleTools(CClassDatabaseManager * pClassDatabaseManager);

	/**
	 * 析构函数
	 */
	~CAssetsBundleTools();

	/**
	 * 获取AssetsBundleFile
	 */
	AssetsBundleFile * GetAssetsBundleFile() { return m_AssetsBundleFile; }

	/**
	 * 获取文件句柄
	 */
	FILE * GetFile() { return m_ABFile; }

	/**
	 * 获取资源包版本
	 */
	DWORD GetVersion() { return GetAssetsBundleFile()->bundleHeader3.fileVersion; }

	/**
	 * 设置UABE日志记录器。
	 *
	 * @param logger 日志输出回调函数
	 */
	void SetUABELogger(AssetsFileVerifyLogger logger) { this->m_UABELogger = logger; }

	/**
	 * 读取AssetsBundle文件。
	 *
	 * @param pABFilePath AssetsBundle文件路径
	 * @param allowCompressed 是否允许AssetsBundle文件压缩过
	 * @return 读取成功返回true,否则false
	 */
	bool Read(const char *pABFilePath, bool allowCompressed = true);
	
	/**
	 * 判断当前AssetsBundle是否压缩过
	 *
	 * @return 压缩过返回true,否则false
	 */
	bool IsCompressed();

	/**
	 * 解压AssetsBundle文件(若压缩过)
	 *
	 * @param pABFilePath AssetsBundle文件路径
	 * @param pFileOutput 输出的文件路径
	 * @return 解压结果 @see eERROR_CODE
	 */
	int UnpackIfCompressed(const char *pABFilePath, const char *pFileOutput);

	/**
	 * 关闭
	 */
	void Close();

	/**
	 * 从解压后的资源包中提取资源文件
	 *
	 * @param pABFilePath 资源包文件路径
	 * @param pDirectoryPath 提取的资源文件所存放的目录
	 * @param pAssetFileSuffix 资源文件后缀,默认为".assets"
	 * @return 提取出来的资源文件名列表
	 */
	set<string> ExtractAssetsFile(const char *pABFilePath, const char *pDirectoryPath, const char *pAssetFileSuffix = ".assets");

	/**
	 * 提取资源文件到指定目录
	 *
	 * @param pABFilePath AssetBundle文件路径
	 * @param pAssetsFileName AssetBundle文件中的AssetsFile名字
	 * @param uiAssetID 资源ID
	 * @param pOutputDirPath 资源输出的文件夹路径
	 * @return 返回资源文件路径  若失败则为empty
	 */
	string ExportAssetFileTo(const char * pABFilePath, const char * pAssetsFileName, UINT64 uiAssetID, const char * pOutputDirPath);

	/**
	 * 从资源包中加载资源文件
	 *
	 * @param pABFilePath 资源包文件路径
	 * @return 第一个资源文件
	 */
	assets_file_ptr LoadFirstAssetsFile(const char *pABFilePath);

	/**
	 * 加载下一个资源文件
	 * @return 下一个资源文件
	 */
	assets_file_ptr LoadNextAssetsFile();

	/**
	 * 加载AssetsFile
	 *
	 * @param pABFilePath 资源包文件路径
	 * @param pAssetsFileName AssetsFile名字
	 * @return AssetsFile对象
	 */
	assets_file_ptr LoadAssetsFile(const char * pABFilePath, const char * pAssetsFileName);

private:
	/**
	 * 资源包文件实例
	 */
	AssetsBundleFile * m_AssetsBundleFile;

	/**
	 * AssetsBundle文件句柄
	 */
	FILE *m_ABFile;

	/**
	 * 提供给UABE的日志记录器
	 */
	AssetsFileVerifyLogger m_UABELogger;

	/**
	 * 类数据库管理器
	 */
	CClassDatabaseManager *m_ClassDatabaseManager;

	/**
	 * AssetBundle对象
	 */
	asset_bundle_ptr m_AssetBundle;
};

typedef shared_ptr<CAssetsBundleTools> ab_file_ptr;

