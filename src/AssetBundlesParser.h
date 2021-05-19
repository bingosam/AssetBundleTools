#pragma once

#include <fstream>

/**
 * ��Դ�ļ���Ϣ����
 *
 * @key string ��ȡ��AssetsBundleDirectoryInfo06->name "CAB-XXXX"
 * @value assets_file_info_ptr ��Դ�ļ���Ϣ
 */
typedef map<string, assets_file_info_ptr, str_less_incase> assets_file_info_map;

/**
 * ��Դ�ļ�����
 *
 * @key ��Դ������ CAB-XXX
 * @value ��Դ�������� CAB-XXX
 */
typedef map<string, set<string>, str_less_incase> dependencies_map;

/**
 * ��Դ������
 *
 * @key ��Դ���ļ���
 * @value ��Դ�ļ������� CAB-XXX
 */
typedef map<string, set<string>> asset_bundle_map;

/**
 * �ظ�����Դ������
 *
 * @key ��Դ������ CAB-XXX
 * @value ��Դ���ļ�������
 */
typedef map<string, set<string>, str_less_incase> duplicate_asset_bundle_map;

struct ASSET_DETAIL_INFO {
	asset_info_ptr m_AssetInfo;
	string m_OwnerName;
	string m_OwnerFileName;
	string m_OwnerAssetBundleName;

	bool operator < (const ASSET_DETAIL_INFO &Other) const
	{
		return (*m_AssetInfo) < (*Other.m_AssetInfo);
	}
};
typedef shared_ptr<ASSET_DETAIL_INFO> asset_detail_info_ptr;

bool inline asset_detail_info_compare(asset_detail_info_ptr &Left, asset_detail_info_ptr &Right)
{
	return (*Left) < (*Right);
}

class CAssetBundlesParser
{
public:
	CAssetBundlesParser(const char * pOutputDirPath, bool bExportAssetFile = false);
	~CAssetBundlesParser();

	const TCHAR *GetName() const { return _T("Asset Bundle Parser"); }

	/**
	 * ��ѹָ���ļ����µ�������Դ���ļ�
	 *
	 * @param pDirectoryPath �����Դ�����ļ���·��
	 */
	void UnpackAssetsBundleFiles(const char *pDirectoryPath);

	/**
	 * ��ѹAssetBundle�ļ�
	 *
	 * @param AssetBundle AssetBundle������
	 * @param AssetBundleFile AssetBundle�ļ�·��
	 * @return �ɹ����ؽ�ѹ����ļ�·��, ���򷵻ؿ�
	 */
	string UnpackAssetBundleFile(const ab_file_ptr & AssetBundle, const string & AssetBundleFile);

	/**
	 * ����������Դ����Ϣ
	 */
	void LoadAssetBundles();

	/**
	 * ����AssetBundle
	 *
	 * @param AssetBundle AssetBundle������
	 * @param AssetBundleFile AssetBundle�ļ�·��
	 * @param DecompressedFile AssetBundle��ѹ����ļ�·��
	 */
	void LoadAssetBundle(const ab_file_ptr & AssetBundle, const string & AssetBundleFile, const string & DecompressedFile);

	/**
	 * ��ȡָ��AssetBundle��ȡ����Դ�Ĵ��·��
	 */
	string GetAssetsDirPath(const string & AssetBundleFile);

	/**
	 * ����Դ�ļ�����·��ת�������·��
	 *
	 * @param AbsAssetPath ��Դ����·��
	 * @return ���·��
	 */
	string ConvertAssetPathToRelativePath(const string & AbsAssetPath);

	/**
	 * �ͷ���Դ
	 */
	void Release();

	/**
	 * ������
	 */
	void Clear();

	/**
	 * ��Դ����
	 *
	 * @return ��������ļ�·��
	 */
	string Analyze();

	/**
	 * ���ɱ���ҳ��
	 *
	 * @return ��������ļ�·��
	 */
	string GenerateReport();

