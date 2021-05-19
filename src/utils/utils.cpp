#include "base.h"
#include <math.h>

#ifdef _MSC_VER
#include <intrin.h>
#pragma intrinsic(_InterlockedCompareExchange64)
#endif

#if defined( _WIN32 )
#include <Wincrypt.h>
#include <Lm.h>
#include <Rpc.h>
#include <mlang.h>
#else
#include <fcntl.h>
#include <sys/utsname.h>
#include <langinfo.h>
#include <openssl/md4.h>
#include <openssl/md5.h>
#include <openssl/des.h>
#include <openssl/sha.h>
#include <iconv.h>
#endif

#include "utils.h"

#ifdef _WIN32
/**
 * ���ڲ���������ļ��ܾ����
 */
static HCRYPTPROV g_hRandomCryptProv = 0;
#else
/**
 * ������豸���ļ���������
 */
static int g_iRandomFile = -1;
#endif
/**
 * ����ϵͳ���ơ�
 */
static tstring g_OsName;

/**
 * ����Base64���롣
 */
static char g_cBase64EncodingTable[] = 
{
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/'
};

/**
 * ����Base64���롣
 */
static char g_cBase64DecodingTable[256];

/**
 * ����Base64���㡣
 */
static int g_iBase64ModTable[] = { 0, 2, 1 };

/**
 * �����ǰʹ�õ��ַ�������ΪUTF-8��Ϊtrue��
 */
static bool g_bWithUtf8 = false;

/**
 * ��ʼ��ʵ�ù��ߺ��������Դ��
 */
class CUtilInit
{
public:
	CUtilInit()
	{
#ifdef _WIN32
		// ����� --------------------------
		CryptAcquireContext( &g_hRandomCryptProv, NULL, NULL, PROV_RSA_FULL, 0 );

		// ����ϵͳ���� --------------------------
		OSVERSIONINFOEXW Ver;
		Ver.dwOSVersionInfoSize = sizeof( Ver );
		if( GetVersionExW((LPOSVERSIONINFO)&Ver) )
		{
			g_OsName.resize( 512 );
			if( Ver.dwPlatformId == VER_PLATFORM_WIN32_NT ) // ������Windows NTϵ��
			{
				if( Ver.dwMajorVersion == 5 && Ver.dwMinorVersion == 0 ) tcscpy( &g_OsName[0], _T("Windows 2000") );
				else if( Ver.dwMajorVersion == 5 && Ver.dwMinorVersion == 1 ) tcscpy( &g_OsName[0], _T("Windows XP") );
				else if( Ver.dwMajorVersion == 5 && Ver.dwMinorVersion == 2 ) tcscpy( &g_OsName[0], _T("Windows Server 2003") );
				else if( Ver.dwMajorVersion == 6 && (Ver.wSuiteMask == VER_SUITE_DATACENTER || Ver.wSuiteMask == VER_SUITE_ENTERPRISE) ) tcscpy( &g_OsName[0], _T("Windows Server 2008") );
				else if( Ver.dwMajorVersion == 6 && Ver.dwMinorVersion == 0 ) tcscpy( &g_OsName[0], _T("Windows Vista") );
				else if( Ver.dwMajorVersion == 6 && Ver.dwMinorVersion == 1 ) tcscpy( &g_OsName[0], _T("Windows 7") );
				else if( Ver.dwMajorVersion == 6 && Ver.dwMinorVersion == 2 ) tcscpy( &g_OsName[0], _T("Windows 8") );
				else tsprintf( &g_OsName[0], g_OsName.size(), _T("Windows NT %d.%d"), Ver.dwMajorVersion, Ver.dwMinorVersion );
			}
			else
			{
				tsprintf( &g_OsName[0], g_OsName.size(), _T("Windows %d.%d"), Ver.dwMajorVersion, Ver.dwMinorVersion );
			}
			
			tsprintf( &g_OsName[tcslen(&g_OsName[0])], g_OsName.size(), _T(" %d"), Ver.dwBuildNumber );
			g_OsName.resize( tcslen(&g_OsName[0]) );
		}
#else
		// ����� --------------------------
		g_iRandomFile = open( "/dev/urandom", O_RDONLY );
			
		// ����ϵͳ���� --------------------------
		struct utsname Name;
		if( uname(&Name) == 0 )
		{
			g_OsName = Name.sysname;
			g_OsName.append( Name.release );
		}
		else
		{
			g_OsName = "Unix";
		}
#endif
	 
		for( int i = 0; i < 0x40; i++ )
		{
			g_cBase64DecodingTable[(int)g_cBase64EncodingTable[i]] = (char)i;
		}

#ifndef UNICODE
		char SysCharset[128];
		if( GetSystemCharset(SysCharset, sizeof(SysCharset)) )
		{
			g_bWithUtf8 = stricmp( SysCharset, "utf-8" ) == 0 || stricmp( SysCharset, "utf8" ) == 0;
		}
#endif
	}

	~CUtilInit()
	{
#ifdef _WIN32
		if( g_hRandomCryptProv != 0 ) CryptReleaseContext( g_hRandomCryptProv, 0 );
#else
		if( g_iRandomFile != -1 ) close( g_iRandomFile );
#endif
	}
};
CUtilInit UtilInit;

/**
 * ���ָ������ϵͳ�������ʾ��խ�ַ��ı���Ϣ��
 *
 * @param iOsError ����ϵͳ�Ĵ����룬���Ϊ0���ʾ��ǰ�߳��������Ĵ���
 * @return ���ִ�гɹ���Ϊ��������ı���Ϣ�����ִ��ʧ����Ϊ���ַ�����
 */
string GetOsErrorMessageA(int iOsError /* = 0 */)
{
#ifdef _WIN32
	if (iOsError == 0) iOsError = GetLastError();

	string Buffer(1024, 0);
	for (int i = 0; i < 8; i++)
	{
		DWORD nMessageLen = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS
			, NULL, (DWORD)iOsError, GetUserDefaultLangID(), &Buffer[0], Buffer.size(), NULL); // ���ϵͳ������Ϣ
		if (nMessageLen < 1) return "";

		if (nMessageLen < Buffer.size())
		{
			Buffer.resize(nMessageLen);
			return FormatStr("[SYSERR %u] %s", iOsError, Buffer.c_str());
		}

		Buffer.resize(Buffer.size() * 2);
	}

	return string();
#else
	if (iOsError == 0) iOsError = errno;

	string Buffer(1024, 0);
	char *pMessage = strerror_r(iOsError, &Buffer[0], Buffer.size());
	if (pMessage == NULL) return "";

	return FormatStr("[SYSERR %d] %s", iOsError, pMessage);
#endif
}

/**
 * ���ָ������ϵͳ�������ʾ�Ŀ��ַ��ı���Ϣ��
 *
 * @param iOsError ����ϵͳ�Ĵ����룬���Ϊ0���ʾ��ǰ�߳��������Ĵ���
 * @return ���ִ�гɹ���Ϊ��������ı���Ϣ�����ִ��ʧ����Ϊ���ַ�����
 */
wstring GetOsErrorMessageW(int iOsError /* = 0 */)
{
#ifdef _WIN32
	if (iOsError == 0) iOsError = GetLastError();

	wstring Buffer(1024, 0);
	for (int i = 0; i < 8; i++)
	{
		DWORD nMessageLen = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS
			, NULL, (DWORD)iOsError, GetUserDefaultLangID(), &Buffer[0], Buffer.size(), NULL); // ���ϵͳ������Ϣ
		if (nMessageLen < 1) return L"";

		if (nMessageLen < Buffer.size())
		{
			Buffer.resize(nMessageLen);
			return FormatStr(L"[SYSERR %u] %s", iOsError, Buffer.c_str());
		}

		Buffer.resize(Buffer.size() * 2);
	}

	return wstring();
#else
	const string &MessageA = GetOsErrorMessageA(iOsError);
	if (MessageA.empty()) return wstring();
	else return AnsiStrToWideStr(MessageA.c_str(), MessageA.size());
#endif
}

/**
 * ��õ�ǰʱ�����������֮�����ķ�������<br>
 * ����ʱ��ʱ����ϸ�ֵ�ܹ��õ���Ӧ�ĸ�������ʱ�䡣
 */
INT32 GetTimeZoneBias()
{
#ifdef _WIN32 // ����ϵͳ��Windows
	TIME_ZONE_INFORMATION TimeZone;
	if( GetTimeZoneInformation(&TimeZone) > 2 ) return 0; // ���ʱ����Ϣʧ��

	return (INT32)TimeZone.Bias;
#else
	struct timeval Tv;
	struct timezone TimeZone;
	if( gettimeofday(&Tv, &TimeZone) != 0 ) return 0;
	else return (INT32)TimeZone.tz_minuteswest;
	/*
	tzset();
	return (INT16)( timezone / 60 );
	*/
#endif
}

/**
 * ��ôӲ���ϵͳ������ʼ����ǰ��΢������
 */
INT64 GetRealClock()
{
#ifdef _WIN32
	static volatile LONG bFrequencyQueried = 0; // ����Ѿ���ѯ�˸߾��ȼ�������Ƶ����Ϊ��0
	static LARGE_INTEGER iFrequency = { 0 };
	
	if( iFrequency.QuadPart == 0 && InterlockedCompareExchange(&bFrequencyQueried,1,0) == 0 )
	{
		if( !QueryPerformanceFrequency(&iFrequency) ) throw COsError( "Failed to QueryPerformanceFrequency:" );
	}
	
	LARGE_INTEGER iPerformanceCount;
	if( !QueryPerformanceCounter(&iPerformanceCount) ) throw COsError( "Failed to QueryPerformanceCounter:" );

	return (INT64)( iPerformanceCount.QuadPart / (double)iFrequency.QuadPart * 1000000 );
#else
	struct timespec Clock;
	if( clock_gettime(CLOCK_REALTIME,&Clock) != 0 ) throw COsError( "Failed to clock_gettime:" );
	return (INT64)Clock.tv_sec * 1000000 + Clock.tv_nsec / 1000;
#endif
}

/**
 * ����һ��32λ��α�������
 *
 * @param iMin �ɲ������������Сֵ��
 * @param iMax �ɲ�������������ֵ��
 * @return �������������
 */
