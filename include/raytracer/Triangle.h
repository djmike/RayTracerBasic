#pragma once

#include "Primitive.h"

namespace raytracer {

class Triangle : public Primitive
{
public:
	Triangle( const ci::Vec3d p_a, const ci::Vec3d p_b, const ci::Vec3d p_c ) :
		m_a( p_a ),
		m_b( p_b ),
		m_c( p_c )
	{
	}

	virtual ~Triangle(){}

	static PrimitiveRef create( const ci::Vec3d& p_a, const ci::Vec3d& p_b, const ci::Vec3d& p_c )
	{ 
		return PrimitiveRef( new Triangle( p_a, p_b, p_c ) );
	}

	virtual bool intersect( const Ray& p_ray, IntersectionData& p_data ) const;
	virtual const ci::Vec3d& getNormal( const ci::Vec3d& p_point ) const;

private:
	ci::Vec3d	m_a;
	ci::Vec3d	m_b;
	ci::Vec3d	m_c;
	ci::Vec3d	m_faceNormal;

	void calcFaceNormal();
};

} // namespace raytracer