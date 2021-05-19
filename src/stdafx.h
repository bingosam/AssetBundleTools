// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#ifdef _WIN32
#define SEPARATOR "\\"
#define SEPARATOR_CHAR '\\'
#else
#define SEPARATOR "/"
#define SEPARATOR_CHAR '/'
#define 
#endif //_WIN32



#include "utils/base.h"
#include "utils/utils.h"
#include "UABEWapper/uabedef.h"


#define FREE(a,b)					\
	do {							\
		if( NULL == (a) ) break;	\
		b((a));						\
		(a) = NULL;					\
	}while(0)


template <typename T>
void SafeRelease(T **ppT) {
	if (*ppT) {
		(*ppT)->Release();
		*ppT = NULL;
	}
}

template <typename T>
void ReleaseVector(vector < T > &vt)
{
	vector< T > tmp;
	tmp.swap(vt);
}

#define FOR_PLATFORM

// TODO: reference additional headers your program requires here