INT32 RandomInt32( INT32 iMin /* = -2147483647 - 1 */, INT32 iMax /* = 2147483647 */ )
{
	static __THREAD UINT32 uiSeed = 0; // ����

	if( iMin >= iMax ) return iMin;

	if( uiSeed == 0 ) // ��û�в�������
	{
#if defined( _WIN32 ) // ����ϵͳ��Windows
		LARGE_INTEGER PerformanceCount;
		if( !QueryPerformanceCounter(&PerformanceCount) ) throw COsError( "Failed to QueryPerformanceCounter:" );
		
		uiSeed = PerformanceCount.HighPart + PerformanceCount.LowPart;
#else
		struct timeval Time;
		if( gettimeofday(&Time,NULL) != 0 ) throw COsError( "Failed to gettimeofday:" );

		uiSeed = Time.tv_sec * Time.tv_usec;
#endif
	}

	uiSeed = uiSeed * 1103515245 + 12345;

	UINT32 nSpan = (UINT32)( iMax - iMin ) + 1; // �������ķ�Χ
	if( nSpan == 0 ) nSpan = 0xFFFFFFFF;
	return (INT32)( uiSeed % nSpan ) + iMin;
}

/**
 * ����һ��64λ��α�������
 *
 * @param iMin �ɲ������������Сֵ��
 * @param iMax �ɲ�������������ֵ��
 * @return �������������
 */
INT64 RandomInt64( INT64 iMin /* = -9223372036854775807LL - 1 */, INT64 iMax /* = 9223372036854775807LL */ )
{
	static __THREAD UINT64 uiSeed = 0; // ����

	if( iMin >= iMax ) return iMin;

	if( uiSeed == 0 ) // ��û�в�������
	{
#if defined( _WIN32 ) // ����ϵͳ��Windows
		if( !QueryPerformanceCounter((LARGE_INTEGER *)&uiSeed) ) throw COsError( "Failed to QueryPerformanceCounter:" );
#else
		struct timeval Time;
		if( gettimeofday(&Time,NULL) != 0 ) throw COsError( "Failed to gettimeofday:" );

		uiSeed = (UINT64)Time.tv_sec * Time.tv_usec + Time.tv_usec;
#endif
	}

	uiSeed = uiSeed * 1103515245 + 12345;

	UINT64 nSpan = (UINT64)( iMax - iMin ) + 1; // �������ķ�Χ
	if( nSpan == 0 ) nSpan = 0xFFFFFFFFFFFFFFFFLL;
	return (INT64)( uiSeed % nSpan ) + iMin;
}

/**
 * ʹ�ø��ɿ��ķ�������һ��32λ���������
 *
 * @param iMin �ɲ������������Сֵ��
 * @param iMax �ɲ�������������ֵ��
 * @return �������������
 * @note ��������������ٶ�Ҫ����RandomInt32��
 */
INT32 RandomInt32Ex( INT32 iMin /* = -2147483647 - 1 */, INT32 iMax /* = 2147483647 */ )
{
	if( iMin >= iMax ) return iMin;

#if defined( _WIN32 ) // ����ϵͳ��Windows
	UINT32 uiValue;
	if( !CryptGenRandom(g_hRandomCryptProv, sizeof(uiValue), (BYTE *)&uiValue) ) throw COsError( "Failed to CryptGenRandom:" );
	
	UINT32 nSpan = (UINT32)( iMax - iMin ) + 1; // �������ķ�Χ
	if( nSpan == 0 ) nSpan = 0xFFFFFFFF;
	return (INT32)( uiValue % nSpan ) + iMin;
#elif defined( __unix__ ) // ����ϵͳ����Unix
	UINT32 uiValue;
	if( read(g_iRandomFile, &uiValue, sizeof(uiValue)) == -1 ) throw COsError( errno, "Failed to read urandom:" );

	UINT32 nSpan = (UINT32)( iMax - iMin ) + 1; // �������ķ�Χ
	if( nSpan == 0 ) nSpan = 0xFFFFFFFF;
	return (INT32)( uiValue % nSpan ) + iMin;
#else
	return RandomInt32( iMin, iMax );
#endif
}

/**
 * ʹ�ø��ɿ��ķ�������һ��64λ���������
 *
 * @param iMin �ɲ������������Сֵ��
 * @param iMax �ɲ�������������ֵ��
 * @return �������������
 * @note ��������������ٶ�Ҫ����RandomInt64��
 */
INT64 RandomInt64Ex( INT64 iMin /* = -9223372036854775807LL - 1 */, INT64 iMax /* = 9223372036854775807LL */ )
{
	if( iMin >= iMax ) return iMin;

#ifdef _WIN32
	UINT64 uiValue;
	if( !CryptGenRandom(g_hRandomCryptProv, sizeof(uiValue), (BYTE *)&uiValue) ) throw COsError( "Failed to CryptGenRandom:" );
	
	UINT64 nSpan = (UINT64)( iMax - iMin ) + 1; // �������ķ�Χ
	if( nSpan == 0 ) nSpan = 0xFFFFFFFFFFFFFFFFLL;
	return (INT64)( uiValue % nSpan ) + iMin;
#else
	UINT64 uiValue;
	if( read(g_iRandomFile, &uiValue, sizeof(uiValue)) == -1 ) throw COsError( "Failed to read urandom:" );

	UINT64 nSpan = (UINT64)( iMax - iMin ) + 1; // �������ķ�Χ
	if( nSpan == 0 ) nSpan = 0xFFFFFFFFFFFFFFFFLL;
	return (INT64)( uiValue % nSpan ) + iMin;
#endif
}

/**
 * ��ñ��ؼ��������������
 */
tstring GetHostName()
{
#if defined( _WIN32 ) // ����ϵͳ��Windows
	TCHAR szName[MAX_COMPUTERNAME_LENGTH+1];
	DWORD nMaxName = sizeof( szName ) / sizeof( szName[0] );
	if( !GetComputerName(szName,&nMaxName) ) return tstring();

	return szName;
#else
    char szName[128];
	if( gethostname(szName,sizeof(szName)) != 0 ) return tstring();

	return szName;
#endif	
}

/**
 * ��ñ��ؼ�������ڵ�����������ơ�
 */
tstring GetDomainName()
{
#if defined( _WIN32 )
	tstring DomainName;

	WKSTA_INFO_100 *pWorkstationInfo;
	if( NetWkstaGetInfo(NULL,100,(LPBYTE *)&pWorkstationInfo ) == NERR_Success ) // ��ñ�������վ��Ϣ
	{
		DomainName.assign( pWorkstationInfo->wki100_langroup );
		NetApiBufferFree( pWorkstationInfo );
	}

	return DomainName;
#else
	return tstring();
#endif
}

/**
 * ��ò���ϵͳ���ơ�
 *
 * @return ����ϵͳ���ơ�
 */
tstring GetOsName()
{
	return g_OsName;
}

/**
 * �ж�һ�����ݵ������Ƿ�ȫ��Ϊ0ֵ��
 *
 * @param pData ������ݵĻ�������
 * @param nSize ���ݵ��ֽ�����
 * @return ������ݵ�����ȫ��Ϊ0ֵ��Ϊtrue������Ϊfalse��
 */
bool DataIsZeros( const void *pData, UINT32 nSize )
{
	UINT32 i;
	for( i = 0; i < nSize; i++ ) if( ((const BYTE *)pData)[i] != 0 ) return false;
	return true;
}

static UINT32 g_Crc32Table[256] = 
{
	0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
	0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
	0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
	0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
	0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
	0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
	0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
	0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
	0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
	0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
	0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
	0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
	0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
	0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
	0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
	0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
	0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
	0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
	0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
	0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
	0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
	0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
	0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
	0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
	0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
	0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
	0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
	0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
	0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
	0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
	0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
	0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
	0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
	0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
	0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
	0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
	0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
	0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
	0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
	0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
	0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
	0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
	0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
	0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
	0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
	0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
	0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
	0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
	0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
	0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
	0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
	0x2d02ef8dL
};

/**
 * ��CRC32�㷨��������ݵ�4���ֽ�ɢ��ֵ��
 *
 * @param pData Ҫ��������ݡ�
 * @param nDataSize Ҫ����������ֽ�����
 * @return ����Ľ����
 * @author ksaiy
 * @note ����ժ��<a href="http://blog.csdn.net/ksaiy/archive/2005/05/27/382162.aspx">��ȡ�ļ���CRC32ֵ(VC++Դ��-�̶����)����������</a>��
 */
UINT32 CRC32( const BYTE *pData, size_t nDataSize )
{
	UINT32 uiHash = 0xffffffff;	
	for( UINT32 i = 0; i < nDataSize; i++ ) uiHash = ( uiHash >> 8 ) ^ g_Crc32Table[(uiHash&0xFF)^pData[i]];
	return uiHash ^ 0xffffffff;
}

/**
 * ��MurmurHash2�㷨��������ݵ�4���ֽ�ɢ��ֵ��
 *
 * MurmurHash2, by Austin Appleby
 * This code makes a few assumptions about how your machine behaves -
 * 1. We can read a 4-byte value from any address without crashing
 * 2. sizeof(int) == 4
 * And it has a few limitations -
 * 1. It will not work incrementally.
 * 2. It will not produce the same results on little-endian and big-endian machines.
 *
 * @param pData Ҫ��������ݡ�
 * @param nDataSize Ҫ����������ֽ�����
 * @param uiSeed ����ֵ��
 * @return ����Ľ����
 * @author Austin Appleby
 */
UINT32 MurmurHash32 ( const void *pData, int nDataSize, unsigned int uiSeed )
{
	// 'm' and 'r' are mixing constants generated offline.
	// They're not really 'magic', they just happen to work well.

	const unsigned int m = 0x5bd1e995;
	const int r = 24;

	// Initialize the hash to a 'random' value

	unsigned int h = uiSeed ^ nDataSize;

	// Mix 4 bytes at a time into the hash

	const unsigned char * data = (const unsigned char *)pData;

	while(nDataSize >= 4)
	{
		unsigned int k = *(unsigned int *)data;

		k *= m; 
		k ^= k >> r; 
		k *= m; 
		
		h *= m; 
		h ^= k;

		data += 4;
		nDataSize -= 4;
	}
	
	// Handle the last few bytes of the input array

	switch(nDataSize)
	{
	case 3: h ^= data[2] << 16;
	case 2: h ^= data[1] << 8;
	case 1: h ^= data[0];
	        h *= m;
	};

	// Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.

	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}

