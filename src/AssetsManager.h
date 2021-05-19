#pragma once

typedef map<string, set<string>> dependencies_map;

class CAssetsManager
{
public:
	CAssetsManager();
	~CAssetsManager();

	/**
	 * ���������б�
	 *
	 * @param AssetName ��Դ��
	 * @return ��������
	 */
	set<string> LookupDependencies(const string &AssetName)
	{
		dependencies_map::iterator It = m_Dependencies.find(AssetName);
		return It == m_Dependencies.end() ? set<string>() : It->second;
	}

	/**
	 * �������
	 *
	 * @param AssetName ��Դ��
	 * @param Dependencies ��������
	 */
	void PutDependencies(const string &AssetName, const set<string> &Dependencies)
	{
		m_Dependencies[AssetName] = Dependencies;
	}


	/**
	 * �Ƴ�ָ����Դ����������
	 */
	void RemoveDependencies(const string &AssetName)
	{
		m_Dependencies.erase(AssetName);
	}

private:
	dependencies_map m_Dependencies;
};
typedef shared_ptr<CAssetsManager> assets_manager_ptr;
