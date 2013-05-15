#pragma once

#include "Primitive.h"
#include "cinder/TriMesh.h"

namespace raytracer {

class TriangleMesh : public Primitive
{
public:
	TriangleMesh( const ci::TriMesh& p_mesh ) :
		m_mesh( p_mesh )
	{
	}

	virtual ~TriangleMesh(){}

	static PrimitiveRef create( const ci::TriMesh& p_mesh )
	{
		return PrimitiveRef( new TriangleMesh( p_mesh ) );
	}

	virtual bool intersect( const Ray& p_ray, IntersectionData& p_data ) const;
	virtual const ci::Vec3d& getNormal( const ci::Vec3d& p_point ) const;

private:
	ci::TriMesh		m_mesh;
};

} // namespace raytracer