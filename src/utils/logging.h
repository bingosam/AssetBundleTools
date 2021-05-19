#pragma once

#include "base.h"
#include "thread.h"

/**
 * 日志记录器。
 */
class CLogger
{
public:
	/**
	 * 构造函数。
	 *
	 * @param pLowestLogLevel 日志的最低记录等级，从低到高为 debug;info;warn;error;fatal。等级比该值低的日志将不会被记录。
	 */
	CLogger( const char *pLowestLogLevel );

	virtual ~CLogger() {}

	/**
	 * 获得日志的最低记录等级数字值，等级比该值低的日志将不会被记录。
	 */
	INT_PTR GetLowestLogLevel() const { return m_iLowestLogLevel; }

	/**
	 * 判断是否记录原始格式的日志信息。
	 */
	virtual bool IsRaw() = 0;

	/**
	 * 记录已格式化的日志信息。
	 *
	 * @param pLog 要记录的日志内容。
	 * @param nLogLen 日志内容的字符数。
	 */
	virtual void DoLog( const TCHAR *pLog, size_t nLogLen ) {}

	/**
	 * 记录原始格式的日志信息。
	 *
	 * @param iLevel 记录的日志等级。
	 * @param pOrigin 日志记录产生来源的名称。
	 * @param pFormat 要记录的格式化信息，格式必须符合ANSI C的格式化规范。
	 * @param pArgs 格式化的参数。
	 */
	virtual void DoRawLog( int iLevel, const TCHAR *pOrigin, const TCHAR *pFormat, va_list pArgs ) {}

private:
	/**
	 * 日志的最低记录等级。
	 */ 
	INT_PTR m_iLowestLogLevel;
};

/**
 * 基于标准输出设备的日志记录器。
 */
class CStdoutLogger : public CLogger
{
public:
	/**
	 * 构造函数。
	 *
	 * @param iLowestLogLevel 日志的最低记录等级，等级比该值低的日志将不会被记录。
	 * @param LogPath 日志文件的路径。
	 * @param nMaxLogSize 可保存的日志文件最大字节数。
	 * @param nBufferSize 日志文件的缓冲区大小。
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
 * 基于文件的日志记录器。
 */
class CFileLogger : public CLogger
{
public:
	/**
	 * 构造函数。
	 *
	 * @param pLowestLogLevel 日志的最低记录等级，等级比该值低的日志将不会被记录。
	 * @param FilePath 日志文件的路径。
	 * @param nMaxFileSize 可保存的日志文件最大字节数。
	 * @param nMaxFileBackups 备份的日志文件最大数量。
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
	 * 日志文件的路径。
	 */
	tstring m_FilePath;

	/**
	 * 可保存的日志文件最大字节数。
	 */
	size_t m_nMaxFileSize;

	/**
	 * 备份的日志文件最大数量。
	 */
	size_t m_nMaxFileBackups;
	
	/**
	 * 用于操作日志的互斥对象。
	 */
	CMutex m_LogMutex;

#if defined( _WIN32 ) // 操作系统是Windows
	/**
	 * 当前打开的日志文件描述符。
	 */
	HANDLE m_hFile;
#else
	/**
	 * 当前打开的日志文件描述符。
	 */
	int m_iFile;
#endif

	/**
	 * 日志文件的大小。
	 */
	size_t m_nFileSize;

	/**
	 * 备份当前的日志文件。
	 */
	void BackupCurrentFile();
};

/**
 * 日志记录服务。
 */
class CLogService
{
public:
	/**
	 * 日志的等级，从低到高。
	 */
	typedef enum
	{
		SVC_LOG_DEBUG = 0, ///< 调试。
		SVC_LOG_INFO = 1, ///< 常规。
		SVC_LOG_WARN = 2, ///< 警告。
		SVC_LOG_ERROR = 3, ///< 一般错误。
		SVC_LOG_FATAL = 4 ///< 致命错误。
	} SVC_LOG_LEVEL;

	/**
	 * 构造函数。
	 *
	 * @param Loggers 日志记录器列表。
	 */
	CLogService( const std::list< shared_ptr<CLogger> > &m_Loggers );

	virtual ~CLogService() {}

