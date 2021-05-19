#include "base.h"
#ifndef _WIN32
#include <signal.h>
#endif
#include "thread.h"

// 初始化CRwLock的静态成员
#ifdef _WIN32
CRwLock::CInitializer CRwLock::m_Initializer;
bool CRwLock::m_bWithSrwLock = false;
HMODULE CRwLock::m_hKernel32 = NULL;
CRwLock::SRWLOCK_FUNC CRwLock::m_pfnInitializeSRWLock = NULL;
CRwLock::SRWLOCK_FUNC CRwLock::m_pfnAcquireSRWLockExclusive = NULL;
CRwLock::SRWLOCK_FUNC CRwLock::m_pfnReleaseSRWLockExclusive = NULL;
CRwLock::SRWLOCK_FUNC CRwLock::m_pfnAcquireSRWLockShared = NULL;
CRwLock::SRWLOCK_FUNC CRwLock::m_pfnReleaseSRWLockShared = NULL;
#endif

/**
 * 构造函数。
 *
 * @param nStackSize 线程堆栈的大小，如果为0则表示使用系统默认值。
 */
CThread::CThread( unsigned int nStackSize /* = 0 */ )
{
	m_bTerminating = false;
	m_bRunning = false;
#ifdef _WIN32
	m_hThread = NULL;
#else
	m_Thread = 0;
#endif
	m_nStackSize = nStackSize;
}

/* virtual */ CThread::~CThread()
{
	if( m_bRunning ) Terminate();
}

/**
 * 启动线程。
 *
 * @return 如果执行成功则为true，如果出现错误则为false。
 */
bool CThread::Start()
{
	CMutexGuard Lock( m_Mutex );

	if( m_bRunning ) return true; // 线程已经运行

	m_bRunning = true;
	m_bTerminating = false;
#if defined( _WIN32 ) // 操作系统是Windows
#if defined( _MSC_VER ) || defined( __MINGW32__ )
	m_hThread = (void *)_beginthreadex( NULL, m_nStackSize, ThreadProc, this, 0, NULL );
#else
	m_hThread = CreateThread( NULL, m_nStackSize, (LPTHREAD_START_ROUTINE)ThreadProc, this, 0, NULL );
#endif
	if( m_hThread == NULL )
	{
		m_bRunning = false;
		return false;
	}
#else
	pthread_attr_t Attr;
	if( pthread_attr_init(&Attr) != 0 ) return false; // 初始化线程属性失败

#ifdef linux
	if( m_nStackSize == 0 ) m_nStackSize = 1024 * 1024;
#endif

	if( m_nStackSize > 0 && pthread_attr_setstacksize(&Attr,m_nStackSize) != 0 ) // 设置线程堆栈大小失败
	{
		pthread_attr_destroy( &Attr );
		return false;
	}
		
	if( pthread_create(&m_Thread,&Attr,ThreadProc,this) != 0 )
	{
		m_Thread = 0;
		m_bRunning = false;
		pthread_attr_destroy( &Attr );
		return false; // 创建线程失败
	}

	pthread_attr_destroy( &Attr );
#endif

	return true;
}

/**
 * 终止线程。
 *
 * @return 如果执行成功则为true。
 */
bool CThread::Terminate()
{
	if( m_bRunning )
	{
		CMutexGuard Lock( m_Mutex );
		if( !m_bRunning ) return true; // 其它调用者终止了这个线程

		m_bRunning = false;
		m_bTerminating = true;
		
#if defined( _WIN32 ) // 操作系统是Windows
		HANDLE hThread = m_hThread;
		m_hThread = NULL;
		DWORD dwResult = WaitForSingleObject( hThread, INFINITE ); // 等待线程终止			
		m_bTerminating = false;
		if( dwResult == WAIT_OBJECT_0 ) // 线程已终止
		{
			CloseHandle( hThread );
			return true;
		}
		else // 出现错误
		{
			DWORD dwLastError = GetLastError();
			CloseHandle( hThread );
			SetLastError( dwLastError );
			return false;
		}
#else
		pthread_t Id = m_Thread;
		m_Thread = 0;
		bool bResult = true;
		if( pthread_kill(Id, 0) == 0 )
		{
			void *pResult;
			bResult = pthread_join( Id, &pResult ) == 0; // 等待线程终止
		}
		m_bTerminating = false;
		return bResult;
#endif
	}

	return true;
}

/**
 * 与平台相关的线程API入口函数。
 */
#if defined( _WIN32 ) // 操作系统是Windows
/* static */ unsigned int WINAPI CThread::ThreadProc( void *pParam )
#else
/* static */ void *CThread::ThreadProc( void *pParam )
#endif
{
	CThread *pThread = (CThread *)pParam;

	pThread->m_bRunning = true;
	try
	{
		pThread->Run();
	}
	catch( ... )
	{
	}

	if( pThread->m_bRunning )
	{
		pThread->m_bRunning = false;
		pThread->m_bTerminating = false;
#ifndef _WIN32
		pthread_detach( pthread_self() ); // 主动释放资源
#else
		HANDLE hThread = pThread->m_hThread;
		pThread->m_hThread = NULL;
		CloseHandle( hThread );
#endif
	}

	return 0;	
}
