#pragma once

#include "base.h"
#include "thread.h"

/**
 * ��־��¼����
 */
class CLogger
{
public:
	/**
	 * ���캯����
	 *
	 * @param pLowestLogLevel ��־����ͼ�¼�ȼ����ӵ͵���Ϊ debug;info;warn;error;fatal���ȼ��ȸ�ֵ�͵���־�����ᱻ��¼��
	 */
	CLogger( const char *pLowestLogLevel );

	virtual ~CLogger() {}

	/**
	 * �����־����ͼ�¼�ȼ�����ֵ���ȼ��ȸ�ֵ�͵���־�����ᱻ��¼��
	 */
	INT_PTR GetLowestLogLevel() const { return m_iLowestLogLevel; }

	/**
	 * �ж��Ƿ��¼ԭʼ��ʽ����־��Ϣ��
	 */
	virtual bool IsRaw() = 0;

	/**
	 * ��¼�Ѹ�ʽ������־��Ϣ��
	 *
	 * @param pLog Ҫ��¼����־���ݡ�
	 * @param nLogLen ��־���ݵ��ַ�����
	 */
	virtual void DoLog( const TCHAR *pLog, size_t nLogLen ) {}

	/**
	 * ��¼ԭʼ��ʽ����־��Ϣ��
	 *
	 * @param iLevel ��¼����־�ȼ���
	 * @param pOrigin ��־��¼������Դ�����ơ�
	 * @param pFormat Ҫ��¼�ĸ�ʽ����Ϣ����ʽ�������ANSI C�ĸ�ʽ���淶��
	 * @param pArgs ��ʽ���Ĳ�����
	 */
	virtual void DoRawLog( int iLevel, const TCHAR *pOrigin, const TCHAR *pFormat, va_list pArgs ) {}

private:
	/**
	 * ��־����ͼ�¼�ȼ���
	 */ 
	INT_PTR m_iLowestLogLevel;
};

/**
 * ���ڱ�׼����豸����־��¼����
 */
class CStdoutLogger : public CLogger
{
public:
	/**
	 * ���캯����
	 *
	 * @param iLowestLogLevel ��־����ͼ�¼�ȼ����ȼ��ȸ�ֵ�͵���־�����ᱻ��¼��
	 * @param LogPath ��־�ļ���·����
	 * @param nMaxLogSize �ɱ������־�ļ�����ֽ�����
	 * @param nBufferSize ��־�ļ��Ļ�������С��
	 */
	CStdoutLogger( const char *pLowestLogLevel ) : CLogger( pLowestLogLevel )
	{
	}
	
	virtual bool IsRaw() { return false; }

	/**
	 * @copydoc CLogger::DoLog
	 */
	virtual void DoLog( const TCHAR *pLog, size_t nLogLen )
	{
#ifdef _UNICODE
		wprintf( pLog );
#else
		printf( pLog );
#endif
	}
};

/**
 * �����ļ�����־��¼����
 */
class CFileLogger : public CLogger
{
public:
	/**
	 * ���캯����
	 *
	 * @param pLowestLogLevel ��־����ͼ�¼�ȼ����ȼ��ȸ�ֵ�͵���־�����ᱻ��¼��
	 * @param FilePath ��־�ļ���·����
	 * @param nMaxFileSize �ɱ������־�ļ�����ֽ�����
	 * @param nMaxFileBackups ���ݵ���־�ļ����������
	 */
	CFileLogger( const char *pLowestLogLevel, const tstring &FilePath, size_t nMaxFileSize, size_t nMaxFileBackups );

	virtual ~CFileLogger();

	virtual bool IsRaw() { return false; }

	/**
	 * @copydoc CLogger::DoLog
	 */
	virtual void DoLog( const TCHAR *pLog, size_t nLogLen );

private:
	/**
	 * ��־�ļ���·����
	 */
	tstring m_FilePath;

	/**
	 * �ɱ������־�ļ�����ֽ�����
	 */
	size_t m_nMaxFileSize;

	/**
	 * ���ݵ���־�ļ����������
	 */
	size_t m_nMaxFileBackups;
	
	/**
	 * ���ڲ�����־�Ļ������
	 */
	CMutex m_LogMutex;

#if defined( _WIN32 ) // ����ϵͳ��Windows
	/**
	 * ��ǰ�򿪵���־�ļ���������
	 */
	HANDLE m_hFile;
#else
	/**
	 * ��ǰ�򿪵���־�ļ���������
	 */
	int m_iFile;
#endif

	/**
	 * ��־�ļ��Ĵ�С��
	 */
	size_t m_nFileSize;

	/**
	 * ���ݵ�ǰ����־�ļ���
	 */
	void BackupCurrentFile();
};

/**
 * ��־��¼����
 */
class CLogService
{
public:
	/**
	 * ��־�ĵȼ����ӵ͵��ߡ�
	 */
	typedef enum
	{
		SVC_LOG_DEBUG = 0, ///< ���ԡ�
		SVC_LOG_INFO = 1, ///< ���档
		SVC_LOG_WARN = 2, ///< ���档
		SVC_LOG_ERROR = 3, ///< һ�����
		SVC_LOG_FATAL = 4 ///< ��������
	} SVC_LOG_LEVEL;

	/**
	 * ���캯����
	 *
	 * @param Loggers ��־��¼���б�
	 */
	CLogService( const std::list< shared_ptr<CLogger> > &m_Loggers );

	virtual ~CLogService() {}

