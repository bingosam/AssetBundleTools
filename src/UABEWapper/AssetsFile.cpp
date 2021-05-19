#include "StdAfx.h"

#include "../svcdef.h"


static const TCHAR * g_pszLogName = _T("Assets File");

/**
 * 构造函数
 *
 * @param pClassDatabaseManager 类数据库管理器
 */
CAssetsFile::CAssetsFile(CClassDatabaseManager *pClassDatabaseManager)
{
	m_AssetsFileTable = NULL;
	m_AssetsFile = NULL;
	m_File = NULL;
	m_IsLoadFromAssetsBundle = false;
	m_ClassDatabaseManager = pClassDatabaseManager;
}

CAssetsFile::~CAssetsFile()
{
	Close();
}

/**
 * 加载资源文件
 *
 * @param pAssetsFilePath 资源文件路径
 */
bool CAssetsFile::Load(const char * pAssetsFilePath)
{
	Close();
	SetFileName(pAssetsFilePath);
	return Load(AssetsReaderFromFile, fopen(pAssetsFilePath, "rb"));
}

/**
 * 从内存中加载资源文件
 *
 * @param reader 资源文件读取器,通过MakeAssetsFileReader获得
 * @param fpAssetsBundleFile 资源包文件实例
 */
bool CAssetsFile::LoadFromAssetsBundle(AssetsFileReader reader, FILE * fpAssetsBundleFile)
{
	Close();
	m_IsLoadFromAssetsBundle = true;
	return Load(reader, fpAssetsBundleFile);
}

/**
 * 关闭资源文件并释放资源
 */
void CAssetsFile::Close()
{
	FREE(m_AssetsFileTable, delete);
	FREE(m_AssetsFile, delete);
	if (m_IsLoadFromAssetsBundle)
	{
		m_File = NULL;
		m_IsLoadFromAssetsBundle = false;
	}
	else
	{
		FREE(m_File, fclose);
	}
	m_Dependencies.clear();
	m_Info.reset(new CAssetsFileInfo());
	m_LoadInformation = false;
}

/**
 * 加载资源类型模版
 *
 * @param pAssetTypeTemplateField 资源类型模版
 * @param type 类型
 * @return 加载成功与否
 */
bool CAssetsFile::LoadAssetTypeTemplateField(AssetTypeTemplateField * pAssetTypeTemplateField, DWORD type)
{
	pAssetTypeTemplateField->Clear();
	bool result = false;
	if (m_AssetsFile->typeTree.hasTypeTree && m_AssetsFile->typeTree.fieldCount > 0)
	{
		if (m_AssetsFile->header.format < 0x0D)
		{//FIXME: Why 0x0D
			Type_07 *pIndex = NULL;
			for (DWORD i = 0; i < m_AssetsFile->typeTree.fieldCount; ++i)
			{
				pIndex = m_AssetsFile->typeTree.pTypes_Unity4 + i;
				if (pIndex->classId == type)
				{
					result = pAssetTypeTemplateField->From07(&pIndex->base);
					break;
				}
			}
		}
		else
		{
			Type_0D *pIndex = NULL;
			for (DWORD i = 0; i < m_AssetsFile->typeTree.fieldCount; ++i)
			{
				pIndex = m_AssetsFile->typeTree.pTypes_Unity5 + i;
				if (pIndex->classId == type)
				{
					result = pAssetTypeTemplateField->From0D(pIndex, 0);
					break;
				}
			}
		}
	}
	else
	{
		result = LoadAssetTypeTemplateFieldFromClassDatabase(pAssetTypeTemplateField, type);
	}

	if (!result)
	{
		pAssetTypeTemplateField->Clear();
	}

	return result;
}

/**
 * 从类数据库中加载资源类型模版
 *
 * @param pAssetTypeTemplateField 资源类型模版
 * @param type 类型
 * @return 加载成功与否
 */
bool CAssetsFile::LoadAssetTypeTemplateFieldFromClassDatabase(AssetTypeTemplateField * pAssetTypeTemplateField, DWORD type)
{
	if (NULL == m_ClassDatabaseManager) return false;

	class_database_ptr ClassDatabase = m_ClassDatabaseManager->LookupClassDatabase(m_AssetsFile->typeTree.unityVersion);
	if (!ClassDatabase.get()) return false;

	size_t nClassesCount = ClassDatabase->classes.size();
	for (size_t i = 0; i < nClassesCount; ++i)
	{
		ClassDatabaseType cClassDatabaseType = ClassDatabase->classes[i];
		if (cClassDatabaseType.classId == type)
		{
			return pAssetTypeTemplateField->FromClassDatabase(ClassDatabase.get(), &cClassDatabaseType, 0);
		}
	}

	return false;
}

