#include "stdafx.h"

#include "svcdef.h"
#include "AssetBundlesParser.h"
#include <fstream>

#define MUTILE_THREAD
#define UNITY3D_SUFFIX ".ndunity3d"


/**
 * 判断文件名是否疑似资源包
 */
bool MayBeAssetsBundle(const char *pFileName)
{
	return !EndWith(pFileName, ".manifest") && !EndWith(pFileName, ".meta") && !EndWith(pFileName, ".obj") &&!EndWith(pFileName, UNITY3D_SUFFIX);
}

/**
 * 判断文件名是否是没用的文件
 */
bool IsUnlessFile(const char *pFileName)
{
	return !MayBeAssetsBundle(pFileName);
}

/**
 * 构造函数
 *
 * @param pOutputDirPath 输出目录
 * @param bExportAssetFile 是否导出资源文件，默认不导出
 */
CAssetBundlesParser::CAssetBundlesParser(const char * pOutputDirPath, bool bExportAssetFile)
{
	const string & OutoutDir = Replace(pOutputDirPath, "\\", "/");
	m_OutputDirPath.assign(pOutputDirPath);
	m_AssetsBundle.reset(new CAssetsBundleTools(GetClassDatabaseManager()));
	m_nDecompressedSize = 0;
	m_nCompressedSize = 0;
	m_nReduAssetsSize = 0;
	if (bExportAssetFile)
	{
		m_AssetExportDir.assign(pOutputDirPath).append("/Assets");
	}
#ifdef MUTILE_THREAD
	m_WorkThreads.resize(3);
	for (size_t i = 0; i < m_WorkThreads.size(); ++i)
	{
		m_WorkThreads[i].reset(new CWorkThread(this));
	}
#endif
}

CAssetBundlesParser::~CAssetBundlesParser()
{
	Release();
}

/**
 * 解压指定文件夹下的所有资源包文件
 *
 * @param pDirectoryPath 存放资源包的文件夹路径
 */
void CAssetBundlesParser::UnpackAssetsBundleFiles(const char *pDirectoryPath)
{
	m_DirPath.assign(pDirectoryPath);

	SYSTEMTIME LocalTime;
	GetLocalTime(&LocalTime);
	string CurDate = FormatStr("%04u-%02u-%02u_%02u-%02u-%02u-%03u"
		, LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay, LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond, LocalTime.wMilliseconds);

	set<string> Files = ListDirectory(m_DirPath.c_str(), true, "*", MayBeAssetsBundle);
	for (set<string>::iterator It = Files.begin(); It != Files.end(); ++It)
	{
		UnpackAssetBundleFile(m_AssetsBundle, Replace(*It, "\\", "/"));
	}
}

/**
* 解压AssetBundle文件
*
* @param AssetBundle AssetBundle工具类
* @param AssetBundleFile AssetBundle文件路径
* @return 成功返回解压后的文件路径, 否则返回空
*/
string CAssetBundlesParser::UnpackAssetBundleFile(const ab_file_ptr & AssetBundle, const string & AssetBundleFile)
{
	string RelativePath = AssetBundleFile.substr(m_DirPath.size() + 1);
	const char *pAssetBundleFilePath = NULL;
	string Unity3DFilePath(m_OutputDirPath);
	Unity3DFilePath.append("/AssetBundles/").append(RelativePath).append(UNITY3D_SUFFIX);

#ifdef MUTILE_THREAD
	{
		CMutexGuard Lock(m_DirMutex);
#endif
		if (CreateDirectoryRecursion(Unity3DFilePath.c_str(), false) != 0) throw COsError();
#ifdef MUTILE_THREAD
	}
#endif

	size_t nFileSize = GetFileSize(AssetBundleFile.c_str());
	int nResult = AssetBundle->UnpackIfCompressed(AssetBundleFile.c_str(), Unity3DFilePath.c_str());
	switch (nResult)
	{
	case SUCCESSED:
	{
		pAssetBundleFilePath = Unity3DFilePath.c_str();
		AddSize(GetFileSize(pAssetBundleFilePath), nFileSize);
		break;
	}
	case ASSETS_BUNDLE_FILE_NOT_COMPRESSED:
	{
		pAssetBundleFilePath = AssetBundleFile.c_str();
		AddSize(nFileSize, nFileSize);
		break;
	}
	default:
		GetLogService()->Warn(GetName(), _T("Failed to unpack assets bundle file \"%hs\"  %hs"), AssetBundleFile.c_str(), CUnityError(nResult).what());
		return string();
	}
	GetLogService()->Debug(GetName(), _T("The \"%hs\" assets bundle file is %hs compressed"), RelativePath.c_str(), nResult == SUCCESSED ? "" : "not");
	PutDecompressedAssetBundleFile(Replace(RelativePath, "\\", "/"), pAssetBundleFilePath);
	return pAssetBundleFilePath;
}

