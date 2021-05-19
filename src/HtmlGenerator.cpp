#include <stdafx.h>

#include <codecvt>

#include "svcdef.h"
#include "AssetBundlesParser.h"

#define ALL_REDU_VAR L"__all"

/**
 * 构造函数
 *
 * @param pOwner 所属的解析器
 */
CAssetBundlesParser::CHtmlGenerator::CHtmlGenerator(CAssetBundlesParser * pOwner)
{
	m_pOwner = pOwner;

	//写入Utf8格式，支持中文
	m_Stream.imbue(locale(std::locale(), new  std::codecvt_utf8<wchar_t>));
	//保留2位小数
	m_Stream.precision(2);
	m_Stream.setf(std::ios::fixed);

	m_DefaultSelected.assign(ALL_REDU_VAR);
}

/**
 * 析构函数
 */
CAssetBundlesParser::CHtmlGenerator::~CHtmlGenerator()
{
}

/**
 * 添加冗余资源
 *
 * @param AssetType 资源类型
 * @param ReduAssets 冗余资源队列
 */
void CAssetBundlesParser::CHtmlGenerator::PutReduAssets(string AssetType, const list<asset_detail_info_ptr>& ReduAssets)
{
	if (ReduAssets.size() < 2) return;
	wstring Teyp = AnsiStrToWideStr(AssetType.c_str());
	m_ReduAssets.insert(make_pair(Teyp, ReduAssets));
	m_ReduAssetTypes.insert(Teyp);
}

/**
 * 添加依赖
 *
 * @param Name 资源包名（可能是ABName、FileName、Name）
 * @param AssetsFileInfo AssetsFile信息
 */
void CAssetBundlesParser::CHtmlGenerator::PutDependency(string Name, const assets_file_info_ptr & AssetsFileInfo)
{
	deps_map::iterator It = m_Dependencies.find(Name);
	if (It != m_Dependencies.end())
	{
		It->second.push_back(AssetsFileInfo);
	}
	else
	{
		list<assets_file_info_ptr> AssetsFiles;
		AssetsFiles.push_back(AssetsFileInfo);
		m_Dependencies[Name] = AssetsFiles;
	}
	if (AssetsFileInfo->GetFileName().empty())
	{
		m_MissingDependencies.insert(AssetsFileInfo->GetName());
	}
}

/**
 * 添加资源
 *
 * @param AssetBundleName  资源所属的AssetBundle名
 * @param AssetInfo 资源信息
 */
void CAssetBundlesParser::CHtmlGenerator::PutAsset(string AssetBundleName, const asset_info_ptr & AssetInfo)
{
	assets_map::iterator It = m_Assets.find(AssetBundleName);
	if (It != m_Assets.end())
	{
		It->second.insert(make_pair(AssetInfo->GetTypeString(), AssetInfo));
	}
	else
	{
		multimap<string, asset_info_ptr> Assets;
		Assets.insert(make_pair(AssetInfo->GetTypeString(), AssetInfo));
		m_Assets[AssetBundleName] = Assets;
	}
}

/**
 * 生成html文件
 *
 * @param FilePath 文件路径
 */
void CAssetBundlesParser::CHtmlGenerator::Generate(const string & FilePath)
{
	CreateDirectoryRecursion(FilePath.c_str(), false);
	m_Stream.open(FilePath.c_str());
	m_uiReduAssetsSize = 0;
	InitHead();
	AppendScript();
	AppendBody();
	m_Stream.close();
}

/**
 * 初始化文件头
 */
void CAssetBundlesParser::CHtmlGenerator::InitHead()
{
	m_Stream
		<< L"<head>\n"
		<< L"<meta http-equiv=\"X-UA-Compatible\" content=\"IE-edge,chrome=1\">\n"
		<< L"<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">\n"
		<< L"<meta charset=\"utf-8\">\n"
		<< L"<title>报告</title>\n"
		<< L"<link href=\"Content/layout.css\" rel=\"stylesheet\" type=\"text/css\">\n"
		<< L"<script src=\"Content/jquery.js\" type=\"text/javascript\"></script>\n"
		<< L"<script src=\"Content/parser.js\" type=\"text/javascript\"></script>\n"
		<< L"</head>\n\n";
}

