#include "raytracer/Concurrency.h"
#include "raytracer/RayTracer.h"
#include "Logger.h"

namespace raytracer {

void RenderThreadPool::threadFunc()
{
	while ( !m_done )
	{
		RenderTaskRef task;
		if ( m_taskQueue.tryPop( task ) )
		{
			task->setThreadId( std::this_thread::get_id() );
			(*task)();
		}
		else
		{
			std::this_thread::yield();
		}
	}
}

void RenderThreadPool::init( const unsigned p_numThreads )
{
	try
	{
		for ( unsigned i = 0; i < p_numThreads; ++i )
		{
			m_threads.push_back( std::thread( &RenderThreadPool::threadFunc, this ) );
			Logger::log() << "thread " << i << " id: " << m_threads.back().get_id() << std::endl;
		}
	}
	catch ( ... )
	{
		m_done	= true;
		throw;
	}

	m_isInitialized		= true;
	Logger::log() << "[RenderThreadPool] numThreads: " << m_threads.size() << std::endl;
}

void RenderThreadPool::addTask( const RenderTaskRef& p_task )
{
	m_taskQueue.push( p_task );
}

} // namespace raytracer