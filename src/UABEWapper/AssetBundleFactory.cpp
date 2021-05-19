#include "stdafx.h"
#include "../svcdef.h"


#define AB_FILEVERSION_UNITY3		3	//Unity3.5 and 4
#define AB_FILEVERSION_UNITY5_3		6	//Unity5.3+

/**
 * ����AssetBundle����
 *
 * @param pClassDatabaseManager �����ݿ������
 * @param pAssetsBundleFile ��Դ������
 * @param ppFile ��Դ�������ַ
 */
asset_bundle_ptr CAssetBundleFactory::CreateAssetBundle(CClassDatabaseManager * pClassDatabaseManager, AssetsBundleFile * pAssetsBundleFile, FILE ** fppFile)
{
	switch (pAssetsBundleFile->bundleHeader3.fileVersion)
	{
	case AB_FILEVERSION_UNITY3:
		return asset_bundle_ptr(new CAssetBundle(pClassDatabaseManager, pAssetsBundleFile, fppFile));
	case AB_FILEVERSION_UNITY5_3:
		return asset_bundle_ptr(new CAssetBundle5(pClassDatabaseManager, pAssetsBundleFile, fppFile));
	default: 
		return asset_bundle_ptr();
	}
}

/**
 * ����AssetInfo����
 *
 * @param pAssetFileInfo ��Դ�ļ���Ϣ
 * @param pBaseTypeValue AssetTypeInstance.GetBaseField()
 */
asset_info_ptr CAssetBundleFactory::CreateAssetInfo(AssetFileInfoEx * pAssetFileInfo, AssetTypeValueField * pBaseTypeValue)
{
	asset_info_ptr Asset(new CAssetInfo(pAssetFileInfo->name, pAssetFileInfo->index
		, pAssetFileInfo->curFileType, pBaseTypeValue->GetTemplateField()->type, pAssetFileInfo->curFileSize));

	AssetTypeValueField * pName = pBaseTypeValue->Get("m_Name");
	if (NULL != pName && !pName->IsDummy())
	{
		char * pValue = pName->GetValue()->AsString();
		if (NULL != pValue)
		{			
			Asset->SetFileName(Utf8StrToAnsiStr(pValue));
		}
	}

	if (strcmp("AudioClip", Asset->GetTypeString().c_str()) == 0)
	{
		AssetTypeValueField * pResource = pBaseTypeValue->Get("m_Resource");
		if (NULL != pResource)
		{
			AssetTypeValueField * pSize = pResource->Get("m_Size");
			if (NULL != pSize)
			{
				Asset->SetResourceSize(pSize->GetValue()->AsUInt64());
			}
		}
	}
	return Asset;
}
