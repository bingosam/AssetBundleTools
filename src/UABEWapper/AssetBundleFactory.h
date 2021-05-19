#pragma once

/**
 * 资源包工厂类
 */
class CAssetBundleFactory
{
public:
	/**
	 * 创建AssetBundle对象
	 *
	 * @param pClassDatabaseManager 类数据库管理器
	 * @param pAssetsBundleFile 资源包对象
	 * @param ppFile 资源包句柄地址
	 */
	static asset_bundle_ptr CreateAssetBundle(CClassDatabaseManager * pClassDatabaseManager, AssetsBundleFile * pAssetsBundleFile, FILE ** fppFile);

	/**
	 * 创建AssetInfo对象
	 *
	 * @param pAssetFileInfo 资源文件信息
	 * @param pBaseTypeValue AssetTypeInstance.GetBaseField()
	 */
	static asset_info_ptr CreateAssetInfo(AssetFileInfoEx * pAssetFileInfo, AssetTypeValueField * pBaseTypeValue);
};