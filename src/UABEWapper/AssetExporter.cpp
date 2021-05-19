#include "StdAfx.h"
#include "../svcdef.h"
#include "AssetExporter.h"


/**
 * 获取资源导出错误信息
 *
 * @param iExportError 导出错误码
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
 * 构造函数
 *
 * @param pAssetBaseTypeValue 资源数据
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
 * 创建资源导出对象
 *
 * @param pAssetBaseTypeValue 资源数据
 * @return 资源提取器
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
 * 构造函数
 *
 * @param pAssetBaseTypeValue 资源数据
 */
CTexture2DExporter::CTexture2DExporter(AssetTypeValueField * pAssetBaseTypeValue)
	: CAssetExporter(pAssetBaseTypeValue)
{
}

CTexture2DExporter::~CTexture2DExporter()
{
}

/**
 * 提取资源
 *
 * @param FilePath 提取资源之后保存的文件路径
 * @return 错误码  0为success
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
 * 获取资源文件后缀
 *
 * @return 文件后缀
 */
const char * CTexture2DExporter::GetSuffix()
{
	return ".png";
}

/**
 * 构造函数
 *
 * @param pAssetBaseTypeValue 资源数据
 */
CAudioClipExporter::CAudioClipExporter(AssetTypeValueField * pAssetBaseTypeValue)
	: CAssetExporter(pAssetBaseTypeValue)
{
}

CAudioClipExporter::~CAudioClipExporter()
{
}

/**
 * 提取资源
 *
 * @param FilePath 提取资源之后保存的文件路径
 * @return 错误码  0为success
 */
unsigned int CAudioClipExporter::ExtractAsset(const string & FilePath)
{
	return 0;
}

/**
 * 获取资源文件后缀
 *
 * @return 文件后缀
 */
const char * CAudioClipExporter::GetSuffix()
{
	return ".wav";
}

/**
 * 构造函数
 *
 * @param pAssetBaseTypeValue 资源数据
 */
CMovieTexture::CMovieTexture(AssetTypeValueField * pAssetBaseTypeValue)
	: CAssetExporter(pAssetBaseTypeValue)
{
}

CMovieTexture::~CMovieTexture()
{
}

/**
 * 提取资源
 *
 * @param FilePath 提取资源之后保存的文件路径
 * @return 错误码  0为success
 */
unsigned int CMovieTexture::ExtractAsset(const string & FilePath)
{
	return 0;
}

/**
 * 获取资源文件后缀
 *
 * @return 文件后缀
 */
const char * CMovieTexture::GetSuffix()
{
	return nullptr;
}
