#pragma once


typedef shared_ptr<ClassDatabaseFile> class_database_ptr;
typedef map<string, class_database_ptr> class_databases_map;

class CClassDatabaseManager
{
public:
	CClassDatabaseManager();
	~CClassDatabaseManager();

	/**
	 * ���������ݿ��ļ�
	 *
	 * @param pDirPath ���ݿ��ļ����·��
	 */
	void LoadDatabaseFiles(const char *pDirPath = NULL);

	/**
	 * ��������ݿ�
	 */
	void PutClassDatabase(class_database_ptr ClassDatabase);

	/**
	 * ���������ݿ�
	 *
	 * @param UnityVersion Unity�汾
	 */
	class_database_ptr LookupClassDatabase(const string &UnityVersion);

private:
	class_databases_map m_ClassDatabases;

	/**
	 * ���������ݿ��ļ�
	 *
	 * @param pClassDatabaseFile �������ݿ��ļ�
	 */
	void LoadDatabaseFile(const char *pClassDatabaseFile);
};

