#pragma once

typedef map<string, set<string>> dependencies_map;

class CAssetsManager
{
public:
	CAssetsManager();
	~CAssetsManager();

	/**
	 * 查找依赖列表
	 *
	 * @param AssetName 资源名
	 * @return 依赖集合
	 */
	set<string> LookupDependencies(const string &AssetName)
	{
		dependencies_map::iterator It = m_Dependencies.find(AssetName);
		return It == m_Dependencies.end() ? set<string>() : It->second;
	}

	/**
	 * 添加依赖
	 *
	 * @param AssetName 资源名
	 * @param Dependencies 依赖集合
	 */
	void PutDependencies(const string &AssetName, const set<string> &Dependencies)
	{
		m_Dependencies[AssetName] = Dependencies;
	}


	/**
	 * 移除指定资源的依赖集合
	 */
	void RemoveDependencies(const string &AssetName)
	{
		m_Dependencies.erase(AssetName);
	}

private:
	dependencies_map m_Dependencies;
};
typedef shared_ptr<CAssetsManager> assets_manager_ptr;