	/**
	 * ����Ŀ���ļ����µ�AssetBundle
	 *
	 * @param pDirectoryPath AssetBundle����Ŀ¼
	 * @return ���ؽ�����Ľ���ļ�
	 */
	string Parse(const char *pDirectoryPath);

	/**
	 * ������Դ�ļ���Ϣ
	 *
	 * @param AssetBundleName ��Դ������
	 */
	assets_file_info_ptr LookupAssetsFileInfo(const string &AssetBundleName);

	/**
	 * ����������Ϣ
	 *
	 * @param AssetBundleName ��Դ������
	 */
	set<string> LookupDependencies(const string &AssetBundleName);
	
	/**
	 * ���ұ�������Ϣ
	 *
	 * @param AssetBundleName ��Դ������
	 */
	set<string> LookupBeDepended(const string &AssetBundleName);

	/**
	 * �����ظ�����Դ���ļ�
	 *
	 * @param AssetBundleName ��Դ������
	 */
	set<string> LookupDuplicateAssetBundle(const string &AssetBundleName);

	/**
	 * ����ָ��AssetBundle�е�AssetsFile
	 *
	 * @param AssetBundleFile AssetBundle�ļ�·��
	 */
	set<string> LookupAssetsFile(const string & AssetBundleFile);

	/**
	 * ���AssetsFile
	 *
	 * @param AssetBundleFile AssetBundle�ļ�·��
	 * @param AssetsFiles AssesFile����
	 */
	void PutAssetsFile(const string & AssetBundleFile, const set<string> & AssetsFiles);

	/**
	 * ���ҽ�ѹ���AssetBundle�ļ�·��
	 */
	string LookupDecompressedAssetBundleFile(const string & AssetBundleFile);

	/**
	 * ��ӽ�ѹ���AssetBundle�ļ�·��
	 */
	void PutDecompressedAssetBundleFile(const string & AssetBundleFile, const string & DecompressedAssetBundle);

	/**
	 * ��Ӵ�С
	 *
	 * @param nDecompressedSize ��ѹ���С
	 * @param nCompressedSize ѹ�����С                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          
	 */
	void AddSize(size_t nDecompressedSize, size_t nCompressedSize);

private:
	class CWorkThread : public CThread
	{
	public:
		CWorkThread(CAssetBundlesParser * pOwner);
		virtual ~CWorkThread();
		
		/**
		 * ���AssetBundle�ļ�·��
		 *
		 * @param AssetBundleFile AssetBundle�ļ�·��
		 */
		void AddAssetBundleFile(const string & AssetBundleFile);

		/**
		 * @copydoc CThread::Run
		 */
		virtual void Run();
	private:
		ab_file_ptr m_AssetsBundle;

		CAssetBundlesParser * m_pOwner;

		vector<string> m_AssetBundleFiles;
	};
	typedef shared_ptr<CWorkThread> work_thread_ptr;

	/**
	 * html�ļ�������
	 */
	class CHtmlGenerator{
	public:
		/**
		 * ���캯��
		 *
		 * @param pOwner �����Ľ�����
		 */
		CHtmlGenerator(CAssetBundlesParser * pOwner);

		/**
		 * ��������
		 */
		~CHtmlGenerator();
		
		/**
		 * ���������Դ
		 *
		 * @param AssetType ��Դ����
		 * @param ReduAssets ������Դ����
		 */
		void PutReduAssets(string AssetType, const list<asset_detail_info_ptr> & ReduAssets);

		/**
		 * �������
		 *
		 * @param Name ��Դ������������ABName��FileName��Name��
		 * @param AssetsFileInfo AssetsFile��Ϣ
		 */
		void PutDependency(string Name, const assets_file_info_ptr & AssetsFileInfo);

		/**
		 * �����Դ
		 *
		 * @param AssetBundleName  ��Դ������AssetBundle��
		 * @param AssetInfo ��Դ��Ϣ
		 */
		void PutAsset(string AssetBundleName, const asset_info_ptr & AssetInfo);