/**
 * ��MurmurHash2�㷨��������ݵ�8���ֽ�ɢ��ֵ��
 *
 * MurmurHash2, 64-bit versions, by Austin Appleby<br>
 * The same caveats as 32-bit MurmurHash2 apply here - beware of alignment 
 * and endian-ness issues if used across multiple platforms.
 *
 * @param pData Ҫ��������ݡ�
 * @param nDataSize Ҫ����������ֽ�����
 * @param uiSeed ����ֵ��
 * @return ����Ľ����
 * @author Austin Appleby
 */
UINT64 MurmurHash64( const void *pData, int nDataSize, unsigned int uiSeed )
{
	// 64-bit hash for 32-bit platforms
	const unsigned int m = 0x5bd1e995;
	const int r = 24;

	unsigned int h1 = uiSeed ^ nDataSize;
	unsigned int h2 = 0;

	const unsigned int * data = (const unsigned int *)pData;

	while(nDataSize >= 8)
	{
		unsigned int k1 = *data++;
		k1 *= m; k1 ^= k1 >> r; k1 *= m;
		h1 *= m; h1 ^= k1;
		nDataSize -= 4;

		unsigned int k2 = *data++;
		k2 *= m; k2 ^= k2 >> r; k2 *= m;
		h2 *= m; h2 ^= k2;
		nDataSize -= 4;
	}

	if(nDataSize >= 4)
	{
		unsigned int k1 = *data++;
		k1 *= m; k1 ^= k1 >> r; k1 *= m;
		h1 *= m; h1 ^= k1;
		nDataSize -= 4;
	}

	switch(nDataSize)
	{
	case 3: h2 ^= ((unsigned char*)data)[2] << 16;
	case 2: h2 ^= ((unsigned char*)data)[1] << 8;
	case 1: h2 ^= ((unsigned char*)data)[0];
			h2 *= m;
	};

	h1 ^= h2 >> 18; h1 *= m;
	h2 ^= h1 >> 22; h2 *= m;
	h1 ^= h2 >> 17; h1 *= m;
	h2 ^= h1 >> 19; h2 *= m;

	UINT64 h = h1;

	h = (h << 32) | h2;

	return h;
}

/**
 * ��MD4�㷨�������ݵ�16���ֽ�ɢ��ֵ��
 *
 * @param pData Ҫ��������ݡ�
 * @param nDataSize Ҫ����������ֽ�����
 * @param[out] pMessageDigest �洢ɢ��ֵ�Ļ���������������С��16�ֽڡ�
 */
void MD4Hash( const void *pData, size_t nDataSize, BYTE *pMessageDigest )
{
#if defined( _WIN32 ) // ����ϵͳ��Windows
	HCRYPTPROV hCryptProv = 0;
	HCRYPTHASH hHash = 0;

	try
	{
		if( !CryptAcquireContext(&hCryptProv,NULL,NULL,PROV_RSA_FULL,0) )
		{
			if( GetLastError() != NTE_BAD_KEYSET || !CryptAcquireContext(&hCryptProv,NULL,NULL,PROV_RSA_FULL,CRYPT_NEWKEYSET) ) throw COsError();
		}
		if( !CryptCreateHash(hCryptProv,CALG_MD4,0,0,&hHash) ) throw COsError(); // ������ϣ��ʧ��
		if( !CryptHashData(hHash,(BYTE *)pData,nDataSize,0) ) throw COsError(); // ��������ʧ��

		DWORD nSize = 16;
		if( !CryptGetHashParam(hHash,HP_HASHVAL,pMessageDigest,&nSize,0) ) throw COsError(); // ��ü�����ʧ��		
	}
	catch( ... )
	{
		if( hHash != 0 ) CryptDestroyHash( hHash );
		if( hCryptProv != 0 ) CryptReleaseContext( hCryptProv, 0 );

		throw;
	}

	if( hHash != 0 ) CryptDestroyHash( hHash );
	if( hCryptProv != 0 ) CryptReleaseContext( hCryptProv, 0 );
#elif defined( HEADER_MD4_H ) // ʹ��OpenSSL�ļ���API
	MD4_CTX Context;
	MD4_Init( &Context );
	MD4_Update( &Context, pData, nDataSize );
	MD4_Final( pMessageDigest, &Context );
#else
#error MD4 not implement!
#endif
}

/**
 * ��MD5�㷨�������ݵ�16���ֽ�ɢ��ֵ��
 *
 * @param pData Ҫ��������ݡ�
 * @param nDataSize Ҫ����������ֽ�����
 * @param[out] pMessageDigest �洢ɢ��ֵ�Ļ���������������С��16�ֽڡ�
 */
void MD5Hash(const void *pData, size_t nDataSize, BYTE *pMessageDigest)
{
#if defined( _WIN32 ) // ����ϵͳ��Windows
	HCRYPTPROV hCryptProv = 0;
	HCRYPTHASH hHash = 0;

	try
	{
		if (!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, 0)) // ��CSPʧ��
		{
			if (GetLastError() != NTE_BAD_KEYSET || !CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET)) throw COsError();
		}
		if (!CryptCreateHash(hCryptProv, CALG_MD5, 0, 0, &hHash)) throw COsError(); // ������ϣ��ʧ��
		if (!CryptHashData(hHash, (BYTE *)pData, nDataSize, 0)) throw COsError(); // ��������ʧ��

		DWORD nSize = 16;
		if (!CryptGetHashParam(hHash, HP_HASHVAL, pMessageDigest, &nSize, 0)) throw COsError(); // ��ü�����ʧ��		
	}
	catch (...)
	{
		if (hHash != 0) CryptDestroyHash(hHash);
		if (hCryptProv != 0) CryptReleaseContext(hCryptProv, 0);
		throw;
	}

	if (hHash != 0) CryptDestroyHash(hHash);
	if (hCryptProv != 0) CryptReleaseContext(hCryptProv, 0);
#elif defined( HEADER_MD5_H ) // ʹ��OpenSSL�ļ���API
	MD5_CTX Context;
	MD5_Init(&Context);
	MD5_Update(&Context, pData, nDataSize);
	MD5_Final(pMessageDigest, &Context);
#else
#error MD5 not implement!
#endif
}

/**
 * ��MD5�㷨��������ݵĹ�ϣֵ���õ�ʮ�������ַ�����
 *
 * @param pData Ҫ��������ݡ�
 * @param nDataSize Ҫ����������ֽ�����
 * @param bWithLowerCase ���Ҫ����Сд�ַ�����Ϊtrue��
 * @return ��ϣֵ��ʮ�������ַ�����
 */
string MD5Hash(const void *pData, size_t nDataSize, bool bWithLowerCase /* = true */)
{
	BYTE byHash[16];
	MD5Hash(pData, nDataSize, byHash);
	return BinToHexA(byHash, 16, bWithLowerCase);
}

/**
 * ��HMAC MD5�㷨�������ݵ�16���ֽ�ɢ��ֵ��
 *
 * @param pKey ����ʹ�õĹؼ��֡�
 * @param nKeySize ����ʹ�õĹؼ����ֽ�����
 * @param pData Ҫ��������ݡ�
 * @param nDataSize Ҫ����������ֽ�����
 * @param[out] pMessageDigest �洢ɢ��ֵ�Ļ���������������С��16�ֽڡ�
 */
void HMACMD5( const void *pKey, size_t nKeySize, const void *pData, size_t nDataSize, BYTE *pMessageDigest )
{
	static const size_t __B = 64;

	BYTE byCurKey[__B];
	size_t nCurKeySize;

	if( nKeySize <= __B ) 
	{
		memcpy( byCurKey, pKey, nKeySize );
		nCurKeySize = nKeySize;
	}
	else 
	{
		MD5Hash( pKey, nKeySize, byCurKey );
		nCurKeySize = 16;
	}

	if( nCurKeySize < __B )
	{
		memset( &byCurKey[nCurKeySize], 0, __B - nCurKeySize );
		nCurKeySize = __B;
	}

	UINT_PTR i;

	vector<BYTE> InPad( __B + nDataSize );
	for( i = 0; i < __B; i++ ) InPad[i] = byCurKey[i] ^ 0x36;
	if( nDataSize > 0 ) memcpy( &InPad[__B], pData, nDataSize );

	vector<BYTE> OutPad( __B + 16 );
	for( i = 0; i < __B; i++ ) OutPad[i] = byCurKey[i] ^ 0x5C;
	MD5Hash( &InPad[0], (UINT32)InPad.size(), &OutPad[__B] );

	MD5Hash( &OutPad[0], (UINT32)OutPad.size(), pMessageDigest );
}

/**
 * ��SHA1�㷨�������ݵ�20���ֽ�ɢ��ֵ��
 *
 * @param pData Ҫ��������ݡ�
 * @param nDataSize Ҫ����������ֽ�����
 * @param[out] pMessageDigest �洢ɢ��ֵ�Ļ���������������С��20�ֽڡ�
 */
void SHA1Hash( const void *pData, size_t nDataSize, BYTE *pMessageDigest )
{
#if defined( _WIN32 ) // ����ϵͳ��Windows
	HCRYPTPROV hCryptProv = 0;
	HCRYPTHASH hHash = 0;

	try
	{
		if( !CryptAcquireContext(&hCryptProv,NULL,NULL,PROV_RSA_FULL,0) ) // ��CSPʧ��
		{
			if( GetLastError() != NTE_BAD_KEYSET || !CryptAcquireContext(&hCryptProv,NULL,NULL,PROV_RSA_FULL,CRYPT_NEWKEYSET) ) throw COsError();
		}
		if( !CryptCreateHash(hCryptProv,CALG_SHA1,0,0,&hHash) ) throw COsError(); // ������ϣ��ʧ��
		if( !CryptHashData(hHash,(BYTE *)pData,nDataSize,0) ) throw COsError(); // ��������ʧ��

		DWORD nSize = 20;
		if( !CryptGetHashParam(hHash,HP_HASHVAL,pMessageDigest,&nSize,0) ) throw COsError(); // ��ü�����ʧ��		
	}
	catch( ... )
	{
		if( hHash != 0 ) CryptDestroyHash( hHash );
		if( hCryptProv != 0 ) CryptReleaseContext( hCryptProv, 0 );
		throw;
	}

	if( hHash != 0 ) CryptDestroyHash( hHash );
	if( hCryptProv != 0 ) CryptReleaseContext( hCryptProv, 0 );
#elif defined( HEADER_SHA_H ) // ʹ��OpenSSL�ļ���API
	SHA_CTX Context;
	SHA1_Init( &Context );
	SHA1_Update( &Context, pData, nDataSize );
	SHA1_Final( pMessageDigest, &Context );
#else
#error SHA1 not implement!
#endif
}

