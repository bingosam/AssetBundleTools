#pragma once


/**
 * 获取资源导出错误信息
 *
 * @param iExportError 导出错误码
 */
const char * GetAssetExportErrorMessage(int iExportError);

/**
 * 资源提取器
 */
class CAssetExporter
{
public:
	/**
	 * 构造函数
	 *
	 * @param pAssetBaseTypeValue 资源数据
	 */
	CAssetExporter(AssetTypeValueField * pAssetBaseTypeValue);
	~CAssetExporter();

	/**
	 * 获取错误码
	 */
	unsigned int GetError() { return m_uError; }

	/**
	 * 获取错误信息
	 */
	string GetErrorMessage() { return m_ErrorMessage; }

	/**
	 * 提取资源
	 *
	 * @param[out] 资源文件路径，若提取失败  则为""
	 * @param OutputDir 输出目录
	 * @return 错误码  0为success
	 */
	virtual unsigned int ExtractAsset(const string & FilePath) = 0;

	/**
	 * 获取资源文件后缀
	 *
	 * @return 文件后缀
	 */
	virtual const char * GetSuffix() = 0;

protected:
	/**
	 * 错误码
	 */
	unsigned int m_uError;

	/**
	 * 错误信息
	 */
	string m_ErrorMessage;

	/**
	 * 资源数据
	 */
	AssetTypeValueField * m_pAssetBaseTypeValue;
};
typedef shared_ptr<CAssetExporter> asset_exporter_ptr;

class CAssetExtractorFactory
{
public:
	/**
	 * 创建资源导出对象
	 *
	 * @param pAssetBaseTypeValue 资源数据
	 * @return 资源提取器
	 */
	static asset_exporter_ptr CreateExporter(AssetTypeValueField * pAssetBaseTypeValue);
};

class CTexture2DExporter : public CAssetExporter
{
public:
	/**
	 * 构造函数
	 *
	 * @param pAssetBaseTypeValue 资源数据
	 */
	CTexture2DExporter(AssetTypeValueField * pAssetBaseTypeValue);
	~CTexture2DExporter();

	/**
	 * 提取资源
	 *
	 * @param FilePath 提取资源之后保存的文件路径
	 * @return 错误码  0为success
	 */
	unsigned int ExtractAsset(const string & FilePath);

	/**
	 * 获取资源文件后缀
	 *
	 * @return 文件后缀
	 */
	const char * GetSuffix();
};

class CAudioClipExporter : public CAssetExporter
{
public:
	/**
	 * 构造函数
	 *
	 * @param pAssetBaseTypeValue 资源数据
	 */
	CAudioClipExporter(AssetTypeValueField * pAssetBaseTypeValue);
	~CAudioClipExporter();

	/**
	 * 提取资源
	 *
	 * @param FilePath 提取资源之后保存的文件路径
	 * @return 错误码  0为success
	 */
	unsigned int ExtractAsset(const string & FilePath);
	
	/**
	 * 获取资源文件后缀
	 *
	 * @return 文件后缀
	 */
	const char * GetSuffix();
};

class CMovieTexture : public CAssetExporter
{
public:
	/**
	 * 构造函数
	 *
	 * @param pAssetBaseTypeValue 资源数据
	 */
	CMovieTexture(AssetTypeValueField * pAssetBaseTypeValue);
	~CMovieTexture();

	/**
	 * 提取资源
	 *
	 * @param FilePath 提取资源之后保存的文件路径
	 * @return 错误码  0为success
	 */
	unsigned int ExtractAsset(const string & FilePath);
	
	/**
	 * 获取资源文件后缀
	 *
	 * @return 文件后缀
	 */
	const char * GetSuffix();
};
