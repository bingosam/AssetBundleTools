#pragma once

#include "base.h"

/**
 * ��õ�ǰ�߳��������Ĳ���ϵͳ�����롣
 */
#ifdef _WIN32
__INLINE int GetOsError() { return (int)GetLastError(); }
#else
__INLINE int GetOsError() { return errno; }
#endif

/**
 * ���õ�ǰ�߳��������Ĳ���ϵͳ�����롣
 */
#ifdef _WIN32
__INLINE void SetOsError(int iOsError) { SetLastError((DWORD)iOsError); }
#else
__INLINE void SetOsError(int iOsError) { errno = iOsError; }
#endif

/**
 * ���ָ������ϵͳ�������ʾ��խ�ַ��ı���Ϣ��
 *
 * @param iOsError ����ϵͳ�Ĵ����룬���Ϊ0���ʾ��ǰ�߳��������Ĵ���
 * @return ���ִ�гɹ���Ϊ��������ı���Ϣ�����ִ��ʧ����Ϊ���ַ�����
 */
string GetOsErrorMessageA(int iOsError = 0);

/**
 * ���ָ������ϵͳ�������ʾ�Ŀ��ַ��ı���Ϣ��
 *
 * @param iOsError ����ϵͳ�Ĵ����룬���Ϊ0���ʾ��ǰ�߳��������Ĵ���
 * @return ���ִ�гɹ���Ϊ��������ı���Ϣ�����ִ��ʧ����Ϊ���ַ�����
 */
wstring GetOsErrorMessageW(int iOsError = 0);

#ifdef UNICODE
#define GetOsErrorMessage GetOsErrorMessageW
#else
#define GetOsErrorMessage GetOsErrorMessageA
#endif

/**
 * ��õ�ǰʱ�����������֮�����ķ�������<br>
 * ����ʱ��ʱ����ϸ�ֵ�ܹ��õ���Ӧ�ĸ�������ʱ�䡣
 */
INT32 GetTimeZoneBias();

/**
 * ��ôӲ���ϵͳ������ʼ����ǰ��΢������
 */
INT64 GetRealClock();

/**
 * ����һ��32λ��α�������
 *
 * @param iMin �ɲ������������Сֵ��
 * @param iMax �ɲ�������������ֵ��
 * @return �������������
 */
INT32 RandomInt32(INT32 iMin = -2147483647 - 1, INT32 iMax = 2147483647);

/**
 * ����һ��64λ��α�������
 *
 * @param iMin �ɲ������������Сֵ��
 * @param iMax �ɲ�������������ֵ��
 * @return �������������
 */
INT64 RandomInt64(INT64 iMin = -9223372036854775807LL - 1, INT64 iMax = 9223372036854775807LL);

/**
 * ʹ�ø��ɿ��ķ�������һ��32λ���������
 *
 * @param iMin �ɲ������������Сֵ��
 * @param iMax �ɲ�������������ֵ��
 * @return �������������
 * @note ��������������ٶ�Ҫ����RandomInt32��
 */
INT32 RandomInt32Ex(INT32 iMin = -2147483647 - 1, INT32 iMax = 2147483647);

/**
 * ʹ�ø��ɿ��ķ�������һ��64λ���������
 *
 * @param iMin �ɲ������������Сֵ��
 * @param iMax �ɲ�������������ֵ��
 * @return �������������
 * @note ��������������ٶ�Ҫ����RandomInt64��
 */
INT64 RandomInt64Ex(INT64 iMin = -9223372036854775807LL - 1, INT64 iMax = 9223372036854775807LL);

/**
 * ��32λ����ִ��ԭ�ӵļӷ�������
 *
 * @param[in,out] pAddend ָ��һ��INT32���͵�ָ�룬���һ��������ӵ���ֵ���ں�������ʱ��ָ������ݽ�������Ϊ�����Ľ����
 * @param iValue ��һ��������ӵ���ֵ��
 * @return ����ִ��֮ǰpAddend��ŵ�ֵ��
 */
