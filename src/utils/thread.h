#pragma once

#include "base.h"

/**
 * �źŵƶ���
 */
class CSemaphore : public CNonCopyable
{
public:
	/**
	 * ���캯����
	 *
	 * @param uiInitialCount ��ʼ���ź�����
	 * @param uiMaximumCount ���������ź������������ֻ����Windowsƽ̨����Ч��
	 */
    CSemaphore( unsigned int uiInitialCount, unsigned int uiMaximumCount )
    {
#ifdef _WIN32
		m_hSemaphore = CreateSemaphore( NULL, uiInitialCount, uiMaximumCount, NULL );
		if( m_hSemaphore == NULL )
		{
			char szMessage[64];
			sprintf( szMessage, "[Error %u] Failed to create semaphore!", (UINT32)GetLastError() );
			throw std::runtime_error( szMessage );
		}
#else
		if( sem_init(&m_Semaphore,0,uiInitialCount) != 0 )
		{
			char szMessage[64];
			sprintf( szMessage, "[Error %d] Failed to create semaphore!", errno );
			throw std::runtime_error( szMessage );
		}
#endif
    }
	
    ~CSemaphore()
    {
#ifdef _WIN32
		CloseHandle( m_hSemaphore );
#else
		sem_destroy( &m_Semaphore );
#endif
    }

	/**
	 * �����ź�����
	 *
	 * @param nMilliseconds �ȴ��ĳ�ʱֵ����λΪ���롣���Ϊ0xFFFFFFFF���ʾ����ʱ�ȴ���
	 * @return ����ȴ��ɹ���Ϊtrue������ȴ���ʱ���߳��ִ�����Ϊfalse��
	 */
	bool Wait( unsigned int nMilliseconds = 0xFFFFFFFF )
    {
#ifdef _WIN32
		return WaitForSingleObject( m_hSemaphore, nMilliseconds ) == WAIT_OBJECT_0;
#else
		switch( nMilliseconds )
		{
		case 0:			
			return sem_trywait( &m_Semaphore ) == 0;
		case 0xFFFFFFFF:
			return sem_wait( &m_Semaphore ) == 0;
			break;
		default:
			struct timespec AbsTime;
			clock_gettime( CLOCK_REALTIME, &AbsTime );
			AbsTime.tv_sec = AbsTime.tv_sec + nMilliseconds / 1000;
			AbsTime.tv_nsec = AbsTime.tv_nsec + nMilliseconds % 1000 * 1000000;
#ifdef _DEBUG
			while( sem_timedwait(&m_Semaphore, &AbsTime) != 0 )
			{
				if( errno == EINTR ) continue; // gdb�����ź�
				else return false;
			}

			return true;
#else
			return sem_timedwait( &m_Semaphore, &AbsTime ) == 0;
#endif
		}
#endif
    }

	/**
	 * �����ź�����
	 *
	 * @return ���ִ�гɹ���Ϊtrue��������ִ�����Ϊfalse��
	 */
    bool Release()
    {
#ifdef _WIN32
		return ReleaseSemaphore( m_hSemaphore, 1, NULL ) != 0;
#else
		return sem_post( &m_Semaphore ) == 0;
#endif
    }

private:
#ifdef _WIN32
	HANDLE m_hSemaphore;
#else
	sem_t m_Semaphore;
#endif
}; // CSemaphore

/**
 * �źŵƶ����ػ��ߣ������Զ����ٺ������ź�����
 */
class CSemaphoreGuard : public CNonCopyable
{
public:
	/**
	 * ���캯����
	 *
	 * @param Semaphore Ҫ�ػ����źŵƶ���
	 * @param nMilliseconds �ȴ��ĳ�ʱֵ����λΪ���롣���Ϊ0xFFFFFFFF���ʾ����ʱ�ȴ���
	 * @return ����ȴ��ɹ���Ϊtrue������ȴ���ʱ���߳��ִ�����Ϊfalse��
	 */
	CSemaphoreGuard( CSemaphore &Semaphore, unsigned int nMilliseconds = 0xFFFFFFFF ) : m_Semaphore( Semaphore )
	{
		m_bIsOk = Semaphore.Wait( nMilliseconds );
	}

