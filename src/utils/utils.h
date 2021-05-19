#pragma once

#include "base.h"

/**
 * 获得当前线程最后产生的操作系统错误码。
 */
#ifdef _WIN32
__INLINE int GetOsError() { return (int)GetLastError(); }
#else
__INLINE int GetOsError() { return errno; }
#endif

/**
 * 设置当前线程最后产生的操作系统错误码。
 */
#ifdef _WIN32
__INLINE void SetOsError(int iOsError) { SetLastError((DWORD)iOsError); }
#else
__INLINE void SetOsError(int iOsError) { errno = iOsError; }
#endif

/**
 * 获得指定操作系统错误码表示的窄字符文本信息。
 *
 * @param iOsError 操作系统的错误码，如果为0则表示当前线程最后产生的错误。
 * @return 如果执行成功则为错误码的文本信息，如果执行失败则为空字符串。
 */
string GetOsErrorMessageA(int iOsError = 0);

/**
 * 获得指定操作系统错误码表示的宽字符文本信息。
 *
 * @param iOsError 操作系统的错误码，如果为0则表示当前线程最后产生的错误。
 * @return 如果执行成功则为错误码的文本信息，如果执行失败则为空字符串。
 */
wstring GetOsErrorMessageW(int iOsError = 0);

#ifdef UNICODE
#define GetOsErrorMessage GetOsErrorMessageW
#else
#define GetOsErrorMessage GetOsErrorMessageA
#endif

/**
 * 获得当前时区与格林威治之间相差的分钟数。<br>
 * 本地时区时间加上该值能够得到对应的格林威治时间。
 */
INT32 GetTimeZoneBias();

/**
 * 获得从操作系统启动开始到当前的微秒数。
 */
INT64 GetRealClock();

/**
 * 产生一个32位的伪随机数。
 *
 * @param iMin 可产生的随机数最小值。
 * @param iMax 可产生的随机数最大值。
 * @return 产生的随机数。
 */
INT32 RandomInt32(INT32 iMin = -2147483647 - 1, INT32 iMax = 2147483647);

/**
 * 产生一个64位的伪随机数。
 *
 * @param iMin 可产生的随机数最小值。
 * @param iMax 可产生的随机数最大值。
 * @return 产生的随机数。
 */
INT64 RandomInt64(INT64 iMin = -9223372036854775807LL - 1, INT64 iMax = 9223372036854775807LL);

/**
 * 使用更可靠的方法产生一个32位的随机数。
 *
 * @param iMin 可产生的随机数最小值。
 * @param iMax 可产生的随机数最大值。
 * @return 产生的随机数。
 * @note 这个函数的运行速度要慢于RandomInt32。
 */
INT32 RandomInt32Ex(INT32 iMin = -2147483647 - 1, INT32 iMax = 2147483647);

/**
 * 使用更可靠的方法产生一个64位的随机数。
 *
 * @param iMin 可产生的随机数最小值。
 * @param iMax 可产生的随机数最大值。
 * @return 产生的随机数。
 * @note 这个函数的运行速度要慢于RandomInt64。
 */
INT64 RandomInt64Ex(INT64 iMin = -9223372036854775807LL - 1, INT64 iMax = 9223372036854775807LL);

/**
 * 对32位整数执行原子的加法操作。
 *
 * @param[in,out] pAddend 指向一个INT32类型的指针，存放一个用于相加的数值。在函数返回时它指向的内容将被设置为操作的结果。
 * @param iValue 另一个用于相加的数值。
 * @return 函数执行之前pAddend存放的值。
 */
__INLINE INT32 AtomicAdd32(volatile INT32 *pAddend, INT32 iValue)
{
#ifdef _WIN32
	return InterlockedExchangeAdd((volatile LONG *)pAddend, iValue);
#elif __GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ >= 40102 // 编译器是GCC 4.1.2或更高版本
	return __sync_fetch_and_add(pAddend, iValue);
#elif defined( __GNUC__ ) && ( defined(LHQ_I386) || defined(LHQ_X64) ) // 编译器是GCC并且是Intel平台
	INT32 iResult;

	asm volatile
		(
			"lock\n\t"
			"xaddl %1, %0":
			"+m"(*pAddend), "=r"(iResult) :
			"1"(iValue) :
			"memory", "cc"
		);

	return iResult;
#else
#error Platform not supported!
#endif
}

/**
 * 对32位整数执行原子的递增操作。
 *
 * @param[in,out] pAddend 指向一个INT32类型的指针，存放用于递增的值。在函数返回时它指向的内容将被设置为操作的结果。
 * @return 操作的结果。
 */
