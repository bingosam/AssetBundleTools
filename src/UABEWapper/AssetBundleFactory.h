#pragma once

/**
 * ��Դ��������
 */
class CAssetBundleFactory
{
public:
	/**
	 * ����AssetBundle����
	 *
	 * @param pClassDatabaseManager �����ݿ������
	 * @param pAssetsBundleFile ��Դ������
	 * @param ppFile ��Դ�������ַ
	 */
	static asset_bundle_ptr CreateAssetBundle(CClassDatabaseManager * pClassDatabaseManager, AssetsBundleFile * pAssetsBundleFile, FILE ** fppFile);

	/**
	 * ����AssetInfo����
	 *
	 * @param pAssetFileInfo ��Դ�ļ���Ϣ
	 * @param pBaseTypeValue AssetTypeInstance.GetBaseField()
	 */
	static asset_info_ptr CreateAssetInfo(AssetFileInfoEx * pAssetFileInfo, AssetTypeValueField * pBaseTypeValue);
};