/**
 * ��HMAC SHA1�㷨�������ݵ�20���ֽ�ɢ��ֵ��
 *
 * @param pKey ����ʹ�õĹؼ��֡�
 * @param nKeySize ����ʹ�õĹؼ����ֽ�����
 * @param pData Ҫ��������ݡ�
 * @param nDataSize Ҫ����������ֽ�����
 * @param[out] pMessageDigest �洢ɢ��ֵ�Ļ���������������С��20�ֽڡ�
 */
void HMACSHA1( const void *pKey, size_t nKeySize, const void *pData, size_t nDataSize, BYTE *pMessageDigest )
{
	static const size_t __B = 64;

	BYTE byKey1[__B], byKey2[__B];
	if( nKeySize <= __B ) 
	{
		memcpy( byKey1, pKey, nKeySize );
	}
	else 
	{
		SHA1Hash( pKey, nKeySize, byKey1 );
		nKeySize = 20;
	}

	if( nKeySize < __B ) memset( &byKey1[nKeySize], 0, __B - nKeySize );

	memcpy( byKey2, byKey1, __B );

    for( UINT32 i = 0; i < __B; i++ )
	{
		byKey1[i] ^= 0x36;
		byKey2[i] ^= 0x5c;
    }

	vector<BYTE> TmpBuf;
	BYTE byTmpHash[20];

	TmpBuf.insert( TmpBuf.end(), byKey1, byKey1 + __B );
	TmpBuf.insert( TmpBuf.end(), (BYTE *)pData, (BYTE *)pData + nDataSize );
	SHA1Hash( &TmpBuf[0], TmpBuf.size(), byTmpHash );

	TmpBuf.clear();
	TmpBuf.insert( TmpBuf.end(), byKey2, byKey2 + __B );
	TmpBuf.insert( TmpBuf.end(), byTmpHash, byTmpHash + 20 );
	SHA1Hash( &TmpBuf[0], TmpBuf.size(), pMessageDigest );
}

/**
 * Base64���롣
 *
 * @param pData Ҫ������������ݡ�
 * @param nDataSize Ҫ����������ֽ�����
 * @param[out] pBuffer ��ű������ݵĻ������������ַ���ĩβ�Ŀ���ֹ������������С�� 4.0 * ceil(nDataSize / 3.0)��
 * @param[out] nBufferSize ��������������
 * @return ���ִ�гɹ���Ϊtrue�����������������Ϊfalse��
 */
bool Base64Encode( const void *pData, size_t nDataSize, char *pBuffer, size_t nBufferSize )
{
	size_t nOutSize = (size_t)( 4.0 * ceil(nDataSize / 3.0) );
	if( nBufferSize < nOutSize ) return false;

    for( UINT32 i = 0, j = 0; i < nDataSize; )
	{
        UINT32 a = i < nDataSize ? ((BYTE *)pData)[i++] : 0;
        UINT32 b = i < nDataSize ? ((BYTE *)pData)[i++] : 0;
        UINT32 c = i < nDataSize ? ((BYTE *)pData)[i++] : 0;
        UINT32 t = ( a << 0x10 ) + ( b << 0x08 ) + c;

        pBuffer[j++] = g_cBase64EncodingTable[(t >> 3 * 6) & 0x3F];
        pBuffer[j++] = g_cBase64EncodingTable[(t >> 2 * 6) & 0x3F];
        pBuffer[j++] = g_cBase64EncodingTable[(t >> 1 * 6) & 0x3F];
        pBuffer[j++] = g_cBase64EncodingTable[(t >> 0 * 6) & 0x3F];
    }

    for( int i = 0; i < g_iBase64ModTable[nDataSize % 3]; i++ ) pBuffer[nOutSize-1-i] = '=';
	if( nBufferSize > nOutSize ) pBuffer[nOutSize] = 0;

	return true;
}

/**
 * Base64���롣
 *
 * @param pData Ҫ������������ݡ�
 * @param nDataSize Ҫ����������ֽ�����
 * @return ���ִ�гɹ���Ϊ������������Ϊ�����ݡ�
 */
string Base64Encode( const void *pData, size_t nDataSize )
{
	string Result( (size_t)(4.0 * ceil(nDataSize / 3.0)), 0 );
	if( !Base64Encode(pData, nDataSize, &Result[0], Result.size()) ) return string();
	return Result;
}

/**
 * Base64���롣
 *
 * @param pData Ҫ������������ݡ�
 * @param nDataSize Ҫ����������ֽ�����
 * @param[out] pBuffer ����ѽ������ݵĻ���������������С�� nDataSize / 4 * 3��
 * @param[in,out] nBufferSize �ں���ִ��ʱ��ʾ���������������ں�������ʱ���ڴ���ѽ������ݵ��ֽ�����
 * @return ���ִ�гɹ���Ϊtrue�����Ҫ�����������Ч���߻�����������Ϊfalse��
 */
bool Base64Decode( const char *pData, size_t nDataSize, void *pBuffer, size_t &nBufferSize )
{
    if( nDataSize % 4 != 0 ) return false;

    size_t nOutSize = nDataSize / 4 * 3;
    if( pData[nDataSize - 1] == '=' ) nOutSize--;
    if( pData[nDataSize - 2] == '=' ) nOutSize--;

	if( nBufferSize < nOutSize ) return false;

    for( UINT32 i = 0, j = 0; i < nDataSize; )
	{
        UINT32 a = ( pData[i] == '=' ) ? ( 0 & i++ ) : g_cBase64DecodingTable[(int)pData[i++]];
        UINT32 b = ( pData[i] == '=' ) ? ( 0 & i++ ) : g_cBase64DecodingTable[(int)pData[i++]];
        UINT32 c = ( pData[i] == '=' ) ? ( 0 & i++ ) : g_cBase64DecodingTable[(int)pData[i++]];
        UINT32 d = ( pData[i] == '=' ) ? ( 0 & i++ ) : g_cBase64DecodingTable[(int)pData[i++]];
        UINT32 t = (a << 3 * 6) + (b << 2 * 6) + (c << 1 * 6) + (d << 0 * 6);

        if( j < nOutSize ) ((BYTE *)pBuffer)[j++] = (t >> 2 * 8) & 0xFF;
        if( j < nOutSize ) ((BYTE *)pBuffer)[j++] = (t >> 1 * 8) & 0xFF;
        if( j < nOutSize ) ((BYTE *)pBuffer)[j++] = (t >> 0 * 8) & 0xFF;
    }

	nBufferSize = nOutSize;

    return true;
}

#ifndef _LHQ_NO_DES_FUNC
/**
 * ��DES�㷨���м��ܻ���ܡ�
 *
 * @param bDoEncrypt ���Ҫ���м�����Ϊtrue�����Ҫ���н�����Ϊfalse��
 * @param pIn ��bDoEncryptΪtrueʱ���������ΪҪ���ܵ����ݡ�
 *                ��bDoEncryptΪfalseʱ���������ΪҪ���ܵ����ݡ�
 * @parma[in] nInSize pIn�������ֽ�����
 * @param[out] pOut ��bDoEncryptΪtrueʱ������������ڴ�ż��ܵĽ����
 *                  ��bDoEncryptΪfalseʱ������������ڴ�Ž��ܵĽ����
 * @param nMaxOut pOut�ܹ���ŵ���������ֽ���������Ϊ8�ı������Ҳ���С��nInSize��
 * @param pKey �����õ�8�ֽڹؼ��֡�
 * 
 * @note DES�㷨�����������ֽ�������Ϊ8�ı�����<br>
 *       �ڼ���ʱ�����pIn�ṩ�ļ��������ֽ�������8�ı������������Զ��ں��油��ŵ��0ֵ����8�ֽ�Ȼ����м��ܡ�<br>
 *       �ڽ���ʱ�����pIn�ṩ�Ľ��������ֽ�������8�ı��������ᵼ�º���ִ��ʧ�ܡ�
 */
