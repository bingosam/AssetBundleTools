#pragma once

#include "../utils/utils.h"

typedef enum eERROR_CODE {
	SUCCESSED = 0,
	FILE_OPEN_FAILED = 0xFF00,
	FILE_WRITE_FAILED,
	ASSETS_BUNDLE_FILE_READ_FAILED,
	ASSETS_BUNDLE_FILE_UNPACK_FAILED,
	ASSETS_BUNDLE_FILE_VERSION_UNSUPPORTED,
	ASSETS_BUNDLE_FILE_NOT_COMPRESSED,
	MAKE_ASSETS_FILE_READER_FAILED,
	ASSETS_DATA_READ_FAILED,
	INVALID_ASSETS_FILE,
	ASSETS_FILE_NOT_FOUND,
	ASSET_NOT_FOUND,
	PICTURE_ASSET_READ_FAILED,
	PICTURE_EXPORT_FAILED,
}ERROR_CODE;


/**
 * 获取Unity错误信息
 *
 * @param iUnityError Unity错误码
 @ @return 错误文本信息
 */
const char *GetUnityErrorMessage(int iUnityError);



class CUnityError :
	public std::runtime_error
{
public:

	/**
	 * 构造函数
	 *
	 * @param iUnityError Unity错误码
	 * @param pPrefix 要在错误信息之前放置的内容，可以包含格式化参数。如果不须要在错误信息之前放置其它内容，这个参数可以为NULL。
	 * @param ... 用于pPrefix的格式化参数。
	 */
	CUnityError(int iUnityError, const char *pPrefix = NULL, ...) : 
		runtime_error(string())
		, m_iUnityError(iUnityError)
	{
		if (NULL != pPrefix) {
			va_list pArgs;
			va_start(pArgs, pPrefix);
			try
			{
				m_ErrorMessage.assign(VFormatStr(pPrefix, pArgs)).append(" : ");
			}
			catch (...)
			{
				va_end(pArgs);
				throw;
			}
			va_end(pArgs);
		}
		m_ErrorMessage.append(GetUnityErrorMessage(m_iUnityError));
	}

	virtual ~CUnityError() throw()
	{
	}

	virtual const char *what() const throw()
	{
		return m_ErrorMessage.c_str();
	}

	/**
	 * 获取Unity错误码
	 */
	int GetUnityError() const { return m_iUnityError; }

private:
	/**
	 * Unity错误码
	 */
	int m_iUnityError;

	/**
	 * 错误的文本信息
	 */
	string m_ErrorMessage;
};