/**
 * 获取资源信息
 *
 * @param name 资源名
 * @return 资源信息指针
 */
AssetFileInfoEx * CAssetsFile::GetAssetInfo(const char * name)
{
	return m_AssetsFileTable->getAssetInfo(name);
}

/**
 * 获取资源信息
 *
 * @param name 资源名
 * @param type 资源类型
 * @return 资源信息指针
 */
AssetFileInfoEx * CAssetsFile::GetAssetInfo(const char * name, DWORD type)
{
	return m_AssetsFileTable->getAssetInfo(name, type);
}

/**
 * 获取资源信息
 *
 * @param pathId 资源ID
 * @return 资源信息指针
 */
AssetFileInfoEx * CAssetsFile::GetAssetInfo(QWORD pathId)
{
	return m_AssetsFileTable->getAssetInfo(pathId);
}

/**
 * 判断是否是manifest资源包(包含Manifest资源即为true)
 */
bool CAssetsFile::IsAssetBundleManifest()
{
	return NULL != GetAssetInfo("AssetBundelManifest");
}

/**
 * 获取名为AssetBundle的资源
 */
AssetFileInfoEx * CAssetsFile::GetAssetBundleAsset()
{
	static QWORD snAssetBundlePathID = 1;
	return GetAssetInfo(snAssetBundlePathID);
}

/**
 * 加载资源信息
 *
 * @param pOutputDirPath 输出目录
 */
void CAssetsFile::LoadAssetInfos(const char * pOutputDirPath/* = NULL*/)
{//UABE 若是MovieTexture，推荐把m_MovieData的类型改成TypelessData，可以减少内存、时间开销

	void (CAssetsFile::*AssetExportMethod)(const asset_info_ptr & AssetInfo, AssetTypeValueField * pBaseTypeValue, const char * pOutputDir);
	if (NULL == pOutputDirPath)
	{
		AssetExportMethod = &CAssetsFile::DoNotExportAsset;
	}
	else
	{
		AssetExportMethod = &CAssetsFile::ExportAsset;
		if (ACCESS(pOutputDirPath, 0) != 0) throw COsError(0, "Unable to access directory %s", pOutputDirPath);
	}

	for (unsigned int i = 0; i < m_AssetsFileTable->assetFileInfoCount; ++i)
	{
		AssetFileInfoEx *pAssetFileInfo = m_AssetsFileTable->pAssetFileInfo + i;
		AssetTypeTemplateField cAssetTypeTemplateField;
		AssetTypeTemplateField *pAssetTypeTemplateField = &cAssetTypeTemplateField;
		if (!LoadAssetTypeTemplateField(pAssetTypeTemplateField, pAssetFileInfo->curFileType))
			throw CUnityError(INVALID_ASSETS_FILE, "Unable to load the AssetTypeTemplateField by asset file type: %lu", pAssetFileInfo->curFileType);

		//耗时较久
		AssetTypeInstance cAssetTypeInstance(1, &pAssetTypeTemplateField, m_AssetsFileTable->getReader(), m_AssetsFileTable->getReaderPar(), IsBigEndian(), pAssetFileInfo->absolutePos);
		AssetTypeValueField *pBaseTypeValue = cAssetTypeInstance.GetBaseField();

		asset_info_ptr Asset = CAssetBundleFactory::CreateAssetInfo(pAssetFileInfo, pBaseTypeValue);
		Asset->SetMD5(MakeMD5(pAssetFileInfo, pBaseTypeValue));
		//It's AssetBundle asset
		if (Asset->IsAssetBundleAsset())
		{
			if (pBaseTypeValue->IsDummy()) throw CUnityError(INVALID_ASSETS_FILE, "The asset type is dummy!");

			AssetTypeValueField *pAssetBundleName = pBaseTypeValue->Get("m_AssetBundleName");
			if (NULL == pAssetBundleName || pAssetBundleName->IsDummy())
				throw CUnityError(INVALID_ASSETS_FILE, "Unable to get the name of assetbundle");
			m_Info->SetAssetBundleName(pAssetBundleName->GetValue()->AsString());
		}

		(this->*AssetExportMethod)(Asset, pBaseTypeValue, pOutputDirPath);
		m_Info->PutAsset(pAssetFileInfo->index, Asset);
	}
}

