#include "stdafx.h"

#include <Dbghelp.h>

#include "svcdef.h"
#include "AssetBundlesParser.h"

#include <codecvt>

/**
 * 
 */
#define IS_WEB_VERSION	1

static CClassDatabaseManager *g_pClassDatabaseManager = NULL;

static bool g_bIsBigEndian;

static string g_sWorkPath;

/**
 * ��־�����ʵ����
 */
static CLogService *g_pLogService = NULL;

/**
 * ��ȡ�����ݿ������
 */
CClassDatabaseManager * GetClassDatabaseManager() { return g_pClassDatabaseManager; }
 
/**
 * �Ƿ���
 */
bool IsBigEndian() { return g_bIsBigEndian; }

/**
 * ��ȡ����·��
 */
string GetWorkPath() { return g_sWorkPath; }
 
/**
 * �����־�����ʵ����
 */
CLogService * GetLogService() { return g_pLogService; }

/**
* ��¼��־��Ϣ��
*/
void LogInfo(const TCHAR *pFormat, ...)
{
	va_list pArgs;
	va_start(pArgs, pFormat);
#ifdef UNICODE
		vwprintf(pFormat, pArgs);
#else
		vprintf(pFormat, pArgs);
#endif

	va_end(pArgs);
}

/**
* �ṹ���쳣�Ĵ�������
*/
static LONG WINAPI MyExceptionFilter(struct _EXCEPTION_POINTERS *pExceptionInfo)
{
	WCHAR szDumpPath[64];
	wsprintf(szDumpPath, L"abt_%u.dmp", GetCurrentProcessId());

	HANDLE hFile = CreateFileW(szDumpPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		LogInfo(L"Failed to create dump file: %s\n", GetOsErrorMessage().c_str());
	}
	else
	{
		MINIDUMP_TYPE DumpType = MiniDumpNormal;

		MINIDUMP_EXCEPTION_INFORMATION Info;
		Info.ThreadId = GetCurrentThreadId();
		Info.ExceptionPointers = pExceptionInfo;
		Info.ClientPointers = TRUE;

		if (!MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, DumpType, &Info, NULL, NULL))
		{
			LogInfo(L"Failed to dump: %s\n", GetOsErrorMessage().c_str());
		}

		CloseHandle(hFile);
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

 
/**
 * ����Ӧ�ó����ʼ��������
 */
static void AppInitialize()
{
	//��ʼ��������������װ�����жϳ����Ƿ�����ִ��
	if (CreateMutex(NULL, FALSE, _T("nd_abt_pc_mutex")) == NULL)
		throw runtime_error("Unabled to create mutex");

	//������Ϣ
	SetUnhandledExceptionFilter(MyExceptionFilter);

	int i = 1;
	g_bIsBigEndian = !(*(char *)&i);

	char szPath[MAX_PATH] = { 0 };
	if (GetModuleFileNameA(NULL, szPath, MAX_PATH) == 0) throw COsError("Failed to GetModuleFileName: ");
	char *pLastSep = strrchr(szPath, '\\');
	if (NULL != pLastSep)
	{
		pLastSep[0] = 0;
	}
	g_sWorkPath.assign(szPath);
	g_pClassDatabaseManager = new CClassDatabaseManager();
	g_pClassDatabaseManager->LoadDatabaseFiles(FormatStr("%s\\ClassData", szPath).c_str());

	std::list< shared_ptr<CLogger> > Loggers;
	shared_ptr<CLogger> Logger(new CStdoutLogger("ERROR"));
	Loggers.push_back(Logger);

	shared_ptr<CLogger> FileLogger(
		new CFileLogger("DEBUG", FormatStr(_T("%hs%clogs%clog.log"), szPath, SEPARATOR_CHAR, SEPARATOR_CHAR).c_str(), 5 * 1024 * 1024, 10)
	);
	Loggers.push_back(FileLogger);

	g_pLogService = new CLogService(Loggers);

#ifdef _WIN32
	//windows ����ҳ936,֧����ʾ���ַ�����
	setlocale(LC_CTYPE, ".936");
#endif
}

/**
 * ������Դ���ļ���
 *
 * @param pAbDirectory ��Դ���ļ���·��
 * @param pOutputDir ���Ŀ¼
 */
static void ParseAssetBundleDirectory(const char * pAbDirectory, const char * pOutputDir)
{
	ab_parser_ptr Parser(new CAssetBundlesParser(pOutputDir, true));
	Parser->Parse(pAbDirectory);
	Parser.reset();
}

/**
 * ���ɱ�������
 *
 * @param pOutputDir ���Ŀ¼
 */
static void GenerateReport(const char * pOutputDir)
{
	string OutputDir(pOutputDir);
	string SrcDir(g_sWorkPath);
	SrcDir.append(SEPARATOR).append("Report");
	CopyDirectory(SrcDir.c_str(), OutputDir.c_str(), TRUE);
#if !IS_WEB_VERSION
	ShellExecuteA(0, "open", OutputDir.append(SEPARATOR).append("report.html").c_str(), NULL, NULL, SW_SHOWNORMAL);
#endif
}

/**
 *
 */
void DoTest()
{
	ab_file_ptr AssetsBundle(new CAssetsBundleTools(GetClassDatabaseManager()));
	AssetsBundle->ExportAssetFileTo("dlgattributeequip.ab.ndunity3d", "CAB-01f8415890f2d178951fa4758de7de51", 8738200644336028248, "ABxx");
	//FILE * m_ABFile = fopen("D:\\Users\\Public\\Documents\\im\\120905@nd\\RecvFile\\֣��_630988\\assetbundle��Ϸ\\bnpc_ysn_wenshenzhuang_head01.skin.mat.assetbundle", "rb");
	FILE * m_ABFile = fopen("dlgattributeequip.ab.ndunity3d", "rb");
	if (NULL == m_ABFile)
	{
		return;
	}

	AssetsBundleFile * pAssetsBundleFile = new AssetsBundleFile();

	if (!pAssetsBundleFile->Read(AssetsReaderFromFile, (LPARAM)m_ABFile, NULL, false))
	{
		fclose(m_ABFile);
		m_ABFile = NULL;
		return;
	}

	asset_bundle_ptr AssetBundle = CAssetBundleFactory::CreateAssetBundle(GetClassDatabaseManager(), pAssetsBundleFile, &m_ABFile);

	assets_file_ptr AssetFile = AssetBundle->LoadFirstAssetsFile();
	while (AssetFile.get())
	{
		//AssetFile->LoadAssetInfos("ab");
		printf("%s    %s \n", AssetFile->GetFileName().c_str(), AssetFile->GetName().c_str());
		AssetFile = AssetBundle->LoadNextAssetsFile();
	}
	AssetBundle.reset();
	//set<string> Files = AssetBundle->ExtractAssetsFiles(&m_ABFile, "5.2");
	pAssetsBundleFile->Close();
	delete pAssetsBundleFile;
	fclose(m_ABFile);
	system("pause");
}

#define TEST_LOCAL

int main(int argc, char *argv[])
{
	AppInitialize();
#ifdef NDEBUG
	if (argc != 3)
	{
		GetLogService()->Error(_T("main"), _T("��ָ��2������: [Asset Bundle Directory] [Output Directory]"));
		return -1;
	}

	const char *pAbDirectory = argv[1];
	const char *pOutputDirectory = argv[2];
#else
	const char *pAbDirectory = "F:\\dialog";
	const char *pOutputDirectory = "F:\\sjmy2";
	ClearDirectory(pOutputDirectory, true);
#endif
	GetLogService()->Debug(_T("main"), _T("The directory path of asset bundle: \"%hs\""), pAbDirectory);
	GetLogService()->Debug(_T("main"), _T("Output directory path: \"%hs\""), pOutputDirectory);

#if (defined TEST_LOCAL) || (defined NDEBUG)
	ParseAssetBundleDirectory(pAbDirectory, pOutputDirectory);
	GenerateReport(pOutputDirectory);
#else
	DoTest();
#endif
}