/**
 * 添加html body
 */
void CAssetBundlesParser::CHtmlGenerator::AppendBody()
{
	m_Stream
		<< L"<body>\n"
		<< L"\t<div class=\"to-top\">△</div>\n"
		<< L"\t<h1>AssetBundle资源检测报告</h1>\n";
	if (m_Assets.empty())
	{
		m_Stream << L"\t<h2 style=\"color:red;margin-left:20px;\">检测不到资源</h2>\n";
	}
	else
	{
		AppendDigest();
		AppendRedus();
		AppendDependencies();
		AppendAssetsInfo();
	}
	m_Stream
		<< L"</body>\n";
}

/**
 * 添加摘要
 */
void CAssetBundlesParser::CHtmlGenerator::AppendDigest()
{
	m_Stream
		<< L"\t<div id=\"result-overview\">\n"
		<< L"\t\t<h3>结果概述</h3>\n"
		<< L"\t\t<table cellspacing=\"0\" cellpadding=\"0\">\n"
		<< L"\t\t\t<tbody>\n"
		<< L"\t\t\t\t<tr>\n"
		<< L"\t\t\t\t\t<td width=\"250\"><span>解压前AssetBundle总大小：</span></td>\n"
		<< L"\t\t\t\t\t<td><i>" << ConvertUnitW(m_pOwner->m_nCompressedSize) << L"</i></td>\n"
		<< L"\t\t\t\t\t<td width=\"250\"><span>解压后AssetBundle总大小：</span></td>\n"
		<< L"\t\t\t\t\t<td><i>" << ConvertUnitW(m_pOwner->m_nDecompressedSize) << L"</i></td>\n"
		<< L"\t\t\t\t</tr>\n";

	float fPercent = m_uiReduAssetsSize * 100.0f / m_pOwner->m_nDecompressedSize;
	wstring TdClass(L"td-normal");
	if (fPercent > 10.0f)
	{
		TdClass.assign(L"td-marked");
	}

	m_Stream
		<< L"\t\t\t\t<tr class=\"tr-bg\">\n"
		<< L"\t\t\t\t\t<td><span>冗余资源大小：</span></td>\n"
		<< L"\t\t\t\t\t<td class=\"" << TdClass << L"\"><i>" << ConvertUnitW(m_uiReduAssetsSize) << L"</i></td>\n"
		<< L"\t\t\t\t\t<td><span>占解压后的AssetBundle：</span></td>\n"
		<< L"\t\t\t\t\t<td class=\"" << TdClass << L"\"><i>" << fPercent << L"%</i></td>\n"
		<< L"\t\t\t\t</tr>\n"
		<< L"\t\t\t\t<tr>\n"
		<< L"\t\t\t\t\t<td colspan=\"4\"><span>缺失的AssetBundle文件：</span>\n";

	ostringstream Stream;
	set<string>::iterator It = m_MissingDependencies.begin();
	int i = 0;
	if (It != m_MissingDependencies.end())
	{
		Stream << *It;
		It++;
		i++;
	}
	for (; It != m_MissingDependencies.end(); ++It, ++i)
	{
		Stream << (i % 3 == 0 ? "<br>" : " , ") << *It;
	}
	m_Stream
		<< FormatStr(L"%hs", Stream.str().c_str())
		<< L"\t\t\t\t\t</td>\n"
		<< L"\t\t\t\t</tr>\n"
		<< L"\t\t\t</tbody>\n"
		<< L"\t\t</table>\n"
		<< L"\t</div>\n";


}

/**
 * 添加冗余信息
 */