/**
 * 提取资源文件
 *
 * @param uiAssetID 资源ID
 * @param pOutputDirPath 输出文件夹路径
 * @return 返回资源文件路径  若失败则为empty
 */
string CAssetsFile::ExportAssetFileTo(UINT64 uiAssetID, const char * pOutputDirPath)
{
	AssetFileInfoEx * pAssetFileInfo = GetAssetInfo(uiAssetID);
	if (NULL == pAssetFileInfo) throw CUnityError(ASSET_NOT_FOUND, "Asset ID %llu", uiAssetID);

	AssetTypeTemplateField cAssetTypeTemplateField;
	AssetTypeTemplateField *pAssetTypeTemplateField = &cAssetTypeTemplateField;
	if (!LoadAssetTypeTemplateField(pAssetTypeTemplateField, pAssetFileInfo->curFileType))
		throw CUnityError(INVALID_ASSETS_FILE, "Unable to load the AssetTypeTemplateField by asset file type: %lu", pAssetFileInfo->curFileType);

	//耗时较久
	AssetTypeInstance cAssetTypeInstance(1, &pAssetTypeTemplateField, m_AssetsFileTable->getReader(), m_AssetsFileTable->getReaderPar(), IsBigEndian(), pAssetFileInfo->absolutePos);
	AssetTypeValueField *pBaseTypeValue = cAssetTypeInstance.GetBaseField();

	asset_info_ptr Asset = CAssetBundleFactory::CreateAssetInfo(pAssetFileInfo, pBaseTypeValue);

	ExportAsset(Asset, pBaseTypeValue, pOutputDirPath);
	return Asset->GetFilePath();
}

/**
 * 加载资源文件
 *
 * @param reader 资源文件读取器
 * @param fpAssetsBundleFile 资源包文件实例
 */
bool CAssetsFile::Load(AssetsFileReader reader, FILE * fpAssetsBundleFile)
{
	if (NULL == fpAssetsBundleFile) return false;

	m_File = fpAssetsBundleFile;

	m_AssetsFile = new AssetsFile(reader, (LPARAM)fpAssetsBundleFile);
	if (!m_AssetsFile->VerifyAssetsFile())
	{
		Close();
		return false;
	}
	m_AssetsFileTable = new AssetsFileTable(m_AssetsFile);
	ParseDependencies();
	return true;
}

/**
 * 生成MD5
 *
 * @param pAssetFileInfo 资源文件信息
 * @param pAssetTypeValueField 资源属性
 */
string CAssetsFile::MakeMD5(AssetFileInfoEx * pAssetFileInfo, AssetTypeValueField * pAssetTypeValueField)
{
	bool bFindName = false;
	string Data = GetAssetData(bFindName, pAssetTypeValueField);
	return MD5Hash(Data.c_str(), Data.size());
}

/**
 * 解析依赖
 */
void CAssetsFile::ParseDependencies()
{
	for (DWORD i = 0; i < m_AssetsFile->dependencies.dependencyCount; ++i)
	{
		AssetsFileDependency *pAssetsFileDependency = m_AssetsFile->dependencies.pDependencies + i;
		vector<string> Items;
		SpliteString(Items, pAssetsFileDependency->assetPath, strlen(pAssetsFileDependency->assetPath), '/');
		if (Items.empty()) continue;//what?
		if (Items[0].compare("archive:") != 0) continue;
		m_Dependencies.insert(Items.back());
	}

}

/**
 * 获取资源数据
 *
 * @param bFindName[in,out] 是否找到资源名字字段
 * @param pAssetTypeValueField
 * @return 资源数据
 */