/**
 * 加载所有资源包信息
 */
void CAssetBundlesParser::LoadAssetBundles()
{
	for (map<string, string>::iterator It = m_AssetsBundleFilePathes.begin(); It != m_AssetsBundleFilePathes.end(); ++It)
	{
		LoadAssetBundle(m_AssetsBundle, It->first, It->second);
	}
	m_AssetsBundle->Close();
}

/**
 * 加载AssetBundle
 *
 * @param AssetBundle AssetBundle工具类
 * @param AssetBundleFile AssetBundle文件路径
 * @param DecompressedFile AssetBundle解压后的文件路径
 */
void CAssetBundlesParser::LoadAssetBundle(const ab_file_ptr & AssetBundle, const string & AssetBundleFile, const string & DecompressedFile)
{
	set<string> AssetsFileNames;
	try
	{
		GetLogService()->Debug(GetName(), _T("Load asset bundle file \"%hs\""), AssetBundleFile.c_str());
		string AssetsDirPath = GetAssetsDirPath(AssetBundleFile);
		const char * pAssetsDirPath = AssetsDirPath.c_str();

#ifdef MUTILE_THREAD
		{
			CMutexGuard Lock(m_DirMutex);
#endif
			if (CreateDirectoryRecursion(pAssetsDirPath) != 0)
				pAssetsDirPath = NULL;
#ifdef MUTILE_THREAD
		}
#endif

		assets_file_ptr AssetsFile = AssetBundle->LoadFirstAssetsFile(DecompressedFile.c_str());

		//FIXME: 多个AssetsFile时，只有其中一个有AssetBundle类型资源
		while (AssetsFile.get())
		{
			if (!AssetsFile->IsAssetBundleManifest())
			{
				AssetsFileNames.insert(AssetsFile->GetName());
				assets_file_info_ptr AssetsFileInfo = LookupAssetsFileInfo(AssetsFile->GetName());
				if (AssetsFileInfo.get())
				{
					//PutDuplicateAssetBundle(AssetsFile->GetName(), It->first);
					GetLogService()->Warn(GetName(), _T("在文件\"%hs\"、\"%s\"中发现同名资源包\"%hs\"")
						, AssetsFileInfo->GetFileName().c_str(), AssetBundleFile.c_str(), AssetsFile->GetName().c_str());
				}
				else
				{
					AssetsFile->SetFileName(AssetBundleFile);
					AssetsFile->LoadAssetInfos(NULL);
					PutAssetsFileInfo(AssetsFile->GetName(), AssetsFile->GetInformation());
					PutDependencies(AssetsFile->GetName(), AssetsFile->GetDependencies());
				}
			}
			AssetsFile = AssetBundle->LoadNextAssetsFile();
		};
		PutAssetsFile(AssetBundleFile, AssetsFileNames);
	}
	catch (exception &e)
	{
		GetLogService()->Warn(GetName(), _T("Failed to load assets file from assets bundle \"%hs\": %hs"), AssetBundleFile.c_str(), e.what());
	}
}

/**
 * 获取指定AssetBundle提取的资源的存放路径
 */
string CAssetBundlesParser::GetAssetsDirPath(const string & AssetBundleFile)
{
	string AssetsDirPath(m_AssetExportDir);
	AssetsDirPath.append("/").append(AssetBundleFile);
	return AssetsDirPath;
}

/**
 * 将资源文件绝对路径转化成相对路径
 *
 * @param AbsAssetPath 资源绝对路径
 * @return 相对路径
 */
string CAssetBundlesParser::ConvertAssetPathToRelativePath(const string & AbsAssetPath)
{
	if (AbsAssetPath.empty()) return AbsAssetPath;
	return Replace(AbsAssetPath.substr(m_OutputDirPath.size() + 1), "\\", "/");
}

