#pragma once

/**
 * 与资源包解析相关的声明
 *
 * @author 张坤彬
 */

/**
 * 资源包解析
 * 支持Unity 5.3以下版本
 */
class CAssetBundle
{
public:
	/**
	 * 构造函数
	 *
	 * @param pClassDatabaseManager 类数据库管理器
	 * @param pAssetsBundleFile 资源包对象
	 * @param ppFile 资源包句柄地址
	 */
	CAssetBundle(CClassDatabaseManager * pClassDatabaseManager, AssetsBundleFile * pAssetsBundleFile, FILE ** ppFile);
	~CAssetBundle();

	/**
	 * 获取AssetsFile的数量
	 *
	 * @param nAssetsFileListIndex 资源文件链表的索引
	 */
	virtual DWORD GetAssetsFileCount(int nAssetsFileListIndex);

	/**
	 * 获取资源文件大小
	 *
	 * @param pAssetsFileInfo 资源文件信息
	 */
	virtual QWORD GetAssetsFileSize(void * pAssetsFileInfo);

	/**
	 * 获取资源文件的偏移量
	 *
	 * @param pAssetsFileInfo 资源文件信息
	 */
	virtual QWORD GetAssetsFileOffset(void * pAssetsFileInfo);

	/**
	 * 获取资源文件的名字
	 *
	 * @param pAssetsFileInfo 资源文件信息
	 */
	virtual const char * GetAssetsFileName(void * pAssetsFileInfo);

	/**
	 * 判断是否为资源文件
	 *
	 * @param pAssetsFileInfo 资源文件信息
	 */
	virtual bool IsAssetsFile(void * pAssetsFileInfo);

	/**
	 * 获取指定索引的资源文件列表
	 *
	 * @param index 资源文件列表索引
	 */
	virtual void * GetAssetsList(int index);

	/**
	 * 获取资源文件链表中指定索引的资源文件信息
	 *
	 * @param pAssetsFileList 资源文件链表
	 * @param index 资源文件索引
	 */
	virtual void * GetAssetsFileInfo(void * pAssetsFileList, int index);

	/**
	 * 判断当前AssetsBundle是否压缩过
	 *
	 * @return 压缩过返回true,否则false
	 */
	virtual bool IsCompressed();

	/**
	 * 创建AssetsFileReader
	 *
	 * @param pAssetsFileInfo AssetsFile文件信息
	 */
	virtual AssetsFileReader MakeAssetsFileReader(void * pAssetsFileInfo);

	/**
	 * 获取版本号
	 */
	DWORD GetVersion();

	/**
	 * 提取AssetsFile
	 *
	 * @param pAssetsFileInfo AssetsFile文件信息
	 * @param pDirectoryPath 提取的资源文件所存放的目录
	 * @param pAssetFileSuffix 资源文件后缀,默认为".assets"
	 * @return 提取出来的文件名
	 */
	string ExtractAssetsFile(void * pAssetsFileInfo, const char * pDirectoryPath, const char * pAssetFileSuffix = ".assets");
	
	/**
	 * 提取所有AssetsFile
	 *
	 * @param pAssetsFileList AssetsFile文件信息列表
	 * @param pDirectoryPath 提取的资源文件所存放的目录
	 * @param pAssetFileSuffix 资源文件后缀,默认为".assets"
	 * @return 提取出来的文件名集合
	 */
	virtual set<string> ExtractAssetsFiles(void * pAssetsFileList, const char * pDirectoryPath, const char * pAssetFileSuffix = ".assets");

	/**
	 * 提取所有AssetsFile
	 *
	 * @param pDirectoryPath 提取的资源文件所存放的目录
	 * @param pAssetFileSuffix 资源文件后缀,默认为".assets"
	 * @return 提取出来的文件名集合
	 */
	set<string> ExtractAssetsFiles(const char * pDirectoryPath, const char * pAssetFileSuffix = ".assets");

	/**
	 * 从资源包中加载资源文件
	 *
	 * @return 第一个资源文件
	 */
	assets_file_ptr LoadFirstAssetsFile();

	/**
	 * 加载下一个资源文件
	 * @return 下一个资源文件
	 */
	assets_file_ptr LoadNextAssetsFile();

	/**
	 * 加载AssetsFile
	 *
	 * @param pAssetsFileName 要加载的AssetsFile名字
	 * @return AssetsFile对象
	 */
	assets_file_ptr LoadAssetsFile(const char * pAssetsFileName);

protected:
	/**
	 * 加载资源文件
	 *
	 * @param pAssetsFileInfo AssetsFile文件信息
	 * @return AssetsFile对象
	 */
	assets_file_ptr LoadAssetsFile(void * pAssetsFileInfo);

protected:
	/**
	 * 资源包对象
	 */
	AssetsBundleFile * m_AssetsBundleFile;

	/**
	 * 资源文件句柄地址
	 */
	FILE ** m_File;

	/**
	 * 类数据库管理器
	 */
	CClassDatabaseManager * m_ClassDatabaseManager;

	/**
	 * 当前资源文件索引
	 */
	DWORD m_nAssetsFileIndex;

	/**
	 * 当前资源文件链表索引
	 */
	DWORD m_nAssetsFileListIndex;

private:

	/**
	 * 资源文件阅读器
	 */
	AssetsFileReader m_AssetsFileReader;

	/**
	 * 释放资源文件阅读器
	 */
	void FreeAssetsFileReader();
};
typedef shared_ptr<CAssetBundle> asset_bundle_ptr;