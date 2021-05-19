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
 * ��־�ȼ������ơ�
 */
static const char g_szLogLevelNames[5][8] = { "DEBUG", "INFO", "WARN", "ERROR", "FATAL" };

/**
 * ���캯����
 *
 * @param pLowestLogLevel ��־����ͼ�¼�ȼ����ӵ͵���Ϊ debug;info;warn;error;fatal���ȼ��ȸ�ֵ�͵���־�����ᱻ��¼��
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
 * ���캯����
 *
 * @param pLowestLogLevel ��־����ͼ�¼�ȼ����ȼ��ȸ�ֵ�͵���־�����ᱻ��¼��
 * @param FilePath ��־�ļ���·����
 * @param nMaxFileSize �ɱ������־�ļ�����ֽ�����
 * @param nMaxFileBackups ���ݵ���־�ļ����������
 */
CFileLogger::CFileLogger( const char *pLowestLogLevel, const tstring &FilePath, size_t nMaxFileSize, size_t nMaxFileBackups ) : CLogger( pLowestLogLevel )
{
	m_FilePath = FilePath;
	m_nMaxFileSize = nMaxFileSize;
	m_nMaxFileBackups = nMaxFileBackups;

#if defined( _WIN32 ) // ����ϵͳ��Windows
	m_hFile = INVALID_HANDLE_VALUE;
#else
	m_iFile = -1;
#endif
	m_nFileSize = 0;
}