void DesEncrypt( bool bDoEncrypt, const BYTE *pIn, UINT32 nInSize, BYTE *pOut, UINT32 nMaxOut, const BYTE *pKey )
{
	if( pIn == NULL || pOut == NULL || pKey == NULL ) throw invalid_argument( "[DesEncrypt] The parameter is null!" );

	if( !bDoEncrypt && nInSize % 8 > 0 ) throw invalid_argument( "[DesEncrypt] The nInSize is invalid!" ); // ���ܵ������ֽ�������8�ı���

	if( nMaxOut < nInSize || nMaxOut % 8 > 0 ) throw invalid_argument( "[DesEncrypt] The nInSize is nMaxOut!" );

	if( nInSize < 1 ) return;

#if defined( _WIN32 ) // ����ϵͳ��Windows
	HCRYPTPROV hCryptProv = 0;
	HCRYPTKEY hDesKey = 0;

	struct
    {
        BLOBHEADER Header;
        DWORD nKeySize;
        BYTE byKeyData[8];
    } KeyBlob; // ���ܹؼ�����Ϣ

	try
	{
		if( nMaxOut < nInSize ) return;

		KeyBlob.Header.bType = PLAINTEXTKEYBLOB;
		KeyBlob.Header.bVersion = CUR_BLOB_VERSION;
		KeyBlob.Header.reserved = 0;
		KeyBlob.Header.aiKeyAlg = CALG_DES;
		KeyBlob.nKeySize = 8;
		memcpy( KeyBlob.byKeyData, pKey, 8 );
			
		if( !CryptAcquireContext(&hCryptProv,NULL,NULL,PROV_RSA_FULL,0) ) throw COsError(); // ����CSP���		
		
		if( !CryptImportKey(hCryptProv,(BYTE *)&KeyBlob,sizeof(KeyBlob),0,0,&hDesKey) ) throw COsError(); // ���ùؼ���
		DWORD dwMode = CRYPT_MODE_ECB;
		CryptSetKeyParam( hDesKey, KP_MODE, (BYTE *)&dwMode, 0 ); // ����DES���ܵķ�ʽ

		if( bDoEncrypt ) // ����
		{
			vector<BYTE> TempBuf( nMaxOut + 16 ); // �������ʱ����������ΪCryptEncrypt������Ҫ�������8���ֽڵĶ������ݣ�
			memcpy( &TempBuf[0], pIn, nInSize );
			if( nInSize % 8 > 0 ) // ���ݴ�С����8�ı���
			{
				if( nMaxOut - nInSize < 8 - nInSize % 8 ) throw COsError( ERROR_INVALID_PARAMETER );

				memset( &TempBuf[nInSize], 0, 8 - nInSize % 8 ); // �ں������0
				nInSize = nInSize + ( 8 - nInSize % 8 );
			}
			DWORD dwDataLen = nInSize;
			if( !CryptEncrypt(hDesKey,0,TRUE,0,&TempBuf[0],&dwDataLen,TempBuf.size()) ) throw COsError();

			memcpy( pOut, &TempBuf[0], min(nMaxOut,(UINT32)dwDataLen) );
		}
		else // ����
		{
			if( nInSize % 8 > 0 ) throw COsError( ERROR_INVALID_PARAMETER );
			memcpy( pOut, pIn, nInSize );
			DWORD dwDataLen = nInSize;
			if( !CryptDecrypt(hDesKey,0,TRUE,0,pOut,&dwDataLen) ) throw COsError();
		}
	}
	catch( ... )
	{
	}

	if( hDesKey != 0 ) CryptDestroyKey( hDesKey );
	if( hCryptProv != 0 ) CryptReleaseContext( hCryptProv, 0 );
#elif defined( HEADER_DES_H ) // ʹ��OpenSSL�ļ���API
	if( !bDoEncrypt && nInSize % 8 > 0 ) return;
	
	DES_key_schedule Schedule;
	DES_set_key_unchecked( (DES_cblock *)pKey, &Schedule ); // ���ùؼ���
	
	UINT32 i;
	for( i = 0; i < nInSize; i += 8 )
	{
		if( bDoEncrypt ) // ����
		{
			if( nInSize - i < 8 ) // ʣ�»�û�м��ܵ����ݴ�С����8�ֽ�
			{
				BYTE byBuffer[8]; // ��ʱ������
				memcpy( byBuffer, &pIn[i], nInSize - i );
				memset( &byBuffer[nInSize-i], 0, 8 - (nInSize-i) ); // �ں������0
				DES_ecb_encrypt( (DES_cblock *)byBuffer, (DES_cblock *)&pOut[i], &Schedule, DES_ENCRYPT );
			}
			else
			{
				DES_ecb_encrypt( (DES_cblock *)&pIn[i], (DES_cblock *)&pOut[i], &Schedule, DES_ENCRYPT );
			}
		}
		else // ����
		{
			DES_ecb_encrypt( (DES_cblock *)&pIn[i], (DES_cblock *)&pOut[i], &Schedule, DES_DECRYPT );
		}
	}
#else
#error DES not implement!
#endif
}
#endif

/**
 * �жϵ�ǰϵͳ�ַ����Ƿ�ΪUTF-8��
 */
bool WithUtf8()
{
	return g_bWithUtf8;
}

/**
 * ���ϵͳĬ�ϵ��ַ����������ơ�
 *
 * @param[out] pBuffer ����ַ����������ƵĻ�������
 * @param nMaxBuffer ����ַ����������ƵĻ��������ɴ洢�ַ����������ַ���ĩβ�Ŀ���ֹ����
 * @return ���ִ�гɹ���Ϊtrue������Ϊfalse��
 */
bool GetSystemCharset( char *pBuffer, int nMaxBuffer )
{
	bool bResult;
#if _MSC_VER >= 1400 // ��������Microsoft C/C++ 14.0����߰汾
	bResult = sprintf_s( pBuffer, nMaxBuffer, "CP%u", GetACP() ) > 0;
#elif defined( _WIN32 ) // ����ϵͳ��Windows
	bResult = snprintf( pBuffer, nMaxBuffer, "CP%u", GetACP() ) > 0;
#else
	char *pDefaultCharset = nl_langinfo( CODESET );
	if( pDefaultCharset == NULL || pDefaultCharset[0] == 0 ) 
	{
		bResult = false;
	}
	else
	{
		strncpy( pBuffer, pDefaultCharset, nMaxBuffer );
		pBuffer[nMaxBuffer-1] = 0;
		bResult = true;
	}
	/*
	const char *pLocale = NULL;
	// ͨ������������ñ��ػ���Ϣ
	pLocale = getenv( "LC_ALL" );
	if( pLocale == NULL || pLocale[0] == 0 )
	{
		pLocale = getenv( "LC_CTYPE" );
		if( pLocale == NULL || pLocale[0] == 0 ) 
		{
			pLocale = getenv( "LANG" );			
		}
	}

	if( pLocale == NULL || pLocale[0] == 0 ) // �޷�ͨ������������ñ��ػ���Ϣ
	{
		errno = EACCES;
		bResult = false;
	}
	else
	{	
		const char *pPoint = strrchr( pLocale, '.' ); // ��鱾�ػ���Ϣ�е��ַ����Ƿ�"."�ָ�
		if( pPoint != NULL ) pLocale = pPoint + 1;

		strncpy( pBuffer, pLocale, nMaxBuffer );
		pBuffer[nMaxBuffer-1] = 0;
		bResult = true;
	}
	*/
#endif

	return bResult;
}

#ifdef _WIN32
#ifdef __CYGWIN__
EXTERN_C const GUID IID_IMultiLanguage = { 0x275c23e1, 0x3747, 0x11d0, { 0x9f, 0xea, 0x00, 0xaa, 0x00, 0x3f, 0x86, 0x46 } };
#endif

/**
 * ���ָ���ַ��������Ӧ�Ĵ���ҳ��
 */
static UINT32 CharsetToCodePage( const char *pCharset )
{
	if( pCharset[0] == 0 ) return CP_ACP;

	static IMultiLanguage *MultiLanguage = NULL;
	if( MultiLanguage == NULL )
	{
		static volatile bool bInitError = false;
		static volatile LONG iInit = 0;

		if( bInitError ) return 0xFFFFFFFF;

		if( InterlockedCompareExchange(&iInit,1,0) == 0 )
		{
			HRESULT hResult = CoCreateInstance( CLSID_CMultiLanguage, NULL, CLSCTX_INPROC_SERVER, IID_IMultiLanguage, (void **)&MultiLanguage );
			if( hResult != S_OK )
			{
				bInitError = true;
				return 0xFFFFFFFF;
			}
		}
		else
		{
			while( MultiLanguage  == NULL )
			{
				if( bInitError ) return 0xFFFFFFFF;
				Sleep( 10 );
			}
		}
	}

	wstring Charset;
	Charset.reserve( 128 );
	for( int i = 0; pCharset[i] != 0; i++ ) Charset.append( 1, pCharset[i] );

	MIMECSETINFO SrcCsInfo;
	HRESULT hResult = MultiLanguage->GetCharsetInfo( (BSTR)Charset.c_str(), &SrcCsInfo );
	if( hResult != S_OK ) 
	{
		if( Charset.compare(L"utf-16le") == 0 || Charset.compare(L"utf-16") == 0 ) return 1200;
		return 0xFFFFFFFF;
	}

	return SrcCsInfo.uiInternetEncoding;
}
#endif

/**
 * ��һ���ַ��������ַ�������ת����
 *
 * @param pSrcCharset Դ�ַ������ַ������룬���ΪNULL���߿��������ʾϵͳĬ��ֵ��
 * @param pSrc Ҫת�����ַ�����
 * @param nSrcSize Ҫת�����ֽ�����
 * @param pDestCharset Ҫת����Ŀ���ַ������룬���ΪNULL����������ʾϵͳĬ��ֵ��
 * @param[out] pBuffer ���ת������Ļ�������
 * @param nMaxBuffer ���ת������Ļ��������ɴ洢�ֽ�����
 * @return ���ִ�гɹ���Ϊת��������ֽ�����������ִ�����߻�����̫С��Ϊ-1��
 */