	~CSemaphoreGuard()
	{
		if( m_bIsOk ) m_Semaphore.Release();
	}

	/**
	 * �ж��Ƿ�ɹ������ź�����
	 */
	bool IsOk() const { return m_bIsOk; }

private:
	CSemaphore &m_Semaphore;
	bool m_bIsOk;
};

/**
 * �������
 */
class CMutex : public CNonCopyable
{
public:
	/**
	 * ���캯����
	 *
	 * @param bRecursive ָ���Ƿ�Ҫ���õݹ����ԡ�
	 */
    CMutex( bool bRecursive = true )
    {
#ifdef _WIN32
		if( bRecursive )
		{
			InitializeCriticalSection( &m_CriticalSection );
			m_hSemaphore = NULL;
		}
		else
		{
			m_hSemaphore = CreateSemaphore( NULL, 1, 1, NULL );
			if( m_hSemaphore == NULL )
			{
				char szMessage[64];
				sprintf( szMessage, "[Error %u] Failed to create mutex!", (UINT32)GetLastError() );
				throw std::runtime_error( szMessage );
			}
		}
#else
		bool bHasError;

		pthread_mutexattr_t Attr;
		bHasError = pthread_mutexattr_init( &Attr ) != 0;
		bHasError = bHasError || pthread_mutexattr_settype( &Attr, bRecursive ? PTHREAD_MUTEX_RECURSIVE : PTHREAD_MUTEX_ERRORCHECK ) != 0;
		bHasError = bHasError || pthread_mutex_init( &m_Mutex, &Attr ) != 0;
		if( bHasError )
		{
			char szMessage[64];
			sprintf( szMessage, "[Error %d] Failed to create mutex!", errno );
			throw std::runtime_error( szMessage );
		}
		else
		{
			pthread_mutexattr_destroy( &Attr );
		}
#endif
    }
	
    ~CMutex()
    {
#ifdef _WIN32
		if( m_hSemaphore == NULL ) DeleteCriticalSection( &m_CriticalSection );
        else CloseHandle( m_hSemaphore );
#else
		pthread_mutex_destroy( &m_Mutex );
#endif
    }

	/**
	 * Ϊ��ǰ�̻߳�ȡ����Ȩ��
	 *
	 * @return ���ִ�гɹ���Ϊtrue��������ִ�����Ϊfalse��
	 */
    bool Lock()
    {
#ifdef _WIN32
		if( m_hSemaphore == NULL )
		{
			EnterCriticalSection( &m_CriticalSection );
			return true;
		}
		else
		{
			return WaitForSingleObject( m_hSemaphore, 0xFFFFFFFF ) == WAIT_OBJECT_0;
		}
#else
		return pthread_mutex_lock( &m_Mutex ) == 0;
#endif
    }

	/**
	 * Ϊ��ǰ�̳߳��Ի�ȡ����Ȩ��
	 *
	 * @return �����ȡ����Ȩ�ɹ���Ϊtrue��������ִ�����������߳��Ѿ���ȡ������Ȩ��Ϊfalse��
	 */
    bool TryLock()
    {
#ifdef _WIN32
		if( m_hSemaphore == NULL )
		{
			return TryEnterCriticalSection( &m_CriticalSection ) != 0;
		}
		else
		{
			return WaitForSingleObject( m_hSemaphore, 0 ) == WAIT_OBJECT_0;
		}
#else
		return pthread_mutex_trylock( &m_Mutex ) == 0;
#endif
    }