string CAssetsFile::GetAssetData(bool & bFindName, AssetTypeValueField * pAssetTypeValueField)
{
	if (NULL == pAssetTypeValueField) return string();
	ostringstream Stream;

	Stream
		<< (NULL == pAssetTypeValueField->GetType() ? "" : pAssetTypeValueField->GetType())
		<< " "
		<< (NULL == pAssetTypeValueField->GetName() ? "" : pAssetTypeValueField->GetName());
 	bool bIsName = false;
	if (!bFindName)
	{
		bFindName = (NULL != pAssetTypeValueField->GetName() && strcmp("m_Name", pAssetTypeValueField->GetName()) == 0);
		bIsName = bFindName;
	}

	AssetTypeValue *pAssetTypeValue = NULL;
	if (NULL != (pAssetTypeValue = pAssetTypeValueField->GetValue()))
	{
		switch (pAssetTypeValue->GetType())
		{
		case ValueType_Bool:
			Stream << " = " << (pAssetTypeValue->AsBool() ? "true" : "false");
			break;
		case ValueType_Int8:
		case ValueType_Int16:
		case ValueType_Int32:
			Stream << " = " << pAssetTypeValue->AsInt();
			break;
		case ValueType_UInt8:
		case ValueType_UInt16:
		case ValueType_UInt32:
			Stream << " = " << pAssetTypeValue->AsUInt();
			break;
		case ValueType_Int64:
			Stream << " = " << pAssetTypeValue->AsInt64();
			break;
		case ValueType_UInt64:
			Stream << " = " << pAssetTypeValue->AsUInt64();
			break;
		case ValueType_Float:
			Stream << " = " << pAssetTypeValue->AsFloat();
			break;
		case ValueType_Double:
			Stream << " = " << pAssetTypeValue->AsDouble();
			break;
		case ValueType_String:
			Stream << " = \"" << (bIsName ? "" : pAssetTypeValue->AsString()) << "\"";
			break;
		case ValueType_Array:
			Stream << "\nint size = " << pAssetTypeValue->AsArray()->size;
			break;
		case ValueType_ByteArray:
		{
			AssetTypeByteArray * pValue = pAssetTypeValue->AsByteArray();
			Stream << " = {";
			if (pValue->size > 0)
			{
				size_t nLength = pValue->size * 2 + 1;
				BYTE * szBuffer = (BYTE *)malloc(nLength);
				szBuffer[0] = 0;

				BinToHex(pValue->data, pValue->size, szBuffer, nLength, true);
				Stream << "0x" << szBuffer[0] << szBuffer[1];
				for (DWORD i = 2; nLength - i > 1; )
				{
					Stream << ", 0x" << szBuffer[i++] << szBuffer[i++];
				}
				free(szBuffer);
			}
			Stream << "}";

		}
			break;
		default:
			break;
		}
	}
	Stream << "\n";
	for (DWORD i = 0; i < pAssetTypeValueField->GetChildrenCount(); ++i)
	{
		Stream << GetAssetData(bFindName, pAssetTypeValueField->GetChildrenList()[i]);
	}
	return Stream.str();
}

/**
 * 提取资源
 *
 * @param AssetInfo 资源信息
 * @param pBaseTypeValue 资源数据
 * @param pOutputDirPath 输出目录
 */
void CAssetsFile::ExportAsset(const asset_info_ptr & AssetInfo, AssetTypeValueField * pBaseTypeValue, const char * pOutputDirPath)
{
	asset_exporter_ptr AssetExtractor = CAssetExtractorFactory::CreateExporter(pBaseTypeValue);
	if (!AssetExtractor.get()) return;

	GetLogService()->Debug(g_pszLogName, _T("Exporting \"%hs\" from \"%hs\""), AssetInfo->GetFileName().c_str(), GetFileName().c_str());
	string FileName(AssetInfo->GetFileName());
	FileName.append(AssetExtractor->GetSuffix());

	string AssetFile(pOutputDirPath);
	AssetFile.append("/").append(FileName);

	unsigned int uError = AssetExtractor->ExtractAsset(AssetFile);
	if (0 == uError)
	{
		GetLogService()->Debug(g_pszLogName, _T("Export \"%hs\" from \"%hs\" success"), AssetInfo->GetFileName().c_str(), GetFileName().c_str());
		AssetInfo->SetFilePath(AssetFile);
	}
	else
	{
		GetLogService()->Warn(g_pszLogName, _T("Unable to export asset \"%hs\", path ID %llu: %hs")
			, AssetInfo->GetFileName(), AssetInfo->GetPathID(), GetAssetExportErrorMessage(uError));
	}
}

/**
 * 提取资源
 *
 * @param AssetInfo 资源信息
 * @param pBaseTypeValue 资源数据
 * @param pOutputDir 输出目录
 */
void CAssetsFile::DoNotExportAsset(const asset_info_ptr & AssetInfo, AssetTypeValueField * pBaseTypeValue, const char * pOutputDir)
{
}