/**
 * 释放资源
 */
void CAssetBundlesParser::Release()
{
	m_AssetsBundle.reset();
	m_WorkThreads.clear();
	Clear();
}

/**
 * 清理缓存
 */
void CAssetBundlesParser::Clear()
{
	m_nDecompressedSize = 0;
	m_nCompressedSize = 0;
	m_nReduAssetsSize = 0;
#ifdef NDEBUG
	for (map<string, string>::iterator It = m_AssetsBundleFilePathes.begin(); It != m_AssetsBundleFilePathes.end(); ++It)
	{
		DeleteFileA(It->second.c_str());
	}
#endif
	m_AssetBundles.clear();
	m_DuplicateAssetbundles.clear();
	m_AssetsFileInfos.clear();
	m_Dependencies.clear();
	m_BeDepended.clear();
	m_AssetsBundleFilePathes.clear();
}

/**
 * 资源解析
 *
 * @return 返回输出文件路径
 */
string CAssetBundlesParser::Analyze()
{
	string Output(m_OutputDirPath);
	Output.append("/ResultJs/result.js");
	CreateDirectoryRecursion(Output.c_str(), false);
	ofstream Stream(Output);

	list<asset_detail_info_ptr> AssetsDetail;
	Stream
		<< "var result = \n"
		<< "{\n"
		<< "\t\"assets_bundles\": [\n";
	for (assets_file_info_map::iterator It = m_AssetsFileInfos.begin(); It != m_AssetsFileInfos.end(); ++It)
	{
		Stream
			<< "\t\t{\n";
		AppendAssetBundle(Stream, It->first, It->second, "\t\t\t");

		//资源
		Stream << "\t\t\t\"assets\": [\n";
		asset_info_map &Assets = It->second->GetAssets();
		map<string, INT32> AssetCount;
		for (asset_info_map::iterator AssetIt = Assets.begin(); AssetIt != Assets.end(); ++AssetIt)
		{
			const asset_info_ptr &Asset = AssetIt->second;
			if (AssetIt->second->IsAssetBundleAsset()) continue;
			asset_detail_info_ptr AssetDetailInfo(new ASSET_DETAIL_INFO());
			AssetDetailInfo->m_AssetInfo = Asset;
			AssetDetailInfo->m_OwnerName = It->second->GetName();
			AssetDetailInfo->m_OwnerFileName = It->second->GetFileName();
			AssetDetailInfo->m_OwnerAssetBundleName = It->second->GetAssetBundleName();
			AssetsDetail.push_back(AssetDetailInfo);
			Stream
				<< "\t\t\t\t{\n"
//				<< "\t\t\t\t\t\"id\": \"" << Asset->GetPathID() << "\",\n"
				<< "\t\t\t\t\t\"name\": \"" << Asset->GetName() << "\",\n"
				<< "\t\t\t\t\t\"type\": \"" << Asset->GetTypeString() << "\",\n"
//				<< "\t\t\t\t\t\"file\": \"" << Asset->GetFilePath() << "\",\n"
				<< "\t\t\t\t\t\"size\": " << Asset->GetSize() << ",\n"
//				<< "\t\t\t\t\t\"md5\": \"" << Asset->GetMD5() << "\",\n"
				<< "\t\t\t\t},\n";
			INT32 count = AssetCount[Asset->GetTypeString()];
			AssetCount[Asset->GetTypeString()] = count + 1;
		}
		Stream << "\t\t\t],\n";

		//各类型资源的数量
		//Stream << "\t\t\t\"asset_count\": [\n";
		//for (map<string, INT32>::iterator AssetIt = AssetCount.begin(); AssetIt != AssetCount.end(); ++AssetIt)
		//{
		//	Stream 
		//		<< "\t\t\t\t{\n"
		//		<< "\t\t\t\t\t\"type\": \"" << AssetIt->first << "\",\n"
		//		<< "\t\t\t\t\t\"count\": " << AssetIt->second << "\n"
		//		<< "\t\t\t\t},\n";
		//}
		//Stream << "\t\t\t],\n";

		//依赖
		Stream << "\t\t\t\"dependencies\": [\n";
		set<string> Dependencies = LookupDependencies(It->second->GetName());
		for (set<string>::iterator DepIt = Dependencies.begin(); DepIt != Dependencies.end(); ++DepIt)
		{
			assets_file_info_ptr AssetsFileInfo = LookupAssetsFileInfo(*DepIt);
			if (AssetsFileInfo.get() && AssetsFileInfo->GetFileName().compare(It->second->GetFileName()) == 0) continue;
			Stream << "\t\t\t\t{\n";
			AppendAssetBundle(Stream, *DepIt, AssetsFileInfo, "\t\t\t\t\t");
			Stream << "\t\t\t\t},\n";
		}
		Stream << "\t\t\t],\n";

		//被依赖
		Stream << "\t\t\t\"be_depended\": [\n";
		Dependencies = LookupBeDepended(It->second->GetName());
		for (set<string>::iterator DepIt = Dependencies.begin(); DepIt != Dependencies.end(); ++DepIt)
		{
			assets_file_info_ptr AssetsFileInfo = LookupAssetsFileInfo(*DepIt);
			if (AssetsFileInfo.get() && AssetsFileInfo->GetFileName().compare(It->second->GetFileName()) == 0) continue;
			Stream << "\t\t\t\t{\n";
			AppendAssetBundle(Stream, *DepIt, AssetsFileInfo, "\t\t\t\t\t");
			Stream << "\t\t\t\t},\n";
		}
		Stream << "\t\t\t]\n";

		Stream << "\t\t},\n";
	}
	Stream << "\t],\n";

	AppendAssets(Stream, AssetsDetail);

	Stream
		<< "\t\"statistics\": {\n"
		<< "\t\t\"compressed_size\": " << m_nCompressedSize << ",\n"
		<< "\t\t\"decompressed_size\": " << m_nDecompressedSize << ",\n"
		<< "\t\t\"redu_assets_size\": " << m_nReduAssetsSize << ",\n"
		<< "\t},\n";


	Stream << "}";
	Stream.close();
	return Output;
}