	/**
	 * Ϊ��ǰ�߳��ͷ�����Ȩ��
	 *
	 * @return ���ִ�гɹ���Ϊtrue��������ִ�����Ϊfalse��
	 */
	bool Unlock()
    {
#ifdef _WIN32
		if( m_hSemaphore == NULL )
		{
			LeaveCriticalSection( &m_CriticalSection );
			return true;
		}
		else
		{
			return ReleaseSemaphore( m_hSemaphore, 1, NULL ) != 0;
		}
#else
		return pthread_mutex_unlock( &m_Mutex ) == 0;
#endif
    }

private:
#ifdef _WIN32
	CRITICAL_SECTION m_CriticalSection;
	HANDLE m_hSemaphore;
#else
	pthread_mutex_t m_Mutex;
#endif
}; // CMutex

/**
 * ��������ػ��ߣ������Զ���ȡ���ͷ�����Ȩ��
 */
class CMutexGuard : public CNonCopyable
{
public:
	CMutexGuard( CMutex &Mutex ) : m_Mutex( Mutex )
	{
		Mutex.Lock();
	}

	~CMutexGuard()
	{
		m_Mutex.Unlock();
	}

private:
	CMutex &m_Mutex;
};

/**
 * �¼�����
 */
class CEvent : public CNonCopyable
{
public:
	/**
	 * ���캯����
	 *
	 * @param bInitialState ָ���Ƿ�Ҫ�����¼�Ϊ�ź�״̬��
	 */
    CEvent( bool bInitialState = false )
    {
#ifdef _WIN32
        m_hEvent = CreateEvent( NULL, FALSE, bInitialState ? 1 : 0, NULL );
		if( m_hEvent == NULL )
		{
			char szMessage[64];
			sprintf( szMessage, "[Error %u] Failed to create event!", (UINT32)GetLastError() );
			throw std::runtime_error( szMessage );
		}
#else
		if( sem_init(&m_Semaphore,0,bInitialState?1:0) != 0 )
		{
			char szMessage[64];
			sprintf( szMessage, "[Error %d] Failed to create event!", errno );
			throw std::runtime_error( szMessage );
		}
#endif
    }
	
    ~CEvent()
    {
#ifdef _WIN32
        CloseHandle( m_hEvent );
#else
		sem_destroy( &m_Semaphore );
#endif
    }

	/**
	 * �ȴ��¼�Ϊ�ź�״̬��
	 *
	 * @param nMilliseconds �ȴ��ĳ�ʱֵ����λΪ���롣���Ϊ0xFFFFFFFF���ʾ����ʱ�ȴ���
	 * @return ����ȴ��ɹ���Ϊtrue��������ִ�����ߵȴ���ʱ��Ϊfalse��
	 */
	bool Wait( unsigned int nMilliseconds = 0xFFFFFFFF )
    {
#ifdef _WIN32
		return WaitForSingleObject( m_hEvent, nMilliseconds ) == WAIT_OBJECT_0;
#else
		switch( nMilliseconds )
		{
		case 0:			
			return sem_trywait( &m_Semaphore ) == 0;
			break;
		case 0xFFFFFFFF:
			return sem_wait( &m_Semaphore ) == 0;
			break;
		default:
			struct timespec AbsTime;
			clock_gettime( CLOCK_REALTIME, &AbsTime );
			UINT64 uiTimeout = AbsTime.tv_sec * 1000000LL + AbsTime.tv_nsec / 1000 + nMilliseconds * 1000LL;
			AbsTime.tv_sec = uiTimeout / 1000000;
			AbsTime.tv_nsec = uiTimeout % 1000000 * 1000;
#ifdef _DEBUG
			while( sem_timedwait(&m_Semaphore, &AbsTime) != 0 )
			{
				if( errno == EINTR ) continue; // gdb�����ź�
				else return false;
			}

			return true;
#else
			return sem_timedwait( &m_Semaphore, &AbsTime ) == 0;
#endif
		}
#endif
    }

