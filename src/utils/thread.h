#pragma once

#include "base.h"

/**
 * 信号灯对象。
 */
class CSemaphore : public CNonCopyable
{
public:
	/**
	 * 构造函数。
	 *
	 * @param uiInitialCount 初始的信号量。
	 * @param uiMaximumCount 最大允许的信号量。这个参数只有在Windows平台上有效。
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
	 * 减少信号量。
	 *
	 * @param nMilliseconds 等待的超时值，单位为毫秒。如果为0xFFFFFFFF则表示无限时等待。
	 * @return 如果等待成功则为true，如果等待超时或者出现错误则为false。
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
				if( errno == EINTR ) continue; // gdb产生信号
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
	 * 增加信号量。
	 *
	 * @return 如果执行成功则为true，如果出现错误则为false。
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
 * 信号灯对象守护者，用于自动减少和增加信号量。
 */
class CSemaphoreGuard : public CNonCopyable
{
public:
	/**
	 * 构造函数。
	 *
	 * @param Semaphore 要守护的信号灯对象。
	 * @param nMilliseconds 等待的超时值，单位为毫秒。如果为0xFFFFFFFF则表示无限时等待。
	 * @return 如果等待成功则为true，如果等待超时或者出现错误则为false。
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
	 * 判断是否成功减少信号量。
	 */
	bool IsOk() const { return m_bIsOk; }

private:
	CSemaphore &m_Semaphore;
	bool m_bIsOk;
};

/**
 * 互斥对象。
 */
class CMutex : public CNonCopyable
{
public:
	/**
	 * 构造函数。
	 *
	 * @param bRecursive 指出是否要设置递归特性。
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
	 * 为当前线程获取所有权。
	 *
	 * @return 如果执行成功则为true，如果出现错误则为false。
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
	 * 为当前线程尝试获取所有权。
	 *
	 * @return 如果获取所有权成功则为true，如果出现错误或者其它线程已经获取了所有权则为false。
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
	 * 为当前线程释放所有权。
	 *
	 * @return 如果执行成功则为true，如果出现错误则为false。
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
 * 互斥对象守护者，用于自动获取和释放所有权。
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
 * 事件对象。
 */
class CEvent : public CNonCopyable
{
public:
	/**
	 * 构造函数。
	 *
	 * @param bInitialState 指出是否要设置事件为信号状态。
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
	 * 等待事件为信号状态。
	 *
	 * @param nMilliseconds 等待的超时值，单位为毫秒。如果为0xFFFFFFFF则表示无限时等待。
	 * @return 如果等待成功则为true，如果出现错误或者等待超时则为false。
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
				if( errno == EINTR ) continue; // gdb产生信号
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
	 * 设置事件为信号状态。
	 *
	 * @return 如果执行成功则为true，如果出现错误则为false。
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
 * 读写锁。
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
	 * 锁定读操作。
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
	 * 解锁读操作。
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
	 * 锁定写操作。
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
	 * 解锁写操作。
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
	 * 用于一些静态函数和变量的初始化。
	 */
	class CInitializer
	{
	public:
		CInitializer()
		{
			if( LOBYTE(LOWORD(GetVersion())) >= 6 ) // 操作系统是Windows Vista或更高版本
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
	 * 如果使用系统提供的读写锁则为true。
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
	 * 用于Windows Vista或更高版本的读写锁。
	 */
	SRWLOCK m_SrwLock;

	/**
	 * 用于通知当前没有请求读锁的事件。
	 */
	HANDLE m_hNoReadesEvent;

	/**
	 * 当前请求了读锁的数量。
	 */
	int m_nReadeCount;

	/**
	 * 用于请求读锁的临界区对象。
	 */
	CRITICAL_SECTION m_ReadLock;

	/**
	 * 用于请求写锁的临界区对象。
	 */
	CRITICAL_SECTION m_WriteLock;
#else
	pthread_rwlock_t m_Rwlock;
#endif
}; // CRwLock

/**
 * 读写锁守护者，用于自动锁定和解锁。
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
 * 线程对象。
 */
class CThread : public CNonCopyable
{
public:	
	/**
	 * 构造函数。
	 *
	 * @param nStackSize 线程堆栈的大小，如果为0则表示使用系统默认值。
	 */
	CThread( unsigned int nStackSize = 0 );

	virtual ~CThread();

	/**
	 * 启动线程。
	 *
	 * @return 如果执行成功则为true，如果出现错误则为false。
	 */
	bool Start();

	/**
	 * 终止线程。
	 *
	 * @return 如果执行成功则为true。
	 */
	bool Terminate();

	/**
	 * 判断线程是否正在执行中。
	 */
	bool IsRunning() const { return m_bRunning; }
	
	/**
	 * 判断线程是否正在终止。
	 */
	bool IsTerminating() const { return m_bTerminating; }
	
protected:
	/**
	 * 线程运行的入口。
	 */
	virtual void Run() = 0;

private:
	/**
	 * 与平台相关的线程API入口函数。
	 */
#if defined( _WIN32 ) // 操作系统是Windows
	static unsigned int WINAPI ThreadProc( void *pParam );
#else
	static void *ThreadProc( void *pParam );
#endif
	
	/**
	 * 操作线程的互斥对象。
	 */
	CMutex m_Mutex;

	/**
	 * 如果线程要终止则为true。
	 */
	volatile bool m_bTerminating;
	
	/**
	 * 如果线程正在执行中则为true。
	 */
	volatile bool m_bRunning;
	
	/**
	 * 与平台相关的线程描述符。
	 */
#ifdef _WIN32
	HANDLE m_hThread;
#else
	pthread_t m_Thread;
#endif

	/**
	 * 堆栈大小。
	 */
	unsigned int m_nStackSize;
}; // CThread

/**
 * 可定制入口函数的简单线程对象。
 */
class CSimpleThread : public CThread
{
public:	
	/**
	 * 构造函数。
	 *
	 * @param pStartAddress 要执行的线程入口函数地址。
	 * @param pParameter 传递给线程入口函数的参数。
	 * @param nStackSize 线程堆栈的大小，如果为0则表示使用系统默认值。
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