/**
 * 生成报告页面
 *
 * @return 返回输出文件路径
 */
string CAssetBundlesParser::GenerateReport()
{
	string Output(m_OutputDirPath);
	Output.append("/report.html");
	m_HtmlGenerator.reset(new CHtmlGenerator(this));

	list<asset_detail_info_ptr> AssetsDetail;

	for (assets_file_info_map::iterator It = m_AssetsFileInfos.begin(); It != m_AssetsFileInfos.end(); ++It)
	{
		asset_info_map &Assets = It->second->GetAssets();
		map<string, INT32> AssetCount;
		for (asset_info_map::iterator AssetIt = Assets.begin(); AssetIt != Assets.end(); ++AssetIt)
		{
			const asset_info_ptr &Asset = AssetIt->second;
			if (AssetIt->second->IsAssetBundleAsset()) continue;
			asset_detail_info_ptr AssetDetailInfo(new ASSET_DETAIL_INFO());
			AssetDetailInfo->m_AssetInfo = Asset;
			AssetDetailInfo->m_OwnerName = It->second->GetName();
			AssetDetailInfo->m_OwnerFileName = It->second->GetFileName();
			AssetDetailInfo->m_OwnerAssetBundleName = It->second->GetAssetBundleName();
			m_HtmlGenerator->PutAsset(It->second->GetFileName(), Asset);
			AssetsDetail.push_back(AssetDetailInfo);
		}

		//添加依赖信息
		set<string> Dependencies = LookupDependencies(It->second->GetName());
		for (set<string>::iterator DepIt = Dependencies.begin(); DepIt != Dependencies.end(); ++DepIt)
		{
			assets_file_info_ptr AssetsFileInfo = LookupAssetsFileInfo(*DepIt);
			if (AssetsFileInfo.get() && AssetsFileInfo->GetFileName().compare(It->second->GetFileName()) == 0) continue;
			if (AssetsFileInfo.get())
			{
				if (AssetsFileInfo->GetFileName().compare(It->second->GetFileName()) == 0) continue;
			}
			else
			{
				AssetsFileInfo.reset(new CAssetsFileInfo());
				AssetsFileInfo->SetName(*DepIt);
			}
			m_HtmlGenerator->PutDependency(It->second->GetFileName(), AssetsFileInfo);
		}
	}

	AssetsDetail.sort(asset_detail_info_compare);
	for (list<asset_detail_info_ptr>::iterator It = AssetsDetail.begin(); It != AssetsDetail.end(); )
	{
		list<asset_detail_info_ptr> RedundAssets;
		RedundAssets.push_back(*It);

		list<asset_detail_info_ptr>::iterator NextIt = It;
		NextIt++;
		//找出冗余资源
		for (; NextIt != AssetsDetail.end();)
		{
			if (It->get()->m_AssetInfo->EqualTo(*NextIt->get()->m_AssetInfo))
			{
				RedundAssets.push_back(*NextIt);
				AssetsDetail.erase(NextIt++);
			}
			else
			{
				break;
			}
		}

		It = NextIt;
		if (RedundAssets.size() < 2) continue;

		ExportAssetFiles(RedundAssets);
		m_HtmlGenerator->PutReduAssets(RedundAssets.begin()->get()->m_AssetInfo->GetTypeString(), RedundAssets);
	}
	m_HtmlGenerator->Generate(Output);
	return Output;
}

