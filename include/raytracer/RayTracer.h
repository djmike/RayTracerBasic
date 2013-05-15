#pragma once

#include <mutex>
#include <thread>
#include "cinder/Surface.h"
#include "Scene.h"

namespace raytracer {

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
	void	setMaxDepth( uint8_t p_maxDepth ) { m_maxDepth = p_maxDepth; }

	bool	getIsRendering() const { return m_isRendering; }
	bool	tryGetImage( ci::Surface& p_outImage ) const;

protected:
	bool			m_isRendering;
	int8_t			m_maxDepth;
	ci::Surface		m_image;
	mutable std::mutex		m_imageMutex;

	void	traceRay( const Ray& p_ray, const Scene& p_scene, ci::Vec4d& p_color, int p_depth );
};

class RenderThreadPool
{
public:

};

class RenderTask
{
};

} // namespace raytracer