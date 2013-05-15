#pragma once

#include <memory>
#include "cinder/Vector.h"

namespace raytracer {

class Light;
typedef std::shared_ptr< Light > LightRef;

class Light
{
public:
	enum class Type
	{
		Point,
		Directional
	};

	explicit Light() :
		m_type( Type::Point ),
		m_constantAtten( 1.0 ),
		m_linearAtten( 0.0 ),
		m_quadraticAtten( 0.0 )
	{
	}

	Light( const ci::Vec3d& p_position, const ci::Vec4d& p_color, Type p_type ) :
		m_position( p_position ),
		m_color( p_color ),
		m_type( p_type ),
		m_constantAtten( 1.0 ),
		m_linearAtten( 0.0 ),
		m_quadraticAtten( 0.0 )
	{
	}

	static LightRef		create( const ci::Vec3d& p_position, const ci::Vec4d& p_color, Type p_type )
	{
		return LightRef( new Light( p_position, p_color, p_type ) );
	}

	const ci::Vec3d&	getPosition() const { return m_position; }
	const ci::Vec4d&	getColor() const { return m_color; }
	Type				getType() const { return m_type; }

	double&		getConstantAtten() { return m_constantAtten; }
	double&		getLinearAtten() { return m_linearAtten; }
	double&		getQuadraticAtten() { return m_quadraticAtten; }

	void setAttenuationTerms( double p_constant, double p_linear, double p_quadratic )
	{
		m_constantAtten		= p_constant;
		m_linearAtten		= p_linear;
		m_quadraticAtten	= p_quadratic;
	}

	double getAttenuation( double p_distance ) const
	{
		if ( m_type == Type::Directional ) return 1.0;
		return ( 1.0 / ( m_constantAtten + m_linearAtten * p_distance + m_quadraticAtten * p_distance * p_distance ) );
	}

protected:
	ci::Vec3d	m_position;
	ci::Vec4d	m_color;
	double		m_constantAtten;
	double		m_linearAtten;
	double		m_quadraticAtten;
	Type		m_type;
};

} // namespace raytracer