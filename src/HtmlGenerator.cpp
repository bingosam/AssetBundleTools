#include <stdafx.h>

#include <codecvt>

#include "svcdef.h"
#include "AssetBundlesParser.h"

#define ALL_REDU_VAR L"__all"

/**
 * ���캯��
 *
 * @param pOwner �����Ľ�����
 */
CAssetBundlesParser::CHtmlGenerator::CHtmlGenerator(CAssetBundlesParser * pOwner)
{
	m_pOwner = pOwner;

	//д��Utf8��ʽ��֧������
	m_Stream.imbue(locale(std::locale(), new  std::codecvt_utf8<wchar_t>));
	//����2λС��
	m_Stream.precision(2);
	m_Stream.setf(std::ios::fixed);

	m_DefaultSelected.assign(ALL_REDU_VAR);
}

/**
 * ��������
 */
CAssetBundlesParser::CHtmlGenerator::~CHtmlGenerator()
{
}

/**
 * ���������Դ
 *
 * @param AssetType ��Դ����
 * @param ReduAssets ������Դ����
 */
void CAssetBundlesParser::CHtmlGenerator::PutReduAssets(string AssetType, const list<asset_detail_info_ptr>& ReduAssets)
{
	if (ReduAssets.size() < 2) return;
	wstring Teyp = AnsiStrToWideStr(AssetType.c_str());
	m_ReduAssets.insert(make_pair(Teyp, ReduAssets));
	m_ReduAssetTypes.insert(Teyp);
}

/**
 * �������
 *
 * @param Name ��Դ������������ABName��FileName��Name��
 * @param AssetsFileInfo AssetsFile��Ϣ
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
 * �����Դ
 *
 * @param AssetBundleName  ��Դ������AssetBundle��
 * @param AssetInfo ��Դ��Ϣ
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
 * ����html�ļ�
 *
 * @param FilePath �ļ�·��
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
 * ��ʼ���ļ�ͷ
 */
void CAssetBundlesParser::CHtmlGenerator::InitHead()
{
	m_Stream
		<< L"<head>\n"
		<< L"<meta http-equiv=\"X-UA-Compatible\" content=\"IE-edge,chrome=1\">\n"
		<< L"<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">\n"
		<< L"<meta charset=\"utf-8\">\n"
		<< L"<title>����</title>\n"
		<< L"<link href=\"Content/layout.css\" rel=\"stylesheet\" type=\"text/css\">\n"
		<< L"<script src=\"Content/jquery.js\" type=\"text/javascript\"></script>\n"
		<< L"<script src=\"Content/parser.js\" type=\"text/javascript\"></script>\n"
		<< L"</head>\n\n";
}

/**
 * ���html body
 */
