#include "raytracer/Sphere.h"

namespace raytracer {

const ci::Vec3d& Sphere::getNormal( const ci::Vec3d& p_point ) const
{
	return ( p_point - m_center ).normalized();
}

//bool Sphere::intersect( const Ray& p_ray, double& p_distance ) const
bool Sphere::intersect( const Ray& p_ray, IntersectionData& p_data ) const
{
	// solve a sphere intersection by solving the quadratic equation
	// ax^2 + bx + c = 0
	// (-b +- sqrt( b^2 - 4ac )) / 2a
	//
	// version for a ray-sphere intersection:
	// t^2(P1 dot P1) + 2t(P1 dot (P0 - C)) + (P0 - C) dot (P0 - C) - r^2
	// where:
	// P1 = ray direction
	// P0 = ray origin
	// C = sphere center
	// r = sphere radius
	// t = points of intersection on sphere, x in the quadractic equation, variable being solved for
	//
	// note that the discriminant is important as it tells us whether the roots will be real or complex as a good first check

	// transform ray from world space into object/model space
	ci::Vec3d rayOrigin			= ( m_worldToModel * ci::Vec4d( p_ray.getOrigin(), 1.0 ) ).xyz();
	ci::Vec3d rayDirection		= ( m_worldToModel * ci::Vec4d( p_ray.getDirection() ) ).xyz();
	ci::Vec3d originToCenter	= rayOrigin - m_center; // ray origin - sphere center

	double a	= rayDirection.dot( rayDirection );
	double b	= 2.0 * rayDirection.dot( originToCenter );
	double c	= originToCenter.dot( originToCenter ) - ( m_radius * m_radius );
	double D	= b * b - 4.0 * a * c;

	if ( D < 0.0 )
	{
		return false;
	}
	else
	{
		D	= ci::math< double >::sqrt( D );

		double invDenom		= 1 / ( 2 * a );
		double r1			= ( -b - D ) * invDenom;
		double r2			= ( -b + D ) * invDenom;
		double t			= 0.0;

		if ( r1 < 0.0 && r2 < 0.0 )
		{
			return false;
		}
		else if ( r1 < r2 )
		{
			t	= ( r1 > 0.0 ) ? r1 : r2;
		}
		else if ( r2 < r1 )
		{
			t	= r2;
		}
		else if ( r1 == r2 )
		{
			t	= r1;
		}
		else
		{
			return false;
		}

		// calc point of transformed ray in world space
		ci::Vec3d point		= rayOrigin + rayDirection * t;							// intersection point in object space
		p_data.point		= ( m_modelToWorld * ci::Vec4d( point, 1.0 ) ).xyz();	// intersection point in world space
		p_data.distance		= ( p_data.point - p_ray.getOrigin() ).length();		// distance from world space intersection point to world space ray origin

		ci::Vec3d N			= ( point - m_center );						// normal in object space
		p_data.normal		= ( m_normalMatrix * N ).normalized();		// object space normal transformed into world space normal

		return true;
	}

	return false;
}

} // namespace raytracer