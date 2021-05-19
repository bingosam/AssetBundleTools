#pragma once


/**
 * ��ȡ��Դ����������Ϣ
 *
 * @param iExportError ����������
 */
const char * GetAssetExportErrorMessage(int iExportError);

/**
 * ��Դ��ȡ��
 */
class CAssetExporter
{
public:
	/**
	 * ���캯��
	 *
	 * @param pAssetBaseTypeValue ��Դ����
	 */
	CAssetExporter(AssetTypeValueField * pAssetBaseTypeValue);
	~CAssetExporter();

	/**
	 * ��ȡ������
	 */
	unsigned int GetError() { return m_uError; }

	/**
	 * ��ȡ������Ϣ
	 */
	string GetErrorMessage() { return m_ErrorMessage; }

	/**
	 * ��ȡ��Դ
	 *
	 * @param[out] ��Դ�ļ�·��������ȡʧ��  ��Ϊ""
	 * @param OutputDir ���Ŀ¼
	 * @return ������  0Ϊsuccess
	 */
	virtual unsigned int ExtractAsset(const string & FilePath) = 0;

	/**
	 * ��ȡ��Դ�ļ���׺
	 *
	 * @return �ļ���׺
	 */
	virtual const char * GetSuffix() = 0;

protected:
	/**
	 * ������
	 */
	unsigned int m_uError;

	/**
	 * ������Ϣ
	 */
	string m_ErrorMessage;

	/**
	 * ��Դ����
	 */
	AssetTypeValueField * m_pAssetBaseTypeValue;
};
typedef shared_ptr<CAssetExporter> asset_exporter_ptr;

class CAssetExtractorFactory
{
public:
	/**
	 * ������Դ��������
	 *
	 * @param pAssetBaseTypeValue ��Դ����
	 * @return ��Դ��ȡ��
	 */
	static asset_exporter_ptr CreateExporter(AssetTypeValueField * pAssetBaseTypeValue);
};

class CTexture2DExporter : public CAssetExporter
{
public:
	/**
	 * ���캯��
	 *
	 * @param pAssetBaseTypeValue ��Դ����
	 */
	CTexture2DExporter(AssetTypeValueField * pAssetBaseTypeValue);
	~CTexture2DExporter();

	/**
	 * ��ȡ��Դ
	 *
	 * @param FilePath ��ȡ��Դ֮�󱣴���ļ�·��
	 * @return ������  0Ϊsuccess
	 */
	unsigned int ExtractAsset(const string & FilePath);

	/**
	 * ��ȡ��Դ�ļ���׺
	 *
	 * @return �ļ���׺
	 */
	const char * GetSuffix();
};

class CAudioClipExporter : public CAssetExporter
{
public:
	/**
	 * ���캯��
	 *
	 * @param pAssetBaseTypeValue ��Դ����
	 */
	CAudioClipExporter(AssetTypeValueField * pAssetBaseTypeValue);
	~CAudioClipExporter();

	/**
	 * ��ȡ��Դ
	 *
	 * @param FilePath ��ȡ��Դ֮�󱣴���ļ�·��
	 * @return ������  0Ϊsuccess
	 */
	unsigned int ExtractAsset(const string & FilePath);
	
	/**
	 * ��ȡ��Դ�ļ���׺
	 *
	 * @return �ļ���׺
	 */
	const char * GetSuffix();
};

class CMovieTexture : public CAssetExporter
{
public:
	/**
	 * ���캯��
	 *
	 * @param pAssetBaseTypeValue ��Դ����
	 */
	CMovieTexture(AssetTypeValueField * pAssetBaseTypeValue);
	~CMovieTexture();

	/**
	 * ��ȡ��Դ
	 *
	 * @param FilePath ��ȡ��Դ֮�󱣴���ļ�·��
	 * @return ������  0Ϊsuccess
	 */
	unsigned int ExtractAsset(const string & FilePath);
	
	/**
	 * ��ȡ��Դ�ļ���׺
	 *
	 * @return �ļ���׺
	 */
	const char * GetSuffix();
};