void CAssetBundlesParser::CHtmlGenerator::AppendBody()
{
	m_Stream
		<< L"<body>\n"
		<< L"\t<div class=\"to-top\">��</div>\n"
		<< L"\t<h1>AssetBundle��Դ��ⱨ��</h1>\n";
	if (m_Assets.empty())
	{
		m_Stream << L"\t<h2 style=\"color:red;margin-left:20px;\">��ⲻ����Դ</h2>\n";
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
 * ���ժҪ
 */
void CAssetBundlesParser::CHtmlGenerator::AppendDigest()
{
	m_Stream
		<< L"\t<div id=\"result-overview\">\n"
		<< L"\t\t<h3>�������</h3>\n"
		<< L"\t\t<table cellspacing=\"0\" cellpadding=\"0\">\n"
		<< L"\t\t\t<tbody>\n"
		<< L"\t\t\t\t<tr>\n"
		<< L"\t\t\t\t\t<td width=\"250\"><span>��ѹǰAssetBundle�ܴ�С��</span></td>\n"
		<< L"\t\t\t\t\t<td><i>" << ConvertUnitW(m_pOwner->m_nCompressedSize) << L"</i></td>\n"
		<< L"\t\t\t\t\t<td width=\"250\"><span>��ѹ��AssetBundle�ܴ�С��</span></td>\n"
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
		<< L"\t\t\t\t\t<td><span>������Դ��С��</span></td>\n"
		<< L"\t\t\t\t\t<td class=\"" << TdClass << L"\"><i>" << ConvertUnitW(m_uiReduAssetsSize) << L"</i></td>\n"
		<< L"\t\t\t\t\t<td><span>ռ��ѹ���AssetBundle��</span></td>\n"
		<< L"\t\t\t\t\t<td class=\"" << TdClass << L"\"><i>" << fPercent << L"%</i></td>\n"
		<< L"\t\t\t\t</tr>\n"
		<< L"\t\t\t\t<tr>\n"
		<< L"\t\t\t\t\t<td colspan=\"4\"><span>ȱʧ��AssetBundle�ļ���</span>\n";

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
 * ���������Ϣ
 */
void CAssetBundlesParser::CHtmlGenerator::AppendRedus()
{
	m_Stream
		<< L"\t<div id=\"redundancies\">\n"
		<< L"\t\t<h3><i>+</i>AssetBundle��Դ�������</h3>\n"
		<< L"\t\t<div class=\"table-con\">\n";

	if (m_ReduAssetTypes.empty())
	{
		m_Stream
			<< L"\t\t\t<div class=\"no-tag\">û�з���������Դ</div>\n"
			<< L"\t\t</div>\n"
			<< L"\t</div>\n";
		return;
	}

	m_Stream
		<< L"\t\t\t<div class=\"query\"><span>��Դ��ѯ��</span>\n"
		<< L"\t\t\t\t<select>\n"
		<< L"\t\t\t\t\t<option value=\"" << ALL_REDU_VAR << "\"" << (m_DefaultSelected == ALL_REDU_VAR ? L" selected=\"selected\"" : L"") << L">������Դ</option>\n";
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
 * ���������Ϣ
 */
void CAssetBundlesParser::CHtmlGenerator::AppendDependencies()
{
	m_Stream
		<< L"\t<div id=\"dependPackage\">\n"
		<< L"\t\t<h3><i>+</i>AssetBundle�ļ�������������</h3>\n"
		<< L"\t\t<div class=\"table-con\">\n";

	if (m_Dependencies.empty())
	{
		m_Stream
			<< L"\t\t\t<div class=\"no-tag\">û�з���������Ϣ</div>\n"
			<< L"\t\t</div>\n"
			<< L"\t</div>\n";
		return;
	}

	m_Stream
		<< L"\t\t\t<table cellspacing=\"0\" cellpadding=\"0\">\n"
		<< L"\t\t\t\t<tbody>\n"
		<< L"\t\t\t\t\t<tr><th>AssetBundle����</th><th>��������</th><th>���������ļ�</th></tr>\n";

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
				Stream << AssetsFileInfo->GetName() << "<i style=\"color:red\">(ȱʧ)</i>";
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
 * �����Դ��Ϣ
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
		<< L"\t\t<h3><i>+</i>AssetBundle�ļ���Դʹ�����</h3>\n"
		<< L"\t\t<div class=\"table-con\">\n"
		<< L"\t\t\t<table cellspacing=\"0\" cellpadding=\"0\">\n"
		<< L"\t\t\t\t<tbody>\n"
		<< L"\t\t\t\t\t<tr><th>AssetBundle����</th>";
		
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
 * ��ӽű�
 */
void CAssetBundlesParser::CHtmlGenerator::AppendScript()
{
	m_Stream << L"<script>\n";
	AppendReduAssetsSelectScript();
	m_Stream << L"</script>\n";
}

/**
 * ���������Դѡ��ű�
 */
void CAssetBundlesParser::CHtmlGenerator::AppendReduAssetsSelectScript()
{
	if (m_ReduAssetTypes.empty()) return;

	m_Stream
		<< L"$(function() {\n";

	m_Stream
		<< L"\tvar __title = '<tr><th width=\"300\">��Դ����</th><th>��Դ����</th><th>��Դ����</th><th>��������</th><th>������Դ��С</th><th>������Դ</th><th>������Դ����</th><th>��������</th></tr>'\n";

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
 * ���������Դ���е��ļ���
 *
 * @param Assets ��Դ����
 */
void CAssetBundlesParser::CHtmlGenerator::AppendReduAssets(list<asset_detail_info_ptr> Assets)
{
	if (Assets.size() < 2) return;

	list<asset_detail_info_ptr>::iterator It = Assets.begin();
	asset_detail_info_ptr Asset;

	//����������Դ��С
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
 * ���������Դ���ļ���
 *
 * @param Asset ��Դ��Ϣ
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
		File = FormatStr("<br> <a href=\"%hs\" target=\"_blank\">�鿴��Դ�ļ�</a>", Asset->m_AssetInfo->GetFilePath().c_str());
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
 * ��λ����
 *
 * @param uiByteSize �ֽ���
 * @return ����������
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
 * ��λ����
 *
 * @param uiByteSize �ֽ���
 * @return ����������
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
