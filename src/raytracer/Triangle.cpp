#include "raytracer/Triangle.h"

namespace raytracer {

const ci::Vec3d& Triangle::getNormal( const ci::Vec3d& p_point ) const
{
	return m_faceNormal;
}

bool Triangle::intersect( const Ray& p_ray, IntersectionData& p_data ) const
{
	ci::Vec3d rayOrigin		= ( m_worldToModel * ci::Vec4d( p_ray.getOrigin(), 1.0 ) ).xyz();
	ci::Vec3d rayDirection	= ( m_worldToModel * ci::Vec4d( p_ray.getDirection() ) ).xyz();

	ci::Vec3d e1	= m_b - m_a;
	ci::Vec3d e2	= m_c - m_a;

	ci::Vec3d s1	= rayDirection.cross( e2 );
	double divisor	= s1.dot( e1 );

	if ( divisor == 0.0 )
	{
		return false;
	}

	double invDivisor	= 1.0 / divisor;

	// compute first barycentric coordinate
	ci::Vec3d d		= rayOrigin - m_a;
	double b1		= d.dot( s1 ) * invDivisor;

	if ( b1 < 0.0 || b1 > 1.0 )
	{
		return false;
	}

	// computer second barycentric coordinate
	ci::Vec3d s2	= d.cross( e1 );
	double b2		= rayDirection.dot( s2 ) * invDivisor;

	if ( b2 < 0.0 || ( b1 + b2 ) > 1.0 )
	{
		return false;
	}

	double t		= e2.dot( s2 ) * invDivisor;

	if ( t < 0.0 )
	{
		return false;
	}

	ci::Vec3d point		= rayOrigin + rayDirection * t;							// object space intersection point
	p_data.point		= ( m_modelToWorld * ci::Vec4d( point, 1.0 ) ).xyz();	// world space intersection point
	p_data.distance		= ( p_data.point - p_ray.getOrigin() ).length();		// distance from world space intersection point to world space ray origin
	
	ci::Vec3d N			= e1.cross( e2 );
	p_data.normal		= ( m_normalMatrix * N ).normalized();

	return true;
}

} // namespace raytracer