__INLINE INT32 AtomicInc32(volatile INT32 *pAddend)
{
#ifdef _WIN32
	return InterlockedIncrement((volatile LONG *)pAddend);
#elif __GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ >= 40102 // 编译器是GCC 4.1.2或更高版本
	return __sync_add_and_fetch(pAddend, 1);
#else
	return AtomicAdd32(pAddend, 1) + 1;
#endif
}

/**
 * 对32位整数执行原子的递减操作。
 *
 * @param[in,out] pAddend 指向一个INT32类型的指针，存放用于递减的值。在函数返回时它指向的内容将被设置为操作的结果。
 * @return 操作的结果。
 */
__INLINE INT32 AtomicDec32(volatile INT32 *pAddend)
{
#ifdef _WIN32
	return InterlockedDecrement((volatile LONG *)pAddend);
#elif __GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ >= 40102 // 编译器是GCC 4.1.2或更高版本
	return __sync_add_and_fetch(pAddend, -1);
#else
	return AtomicAdd32(pAddend, -1) - 1;
#endif
}

/**
 * 对32位整数执行原子的赋值操作。
 *
 * @param[in,out] pDestination 指向一个INT32类型的指针，存放要比较的目标值。
 * @param iExchange 要赋予的值。
 * @return 函数执行之前pDestination存放的值。
 */
__INLINE INT32 AtomicExchange32(volatile INT32 *pDestination, INT32 iExchange)
{
#ifdef _WIN32
	return InterlockedExchange((volatile LONG *)pDestination, iExchange);
#elif defined( __GNUC__ ) && ( defined(LHQ_I386) || defined(LHQ_X64) ) // 编译器是GCC并且是Intel平台
	INT32 iPrev = iExchange;

	asm volatile(
		"xchgl %3,%1"
		: "=a" (iPrev), "=m" (*(pDestination))
		: "0" (iPrev), "r" (iExchange)
		: "memory", "cc"
		);

	return iPrev;
#else
	INT32 iTmp = *pDestination;
	*pDestination = iExchange;
	return iTmp;
#endif
}

/**
 * 对32位整数执行原子的比较和赋值操作。
 *
 * @param[in,out] pDestination 指向一个INT32类型的指针，存放要比较的目标值。
 * @param iExchange 如果pDestination与Comparand相等，这个参数的值将被赋予pDestination。
 * @param Comparand 用于比较的值。
 * @return 函数执行之前pDestination存放的值。
 */
__INLINE INT32 AtomicCompareExchange32(volatile INT32 *pDestination, INT32 iExchange, INT32 iComparand)
{
#ifdef _WIN32
	return InterlockedCompareExchange((volatile LONG *)pDestination, iExchange, iComparand);
#elif __GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ >= 40102 // 编译器是GCC 4.1.2或更高版本
	return __sync_val_compare_and_swap(pDestination, iComparand, iExchange);
#elif defined( __GNUC__ ) && ( defined(LHQ_I386) || defined(LHQ_X64) ) // 编译器是GCC并且是Intel平台
	INT32 iPrev = iComparand;

	asm volatile(
		"lock\n\t"
		"cmpxchgl %3,%1"
		: "=a" (iPrev), "=m" (*(pDestination))
		: "0" (iPrev), "r" (iExchange)
		: "memory", "cc"
		);

	return iPrev;
#else
#error Platform not supported!
#endif
}

/**
 * 对64位整数执行原子的加法操作。
 *
 * @param[in,out] pAddend 指向一个INT64类型的指针，存放一个用于相加的数值。在函数返回时它指向的内容将被设置为操作的结果。
 * @param iValue 另一个用于相加的数值。
 * @return 函数执行之前pAddend存放的值。
 */
__INLINE INT64 AtomicAdd64(volatile INT64 *pAddend, INT64 iValue)
{
#if __GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ >= 40102 // 编译器是GCC 4.1.2或更高版本
	return __sync_fetch_and_add(pAddend, iValue);
#elif defined( _MSC_VER )
	INT64 iOld;
	do
	{
		iOld = *pAddend;
	} while (_InterlockedCompareExchange64(pAddend, iOld + iValue, iOld) != iOld);

	return iOld;
#else
#error Platform not supported!
	/* 由于时间关系先用愚蠢的办法实现原子操作
	static lhq::CMutex Mutex[10];

	lhq::CMutexGuard Lock( Mutex[(UINT_PTR)pAddend%10] );
	INT64 iPrevValue = *pAddend;
	*pAddend += iValue;
	return iPrevValue;
	*/
#endif
}