/**
 * 解析目标文件夹下的AssetBundle
 *
 * @param pDirectoryPath AssetBundle所在目录
 * @return 返回解析后的结果文件
 */
string CAssetBundlesParser::Parse(const char * pDirectoryPath)
{
	Clear();
#ifndef MUTILE_THREAD
	UnpackAssetsBundleFiles(pDirectoryPath);
	LoadAssetBundles();
#else
	m_DirPath.assign(pDirectoryPath);
	size_t nIndex = 0;
	size_t nWorkThreads = m_WorkThreads.size();
	set<string> Files = ListDirectory(m_DirPath.c_str(), true, "*", MayBeAssetsBundle);
	for (set<string>::iterator It = Files.begin(); It != Files.end(); ++It)
	{
		m_WorkThreads[nIndex++ % nWorkThreads]->AddAssetBundleFile(Replace(*It, "\\", "/"));
	}
	for (nIndex = 0; nIndex < m_WorkThreads.size(); ++nIndex)
	{
		m_WorkThreads[nIndex]->Start();
	}
	for (nIndex = 0; nIndex < m_WorkThreads.size(); ++nIndex)
	{
		while (m_WorkThreads[nIndex]->IsRunning()) SLEEP(1000);
	}
#endif
	return GenerateReport();
}

/**
 * 查找资源文件信息
 *
 * @param AssetBundleName 资源包名字
 */
assets_file_info_ptr CAssetBundlesParser::LookupAssetsFileInfo(const string & AssetBundleName)
{
#ifdef MUTILE_THREAD
	CMutexGuard Lock(m_AssetsFileInfosMutex);
#endif
	assets_file_info_map::iterator It = m_AssetsFileInfos.find(AssetBundleName);
	return It == m_AssetsFileInfos.end() ? assets_file_info_ptr() : It->second;
}

/**
 * 查找依赖信息
 *
 * @param AssetBundleName 资源包名字
 */
set<string> CAssetBundlesParser::LookupDependencies(const string & AssetBundleName)
{
#ifdef MUTILE_THREAD
	CMutexGuard Lock(m_DepsMutex);
#endif
	dependencies_map::iterator It = m_Dependencies.find(AssetBundleName);
	return It == m_Dependencies.end() ? set<string>() : It->second;
}

/**
 * 查找被依赖信息
 *
 * @param AssetBundleName 资源包名字
 */
set<string> CAssetBundlesParser::LookupBeDepended(const string & AssetBundleName)
{
	dependencies_map::iterator It = m_BeDepended.find(AssetBundleName);
	return It == m_BeDepended.end() ? set<string>() : It->second;
}

/**
 * 查找重复的资源包文件
 *
 * @param AssetBundleName 资源包名字
 */
set<string> CAssetBundlesParser::LookupDuplicateAssetBundle(const string & AssetBundleName)
{
	duplicate_asset_bundle_map::iterator It = m_DuplicateAssetbundles.find(AssetBundleName);
	return It == m_DuplicateAssetbundles.end() ? set<string>() : It->second;
}

/**
 * 查找指定AssetBundle中的AssetsFile
 *
 * @param AssetBundleFile AssetBundle文件路径
 */
set<string> CAssetBundlesParser::LookupAssetsFile(const string & AssetBundleFile)
{
#ifdef MUTILE_THREAD
	CMutexGuard Lock(m_AssetBundlesMutex);
#endif
	asset_bundle_map::iterator It = m_AssetBundles.find(AssetBundleFile);
	return It == m_AssetBundles.end() ? set<string>() : It->second;
}