CFileLogger::~CFileLogger()
{
#if defined( _WIN32 ) // ����ϵͳ��Windows
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
		CMutexGuard Lock( m_LogMutex ); // ������־����

#ifdef UNICODE
		size_t nLogSize = sizeof( TCHAR ) * nLogLen;
#else
		size_t &nLogSize = nLogLen;
#endif
		
#if defined( _WIN32 ) // ����ϵͳ��Windows
		DWORD dwError = 0;

		if( m_hFile == INVALID_HANDLE_VALUE ) // ��־�ļ���û�д�
		{
			m_hFile = CreateFile( m_FilePath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
			if( m_hFile == INVALID_HANDLE_VALUE ) return;

			m_nFileSize = GetFileSize( m_hFile, NULL ); // ����ļ���С
			if( (DWORD)m_nFileSize == INVALID_FILE_SIZE ) dwError = GetLastError(); // �޷�����ļ���С
			
			if( dwError == 0 )
			{
				if( m_nFileSize < 1 ) // �����ļ�
				{
					DWORD nBytesWritten;
					if( WriteFile(m_hFile,"\xFF\xFE",2,&nBytesWritten,NULL) ) m_nFileSize = 2; // ��BOMд���ļ�
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
			if( !WriteFile(m_hFile,pLog,nLogSize,&nBytesWritten,NULL) ) dwError = GetLastError(); // д���ļ�ʧ��
		}

		if( dwError != 0 )
		{
			CloseHandle( m_hFile );
			m_hFile = INVALID_HANDLE_VALUE;
		}
#else
		int iError = 0;

		if( m_iFile == -1 ) // ��־�ļ���û�д�
		{
			m_iFile = open( m_FilePath.c_str(), O_RDWR | O_CREAT, 0777 );
			if( m_iFile == -1 ) return;

			struct stat Stat;
			if( fstat(m_iFile,&Stat) != 0 ) iError = errno; // ����ļ���Ϣʧ��
			else m_nFileSize = Stat.st_size;

			if( iError == 0 ) lseek( m_iFile, 0, SEEK_END );
		}

		if( iError == 0 )
		{
			if( write(m_iFile,pLog,nLogSize) == (ssize_t)-1 ) iError = errno; // д���ļ�ʧ��
		}

		if( iError != 0 )
		{
			close( m_iFile );
			m_iFile = -1;
		}
#endif
		
		m_nFileSize += nLogSize;
		if( m_nMaxFileSize > 0 && m_nFileSize > m_nMaxFileSize ) BackupCurrentFile(); // ���������־�ļ�������
	}
	catch( ... )
	{
	}
}

/**
 * ���ݵ�ǰ����־�ļ���
 */
void CFileLogger::BackupCurrentFile()
{
	try
	{
#if defined( _WIN32 ) // ����ϵͳ��Windows
		if( m_hFile == INVALID_HANDLE_VALUE ) return; // ��־�ļ���û�д�
		
		CloseHandle( m_hFile );
		m_hFile = INVALID_HANDLE_VALUE;

		const TCHAR *pSep = tcsrchr( m_FilePath.c_str(), _T('\\') );

		tstring Pattern( m_FilePath ); // ���ڱȽ��ļ����Ƿ�Ϊ������־���ַ���
		Pattern.append( _T(".????????_??????_???") );

		tstring LogDir = GetDirectoryFromPath( m_FilePath, '\\' ); // �����־�ļ���Ŀ¼·��

		WIN32_FIND_DATA FindData;
		HANDLE hFind = FindFirstFile( Pattern.c_str(), &FindData ); // ��ʼ����֮ǰ���ݵ���־�ļ�
		if( hFind != INVALID_HANDLE_VALUE )
		{
			set<tstring> BackupFiles; // ������б����ļ����Ƶ�vector����
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
			for( ; NameIt != BackupFiles.end() && nBackupCount >= m_nMaxFileBackups; NameIt++ ) // ɾ�����ϵ�һЩ��־�ļ�
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

		wstring BackupFilePath( m_FilePath ); // �µı����ļ�·��
		BackupFilePath.append( szSuffix );
		
		MoveFile( m_FilePath.c_str(), BackupFilePath.c_str() );
#else
		if( m_iFile == -1 ) return; // ��־�ļ���û�д�
		
		close( m_iFile );
		m_iFile = -1;

		const char *pSep = strrchr( m_FilePath.c_str(), '/' );

		string Pattern; // ���ڱȽ��ļ����Ƿ�Ϊ������־���ַ���
		if( pSep == NULL ) Pattern.assign( m_FilePath );
		else Pattern.assign( pSep + 1 );
		Pattern.append( ".????????_??????_???" );

		string LogDir = GetDirectoryFromPath( m_FilePath ); // �����־�ļ���Ŀ¼·��
		if( LogDir.empty() ) LogDir = ".";

		DIR *pDir = opendir( LogDir.c_str() );
		if( pDir != NULL )
		{
			set<string> BackupFiles; // ������б����ļ����Ƶ�vector����
			try
			{
				while( true )
				{
					struct dirent *pEntry = readdir( pDir ); // ���һ��Ŀ¼��Ŀ
					if( pEntry == NULL ) break;

					int iMatch = fnmatch( Pattern.c_str(), pEntry->d_name, FNM_CASEFOLD ); // �жϽڵ������Ƿ�Ͳ�������ƥ��
					if( iMatch == FNM_NOMATCH ) continue; // ���Ʋ�ƥ��
					if( iMatch != 0 ) continue; // ���ִ���

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
			for( ; NameIt != BackupFiles.end() && nBackupCount >= m_nMaxFileBackups; NameIt++ ) // ɾ�����ϵ�һЩ��־�ļ�
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

		string BackupFilePath( m_FilePath ); // �µı����ļ�·��
		BackupFilePath.append( szSuffix );
		
		rename( m_FilePath.c_str(), BackupFilePath.c_str() );
#endif
	}
	catch( ... )
	{
	}
}

/**
 * ���캯����
 *
 * @param Loggers ��־��¼���б�
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
 * ��¼��־��Ϣ��
 *
 * @param iLevel ��¼����־�ȼ���
 * @param pOrigin ��־��¼������Դ�����ơ�
 * @param pFormat Ҫ��¼�ĸ�ʽ����Ϣ����ʽ�������ANSI C�ĸ�ʽ���淶��
 * @param pArgs ��ʽ���Ĳ�����
 */
void CLogService::Log( int iLevel, const TCHAR *pOrigin, const TCHAR *pFormat, va_list pArgs )
{
	if( iLevel < m_iLowestLogLevel ) return; // �����־����Ҫ��¼

	try
	{
		if( !m_RawLoggers.empty() )
		{
			for( logger_list::iterator LoggerIt = m_RawLoggers.begin(); LoggerIt != m_RawLoggers.end(); ++LoggerIt )
			{
				if( iLevel < LoggerIt->get()->GetLowestLogLevel() ) continue; // �����־��¼������Ҫ��¼��־
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

#if defined( _WIN32 ) // ����ϵͳ��Windows
		SYSTEMTIME LocalTime;
		GetLocalTime( &LocalTime );

		int nLogLen = swprintf( pBuffer, nMaxBuffer, L"{%04d-%02u-%02u %02u:%02u:%02u.%03u} [Thread %u] %s [%hs] "
			, LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay, LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond, LocalTime.wMilliseconds
			, GetCurrentThreadId(), pOrigin, g_szLogLevelNames[iLevel] ); // ������־ͷ
#else // ��������ϵͳ
		struct timeval TimeVal;
		gettimeofday( &TimeVal, NULL );

		struct tm LocalTime;	
		if( localtime_r(&TimeVal.tv_sec,&LocalTime) == NULL ) return;

		int nLogLen = snprintf( pBuffer, nMaxBuffer, "{%04d-%02u-%02u %02u:%02u:%02u.%06u} [Thread %u] %s [%s] "
			, LocalTime.tm_year + 1900, LocalTime.tm_mon + 1, LocalTime.tm_mday, LocalTime.tm_hour, LocalTime.tm_min, LocalTime.tm_sec
			, (UINT32)TimeVal.tv_usec
			, (int)gettid(), pOrigin, g_szLogLevelNames[iLevel] ); // ������־ͷ
#endif // _WIN32
		if( nLogLen < 1 || nLogLen >= nMaxBuffer ) return;
		
#ifdef UNICODE
		int nContentLen = vswprintf( &pBuffer[nLogLen], nMaxBuffer - nLogLen, pFormat, pArgs ); // ������־����
#else 
		int nContentLen = vsnprintf( &pBuffer[nLogLen], nMaxBuffer - nLogLen, pFormat, pArgs ); // ������־����
#endif
		if( nContentLen < 1 || nContentLen >= nMaxBuffer - nLogLen ) // �����ǻ���������
		{
			if( nContentLen >= nMaxBuffer - nLogLen ) BigBuf.resize( nLogLen + nContentLen + 16 );
			else BigBuf.resize( 1024 * 8 );
			nMaxBuffer = BigBuf.size();

			tcsncpy( &BigBuf[0], pBuffer, nLogLen );
			pBuffer = &BigBuf[0];
#ifdef UNICODE
			nContentLen = vswprintf( &pBuffer[nLogLen], nMaxBuffer - nLogLen, pFormat, pArgs ); // ������־����
#else 
			nContentLen = vsnprintf( &pBuffer[nLogLen], nMaxBuffer - nLogLen, pFormat, pArgs ); // ������־����
#endif
			if( nContentLen < 1 || nContentLen >= nMaxBuffer - nLogLen ) return; // �����ǻ���������
		}
		nLogLen += nContentLen;
		if( nLogLen > (int)nMaxBuffer - 3 ) return;

#ifdef _WIN32
		pBuffer[nLogLen++] = _T( '\r' ); // ���ϻس���
#endif
		pBuffer[nLogLen++] = _T( '\n' ); // ���ϻ��з�
		pBuffer[nLogLen] = 0;
		
		for( logger_list::iterator LoggerIt = m_Loggers.begin(); LoggerIt != m_Loggers.end(); ++LoggerIt )
		{
			if( iLevel < LoggerIt->get()->GetLowestLogLevel() ) continue; // �����־��¼������Ҫ��¼��־
			LoggerIt->get()->DoLog( pBuffer, nLogLen );
		}
	}
	catch( ... )
	{
	}
}