int StrIconv( const char *pSrcCharset, const char *pSrc, int nSrcSize, const char *pDestCharset, char *pBuffer, int nMaxBuffer )
{
	if( pSrc == NULL || nSrcSize < 0 ) return -1;
	if( pBuffer == NULL || nMaxBuffer < 0 ) return -1;

	if( nSrcSize == 0 ) return 0;
	if( nMaxBuffer == 0 ) return -1;

	if( pSrcCharset == NULL ) pSrcCharset = "";
	if( pDestCharset == NULL ) pDestCharset = "";

#ifdef _WIN32
	UINT32 uiSrcCodePage = CharsetToCodePage( pSrcCharset );
	if( uiSrcCodePage == 0xFFFFFFFF ) return -1;

	UINT32 uiDestCodePage = CharsetToCodePage( pDestCharset );
	if( uiDestCodePage == 0xFFFFFFFF ) return -1;
	
	if( uiSrcCodePage == uiDestCodePage ) // ����Ҫת��
	{
		if( nSrcSize > nMaxBuffer ) return -1;
		memcpy( pBuffer, pSrc, nSrcSize );
		return nSrcSize;
	}
	else if( uiSrcCodePage == 1200 ) // �ӿ��ַ�ת��
	{
		nSrcSize /= sizeof( WCHAR );
		int nDestLen = WideCharToMultiByte( uiDestCodePage, 0, (WCHAR *)pSrc, nSrcSize, pBuffer, nMaxBuffer, NULL, NULL );
		return nDestLen == 0 ? -1 : nDestLen;
	}
	else if( uiDestCodePage == 1200 ) // ת�������ַ�
	{
		nMaxBuffer /= sizeof( WCHAR );
		int nDestLen = MultiByteToWideChar( uiSrcCodePage, 0, pSrc, nSrcSize, (WCHAR *)pBuffer, nMaxBuffer );
		return nDestLen == 0 ? -1 : sizeof( WCHAR ) * nDestLen;
	}
	else
	{
		vector<WCHAR> UnicodeSrc( 512 );
		int nUnicodeLen = MultiByteToWideChar( uiSrcCodePage, 0, pSrc, nSrcSize, &UnicodeSrc[0], UnicodeSrc.size() ); // ��ת�������ַ�
		if( nUnicodeLen == 0 )
		{
			if( GetLastError() != ERROR_INSUFFICIENT_BUFFER ) return -1; // �����ԭ�򲢷ǻ���������

			nUnicodeLen = MultiByteToWideChar( uiSrcCodePage, 0, pSrc, nSrcSize, NULL, 0 ); // ���ת�������ַ�����ַ���
			if( nUnicodeLen == 0 ) return -1;

			UnicodeSrc.resize( nUnicodeLen );
			nUnicodeLen = MultiByteToWideChar( uiSrcCodePage, 0, pSrc, nSrcSize, &UnicodeSrc[0], UnicodeSrc.size() ); // ת�������ַ�
			if( nUnicodeLen == 0 ) return -1;
		}

		int nDestLen = WideCharToMultiByte( uiDestCodePage, 0, &UnicodeSrc[0], nUnicodeLen, pBuffer, nMaxBuffer, NULL, NULL ); // �ӿ��ַ�ת����Ŀ���ַ�������
		return nDestLen == 0 ? -1 : nDestLen;
	}
#else
	iconv_t iCd = iconv_open( pDestCharset, pSrcCharset );
	if( iCd == (iconv_t)-1 ) return -1;

	size_t nIn = nSrcSize;
	size_t nOut = nMaxBuffer;
	char *pDest = pBuffer;
	if( iconv(iCd, (char **)&pSrc, &nIn, (char **)&pDest, &nOut) == (size_t)-1 )
	{
		iconv_close( iCd );
		return -1;
	}

	iconv_close( iCd );

	return nMaxBuffer - nOut; 
#endif
}

#if __SIZEOF_WCHAR_T__ == 2 // �ڵ�ǰƽ̨��wchar_t���͵��ֽ�����2
#define WIDE_CHARSET "utf-16le"
#else
#define WIDE_CHARSET "utf-32le"
#endif

/**
 * ����char�����ַ���ɵ��ַ���ת��Ϊ��char�����ַ���ɲ��Ұ�UTF-8������ַ�����
 *
 * @param pSrc Ҫת�����ַ�����
 * @param nSrcLen Ҫת�����ַ��������Ϊ-1���ʾת��ֱ������ֹ��Ϊֹ��
 * @param[out] pBuffer ���ת������Ļ�������
 * @param nMaxBuffer ���ת������Ļ��������ɴ洢�ַ�����
 * @param pSrcCharset �ַ������ַ������룬���ΪNULL���߿��������ʾϵͳĬ��ֵ��
 * @return ���ִ�гɹ���Ϊת��������ַ�����������ִ�����߻�����̫С��Ϊ-1��
 */
int AnsiStrToUtf8Str( const char *pSrc, int nSrcLen, char *pBuffer, int nMaxBuffer, const char *pSrcCharset /* = NULL */ )
{
	if( pSrc == NULL ) return -1;
	if( nSrcLen == 0 ) return 0;
	if( pBuffer == NULL || nMaxBuffer < 0 ) return -1;

	if( nSrcLen == -1 ) nSrcLen = strlen( pSrc ) + 1;

	if( (g_bWithUtf8 && (pSrcCharset == NULL || pSrcCharset[0] == 0))
		|| (pSrcCharset != NULL && stricmp(pSrcCharset, "utf-8") == 0) ) // Դ�ַ�����Ŀ���ַ�����ΪUTF-8
	{
		if( nSrcLen > nMaxBuffer ) return -1;
		memcpy( pBuffer, pSrc, nSrcLen );
		return nSrcLen;
	}

#ifdef _WIN32
	if( pSrcCharset != NULL && pSrcCharset[0] != 0 ) return StrIconv( pSrcCharset, pSrc, nSrcLen, "utf-8", pBuffer, nMaxBuffer );

	vector<WCHAR> UnicodeSrc( nSrcLen );
	int nUnicodeLen = MultiByteToWideChar( CP_ACP, 0, pSrc, nSrcLen, &UnicodeSrc[0], UnicodeSrc.size() ); // ��ת�������ַ�
	if( nUnicodeLen == 0 ) return -1;

	int nDestLen = WideCharToMultiByte( CP_UTF8, 0, &UnicodeSrc[0], nUnicodeLen, pBuffer, nMaxBuffer, NULL, NULL ); // �ӿ��ַ�ת����Ŀ���ַ�������
	return nDestLen == 0 ? -1 : nDestLen;
#else
	return StrIconv( pSrcCharset, pSrc, nSrcLen, "utf-8", pBuffer, nMaxBuffer );
#endif
}

/**
 * ����char�����ַ���ɵ��ַ���ת��Ϊ��UTF-8�����string����
 *
 * @param pSrc Ҫת�����ַ�����
 * @param nSrcLen Ҫת�����ַ��������Ϊ-1���ʾת��ֱ������ֹ��Ϊֹ��
 * @param pSrcCharset �ַ������ַ������룬���ΪNULL���߿��������ʾϵͳĬ��ֵ��
 * @return ���ִ�гɹ���Ϊת���Ľ�������û�����ݿ�ת������ת��ʧ����Ϊ���ַ�����
 */
string AnsiStrToUtf8Str( const char *pSrc, int nSrcLen /* = -1 */, const char *pSrcCharset /* = NULL */ )
{
	if( nSrcLen == 0 ) return string();

	if( (g_bWithUtf8 && (pSrcCharset == NULL || pSrcCharset[0] == 0))
		|| (pSrcCharset != NULL && stricmp(pSrcCharset, "utf-8") == 0) ) // Դ�ַ�����Ŀ���ַ�����ΪUTF-8
	{
		if( nSrcLen < 0 ) return string( pSrc );
		else return string( pSrc, nSrcLen );
	}
	
	bool bInlcudeTerminator = nSrcLen == -1;
	string Buffer( 1024, 0 );
	for( int i = 0; i < 10; i++ )
	{
		int nDestLen = AnsiStrToUtf8Str( pSrc, nSrcLen, &Buffer[0], Buffer.size(), pSrcCharset );
		if( nDestLen >= 0 )
		{
			Buffer.resize( nDestLen );
			if( bInlcudeTerminator && !Buffer.empty() ) Buffer.resize( Buffer.size() - 1 ); // ȥ������ֹ��
			return Buffer;
		}

		Buffer.resize( Buffer.size() * 2 ); // ������̫С��Ҫ���·���Ȼ���ٴν��и�ʽ��
	}

	return string();
}

/**
 * ����wchar_t�����ַ���ɵ��ַ���ת��Ϊ��char�����ַ���ɲ��Ұ�UTF-8������ַ�����
 *
 * @param pSrc Ҫת�����ַ�����
 * @param nSrcLen Ҫת�����ַ��������Ϊ-1���ʾת��ֱ������ֹ��Ϊֹ��
 * @param[out] pBuffer ���ת������Ļ�������
 * @param nMaxBuffer ���ת������Ļ��������ɴ洢�ַ�����
 * @return ���ִ�гɹ���Ϊת��������ַ�����������ִ�����߻�����̫С��Ϊ-1��
 */
int WideStrToUtf8Str( const wchar_t *pSrc, int nSrcLen, char *pBuffer, int nMaxBuffer )
{
	if( pSrc == NULL ) return -1;
	if( nSrcLen == 0 ) return 0;
	if( pBuffer == NULL || nMaxBuffer < 0 ) return -1;

#ifdef _WIN32
	int nDestLen = WideCharToMultiByte( CP_UTF8, 0, pSrc, nSrcLen, pBuffer, nMaxBuffer, NULL, NULL );
	return nDestLen == 0 ? -1 : nDestLen;
#else
	if( nSrcLen == -1 ) nSrcLen = wcslen( pSrc ) + 1;
	return StrIconv( WIDE_CHARSET, (char *)pSrc, sizeof(wchar_t) * nSrcLen, "utf-8", pBuffer, nMaxBuffer );
#endif
}

/**
 * ����wchar_t�����ַ���ɵ��ַ���ת��Ϊ��UTF-8�����string����
 *
 * @param pSrc Ҫת�����ַ�����
 * @param nSrcLen Ҫת�����ַ��������Ϊ-1���ʾת��ֱ������ֹ��Ϊֹ��
 * @return ���ִ�гɹ���Ϊת���Ľ�������û�����ݿ�ת������ת��ʧ����Ϊ���ַ�����
 */
string WideStrToUtf8Str( const wchar_t *pSrc, int nSrcLen /* = -1 */ )
{
	if( nSrcLen == 0 ) return string();

	bool bInlcudeTerminator = nSrcLen == -1;
	string Buffer( 1024, 0 );
	for( int i = 0; i < 10; i++ )
	{
		int nDestLen = WideStrToUtf8Str( pSrc, nSrcLen, &Buffer[0], Buffer.size() );
		if( nDestLen >= 0 )
		{
			Buffer.resize( nDestLen );
			if( bInlcudeTerminator && !Buffer.empty() ) Buffer.resize( Buffer.size() - 1 ); // ȥ������ֹ��
			return Buffer;
		}

		Buffer.resize( Buffer.size() * 2 ); // ������̫С��Ҫ���·���Ȼ���ٴν��и�ʽ��
	}

	return string();
}

