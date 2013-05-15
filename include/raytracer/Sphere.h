#pragma once

#include "Primitive.h"

namespace raytracer {

class Sphere : public Primitive
{
public:
	Sphere( double p_radius, const ci::Vec3d& p_center ) :
		m_radius( p_radius ),
		m_center( p_center )
	{
	}

	virtual ~Sphere(){}

	static PrimitiveRef	create( double p_radius, const ci::Vec3d& p_center )
	{
		return PrimitiveRef( new Sphere( p_radius, p_center ) );
	}

	virtual bool intersect( const Ray& p_ray, IntersectionData& p_data ) const;
	virtual const ci::Vec3d& getNormal( const ci::Vec3d& p_point ) const;

protected:
	double		m_radius;
	ci::Vec3d	m_center;
};

} // namespace raytracer