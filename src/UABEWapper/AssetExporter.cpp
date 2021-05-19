#include "StdAfx.h"
#include "../svcdef.h"
#include "AssetExporter.h"


/**
 * ��ȡ��Դ����������Ϣ
 *
 * @param iExportError ����������
 */
const char * GetAssetExportErrorMessage(int iExportError)
{
	switch (iExportError)
	{
	case SUCCESSED: return "Success";
	case PICTURE_ASSET_READ_FAILED: return "Failed to read texture file";
	default: return lodepng_error_text(iExportError);

	}
}

/**
 * ���캯��
 *
 * @param pAssetBaseTypeValue ��Դ����
 */
CAssetExporter::CAssetExporter(AssetTypeValueField * pAssetBaseTypeValue)
{
	m_pAssetBaseTypeValue = pAssetBaseTypeValue;
}


CAssetExporter::~CAssetExporter()
{
	m_pAssetBaseTypeValue = NULL;
}

/**
 * ������Դ��������
 *
 * @param pAssetBaseTypeValue ��Դ����
 * @return ��Դ��ȡ��
 */
asset_exporter_ptr CAssetExtractorFactory::CreateExporter(AssetTypeValueField * pAssetBaseTypeValue)
{
	asset_exporter_ptr result;
	if(strcmp(pAssetBaseTypeValue->GetTemplateField()->type, "Texture2D") == 0)
	{
		result.reset(new CTexture2DExporter(pAssetBaseTypeValue));
	}
	else if (strcmp(pAssetBaseTypeValue->GetTemplateField()->type, "AudioClip") == 0)
	{

	}
	else if (strcmp(pAssetBaseTypeValue->GetTemplateField()->type, "MovieTexture") == 0)
	{

	}
	return result;
}

/**
 * ���캯��
 *
 * @param pAssetBaseTypeValue ��Դ����
 */
CTexture2DExporter::CTexture2DExporter(AssetTypeValueField * pAssetBaseTypeValue)
	: CAssetExporter(pAssetBaseTypeValue)
{
}

CTexture2DExporter::~CTexture2DExporter()
{
}

/**
 * ��ȡ��Դ
 *
 * @param FilePath ��ȡ��Դ֮�󱣴���ļ�·��
 * @return ������  0Ϊsuccess
 */
unsigned int CTexture2DExporter::ExtractAsset(const string & FilePath)
{
	TextureFile stTextureFile;
	memset(&stTextureFile, 0, sizeof(stTextureFile));
	if (!ReadTextureFile(&stTextureFile, m_pAssetBaseTypeValue))
	{
		return PICTURE_ASSET_READ_FAILED;
	}

	vector<BYTE> vBuffer(stTextureFile.m_Width * stTextureFile.m_Height * 4);
	if (!GetTextureData(&stTextureFile, &vBuffer[0])) return PICTURE_ASSET_READ_FAILED;

	return lodepng::encode(FilePath, vBuffer, stTextureFile.m_Width, stTextureFile.m_Height);
}

/**
 * ��ȡ��Դ�ļ���׺
 *
 * @return �ļ���׺
 */
const char * CTexture2DExporter::GetSuffix()
{
	return ".png";
}

/**
 * ���캯��
 *
 * @param pAssetBaseTypeValue ��Դ����
 */
CAudioClipExporter::CAudioClipExporter(AssetTypeValueField * pAssetBaseTypeValue)
	: CAssetExporter(pAssetBaseTypeValue)
{
}

CAudioClipExporter::~CAudioClipExporter()
{
}

/**
 * ��ȡ��Դ
 *
 * @param FilePath ��ȡ��Դ֮�󱣴���ļ�·��
 * @return ������  0Ϊsuccess
 */
unsigned int CAudioClipExporter::ExtractAsset(const string & FilePath)
{
	return 0;
}

/**
 * ��ȡ��Դ�ļ���׺
 *
 * @return �ļ���׺
 */
const char * CAudioClipExporter::GetSuffix()
{
	return ".wav";
}

/**
 * ���캯��
 *
 * @param pAssetBaseTypeValue ��Դ����
 */
CMovieTexture::CMovieTexture(AssetTypeValueField * pAssetBaseTypeValue)
	: CAssetExporter(pAssetBaseTypeValue)
{
}

CMovieTexture::~CMovieTexture()
{
}

/**
 * ��ȡ��Դ
 *
 * @param FilePath ��ȡ��Դ֮�󱣴���ļ�·��
 * @return ������  0Ϊsuccess
 */
unsigned int CMovieTexture::ExtractAsset(const string & FilePath)
{
	return 0;
}

/**
 * ��ȡ��Դ�ļ���׺
 *
 * @return �ļ���׺
 */
const char * CMovieTexture::GetSuffix()
{
	return nullptr;
}
