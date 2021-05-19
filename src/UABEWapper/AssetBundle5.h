#pragma once
/**
 * ����Դ��������ص�����
 *
 * @author ������
 */

/**
 * ��Դ������
 * ֧��Unity 5.3+
 */
class CAssetBundle5 :
	public CAssetBundle
{
public:
	/**
	 * ���캯��
	 *
	 * @param pClassDatabaseManager �����ݿ������
	 * @param pAssetsBundleFile ��Դ������
	 * @param ppFile ��Դ�������ַ
	 */
	CAssetBundle5(CClassDatabaseManager * pClassDatabaseManager, AssetsBundleFile * pAssetsBundleFile, FILE ** ppFile);
	~CAssetBundle5();

	/**
	 * ��ȡAssetsFile������
	 *
	 * @param nAssetsFileListIndex ��Դ�ļ����������
	 */
	DWORD GetAssetsFileCount(int nAssetsFileListIndex);

	/**
	 * ��ȡ��Դ�ļ���С
	 *
	 * @param pAssetsFileInfo ��Դ�ļ���Ϣ
	 */
	QWORD GetAssetsFileSize(void * pAssetsFileInfo);

	/**
	 * ��ȡ��Դ�ļ���ƫ����
	 *
	 * @param pAssetsFileInfo ��Դ�ļ���Ϣ
	 */
	QWORD GetAssetsFileOffset(void * pAssetsFileInfo);

	/**
	 * ��ȡ��Դ�ļ�������
	 *
	 * @param pAssetsFileInfo ��Դ�ļ���Ϣ
	 */
	const char * GetAssetsFileName(void * pAssetsFileInfo);

	/**
	 * ��ȡ��Դ�ļ�������ָ����������Դ�ļ���Ϣ
	 *
	 * @param pAssetsFileList ��Դ�ļ�����
	 * @param index ��Դ�ļ�����
	 */
	void * GetAssetsFileInfo(void * pAssetsFileList, int index);

	/**
	 * �ж��Ƿ�Ϊ��Դ�ļ�
	 *
	 * @param pAssetsFileInfo ��Դ�ļ���Ϣ
	 */
	bool IsAssetsFile(void * pAssetsFileInfo);

	/**
	 * ��ȡָ����������Դ�ļ��б�
	 *
	 * @param index ��Դ�ļ��б�����
	 */
	void * GetAssetsList(int index);

	/**
	 * �жϵ�ǰAssetsBundle�Ƿ�ѹ����
	 *
	 * @return ѹ��������true,����false
	 */
	bool IsCompressed();

	/**
	 * ����AssetsFileReader
	 *
	 * @param pAssetsFileInfo AssetsFile�ļ���Ϣ
	 */
	AssetsFileReader MakeAssetsFileReader(void * pAssetsFileInfo);
	
	/**
	 * ��ȡ����AssetsFile
	 *
	 * @param pAssetsFileList AssetsFile�ļ���Ϣ�б�
	 * @param pDirectoryPath ��ȡ����Դ�ļ�����ŵ�Ŀ¼
	 * @param pAssetFileSuffix ��Դ�ļ���׺,Ĭ��Ϊ".assets"
	 * @return ��ȡ�������ļ�������
	 */
	set<string> ExtractAssetsFiles(void * pAssetsFileList, const char * pDirectoryPath, const char * pAssetFileSuffix = ".assets");
};