/**
 * ����char�����ַ���ɲ��Ұ�UTF-8������ַ���ת��Ϊ��char�����ַ���ɵ��ַ�����
 *
 * @param pSrc Ҫת�����ַ�����
 * @param nSrcLen Ҫת�����ַ��������Ϊ-1���ʾת��ֱ������ֹ��Ϊֹ��
 * @param[out] pBuffer ���ת������Ļ�������
 * @param nMaxBuffer ���ת������Ļ��������ɴ洢�ַ�����
 * @param pDestCharset Ŀ���ַ������ַ������룬���ΪNULL���߿��������ʾϵͳĬ��ֵ��
 * @return ���ִ�гɹ���Ϊת��������ַ�����������ִ�����߻�����̫С��Ϊ-1��
 */
int Utf8StrToAnsiStr( const char *pSrc, int nSrcLen, char *pBuffer, int nMaxBuffer, const char *pDestCharset /* = NULL */ )
{
	if( pSrc == NULL ) return -1;
	if( nSrcLen == 0 ) return 0;
	if( pBuffer == NULL || nMaxBuffer < 0 ) return -1;

	if( nSrcLen == -1 ) nSrcLen = strlen( pSrc ) + 1;

	if( (g_bWithUtf8 && (pDestCharset == NULL || pDestCharset[0] == 0))
		|| (pDestCharset != NULL && stricmp(pDestCharset, "utf-8") == 0) ) // Դ�ַ�����Ŀ���ַ�����ΪUTF-8
	{
		if( nSrcLen > nMaxBuffer ) return -1;
		memcpy( pBuffer, pSrc, nSrcLen );
		return nSrcLen;
	}

#ifdef _WIN32
	vector<WCHAR> UnicodeSrc( nSrcLen );
	int nUnicodeLen = MultiByteToWideChar( CP_UTF8, 0, pSrc, nSrcLen, &UnicodeSrc[0], UnicodeSrc.size() ); // ��ת�������ַ�
	if( nUnicodeLen == 0 ) return -1;
	
	int nDestLen = WideCharToMultiByte( CP_ACP, 0, &UnicodeSrc[0], nUnicodeLen, pBuffer, nMaxBuffer, NULL, NULL ); // �ӿ��ַ�ת����Ŀ���ַ�������
	return nDestLen == 0 ? -1 : nDestLen;
#else
	return StrIconv( "utf-8", pSrc, nSrcLen, "", pBuffer, nMaxBuffer );
#endif
}

/**
 * ����char�����ַ���ɲ��Ұ�UTF-8������ַ���ת��Ϊstring����
 *
 * @param pSrc Ҫת�����ַ�����
 * @param nSrcLen Ҫת�����ַ��������Ϊ-1���ʾת��ֱ������ֹ��Ϊֹ��
 * @param pDestCharset Ŀ���ַ������ַ������룬���ΪNULL���߿��������ʾϵͳĬ��ֵ��
 * @return ���ִ�гɹ���Ϊת���Ľ�������û�����ݿ�ת������ת��ʧ����Ϊ���ַ�����
 */
string Utf8StrToAnsiStr( const char *pSrc, int nSrcLen /* = -1 */, const char *pDestCharset /* = NULL */ )
{
	if( nSrcLen == 0 ) return string();
	
	if( (g_bWithUtf8 && (pDestCharset == NULL || pDestCharset[0] == 0))
		|| (pDestCharset != NULL && stricmp(pDestCharset, "utf-8") == 0) ) // Դ�ַ�����Ŀ���ַ�����ΪUTF-8
	{
		if( nSrcLen < 0 ) return string( pSrc );
		else return string( pSrc, nSrcLen );
	}
	
	bool bInlcudeTerminator = nSrcLen == -1;
	string Buffer( 1024, 0 );
	for( int i = 0; i < 10; i++ )
	{
		int nDestLen = Utf8StrToAnsiStr( pSrc, nSrcLen, &Buffer[0], Buffer.size() );
		if( nDestLen >= 0 )
		{
			Buffer.resize( nDestLen );
			if( bInlcudeTerminator && !Buffer.empty() ) Buffer.resize( Buffer.size() - 1 ); // ȥ������ֹ��
			return Buffer;
		}

		Buffer.resize( Buffer.size() * 2 ); // ������̫С��Ҫ���·���Ȼ���ٴν��и�ʽ��
	}

	return string();
}

/**
 * ����char�����ַ���ɲ��Ұ�UTF-8������ַ���ת��Ϊ��wchar_t�����ַ���ɵ��ַ�����
 *
 * @param pSrc Ҫת�����ַ�����
 * @param nSrcLen Ҫת�����ַ��������Ϊ-1���ʾת��ֱ������ֹ��Ϊֹ��
 * @param[out] pBuffer ���ת������Ļ�������
 * @param nMaxBuffer ���ת������Ļ��������ɴ洢�ַ�����
 * @return ���ִ�гɹ���Ϊת��������ַ�����������ִ�����߻�����̫С��Ϊ-1��
 */
int Utf8StrToWideStr( const char *pSrc, int nSrcLen, wchar_t *pBuffer, int nMaxBuffer )
{
	if( pSrc == NULL ) return -1;
	if( nSrcLen == 0 ) return 0;
	if( pBuffer == NULL || nMaxBuffer < 0 ) return -1;

#ifdef _WIN32
	int nDestLen = MultiByteToWideChar( CP_UTF8, 0, pSrc, nSrcLen, &pBuffer[0], nMaxBuffer );
	return nDestLen == 0 ? -1 : nDestLen;
#else
	if( nSrcLen == -1 ) nSrcLen = strlen( pSrc ) + 1;
	int nDestSize = StrIconv( "utf-8", pSrc, nSrcLen, WIDE_CHARSET, (char *)pBuffer, sizeof(wchar_t) * nMaxBuffer );
	return nDestSize == -1 ? -1 : nDestSize / sizeof( wchar_t );
#endif
}

/**
 * ����char�����ַ���ɲ��Ұ�UTF-8������ַ���ת��Ϊwstring����
 *
 * @param pSrc Ҫת�����ַ�����
 * @param nSrcLen Ҫת�����ַ��������Ϊ-1���ʾת��ֱ������ֹ��Ϊֹ��
 * @return ���ִ�гɹ���Ϊת���Ľ�������û�����ݿ�ת������ת��ʧ����Ϊ���ַ�����
 */
wstring Utf8StrToWideStr( const char *pSrc, int nSrcLen /* = -1 */ )
{
	if( nSrcLen == 0 ) return wstring();

	bool bInlcudeTerminator = nSrcLen == -1;
	wstring Buffer( 1024, 0 );
	for( int i = 0; i < 10; i++ )
	{
		int nDestLen = Utf8StrToWideStr( pSrc, nSrcLen, &Buffer[0], Buffer.size() );
		if( nDestLen >= 0 )
		{
			Buffer.resize( nDestLen );
			if( bInlcudeTerminator && !Buffer.empty() ) Buffer.resize( Buffer.size() - 1 ); // ȥ������ֹ��
			return Buffer;
		}

		Buffer.resize( Buffer.size() * 2 ); // ������̫С��Ҫ���·���Ȼ���ٴν��и�ʽ��
	}

	return wstring();
}

/**
 * ����char�����ַ���ɵ��ַ���ת��Ϊwstring����
 *
 * @param pSrc Ҫת�����ַ�����
 * @param nSrcLen Ҫת�����ַ��������Ϊ-1���ʾת��ֱ������ֹ��Ϊֹ��
 * @param[out] pBuffer ���ת������Ļ����������ΪNULL���ʾֻ���ת��������ֽ�������������ִ��ת����
 * @param nMaxBuffer ���ת������Ļ��������ɴ洢�ֽ�����
 * @return ���ִ�гɹ���Ϊת��������ַ�����������ִ�����߻�����̫С��Ϊ-1��
 */
int AnsiStrToWideStr( const char *pSrc, int nSrcLen, wchar_t *pBuffer, int nMaxBuffer )
{
	if( pSrc == NULL ) return -1;
	if( nSrcLen == 0 ) return 0;
	if( pBuffer == NULL || nMaxBuffer < 0 ) return -1;

#ifdef _WIN32
	int nDestLen = MultiByteToWideChar( CP_ACP, 0, pSrc, nSrcLen, pBuffer, nMaxBuffer );
	return nDestLen == 0 ? -1 : nDestLen;
#else
	if( nSrcLen == -1 ) nSrcLen = strlen( pSrc ) + 1;
	int nDestSize = StrIconv( "", pSrc, nSrcLen, WIDE_CHARSET, (char *)pBuffer, sizeof(wchar_t) * nMaxBuffer );
	return nDestSize == -1 ? -1 : nDestSize / sizeof( wchar_t );
#endif
}

/**
 * ����char�����ַ���ɵ��ַ���ת��Ϊwstring����
 *
 * @param pSrc Ҫת�����ַ�����
 * @param nSrcLen Ҫת�����ַ��������Ϊ-1���ʾת��ֱ������ֹ��Ϊֹ��
 * @return ���ִ�гɹ���Ϊת���Ľ�������û�����ݿ�ת������ת��ʧ����Ϊ���ַ�����
 */
wstring AnsiStrToWideStr( const char *pSrc, int nSrcLen /* = -1 */ )
{
	if( nSrcLen == 0 ) return wstring();

	bool bInlcudeTerminator = nSrcLen == -1;
	wstring Buffer( 1024, 0 );
	for( int i = 0; i < 10; i++ )
	{
		int nDestLen = AnsiStrToWideStr( pSrc, nSrcLen, &Buffer[0], Buffer.size() );
		if( nDestLen >= 0 )
		{
			Buffer.resize( nDestLen );
			if( bInlcudeTerminator && !Buffer.empty() ) Buffer.resize( Buffer.size() - 1 ); // ȥ������ֹ��
			return Buffer;
		}

		Buffer.resize( Buffer.size() * 2 ); // ������̫С��Ҫ���·���Ȼ���ٴν��и�ʽ��
	}

	return wstring();
}

/**
 * ����wchar_t�����ַ���ɵ��ַ���ת��Ϊ��char�����ַ���ɵ��ַ�����
 *
 * @param pSrc Ҫת�����ַ�����
 * @param nSrcLen Ҫת�����ַ��������Ϊ-1���ʾת��ֱ������ֹ��Ϊֹ��
 * @param[out] pBuffer ���ת������Ļ�������
 * @param nMaxBuffer ���ת������Ļ��������ɴ洢�ַ�����
 * @return ���ִ�гɹ���Ϊת��������ַ�����������ִ�����߻�����̫С��Ϊ-1��
 */
