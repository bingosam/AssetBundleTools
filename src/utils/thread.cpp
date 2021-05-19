#include "base.h"
#ifndef _WIN32
#include <signal.h>
#endif
#include "thread.h"

// ��ʼ��CRwLock�ľ�̬��Ա
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
 * ���캯����
 *
 * @param nStackSize �̶߳�ջ�Ĵ�С�����Ϊ0���ʾʹ��ϵͳĬ��ֵ��
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
 * �����̡߳�
 *
 * @return ���ִ�гɹ���Ϊtrue��������ִ�����Ϊfalse��
 */
bool CThread::Start()
{
	CMutexGuard Lock( m_Mutex );

	if( m_bRunning ) return true; // �߳��Ѿ�����

	m_bRunning = true;
	m_bTerminating = false;
#if defined( _WIN32 ) // ����ϵͳ��Windows
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
	if( pthread_attr_init(&Attr) != 0 ) return false; // ��ʼ���߳�����ʧ��

#ifdef linux
	if( m_nStackSize == 0 ) m_nStackSize = 1024 * 1024;
#endif

	if( m_nStackSize > 0 && pthread_attr_setstacksize(&Attr,m_nStackSize) != 0 ) // �����̶߳�ջ��Сʧ��
	{
		pthread_attr_destroy( &Attr );
		return false;
	}
		
	if( pthread_create(&m_Thread,&Attr,ThreadProc,this) != 0 )
	{
		m_Thread = 0;
		m_bRunning = false;
		pthread_attr_destroy( &Attr );
		return false; // �����߳�ʧ��
	}

	pthread_attr_destroy( &Attr );
#endif

	return true;
}

/**
 * ��ֹ�̡߳�
 *
 * @return ���ִ�гɹ���Ϊtrue��
 */
bool CThread::Terminate()
{
	if( m_bRunning )
	{
		CMutexGuard Lock( m_Mutex );
		if( !m_bRunning ) return true; // ������������ֹ������߳�

		m_bRunning = false;
		m_bTerminating = true;
		
#if defined( _WIN32 ) // ����ϵͳ��Windows
		HANDLE hThread = m_hThread;
		m_hThread = NULL;
		DWORD dwResult = WaitForSingleObject( hThread, INFINITE ); // �ȴ��߳���ֹ			
		m_bTerminating = false;
		if( dwResult == WAIT_OBJECT_0 ) // �߳�����ֹ
		{
			CloseHandle( hThread );
			return true;
		}
		else // ���ִ���
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
			bResult = pthread_join( Id, &pResult ) == 0; // �ȴ��߳���ֹ
		}
		m_bTerminating = false;
		return bResult;
#endif
	}

	return true;
}

/**
 * ��ƽ̨��ص��߳�API��ں�����
 */
#if defined( _WIN32 ) // ����ϵͳ��Windows
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
		pthread_detach( pthread_self() ); // �����ͷ���Դ
#else
		HANDLE hThread = pThread->m_hThread;
		pThread->m_hThread = NULL;
		CloseHandle( hThread );
#endif
	}

	return 0;	
}
