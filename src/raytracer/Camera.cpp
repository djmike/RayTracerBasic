#include "raytracer/Camera.h"
#include "cinder/CinderMath.h"

namespace raytracer {

Camera::Camera( uint16_t p_imageWidth, uint16_t p_imageHeight, double p_fovY )
{
	m_aspectRatio		= p_imageWidth / static_cast< double >( p_imageHeight );
	m_halfImageWidth	= p_imageWidth * 0.5;
	m_halfImageHeight	= p_imageHeight * 0.5;

	m_fovY				= ci::toRadians( p_fovY );
	m_tanHalfFovY		= ci::math< double >::tan( m_fovY * 0.5 );
	m_tanHalfFovX		= m_tanHalfFovY * m_aspectRatio;
}

void Camera::lookAt( const ci::Vec3d& p_position, const ci::Vec3d& p_target, const ci::Vec3d& p_up )
{
	m_position		= p_position;
	m_w				= ( p_position - p_target ).normalized();
	m_u				= p_up.cross( m_w ).normalized();
	m_v				= m_w.cross( m_u );
}

Ray Camera::generateRay( uint16_t p_x, uint16_t p_y ) const
{
	double x	= p_x + 0.5;
	double y	= p_y + 0.5;
	
	/*double tanFovX		= tan( m_fovY * 0.5 ) * m_aspectRatio;
	double alpha		= tanFovX * ( ( x - m_halfImageWidth ) / m_halfImageWidth );
	double beta			= tan( m_fovY * 0.5 ) * ( ( m_halfImageHeight - y ) / m_halfImageHeight );*/

	double alpha	= m_tanHalfFovX * ( ( x - m_halfImageWidth ) / m_halfImageWidth );
	double beta		= m_tanHalfFovY * ( ( m_halfImageHeight - y ) / m_halfImageHeight );

	ci::Vec3d rayDirection	= ( alpha * m_u + beta * m_v - m_w ).normalized();

	return Ray( m_position, rayDirection );
}

} // namespace raytracer