	/**
	 * 记录调试信息。
	 *
	 * @param pOrigin 日志记录产生来源的名称。
	 * @param pFormat 要记录的格式化信息，格式必须符合ANSI C的格式化规范。
	 * @param ... 格式化的参数。
	 */
	void Debug( const TCHAR *pOrigin, const TCHAR *pFormat, ... )
	{
		if( SVC_LOG_DEBUG < m_iLowestLogLevel ) return; // 这个日志不须要记录

		va_list pArgs;
		va_start( pArgs, pFormat );
		Log( SVC_LOG_DEBUG, pOrigin, pFormat, pArgs );
		va_end( pArgs );
	}

	/**
	 * 记录常规信息。
	 *
	 * @param pOrigin 日志记录产生来源的名称。
	 * @param pFormat 要记录的格式化信息，格式必须符合ANSI C的格式化规范。
	 * @param ... 格式化的参数。
	 */
	void Info( const TCHAR *pOrigin, const TCHAR *pFormat, ... )
	{
		if( SVC_LOG_INFO < m_iLowestLogLevel ) return; // 这个日志不须要记录

		va_list pArgs;
		va_start( pArgs, pFormat );
		Log( SVC_LOG_INFO, pOrigin, pFormat, pArgs );
		va_end( pArgs );
	}

	/**
	 * 记录警告信息。
	 *
	 * @param pOrigin 日志记录产生来源的名称。
	 * @param pFormat 要记录的格式化信息，格式必须符合ANSI C的格式化规范。
	 * @param ... 格式化的参数。
	 */
	void Warn( const TCHAR *pOrigin, const TCHAR *pFormat, ... )
	{
		if( SVC_LOG_WARN < m_iLowestLogLevel ) return; // 这个日志不须要记录

		va_list pArgs;
		va_start( pArgs, pFormat );
		Log( SVC_LOG_WARN, pOrigin, pFormat, pArgs );
		va_end( pArgs );
	}

	/**
	 * 记录一般错误信息。
	 *
	 * @param pOrigin 日志记录产生来源的名称。
	 * @param pFormat 要记录的格式化信息，格式必须符合ANSI C的格式化规范。
	 * @param ... 格式化的参数。
	 * @return 如果执行成功则为true，否则为false。
	 */
	void Error( const TCHAR *pOrigin, const TCHAR *pFormat, ... )
	{
		if( SVC_LOG_ERROR < m_iLowestLogLevel ) return; // 这个日志不须要记录

		va_list pArgs;
		va_start( pArgs, pFormat );
		Log( SVC_LOG_ERROR, pOrigin, pFormat, pArgs );
		va_end( pArgs );
	}

	/**
	 * 记录一个异常对象为错误信息。
	 *
	 * @param pOrigin 日志记录产生来源的名称。
	 * @param e 要记录的异常对象。
	 */
	void Error( const TCHAR *pOrigin, const std::exception &e )
	{
		if( SVC_LOG_ERROR < m_iLowestLogLevel ) return; // 这个日志不须要记录

#ifdef _DEBUG
		Error( pOrigin, _T("<%hs> %hs"), typeid(e).name(), e.what() );
#else
		Error( pOrigin, _T("%hs"), e.what() );
#endif
	}

	/**
	 * 记录致命错误信息。
	 *
	 * @param pOrigin 日志记录产生来源的名称。
	 * @param pFormat 要记录的格式化信息，格式必须符合ANSI C的格式化规范。
	 * @param ... 格式化的参数。
	 */
	void Fatal( const TCHAR *pOrigin, const TCHAR *pFormat, ... )
	{
		if( SVC_LOG_FATAL < m_iLowestLogLevel ) return; // 这个日志不须要记录

		va_list pArgs;
		va_start( pArgs, pFormat );
		Log( SVC_LOG_FATAL, pOrigin, pFormat, pArgs );
		va_end( pArgs );
	}

	/**
	 * 记录日志信息。
	 *
	 * @param iLevel 记录的日志等级，请参见SVC_LOG_LEVEL。
	 * @param pOrigin 日志记录产生来源的名称。
	 * @param pFormat 要记录的格式化信息，格式必须符合ANSI C的格式化规范。
	 * @param pArgs 格式化的参数。
	 */
	void Log( int iLevel, const TCHAR *pOrigin, const TCHAR *pFormat, va_list pArgs );

private:
	/**
	 * 存放日志记录器的list对象。
	 */
	typedef std::list< shared_ptr<CLogger> > logger_list;
	
	/**
	 * 当前可用日志记录器列表。
	 */
	logger_list m_Loggers;

	/**
	 * 当前可用原始格式日志记录器列表。
	 */
	logger_list m_RawLoggers;

	/**
	 * 日志的最低记录等级。
	 */ 
	int m_iLowestLogLevel;
};
