#pragma once

#if defined( _MSC_VER ) && defined( _DEBUG )
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#define HAS_BITS( v, b ) ( ((v)&(b)) == (b) )
#define GET_BITS( v, b ) ( (v) & (b) )
#define SET_BITS( v, b ) ( (v) |= (b) )
#define UNSET_BITS( v, b ) ( (v) &= ~(b) )

#define MAKE_INT64( l, h ) ( ((INT64)(h)<<32) | (l) )

#if _MSC_VER >= 1400 // 编译器是Microsoft C/C++ 14.0或更高版本
// 使用新的随机函数
#define _CRT_RAND_S
#endif

#if !defined( __STDC_VERSION__ ) && !defined( __CYGWIN__ )
#if __GNUC__ >= 4 // 支持C99的编译器
#define __STDC_VERSION__ 199901L
#endif
#endif

#ifndef _WIN32
#define _FILE_OFFSET_BITS 64
#endif

// 包含标准头文件 -->
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <wctype.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>


#ifdef _WIN32
#include <direct.h>
#include <io.h>
#elif _LINUX
#include <stdarg.h>
#include <sys/stat.h>
#endif

#include <memory>
#if defined( __GNUC__ )
#include <tr1/memory>
#endif
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <stdexcept>
using namespace std;
using namespace std::tr1;
// <-- 包含标准头文件

// 包含系统头文件 -->
#if defined( _WIN32 ) // 操作系统是Windows

#ifdef _MSC_VER
#include <process.h>
#endif
#include <winsock2.h>
#include <ws2ipdef.h>
#define PATH_SEP _T("\\")

#else

#define PATH_SEP _T("/")
#define MAX_PATH 1024

#if defined( __GNUC__ ) // 编译器是GCC
#ifndef __USE_GNU
// 使用GNU扩展特性
#define __USE_GNU
#endif
#endif

#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/syscall.h>  
#define gettid() syscall(__NR_gettid) 

#endif
// <-- 包含系统头文件

#if defined( _MSC_VER ) // 编译器是Microsoft C/C++
#define __THREAD __declspec(thread)
#define __INLINE inline
#elif defined( __GNUC__ ) // 编译器是GCC
#define __THREAD __thread
#define __INLINE inline
#else
#define __THREAD
#define __INLINE
#endif

#if !defined( _WINDEF_ ) // 没有包含Windows SDK的类型定义头文件

typedef unsigned long long UINT64;
typedef long long INT64;
typedef unsigned int UINT32;
typedef int INT32;
typedef unsigned short UINT16;
typedef short INT16;
typedef unsigned char UINT8;
typedef signed char INT8;

#if !defined( __GNUC__ ) || !defined( _WINDEF_H ) // 编译器不是GCC或者操作系统不是Windows

typedef UINT32 DWORD;
typedef UINT16 WORD;
typedef UINT8 BYTE;

typedef char CHAR;
typedef wchar_t WCHAR;
#ifdef _UNICODE
typedef WCHAR TCHAR
#else
typedef CHAR TCHAR;
#endif // _UNICODE

#ifdef LHQ_X64
typedef UINT64 UINT_PTR;
typedef INT64 INT_PTR;
#else
typedef UINT32 UINT_PTR;
typedef INT32 INT_PTR;
#endif // LHQ_X64

#endif // __GNUC__

#endif // _WINDEF_

#ifndef _MSC_VER
// 定义兼容Microsoft命名的字符串函数
#define stricmp strcasecmp
#define strnicmp strncasecmp
#define wcsicmp wcscasecmp
#define wcsnicmp wcsncasecmp
#endif

#if _MSC_VER >= 1400 // 编译器是Microsoft C/C++ 14.0或更高版本
// 定义兼容ANSI C命名的函数
#define snprintf _snprintf
#endif // _MSC_VER

#ifdef _MSC_VER // 编译器是Microsoft C/C++
#define strtoull _strtoui64
#define strtoll _strtoi64
#define wcstoull _wcstoui64
#define wcstoll _wcstoi64
#define localtime_r( t, tm ) ( localtime_s((tm), (t)) == 0 )
#endif // _MSC_VER

#ifdef UNICODE
#define _T( s ) L##s
#define TCHAR_LEN( t ) ( sizeof(t) / sizeof(wchar_t) )
#define TSTR_FLD "%ls"
typedef std::wstring tstring;

#define tcscpy wcscpy
#define tcsncpy wcsncpy
#define tcslen wcslen
#define tcscmp wcscmp
#define tcsicmp wcsicmp
#define tcsstr wcsstr
#define tcschr wcschr
#define tcsrchr wcsrchr
#define tcstol wcstol
#define tcstoll wcstoll
#define tcstoull wcstoull
#define tsprintf swprintf
#else
#define _T( s ) s
#define TCHAR_LEN( t ) sizeof(t)
#define TSTR_FLD "%s"
typedef std::string tstring;

#define tcscpy strcpy
#define tcsncpy strncpy
#define tcslen strlen
#define tcscmp strcmp
#define tcsicmp stricmp
#define tcsstr strstr
#define tcschr strchr
#define tcsrchr strrchr
#define tcstol strtol
#define tcstoll strtoll
#define tcstoull strtoull
#define tsprintf snprintf
#endif // UNICODE

#ifdef _WIN32
#define ACCESS _access
#define MKDIR(d) _mkdir((d))
#elif _LINUX
#define ACCESS access
#define MKDIR(d) mkdir((d), 0755)
#endif

#ifndef _WINSOCKAPI_
#ifdef _WIN32
typedef UINT_PTR SOCKET;
#else
typedef int SOCKET;
#endif // _WIN32
#define INVALID_SOCKET ((SOCKET)-1)
#endif // _WINSOCKAPI_

#ifdef _WIN32
#define SLEEP( msec ) Sleep( msec )
typedef int socklen_t;
#define CLOSE_SOCKET closesocket
#else
#define SLEEP( msec ) usleep( msec * 1000LL )
#define CLOSE_SOCKET close
#endif

/**
* 忽略大小写比较字符串的函数对象。
*/
struct str_less_incase : public binary_function<string, string, bool>
{
	bool operator()(const string&Left, const string &Right) const
	{
		return stricmp(Left.c_str(), Right.c_str()) < 0;
	}
};

/**
* 忽略大小写比较字符串的函数对象。
*/
struct wstr_less_incase : public binary_function<wstring, wstring, bool>
{
	bool operator()(const wstring&Left, const wstring &Right) const
	{
		return wcsicmp(Left.c_str(), Right.c_str()) < 0;
	}
};

#ifdef UNICODE
#define tstr_less_incase wstr_less_incase
#else
#define tstr_less_incase str_less_incase
#endif

/**
 * 不可复制的类。
 */
class CNonCopyable
{
protected:
	CNonCopyable() {}
	virtual ~CNonCopyable() {}
	
private:
	CNonCopyable( const CNonCopyable & ) {}
	
	const CNonCopyable & operator=( const CNonCopyable & )
	{
		return *this;
	}
}; // CNonCopyable
