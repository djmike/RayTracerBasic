#include "raytracer/TriangleMesh.h"
#include "raytracer/Triangle.h"

namespace raytracer {

const ci::Vec3d& TriangleMesh::getNormal( const ci::Vec3d& p_point ) const
{
	return ci::Vec3d::zero();
}

bool TriangleMesh::intersect( const Ray& p_ray, IntersectionData& p_data ) const
{
	bool doesIntersect = false;
	size_t numTris = m_mesh.getNumTriangles();

	for ( size_t i = 0; i < numTris; ++i )
	{
		ci::Vec3f a, b, c;
		m_mesh.getTriangleVertices( i, &a, &b, &c );

		PrimitiveRef tri = Triangle::create( a, b, c );
		if ( tri->intersect( p_ray, p_data ) )
		{
			doesIntersect	= true;
		}
	}

	return doesIntersect;
}

} // namespace raytracer