#include "base.h"

#ifdef _WIN32
#else
#include <fcntl.h>
#include <dirent.h>
#include <fnmatch.h>
#include <sys/stat.h>
#endif
#include <set>

#include "logging.h"
#include "utils.h"

/**
 * 日志等级的名称。
 */
static const char g_szLogLevelNames[5][8] = { "DEBUG", "INFO", "WARN", "ERROR", "FATAL" };

/**
 * 构造函数。
 *
 * @param pLowestLogLevel 日志的最低记录等级，从低到高为 debug;info;warn;error;fatal。等级比该值低的日志将不会被记录。
 */
CLogger::CLogger( const char *pLowestLogLevel )
{
	m_iLowestLogLevel = 1;

	UINT_PTR i;
	for( i = 0; i < sizeof(g_szLogLevelNames) / sizeof(g_szLogLevelNames[0]); i++ )
	{
		if( stricmp(pLowestLogLevel, g_szLogLevelNames[i]) == 0 )
		{
			m_iLowestLogLevel = i;
			break;
		}
	}
}

/**
 * 构造函数。
 *
 * @param pLowestLogLevel 日志的最低记录等级，等级比该值低的日志将不会被记录。
 * @param FilePath 日志文件的路径。
 * @param nMaxFileSize 可保存的日志文件最大字节数。
 * @param nMaxFileBackups 备份的日志文件最大数量。
 */
CFileLogger::CFileLogger( const char *pLowestLogLevel, const tstring &FilePath, size_t nMaxFileSize, size_t nMaxFileBackups ) : CLogger( pLowestLogLevel )
{
	m_FilePath = FilePath;
	m_nMaxFileSize = nMaxFileSize;
	m_nMaxFileBackups = nMaxFileBackups;

#if defined( _WIN32 ) // 操作系统是Windows
	m_hFile = INVALID_HANDLE_VALUE;
#else
	m_iFile = -1;
#endif
	m_nFileSize = 0;
}

CFileLogger::~CFileLogger()
{
#if defined( _WIN32 ) // 操作系统是Windows
	if( m_hFile != INVALID_HANDLE_VALUE ) CloseHandle( m_hFile );
#else
	if( m_iFile != -1 ) close( m_iFile );
#endif
}

/**
 * @copydoc CLogger::DoLog
 */