/**
 * 添加AssetsFile
 *
 * @param AssetBundleFile AssetBundle文件路径
 * @param AssetsFiles AssesFile集合
 */
void CAssetBundlesParser::PutAssetsFile(const string & AssetBundleFile, const set<string>& AssetsFiles)
{
#ifdef MUTILE_THREAD
	CMutexGuard Lock(m_AssetBundlesMutex);
#endif
	m_AssetBundles[AssetBundleFile] = AssetsFiles;
}

/**
 * 查找解压后的AssetBundle文件路径
 */
string CAssetBundlesParser::LookupDecompressedAssetBundleFile(const string & AssetBundleFile)
{
#ifdef MUTILE_THREAD
	CMutexGuard Lock(m_ABPathesMutex);
#endif	
	map<string, string>::iterator It = m_AssetsBundleFilePathes.find(AssetBundleFile);
	return It == m_AssetsBundleFilePathes.end() ? string() : It->second;
}

/**
 * 添加解压后的AssetBundle文件路径
 */
void CAssetBundlesParser::PutDecompressedAssetBundleFile(const string & AssetBundleFile, const string & DecompressedAssetBundle)
{
#ifdef MUTILE_THREAD
	CMutexGuard Lock(m_ABPathesMutex);
#endif	
	m_AssetsBundleFilePathes[AssetBundleFile] = DecompressedAssetBundle;
}

/**
 * 添加大小
 *
 * @param nDecompressedSize 解压后大小
 * @param nCompressedSize 压缩后大小
 */
void CAssetBundlesParser::AddSize(size_t nDecompressedSize, size_t nCompressedSize)
{
#ifdef MUTILE_THREAD
	CMutexGuard Lock(m_AssetBundleSizeMutex);
#endif
	m_nDecompressedSize += nDecompressedSize;
	m_nCompressedSize += nCompressedSize;
}

/**
 * 添加AssetsFile信息
 *
 * @param AssetBundleName AssetBundle名字
 * @param AssetsFileInfo AssetsFile信息
 */
void CAssetBundlesParser::PutAssetsFileInfo(const string & AssetBundleName, const assets_file_info_ptr & AssetsFileInfo)
{
#ifdef MUTILE_THREAD
	CMutexGuard Lock(m_AssetsFileInfosMutex);
#endif
	m_AssetsFileInfos[AssetBundleName] = AssetsFileInfo;
}

/**
 * 新增依赖
 *
 * @param AssetBundleName 资源包名
 * @param Dependencies 依赖集合
 */
void CAssetBundlesParser::PutDependencies(const string &AssetBundleName, const set<string> &Dependencies)
{
#ifdef MUTILE_THREAD
	CMutexGuard Lock(m_DepsMutex);
#endif
	m_Dependencies.insert(make_pair(AssetBundleName, Dependencies));
	for (set<string>::iterator It = Dependencies.begin(); It != Dependencies.end(); ++It)
	{
		dependencies_map::iterator It2 = m_BeDepended.find(*It);
		if (It2 == m_BeDepended.end())
		{
			set<string> kk;
			kk.insert(AssetBundleName);
			m_BeDepended[*It] = kk;
		}
		else
		{
			It2->second.insert(AssetBundleName);
		}
	}
}

/**
 * 添加重复的资源包文件
 *
 * @param AssetBundleName 资源包名
 * @param FileName 文件名
 */
void CAssetBundlesParser::PutDuplicateAssetBundle(const string &AssetBundleName, const string &FileName)
{
	duplicate_asset_bundle_map::iterator It = m_DuplicateAssetbundles.find(AssetBundleName);
	if (It == m_DuplicateAssetbundles.end())
	{
		set<string> Files;
		Files.insert(FileName);
		m_DuplicateAssetbundles[AssetBundleName] = Files;
	}
	else
	{
		It->second.insert(FileName);
	}
}

/**
 * 在文件流中追加资源文件信息
 *
 * @param Stream 文件流
 * @param Name AssetBundle名
 * @param AssetsFileInfo 资源文件信息
 * @param Prefix 每一行的前缀
 */