	/**
	 * �����¼�Ϊ�ź�״̬��
	 *
	 * @return ���ִ�гɹ���Ϊtrue��������ִ�����Ϊfalse��
	 */
    bool Set()
    {
#ifdef _WIN32
        return SetEvent( m_hEvent ) != 0;
#else
		return sem_post( &m_Semaphore ) == 0;
#endif
    }

private:
#ifdef _WIN32
	HANDLE m_hEvent;
#else
	sem_t m_Semaphore;
#endif
}; // CEvent

/**
 * ��д����
 */
class CRwLock : public CNonCopyable
{
public:
	CRwLock()
	{
#ifdef _WIN32
		if( m_bWithSrwLock )
		{
			if( m_pfnInitializeSRWLock == NULL || m_pfnAcquireSRWLockExclusive == NULL || m_pfnReleaseSRWLockExclusive == NULL
				|| m_pfnAcquireSRWLockShared == NULL || m_pfnReleaseSRWLockShared == NULL )
			{			
				throw runtime_error( "System function missing!" );
			}

			m_pfnInitializeSRWLock( &m_SrwLock );
		}
		else
		{
			m_hNoReadesEvent = CreateEvent( NULL, TRUE, TRUE, NULL );
			if( m_hNoReadesEvent == NULL )
			{
				char szMessage[64];
				sprintf( szMessage, "[Error %u] Failed to create event!", (UINT32)GetLastError() );
				throw std::runtime_error( szMessage );
			}
			m_nReadeCount = 0;
			InitializeCriticalSection( &m_ReadLock );
			InitializeCriticalSection( &m_WriteLock );
		}
#else
		if( pthread_rwlock_init(&m_Rwlock, NULL) != 0 )
		{
			char szMessage[64];
			sprintf( szMessage, "[Error %d] Failed to pthread_rwlock_init!", errno );
			throw std::runtime_error( szMessage );
		}
#endif
	}

	~CRwLock()
	{
#ifdef _WIN32
		if( !m_bWithSrwLock )
		{
			CloseHandle( m_hNoReadesEvent );
			DeleteCriticalSection( &m_ReadLock );
			DeleteCriticalSection( &m_WriteLock );
		}
#else
		pthread_rwlock_destroy( &m_Rwlock );
#endif
	}

	/**
	 * ������������
	 */
	void ReadLock()
	{
#ifdef _WIN32
		if( m_bWithSrwLock )
		{
			m_pfnAcquireSRWLockShared( &m_SrwLock );
		}
		else
		{
			EnterCriticalSection( &m_WriteLock );
			EnterCriticalSection( &m_ReadLock );
			if( ++m_nReadeCount == 1 ) ResetEvent( m_hNoReadesEvent );
			LeaveCriticalSection( &m_ReadLock );
			LeaveCriticalSection( &m_WriteLock );
		}
#else
		if( pthread_rwlock_rdlock(&m_Rwlock) != 0 )
		{
			char szMessage[64];
			sprintf( szMessage, "[Error %d] Failed to pthread_rwlock_rdlock!", errno );
			throw std::runtime_error( szMessage );
		}
#endif
	}

	/**
	 * ������������
	 */ 
	void ReadUnlock()
	{
#ifdef _WIN32
		if( m_bWithSrwLock )
		{
			m_pfnReleaseSRWLockShared( &m_SrwLock );
		}
		else
		{
			EnterCriticalSection( &m_ReadLock );
			if( m_nReadeCount < 1 ) // bug?
			{
				LeaveCriticalSection( &m_ReadLock );
				throw runtime_error( "The read-write lock is bad!" );
			}

			if( --m_nReadeCount == 0 ) SetEvent( m_hNoReadesEvent );
			LeaveCriticalSection( &m_ReadLock );
		}
#else
		pthread_rwlock_unlock( &m_Rwlock );
#endif
	}