void CAssetBundlesParser::CHtmlGenerator::AppendRedus()
{
	m_Stream
		<< L"\t<div id=\"redundancies\">\n"
		<< L"\t\t<h3><i>+</i>AssetBundle资源冗余情况</h3>\n"
		<< L"\t\t<div class=\"table-con\">\n";

	if (m_ReduAssetTypes.empty())
	{
		m_Stream
			<< L"\t\t\t<div class=\"no-tag\">没有发现冗余资源</div>\n"
			<< L"\t\t</div>\n"
			<< L"\t</div>\n";
		return;
	}

	m_Stream
		<< L"\t\t\t<div class=\"query\"><span>资源查询：</span>\n"
		<< L"\t\t\t\t<select>\n"
		<< L"\t\t\t\t\t<option value=\"" << ALL_REDU_VAR << "\"" << (m_DefaultSelected == ALL_REDU_VAR ? L" selected=\"selected\"" : L"") << L">所有资源</option>\n";
	set<wstring>::iterator It = m_ReduAssetTypes.begin();
	for (; It != m_ReduAssetTypes.end(); ++It)
	{
		m_Stream << L"\t\t\t\t\t<option" << (m_DefaultSelected == *It ? L" selected=\"selected\"" : L"") << L">" << *It << L"</option>\n";
	}
	m_Stream
		<< L"\t\t\t\t</select>\n"
		<< L"\t\t\t</div>\n"
		<< L"\t\t\t<table cellspacing=\"0\" cellpadding=\"0\"></table>\n"
		<< L"\t\t</div>\n"
		<< L"\t</div>\n";
}

/**
 * 添加依赖信息
 */
void CAssetBundlesParser::CHtmlGenerator::AppendDependencies()
{
	m_Stream
		<< L"\t<div id=\"dependPackage\">\n"
		<< L"\t\t<h3><i>+</i>AssetBundle文件的依赖打包情况</h3>\n"
		<< L"\t\t<div class=\"table-con\">\n";

	if (m_Dependencies.empty())
	{
		m_Stream
			<< L"\t\t\t<div class=\"no-tag\">没有发现依赖信息</div>\n"
			<< L"\t\t</div>\n"
			<< L"\t</div>\n";
		return;
	}

	m_Stream
		<< L"\t\t\t<table cellspacing=\"0\" cellpadding=\"0\">\n"
		<< L"\t\t\t\t<tbody>\n"
		<< L"\t\t\t\t\t<tr><th>AssetBundle名称</th><th>依赖数量</th><th>具体依赖文件</th></tr>\n";

	ostringstream Stream;
	deps_map::iterator It = m_Dependencies.begin();
	for (; It != m_Dependencies.end(); ++It)
	{
		list<assets_file_info_ptr> Deps = It->second;
		Stream
			<< "\t\t\t\t\t<tr>\n"
			<< "\t\t\t\t\t\t<td>" << It->first << "</td>\n"
			<< "\t\t\t\t\t\t<td>" << Deps.size() << "</td>\n"
			<< "\t\t\t\t\t\t<td>";

		list<assets_file_info_ptr>::iterator DepIt = Deps.begin();
		for (; DepIt != Deps.end(); ++DepIt)
		{
			assets_file_info_ptr AssetsFileInfo = *DepIt;
			if (AssetsFileInfo->GetFileName().empty())
			{
				Stream << AssetsFileInfo->GetName() << "<i style=\"color:red\">(缺失)</i>";
			}
			else
			{
				Stream << AssetsFileInfo->GetFileName();
			}
			Stream << "<br>";

		}
		Stream << "</td>\n";
	}

	m_Stream << FormatStr(L"%hs", Stream.str().c_str());
	m_Stream
		<< L"\t\t\t\t</tbody>\n"
		<< L"\t\t\t</table>\n"
		<< L"\t\t</div>\n"
		<< L"\t</div>\n";

}

/**
 * 添加资源信息
 */