void CAssetBundlesParser::AppendAssetBundle(ofstream &Stream, const string &Name, const assets_file_info_ptr &AssetsFileInfo, const string &Prefix)
{
	if (AssetsFileInfo.get())
	{
		Stream
			<< Prefix << "\"name\":\"" << AssetsFileInfo->GetName() << "\",\n"
			<< Prefix << "\"file_name\":\"" << AssetsFileInfo->GetFileName() << "\",\n"
			<< Prefix << "\"size\":" << AssetsFileInfo->GetSize() << ", \n"
			<< Prefix << "\"asset_bundle_name\":\"" << AssetsFileInfo->GetAssetBundleName() << "\",\n"
			<< Prefix << "\"exists\":true,\n";
	}
	else
	{
		Stream
			<< Prefix << "\"name\":\"" << Name << "\",\n"
			<< Prefix << "\"exists\":false,\n";
	}
}

/**
 * 在文件流中追加资源信息
 *
 * @param Stream 文件流
 * @param AssetsDetail 资源信息列表
 */
void CAssetBundlesParser::AppendAssets(ofstream & Stream, list<asset_detail_info_ptr>& AssetsDetail)
{
	AssetsDetail.sort(asset_detail_info_compare);
	Stream << "\t\"assets\": [\n";
	for (list<asset_detail_info_ptr>::iterator It = AssetsDetail.begin(); It != AssetsDetail.end(); )
	{
		list<asset_detail_info_ptr> RedundAssets;
		RedundAssets.push_back(*It);

		list<asset_detail_info_ptr>::iterator NextIt = It;
		NextIt++;
		//找出冗余资源
		for (; NextIt != AssetsDetail.end();)
		{
			if (It->get()->m_AssetInfo->EqualTo(*NextIt->get()->m_AssetInfo))
			{
				RedundAssets.push_back(*NextIt);
				AssetsDetail.erase(NextIt++);
			}
			else
			{
				break;
			}
		}

		It = NextIt;
		if (RedundAssets.size() < 2) continue;

		ExportAssetFiles(RedundAssets);

		bool bFirst = true;
		for (list<asset_detail_info_ptr>::iterator It2 = RedundAssets.begin(); It2 != RedundAssets.end(); ++It2)
		{
			const asset_detail_info_ptr &Asset = *It2;

			Stream
				<< "\t\t{\n";
			AppendAsset(Stream, Asset, "\t\t\t");
			Stream
				<< "\t\t\t" << "\"redundancies\": [\n";
			QWORD nRedundSize = 0;
			for (list<asset_detail_info_ptr>::iterator It3 = RedundAssets.begin(); It3 != RedundAssets.end(); ++It3)
			{
				if (It3 == It2) continue;
				const asset_detail_info_ptr &Asset2 = *It3;

				Stream
					<< "\t\t\t\t{\n";
				AppendAsset(Stream, Asset2, "\t\t\t\t\t");
				Stream
					<< "\t\t\t\t\t" << "\"description\": \"" << GetAssetCompareResultString(Asset->m_AssetInfo->Compare(*Asset2->m_AssetInfo)) << "\",\n"
					<< "\t\t\t\t" << "},\n";
				if (bFirst)
				{
					nRedundSize += Asset2->m_AssetInfo->GetActualSize();
					m_nReduAssetsSize += nRedundSize;
				}
			}

			Stream << "\t\t\t" << "],\n";

			if (bFirst)
			{
				Stream << "\t\t\t" << "\"redund_size\" : " << nRedundSize << ",\n";
				bFirst = false;
			}

			Stream << "\t\t},\n";
		}
	}
	Stream << "\t],\n";
}

/**
 * 在文件流中追加资源信息
 * @param Stream 文件流
 * @param AssetDetailInfo 资源信息
 * @param Prefix 每一行的前缀
 */
