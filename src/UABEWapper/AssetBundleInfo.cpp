#include "StdAfx.h"
#include "uabedef.h"
#include  "AssetBundleInfo.h"

/**
 * ��ȡ�ȽϽ�����ַ�����Ϣ
 *
 * @return result �ȽϽ��
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
* ��ȡ�ȽϽ��������
*
* return �ȽϽ����������Ϣ
*/
const char * GetAssetCompareResultDesc(ASSET_COMPARE_RESULT result)
{
	switch (result)
	{
	case ASSET_DIFFERENCE: return "��ͬ��Դ";
	case ASSET_THE_SAME: return "ͬһ��Դ�ļ�";
	case ASSET_EQUAL: return "��Դ������ͬ,�ļ���ͬ";
	case ASSET_DIFFERENT_NAME: return "��Դ������ͬ,��Դ����ͬ";
	default: return "UNKNOW_COMPARE_RESULT";
	}
}

/**
 * ��ȡ�ȽϽ��������
 *
 * return �ȽϽ����������Ϣ
 */
const wchar_t * GetAssetCompareResultDescW(ASSET_COMPARE_RESULT result)
{
	switch (result)
	{
	case ASSET_DIFFERENCE: return L"��ͬ��Դ";
	case ASSET_THE_SAME: return L"ͬһ��Դ�ļ�";
	case ASSET_EQUAL: return L"��Դ������ͬ,�ļ���ͬ";
	case ASSET_DIFFERENT_NAME: return L"��Դ������ͬ,��Դ����ͬ";
	default: return L"UNKNOW_COMPARE_RESULT";
	}
}


/**
 * ���캯��
 *
 * @param pName ��Դ����
 * @param uPathID ��Դ·��ID
 * @param nType ��Դ����
 * @param type ��Դ�����ַ���
 * @param nSize ��Դ��С
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
 * �ж���Դ�Ƿ����
 *
 * @param Other ������Դ
 */
bool CAssetInfo::EqualTo(const CAssetInfo &Other)
{
	if (Other.GetType() != GetType()) return false;
	return strcmp(Other.GetMD5().c_str(), GetMD5().c_str()) == 0;
}

/**
 * �ж��Ƿ�ΪAssetBundle��Դ
 */
bool CAssetInfo::IsAssetBundleAsset()
{
	return strcmp(GetTypeString().c_str(), "AssetBundle") == 0;
}

/**
 * ����С�ں�
 *
 * @param Other ��һ������
 * @return �Ƿ�С��Other
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
 * ����һ����Դ���бȽ�
 *
 * @param Other ������Դ
 * @return �ȽϽ�� @see ASSET_COMPARE_RESULT
 */
ASSET_COMPARE_RESULT CAssetInfo::Compare(const CAssetInfo & Other)
{
	//��Դ���ͻ���md5��ͬ���ǲ�ͬ����Դ
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