/**
 * 对64位整数执行原子的递增操作。
 *
 * @param[in,out] pAddend 指向一个INT64类型的指针，存放用于递增的值。在函数返回时它指向的内容将被设置为操作的结果。
 * @return 操作的结果。
 */
__INLINE INT64 AtomicInc64(volatile INT64 *pAddend)
{
#if __GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ >= 40102 // 编译器是GCC 4.1.2或更高版本
	return __sync_add_and_fetch(pAddend, 1);
#elif defined( _MSC_VER )
	INT64 iOld;
	do
	{
		iOld = *pAddend;
	} while (_InterlockedCompareExchange64(pAddend, iOld + 1, iOld) != iOld);

	return iOld + 1;
#else
	return AtomicAdd64(pAddend, 1) + 1;
#endif
}

/**
 * 对64位整数执行原子的递减操作。
 *
 * @param[in,out] pAddend 指向一个INT64类型的指针，存放用于递减的值。在函数返回时它指向的内容将被设置为操作的结果。
 * @return 操作的结果。
 */
__INLINE INT64 AtomicDec64(volatile INT64 *pAddend)
{
#if __GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ >= 40102 // 编译器是GCC 4.1.2或更高版本
	return __sync_add_and_fetch(pAddend, -1);
#elif defined( _MSC_VER )
	INT64 iOld;
	do
	{
		iOld = *pAddend;
	} while (_InterlockedCompareExchange64(pAddend, iOld - 1, iOld) != iOld);

	return iOld - 1;
#else
	return AtomicAdd32(pAddend, -1) - 1;
#endif
}

/**
 * 把二进制数据转换为表现为十六进制数形式的文本。
 *
 * @param pData 要转换的数据。
 * @param nDataSize 要转换的数据字节数。
 * @param[out] pBuffer 用于存放文本的缓冲区。
 * @param nMaxBuffer 用于存放文本的缓冲区最大可存储字节数，包括字符串末尾的空终止符。
 *                       这个参数的值应该大于nDataSize的两倍。
 * @param bWithLowerCase 如果要得到使用小写字母的结果则为true。
 */