void CAssetBundlesParser::AppendAsset(ofstream &Stream, const asset_detail_info_ptr &AssetDetailInfo, const string &Prefix)
{
	Stream
		<< Prefix << "\"name\": \"" << AssetDetailInfo->m_AssetInfo->GetName() << "\",\n"
		<< Prefix << "\"type\": \"" << AssetDetailInfo->m_AssetInfo->GetTypeString() << "\",\n"
		<< Prefix << "\"md5\": \"" << AssetDetailInfo->m_AssetInfo->GetMD5() << "\",\n"
		<< Prefix << "\"size\": " << AssetDetailInfo->m_AssetInfo->GetActualSize() << ",\n"
		<< Prefix << "\"file\": \"" << AnsiStrToUtf8Str(AssetDetailInfo->m_AssetInfo->GetFilePath().c_str()) << "\",\n"
		<< Prefix << "\"owner\": {\n"
		<< Prefix << "\t\"name\": \"" << AssetDetailInfo->m_OwnerName << "\",\n"
		<< Prefix << "\t\"file_name\": \"" << AssetDetailInfo->m_OwnerFileName << "\",\n"
		<< Prefix << "\t\"asset_bundle_name\": \"" << AssetDetailInfo->m_OwnerAssetBundleName << "\",\n"
		<< Prefix << "},\n";
}

/**
 * 导出资源文件
 *
 * @param AssetBundleFile 解压前的AssetBundle文件路径
 * @param AssetsFileName 要导出的资源所属的AssetsFile名字
 * @param uiAssetID 资源ID
 * @return 返回资源文件路径
 */
string CAssetBundlesParser::ExportAssetFile(const string & AssetBundleFile, const string & AssetsFileName, UINT64 uiAssetID)
{
	const string & DecompressFilePath = LookupDecompressedAssetBundleFile(AssetBundleFile);
	if (DecompressFilePath.empty()) return DecompressFilePath;

	const string & AssetsDirPath = GetAssetsDirPath(AssetBundleFile);
	try
	{
		return m_AssetsBundle->ExportAssetFileTo(DecompressFilePath.c_str(), AssetsFileName.c_str(), uiAssetID, AssetsDirPath.c_str());
	}
	catch (CUnityError & e)
	{
		GetLogService()->Warn(GetName(), _T("Unabled to export asset file %llu from %s(%hs): %hs")
			, uiAssetID, AssetsFileName.c_str(), AssetBundleFile.c_str(), e.what());
		return string();
	}
}

/**
 * 导出队列中的所有资源文件
 *
 * @param AssetsDetail 资源详情队列
 */
void CAssetBundlesParser::ExportAssetFiles(const list<asset_detail_info_ptr>& AssetsDetail)
{
	for (list<asset_detail_info_ptr>::const_iterator It = AssetsDetail.begin(); It != AssetsDetail.end(); ++It)
	{
		const asset_detail_info_ptr &Asset = *It;
		const string & AssetFilePath = ExportAssetFile(Asset->m_OwnerFileName, Asset->m_OwnerName, Asset->m_AssetInfo->GetPathID());
		Asset->m_AssetInfo->SetFilePath(ConvertAssetPathToRelativePath(AssetFilePath));
	}

}

CAssetBundlesParser::CWorkThread::CWorkThread(CAssetBundlesParser * pOwner)
{
	m_pOwner = pOwner;
	m_AssetsBundle.reset(new CAssetsBundleTools(GetClassDatabaseManager()));
}

/**
 * 添加AssetBundle文件路径
 *
 * @param AssetBundleFile AssetBundle文件路径
 */
CAssetBundlesParser::CWorkThread::~CWorkThread()
{
	Terminate();
	m_AssetsBundle.reset();
}

void CAssetBundlesParser::CWorkThread::AddAssetBundleFile(const string & AssetBundleFile)
{
	m_AssetBundleFiles.insert(m_AssetBundleFiles.end(), AssetBundleFile);
}

/**
 * @copydoc CThread::Run
 */
void CAssetBundlesParser::CWorkThread::Run()
{
	try
	{
		for (size_t i = 0; !IsTerminating() && i < m_AssetBundleFiles.size();++i)
		{
			string RelativePath = m_AssetBundleFiles[i].substr(m_pOwner->m_DirPath.size() + 1);
			const string & OutputFile = m_pOwner->UnpackAssetBundleFile(m_AssetsBundle, m_AssetBundleFiles[i]);
			if (OutputFile.empty()) continue;
			m_pOwner->LoadAssetBundle(m_AssetsBundle, RelativePath, OutputFile);
		}
	}
	catch (exception & e)
	{
		GetLogService()->Error(m_pOwner->GetName(), _T("WorkThread occur fatal error: %hs"), e.what());
	}
	catch (...)
	{
		GetLogService()->Error(m_pOwner->GetName(), _T("WorkThread occur fatal error!"));
	}
}
