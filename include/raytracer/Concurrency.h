#pragma once

#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <queue>
#include <vector>

namespace raytracer {

class ThreadJoiner
{
protected:
	std::vector< std::thread >&		m_threads;

public:
	explicit ThreadJoiner( std::vector< std::thread >& p_threads ) :
		m_threads( p_threads )
	{
	}

	~ThreadJoiner()
	{
		for ( decltype( m_threads.size() ) i = 0; i < m_threads.size(); ++i )
		{
			if ( m_threads[ i ].joinable() )
			{
				m_threads[ i ].join();
			}
		}
	}
};

template< typename T >
class ThreadSafeQueue
{
protected:
	std::queue< T >				m_taskQueue;
	mutable std::mutex			m_mutex;
	std::condition_variable		m_conditionVariable;

public:
	void push( const T& p_task )
	{
		std::unique_lock< std::mutex > lock( m_mutex );
		m_taskQueue.push( p_task );
		lock.unlock();
		
		m_conditionVariable.notify_one();
	}

	bool empty() const
	{
		std::unique_lock< std::mutex > lock( m_mutex );
		return m_taskQueue.empty();
	}

	bool tryPop( T& p_taskOut )
	{
		std::unique_lock< std::mutex > lock( m_mutex );
		if ( m_taskQueue.empty() )
		{
			return false;
		}

		p_taskOut = m_taskQueue.front();
		m_taskQueue.pop();
		
		return true;
	}

	void waitPop( T& p_taskOut )
	{
		std::unique_lock< std::mutex > lock( m_mutex );

		while ( m_taskQueue.empty() )
		{
			m_conditionVariable.wait( lock );
		}

		p_taskOut = m_taskQueue.front();
		m_taskQueue.pop();
	}
};

class RenderTask;
typedef std::shared_ptr< RenderTask >		RenderTaskRef;
typedef ThreadSafeQueue< RenderTaskRef >	RenderTaskQueue;

class RenderThreadPool
{
protected:
	std::atomic_bool			m_done;
	std::atomic_bool			m_isInitialized;
	RenderTaskQueue				m_taskQueue;
	std::vector< std::thread >	m_threads;
	ThreadJoiner				m_joiner;

	void threadFunc();

public:
	RenderThreadPool() :
		m_joiner( m_threads )
	{
		// msvc is missing constructor for atomic_bool
		// so we must initialize by assignment
		m_done				= false;
		m_isInitialized		= false;
	}

	~RenderThreadPool()
	{
		m_done	= true;
		//std::cout << "[RenderThreadPool] DTOR" << std::endl;
	}

	bool isInitialized() const { return m_isInitialized; }
	std::vector< std::thread >::size_type getNumThreads() const { return m_threads.size(); }
	bool isTaskQueueEmpty() const { return m_taskQueue.empty(); }

	void init( const unsigned p_numThreads );
	void addTask( const RenderTaskRef& p_task );
};

} // namespace raytracer