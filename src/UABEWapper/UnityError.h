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
 * ��ȡUnity������Ϣ
 *
 * @param iUnityError Unity������
 @ @return �����ı���Ϣ
 */
const char *GetUnityErrorMessage(int iUnityError);



class CUnityError :
	public std::runtime_error
{
public:

	/**
	 * ���캯��
	 *
	 * @param iUnityError Unity������
	 * @param pPrefix Ҫ�ڴ�����Ϣ֮ǰ���õ����ݣ����԰�����ʽ���������������Ҫ�ڴ�����Ϣ֮ǰ�����������ݣ������������ΪNULL��
	 * @param ... ����pPrefix�ĸ�ʽ��������
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
	 * ��ȡUnity������
	 */
	int GetUnityError() const { return m_iUnityError; }

private:
	/**
	 * Unity������
	 */
	int m_iUnityError;

	/**
	 * ������ı���Ϣ
	 */
	string m_ErrorMessage;
};