	/**
	 * ��¼������Ϣ��
	 *
	 * @param pOrigin ��־��¼������Դ�����ơ�
	 * @param pFormat Ҫ��¼�ĸ�ʽ����Ϣ����ʽ�������ANSI C�ĸ�ʽ���淶��
	 * @param ... ��ʽ���Ĳ�����
	 */
	void Debug( const TCHAR *pOrigin, const TCHAR *pFormat, ... )
	{
		if( SVC_LOG_DEBUG < m_iLowestLogLevel ) return; // �����־����Ҫ��¼

		va_list pArgs;
		va_start( pArgs, pFormat );
		Log( SVC_LOG_DEBUG, pOrigin, pFormat, pArgs );
		va_end( pArgs );
	}

	/**
	 * ��¼������Ϣ��
	 *
	 * @param pOrigin ��־��¼������Դ�����ơ�
	 * @param pFormat Ҫ��¼�ĸ�ʽ����Ϣ����ʽ�������ANSI C�ĸ�ʽ���淶��
	 * @param ... ��ʽ���Ĳ�����
	 */
	void Info( const TCHAR *pOrigin, const TCHAR *pFormat, ... )
	{
		if( SVC_LOG_INFO < m_iLowestLogLevel ) return; // �����־����Ҫ��¼

		va_list pArgs;
		va_start( pArgs, pFormat );
		Log( SVC_LOG_INFO, pOrigin, pFormat, pArgs );
		va_end( pArgs );
	}

	/**
	 * ��¼������Ϣ��
	 *
	 * @param pOrigin ��־��¼������Դ�����ơ�
	 * @param pFormat Ҫ��¼�ĸ�ʽ����Ϣ����ʽ�������ANSI C�ĸ�ʽ���淶��
	 * @param ... ��ʽ���Ĳ�����
	 */
	void Warn( const TCHAR *pOrigin, const TCHAR *pFormat, ... )
	{
		if( SVC_LOG_WARN < m_iLowestLogLevel ) return; // �����־����Ҫ��¼

		va_list pArgs;
		va_start( pArgs, pFormat );
		Log( SVC_LOG_WARN, pOrigin, pFormat, pArgs );
		va_end( pArgs );
	}

	/**
	 * ��¼һ�������Ϣ��
	 *
	 * @param pOrigin ��־��¼������Դ�����ơ�
	 * @param pFormat Ҫ��¼�ĸ�ʽ����Ϣ����ʽ�������ANSI C�ĸ�ʽ���淶��
	 * @param ... ��ʽ���Ĳ�����
	 * @return ���ִ�гɹ���Ϊtrue������Ϊfalse��
	 */
	void Error( const TCHAR *pOrigin, const TCHAR *pFormat, ... )
	{
		if( SVC_LOG_ERROR < m_iLowestLogLevel ) return; // �����־����Ҫ��¼

		va_list pArgs;
		va_start( pArgs, pFormat );
		Log( SVC_LOG_ERROR, pOrigin, pFormat, pArgs );
		va_end( pArgs );
	}

	/**
	 * ��¼һ���쳣����Ϊ������Ϣ��
	 *
	 * @param pOrigin ��־��¼������Դ�����ơ�
	 * @param e Ҫ��¼���쳣����
	 */
	void Error( const TCHAR *pOrigin, const std::exception &e )
	{
		if( SVC_LOG_ERROR < m_iLowestLogLevel ) return; // �����־����Ҫ��¼

#ifdef _DEBUG
		Error( pOrigin, _T("<%hs> %hs"), typeid(e).name(), e.what() );
#else
		Error( pOrigin, _T("%hs"), e.what() );
#endif
	}

	/**
	 * ��¼����������Ϣ��
	 *
	 * @param pOrigin ��־��¼������Դ�����ơ�
	 * @param pFormat Ҫ��¼�ĸ�ʽ����Ϣ����ʽ�������ANSI C�ĸ�ʽ���淶��
	 * @param ... ��ʽ���Ĳ�����
	 */
	void Fatal( const TCHAR *pOrigin, const TCHAR *pFormat, ... )
	{
		if( SVC_LOG_FATAL < m_iLowestLogLevel ) return; // �����־����Ҫ��¼

		va_list pArgs;
		va_start( pArgs, pFormat );
		Log( SVC_LOG_FATAL, pOrigin, pFormat, pArgs );
		va_end( pArgs );
	}

	/**
	 * ��¼��־��Ϣ��
	 *
	 * @param iLevel ��¼����־�ȼ�����μ�SVC_LOG_LEVEL��
	 * @param pOrigin ��־��¼������Դ�����ơ�
	 * @param pFormat Ҫ��¼�ĸ�ʽ����Ϣ����ʽ�������ANSI C�ĸ�ʽ���淶��
	 * @param pArgs ��ʽ���Ĳ�����
	 */
	void Log( int iLevel, const TCHAR *pOrigin, const TCHAR *pFormat, va_list pArgs );

private:
	/**
	 * �����־��¼����list����
	 */
	typedef std::list< shared_ptr<CLogger> > logger_list;
	
	/**
	 * ��ǰ������־��¼���б�
	 */
	logger_list m_Loggers;

	/**
	 * ��ǰ����ԭʼ��ʽ��־��¼���б�
	 */
	logger_list m_RawLoggers;

	/**
	 * ��־����ͼ�¼�ȼ���
	 */ 
	int m_iLowestLogLevel;
};