		/**
		 * ����html�ļ�
		 *
		 * @param FilePath �ļ�·��
		 */
		void Generate(const string & FilePath);

	private:
		typedef multimap<wstring, list<asset_detail_info_ptr>> redu_assets_map;
		typedef map<string, list<assets_file_info_ptr>> deps_map;
		typedef map<string, multimap<string, asset_info_ptr>> assets_map;

		CAssetBundlesParser * m_pOwner;

		wofstream m_Stream;

		/**
		 * ���������Դ�ļ���
		 *
		 * @key ��Դ����
		 * @value ������Դ����
		 */
		redu_assets_map m_ReduAssets;

		/**
		 * Ĭ��ѡ���������Դ
		 */
		wstring m_DefaultSelected;

		set<wstring> m_ReduAssetTypes;

		/**
		 * ȱ�ٵ�������
		 */
		set<string> m_MissingDependencies;

		/**
		 * ��������
		 */
		deps_map m_Dependencies;

		/**
		 * ������Դ��С
		 */
		UINT64 m_uiReduAssetsSize;

		/**
		 * ��Դ����
		 */
		assets_map m_Assets;

		/**
		 * ��ʼ���ļ�ͷ
		 */
		void InitHead();

		/**
		 * ���html body
		 */
		void AppendBody();

		/**
		 * ���ժҪ
		 */
		void AppendDigest();

		/**
		 * ���������Ϣ
		 */
		void AppendRedus();

		/**
		 * ���������Ϣ
		 */
		void AppendDependencies();

		/**
		 * �����Դ��Ϣ
		 */
		void AppendAssetsInfo();

		/**
		 * ��ӽű�
		 */
		void AppendScript();

		/**
		 * ���������Դѡ��ű�
		 */
		void AppendReduAssetsSelectScript();

		/**
		 * ���������Դ���е��ļ���
		 *
		 * @param Assets ��Դ����
		 */
		void AppendReduAssets(list<asset_detail_info_ptr> Asset);

		/**
		 * ���������Դ���ļ���
		 *
		 * @param Asset ��Դ��Ϣ
		 */
		void AppendReduAsset(const asset_detail_info_ptr & Asset, int nRowSpan = 0);

		/**
		 * ��λ����
		 *
		 * @param uiByteSize �ֽ���
		 * @return ����������
		 */
		string ConvertUnitA(UINT64 uiByteSize);

		/**
		 * ��λ����
		 *
		 * @param uiByteSize �ֽ���
		 * @return ����������
		 */
		wstring ConvertUnitW(UINT64 uiByteSize);
	};
	typedef shared_ptr<CHtmlGenerator> html_generator_ptr;

	/**
	 * ��Դ��
	 */
	ab_file_ptr m_AssetsBundle;

	/**
	 * �����Դ�����ļ���·��
	 */
	string m_DirPath;

	/**
	 * ��Դ���ļ�·������
	 *
	 * @key ��ѹǰ��AssetBundle�ļ�·��
	 * @param ��ѹ���AssetBundle�ļ�·��
	 */
	map<string, string> m_AssetsBundleFilePathes;

	/**
	 * ������Դ���ļ�·�����ϵĻ������
	 */
	CMutex m_ABPathesMutex;

	/**
	 * ��Դ������
	 *
	 * @key AssetBundle�ļ���
	 * @value AssetBundle�н�����������Դ������
	 */
	asset_bundle_map m_AssetBundles;

	/**
	 * �ظ�����Դ������
	 *
	 * @key ��Դ����
	 * @value �����и���Դ������AssetBundle�ļ�������
	 */
	duplicate_asset_bundle_map m_DuplicateAssetbundles;

	assets_file_info_map m_AssetsFileInfos;

	/**
	 * ��������
	 *
	 * @key ��Դ����
	 * @value ��������Դ������
	 */
	dependencies_map m_Dependencies;