void CAssetBundlesParser::CHtmlGenerator::AppendAssetsInfo()
{
	static const char s_Type[][16] = 
	{
		"Mesh",
		"Material",
		"Texture2D",
		"Shader",
	};
	static const size_t s_iTypeSize = sizeof(s_Type) / sizeof(s_Type[0]);

	m_Stream
		<< L"\t<div id=\"packageName\">\n"
		<< L"\t\t<h3><i>+</i>AssetBundle文件资源使用情况</h3>\n"
		<< L"\t\t<div class=\"table-con\">\n"
		<< L"\t\t\t<table cellspacing=\"0\" cellpadding=\"0\">\n"
		<< L"\t\t\t\t<tbody>\n"
		<< L"\t\t\t\t\t<tr><th>AssetBundle名称</th>";
		
	for (size_t i = 0; i < s_iTypeSize; ++i)
	{
		m_Stream << L"<th>" << s_Type[i] << "</th>";
	}
	m_Stream << L"</tr>\n";

	assets_map::iterator It = m_Assets.begin();
	for (; It != m_Assets.end(); ++It)
	{
		multimap<string, asset_info_ptr> Assets = It->second;
		m_Stream << L"\t\t\t\t\t<tr><td>" << AnsiStrToWideStr(It->first.c_str()) << L"</td>";
		for (size_t i = 0; i < s_iTypeSize; ++i)
		{
			size_t iS = Assets.count(s_Type[i]);
			m_Stream << L"<td>" << Assets.count(s_Type[i]) << L"</td>";
		}
		m_Stream << L"</tr>\n";
	}

	m_Stream
		<< L"\t\t\t\t</tbody>\n"
		<< L"\t\t\t</table>\n"
		<< L"\t\t</div>\n";
}

/**
 * 添加脚本
 */
void CAssetBundlesParser::CHtmlGenerator::AppendScript()
{
	m_Stream << L"<script>\n";
	AppendReduAssetsSelectScript();
	m_Stream << L"</script>\n";
}

/**
 * 添加冗余资源选择脚本
 */
void CAssetBundlesParser::CHtmlGenerator::AppendReduAssetsSelectScript()
{
	if (m_ReduAssetTypes.empty()) return;

	m_Stream
		<< L"$(function() {\n";

	m_Stream
		<< L"\tvar __title = '<tr><th width=\"300\">资源名称</th><th>资源类型</th><th>资源包名</th><th>冗余数量</th><th>冗余资源大小</th><th>冗余资源</th><th>冗余资源包名</th><th>冗余详情</th></tr>'\n";

	wostringstream AllAssets;
	set<wstring>::iterator It = m_ReduAssetTypes.begin();
	typedef redu_assets_map::iterator Iterator;
	int i = 0;
	for (; It != m_ReduAssetTypes.end(); ++It, ++i)
	{
		pair<Iterator, Iterator> Pos = m_ReduAssets.equal_range(*It);
		m_Stream << L"\tvar " << *It << L"='";
		while (Pos.first != Pos.second)
		{
			AppendReduAssets(Pos.first->second);
			Pos.first++;
		}
		m_Stream << L"';\n";

		AllAssets << (i > 0 ? L" + " : L"") << *It;
		if ((*It) == L"Texture2D") {
			m_DefaultSelected.assign(*It);
		}
	}
	m_Stream << L"\tvar " << ALL_REDU_VAR << L" = " << AllAssets.str();

	m_Stream
		<< L";\n"
		<< L"\t$(\"#redundancies table\").html(__title+" << m_DefaultSelected << L");\n";

	m_Stream
		<< L"\tselectChange();\n"
		<< L"\tfunction selectChange() {\n"
		<< L"\t\t$(\"select\").change(function() {\n";
	m_Stream
		<< L"\t\t\tvar selectedOption=$(\"select\").val();\n"
		<< L"\t\t\t$(\"#redundancies table\").html(__title + eval(selectedOption));\n";

	m_Stream
		<< L"\t\t})\n"	//$(\"select\").change(function() {
		<< L"\t}\n"		//function selectChange() {
		<< L"});\n";	//$(function(){
}

/**
 * 添加冗余资源队列到文件流
 *
 * @param Assets 资源队列
 */
