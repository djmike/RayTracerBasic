#pragma once

#include "cinder/Vector.h"
#include "raytracer/Ray.h"

namespace raytracer {

class Camera
{
public:
	Camera( uint16_t p_imageWidth, uint16_t p_imageHeight, double p_fovY );

	void	lookAt( const ci::Vec3d& p_position, const ci::Vec3d& p_target, const ci::Vec3d& p_up = ci::Vec3d::yAxis() );
	Ray		generateRay( uint16_t p_x, uint16_t p_y ) const;

protected:
	ci::Vec3d	m_position;
	ci::Vec3d	m_u, m_v, m_w;	// x, y, z coordinate frame vectors
	double		m_fovY, m_tanHalfFovX, m_tanHalfFovY;
	double		m_aspectRatio, m_halfImageWidth, m_halfImageHeight;
};

}