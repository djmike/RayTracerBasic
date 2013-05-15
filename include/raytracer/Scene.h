#pragma once

#include <string>
#include <vector>
#include "cinder/Filesystem.h"
#include "cinder/Vector.h"
#include "Camera.h"
#include "Light.h"
#include "Primitive.h"

namespace raytracer {

class Scene
{
public:
	Scene() :
		m_width( 640 ),
		m_height( 480 ),
		m_outputFileName( "scene-test.png" ),
		m_camera( m_width, m_height, 60.0 )
	{
		m_camera.lookAt( ci::Vec3d::zero(), ci::Vec3d( 0.0, 0.0, -1.0 ) );
	}

	~Scene(){}

	void parseSceneFile( const std::string& p_fileName );
	void clear();

	uint16_t		getWidth() const { return m_width; }
	void			setWidth( uint16_t p_width ) { m_width = p_width; }

	uint16_t		getHeight() const { return m_height; }
	void			setHeight( uint16_t p_height ) { m_height = p_height; }

	const Camera&	getCamera() const { return m_camera; }
	void			setCamera( const Camera& p_camera ) { m_camera = p_camera; }

	const ci::fs::path	getOutputFile() const { return ( m_outputFilePath / m_outputFileName ); }

	const std::string&	getOutputFileName() const { return m_outputFileName; }
	void				setOutputFileName( const std::string& p_fileName ) { m_outputFileName = p_fileName; }

	const ci::fs::path&		getOutputFilePath() const { return m_outputFilePath; }
	void					setOutputFilePath( const ci::fs::path p_outputFilePath ) { m_outputFilePath = p_outputFilePath; }

	std::vector< LightRef >::size_type		getNumLights() const { return m_lights.size(); }
	std::vector< PrimitiveRef >::size_type	getNumPrimitives() const { return m_primitives.size(); }

	void				addLight( const LightRef& p_light ) { m_lights.push_back( p_light ); }
	const LightRef&		getLight( std::vector< LightRef >::size_type p_index ) const { return m_lights[ p_index ]; }

	void					addPrimitive( const PrimitiveRef& p_primitive ) { m_primitives.push_back( p_primitive ); }
	const PrimitiveRef&		getPrimitive( std::vector< PrimitiveRef >::size_type p_index ) const { return m_primitives[ p_index ]; }

private:
	std::vector< LightRef >			m_lights;
	std::vector< PrimitiveRef >		m_primitives;

	Camera			m_camera;
	uint16_t		m_width;
	uint16_t		m_height;
	std::string		m_outputFileName;
	ci::fs::path	m_outputFilePath;
};

} // namespace raytracer