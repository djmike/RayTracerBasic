#pragma once

#include <memory>
#include "cinder/Matrix.h"
#include "cinder/Vector.h"
#include "Material.h"
#include "Ray.h"

namespace raytracer {

class Primitive;
typedef std::shared_ptr< Primitive > PrimitiveRef;

struct IntersectionData
{
	//const PrimitiveRef	primitive;
	ci::Vec3d	point;
	ci::Vec3d	normal;
	double		distance;
	double		epsilon;
};

class Primitive
{
public:
	virtual ~Primitive(){}

	virtual bool intersect( const Ray& p_ray, IntersectionData& p_data ) const = 0;
	virtual const ci::Vec3d& getNormal( const ci::Vec3d& p_point ) const = 0;

	uint32_t	getId() const { return m_id; }
	void		setId( uint32_t p_id ) { m_id = p_id; }

	const Material&		getMaterial() const { return m_material; }
	void				setMaterial( const Material& p_material ) { m_material = p_material; }

	void setTransform( const ci::Matrix44d& p_modelToWorld )
	{
		m_modelToWorld		= p_modelToWorld;
		m_worldToModel		= m_modelToWorld.inverted();
		m_normalMatrix		= m_worldToModel.transposed().subMatrix33( 0, 0 );
	}

protected:
	ci::Matrix44d	m_modelToWorld;
	ci::Matrix44d	m_worldToModel;
	ci::Matrix33d	m_normalMatrix;
	
	Material		m_material;
	uint32_t		m_id;

	Primitive(){}
};

} // namespace raytracer