void CAssetBundlesParser::CHtmlGenerator::AppendReduAssets(list<asset_detail_info_ptr> Assets)
{
	if (Assets.size() < 2) return;

	list<asset_detail_info_ptr>::iterator It = Assets.begin();
	asset_detail_info_ptr Asset;

	//计算冗余资源大小
	if (It != Assets.end())
	{
		It++;
	}

	UINT64 uiReduAssetsSize = 0;
	for (; It != Assets.end(); ++It)
	{
		uiReduAssetsSize += (*It)->m_AssetInfo->GetActualSize();
	}
	m_uiReduAssetsSize += uiReduAssetsSize;

	//
	It = Assets.begin();
	Asset = *It;
	int nRowSpan = Assets.size() - 1;
	m_Stream << L"<tr>";
	AppendReduAsset(Asset, Assets.size() - 1);
	m_Stream
		<< L"<td rowspan=\"" << nRowSpan << L"\">" << nRowSpan << L"</td>"
		<< L"<td rowspan=\"" << nRowSpan << L"\">" << ConvertUnitW(uiReduAssetsSize) << L"</td>";
	It++;
	
	AppendReduAsset(*It);
	m_Stream
		<< L"<td>" << GetAssetCompareResultDescW(Asset->m_AssetInfo->Compare(*(*It)->m_AssetInfo)) << L"</td>"
		<< L"</tr>";
	It++;

	for (; It != Assets.end(); ++It)
	{
		m_Stream
			<< L"<tr>";
		AppendReduAsset(*It);
		m_Stream
			<< L"<td>" << GetAssetCompareResultDescW(Asset->m_AssetInfo->Compare(*(*It)->m_AssetInfo)) << L"</td>"
			<< L"</tr>";
	}
}

/**
 * 添加冗余资源到文件流
 *
 * @param Asset 资源信息
 */
void CAssetBundlesParser::CHtmlGenerator::AppendReduAsset(const asset_detail_info_ptr & Asset, int nRowSpan /* = 0*/)
{	
	string Type;
	if(nRowSpan > 0)
	{
		Type = FormatStr("<td rowspan=\"%d\">%s</td>", nRowSpan, Asset->m_AssetInfo->GetTypeString().c_str());
	}
	else
	{
		nRowSpan = 1;
	}

	string File;
	if(!Asset->m_AssetInfo->GetFilePath().empty())
	{
		File = FormatStr("<br> <a href=\"%hs\" target=\"_blank\">查看资源文件</a>", Asset->m_AssetInfo->GetFilePath().c_str());
	}

	m_Stream
		<<
		FormatStr(L"<td rowspan=\"%d\">%hs</td>"
					L"%hs"
					L"<td rowspan=\"%d\">%hs%hs</td>"
			, nRowSpan, (Asset->m_AssetInfo->GetName().empty() ? "Unnamed" : Asset->m_AssetInfo->GetName().c_str())
			, Type.c_str()
			, nRowSpan, Asset->m_OwnerFileName.c_str(), File.c_str()
			);
}

static const char gs_szUnit[][6] = {
	"byte",
	"KB",
	"MB",
	"GB"
};
static const int gs_nLen = sizeof(gs_szUnit) / sizeof(gs_szUnit[0]);

/**
 * 单位换算
 *
 * @param uiByteSize 字节数
 * @return 换算后的数据
 */
string CAssetBundlesParser::CHtmlGenerator::ConvertUnitA(UINT64 uiByteSize)
{
	string result;
	float fTemp = uiByteSize / 1.0f;
	int i = 0;
	for (; i < gs_nLen && fTemp > 1024.0f; ++i)
	{
		fTemp = fTemp / 1024.0f;
	}

	return 0 == i ?
		FormatStr("%u(%s)", uiByteSize, gs_szUnit[i])
		: FormatStr("%0.2f(%s)", fTemp, gs_szUnit[i]);
}

/**
 * 单位换算
 *
 * @param uiByteSize 字节数
 * @return 换算后的数据
 */
wstring CAssetBundlesParser::CHtmlGenerator::ConvertUnitW(UINT64 uiByteSize)
{
	wstring result;
	float fTemp = uiByteSize / 1.0f;
	int i = 0;
	for (; i < gs_nLen && fTemp > 1024.0f; ++i)
	{
		fTemp = fTemp / 1024.0f;
	}

	return 0 == i ?
		FormatStr(L"%u(%hs)", uiByteSize, gs_szUnit[i])
		: FormatStr(L"%0.2f(%hs)", fTemp, gs_szUnit[i]);
}