/* virtual */ void CFileLogger::DoLog( const TCHAR *pLog, size_t nLogLen )
{
	try
	{
		CMutexGuard Lock( m_LogMutex ); // 锁定日志操作

#ifdef UNICODE
		size_t nLogSize = sizeof( TCHAR ) * nLogLen;
#else
		size_t &nLogSize = nLogLen;
#endif
		
#if defined( _WIN32 ) // 操作系统是Windows
		DWORD dwError = 0;

		if( m_hFile == INVALID_HANDLE_VALUE ) // 日志文件还没有打开
		{
			m_hFile = CreateFile( m_FilePath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
			if( m_hFile == INVALID_HANDLE_VALUE ) return;

			m_nFileSize = GetFileSize( m_hFile, NULL ); // 获得文件大小
			if( (DWORD)m_nFileSize == INVALID_FILE_SIZE ) dwError = GetLastError(); // 无法获得文件大小
			
			if( dwError == 0 )
			{
				if( m_nFileSize < 1 ) // 是新文件
				{
					DWORD nBytesWritten;
					if( WriteFile(m_hFile,"\xFF\xFE",2,&nBytesWritten,NULL) ) m_nFileSize = 2; // 把BOM写入文件
					else dwError = GetLastError();
				}
				else
				{
					SetFilePointer( m_hFile, 0, NULL, FILE_END );
				}
			}
		}
		
		if( dwError == 0 )
		{
			DWORD nBytesWritten;
			if( !WriteFile(m_hFile,pLog,nLogSize,&nBytesWritten,NULL) ) dwError = GetLastError(); // 写入文件失败
		}

		if( dwError != 0 )
		{
			CloseHandle( m_hFile );
			m_hFile = INVALID_HANDLE_VALUE;
		}
#else
		int iError = 0;

		if( m_iFile == -1 ) // 日志文件还没有打开
		{
			m_iFile = open( m_FilePath.c_str(), O_RDWR | O_CREAT, 0777 );
			if( m_iFile == -1 ) return;

			struct stat Stat;
			if( fstat(m_iFile,&Stat) != 0 ) iError = errno; // 获得文件信息失败
			else m_nFileSize = Stat.st_size;

			if( iError == 0 ) lseek( m_iFile, 0, SEEK_END );
		}

		if( iError == 0 )
		{
			if( write(m_iFile,pLog,nLogSize) == (ssize_t)-1 ) iError = errno; // 写入文件失败
		}

		if( iError != 0 )
		{
			close( m_iFile );
			m_iFile = -1;
		}
#endif
		
		m_nFileSize += nLogSize;
		if( m_nMaxFileSize > 0 && m_nFileSize > m_nMaxFileSize ) BackupCurrentFile(); // 超出最大日志文件的限制
	}
	catch( ... )
	{
	}
}

/**
 * 备份当前的日志文件。
 */
void CFileLogger::BackupCurrentFile()
{
	try
	{
#if defined( _WIN32 ) // 操作系统是Windows
		if( m_hFile == INVALID_HANDLE_VALUE ) return; // 日志文件还没有打开
		
		CloseHandle( m_hFile );
		m_hFile = INVALID_HANDLE_VALUE;

		const TCHAR *pSep = tcsrchr( m_FilePath.c_str(), _T('\\') );

		tstring Pattern( m_FilePath ); // 用于比较文件名是否为备份日志的字符串
		Pattern.append( _T(".????????_??????_???") );

		tstring LogDir = GetDirectoryFromPath( m_FilePath, '\\' ); // 存放日志文件的目录路径

		WIN32_FIND_DATA FindData;
		HANDLE hFind = FindFirstFile( Pattern.c_str(), &FindData ); // 开始查找之前备份的日志文件
		if( hFind != INVALID_HANDLE_VALUE )
		{
			set<tstring> BackupFiles; // 存放所有备份文件名称的vector对象
			try
			{
				do
				{
					if( HAS_BITS(FindData.dwFileAttributes,FILE_ATTRIBUTE_DIRECTORY) ) continue;
					BackupFiles.insert( FindData.cFileName );
				}
				while( FindNextFile(hFind,&FindData) );

				FindClose( hFind );
			}
			catch( ... )
			{
				FindClose( hFind );
				return;
			}

			size_t nBackupCount = BackupFiles.size();
			std::set<tstring>::iterator NameIt = BackupFiles.begin();
			for( ; NameIt != BackupFiles.end() && nBackupCount >= m_nMaxFileBackups; NameIt++ ) // 删除最老的一些日志文件
			{
				tstring DelFilePath( LogDir );
				if( !LogDir.empty() ) DelFilePath.append( 1, _T('\\') );
				DelFilePath.append( *NameIt );
				DeleteFile( DelFilePath.c_str() );
				nBackupCount--;
			}
		}

		SYSTEMTIME Time;
		GetLocalTime( &Time );

		TCHAR szSuffix[32];
		swprintf( szSuffix, 32, L".%04d%02d%02d_%02d%02d%02d_%03d", Time.wYear, Time.wMonth, Time.wDay, Time.wHour, Time.wMinute, Time.wSecond, Time.wMilliseconds );

		wstring BackupFilePath( m_FilePath ); // 新的备份文件路径
		BackupFilePath.append( szSuffix );
		
		MoveFile( m_FilePath.c_str(), BackupFilePath.c_str() );
#else
		if( m_iFile == -1 ) return; // 日志文件还没有打开
		
		close( m_iFile );
		m_iFile = -1;

		const char *pSep = strrchr( m_FilePath.c_str(), '/' );

		string Pattern; // 用于比较文件名是否为备份日志的字符串
		if( pSep == NULL ) Pattern.assign( m_FilePath );
		else Pattern.assign( pSep + 1 );
		Pattern.append( ".????????_??????_???" );

		string LogDir = GetDirectoryFromPath( m_FilePath ); // 存放日志文件的目录路径
		if( LogDir.empty() ) LogDir = ".";

		DIR *pDir = opendir( LogDir.c_str() );
		if( pDir != NULL )
		{
			set<string> BackupFiles; // 存放所有备份文件名称的vector对象
			try
			{
				while( true )
				{
					struct dirent *pEntry = readdir( pDir ); // 获得一个目录条目
					if( pEntry == NULL ) break;

					int iMatch = fnmatch( Pattern.c_str(), pEntry->d_name, FNM_CASEFOLD ); // 判断节点名称是否和查找名称匹配
					if( iMatch == FNM_NOMATCH ) continue; // 名称不匹配
					if( iMatch != 0 ) continue; // 出现错误

					BackupFiles.insert( pEntry->d_name );
				}

				closedir( pDir );
			}
			catch( ... )
			{
				closedir( pDir );
				return;
			}
		
			size_t nBackupCount = BackupFiles.size();
			std::set<string>::iterator NameIt = BackupFiles.begin();
			for( ; NameIt != BackupFiles.end() && nBackupCount >= m_nMaxFileBackups; NameIt++ ) // 删除最老的一些日志文件
			{
				string DelFilePath( LogDir );
				if( !LogDir.empty() ) DelFilePath.append( 1, '/' );
				DelFilePath.append( *NameIt );
				unlink( DelFilePath.c_str() );
				nBackupCount--;
			}
		}

		struct timeval TimeVal;
		if( gettimeofday(&TimeVal,NULL) != 0 ) return;

		struct tm DateTime;
		if( localtime_r(&TimeVal.tv_sec,&DateTime) == NULL ) return;
			
		char szSuffix[32];
		sprintf( szSuffix, ".%04d%02d%02d_%02d%02d%02d_%03d", DateTime.tm_year + 1900, DateTime.tm_mon + 1, DateTime.tm_mday
			, DateTime.tm_hour, DateTime.tm_min, DateTime.tm_sec, (UINT32)(TimeVal.tv_usec/1000) );

		string BackupFilePath( m_FilePath ); // 新的备份文件路径
		BackupFilePath.append( szSuffix );
		
		rename( m_FilePath.c_str(), BackupFilePath.c_str() );
#endif
	}
	catch( ... )
	{
	}
}

/**
 * 构造函数。
 *
 * @param Loggers 日志记录器列表。
 */
CLogService::CLogService( const logger_list &Loggers )
{
	m_iLowestLogLevel = 999;
	for( logger_list::const_iterator it = Loggers.begin(); it != Loggers.end(); ++it )
	{
		if( m_iLowestLogLevel > it->get()->GetLowestLogLevel() ) m_iLowestLogLevel = it->get()->GetLowestLogLevel();

		if( it->get()->IsRaw() ) m_RawLoggers.push_back( *it );
		else m_Loggers.push_back( *it );
	}
}

/**
 * 记录日志信息。
 *
 * @param iLevel 记录的日志等级。
 * @param pOrigin 日志记录产生来源的名称。
 * @param pFormat 要记录的格式化信息，格式必须符合ANSI C的格式化规范。
 * @param pArgs 格式化的参数。
 */
void CLogService::Log( int iLevel, const TCHAR *pOrigin, const TCHAR *pFormat, va_list pArgs )
{
	if( iLevel < m_iLowestLogLevel ) return; // 这个日志不须要记录

	try
	{
		if( !m_RawLoggers.empty() )
		{
			for( logger_list::iterator LoggerIt = m_RawLoggers.begin(); LoggerIt != m_RawLoggers.end(); ++LoggerIt )
			{
				if( iLevel < LoggerIt->get()->GetLowestLogLevel() ) continue; // 这个日志记录器不须要记录日志
#ifdef _MSC_VER
				LoggerIt->get()->DoRawLog( iLevel, pOrigin, pFormat, pArgs );
#else
				va_list pArgs2;
				va_copy( pArgs2, pArgs );
				LoggerIt->get()->DoRawLog( iLevel, pOrigin, pFormat, pArgs2 );
				va_end( pArgs2 );
#endif
			}
		}

		if( m_Loggers.empty() ) return;

		TCHAR szFixBuf[1024];
		vector<TCHAR> BigBuf;
		TCHAR *pBuffer = szFixBuf;
		size_t nMaxBuffer = sizeof( szFixBuf ) / sizeof( szFixBuf[1] );

#if defined( _WIN32 ) // 操作系统是Windows
		SYSTEMTIME LocalTime;
		GetLocalTime( &LocalTime );

		int nLogLen = swprintf( pBuffer, nMaxBuffer, L"{%04d-%02u-%02u %02u:%02u:%02u.%03u} [Thread %u] %s [%hs] "
			, LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay, LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond, LocalTime.wMilliseconds
			, GetCurrentThreadId(), pOrigin, g_szLogLevelNames[iLevel] ); // 生成日志头
#else // 其它操作系统
		struct timeval TimeVal;
		gettimeofday( &TimeVal, NULL );

		struct tm LocalTime;	
		if( localtime_r(&TimeVal.tv_sec,&LocalTime) == NULL ) return;

		int nLogLen = snprintf( pBuffer, nMaxBuffer, "{%04d-%02u-%02u %02u:%02u:%02u.%06u} [Thread %u] %s [%s] "
			, LocalTime.tm_year + 1900, LocalTime.tm_mon + 1, LocalTime.tm_mday, LocalTime.tm_hour, LocalTime.tm_min, LocalTime.tm_sec
			, (UINT32)TimeVal.tv_usec
			, (int)gettid(), pOrigin, g_szLogLevelNames[iLevel] ); // 生成日志头
#endif // _WIN32
		if( nLogLen < 1 || nLogLen >= nMaxBuffer ) return;
		
#ifdef UNICODE
		int nContentLen = vswprintf( &pBuffer[nLogLen], nMaxBuffer - nLogLen, pFormat, pArgs ); // 生成日志内容
#else 
		int nContentLen = vsnprintf( &pBuffer[nLogLen], nMaxBuffer - nLogLen, pFormat, pArgs ); // 生成日志内容
#endif
		if( nContentLen < 1 || nContentLen >= nMaxBuffer - nLogLen ) // 可能是缓冲区不足
		{
			if( nContentLen >= nMaxBuffer - nLogLen ) BigBuf.resize( nLogLen + nContentLen + 16 );
			else BigBuf.resize( 1024 * 8 );
			nMaxBuffer = BigBuf.size();

			tcsncpy( &BigBuf[0], pBuffer, nLogLen );
			pBuffer = &BigBuf[0];
#ifdef UNICODE
			nContentLen = vswprintf( &pBuffer[nLogLen], nMaxBuffer - nLogLen, pFormat, pArgs ); // 生成日志内容
#else 
			nContentLen = vsnprintf( &pBuffer[nLogLen], nMaxBuffer - nLogLen, pFormat, pArgs ); // 生成日志内容
#endif
			if( nContentLen < 1 || nContentLen >= nMaxBuffer - nLogLen ) return; // 可能是缓冲区不足
		}
		nLogLen += nContentLen;
		if( nLogLen > (int)nMaxBuffer - 3 ) return;

#ifdef _WIN32
		pBuffer[nLogLen++] = _T( '\r' ); // 加上回车符
#endif
		pBuffer[nLogLen++] = _T( '\n' ); // 加上换行符
		pBuffer[nLogLen] = 0;
		
		for( logger_list::iterator LoggerIt = m_Loggers.begin(); LoggerIt != m_Loggers.end(); ++LoggerIt )
		{
			if( iLevel < LoggerIt->get()->GetLowestLogLevel() ) continue; // 这个日志记录器不须要记录日志
			LoggerIt->get()->DoLog( pBuffer, nLogLen );
		}
	}
	catch( ... )
	{
	}
}
