#include "StdAfx.h"
#include "uabedef.h"
#include  "AssetBundleInfo.h"

/**
 * 获取比较结果的字符串信息
 *
 * @return result 比较结果
 */
const char * GetAssetCompareResultString(ASSET_COMPARE_RESULT result)
{
	switch (result)
	{
#define CASE(X) case X: return #X
		CASE(ASSET_DIFFERENCE);
		CASE(ASSET_THE_SAME);
		CASE(ASSET_EQUAL);
		CASE(ASSET_DIFFERENT_NAME);
#undef CASE
	default: return "UNKNOW_COMPARE_RESULT";
	}
}

/**
* 获取比较结果的描述
*
* return 比较结果的描述信息
*/
const char * GetAssetCompareResultDesc(ASSET_COMPARE_RESULT result)
{
	switch (result)
	{
	case ASSET_DIFFERENCE: return "不同资源";
	case ASSET_THE_SAME: return "同一资源文件";
	case ASSET_EQUAL: return "资源数据相同,文件不同";
	case ASSET_DIFFERENT_NAME: return "资源数据相同,资源名不同";
	default: return "UNKNOW_COMPARE_RESULT";
	}
}

/**
 * 获取比较结果的描述
 *
 * return 比较结果的描述信息
 */
const wchar_t * GetAssetCompareResultDescW(ASSET_COMPARE_RESULT result)
{
	switch (result)
	{
	case ASSET_DIFFERENCE: return L"不同资源";
	case ASSET_THE_SAME: return L"同一资源文件";
	case ASSET_EQUAL: return L"资源数据相同,文件不同";
	case ASSET_DIFFERENT_NAME: return L"资源数据相同,资源名不同";
	default: return L"UNKNOW_COMPARE_RESULT";
	}
}


/**
 * 构造函数
 *
 * @param pName 资源名字
 * @param uPathID 资源路径ID
 * @param nType 资源类型
 * @param type 资源类型字符串
 * @param nSize 资源大小
 */
CAssetInfo::CAssetInfo(const char *pName, unsigned __int64 uPathID, DWORD nType, string type, DWORD nSize)
	: m_Name(pName)
	, m_uPathID(uPathID)
	, m_nType(nType)
	, m_Type(type)
	, m_nSize(nSize)
	, m_nResourceSize(0)
	, m_FileName(pName)

{
}

CAssetInfo::~CAssetInfo()
{
}

/**
 * 判断资源是否相等
 *
 * @param Other 其他资源
 */
bool CAssetInfo::EqualTo(const CAssetInfo &Other)
{
	if (Other.GetType() != GetType()) return false;
	return strcmp(Other.GetMD5().c_str(), GetMD5().c_str()) == 0;
}

/**
 * 判断是否为AssetBundle资源
 */
bool CAssetInfo::IsAssetBundleAsset()
{
	return strcmp(GetTypeString().c_str(), "AssetBundle") == 0;
}

/**
 * 重载小于号
 *
 * @param Other 另一个对象
 * @return 是否小于Other
 */
bool CAssetInfo::operator<(const CAssetInfo & Other) const
{
	if (GetType() < Other.GetType()) return true;
	if (GetType() > Other.GetType()) return false;

	int nRet = strcmp(GetMD5().c_str(), Other.GetMD5().c_str());
	if (nRet != 0) return nRet < 0;

	return strcmp(GetName().c_str(), Other.GetName().c_str()) < 0;
}

/**
 * 与另一个资源进行比较
 *
 * @param Other 其他资源
 * @return 比较结果 @see ASSET_COMPARE_RESULT
 */
ASSET_COMPARE_RESULT CAssetInfo::Compare(const CAssetInfo & Other)
{
	//资源类型或者md5不同则是不同的资源
	if (Other.GetType() != GetType() || Other.GetMD5() != GetMD5()) return ASSET_DIFFERENCE;

	if (Other.GetName() == GetName())
	{
		return Other.GetPathID() == GetPathID() ? ASSET_THE_SAME : ASSET_EQUAL;
	}
	else
	{
		return ASSET_DIFFERENT_NAME;
	}
}

CAssetsFileInfo::CAssetsFileInfo()
{
}

CAssetsFileInfo::~CAssetsFileInfo()
{
	m_Assets.clear();
}
