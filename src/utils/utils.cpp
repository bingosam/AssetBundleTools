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
 * 用于产生随机数的加密句柄。
 */
static HCRYPTPROV g_hRandomCryptProv = 0;
#else
/**
 * 随机数设备的文件描述符。
 */
static int g_iRandomFile = -1;
#endif
/**
 * 操作系统名称。
 */
static tstring g_OsName;

/**
 * 用于Base64编码。
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
 * 用于Base64解码。
 */
static char g_cBase64DecodingTable[256];

/**
 * 用于Base64计算。
 */
static int g_iBase64ModTable[] = { 0, 2, 1 };

/**
 * 如果当前使用的字符集编码为UTF-8则为true。
 */
static bool g_bWithUtf8 = false;

/**
 * 初始化实用工具函数相关资源。
 */
class CUtilInit
{
public:
	CUtilInit()
	{
#ifdef _WIN32
		// 随机数 --------------------------
		CryptAcquireContext( &g_hRandomCryptProv, NULL, NULL, PROV_RSA_FULL, 0 );

		// 操作系统名称 --------------------------
		OSVERSIONINFOEXW Ver;
		Ver.dwOSVersionInfoSize = sizeof( Ver );
		if( GetVersionExW((LPOSVERSIONINFO)&Ver) )
		{
			g_OsName.resize( 512 );
			if( Ver.dwPlatformId == VER_PLATFORM_WIN32_NT ) // 操作是Windows NT系列
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
		// 随机数 --------------------------
		g_iRandomFile = open( "/dev/urandom", O_RDONLY );
			
		// 操作系统名称 --------------------------
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
 * 获得指定操作系统错误码表示的窄字符文本信息。
 *
 * @param iOsError 操作系统的错误码，如果为0则表示当前线程最后产生的错误。
 * @return 如果执行成功则为错误码的文本信息，如果执行失败则为空字符串。
 */
string GetOsErrorMessageA(int iOsError /* = 0 */)
{
#ifdef _WIN32
	if (iOsError == 0) iOsError = GetLastError();

	string Buffer(1024, 0);
	for (int i = 0; i < 8; i++)
	{
		DWORD nMessageLen = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS
			, NULL, (DWORD)iOsError, GetUserDefaultLangID(), &Buffer[0], Buffer.size(), NULL); // 获得系统错误信息
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
 * 获得指定操作系统错误码表示的宽字符文本信息。
 *
 * @param iOsError 操作系统的错误码，如果为0则表示当前线程最后产生的错误。
 * @return 如果执行成功则为错误码的文本信息，如果执行失败则为空字符串。
 */
wstring GetOsErrorMessageW(int iOsError /* = 0 */)
{
#ifdef _WIN32
	if (iOsError == 0) iOsError = GetLastError();

	wstring Buffer(1024, 0);
	for (int i = 0; i < 8; i++)
	{
		DWORD nMessageLen = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS
			, NULL, (DWORD)iOsError, GetUserDefaultLangID(), &Buffer[0], Buffer.size(), NULL); // 获得系统错误信息
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
 * 获得当前时区与格林威治之间相差的分钟数。<br>
 * 本地时区时间加上该值能够得到对应的格林威治时间。
 */
INT32 GetTimeZoneBias()
{
#ifdef _WIN32 // 操作系统是Windows
	TIME_ZONE_INFORMATION TimeZone;
	if( GetTimeZoneInformation(&TimeZone) > 2 ) return 0; // 获得时区信息失败

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
 * 获得从操作系统启动开始到当前的微秒数。
 */
INT64 GetRealClock()
{
#ifdef _WIN32
	static volatile LONG bFrequencyQueried = 0; // 如果已经查询了高精度计数器的频率则为非0
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
 * 产生一个32位的伪随机数。
 *
 * @param iMin 可产生的随机数最小值。
 * @param iMax 可产生的随机数最大值。
 * @return 产生的随机数。
 */
INT32 RandomInt32( INT32 iMin /* = -2147483647 - 1 */, INT32 iMax /* = 2147483647 */ )
{
	static __THREAD UINT32 uiSeed = 0; // 种子

	if( iMin >= iMax ) return iMin;

	if( uiSeed == 0 ) // 还没有产生种子
	{
#if defined( _WIN32 ) // 操作系统是Windows
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

	UINT32 nSpan = (UINT32)( iMax - iMin ) + 1; // 计算结果的范围
	if( nSpan == 0 ) nSpan = 0xFFFFFFFF;
	return (INT32)( uiSeed % nSpan ) + iMin;
}

/**
 * 产生一个64位的伪随机数。
 *
 * @param iMin 可产生的随机数最小值。
 * @param iMax 可产生的随机数最大值。
 * @return 产生的随机数。
 */
INT64 RandomInt64( INT64 iMin /* = -9223372036854775807LL - 1 */, INT64 iMax /* = 9223372036854775807LL */ )
{
	static __THREAD UINT64 uiSeed = 0; // 种子

	if( iMin >= iMax ) return iMin;

	if( uiSeed == 0 ) // 还没有产生种子
	{
#if defined( _WIN32 ) // 操作系统是Windows
		if( !QueryPerformanceCounter((LARGE_INTEGER *)&uiSeed) ) throw COsError( "Failed to QueryPerformanceCounter:" );
#else
		struct timeval Time;
		if( gettimeofday(&Time,NULL) != 0 ) throw COsError( "Failed to gettimeofday:" );

		uiSeed = (UINT64)Time.tv_sec * Time.tv_usec + Time.tv_usec;
#endif
	}

	uiSeed = uiSeed * 1103515245 + 12345;

	UINT64 nSpan = (UINT64)( iMax - iMin ) + 1; // 计算结果的范围
	if( nSpan == 0 ) nSpan = 0xFFFFFFFFFFFFFFFFLL;
	return (INT64)( uiSeed % nSpan ) + iMin;
}

/**
 * 使用更可靠的方法产生一个32位的随机数。
 *
 * @param iMin 可产生的随机数最小值。
 * @param iMax 可产生的随机数最大值。
 * @return 产生的随机数。
 * @note 这个函数的运行速度要慢于RandomInt32。
 */
INT32 RandomInt32Ex( INT32 iMin /* = -2147483647 - 1 */, INT32 iMax /* = 2147483647 */ )
{
	if( iMin >= iMax ) return iMin;

#if defined( _WIN32 ) // 操作系统是Windows
	UINT32 uiValue;
	if( !CryptGenRandom(g_hRandomCryptProv, sizeof(uiValue), (BYTE *)&uiValue) ) throw COsError( "Failed to CryptGenRandom:" );
	
	UINT32 nSpan = (UINT32)( iMax - iMin ) + 1; // 计算结果的范围
	if( nSpan == 0 ) nSpan = 0xFFFFFFFF;
	return (INT32)( uiValue % nSpan ) + iMin;
#elif defined( __unix__ ) // 操作系统是类Unix
	UINT32 uiValue;
	if( read(g_iRandomFile, &uiValue, sizeof(uiValue)) == -1 ) throw COsError( errno, "Failed to read urandom:" );

	UINT32 nSpan = (UINT32)( iMax - iMin ) + 1; // 计算结果的范围
	if( nSpan == 0 ) nSpan = 0xFFFFFFFF;
	return (INT32)( uiValue % nSpan ) + iMin;
#else
	return RandomInt32( iMin, iMax );
#endif
}

/**
 * 使用更可靠的方法产生一个64位的随机数。
 *
 * @param iMin 可产生的随机数最小值。
 * @param iMax 可产生的随机数最大值。
 * @return 产生的随机数。
 * @note 这个函数的运行速度要慢于RandomInt64。
 */
INT64 RandomInt64Ex( INT64 iMin /* = -9223372036854775807LL - 1 */, INT64 iMax /* = 9223372036854775807LL */ )
{
	if( iMin >= iMax ) return iMin;

#ifdef _WIN32
	UINT64 uiValue;
	if( !CryptGenRandom(g_hRandomCryptProv, sizeof(uiValue), (BYTE *)&uiValue) ) throw COsError( "Failed to CryptGenRandom:" );
	
	UINT64 nSpan = (UINT64)( iMax - iMin ) + 1; // 计算结果的范围
	if( nSpan == 0 ) nSpan = 0xFFFFFFFFFFFFFFFFLL;
	return (INT64)( uiValue % nSpan ) + iMin;
#else
	UINT64 uiValue;
	if( read(g_iRandomFile, &uiValue, sizeof(uiValue)) == -1 ) throw COsError( "Failed to read urandom:" );

	UINT64 nSpan = (UINT64)( iMax - iMin ) + 1; // 计算结果的范围
	if( nSpan == 0 ) nSpan = 0xFFFFFFFFFFFFFFFFLL;
	return (INT64)( uiValue % nSpan ) + iMin;
#endif
}

/**
 * 获得本地计算机的主机名。
 */
tstring GetHostName()
{
#if defined( _WIN32 ) // 操作系统是Windows
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
 * 获得本地计算机所在的域或工作组名称。
 */
tstring GetDomainName()
{
#if defined( _WIN32 )
	tstring DomainName;

	WKSTA_INFO_100 *pWorkstationInfo;
	if( NetWkstaGetInfo(NULL,100,(LPBYTE *)&pWorkstationInfo ) == NERR_Success ) // 获得本机工作站信息
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
 * 获得操作系统名称。
 *
 * @return 操作系统名称。
 */
tstring GetOsName()
{
	return g_OsName;
}

/**
 * 判断一段数据的内容是否全部为0值。
 *
 * @param pData 存放数据的缓冲区。
 * @param nSize 数据的字节数。
 * @return 如果数据的内容全部为0值则为true，否则为false。
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
 * 用CRC32算法计算出数据的4个字节散列值。
 *
 * @param pData 要计算的数据。
 * @param nDataSize 要计算的数据字节数。
 * @return 计算的结果。
 * @author ksaiy
 * @note 代码摘自<a href="http://blog.csdn.net/ksaiy/archive/2005/05/27/382162.aspx">获取文件的CRC32值(VC++源码-固定码表)。。。。。</a>。
 */
UINT32 CRC32( const BYTE *pData, size_t nDataSize )
{
	UINT32 uiHash = 0xffffffff;	
	for( UINT32 i = 0; i < nDataSize; i++ ) uiHash = ( uiHash >> 8 ) ^ g_Crc32Table[(uiHash&0xFF)^pData[i]];
	return uiHash ^ 0xffffffff;
}

/**
 * 用MurmurHash2算法计算出数据的4个字节散列值。
 *
 * MurmurHash2, by Austin Appleby
 * This code makes a few assumptions about how your machine behaves -
 * 1. We can read a 4-byte value from any address without crashing
 * 2. sizeof(int) == 4
 * And it has a few limitations -
 * 1. It will not work incrementally.
 * 2. It will not produce the same results on little-endian and big-endian machines.
 *
 * @param pData 要计算的数据。
 * @param nDataSize 要计算的数据字节数。
 * @param uiSeed 种子值。
 * @return 计算的结果。
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
 * 用MurmurHash2算法计算出数据的8个字节散列值。
 *
 * MurmurHash2, 64-bit versions, by Austin Appleby<br>
 * The same caveats as 32-bit MurmurHash2 apply here - beware of alignment 
 * and endian-ness issues if used across multiple platforms.
 *
 * @param pData 要计算的数据。
 * @param nDataSize 要计算的数据字节数。
 * @param uiSeed 种子值。
 * @return 计算的结果。
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
 * 用MD4算法计算数据的16个字节散列值。
 *
 * @param pData 要计算的数据。
 * @param nDataSize 要计算的数据字节数。
 * @param[out] pMessageDigest 存储散列值的缓冲区，容量不能小于16字节。
 */
void MD4Hash( const void *pData, size_t nDataSize, BYTE *pMessageDigest )
{
#if defined( _WIN32 ) // 操作系统是Windows
	HCRYPTPROV hCryptProv = 0;
	HCRYPTHASH hHash = 0;

	try
	{
		if( !CryptAcquireContext(&hCryptProv,NULL,NULL,PROV_RSA_FULL,0) )
		{
			if( GetLastError() != NTE_BAD_KEYSET || !CryptAcquireContext(&hCryptProv,NULL,NULL,PROV_RSA_FULL,CRYPT_NEWKEYSET) ) throw COsError();
		}
		if( !CryptCreateHash(hCryptProv,CALG_MD4,0,0,&hHash) ) throw COsError(); // 创建哈希表失败
		if( !CryptHashData(hHash,(BYTE *)pData,nDataSize,0) ) throw COsError(); // 计算数据失败

		DWORD nSize = 16;
		if( !CryptGetHashParam(hHash,HP_HASHVAL,pMessageDigest,&nSize,0) ) throw COsError(); // 获得计算结果失败		
	}
	catch( ... )
	{
		if( hHash != 0 ) CryptDestroyHash( hHash );
		if( hCryptProv != 0 ) CryptReleaseContext( hCryptProv, 0 );

		throw;
	}

	if( hHash != 0 ) CryptDestroyHash( hHash );
	if( hCryptProv != 0 ) CryptReleaseContext( hCryptProv, 0 );
#elif defined( HEADER_MD4_H ) // 使用OpenSSL的加密API
	MD4_CTX Context;
	MD4_Init( &Context );
	MD4_Update( &Context, pData, nDataSize );
	MD4_Final( pMessageDigest, &Context );
#else
#error MD4 not implement!
#endif
}

/**
 * 用MD5算法计算数据的16个字节散列值。
 *
 * @param pData 要计算的数据。
 * @param nDataSize 要计算的数据字节数。
 * @param[out] pMessageDigest 存储散列值的缓冲区，容量不能小于16字节。
 */
void MD5Hash(const void *pData, size_t nDataSize, BYTE *pMessageDigest)
{
#if defined( _WIN32 ) // 操作系统是Windows
	HCRYPTPROV hCryptProv = 0;
	HCRYPTHASH hHash = 0;

	try
	{
		if (!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, 0)) // 打开CSP失败
		{
			if (GetLastError() != NTE_BAD_KEYSET || !CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET)) throw COsError();
		}
		if (!CryptCreateHash(hCryptProv, CALG_MD5, 0, 0, &hHash)) throw COsError(); // 创建哈希表失败
		if (!CryptHashData(hHash, (BYTE *)pData, nDataSize, 0)) throw COsError(); // 计算数据失败

		DWORD nSize = 16;
		if (!CryptGetHashParam(hHash, HP_HASHVAL, pMessageDigest, &nSize, 0)) throw COsError(); // 获得计算结果失败		
	}
	catch (...)
	{
		if (hHash != 0) CryptDestroyHash(hHash);
		if (hCryptProv != 0) CryptReleaseContext(hCryptProv, 0);
		throw;
	}

	if (hHash != 0) CryptDestroyHash(hHash);
	if (hCryptProv != 0) CryptReleaseContext(hCryptProv, 0);
#elif defined( HEADER_MD5_H ) // 使用OpenSSL的加密API
	MD5_CTX Context;
	MD5_Init(&Context);
	MD5_Update(&Context, pData, nDataSize);
	MD5_Final(pMessageDigest, &Context);
#else
#error MD5 not implement!
#endif
}

/**
 * 用MD5算法计算出数据的哈希值并得到十六进制字符串。
 *
 * @param pData 要计算的数据。
 * @param nDataSize 要计算的数据字节数。
 * @param bWithLowerCase 如果要返回小写字符串则为true。
 * @return 哈希值的十六进制字符串。
 */
string MD5Hash(const void *pData, size_t nDataSize, bool bWithLowerCase /* = true */)
{
	BYTE byHash[16];
	MD5Hash(pData, nDataSize, byHash);
	return BinToHexA(byHash, 16, bWithLowerCase);
}

/**
 * 用HMAC MD5算法计算数据的16个字节散列值。
 *
 * @param pKey 加密使用的关键字。
 * @param nKeySize 加密使用的关键字字节数。
 * @param pData 要计算的数据。
 * @param nDataSize 要计算的数据字节数。
 * @param[out] pMessageDigest 存储散列值的缓冲区，容量不能小于16字节。
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
 * 用SHA1算法计算数据的20个字节散列值。
 *
 * @param pData 要计算的数据。
 * @param nDataSize 要计算的数据字节数。
 * @param[out] pMessageDigest 存储散列值的缓冲区，容量不能小于20字节。
 */
void SHA1Hash( const void *pData, size_t nDataSize, BYTE *pMessageDigest )
{
#if defined( _WIN32 ) // 操作系统是Windows
	HCRYPTPROV hCryptProv = 0;
	HCRYPTHASH hHash = 0;

	try
	{
		if( !CryptAcquireContext(&hCryptProv,NULL,NULL,PROV_RSA_FULL,0) ) // 打开CSP失败
		{
			if( GetLastError() != NTE_BAD_KEYSET || !CryptAcquireContext(&hCryptProv,NULL,NULL,PROV_RSA_FULL,CRYPT_NEWKEYSET) ) throw COsError();
		}
		if( !CryptCreateHash(hCryptProv,CALG_SHA1,0,0,&hHash) ) throw COsError(); // 创建哈希表失败
		if( !CryptHashData(hHash,(BYTE *)pData,nDataSize,0) ) throw COsError(); // 计算数据失败

		DWORD nSize = 20;
		if( !CryptGetHashParam(hHash,HP_HASHVAL,pMessageDigest,&nSize,0) ) throw COsError(); // 获得计算结果失败		
	}
	catch( ... )
	{
		if( hHash != 0 ) CryptDestroyHash( hHash );
		if( hCryptProv != 0 ) CryptReleaseContext( hCryptProv, 0 );
		throw;
	}

	if( hHash != 0 ) CryptDestroyHash( hHash );
	if( hCryptProv != 0 ) CryptReleaseContext( hCryptProv, 0 );
#elif defined( HEADER_SHA_H ) // 使用OpenSSL的加密API
	SHA_CTX Context;
	SHA1_Init( &Context );
	SHA1_Update( &Context, pData, nDataSize );
	SHA1_Final( pMessageDigest, &Context );
#else
#error SHA1 not implement!
#endif
}

/**
 * 用HMAC SHA1算法计算数据的20个字节散列值。
 *
 * @param pKey 加密使用的关键字。
 * @param nKeySize 加密使用的关键字字节数。
 * @param pData 要计算的数据。
 * @param nDataSize 要计算的数据字节数。
 * @param[out] pMessageDigest 存储散列值的缓冲区，容量不能小于20字节。
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
 * Base64编码。
 *
 * @param pData 要编码的数据内容。
 * @param nDataSize 要编码的数据字节数。
 * @param[out] pBuffer 存放编码内容的缓冲区，包含字符串末尾的空终止符。容量不能小于 4.0 * ceil(nDataSize / 3.0)。
 * @param[out] nBufferSize 缓冲区的容量。
 * @return 如果执行成功则为true，如果缓冲区不足则为false。
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
 * Base64编码。
 *
 * @param pData 要编码的数据内容。
 * @param nDataSize 要编码的数据字节数。
 * @return 如果执行成功则为编码结果，否则为空内容。
 */
string Base64Encode( const void *pData, size_t nDataSize )
{
	string Result( (size_t)(4.0 * ceil(nDataSize / 3.0)), 0 );
	if( !Base64Encode(pData, nDataSize, &Result[0], Result.size()) ) return string();
	return Result;
}

/**
 * Base64解码。
 *
 * @param pData 要解码的数据内容。
 * @param nDataSize 要解码的数据字节数。
 * @param[out] pBuffer 存放已解码内容的缓冲区，容量不能小于 nDataSize / 4 * 3。
 * @param[in,out] nBufferSize 在函数执行时表示缓冲区的容量，在函数返回时用于存放已解码内容的字节数。
 * @return 如果执行成功则为true，如果要解码的数据无效或者缓冲区不足则为false。
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
 * 用DES算法进行加密或解密。
 *
 * @param bDoEncrypt 如果要进行加密则为true，如果要进行解密则为false。
 * @param pIn 当bDoEncrypt为true时，这个参数为要加密的数据。
 *                当bDoEncrypt为false时，这个参数为要解密的数据。
 * @parma[in] nInSize pIn的数据字节数。
 * @param[out] pOut 当bDoEncrypt为true时，这个参数用于存放加密的结果。
 *                  当bDoEncrypt为false时，这个参数用于存放解密的结果。
 * @param nMaxOut pOut能够存放的最大数据字节数，必须为8的倍数并且不能小于nInSize。
 * @param pKey 加密用的8字节关键字。
 * 
 * @note DES算法操作的数据字节数必须为8的倍数。<br>
 *       在加密时，如果pIn提供的加密数据字节数不是8的倍数，函数会自动在后面补充诺干0值对齐8字节然后进行加密。<br>
 *       在解密时，如果pIn提供的解密数据字节数不是8的倍数，将会导致函数执行失败。
 */
void DesEncrypt( bool bDoEncrypt, const BYTE *pIn, UINT32 nInSize, BYTE *pOut, UINT32 nMaxOut, const BYTE *pKey )
{
	if( pIn == NULL || pOut == NULL || pKey == NULL ) throw invalid_argument( "[DesEncrypt] The parameter is null!" );

	if( !bDoEncrypt && nInSize % 8 > 0 ) throw invalid_argument( "[DesEncrypt] The nInSize is invalid!" ); // 解密的数据字节数不是8的倍数

	if( nMaxOut < nInSize || nMaxOut % 8 > 0 ) throw invalid_argument( "[DesEncrypt] The nInSize is nMaxOut!" );

	if( nInSize < 1 ) return;

#if defined( _WIN32 ) // 操作系统是Windows
	HCRYPTPROV hCryptProv = 0;
	HCRYPTKEY hDesKey = 0;

	struct
    {
        BLOBHEADER Header;
        DWORD nKeySize;
        BYTE byKeyData[8];
    } KeyBlob; // 加密关键字信息

	try
	{
		if( nMaxOut < nInSize ) return;

		KeyBlob.Header.bType = PLAINTEXTKEYBLOB;
		KeyBlob.Header.bVersion = CUR_BLOB_VERSION;
		KeyBlob.Header.reserved = 0;
		KeyBlob.Header.aiKeyAlg = CALG_DES;
		KeyBlob.nKeySize = 8;
		memcpy( KeyBlob.byKeyData, pKey, 8 );
			
		if( !CryptAcquireContext(&hCryptProv,NULL,NULL,PROV_RSA_FULL,0) ) throw COsError(); // 创建CSP句柄		
		
		if( !CryptImportKey(hCryptProv,(BYTE *)&KeyBlob,sizeof(KeyBlob),0,0,&hDesKey) ) throw COsError(); // 设置关键字
		DWORD dwMode = CRYPT_MODE_ECB;
		CryptSetKeyParam( hDesKey, KP_MODE, (BYTE *)&dwMode, 0 ); // 设置DES加密的方式

		if( bDoEncrypt ) // 加密
		{
			vector<BYTE> TempBuf( nMaxOut + 16 ); // 更大的临时缓冲区（因为CryptEncrypt函数须要额外产生8个字节的对齐内容）
			memcpy( &TempBuf[0], pIn, nInSize );
			if( nInSize % 8 > 0 ) // 数据大小不是8的倍数
			{
				if( nMaxOut - nInSize < 8 - nInSize % 8 ) throw COsError( ERROR_INVALID_PARAMETER );

				memset( &TempBuf[nInSize], 0, 8 - nInSize % 8 ); // 在后面填充0
				nInSize = nInSize + ( 8 - nInSize % 8 );
			}
			DWORD dwDataLen = nInSize;
			if( !CryptEncrypt(hDesKey,0,TRUE,0,&TempBuf[0],&dwDataLen,TempBuf.size()) ) throw COsError();

			memcpy( pOut, &TempBuf[0], min(nMaxOut,(UINT32)dwDataLen) );
		}
		else // 解密
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
#elif defined( HEADER_DES_H ) // 使用OpenSSL的加密API
	if( !bDoEncrypt && nInSize % 8 > 0 ) return;
	
	DES_key_schedule Schedule;
	DES_set_key_unchecked( (DES_cblock *)pKey, &Schedule ); // 设置关键字
	
	UINT32 i;
	for( i = 0; i < nInSize; i += 8 )
	{
		if( bDoEncrypt ) // 加密
		{
			if( nInSize - i < 8 ) // 剩下还没有加密的数据大小不足8字节
			{
				BYTE byBuffer[8]; // 临时缓冲区
				memcpy( byBuffer, &pIn[i], nInSize - i );
				memset( &byBuffer[nInSize-i], 0, 8 - (nInSize-i) ); // 在后面填充0
				DES_ecb_encrypt( (DES_cblock *)byBuffer, (DES_cblock *)&pOut[i], &Schedule, DES_ENCRYPT );
			}
			else
			{
				DES_ecb_encrypt( (DES_cblock *)&pIn[i], (DES_cblock *)&pOut[i], &Schedule, DES_ENCRYPT );
			}
		}
		else // 解密
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
 * 判断当前系统字符集是否为UTF-8。
 */
bool WithUtf8()
{
	return g_bWithUtf8;
}

/**
 * 获得系统默认的字符集编码名称。
 *
 * @param[out] pBuffer 存放字符集编码名称的缓冲区。
 * @param nMaxBuffer 存放字符集编码名称的缓冲区最大可存储字符数，包括字符串末尾的空终止符。
 * @return 如果执行成功则为true，否则为false。
 */
bool GetSystemCharset( char *pBuffer, int nMaxBuffer )
{
	bool bResult;
#if _MSC_VER >= 1400 // 编译器是Microsoft C/C++ 14.0或更高版本
	bResult = sprintf_s( pBuffer, nMaxBuffer, "CP%u", GetACP() ) > 0;
#elif defined( _WIN32 ) // 操作系统是Windows
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
	// 通过环境变量获得本地化信息
	pLocale = getenv( "LC_ALL" );
	if( pLocale == NULL || pLocale[0] == 0 )
	{
		pLocale = getenv( "LC_CTYPE" );
		if( pLocale == NULL || pLocale[0] == 0 ) 
		{
			pLocale = getenv( "LANG" );			
		}
	}

	if( pLocale == NULL || pLocale[0] == 0 ) // 无法通过环境变量获得本地化信息
	{
		errno = EACCES;
		bResult = false;
	}
	else
	{	
		const char *pPoint = strrchr( pLocale, '.' ); // 检查本地化信息中的字符集是否被"."分隔
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
 * 获得指定字符集编码对应的代码页。
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
 * 对一个字符串进行字符集编码转换。
 *
 * @param pSrcCharset 源字符串的字符集编码，如果为NULL或者空内容则表示系统默认值。
 * @param pSrc 要转换的字符串。
 * @param nSrcSize 要转换的字节数。
 * @param pDestCharset 要转换的目标字符集编码，如果为NULL或空内容则表示系统默认值。
 * @param[out] pBuffer 存放转换结果的缓冲区。
 * @param nMaxBuffer 存放转换结果的缓冲区最大可存储字节数。
 * @return 如果执行成功则为转换结果的字节数。如果出现错误或者缓冲区太小则为-1。
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
	
	if( uiSrcCodePage == uiDestCodePage ) // 不须要转换
	{
		if( nSrcSize > nMaxBuffer ) return -1;
		memcpy( pBuffer, pSrc, nSrcSize );
		return nSrcSize;
	}
	else if( uiSrcCodePage == 1200 ) // 从宽字符转换
	{
		nSrcSize /= sizeof( WCHAR );
		int nDestLen = WideCharToMultiByte( uiDestCodePage, 0, (WCHAR *)pSrc, nSrcSize, pBuffer, nMaxBuffer, NULL, NULL );
		return nDestLen == 0 ? -1 : nDestLen;
	}
	else if( uiDestCodePage == 1200 ) // 转换到宽字符
	{
		nMaxBuffer /= sizeof( WCHAR );
		int nDestLen = MultiByteToWideChar( uiSrcCodePage, 0, pSrc, nSrcSize, (WCHAR *)pBuffer, nMaxBuffer );
		return nDestLen == 0 ? -1 : sizeof( WCHAR ) * nDestLen;
	}
	else
	{
		vector<WCHAR> UnicodeSrc( 512 );
		int nUnicodeLen = MultiByteToWideChar( uiSrcCodePage, 0, pSrc, nSrcSize, &UnicodeSrc[0], UnicodeSrc.size() ); // 先转换到宽字符
		if( nUnicodeLen == 0 )
		{
			if( GetLastError() != ERROR_INSUFFICIENT_BUFFER ) return -1; // 出错的原因并非缓冲区不足

			nUnicodeLen = MultiByteToWideChar( uiSrcCodePage, 0, pSrc, nSrcSize, NULL, 0 ); // 获得转换到宽字符后的字符数
			if( nUnicodeLen == 0 ) return -1;

			UnicodeSrc.resize( nUnicodeLen );
			nUnicodeLen = MultiByteToWideChar( uiSrcCodePage, 0, pSrc, nSrcSize, &UnicodeSrc[0], UnicodeSrc.size() ); // 转换到宽字符
			if( nUnicodeLen == 0 ) return -1;
		}

		int nDestLen = WideCharToMultiByte( uiDestCodePage, 0, &UnicodeSrc[0], nUnicodeLen, pBuffer, nMaxBuffer, NULL, NULL ); // 从宽字符转换到目标字符集编码
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

#if __SIZEOF_WCHAR_T__ == 2 // 在当前平台上wchar_t类型的字节数是2
#define WIDE_CHARSET "utf-16le"
#else
#define WIDE_CHARSET "utf-32le"
#endif

/**
 * 把由char类型字符组成的字符串转换为由char类型字符组成并且按UTF-8编码的字符串。
 *
 * @param pSrc 要转换的字符串。
 * @param nSrcLen 要转换的字符数，如果为-1则表示转换直到空终止符为止。
 * @param[out] pBuffer 存放转换结果的缓冲区。
 * @param nMaxBuffer 存放转换结果的缓冲区最大可存储字符数。
 * @param pSrcCharset 字符串的字符集编码，如果为NULL或者空内容则表示系统默认值。
 * @return 如果执行成功则为转换结果的字符数。如果出现错误或者缓冲区太小则为-1。
 */
int AnsiStrToUtf8Str( const char *pSrc, int nSrcLen, char *pBuffer, int nMaxBuffer, const char *pSrcCharset /* = NULL */ )
{
	if( pSrc == NULL ) return -1;
	if( nSrcLen == 0 ) return 0;
	if( pBuffer == NULL || nMaxBuffer < 0 ) return -1;

	if( nSrcLen == -1 ) nSrcLen = strlen( pSrc ) + 1;

	if( (g_bWithUtf8 && (pSrcCharset == NULL || pSrcCharset[0] == 0))
		|| (pSrcCharset != NULL && stricmp(pSrcCharset, "utf-8") == 0) ) // 源字符集和目标字符集都为UTF-8
	{
		if( nSrcLen > nMaxBuffer ) return -1;
		memcpy( pBuffer, pSrc, nSrcLen );
		return nSrcLen;
	}

#ifdef _WIN32
	if( pSrcCharset != NULL && pSrcCharset[0] != 0 ) return StrIconv( pSrcCharset, pSrc, nSrcLen, "utf-8", pBuffer, nMaxBuffer );

	vector<WCHAR> UnicodeSrc( nSrcLen );
	int nUnicodeLen = MultiByteToWideChar( CP_ACP, 0, pSrc, nSrcLen, &UnicodeSrc[0], UnicodeSrc.size() ); // 先转换到宽字符
	if( nUnicodeLen == 0 ) return -1;

	int nDestLen = WideCharToMultiByte( CP_UTF8, 0, &UnicodeSrc[0], nUnicodeLen, pBuffer, nMaxBuffer, NULL, NULL ); // 从宽字符转换到目标字符集编码
	return nDestLen == 0 ? -1 : nDestLen;
#else
	return StrIconv( pSrcCharset, pSrc, nSrcLen, "utf-8", pBuffer, nMaxBuffer );
#endif
}

/**
 * 把由char类型字符组成的字符串转换为按UTF-8编码的string对象。
 *
 * @param pSrc 要转换的字符串。
 * @param nSrcLen 要转换的字符数，如果为-1则表示转换直到空终止符为止。
 * @param pSrcCharset 字符串的字符集编码，如果为NULL或者空内容则表示系统默认值。
 * @return 如果执行成功则为转换的结果，如果没有内容可转换或者转换失败则为空字符串。
 */
string AnsiStrToUtf8Str( const char *pSrc, int nSrcLen /* = -1 */, const char *pSrcCharset /* = NULL */ )
{
	if( nSrcLen == 0 ) return string();

	if( (g_bWithUtf8 && (pSrcCharset == NULL || pSrcCharset[0] == 0))
		|| (pSrcCharset != NULL && stricmp(pSrcCharset, "utf-8") == 0) ) // 源字符集和目标字符集都为UTF-8
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
			if( bInlcudeTerminator && !Buffer.empty() ) Buffer.resize( Buffer.size() - 1 ); // 去掉空终止符
			return Buffer;
		}

		Buffer.resize( Buffer.size() * 2 ); // 缓冲区太小须要重新分配然后再次进行格式化
	}

	return string();
}

/**
 * 把由wchar_t类型字符组成的字符串转换为由char类型字符组成并且按UTF-8编码的字符串。
 *
 * @param pSrc 要转换的字符串。
 * @param nSrcLen 要转换的字符数，如果为-1则表示转换直到空终止符为止。
 * @param[out] pBuffer 存放转换结果的缓冲区。
 * @param nMaxBuffer 存放转换结果的缓冲区最大可存储字符数。
 * @return 如果执行成功则为转换结果的字符数。如果出现错误或者缓冲区太小则为-1。
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
 * 把由wchar_t类型字符组成的字符串转换为按UTF-8编码的string对象。
 *
 * @param pSrc 要转换的字符串。
 * @param nSrcLen 要转换的字符数，如果为-1则表示转换直到空终止符为止。
 * @return 如果执行成功则为转换的结果，如果没有内容可转换或者转换失败则为空字符串。
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
			if( bInlcudeTerminator && !Buffer.empty() ) Buffer.resize( Buffer.size() - 1 ); // 去掉空终止符
			return Buffer;
		}

		Buffer.resize( Buffer.size() * 2 ); // 缓冲区太小须要重新分配然后再次进行格式化
	}

	return string();
}

/**
 * 把由char类型字符组成并且按UTF-8编码的字符串转换为由char类型字符组成的字符串。
 *
 * @param pSrc 要转换的字符串。
 * @param nSrcLen 要转换的字符数，如果为-1则表示转换直到空终止符为止。
 * @param[out] pBuffer 存放转换结果的缓冲区。
 * @param nMaxBuffer 存放转换结果的缓冲区最大可存储字符数。
 * @param pDestCharset 目标字符串的字符集编码，如果为NULL或者空内容则表示系统默认值。
 * @return 如果执行成功则为转换结果的字符数。如果出现错误或者缓冲区太小则为-1。
 */
int Utf8StrToAnsiStr( const char *pSrc, int nSrcLen, char *pBuffer, int nMaxBuffer, const char *pDestCharset /* = NULL */ )
{
	if( pSrc == NULL ) return -1;
	if( nSrcLen == 0 ) return 0;
	if( pBuffer == NULL || nMaxBuffer < 0 ) return -1;

	if( nSrcLen == -1 ) nSrcLen = strlen( pSrc ) + 1;

	if( (g_bWithUtf8 && (pDestCharset == NULL || pDestCharset[0] == 0))
		|| (pDestCharset != NULL && stricmp(pDestCharset, "utf-8") == 0) ) // 源字符集和目标字符集都为UTF-8
	{
		if( nSrcLen > nMaxBuffer ) return -1;
		memcpy( pBuffer, pSrc, nSrcLen );
		return nSrcLen;
	}

#ifdef _WIN32
	vector<WCHAR> UnicodeSrc( nSrcLen );
	int nUnicodeLen = MultiByteToWideChar( CP_UTF8, 0, pSrc, nSrcLen, &UnicodeSrc[0], UnicodeSrc.size() ); // 先转换到宽字符
	if( nUnicodeLen == 0 ) return -1;
	
	int nDestLen = WideCharToMultiByte( CP_ACP, 0, &UnicodeSrc[0], nUnicodeLen, pBuffer, nMaxBuffer, NULL, NULL ); // 从宽字符转换到目标字符集编码
	return nDestLen == 0 ? -1 : nDestLen;
#else
	return StrIconv( "utf-8", pSrc, nSrcLen, "", pBuffer, nMaxBuffer );
#endif
}

/**
 * 把由char类型字符组成并且按UTF-8编码的字符串转换为string对象。
 *
 * @param pSrc 要转换的字符串。
 * @param nSrcLen 要转换的字符数，如果为-1则表示转换直到空终止符为止。
 * @param pDestCharset 目标字符串的字符集编码，如果为NULL或者空内容则表示系统默认值。
 * @return 如果执行成功则为转换的结果，如果没有内容可转换或者转换失败则为空字符串。
 */
string Utf8StrToAnsiStr( const char *pSrc, int nSrcLen /* = -1 */, const char *pDestCharset /* = NULL */ )
{
	if( nSrcLen == 0 ) return string();
	
	if( (g_bWithUtf8 && (pDestCharset == NULL || pDestCharset[0] == 0))
		|| (pDestCharset != NULL && stricmp(pDestCharset, "utf-8") == 0) ) // 源字符集和目标字符集都为UTF-8
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
			if( bInlcudeTerminator && !Buffer.empty() ) Buffer.resize( Buffer.size() - 1 ); // 去掉空终止符
			return Buffer;
		}

		Buffer.resize( Buffer.size() * 2 ); // 缓冲区太小须要重新分配然后再次进行格式化
	}

	return string();
}

/**
 * 把由char类型字符组成并且按UTF-8编码的字符串转换为由wchar_t类型字符组成的字符串。
 *
 * @param pSrc 要转换的字符串。
 * @param nSrcLen 要转换的字符数，如果为-1则表示转换直到空终止符为止。
 * @param[out] pBuffer 存放转换结果的缓冲区。
 * @param nMaxBuffer 存放转换结果的缓冲区最大可存储字符数。
 * @return 如果执行成功则为转换结果的字符数。如果出现错误或者缓冲区太小则为-1。
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
 * 把由char类型字符组成并且按UTF-8编码的字符串转换为wstring对象。
 *
 * @param pSrc 要转换的字符串。
 * @param nSrcLen 要转换的字符数，如果为-1则表示转换直到空终止符为止。
 * @return 如果执行成功则为转换的结果，如果没有内容可转换或者转换失败则为空字符串。
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
			if( bInlcudeTerminator && !Buffer.empty() ) Buffer.resize( Buffer.size() - 1 ); // 去掉空终止符
			return Buffer;
		}

		Buffer.resize( Buffer.size() * 2 ); // 缓冲区太小须要重新分配然后再次进行格式化
	}

	return wstring();
}

/**
 * 把由char类型字符组成的字符串转换为wstring对象。
 *
 * @param pSrc 要转换的字符串。
 * @param nSrcLen 要转换的字符数，如果为-1则表示转换直到空终止符为止。
 * @param[out] pBuffer 存放转换结果的缓冲区，如果为NULL则表示只获得转换结果的字节数而不是真正执行转换。
 * @param nMaxBuffer 存放转换结果的缓冲区最大可存储字节数。
 * @return 如果执行成功则为转换结果的字符数。如果出现错误或者缓冲区太小则为-1。
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
 * 把由char类型字符组成的字符串转换为wstring对象。
 *
 * @param pSrc 要转换的字符串。
 * @param nSrcLen 要转换的字符数，如果为-1则表示转换直到空终止符为止。
 * @return 如果执行成功则为转换的结果，如果没有内容可转换或者转换失败则为空字符串。
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
			if( bInlcudeTerminator && !Buffer.empty() ) Buffer.resize( Buffer.size() - 1 ); // 去掉空终止符
			return Buffer;
		}

		Buffer.resize( Buffer.size() * 2 ); // 缓冲区太小须要重新分配然后再次进行格式化
	}

	return wstring();
}

/**
 * 把由wchar_t类型字符组成的字符串转换为由char类型字符组成的字符串。
 *
 * @param pSrc 要转换的字符串。
 * @param nSrcLen 要转换的字符数，如果为-1则表示转换直到空终止符为止。
 * @param[out] pBuffer 存放转换结果的缓冲区。
 * @param nMaxBuffer 存放转换结果的缓冲区最大可存储字符数。
 * @return 如果执行成功则为转换结果的字符数。如果出现错误或者缓冲区太小则为-1。
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
 * 把由wchar_t类型字符组成的字符串转换为string对象。
 *
 * @param pSrc 要转换的字符串。
 * @param nSrcLen 要转换的字符数，如果为-1则表示转换直到空终止符为止。
 * @return 如果执行成功则为转换的结果，如果没有内容可转换或者转换失败则为空字符串。
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
			if( bInlcudeTerminator && !Buffer.empty() ) Buffer.resize( Buffer.size() - 1 ); // 去掉空终止符
			return Buffer;
		}

		Buffer.resize( Buffer.size() * 2 ); // 缓冲区太小须要重新分配然后再次进行格式化
	}

	return string();
}

/**
 * 格式化一个字符串。
 *
 * @param pFormat 要记录的格式化信息，格式必须符合ANSI C的格式化规范。
 * @param ... 格式化的参数。
 * @return 存放格式化结果的tstring对象。
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
 * 格式化一个字符串。
 *
 * @param pFormat 格式化信息，必须符合ANSI C的格式化规范。
 * @param pArgs 格式化的参数。
 * @return 存放格式化结果的string对象。
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

		if (nResultLen < 0) // 缓冲区太小须要重新分配然后再次进行格式化
		{
			Result.resize(Result.size() * 2);
		}
		else if (nResultLen < (int)Result.size()) // 格式化成功
		{
			Result.resize(nResultLen);
			return Result;
		}
		else // glibc 2.1会返回需要的字符数
		{
			Result.resize(nResultLen + 1);
		}
	}

	return string();
}

/**
 * 格式化一个字符串。
 *
 * @param pFormat 要记录的格式化信息，格式必须符合ANSI C的格式化规范。
 * @param ... 格式化的参数。
 * @return 存放格式化结果的wstring对象。
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
 * 格式化一个字符串。
 *
 * @param pFormat 格式化信息，必须符合ANSI C的格式化规范。
 * @param pArgs 格式化的参数。
 * @return 存放格式化结果的wstring对象。
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

		if (nResultLen < 0) // 缓冲区太小须要重新分配然后再次进行格式化
		{
			Result.resize(Result.size() * 2);
		}
		else if (nResultLen < (int)Result.size()) // 格式化成功
		{
			Result.resize(nResultLen);
			return Result;
		}
		else // glibc将来的版本会返回需要的字符数?
		{
			Result.resize(nResultLen + 1);
		}
	}

	return wstring();
}

/**
 * 获得一个路径的目录部分。
 *
 * @param Path 路径。
 * @param cSep 路径中的分隔符。
 * @return 路径的目录部分。
 */
wstring GetDirectoryFromPathW(const wstring & Path, const wchar_t cSep)
{
	wstring::size_type SepPos = Path.rfind(cSep); // 查找最后一个分隔符
	if (SepPos == wstring::npos) return wstring(); // 没有找到分隔符
	if (SepPos == Path.size() - 1) return wstring(); // 分隔符位于路径的末尾

	if (SepPos == 0)
	{
		if (Path.size() == 1) return wstring(); // 路径是根目录
		else return wstring(1, cSep); // 路径是根目录下的节点
	}

	return Path.substr(0, SepPos);
}

/**
 * 获得一个路径的文件名部分。
 *
 * @param Path 路径。
 * @param cSep 路径中的分隔符。
 * @return 路径的文件名部分。
 */
wstring GetNameFromPathW(const wstring & Path, const wchar_t cSep)
{
	wstring::size_type SepPos = Path.rfind(cSep); // 查找最后一个分隔符
	if (SepPos == wstring::npos) return wstring(); // 没有找到分隔符
	if (SepPos == Path.size() - 1) return wstring(); // 分隔符位于路径的末尾

	if (SepPos == 0 && Path.size() == 1) return wstring(1, cSep); // 路径是根目录	

	return Path.substr(SepPos + 1);
}

/**
 * 获得一个路径的目录部分。
 *
 * @param Path 路径。
 * @param cSep 路径中的分隔符。
 * @return 路径的目录部分。
 */
string GetDirectoryFromPathA(const string & Path, const char cSep)
{
	string::size_type SepPos = Path.rfind(cSep); // 查找最后一个分隔符
	if (SepPos == string::npos) return string(); // 没有找到分隔符
	if (SepPos == Path.size() - 1) return string(); // 分隔符位于路径的末尾

	if (SepPos == 0)
	{
		if (Path.size() == 1) return string(); // 路径是根目录
		else return string(1, cSep); // 路径是根目录下的节点
	}

	return Path.substr(0, SepPos);
}

/**
 * 获得一个路径的文件名部分。
 *
 * @param Path 路径。
 * @param cSep 路径中的分隔符。
 * @return 路径的文件名部分。
 */
string GetNameFromPathA(const string & Path, const char cSep)
{
	string::size_type SepPos = Path.rfind(cSep); // 查找最后一个分隔符
	if (SepPos == string::npos) return string(); // 没有找到分隔符
	if (SepPos == Path.size() - 1) return string(); // 分隔符位于路径的末尾

	if (SepPos == 0 && Path.size() == 1) return string(1, cSep); // 路径是根目录	

	return Path.substr(SepPos + 1);
}

/**
 * 判断字符串是否以另一字符串结尾
 *
 * @param pSrc 做判断的字符串
 * @param pEnd 结尾字符串
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
 * 替换字符串
 *
 * @param src 原字符串
 * @param pOldString 需要替换的字符串
 * @param pNewString 新字符串
 * @return 替换后的字符串
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
