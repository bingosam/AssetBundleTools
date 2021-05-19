#include "StdAfx.h"
#include "../svcdef.h"

static const char *g_pDefaultDatabaseDirPath = "ClassData";


CClassDatabaseManager::CClassDatabaseManager()
{
}


CClassDatabaseManager::~CClassDatabaseManager()
{
	m_ClassDatabases.clear();
}

/**
 * 加载类数据库文件
 * @param pDirPath 数据库文件存放路径
 */
void CClassDatabaseManager::LoadDatabaseFiles(const char * pDirPath/* = NULL*/)
{
	if (NULL == pDirPath) pDirPath = g_pDefaultDatabaseDirPath;
	string Pattern(pDirPath);
	Pattern.append("\\*.dat");

	WIN32_FIND_DATAA FindData;
	HANDLE hFind = FindFirstFileA(Pattern.c_str(), &FindData);
	if (INVALID_HANDLE_VALUE != hFind)
	{
		set<string> DatabaseFiles;
		try
		{
			do
			{
				if (HAS_BITS(FindData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY)) continue;
				DatabaseFiles.insert(FindData.cFileName);
			} while (FindNextFileA(hFind, &FindData));
			FindClose(hFind);
		}
		catch (...)
		{
			FindClose(hFind);
			throw;
		}

		for (set<string>::iterator It = DatabaseFiles.begin(); It != DatabaseFiles.end(); ++It)
		{
			string DatabaseFilePath(pDirPath);
			DatabaseFilePath.append("\\").append(*It);
			LoadDatabaseFile(DatabaseFilePath.c_str());
		}
	}
}

/**
 * 添加类数据库
 */
void CClassDatabaseManager::PutClassDatabase(class_database_ptr ClassDatabase)
{
	for (BYTE i = 0; i < ClassDatabase->header.unityVersionCount; ++i)
		m_ClassDatabases[ClassDatabase->header.pUnityVersions[i]] = ClassDatabase;
}

/**
 * 查找类数据库
 *
 * @param UnityVersion Unity版本
 */
class_database_ptr CClassDatabaseManager::LookupClassDatabase(const string &UnityVersion)
{
	class_databases_map::iterator It = m_ClassDatabases.find(UnityVersion);
	if (It != m_ClassDatabases.end()) return It->second;

	vector<string> Versions;
	SpliteString(Versions, UnityVersion.c_str(), UnityVersion.length(), '.', true, 3);
	if(Versions.size() != 3) return class_database_ptr();

	It = m_ClassDatabases.find(Versions[0] + "." + Versions[1] + ".*");
	if (It != m_ClassDatabases.end()) return It->second;

	It = m_ClassDatabases.find(Versions[0] + ".*");
	if (It != m_ClassDatabases.end()) return It->second;
	return class_database_ptr();
}

/**
 * 加载类数据库文件
 * @param pClassDatabaseFile 类型数据库文件
 */
void CClassDatabaseManager::LoadDatabaseFile(const char * pClassDatabaseFile)
{
	FILE *fpFile = NULL;
	fpFile = fopen(pClassDatabaseFile, "rb");

	class_database_ptr ClassDatabase(new ClassDatabaseFile());
	if (!ClassDatabase->Read(AssetsReaderFromFile, (LPARAM)fpFile))
		throw runtime_error(FormatStr("Failed to load class database file: %s", pClassDatabaseFile));
	PutClassDatabase(ClassDatabase);
	//需要close文件么？
}
