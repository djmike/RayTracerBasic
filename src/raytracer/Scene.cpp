#include "raytracer/Ray.h"
#include "raytracer/Scene.h"
#include "raytracer/Sphere.h"
#include "raytracer/Triangle.h"
#include "Logger.h"

#include <exception>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stack>
#include <vector>

using namespace std;

namespace raytracer {

void Scene::clear()
{
	m_outputFileName	= "scene-test.png";
	
	m_outputFilePath.clear();
	m_lights.clear();
	m_primitives.clear();
}

// Helper function for parsing scene file
bool readValues( stringstream& p_sstream, vector< float >& p_values, int p_expectedCount )
{
	float value = 0.0f;

	while ( p_sstream >> value )
	{
		p_values.push_back( value );
	}

	return ( p_values.size() == p_expectedCount );
}

// Helper function for parsing scene file
void outputErrorMessage( string p_command, const string& p_fileName, unsigned int p_line )
{
	Logger::log() << "<< ERROR >> Invalid " << p_command << " definition\n\tfile: " << p_fileName << "\n\tat line: " << p_line << endl;
}

void Scene::parseSceneFile( const std::string& p_fileName )
{
	Logger::log() << "Scene::parseSceneFile() " << p_fileName << endl;

	string currLine, cmd;
	
	ifstream sceneFile;
	sceneFile.open( p_fileName );

	uint32_t lineCount	= 0;
	uint32_t maxVerts	= 0;
	uint8_t	maxRecursionDepth = 5;

	// camera values
	ci::Vec3d cameraPosition;
	ci::Vec3d cameraTarget;
	ci::Vec3d cameraUp;
	double cameraFovY;

	// material properties
	ci::Vec4d ambient( 0.2, 0.2, 0.2, 1.0 );
	ci::Vec4d diffuse;
	ci::Vec4d specular;
	ci::Vec4d emissive;
	double shininess = 1.0;

	// light properties
	ci::Vec3d lightPosition;
	ci::Vec4d lightColor;
	ci::Vec3d lightAttenuation( 1.0, 0.0, 0.0 );

	// vertices
	vector< ci::Vec3d > vertices;

	if ( sceneFile.is_open() )
	{
		vector< float > values;
		stack< ci::Matrix44d > transformStack;
		transformStack.push( ci::Matrix44d::identity() );

		while ( sceneFile.good() )
		{
			getline( sceneFile, currLine );
			
			values.clear();
			++lineCount;

			if ( currLine.find_first_not_of( " \t\r\n" ) != string::npos && currLine[ 0 ] != '#' )
			{
				stringstream ss( currLine );

				ss >> cmd;

				if ( cmd == "size" )
				{
					ss >> m_width;
					ss >> m_height;
				}
				else if ( cmd == "maxdepth" )
				{
					ss >> maxRecursionDepth;
				}
				else if ( cmd == "output" )
				{
					ss >> m_outputFileName;
				}
				else if ( cmd == "camera" )
				{
					if ( readValues( ss, values, 10 ) )
					{
						cameraPosition	= ci::Vec3d( values[ 0 ], values[ 1 ], values[ 2 ] );
						cameraTarget	= ci::Vec3d( values[ 3 ], values[ 4 ], values[ 5 ] );
						cameraUp		= ci::Vec3d( values[ 6 ], values[ 7 ], values[ 8 ] );
						cameraFovY		= values[ 9 ];
					}
					else
					{
						outputErrorMessage( cmd, p_fileName, lineCount );
					}
				}
				else if ( cmd == "directional" )
				{
					if ( readValues( ss, values, 6 ) )
					{
						lightPosition	= ci::Vec3d( values[ 0 ], values[ 1 ], values[ 2 ] );
						lightColor		= ci::Vec4d( values[ 3 ], values[ 4 ], values[ 5 ], 1.0 );

						addLight( Light::create( lightPosition, lightColor, Light::Type::Directional ) );
					}
					else
					{
						outputErrorMessage( cmd, p_fileName, lineCount );
					}
				}
				else if ( cmd == "point" )
				{
					if ( readValues( ss, values, 6 ) )
					{
						lightPosition	= ci::Vec3d( values[ 0 ], values[ 1 ], values[ 2 ] );
						lightColor		= ci::Vec4d( values[ 3 ], values[ 4 ], values[ 5 ], 1.0 );

						addLight( Light::create( lightPosition, lightColor, Light::Type::Point ) );
					}
					else
					{
						outputErrorMessage( cmd, p_fileName, lineCount );
					}
				}
				else if ( cmd == "attenuation" )
				{
					if ( readValues( ss, values, 3 ) )
					{
						lightAttenuation = ci::Vec3d( values[ 0 ], values[ 1 ], values[ 2 ] );
					}
					else
					{
						outputErrorMessage( cmd, p_fileName, lineCount );
					}
				}
				else if ( cmd == "ambient" )
				{
					if ( readValues( ss, values, 3 ) )
					{
						ambient = ci::Vec4d( values[ 0 ], values[ 1 ], values[ 2 ], 1.0 );
					}
					else
					{
						outputErrorMessage( cmd, p_fileName, lineCount );
					}
				}
				else if ( cmd == "emission" )
				{
					if ( readValues( ss, values, 3 ) )
					{
						emissive = ci::Vec4d( values[ 0 ], values[ 1 ], values[ 2 ], 1.0 );
					}
					else
					{
						outputErrorMessage( cmd, p_fileName, lineCount );
					}
				}
				else if ( cmd == "diffuse" )
				{
					if ( readValues( ss, values, 3 ) )
					{
						diffuse = ci::Vec4d( values[ 0 ], values[ 1 ], values[ 2 ], 1.0 );
					}
					else
					{
						outputErrorMessage( cmd, p_fileName, lineCount );
					}
				}
				else if ( cmd == "specular" )
				{
					if ( readValues( ss, values, 3 ) )
					{
						specular = ci::Vec4d( values[ 0 ], values[ 1 ], values[ 2 ], 1.0 );
					}
					else
					{
						outputErrorMessage( cmd, p_fileName, lineCount );
					}
				}
				else if ( cmd == "shininess" )
				{
					if ( readValues( ss, values, 1 ) )
					{
						shininess = values[ 0 ];
					}
					else
					{
						outputErrorMessage( cmd, p_fileName, lineCount );
					}
				}
				else if ( cmd == "translate" )
				{
					if ( readValues( ss, values, 3 ) )
					{
						ci::Matrix44d translation = ci::Matrix44d::createTranslation( ci::Vec3d( values[ 0 ], values[ 1 ], values[ 2 ] ) );

						transformStack.top() *= translation;
					}
					else
					{
						outputErrorMessage( cmd, p_fileName, lineCount );
					}
				}
				else if ( cmd == "rotate" )
				{
					if ( readValues( ss, values, 4 ) )
					{
						ci::Matrix44d rotation = ci::Matrix44d::createRotation( ci::Vec3d( values[ 0 ], values[ 1 ], values[ 2 ] ), ci::toRadians( values[ 3 ] ) );

						transformStack.top() *= rotation;
					}
					else
					{
						outputErrorMessage( cmd, p_fileName, lineCount );
					}
				}
				else if ( cmd == "scale" )
				{
					if ( readValues( ss, values, 3 ) )
					{
						ci::Matrix44d scale = ci::Matrix44d::createScale( ci::Vec3d( values[ 0 ], values[ 1 ], values[ 2 ] ) );

						transformStack.top() *= scale;
					}
					else
					{
						outputErrorMessage( cmd, p_fileName, lineCount );
					}
				}
				else if ( cmd == "pushTransform" )
				{
					transformStack.push( transformStack.top() );
				}
				else if ( cmd == "popTransform" )
				{
					if ( transformStack.size() <= 1 )
					{
						Logger::log() << "<< ERROR >> attempt to pop transform stack that has a single or less element\n\tfile: " << p_fileName << "\n\tat line: " << lineCount << endl;
					}
					else
					{
						transformStack.pop();
					}
				}
				else if ( cmd == "sphere" )
				{
					if ( readValues( ss, values, 4 ) )
					{
						ci::Vec3d spherePosition( values[ 0 ], values[ 1 ], values[ 2 ] );

						PrimitiveRef sphere = Sphere::create( values[ 3 ], spherePosition );
						Material material;

						material.setAmbient( ambient );
						material.setEmissive( emissive );
						material.setDiffuse( diffuse );
						material.setSpecular( specular );
						material.setShininess( shininess );
						
						sphere->setId( lineCount );
						sphere->setMaterial( material );
						sphere->setTransform( transformStack.top() );

						addPrimitive( sphere );
					}
					else 
					{
						outputErrorMessage( cmd, p_fileName, lineCount );
					}
				}
				else if ( cmd == "maxverts" )
				{
					if ( readValues( ss, values, 1 ) )
					{
						maxVerts = static_cast< unsigned int >( values[ 0 ] );
						Logger::log() << "Num Vertices: " << maxVerts << endl;
					}
					else 
					{
						outputErrorMessage( cmd, p_fileName, lineCount );
					}
				}
				else if ( cmd == "vertex" )
				{
					if ( readValues( ss, values, 3 ) )
					{
						vertices.push_back( ci::Vec3d( values[ 0 ], values[ 1 ], values[ 2 ] ) );
					}
					else 
					{
						outputErrorMessage( cmd, p_fileName, lineCount );
					}
				}
				else if ( cmd == "tri" )
				{
					if ( readValues( ss, values, 3 ) )
					{
						PrimitiveRef tri	= Triangle::create( vertices[ values[ 0 ] ], vertices[ values[ 1 ] ], vertices[ values[ 2 ] ] );
						Material material;

						material.setAmbient( ambient );
						material.setEmissive( emissive );
						material.setDiffuse( diffuse );
						material.setSpecular( specular );
						material.setShininess( shininess );

						tri->setId( lineCount );
						tri->setMaterial( material );
						tri->setTransform( transformStack.top() );

						addPrimitive( tri );
					}
					else 
					{
						outputErrorMessage( cmd, p_fileName, lineCount );
					}
				}
				else
				{
					Logger::log() << "<< ERROR >> Unknown command " << cmd << "\n\tfile: " << p_fileName << "\n\tat line: " << lineCount << endl;
				}
			}
		}
	}
	else
	{
		string errMsg = "Unable to open scene file: "; 
		errMsg += p_fileName;

		Logger::log() << errMsg << std::endl;
		throw exception( errMsg.c_str() );
	}

	// Finished parsing, do some setup

	// setup camera
	m_camera	= Camera( m_width, m_height, cameraFovY );
	m_camera.lookAt( cameraPosition, cameraTarget, cameraUp );

	// set light attenuation for each light in scene
	// loop through each light and set attenuation
	for ( LightRef light : m_lights )
	{
		light->setAttenuationTerms( lightAttenuation.x, lightAttenuation.y, lightAttenuation.z );
	}

	// if a filename was not specified, use the filename
	// from the scene description file
	if ( m_outputFileName.empty() || m_outputFileName == "scene-test.png" )
	{
		// "D:\\Dev\\frameworks\\cinder\\cinder_apps\\SimpleRayTracer\\assets\\scenes\\scene1.test"
		string inputFileName( p_fileName );
		auto extensionIndex		= inputFileName.find_last_of( '.' );
		auto lastSlashIndex		= inputFileName.find_last_of( '\\' ) + 1;
		auto numChars			= extensionIndex - lastSlashIndex;
		string fileName			= inputFileName.substr( lastSlashIndex, numChars );
		m_outputFileName		= fileName + ".png";

		Logger::log() << "Output File Name: " << m_outputFileName << endl;
	}
}

} // namespace raytracer