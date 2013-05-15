#pragma once

#include "cinder/Vector.h"

namespace raytracer {

class Ray
{
public:
	explicit Ray(){}
	Ray( const ci::Vec3d& p_origin, const ci::Vec3d& p_direction ) :
		m_origin( p_origin ),
		m_direction( p_direction )
	{
	}

	const ci::Vec3d&	getOrigin() const { return m_origin; }
	const ci::Vec3d&	getDirection() const { return m_direction; }

	ci::Vec3d	getPointAtDistance( double p_distance ) const
	{
		return ( m_origin + m_direction * p_distance );
	}

protected:
	ci::Vec3d	m_origin;
	ci::Vec3d	m_direction;
};

} // namespace raytracer