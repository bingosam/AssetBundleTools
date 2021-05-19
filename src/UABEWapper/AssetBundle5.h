#pragma once
/**
 * 与资源包解析相关的声明
 *
 * @author 张坤彬
 */

/**
 * 资源包解析
 * 支持Unity 5.3+
 */
class CAssetBundle5 :
	public CAssetBundle
{
public:
	/**
	 * 构造函数
	 *
	 * @param pClassDatabaseManager 类数据库管理器
	 * @param pAssetsBundleFile 资源包对象
	 * @param ppFile 资源包句柄地址
	 */
	CAssetBundle5(CClassDatabaseManager * pClassDatabaseManager, AssetsBundleFile * pAssetsBundleFile, FILE ** ppFile);
	~CAssetBundle5();

	/**
	 * 获取AssetsFile的数量
	 *
	 * @param nAssetsFileListIndex 资源文件链表的索引
	 */
	DWORD GetAssetsFileCount(int nAssetsFileListIndex);

	/**
	 * 获取资源文件大小
	 *
	 * @param pAssetsFileInfo 资源文件信息
	 */
	QWORD GetAssetsFileSize(void * pAssetsFileInfo);

	/**
	 * 获取资源文件的偏移量
	 *
	 * @param pAssetsFileInfo 资源文件信息
	 */
	QWORD GetAssetsFileOffset(void * pAssetsFileInfo);

	/**
	 * 获取资源文件的名字
	 *
	 * @param pAssetsFileInfo 资源文件信息
	 */
	const char * GetAssetsFileName(void * pAssetsFileInfo);

	/**
	 * 获取资源文件链表中指定索引的资源文件信息
	 *
	 * @param pAssetsFileList 资源文件链表
	 * @param index 资源文件索引
	 */
	void * GetAssetsFileInfo(void * pAssetsFileList, int index);

	/**
	 * 判断是否为资源文件
	 *
	 * @param pAssetsFileInfo 资源文件信息
	 */
	bool IsAssetsFile(void * pAssetsFileInfo);

	/**
	 * 获取指定索引的资源文件列表
	 *
	 * @param index 资源文件列表索引
	 */
	void * GetAssetsList(int index);

	/**
	 * 判断当前AssetsBundle是否压缩过
	 *
	 * @return 压缩过返回true,否则false
	 */
	bool IsCompressed();

	/**
	 * 创建AssetsFileReader
	 *
	 * @param pAssetsFileInfo AssetsFile文件信息
	 */
	AssetsFileReader MakeAssetsFileReader(void * pAssetsFileInfo);
	
	/**
	 * 提取所有AssetsFile
	 *
	 * @param pAssetsFileList AssetsFile文件信息列表
	 * @param pDirectoryPath 提取的资源文件所存放的目录
	 * @param pAssetFileSuffix 资源文件后缀,默认为".assets"
	 * @return 提取出来的文件名集合
	 */
	set<string> ExtractAssetsFiles(void * pAssetsFileList, const char * pDirectoryPath, const char * pAssetFileSuffix = ".assets");
};