	/**
	 * ����д������
	 */
	void WriteLock()
	{
#ifdef _WIN32
		if( m_bWithSrwLock )
		{
			m_pfnAcquireSRWLockExclusive( &m_SrwLock );
		}
		else
		{
			EnterCriticalSection( &m_WriteLock );
			if( m_nReadeCount > 0 ) WaitForSingleObject( m_hNoReadesEvent, INFINITE );
		}
#else
		if( pthread_rwlock_wrlock(&m_Rwlock) != 0 )
		{
			char szMessage[64];
			sprintf( szMessage, "[Error %d] Failed to pthread_rwlock_wrlock!", errno );
			throw std::runtime_error( szMessage );
		}
#endif
	}

	/**
	 * ����д������
	 */
	void WriteUnlock()
	{
#ifdef _WIN32
		if( m_bWithSrwLock )
		{
			m_pfnReleaseSRWLockExclusive( &m_SrwLock );
		}
		else
		{
			LeaveCriticalSection( &m_WriteLock );
		}
#else
		pthread_rwlock_unlock( &m_Rwlock );
#endif
	}

private:
#ifdef _WIN32
	/**
	 * ����һЩ��̬�����ͱ����ĳ�ʼ����
	 */
	class CInitializer
	{
	public:
		CInitializer()
		{
			if( LOBYTE(LOWORD(GetVersion())) >= 6 ) // ����ϵͳ��Windows Vista����߰汾
			{
				m_hKernel32 = LoadLibrary( L"kernel32.dll" );
				if( m_hKernel32 != NULL )
				{
					m_pfnInitializeSRWLock = (SRWLOCK_FUNC)GetProcAddress( m_hKernel32, "InitializeSRWLock" );
					m_pfnAcquireSRWLockExclusive = (SRWLOCK_FUNC)GetProcAddress( m_hKernel32, "AcquireSRWLockExclusive" );
					m_pfnReleaseSRWLockExclusive = (SRWLOCK_FUNC)GetProcAddress( m_hKernel32, "ReleaseSRWLockExclusive" );
					m_pfnAcquireSRWLockShared = (SRWLOCK_FUNC)GetProcAddress( m_hKernel32, "AcquireSRWLockShared" );
					m_pfnReleaseSRWLockShared = (SRWLOCK_FUNC)GetProcAddress( m_hKernel32, "ReleaseSRWLockShared" );
				}
			}
		}

		~CInitializer()
		{
			if( m_hKernel32 != NULL ) FreeLibrary( m_hKernel32 );
		}
	};

	static CInitializer m_Initializer;

	/**
	 * ���ʹ��ϵͳ�ṩ�Ķ�д����Ϊtrue��
	 */
	static bool m_bWithSrwLock;

#ifdef __CYGWIN__
	typedef void *SRWLOCK;
	typedef SRWLOCK *PSRWLOCK;
#endif
	typedef VOID (WINAPI *SRWLOCK_FUNC)( PSRWLOCK SRWLock );

	static HMODULE m_hKernel32;
	static VOID (WINAPI *m_pfnInitializeSRWLock)( PSRWLOCK SRWLock );
	static SRWLOCK_FUNC m_pfnAcquireSRWLockExclusive;
	static SRWLOCK_FUNC m_pfnReleaseSRWLockExclusive;
	static SRWLOCK_FUNC m_pfnAcquireSRWLockShared;
	static SRWLOCK_FUNC m_pfnReleaseSRWLockShared;

	/**
	 * ����Windows Vista����߰汾�Ķ�д����
	 */
	SRWLOCK m_SrwLock;

	/**
	 * ����֪ͨ��ǰû������������¼���
	 */
	HANDLE m_hNoReadesEvent;

	/**
	 * ��ǰ�����˶�����������
	 */
	int m_nReadeCount;

	/**
	 * ��������������ٽ�������
	 */
	CRITICAL_SECTION m_ReadLock;