__INLINE INT32 AtomicAdd32(volatile INT32 *pAddend, INT32 iValue)
{
#ifdef _WIN32
	return InterlockedExchangeAdd((volatile LONG *)pAddend, iValue);
#elif __GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ >= 40102 // ��������GCC 4.1.2����߰汾
	return __sync_fetch_and_add(pAddend, iValue);
#elif defined( __GNUC__ ) && ( defined(LHQ_I386) || defined(LHQ_X64) ) // ��������GCC������Intelƽ̨
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
 * ��32λ����ִ��ԭ�ӵĵ���������
 *
 * @param[in,out] pAddend ָ��һ��INT32���͵�ָ�룬������ڵ�����ֵ���ں�������ʱ��ָ������ݽ�������Ϊ�����Ľ����
 * @return �����Ľ����
 */
__INLINE INT32 AtomicInc32(volatile INT32 *pAddend)
{
#ifdef _WIN32
	return InterlockedIncrement((volatile LONG *)pAddend);
#elif __GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ >= 40102 // ��������GCC 4.1.2����߰汾
	return __sync_add_and_fetch(pAddend, 1);
#else
	return AtomicAdd32(pAddend, 1) + 1;
#endif
}

/**
 * ��32λ����ִ��ԭ�ӵĵݼ�������
 *
 * @param[in,out] pAddend ָ��һ��INT32���͵�ָ�룬������ڵݼ���ֵ���ں�������ʱ��ָ������ݽ�������Ϊ�����Ľ����
 * @return �����Ľ����
 */
__INLINE INT32 AtomicDec32(volatile INT32 *pAddend)
{
#ifdef _WIN32
	return InterlockedDecrement((volatile LONG *)pAddend);
#elif __GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ >= 40102 // ��������GCC 4.1.2����߰汾
	return __sync_add_and_fetch(pAddend, -1);
#else
	return AtomicAdd32(pAddend, -1) - 1;
#endif
}

/**
 * ��32λ����ִ��ԭ�ӵĸ�ֵ������
 *
 * @param[in,out] pDestination ָ��һ��INT32���͵�ָ�룬���Ҫ�Ƚϵ�Ŀ��ֵ��
 * @param iExchange Ҫ�����ֵ��
 * @return ����ִ��֮ǰpDestination��ŵ�ֵ��
 */
__INLINE INT32 AtomicExchange32(volatile INT32 *pDestination, INT32 iExchange)
{
#ifdef _WIN32
	return InterlockedExchange((volatile LONG *)pDestination, iExchange);
#elif defined( __GNUC__ ) && ( defined(LHQ_I386) || defined(LHQ_X64) ) // ��������GCC������Intelƽ̨
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
 * ��32λ����ִ��ԭ�ӵıȽϺ͸�ֵ������
 *
 * @param[in,out] pDestination ָ��һ��INT32���͵�ָ�룬���Ҫ�Ƚϵ�Ŀ��ֵ��
 * @param iExchange ���pDestination��Comparand��ȣ����������ֵ��������pDestination��
 * @param Comparand ���ڱȽϵ�ֵ��
 * @return ����ִ��֮ǰpDestination��ŵ�ֵ��
 */
__INLINE INT32 AtomicCompareExchange32(volatile INT32 *pDestination, INT32 iExchange, INT32 iComparand)
{
#ifdef _WIN32
	return InterlockedCompareExchange((volatile LONG *)pDestination, iExchange, iComparand);
#elif __GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ >= 40102 // ��������GCC 4.1.2����߰汾
	return __sync_val_compare_and_swap(pDestination, iComparand, iExchange);
#elif defined( __GNUC__ ) && ( defined(LHQ_I386) || defined(LHQ_X64) ) // ��������GCC������Intelƽ̨
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
 * ��64λ����ִ��ԭ�ӵļӷ�������
 *
 * @param[in,out] pAddend ָ��һ��INT64���͵�ָ�룬���һ��������ӵ���ֵ���ں�������ʱ��ָ������ݽ�������Ϊ�����Ľ����
 * @param iValue ��һ��������ӵ���ֵ��
 * @return ����ִ��֮ǰpAddend��ŵ�ֵ��
 */
__INLINE INT64 AtomicAdd64(volatile INT64 *pAddend, INT64 iValue)
{
#if __GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ >= 40102 // ��������GCC 4.1.2����߰汾
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
	/* ����ʱ���ϵ�����޴��İ취ʵ��ԭ�Ӳ���
	static lhq::CMutex Mutex[10];

	lhq::CMutexGuard Lock( Mutex[(UINT_PTR)pAddend%10] );
	INT64 iPrevValue = *pAddend;
	*pAddend += iValue;
	return iPrevValue;
	*/
#endif
}

/**
 * ��64λ����ִ��ԭ�ӵĵ���������
 *
 * @param[in,out] pAddend ָ��һ��INT64���͵�ָ�룬������ڵ�����ֵ���ں�������ʱ��ָ������ݽ�������Ϊ�����Ľ����
 * @return �����Ľ����
 */
__INLINE INT64 AtomicInc64(volatile INT64 *pAddend)
{
#if __GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ >= 40102 // ��������GCC 4.1.2����߰汾
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
 * ��64λ����ִ��ԭ�ӵĵݼ�������
 *
 * @param[in,out] pAddend ָ��һ��INT64���͵�ָ�룬������ڵݼ���ֵ���ں�������ʱ��ָ������ݽ�������Ϊ�����Ľ����
 * @return �����Ľ����
 */
__INLINE INT64 AtomicDec64(volatile INT64 *pAddend)
{
#if __GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ >= 40102 // ��������GCC 4.1.2����߰汾
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
 * �Ѷ���������ת��Ϊ����Ϊʮ����������ʽ���ı���
 *
 * @param pData Ҫת�������ݡ�
 * @param nDataSize Ҫת���������ֽ�����
 * @param[out] pBuffer ���ڴ���ı��Ļ�������
 * @param nMaxBuffer ���ڴ���ı��Ļ��������ɴ洢�ֽ����������ַ���ĩβ�Ŀ���ֹ����
 *                       ���������ֵӦ�ô���nDataSize��������
 * @param bWithLowerCase ���Ҫ�õ�ʹ��Сд��ĸ�Ľ����Ϊtrue��
 */
template<typename T> __INLINE void BinToHex(const void *pData, size_t nDataSize, T *pBuffer, size_t nMaxBuffer, bool bWithLowerCase = false)
{
	// ��ʾʮ���������ַ������顣
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
 * �Ѷ���������ת��Ϊ����Ϊʮ����������ʽ���ı���
 *
 * @param pData Ҫת�������ݡ�
 * @param nDataSize Ҫת���������ֽ�����
 * @param bWithLowerCase ���Ҫ�õ�ʹ��Сд��ĸ�Ľ����Ϊtrue��
 * @return ת���Ľ����
 */
__INLINE string BinToHexA(const void *pData, size_t nDataSize, bool bWithLowerCase = false)
{
	string Result(nDataSize * 2 + 1, 0);
	BinToHex(pData, nDataSize, &Result[0], Result.size(), bWithLowerCase);
	Result.resize(nDataSize * 2);
	return Result;
}

/**
 * �Ѷ���������ת��Ϊ����Ϊʮ����������ʽ���ı���
 *
 * @param pData Ҫת�������ݡ�
 * @param nDataSize Ҫת���������ֽ�����
 * @param bWithLowerCase ���Ҫ�õ�ʹ��Сд��ĸ�Ľ����Ϊtrue��
 * @return ת���Ľ����
 */
__INLINE wstring BinToHexW(const void *pData, size_t nDataSize, bool bWithLowerCase = false)
{
	wstring Result(nDataSize * 2 + 1, 0);
	BinToHex(pData, nDataSize, &Result[0], Result.size(), bWithLowerCase);
	Result.resize(nDataSize * 2);
	return Result;
}

/**
 * �ѱ���Ϊʮ����������ʽ���ı�ת��Ϊ���������ݡ�
 *
 * @param pHexText Ҫת�����ı���
 * @param[out] pBuffer ���ڴ�Ŷ��������ݵĻ�������
 * @param nMaxBuffer ���ڴ�Ŷ��������ݵĻ��������ɴ洢�ֽ�����
 * @return ת���õ��Ķ����������ֽ�����
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
 * �����ж�ȡһ�����ݣ����ض�β���ո�
 */
template<class _Elem, class _Traits, class _Alloc> __INLINE basic_istream<_Elem, _Traits> &GetLineTe(
	basic_istream<_Elem, _Traits>& _Istr, basic_string<_Elem, _Traits, _Alloc>& _Str, const _Elem _Delim = '\n')
{
	basic_istream<_Elem, _Traits> &Result = getline(_Istr, _Str, _Delim);
	while (!_Str.empty() && isspace(_Str[_Str.size() - 1])) _Str.resize(_Str.size() - 1);
	return Result;
}

/**
 * ��ָ���ķָ����ָ��ַ���Ϊǰ�������֡�
 *
 * @param[out] First �ָ����ǰ�벿�֡�
 * @param[out] Second �ָ���ĺ�벿�֡�
 * @param pStr Ҫ�ָ���ַ�����
 * @param nStrLen Ҫ�ָ���ַ������ȣ����Ϊ-1���ʾֱ������ֹ����
 * @param cSep �ָ�����
 * @param bTrim ���Ҫ�Էָ���Ĳ��ֽض�ͷβ�ո���Ϊtrue��
 */
template<typename T> __INLINE void DimidiateString(basic_string<T> &First, basic_string<T> &Second, const T *pStr, int nStrLen = -1, T cSep = ',', bool bTrim = true)
{
	vector< basic_string<T> > Result;
	SpliteString(Result, pStr, nStrLen, cSep, bTrim, 2);

	if (Result.size() >= 1) First = Result[0];
	if (Result.size() >= 2) Second = Result[1];
}

/**
 * ��ָ���ķָ����ָ��ַ�����
 *
 * @param[out] Result ��ŷָ�����
 * @param pStr Ҫ�ָ���ַ�����
 * @param nStrLen Ҫ�ָ���ַ������ȣ����Ϊ-1���ʾֱ������ֹ����
 * @param cSep �ָ�����
 * @param bTrim ���Ҫ�Էָ���Ĳ��ֽض�ͷβ�ո���Ϊtrue��
 * @param nMaxCount ���ָ����������Ϊ-1���ʾ���ޡ�
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
 * ��ñ��ؼ��������������
 */
tstring GetHostName();

/**
 * ��ñ��ؼ�������ڵ�����������ơ�
 */
tstring GetDomainName();

/**
 * ��ò���ϵͳ���ơ�
 */
tstring GetOsName();

/**
 * �ж�һ�����ݵ������Ƿ�ȫ��Ϊ0ֵ��
 *
 * @param pData ������ݵĻ�������
 * @param nSize ���ݵ��ֽ�����
 * @return ������ݵ�����ȫ��Ϊ0ֵ��Ϊtrue������Ϊfalse��
 */
bool DataIsZeros(const void *pData, UINT32 nSize);

/**
 * ��CRC32�㷨��������ݵ�4���ֽ�ɢ��ֵ��
 *
 * @param pData Ҫ��������ݡ�
 * @param nDataSize Ҫ����������ֽ�����
 * @return ����Ľ����
 * @author ksaiy
 * @note ����ժ��<a href="http://blog.csdn.net/ksaiy/archive/2005/05/27/382162.aspx">��ȡ�ļ���CRC32ֵ(VC++Դ��-�̶����)����������</a>��
 */
UINT32 CRC32(const BYTE *pData, size_t nDataSize);

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
UINT32 MurmurHash32(const void *pData, int nDataSize, unsigned int uiSeed);

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
UINT64 MurmurHash64(const void *pData, int nDataSize, unsigned int uiSeed);

/**
 * ��MD4�㷨��������ݵ�16���ֽ�ɢ��ֵ��
 *
 * @param pData Ҫ��������ݡ�
 * @param nDataSize Ҫ����������ֽ�����
 * @param[out] pMessageDigest �洢16���ֽ�ɢ��ֵ�Ļ�������
 */
void MD4Hash(const void *pData, size_t nDataSize, BYTE *pMessageDigest);

/**
 * ��MD5�㷨��������ݵ�16���ֽ�ɢ��ֵ��
 *
 * @param pData Ҫ��������ݡ�
 * @param nDataSize Ҫ����������ֽ�����
 * @param[out] pMessageDigest �洢16���ֽ�ɢ��ֵ�Ļ�������
 */
void MD5Hash(const void *pData, size_t nDataSize, BYTE *pMessageDigest);

/**
 * ��MD5�㷨��������ݵĹ�ϣֵ���õ�ʮ�������ַ�����
 *
 * @param pData Ҫ��������ݡ�
 * @param nDataSize Ҫ����������ֽ�����
 * @param bWithLowerCase ���Ҫ����Сд�ַ�����Ϊtrue��
 * @return ��ϣֵ��ʮ�������ַ�����
 */
string MD5Hash(const void *pData, size_t nDataSize, bool bWithLowerCase = true);

/**
 * ��HMAC MD5�㷨�������ݵ�16���ֽ�ɢ��ֵ��
 *
 * @param pKey ����ʹ�õĹؼ��֡�
 * @param nKeySize ����ʹ�õĹؼ����ֽ�����
 * @param pData Ҫ��������ݡ�
 * @param nDataSize Ҫ����������ֽ�����
 * @param[out] pMessageDigest �洢ɢ��ֵ�Ļ���������������С��16�ֽڡ�
 */
void HMACMD5(const void *pKey, size_t nKeySize, const void *pData, size_t nDataSize, BYTE *pMessageDigest);

/**
 * ��SHA1�㷨�������ݵ�20���ֽ�ɢ��ֵ��
 *
 * @param pData Ҫ��������ݡ�
 * @param nDataSize Ҫ����������ֽ�����
 * @param[out] pMessageDigest �洢ɢ��ֵ�Ļ���������������С��20�ֽڡ�
 */
void SHA1Hash(const void *pData, size_t nDataSize, BYTE *pMessageDigest);

/**
 * ��HMAC SHA1�㷨�������ݵ�20���ֽ�ɢ��ֵ��
 *
 * @param pKey ����ʹ�õĹؼ��֡�
 * @param nKeySize ����ʹ�õĹؼ����ֽ�����
 * @param pData Ҫ��������ݡ�
 * @param nDataSize Ҫ����������ֽ�����
 * @param[out] pMessageDigest �洢ɢ��ֵ�Ļ���������������С��20�ֽڡ�
 */
void HMACSHA1(const void *pKey, size_t nKeySize, const void *pData, size_t nDataSize, BYTE *pMessageDigest);

/**
 * Base64���롣
 *
 * @param pData Ҫ������������ݡ�
 * @param nDataSize Ҫ����������ֽ�����
 * @param[out] pBuffer ��ű������ݵĻ������������ַ���ĩβ�Ŀ���ֹ������������С�� 4.0 * ceil(nDataSize / 3.0)��
 * @param[out] nBufferSize ��������������
 * @return ���ִ�гɹ���Ϊtrue�����������������Ϊfalse��
 */
bool Base64Encode(const void *pData, size_t nDataSize, char *pBuffer, size_t nBufferSize);

/**
 * Base64���롣
 *
 * @param pData Ҫ������������ݡ�
 * @param nDataSize Ҫ����������ֽ�����
 * @return ���ִ�гɹ���Ϊ������������Ϊ�����ݡ�
 */
string Base64Encode(const void *pData, size_t nDataSize);

/**
 * Base64���롣
 *
 * @param pData Ҫ������������ݡ�
 * @param nDataSize Ҫ����������ֽ�����
 * @param[out] pBuffer ����ѽ������ݵĻ���������������С�� nDataSize / 4 * 3��
 * @param[in,out] nBufferSize �ں���ִ��ʱ��ʾ���������������ں�������ʱ���ڴ���ѽ������ݵ��ֽ�����
 * @return ���ִ�гɹ���Ϊtrue�����Ҫ�����������Ч���߻�����������Ϊfalse��
 */
bool Base64Decode(const char *pData, size_t nDataSize, void *pBuffer, size_t &nBufferSize);

#ifndef _LHQ_NO_DES_FUNC
/**
 * ��DES�㷨���м��ܻ���ܡ�
 *
 * @param bDoEncrypt ���Ҫ���м�����Ϊtrue�����Ҫ���н�����Ϊfalse��
 * @param pIn ��bDoEncryptΪtrueʱ���������ΪҪ���ܵ����ݡ�
 *                ��bDoEncryptΪfalseʱ���������ΪҪ���ܵ����ݡ�
 * @param nInSize pIn�������ֽ�����
 * @param[out] pOut ��bDoEncryptΪtrueʱ������������ڴ�ż��ܵĽ����
 *                  ��bDoEncryptΪfalseʱ������������ڴ�Ž��ܵĽ����
 * @param nMaxOut pOut�ܹ���ŵ���������ֽ���������Ϊ8�ı������Ҳ���С��nInSize��
 * @param pKey �����õ�8�ֽڹؼ��֡�
 *
 * @note DES�㷨�����������ֽ�������Ϊ8�ı�����<br>
 *       �ڼ���ʱ�����pIn�ṩ�ļ��������ֽ�������8�ı������������Զ��ں��油��ŵ��0ֵ����8�ֽ�Ȼ����м��ܡ�<br>
 *       �ڽ���ʱ�����pIn�ṩ�Ľ��������ֽ�������8�ı��������ᵼ�º���ִ��ʧ�ܡ�
 */
void DesEncrypt(bool bDoEncrypt, const BYTE *pIn, UINT32 nInSize, BYTE *pOut, UINT32 nMaxOut, const BYTE *pKey);
#endif

/**
 * �жϵ�ǰϵͳ�ַ����Ƿ�ΪUTF-8��
 */
bool WithUtf8();

/**
 * ���ϵͳĬ�ϵ��ַ����������ơ�
 *
 * @param[out] pBuffer ����ַ����������ƵĻ�������
 * @param nMaxBuffer ����ַ����������ƵĻ��������ɴ洢�ַ����������ַ���ĩβ�Ŀ���ֹ����
 * @return ���ִ�гɹ���Ϊtrue������Ϊfalse��
 */
bool GetSystemCharset(char *pBuffer, int nMaxBuffer);

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
int StrIconv(const char *pSrcCharset, const char *pSrc, int nSrcSize, const char *pDestCharset, char *pBuffer, int nMaxBuffer);

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
int AnsiStrToUtf8Str(const char *pSrc, int nSrcLen, char *pBuffer, int nMaxBuffer, const char *pSrcCharset = NULL);

/**
 * ����char�����ַ���ɵ��ַ���ת��Ϊ��UTF-8�����string����
 *
 * @param pSrc Ҫת�����ַ�����
 * @param nSrcLen Ҫת�����ַ��������Ϊ-1���ʾת��ֱ������ֹ��Ϊֹ��
 * @param pSrcCharset �ַ������ַ������룬���ΪNULL���߿��������ʾϵͳĬ��ֵ��
 * @return ���ִ�гɹ���Ϊת���Ľ�������û�����ݿ�ת������ת��ʧ����Ϊ���ַ�����
 */
string AnsiStrToUtf8Str(const char *pSrc, int nSrcLen = -1, const char *pSrcCharset = NULL);

/**
 * ����wchar_t�����ַ���ɵ��ַ���ת��Ϊ��char�����ַ���ɲ��Ұ�UTF-8������ַ�����
 *
 * @param pSrc Ҫת�����ַ�����
 * @param nSrcLen Ҫת�����ַ��������Ϊ-1���ʾת��ֱ������ֹ��Ϊֹ��
 * @param[out] pBuffer ���ת������Ļ�������
 * @param nMaxBuffer ���ת������Ļ��������ɴ洢�ַ�����
 * @return ���ִ�гɹ���Ϊת��������ַ�����������ִ�����߻�����̫С��Ϊ-1��
 */
int WideStrToUtf8Str(const wchar_t *pSrc, int nSrcLen, char *pBuffer, int nMaxBuffer);

/**
 * ����wchar_t�����ַ���ɵ��ַ���ת��Ϊ��UTF-8�����string����
 *
 * @param pSrc Ҫת�����ַ�����
 * @param nSrcLen Ҫת�����ַ��������Ϊ-1���ʾת��ֱ������ֹ��Ϊֹ��
 * @return ���ִ�гɹ���Ϊת���Ľ�������û�����ݿ�ת������ת��ʧ����Ϊ���ַ�����
 */
string WideStrToUtf8Str(const wchar_t *pSrc, int nSrcLen = -1);

/**
 * ���ɱ����ַ��������ַ���ת��Ϊ��UTF-8�����string����
 */
#ifdef UNICODE
#define TStrToUtf8Str WideStrToUtf8Str
#else
#define TStrToUtf8Str AnsiStrToUtf8Str
#endif

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
int Utf8StrToAnsiStr(const char *pSrc, int nSrcLen, char *pBuffer, int nMaxBuffer, const char *pDestCharset = NULL);

/**
 * ����char�����ַ���ɲ��Ұ�UTF-8������ַ���ת��Ϊstring����
 *
 * @param pSrc Ҫת�����ַ�����
 * @param nSrcLen Ҫת�����ַ��������Ϊ-1���ʾת��ֱ������ֹ��Ϊֹ��
 * @param pDestCharset Ŀ���ַ������ַ������룬���ΪNULL���߿��������ʾϵͳĬ��ֵ��
 * @return ���ִ�гɹ���Ϊת���Ľ�������û�����ݿ�ת������ת��ʧ����Ϊ���ַ�����
 */
string Utf8StrToAnsiStr(const char *pSrc, int nSrcLen = -1, const char *pDestCharset = NULL);

/**
 * ����char�����ַ���ɲ��Ұ�UTF-8������ַ���ת��Ϊ��wchar_t�����ַ���ɵ��ַ�����
 *
 * @param pSrc Ҫת�����ַ�����
 * @param nSrcLen Ҫת�����ַ��������Ϊ-1���ʾת��ֱ������ֹ��Ϊֹ��
 * @param[out] pBuffer ���ת������Ļ�������
 * @param nMaxBuffer ���ת������Ļ��������ɴ洢�ַ�����
 * @return ���ִ�гɹ���Ϊת��������ַ�����������ִ�����߻�����̫С��Ϊ-1��
 */
int Utf8StrToWideStr(const char *pSrc, int nSrcLen, wchar_t *pBuffer, int nMaxBuffer);

/**
 * ����char�����ַ���ɲ��Ұ�UTF-8������ַ���ת��Ϊwstring����
 *
 * @param pSrc Ҫת�����ַ�����
 * @param nSrcLen Ҫת�����ַ��������Ϊ-1���ʾת��ֱ������ֹ��Ϊֹ��
 * @return ���ִ�гɹ���Ϊת���Ľ�������û�����ݿ�ת������ת��ʧ����Ϊ���ַ�����
 */
wstring Utf8StrToWideStr(const char *pSrc, int nSrcLen = -1);

/**
 * ����char�����ַ���ɲ��Ұ�UTF-8������ַ���ת��Ϊtstring����
 */
#ifdef UNICODE
#define Utf8StrToTStr Utf8StrToWideStr
#else
#define Utf8StrToTStr Utf8StrToAnsiStr
#endif

/**
 * ����char�����ַ���ɵ��ַ���ת��Ϊ��wchar_t�����ַ���ɵ��ַ�����
 *
 * @param pSrc Ҫת�����ַ�����
 * @param nSrcLen Ҫת�����ַ��������Ϊ-1���ʾת��ֱ������ֹ��Ϊֹ��
 * @param[out] pBuffer ���ת������Ļ�������
 * @param nMaxBuffer ���ת������Ļ��������ɴ洢�ַ�����
 * @return ���ִ�гɹ���Ϊת��������ַ�����������ִ�����߻�����̫С��Ϊ-1��
 */
int AnsiStrToWideStr(const char *pSrc, int nSrcLen, wchar_t *pBuffer, int nMaxBuffer);

/**
 * ����char�����ַ���ɵ��ַ���ת��Ϊwstring����
 *
 * @param pSrc Ҫת�����ַ�����
 * @param nSrcLen Ҫת�����ַ��������Ϊ-1���ʾת��ֱ������ֹ��Ϊֹ��
 * @return ���ִ�гɹ���Ϊת���Ľ�������û�����ݿ�ת������ת��ʧ����Ϊ���ַ�����
 */
wstring AnsiStrToWideStr(const char *pSrc, int nSrcLen = -1);

/**
 * ����wchar_t�����ַ���ɵ��ַ���ת��Ϊ��char�����ַ���ɵ��ַ�����
 *
 * @param pSrc Ҫת�����ַ�����
 * @param nSrcLen Ҫת�����ַ��������Ϊ-1���ʾת��ֱ������ֹ��Ϊֹ��
 * @param[out] pBuffer ���ת������Ļ�������
 * @param nMaxBuffer ���ת������Ļ��������ɴ洢�ַ�����
 * @return ���ִ�гɹ���Ϊת��������ַ�����������ִ�����߻�����̫С��Ϊ-1��
 */
int WideStrToAnsiStr(const wchar_t *pSrc, int nSrcLen, char *pBuffer, int nMaxBuffer);

/**
 * ����wchar_t�����ַ���ɵ��ַ���ת��Ϊstring����
 *
 * @param pSrc Ҫת�����ַ�����
 * @param nSrcLen Ҫת�����ַ��������Ϊ-1���ʾת��ֱ������ֹ��Ϊֹ��
 * @return ���ִ�гɹ���Ϊת���Ľ�������û�����ݿ�ת������ת��ʧ����Ϊ���ַ�����
 */
string WideStrToAnsiStr(const wchar_t *pSrc, int nSrcLen = -1);

/**
 * ��ʽ��һ���ַ�����
 *
 * @param pFormat ��ʽ����Ϣ���������ANSI C�ĸ�ʽ���淶��
 * @param ... ��ʽ���Ĳ�����
 * @return ��Ÿ�ʽ�������string����
 */
string FormatStr(const char *pFormat, ...);

/**
 * ��ʽ��һ���ַ�����
 *
 * @param pFormat ��ʽ����Ϣ���������ANSI C�ĸ�ʽ���淶��
 * @param pArgs ��ʽ���Ĳ�����
 * @return ��Ÿ�ʽ�������string����
 */
string VFormatStr(const char *pFormat, va_list pArgs);

/**
 * ��ʽ��һ�����ַ�����
 *
 * @param pFormat ��ʽ����Ϣ���������ANSI C�ĸ�ʽ���淶��
 * @param ... ��ʽ���Ĳ�����
 * @return ��Ÿ�ʽ�������wstring����
 */
wstring FormatStr(const wchar_t *pFormat, ...);

/**
 * ��ʽ��һ�����ַ�����
 *
 * @param pFormat ��ʽ����Ϣ���������ANSI C�ĸ�ʽ���淶��
 * @param pArgs ��ʽ���Ĳ�����
 * @return ��Ÿ�ʽ�������wstring����
 */
wstring VFormatStr(const wchar_t *pFormat, va_list pArgs);

/**
 * ���һ��·����Ŀ¼���֡�
 *
 * @param Path ·����
 * @param cSep ·���еķָ�����
 * @return ·����Ŀ¼���֡�
 */
wstring GetDirectoryFromPathW(const wstring &Path, const wchar_t cSep = L'/');

/**
 * ���һ��·�����ļ������֡�
 *
 * @param Path ·����
 * @param cSep ·���еķָ�����
 * @return ·�����ļ������֡�
 */
wstring GetNameFromPathW(const wstring &Path, const wchar_t cSep = L'/');

/**
 * ���һ��·����Ŀ¼���֡�
 *
 * @param Path ·����
 * @param cSep ·���еķָ�����
 * @return ·����Ŀ¼���֡�
 */
string GetDirectoryFromPathA(const string &Path, const char cSep = '/');

/**
 * ���һ��·�����ļ������֡�
 *
 * @param Path ·����
 * @param cSep ·���еķָ�����
 * @return ·�����ļ������֡�
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
 * �ж��ַ����Ƿ�����һ�ַ�����β
 *
 * @param pSrc ���жϵ��ַ���
 * @param pEnd ��β�ַ���
 * @return
 */
bool EndWith(const char *pSrc, const char *pEnd);

/**
 * �滻�ַ���
 *
 * @param src ԭ�ַ���
 * @param pOldString ��Ҫ�滻���ַ���
 * @param pNewString ���ַ���
 * @return �滻����ַ���
 */
string Replace(const string & src, const char * pOldString, const char * pNewString);

/**
 * ��ʾ����ϵͳ������쳣��
 */
class COsError : public std::runtime_error
{
public:
	/**
	 * ���캯����
	 *
	 * @param iOsError ����ϵͳ�Ĵ����룬���Ϊ0���ʾ��ǰ�߳��������Ĵ���
	 * @param pPrefix Ҫ�ڴ�����Ϣ֮ǰ���õ����ݣ����԰�����ʽ���������������Ҫ�ڴ�����Ϣ֮ǰ�����������ݣ������������ΪNULL��
	 * @param ... ����pPrefix�ĸ�ʽ��������
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
	 * ���캯����
	 *
	 * @param pPrefix Ҫ�ڴ�����Ϣ֮ǰ���õ����ݡ��������Ҫ�ڴ�����Ϣ֮ǰ�����������ݣ������������ΪNULL��
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
	 * ��ò���ϵͳ�Ĵ����롣
	 */
	int GetOsError() const { return m_iOsError; }

private:
	/**
	 * ����ϵͳ�Ĵ����롣
	 */
	int m_iOsError;

	/**
	 * ������ı���Ϣ��
	 */
	string m_ErrorMessage;
}; // COsError