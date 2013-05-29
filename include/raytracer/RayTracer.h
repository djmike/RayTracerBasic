#pragma once

#include "Concurrency.h"
#include "Scene.h"
#include "cinder/Surface.h"

#include <unordered_map>

namespace raytracer {

class RayMap
{
public:
	void	addRay( const ci::Vec3d& p_originPoint, const ci::Vec3d& p_intersectionPoint );
	void	dump();

protected:
	typedef std::vector< ci::Vec3d > OriginPoints;
	typedef OriginPoints::size_type OriginPointIndex;
	typedef std::unordered_multimap< OriginPointIndex, ci::Vec3d > IntersectionPoints;

	OriginPoints			m_originPoints;
	IntersectionPoints 		m_intersectionPoints;
};

class RayTracer
{
public:
	RayTracer() : 
		m_maxDepth( 5 ),
		m_isRendering( false )
	{
	}

	~RayTracer(){}

	void	render( const Scene& p_scene );
	void	renderThreaded( const Scene& p_scene );
	void	renderDebug( const Scene& p_scene, double p_pixelX, double p_pixelY );
	void	setMaxDepth( uint8_t p_maxDepth ) { m_maxDepth = p_maxDepth; }

	bool	getIsRendering() const { return m_isRendering; }
	bool	getImageCloneThreadSafe( ci::Surface& p_outImage ) const;
	void	getImageClone( ci::Surface& p_outImage ) const;

protected:
	mutable bool	m_isRendering;
	int8_t			m_maxDepth;
	ci::Surface		m_image;
	RenderThreadPool		m_threadPool;
	mutable std::mutex		m_imageMutex;

	friend class RenderTask;

	void	traceRay( const Ray& p_ray, const Scene& p_scene, ci::Vec4d& p_color, int p_depth );
	void	traceRayDebug( const Ray& p_ray, const Scene& p_scene, RayMap& p_rayMap, int p_depth );
};

class RenderTask
{
protected:
	int m_startX, m_startY, m_tileWidth, m_tileHeight;
	std::thread::id m_threadId;

	const Scene*		m_scene;
	RayTracer*			m_rayTracer;

public:
	RenderTask() :
		m_startX( 0 ),
		m_startY( 0 ),
		m_tileWidth( 1 ),
		m_tileHeight( 1 )
	{
		m_scene			= nullptr;
		m_rayTracer		= nullptr;
	}

	RenderTask( int p_startX, int p_startY, int p_tileWidth, int p_tileHeight, const Scene* p_scene, RayTracer* p_rayTracer ) :
		m_startX( p_startX ),
		m_startY( p_startY ),
		m_tileWidth( p_tileWidth ),
		m_tileHeight( p_tileHeight )
	{
		m_scene			= p_scene;
		m_rayTracer		= p_rayTracer;
	}

	void operator()() const
	{
		render();
	}

	void setThreadId( std::thread::id p_threadId )
	{ 
		m_threadId = p_threadId;
	}

	void render() const;
};

} // namespace raytracer