	/**
	 * ��������д�����ٽ�������
	 */
	CRITICAL_SECTION m_WriteLock;
#else
	pthread_rwlock_t m_Rwlock;
#endif
}; // CRwLock

/**
 * ��д���ػ��ߣ������Զ������ͽ�����
 */
class CRwLockGuard : public CNonCopyable
{
public:
	CRwLockGuard( CRwLock &RwLock, bool bDoWrite ) : m_RwLock( RwLock ), m_bDoWrite( bDoWrite )
	{
		if( m_bDoWrite ) m_RwLock.WriteLock();
		else m_RwLock.ReadLock();
	}

	~CRwLockGuard()
	{
		if( m_bDoWrite ) m_RwLock.WriteUnlock();
		else m_RwLock.ReadUnlock();
	}

private:
	CRwLock &m_RwLock;
	bool m_bDoWrite;
};

/**
 * �̶߳���
 */
class CThread : public CNonCopyable
{
public:	
	/**
	 * ���캯����
	 *
	 * @param nStackSize �̶߳�ջ�Ĵ�С�����Ϊ0���ʾʹ��ϵͳĬ��ֵ��
	 */
	CThread( unsigned int nStackSize = 0 );

	virtual ~CThread();

	/**
	 * �����̡߳�
	 *
	 * @return ���ִ�гɹ���Ϊtrue��������ִ�����Ϊfalse��
	 */
	bool Start();

	/**
	 * ��ֹ�̡߳�
	 *
	 * @return ���ִ�гɹ���Ϊtrue��
	 */
	bool Terminate();

	/**
	 * �ж��߳��Ƿ�����ִ���С�
	 */
	bool IsRunning() const { return m_bRunning; }
	
	/**
	 * �ж��߳��Ƿ�������ֹ��
	 */
	bool IsTerminating() const { return m_bTerminating; }
	
protected:
	/**
	 * �߳����е���ڡ�
	 */
	virtual void Run() = 0;

private:
	/**
	 * ��ƽ̨��ص��߳�API��ں�����
	 */
#if defined( _WIN32 ) // ����ϵͳ��Windows
	static unsigned int WINAPI ThreadProc( void *pParam );
#else
	static void *ThreadProc( void *pParam );
#endif
	
	/**
	 * �����̵߳Ļ������
	 */
	CMutex m_Mutex;

	/**
	 * ����߳�Ҫ��ֹ��Ϊtrue��
	 */
	volatile bool m_bTerminating;
	
	/**
	 * ����߳�����ִ������Ϊtrue��
	 */
	volatile bool m_bRunning;
	
	/**
	 * ��ƽ̨��ص��߳���������
	 */
#ifdef _WIN32
	HANDLE m_hThread;
#else
	pthread_t m_Thread;
#endif

	/**
	 * ��ջ��С��
	 */
	unsigned int m_nStackSize;
}; // CThread

/**
 * �ɶ�����ں����ļ��̶߳���
 */
class CSimpleThread : public CThread
{
public:	
	/**
	 * ���캯����
	 *
	 * @param pStartAddress Ҫִ�е��߳���ں�����ַ��
	 * @param pParameter ���ݸ��߳���ں����Ĳ�����
	 * @param nStackSize �̶߳�ջ�Ĵ�С�����Ϊ0���ʾʹ��ϵͳĬ��ֵ��
	 */
	CSimpleThread( void (*pStartAddress)(CThread *, void *), void *pParameter, unsigned int nStackSize = 0 )
		: CThread( nStackSize ), m_pStartAddress( pStartAddress ), m_pParameter( pParameter )
	{
	}

	virtual ~CSimpleThread()
	{
		Terminate();
	}

protected:
	virtual void Run()
	{
		if( m_pStartAddress != NULL ) m_pStartAddress( this, m_pParameter );
	}

private:
	void (*m_pStartAddress)(CThread *, void *);
	void *m_pParameter;
}; // CSimpleThread