	/**
	 * ���Ŀ¼
	 */
	string m_OutputDirPath;

	/**
	 * ��ѹ��Ĵ�С
	 */
	size_t m_nDecompressedSize;

	/**
	 * ������Դ��С
	 */
	size_t m_nReduAssetsSize;

	/**
	 * ��ѹǰ�Ĵ�С
	 */
	size_t m_nCompressedSize;

	/**
	 * ��Դ����Ŀ¼
	 */
	string m_AssetExportDir;
	
	/**
	 * ������Դ�ļ���Ϣ�Ļ������
	 */
	CMutex m_AssetsFileInfosMutex;

	/**
	 * ������Դ�����ϵĻ������
	 */
	CMutex m_AssetBundlesMutex;

	/**
	 * �����������ϵĻ������
	 */
	CMutex m_DepsMutex;

	/**
	 * AssetBundle��С�Ļ�����
	 */
	CMutex m_AssetBundleSizeMutex;

	vector<work_thread_ptr> m_WorkThreads;

	CMutex m_DirMutex;

	/**
	 * ����������
	 *
	 * @key ��Դ����
	 * @value ����������Դ������
	 */
	dependencies_map m_BeDepended;

	html_generator_ptr m_HtmlGenerator;
	
	/**
	 * ���AssetsFile��Ϣ
	 *
	 * @param AssetBundleName AssetBundle����
	 * @param AssetsFileInfo AssetsFile��Ϣ
	 */
	void PutAssetsFileInfo(const string &AssetBundleName, const assets_file_info_ptr &AssetsFileInfo);

	/**
	 * ��������
	 *
	 * @param AssetBundleName ��Դ����
	 * @param Dependencies ��������
	 */
	void PutDependencies(const string &AssetBundleName, const set<string> &Dependencies);

	/**
	 * ����ظ�����Դ���ļ�
	 *
	 * @param AssetBundleName ��Դ����
	 * @param FileName �ļ���
	 */
	void PutDuplicateAssetBundle(const string &AssetBundleName, const string &FileName);

	/**
	 * ���ļ�����׷����Դ�ļ���Ϣ
	 *
	 * @param Stream �ļ���
	 * @param Name AssetBundle��
	 * @param AssetsFileInfo ��Դ�ļ���Ϣ
	 * @param Prefix ÿһ�е�ǰ׺
	 */
	void AppendAssetBundle(ofstream &Stream, const string &Name, const assets_file_info_ptr &AssetsFileInfo, const string &Prefix);

	/**
	 * ���ļ�����׷����Դ��Ϣ
	 *
	 * @param Stream �ļ���
	 * @param AssetsDetail ��Դ��Ϣ�б�
	 */
	void AppendAssets(ofstream &Stream, list<asset_detail_info_ptr> &AssetsDetail);

	/**
	 * ���ļ�����׷����Դ��Ϣ
	 * @param Stream �ļ���
	 * @param AssetDetailInfo ��Դ��Ϣ
	 * @param Prefix ÿһ�е�ǰ׺
	 */
	void AppendAsset(ofstream &Stream, const asset_detail_info_ptr &AssetDetailInfo, const string &Prefix);

	/**
	 * ������Դ�ļ�
	 *
	 * @param AssetBundleFile ��ѹǰ��AssetBundle�ļ�·��
	 * @param AssetsFileName Ҫ��������Դ������AssetsFile����
	 * @param uiAssetID ��ԴID
	 * @return ������Դ�ļ�·��
	 */
	string ExportAssetFile(const string & AssetBundleFile, const string & AssetsFileName, UINT64 uiAssetID);

	/**
	 * ���������е�������Դ�ļ�
	 *
	 * @param AssetsDetail ��Դ�������
	 */
	void ExportAssetFiles(const list<asset_detail_info_ptr> &AssetsDetail);
};

typedef shared_ptr<CAssetBundlesParser> ab_parser_ptr;

