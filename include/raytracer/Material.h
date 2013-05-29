#pragma once

#include "cinder/Vector.h"

namespace raytracer {

class Material
{
public:
	const ci::Vec4d& getDiffuse() const { return m_diffuse; }
	void setDiffuse( const ci::Vec4d& p_diffuse ) { m_diffuse = p_diffuse; }
	void setDiffuse( double p_red, double p_green, double p_blue ) { m_diffuse = ci::Vec4d( p_red, p_green, p_blue, 1.0 ); }

	const ci::Vec4d& getSpecular() const { return m_specular; }
	void setSpecular( const ci::Vec4d& p_specular ) { m_specular = p_specular; }
	void setSpecular( double p_red, double p_green, double p_blue ) { m_specular = ci::Vec4d( p_red, p_green, p_blue, 1.0 ); }

	const ci::Vec4d& getAmbient() const { return m_ambient; }
	void setAmbient( const ci::Vec4d& p_ambient ) { m_ambient = p_ambient; }
	void setAmbient( double p_red, double p_green, double p_blue ) { m_ambient = ci::Vec4d( p_red, p_green, p_blue, 1.0 ); }

	const ci::Vec4d& getEmissive() const { return m_emissive; }
	void setEmissive( const ci::Vec4d& p_emissive ) { m_emissive = p_emissive; }
	void setEmissive( double p_red, double p_green, double p_blue ) { m_emissive = ci::Vec4d( p_red, p_green, p_blue, 1.0 ); }

	const double& getShininess() const { return m_shininess; }
	void setShininess( double p_shininess ) { m_shininess = p_shininess; }

	const bool getRefract() const { return m_refract; }
	void setRefract( bool p_refract ) { m_refract = p_refract; }

protected:
	ci::Vec4d	m_ambient;
	ci::Vec4d	m_emissive;
	ci::Vec4d	m_diffuse;
	ci::Vec4d	m_specular;
	double		m_shininess;
	bool		m_refract;
};

} // namespace raytracer