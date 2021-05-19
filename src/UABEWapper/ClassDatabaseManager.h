#pragma once


typedef shared_ptr<ClassDatabaseFile> class_database_ptr;
typedef map<string, class_database_ptr> class_databases_map;

class CClassDatabaseManager
{
public:
	CClassDatabaseManager();
	~CClassDatabaseManager();

	/**
	 * 加载类数据库文件
	 *
	 * @param pDirPath 数据库文件存放路径
	 */
	void LoadDatabaseFiles(const char *pDirPath = NULL);

	/**
	 * 添加类数据库
	 */
	void PutClassDatabase(class_database_ptr ClassDatabase);

	/**
	 * 查找类数据库
	 *
	 * @param UnityVersion Unity版本
	 */
	class_database_ptr LookupClassDatabase(const string &UnityVersion);

private:
	class_databases_map m_ClassDatabases;

	/**
	 * 加载类数据库文件
	 *
	 * @param pClassDatabaseFile 类型数据库文件
	 */
	void LoadDatabaseFile(const char *pClassDatabaseFile);
};