template<typename T> __INLINE void BinToHex(const void *pData, size_t nDataSize, T *pBuffer, size_t nMaxBuffer, bool bWithLowerCase = false)
{
	// 表示十六进制数字符的数组。
	static const char cDigitsU[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	static const char cDigitsL[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

	const char *pDigits = bWithLowerCase ? cDigitsL : cDigitsU;

	if (nMaxBuffer < 1) return;

	UINT32 i = 0, j = 0;
	for (; i < nDataSize; i++)
	{
		if (nMaxBuffer - j <= 2) break;

		pBuffer[j++] = pDigits[(((BYTE *)pData)[i] >> 4) & 0xF];
		pBuffer[j++] = pDigits[((BYTE *)pData)[i] & 0xF];
	}

	pBuffer[j] = 0;
}

/**
 * 把二进制数据转换为表现为十六进制数形式的文本。
 *
 * @param pData 要转换的数据。
 * @param nDataSize 要转换的数据字节数。
 * @param bWithLowerCase 如果要得到使用小写字母的结果则为true。
 * @return 转换的结果。
 */
__INLINE string BinToHexA(const void *pData, size_t nDataSize, bool bWithLowerCase = false)
{
	string Result(nDataSize * 2 + 1, 0);
	BinToHex(pData, nDataSize, &Result[0], Result.size(), bWithLowerCase);
	Result.resize(nDataSize * 2);
	return Result;
}

/**
 * 把二进制数据转换为表现为十六进制数形式的文本。
 *
 * @param pData 要转换的数据。
 * @param nDataSize 要转换的数据字节数。
 * @param bWithLowerCase 如果要得到使用小写字母的结果则为true。
 * @return 转换的结果。
 */
__INLINE wstring BinToHexW(const void *pData, size_t nDataSize, bool bWithLowerCase = false)
{
	wstring Result(nDataSize * 2 + 1, 0);
	BinToHex(pData, nDataSize, &Result[0], Result.size(), bWithLowerCase);
	Result.resize(nDataSize * 2);
	return Result;
}

/**
 * 把表现为十六进制数形式的文本转换为二进制数据。
 *
 * @param pHexText 要转换的文本。
 * @param[out] pBuffer 用于存放二进制数据的缓冲区。
 * @param nMaxBuffer 用于存放二进制数据的缓冲区最大可存储字节数。
 * @return 转换得到的二进制数据字节数。
 */
template<typename T> __INLINE UINT32 HexToBin(const T *pHexText, BYTE *pBuffer, UINT32 nMaxBuffer)
{
	if (pHexText == NULL || nMaxBuffer < 1) return 0;

	UINT32 i = 0, j = 0;
	for (; j < nMaxBuffer; j++)
	{
		if (pHexText[i] == 0) break;

		if (pHexText[i] >= '0' && pHexText[i] <= '9') pBuffer[j] = (pHexText[i] - '0') << 4;
		else if (pHexText[i] >= 'a' && pHexText[i] <= 'f') pBuffer[j] = (pHexText[i] - 'a' + 10) << 4;
		else if (pHexText[i] >= 'A' && pHexText[i] <= 'F') pBuffer[j] = (pHexText[i] - 'A' + 10) << 4;
		else break;
		i++;

		if (pHexText[i] == 0) break;

		if (pHexText[i] >= '0' && pHexText[i] <= '9') pBuffer[j] |= (pHexText[i] - '0');
		else if (pHexText[i] >= 'a' && pHexText[i] <= 'f') pBuffer[j] |= (pHexText[i] - 'a' + 10);
		else if (pHexText[i] >= 'A' && pHexText[i] <= 'F') pBuffer[j] |= (pHexText[i] - 'A' + 10);
		else break;
		i++;
	}

	return j;
}

/**
 * 从流中读取一行内容，并截断尾部空格。
 */
template<class _Elem, class _Traits, class _Alloc> __INLINE basic_istream<_Elem, _Traits> &GetLineTe(
	basic_istream<_Elem, _Traits>& _Istr, basic_string<_Elem, _Traits, _Alloc>& _Str, const _Elem _Delim = '\n')
{
	basic_istream<_Elem, _Traits> &Result = getline(_Istr, _Str, _Delim);
	while (!_Str.empty() && isspace(_Str[_Str.size() - 1])) _Str.resize(_Str.size() - 1);
	return Result;
}

/**
 * 用指定的分隔符分割字符串为前后两部分。
 *
 * @param[out] First 分割出的前半部分。
 * @param[out] Second 分割出的后半部分。
 * @param pStr 要分割的字符串。
 * @param nStrLen 要分割的字符串长度，如果为-1则表示直到空终止符。
 * @param cSep 分隔符。
 * @param bTrim 如果要对分割出的部分截断头尾空格则为true。
 */
template<typename T> __INLINE void DimidiateString(basic_string<T> &First, basic_string<T> &Second, const T *pStr, int nStrLen = -1, T cSep = ',', bool bTrim = true)
{
	vector< basic_string<T> > Result;
	SpliteString(Result, pStr, nStrLen, cSep, bTrim, 2);

	if (Result.size() >= 1) First = Result[0];
	if (Result.size() >= 2) Second = Result[1];
}

/**
 * 用指定的分隔符分割字符串。
 *
 * @param[out] Result 存放分割结果。
 * @param pStr 要分割的字符串。
 * @param nStrLen 要分割的字符串长度，如果为-1则表示直到空终止符。
 * @param cSep 分隔符。
 * @param bTrim 如果要对分割出的部分截断头尾空格则为true。
 * @param nMaxCount 最大分割数量，如果为-1则表示无限。
 */
template<typename T> __INLINE void SpliteString(vector< basic_string<T> > &Result, const T *pStr, int nStrLen = -1, T cSep = ',', bool bTrim = true, int nMaxCount = -1)
{
	Result.clear();

	if (nMaxCount == 0) return;

	const T *pEnd = (nStrLen == -1 ? NULL : pStr + nStrLen);
	while (*pStr != 0 && (pEnd == NULL || pStr < pEnd))
	{
		if (bTrim && *pStr == ' ')
		{
			pStr++;
			continue;
		}

		const T *pSep;
		if (nMaxCount > -1 && (int)Result.size() + 1 >= nMaxCount)
		{
			pSep = NULL;
		}
		else
		{
			pSep = pStr;
			while (*pSep != cSep)
			{
				pSep++;
				if (*pSep == 0 || (pEnd != NULL && pSep >= pEnd))
				{
					pSep = NULL;
					break;
				}
			}
		}

		basic_string<T> Item;

		if (pSep == NULL) Item.assign(pStr);
		else Item.assign(pStr, pSep);

		while (bTrim && !Item.empty() && Item[Item.size() - 1] == ' ') Item.resize(Item.size() - 1);

		if (!Item.empty()) Result.push_back(Item);

		if (pSep == NULL) break;
		pStr = pSep + 1;
	}
}

/**
 * 获得本地计算机的主机名。
 */
tstring GetHostName();

/**
 * 获得本地计算机所在的域或工作组名称。
 */
tstring GetDomainName();

/**
 * 获得操作系统名称。
 */
tstring GetOsName();

/**
 * 判断一段数据的内容是否全部为0值。
 *
 * @param pData 存放数据的缓冲区。
 * @param nSize 数据的字节数。
 * @return 如果数据的内容全部为0值则为true，否则为false。
 */
bool DataIsZeros(const void *pData, UINT32 nSize);

/**
 * 用CRC32算法计算出数据的4个字节散列值。
 *
 * @param pData 要计算的数据。
 * @param nDataSize 要计算的数据字节数。
 * @return 计算的结果。
 * @author ksaiy
 * @note 代码摘自<a href="http://blog.csdn.net/ksaiy/archive/2005/05/27/382162.aspx">获取文件的CRC32值(VC++源码-固定码表)。。。。。</a>。
 */
UINT32 CRC32(const BYTE *pData, size_t nDataSize);

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
UINT32 MurmurHash32(const void *pData, int nDataSize, unsigned int uiSeed);

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
UINT64 MurmurHash64(const void *pData, int nDataSize, unsigned int uiSeed);

/**
 * 用MD4算法计算出数据的16个字节散列值。
 *
 * @param pData 要计算的数据。
 * @param nDataSize 要计算的数据字节数。
 * @param[out] pMessageDigest 存储16个字节散列值的缓冲区。
 */
void MD4Hash(const void *pData, size_t nDataSize, BYTE *pMessageDigest);

/**
 * 用MD5算法计算出数据的16个字节散列值。
 *
 * @param pData 要计算的数据。
 * @param nDataSize 要计算的数据字节数。
 * @param[out] pMessageDigest 存储16个字节散列值的缓冲区。
 */
void MD5Hash(const void *pData, size_t nDataSize, BYTE *pMessageDigest);

/**
 * 用MD5算法计算出数据的哈希值并得到十六进制字符串。
 *
 * @param pData 要计算的数据。
 * @param nDataSize 要计算的数据字节数。
 * @param bWithLowerCase 如果要返回小写字符串则为true。
 * @return 哈希值的十六进制字符串。
 */
string MD5Hash(const void *pData, size_t nDataSize, bool bWithLowerCase = true);

/**
 * 用HMAC MD5算法计算数据的16个字节散列值。
 *
 * @param pKey 加密使用的关键字。
 * @param nKeySize 加密使用的关键字字节数。
 * @param pData 要计算的数据。
 * @param nDataSize 要计算的数据字节数。
 * @param[out] pMessageDigest 存储散列值的缓冲区，容量不能小于16字节。
 */
void HMACMD5(const void *pKey, size_t nKeySize, const void *pData, size_t nDataSize, BYTE *pMessageDigest);

/**
 * 用SHA1算法计算数据的20个字节散列值。
 *
 * @param pData 要计算的数据。
 * @param nDataSize 要计算的数据字节数。
 * @param[out] pMessageDigest 存储散列值的缓冲区，容量不能小于20字节。
 */
void SHA1Hash(const void *pData, size_t nDataSize, BYTE *pMessageDigest);

/**
 * 用HMAC SHA1算法计算数据的20个字节散列值。
 *
 * @param pKey 加密使用的关键字。
 * @param nKeySize 加密使用的关键字字节数。
 * @param pData 要计算的数据。
 * @param nDataSize 要计算的数据字节数。
 * @param[out] pMessageDigest 存储散列值的缓冲区，容量不能小于20字节。
 */
void HMACSHA1(const void *pKey, size_t nKeySize, const void *pData, size_t nDataSize, BYTE *pMessageDigest);

/**
 * Base64编码。
 *
 * @param pData 要编码的数据内容。
 * @param nDataSize 要编码的数据字节数。
 * @param[out] pBuffer 存放编码内容的缓冲区，包含字符串末尾的空终止符。容量不能小于 4.0 * ceil(nDataSize / 3.0)。
 * @param[out] nBufferSize 缓冲区的容量。
 * @return 如果执行成功则为true，如果缓冲区不足则为false。
 */
bool Base64Encode(const void *pData, size_t nDataSize, char *pBuffer, size_t nBufferSize);

/**
 * Base64编码。
 *
 * @param pData 要编码的数据内容。
 * @param nDataSize 要编码的数据字节数。
 * @return 如果执行成功则为编码结果，否则为空内容。
 */
string Base64Encode(const void *pData, size_t nDataSize);

/**
 * Base64解码。
 *
 * @param pData 要解码的数据内容。
 * @param nDataSize 要解码的数据字节数。
 * @param[out] pBuffer 存放已解码内容的缓冲区，容量不能小于 nDataSize / 4 * 3。
 * @param[in,out] nBufferSize 在函数执行时表示缓冲区的容量，在函数返回时用于存放已解码内容的字节数。
 * @return 如果执行成功则为true，如果要解码的数据无效或者缓冲区不足则为false。
 */
bool Base64Decode(const char *pData, size_t nDataSize, void *pBuffer, size_t &nBufferSize);

#ifndef _LHQ_NO_DES_FUNC
/**
 * 用DES算法进行加密或解密。
 *
 * @param bDoEncrypt 如果要进行加密则为true，如果要进行解密则为false。
 * @param pIn 当bDoEncrypt为true时，这个参数为要加密的数据。
 *                当bDoEncrypt为false时，这个参数为要解密的数据。
 * @param nInSize pIn的数据字节数。
 * @param[out] pOut 当bDoEncrypt为true时，这个参数用于存放加密的结果。
 *                  当bDoEncrypt为false时，这个参数用于存放解密的结果。
 * @param nMaxOut pOut能够存放的最大数据字节数，必须为8的倍数并且不能小于nInSize。
 * @param pKey 加密用的8字节关键字。
 *
 * @note DES算法操作的数据字节数必须为8的倍数。<br>
 *       在加密时，如果pIn提供的加密数据字节数不是8的倍数，函数会自动在后面补充诺干0值对齐8字节然后进行加密。<br>
 *       在解密时，如果pIn提供的解密数据字节数不是8的倍数，将会导致函数执行失败。
 */
void DesEncrypt(bool bDoEncrypt, const BYTE *pIn, UINT32 nInSize, BYTE *pOut, UINT32 nMaxOut, const BYTE *pKey);
#endif

/**
 * 判断当前系统字符集是否为UTF-8。
 */
bool WithUtf8();

/**
 * 获得系统默认的字符集编码名称。
 *
 * @param[out] pBuffer 存放字符集编码名称的缓冲区。
 * @param nMaxBuffer 存放字符集编码名称的缓冲区最大可存储字符数，包括字符串末尾的空终止符。
 * @return 如果执行成功则为true，否则为false。
 */
bool GetSystemCharset(char *pBuffer, int nMaxBuffer);

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
int StrIconv(const char *pSrcCharset, const char *pSrc, int nSrcSize, const char *pDestCharset, char *pBuffer, int nMaxBuffer);

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
int AnsiStrToUtf8Str(const char *pSrc, int nSrcLen, char *pBuffer, int nMaxBuffer, const char *pSrcCharset = NULL);

/**
 * 把由char类型字符组成的字符串转换为按UTF-8编码的string对象。
 *
 * @param pSrc 要转换的字符串。
 * @param nSrcLen 要转换的字符数，如果为-1则表示转换直到空终止符为止。
 * @param pSrcCharset 字符串的字符集编码，如果为NULL或者空内容则表示系统默认值。
 * @return 如果执行成功则为转换的结果，如果没有内容可转换或者转换失败则为空字符串。
 */
string AnsiStrToUtf8Str(const char *pSrc, int nSrcLen = -1, const char *pSrcCharset = NULL);

/**
 * 把由wchar_t类型字符组成的字符串转换为由char类型字符组成并且按UTF-8编码的字符串。
 *
 * @param pSrc 要转换的字符串。
 * @param nSrcLen 要转换的字符数，如果为-1则表示转换直到空终止符为止。
 * @param[out] pBuffer 存放转换结果的缓冲区。
 * @param nMaxBuffer 存放转换结果的缓冲区最大可存储字符数。
 * @return 如果执行成功则为转换结果的字符数。如果出现错误或者缓冲区太小则为-1。
 */
int WideStrToUtf8Str(const wchar_t *pSrc, int nSrcLen, char *pBuffer, int nMaxBuffer);

/**
 * 把由wchar_t类型字符组成的字符串转换为按UTF-8编码的string对象。
 *
 * @param pSrc 要转换的字符串。
 * @param nSrcLen 要转换的字符数，如果为-1则表示转换直到空终止符为止。
 * @return 如果执行成功则为转换的结果，如果没有内容可转换或者转换失败则为空字符串。
 */
string WideStrToUtf8Str(const wchar_t *pSrc, int nSrcLen = -1);

/**
 * 把由本地字符环境的字符串转换为按UTF-8编码的string对象。
 */
#ifdef UNICODE
#define TStrToUtf8Str WideStrToUtf8Str
#else
#define TStrToUtf8Str AnsiStrToUtf8Str
#endif

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
int Utf8StrToAnsiStr(const char *pSrc, int nSrcLen, char *pBuffer, int nMaxBuffer, const char *pDestCharset = NULL);

/**
 * 把由char类型字符组成并且按UTF-8编码的字符串转换为string对象。
 *
 * @param pSrc 要转换的字符串。
 * @param nSrcLen 要转换的字符数，如果为-1则表示转换直到空终止符为止。
 * @param pDestCharset 目标字符串的字符集编码，如果为NULL或者空内容则表示系统默认值。
 * @return 如果执行成功则为转换的结果，如果没有内容可转换或者转换失败则为空字符串。
 */
string Utf8StrToAnsiStr(const char *pSrc, int nSrcLen = -1, const char *pDestCharset = NULL);

/**
 * 把由char类型字符组成并且按UTF-8编码的字符串转换为由wchar_t类型字符组成的字符串。
 *
 * @param pSrc 要转换的字符串。
 * @param nSrcLen 要转换的字符数，如果为-1则表示转换直到空终止符为止。
 * @param[out] pBuffer 存放转换结果的缓冲区。
 * @param nMaxBuffer 存放转换结果的缓冲区最大可存储字符数。
 * @return 如果执行成功则为转换结果的字符数。如果出现错误或者缓冲区太小则为-1。
 */
int Utf8StrToWideStr(const char *pSrc, int nSrcLen, wchar_t *pBuffer, int nMaxBuffer);

/**
 * 把由char类型字符组成并且按UTF-8编码的字符串转换为wstring对象。
 *
 * @param pSrc 要转换的字符串。
 * @param nSrcLen 要转换的字符数，如果为-1则表示转换直到空终止符为止。
 * @return 如果执行成功则为转换的结果，如果没有内容可转换或者转换失败则为空字符串。
 */
wstring Utf8StrToWideStr(const char *pSrc, int nSrcLen = -1);

/**
 * 把由char类型字符组成并且按UTF-8编码的字符串转换为tstring对象。
 */
#ifdef UNICODE
#define Utf8StrToTStr Utf8StrToWideStr
#else
#define Utf8StrToTStr Utf8StrToAnsiStr
#endif

/**
 * 把由char类型字符组成的字符串转换为由wchar_t类型字符组成的字符串。
 *
 * @param pSrc 要转换的字符串。
 * @param nSrcLen 要转换的字符数，如果为-1则表示转换直到空终止符为止。
 * @param[out] pBuffer 存放转换结果的缓冲区。
 * @param nMaxBuffer 存放转换结果的缓冲区最大可存储字符数。
 * @return 如果执行成功则为转换结果的字符数。如果出现错误或者缓冲区太小则为-1。
 */
int AnsiStrToWideStr(const char *pSrc, int nSrcLen, wchar_t *pBuffer, int nMaxBuffer);

/**
 * 把由char类型字符组成的字符串转换为wstring对象。
 *
 * @param pSrc 要转换的字符串。
 * @param nSrcLen 要转换的字符数，如果为-1则表示转换直到空终止符为止。
 * @return 如果执行成功则为转换的结果，如果没有内容可转换或者转换失败则为空字符串。
 */
wstring AnsiStrToWideStr(const char *pSrc, int nSrcLen = -1);

/**
 * 把由wchar_t类型字符组成的字符串转换为由char类型字符组成的字符串。
 *
 * @param pSrc 要转换的字符串。
 * @param nSrcLen 要转换的字符数，如果为-1则表示转换直到空终止符为止。
 * @param[out] pBuffer 存放转换结果的缓冲区。
 * @param nMaxBuffer 存放转换结果的缓冲区最大可存储字符数。
 * @return 如果执行成功则为转换结果的字符数。如果出现错误或者缓冲区太小则为-1。
 */
int WideStrToAnsiStr(const wchar_t *pSrc, int nSrcLen, char *pBuffer, int nMaxBuffer);

/**
 * 把由wchar_t类型字符组成的字符串转换为string对象。
 *
 * @param pSrc 要转换的字符串。
 * @param nSrcLen 要转换的字符数，如果为-1则表示转换直到空终止符为止。
 * @return 如果执行成功则为转换的结果，如果没有内容可转换或者转换失败则为空字符串。
 */
string WideStrToAnsiStr(const wchar_t *pSrc, int nSrcLen = -1);

/**
 * 格式化一个字符串。
 *
 * @param pFormat 格式化信息，必须符合ANSI C的格式化规范。
 * @param ... 格式化的参数。
 * @return 存放格式化结果的string对象。
 */
string FormatStr(const char *pFormat, ...);

/**
 * 格式化一个字符串。
 *
 * @param pFormat 格式化信息，必须符合ANSI C的格式化规范。
 * @param pArgs 格式化的参数。
 * @return 存放格式化结果的string对象。
 */
string VFormatStr(const char *pFormat, va_list pArgs);

/**
 * 格式化一个宽字符串。
 *
 * @param pFormat 格式化信息，必须符合ANSI C的格式化规范。
 * @param ... 格式化的参数。
 * @return 存放格式化结果的wstring对象。
 */
wstring FormatStr(const wchar_t *pFormat, ...);

/**
 * 格式化一个宽字符串。
 *
 * @param pFormat 格式化信息，必须符合ANSI C的格式化规范。
 * @param pArgs 格式化的参数。
 * @return 存放格式化结果的wstring对象。
 */
wstring VFormatStr(const wchar_t *pFormat, va_list pArgs);

/**
 * 获得一个路径的目录部分。
 *
 * @param Path 路径。
 * @param cSep 路径中的分隔符。
 * @return 路径的目录部分。
 */
wstring GetDirectoryFromPathW(const wstring &Path, const wchar_t cSep = L'/');

/**
 * 获得一个路径的文件名部分。
 *
 * @param Path 路径。
 * @param cSep 路径中的分隔符。
 * @return 路径的文件名部分。
 */
wstring GetNameFromPathW(const wstring &Path, const wchar_t cSep = L'/');

/**
 * 获得一个路径的目录部分。
 *
 * @param Path 路径。
 * @param cSep 路径中的分隔符。
 * @return 路径的目录部分。
 */
string GetDirectoryFromPathA(const string &Path, const char cSep = '/');

/**
 * 获得一个路径的文件名部分。
 *
 * @param Path 路径。
 * @param cSep 路径中的分隔符。
 * @return 路径的文件名部分。
 */
string GetNameFromPathA(const string &Path, const char cSep = '/');

#ifdef UNICODE
#define GetNameFromPath GetNameFromPathW
#define GetDirectoryFromPath GetDirectoryFromPathW
#else
#define GetNameFromPath GetNameFromPathA
#define GetDirectoryFromPath GetDirectoryFromPathA
#endif

/**
 * 判断字符串是否以另一字符串结尾
 *
 * @param pSrc 做判断的字符串
 * @param pEnd 结尾字符串
 * @return
 */
bool EndWith(const char *pSrc, const char *pEnd);

/**
 * 替换字符串
 *
 * @param src 原字符串
 * @param pOldString 需要替换的字符串
 * @param pNewString 新字符串
 * @return 替换后的字符串
 */
string Replace(const string & src, const char * pOldString, const char * pNewString);

/**
 * 表示操作系统错误的异常。
 */
class COsError : public std::runtime_error
{
public:
	/**
	 * 构造函数。
	 *
	 * @param iOsError 操作系统的错误码，如果为0则表示当前线程最后产生的错误。
	 * @param pPrefix 要在错误信息之前放置的内容，可以包含格式化参数。如果不须要在错误信息之前放置其它内容，这个参数可以为NULL。
	 * @param ... 用于pPrefix的格式化参数。
	 */
	COsError(int iOsError = 0, const char *pPrefix = NULL, ...) :
		runtime_error(string())
#ifdef _WIN32
		, m_iOsError(iOsError == 0 ? GetLastError() : iOsError)
#else
		, m_iOsError(iOsError == 0 ? errno : iOsError)
#endif		
	{
		if (pPrefix != NULL)
		{
			va_list pArgs;
			va_start(pArgs, pPrefix);
			try
			{
				m_ErrorMessage.assign(VFormatStr(pPrefix, pArgs)).append(" ");
			}
			catch (...)
			{
				va_end(pArgs);
				throw;
			}
			va_end(pArgs);
		}
		m_ErrorMessage.append(GetOsErrorMessageA(m_iOsError));
	}

	/**
	 * 构造函数。
	 *
	 * @param pPrefix 要在错误信息之前放置的内容。如果不须要在错误信息之前放置其它内容，这个参数可以为NULL。
	 */
	COsError(const char *pPrefix) :
		runtime_error(string())
#ifdef _WIN32
		, m_iOsError(GetLastError())
#else
		, m_iOsError(errno)
#endif
	{
		if (pPrefix != NULL) m_ErrorMessage.assign(pPrefix);
		m_ErrorMessage.append(GetOsErrorMessageA(m_iOsError));
	}

	virtual ~COsError() throw()
	{
	}

	virtual const char *what() const throw()
	{
		return m_ErrorMessage.c_str();
	}

	/**
	 * 获得操作系统的错误码。
	 */
	int GetOsError() const { return m_iOsError; }

private:
	/**
	 * 操作系统的错误码。
	 */
	int m_iOsError;

	/**
	 * 错误的文本信息。
	 */
	string m_ErrorMessage;
}; // COsError