int WideStrToAnsiStr( const wchar_t *pSrc, int nSrcLen, char *pBuffer, int nMaxBuffer )
{
	if( pSrc == NULL ) return -1;
	if( nSrcLen == 0 ) return 0;
	if( pBuffer == NULL || nMaxBuffer < 0 ) return -1;

#ifdef _WIN32
	int nDestLen = WideCharToMultiByte( CP_ACP, 0, pSrc, nSrcLen, pBuffer, nMaxBuffer, NULL, NULL );
	return nDestLen == 0 ? -1 : nDestLen;
#else
	if( nSrcLen == -1 ) nSrcLen = wcslen( pSrc ) + 1;
	return StrIconv( WIDE_CHARSET, (char *)pSrc, sizeof(wchar_t) * nSrcLen, "", pBuffer, nMaxBuffer );
#endif
}

/**
 * ����wchar_t�����ַ���ɵ��ַ���ת��Ϊstring����
 *
 * @param pSrc Ҫת�����ַ�����
 * @param nSrcLen Ҫת�����ַ��������Ϊ-1���ʾת��ֱ������ֹ��Ϊֹ��
 * @return ���ִ�гɹ���Ϊת���Ľ�������û�����ݿ�ת������ת��ʧ����Ϊ���ַ�����
 */
string WideStrToAnsiStr( const wchar_t *pSrc, int nSrcLen /* = -1 */ )
{
	if( nSrcLen == 0 ) return string();

	bool bInlcudeTerminator = nSrcLen == -1;
	string Buffer( 1024, 0 );
	for( int i = 0; i < 10; i++ )
	{
		int nDestLen = WideStrToAnsiStr( pSrc, nSrcLen, &Buffer[0], Buffer.size() );
		if( nDestLen >= 0 )
		{
			Buffer.resize( nDestLen );
			if( bInlcudeTerminator && !Buffer.empty() ) Buffer.resize( Buffer.size() - 1 ); // ȥ������ֹ��
			return Buffer;
		}

		Buffer.resize( Buffer.size() * 2 ); // ������̫С��Ҫ���·���Ȼ���ٴν��и�ʽ��
	}

	return string();
}

/**
 * ��ʽ��һ���ַ�����
 *
 * @param pFormat Ҫ��¼�ĸ�ʽ����Ϣ����ʽ�������ANSI C�ĸ�ʽ���淶��
 * @param ... ��ʽ���Ĳ�����
 * @return ��Ÿ�ʽ�������tstring����
 */
string FormatStr(const char *pFormat, ...)
{
	va_list pArgs;
	va_start(pArgs, pFormat);
	const string &Result = VFormatStr(pFormat, pArgs);
	va_end(pArgs);

	return Result;
}

/**
 * ��ʽ��һ���ַ�����
 *
 * @param pFormat ��ʽ����Ϣ���������ANSI C�ĸ�ʽ���淶��
 * @param pArgs ��ʽ���Ĳ�����
 * @return ��Ÿ�ʽ�������string����
 */
string VFormatStr(const char *pFormat, va_list pArgs)
{
	string Result(1024, 0);
	for (int i = 0; i < 10; i++)
	{
#ifdef _WIN32
		int nResultLen = vsnprintf(&Result[0], Result.size(), pFormat, pArgs);
#else
		va_list pArgs2;
		va_copy(pArgs2, pArgs);
		int nResultLen = vsnprintf(&Result[0], Result.size(), pFormat, pArgs2);
		va_end(pArgs2);
#endif

		if (nResultLen < 0) // ������̫С��Ҫ���·���Ȼ���ٴν��и�ʽ��
		{
			Result.resize(Result.size() * 2);
		}
		else if (nResultLen < (int)Result.size()) // ��ʽ���ɹ�
		{
			Result.resize(nResultLen);
			return Result;
		}
		else // glibc 2.1�᷵����Ҫ���ַ���
		{
			Result.resize(nResultLen + 1);
		}
	}

	return string();
}

/**
 * ��ʽ��һ���ַ�����
 *
 * @param pFormat Ҫ��¼�ĸ�ʽ����Ϣ����ʽ�������ANSI C�ĸ�ʽ���淶��
 * @param ... ��ʽ���Ĳ�����
 * @return ��Ÿ�ʽ�������wstring����
 */
wstring FormatStr(const wchar_t *pFormat, ...)
{
	va_list pArgs;
	va_start(pArgs, pFormat);
	const wstring &Result = VFormatStr(pFormat, pArgs);
	va_end(pArgs);

	return Result;
}

/**
 * ��ʽ��һ���ַ�����
 *
 * @param pFormat ��ʽ����Ϣ���������ANSI C�ĸ�ʽ���淶��
 * @param pArgs ��ʽ���Ĳ�����
 * @return ��Ÿ�ʽ�������wstring����
 */
wstring VFormatStr(const wchar_t *pFormat, va_list pArgs)
{
	wstring Result(1024, 0);
	for (int i = 0; i < 10; i++)
	{
#ifdef _WIN32
		int nResultLen = vswprintf(&Result[0], Result.size(), pFormat, pArgs);
#else
		va_list pArgs2;
		va_copy(pArgs2, pArgs);
		int nResultLen = vswprintf(&Result[0], Result.size(), pFormat, pArgs2);
		va_end(pArgs2);
#endif

		if (nResultLen < 0) // ������̫С��Ҫ���·���Ȼ���ٴν��и�ʽ��
		{
			Result.resize(Result.size() * 2);
		}
		else if (nResultLen < (int)Result.size()) // ��ʽ���ɹ�
		{
			Result.resize(nResultLen);
			return Result;
		}
		else // glibc�����İ汾�᷵����Ҫ���ַ���?
		{
			Result.resize(nResultLen + 1);
		}
	}

	return wstring();
}

/**
 * ���һ��·����Ŀ¼���֡�
 *
 * @param Path ·����
 * @param cSep ·���еķָ�����
 * @return ·����Ŀ¼���֡�
 */
wstring GetDirectoryFromPathW(const wstring & Path, const wchar_t cSep)
{
	wstring::size_type SepPos = Path.rfind(cSep); // �������һ���ָ���
	if (SepPos == wstring::npos) return wstring(); // û���ҵ��ָ���
	if (SepPos == Path.size() - 1) return wstring(); // �ָ���λ��·����ĩβ

	if (SepPos == 0)
	{
		if (Path.size() == 1) return wstring(); // ·���Ǹ�Ŀ¼
		else return wstring(1, cSep); // ·���Ǹ�Ŀ¼�µĽڵ�
	}

	return Path.substr(0, SepPos);
}

/**
 * ���һ��·�����ļ������֡�
 *
 * @param Path ·����
 * @param cSep ·���еķָ�����
 * @return ·�����ļ������֡�
 */
wstring GetNameFromPathW(const wstring & Path, const wchar_t cSep)
{
	wstring::size_type SepPos = Path.rfind(cSep); // �������һ���ָ���
	if (SepPos == wstring::npos) return wstring(); // û���ҵ��ָ���
	if (SepPos == Path.size() - 1) return wstring(); // �ָ���λ��·����ĩβ

	if (SepPos == 0 && Path.size() == 1) return wstring(1, cSep); // ·���Ǹ�Ŀ¼	

	return Path.substr(SepPos + 1);
}

/**
 * ���һ��·����Ŀ¼���֡�
 *
 * @param Path ·����
 * @param cSep ·���еķָ�����
 * @return ·����Ŀ¼���֡�
 */
string GetDirectoryFromPathA(const string & Path, const char cSep)
{
	string::size_type SepPos = Path.rfind(cSep); // �������һ���ָ���
	if (SepPos == string::npos) return string(); // û���ҵ��ָ���
	if (SepPos == Path.size() - 1) return string(); // �ָ���λ��·����ĩβ

	if (SepPos == 0)
	{
		if (Path.size() == 1) return string(); // ·���Ǹ�Ŀ¼
		else return string(1, cSep); // ·���Ǹ�Ŀ¼�µĽڵ�
	}

	return Path.substr(0, SepPos);
}

/**
 * ���һ��·�����ļ������֡�
 *
 * @param Path ·����
 * @param cSep ·���еķָ�����
 * @return ·�����ļ������֡�
 */
string GetNameFromPathA(const string & Path, const char cSep)
{
	string::size_type SepPos = Path.rfind(cSep); // �������һ���ָ���
	if (SepPos == string::npos) return string(); // û���ҵ��ָ���
	if (SepPos == Path.size() - 1) return string(); // �ָ���λ��·����ĩβ

	if (SepPos == 0 && Path.size() == 1) return string(1, cSep); // ·���Ǹ�Ŀ¼	

	return Path.substr(SepPos + 1);
}

/**
 * �ж��ַ����Ƿ�����һ�ַ�����β
 *
 * @param pSrc ���жϵ��ַ���
 * @param pEnd ��β�ַ���
 * @return
 */
bool EndWith(const char * pSrc, const char * pEnd)
{
	bool result = false;
	if (NULL != pSrc && NULL != pEnd)
	{
		size_t srcLen = strlen(pSrc);
		size_t endLen = strlen(pEnd);
		if (srcLen >= endLen)
		{
			return strcmp(pSrc + srcLen - endLen, pEnd) == 0;
		}
	}
	return result;
}

/**
 * �滻�ַ���
 *
 * @param src ԭ�ַ���
 * @param pOldString ��Ҫ�滻���ַ���
 * @param pNewString ���ַ���
 * @return �滻����ַ���
 */
 string Replace(const string & src, const char * pOldString, const char * pNewString)
 {
	 string strResult(src);
	 size_t nOldStringLen = strlen(pOldString);
	 size_t nNewStringLen = strlen(pNewString);
	 
	 for (string::size_type nPos(0); string::npos != (nPos = strResult.find(pOldString, nPos)); nPos += nNewStringLen)
	 {
		 strResult.replace(nPos, nOldStringLen, pNewString);
	 }
	 return strResult;
 }
