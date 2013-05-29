#include "raytracer/RayTracer.h"
#include "raytracer/Concurrency.h"
#include "cinder/Clipboard.h"
#include "cinder/Color.h"
#include "cinder/ImageIo.h"
#include "Logger.h"

#include <system_error>
#include <sstream>

namespace raytracer {

void RenderTask::render() const
{
	const Camera& cam	= m_scene->getCamera();
	int endX			= m_startX + m_tileWidth;
	int endY			= m_startY + m_tileHeight;

	for ( int y = m_startY; y < endY; ++y )
	{
		for ( int x = m_startX; x < endX; ++x )
		{
			ci::Vec4d pixelColor	= ci::Vec4d( 0.0, 0.0, 0.0, 1.0 );
			int8_t depth			= 0;
			Ray primaryRay			= cam.generateRay( x, y );

			m_rayTracer->traceRay( primaryRay, *m_scene, pixelColor, depth );

			pixelColor.x	= ci::math< double >::clamp( pixelColor.x );
			pixelColor.y	= ci::math< double >::clamp( pixelColor.y );
			pixelColor.z	= ci::math< double >::clamp( pixelColor.z );
			pixelColor.w	= ci::math< double >::clamp( pixelColor.w );

			ci::ColorA color( pixelColor.x, pixelColor.y, pixelColor.z, pixelColor.w );

			std::unique_lock< std::mutex > lock( m_rayTracer->m_imageMutex );
			m_rayTracer->m_image.setPixel( ci::Vec2i( x, y ), color );
			lock.unlock();
		}
	}
}

void RayTracer::getImageClone( ci::Surface& p_outImage ) const
{
	p_outImage	= m_image.clone();
}

bool RayTracer::getImageCloneThreadSafe( ci::Surface& p_outImage ) const
{
	bool isCloned = false;
	
	try
	{
		std::unique_lock< std::mutex > lock( m_imageMutex, std::try_to_lock );

		if ( lock.owns_lock() )
		{
			if ( m_image )
			{
				p_outImage	= m_image.clone();
				isCloned	= true;
			}
		}
	}
	catch ( const std::system_error& p_exc )
	{
		Logger::log() << p_exc.what() << std::endl;
	}

	//lock.unlock();

	if ( getIsRendering() && m_threadPool.isTaskQueueEmpty() )
	{
		m_isRendering = false;
		Logger::log() << "end render time: " << ci::app::getElapsedSeconds() << std::endl;
	}

	return isCloned;
}

void RayTracer::renderThreaded( const Scene& p_scene )
{
	if ( !m_threadPool.isInitialized() )
	{
		int numThreads = std::thread::hardware_concurrency();

		if ( numThreads == 0 )
		{
			numThreads = 1;
		}
		else
		{
			numThreads -= 1;
		}

		m_threadPool.init( numThreads );
	}

	auto numThreads		= m_threadPool.getNumThreads();
	m_image				= ci::Surface( p_scene.getWidth(), p_scene.getHeight(), true, ci::SurfaceChannelOrder::RGBA );
	m_isRendering		= true;

	// calculate number of tasks based on a 16x16 pixel grid
	ci::Vec2i imageSize		= m_image.getSize();
	uint32_t numTasksX		= imageSize.x / 16 + 1;
	uint32_t numTasksY		= imageSize.y / 16 + 1;

	Logger::log() << "start render time: " << ci::app::getElapsedSeconds() << std::endl;
	
	// populate renderpool with render tasks
	for ( unsigned y = 0; y < numTasksY; ++y )
	{
		for ( unsigned x = 0; x < numTasksX; ++x )
		{
			int startX	= x * 16;
			int startY	= y * 16;

			m_threadPool.addTask( std::make_shared< RenderTask >( RenderTask( startX, startY, 16, 16, &p_scene, this ) ) );
		}
	}
}

void RayTracer::render( const Scene& p_scene )
{
	const uint16_t w	= p_scene.getWidth();
	const uint16_t h	= p_scene.getHeight();
	const Camera& cam	= p_scene.getCamera();

	Logger::log() << "RayTracer::render()\n\tw: " << w << "\n\th: " << h << std::endl;

	//ci::Surface outputImage( w, h, true, ci::SurfaceChannelOrder::RGBA );
	m_image			= ci::Surface( w, h, true, ci::SurfaceChannelOrder::RGBA );
	m_isRendering	= true;

	double startTime = ci::app::getElapsedSeconds();

	for ( uint16_t y = 0; y < h; ++y )
	{
		for ( uint16_t x = 0; x < w; ++x )
		{
			ci::Vec4d pixelColor	= ci::Vec4d( 0.0, 0.0, 0.0, 1.0 );
			int8_t depth			= 0;
			Ray primaryRay			= cam.generateRay( x, y );

			traceRay( primaryRay, p_scene, pixelColor, depth );

			pixelColor.x	= ci::math< double >::clamp( pixelColor.x );
			pixelColor.y	= ci::math< double >::clamp( pixelColor.y );
			pixelColor.z	= ci::math< double >::clamp( pixelColor.z );
			pixelColor.w	= ci::math< double >::clamp( pixelColor.w );

			ci::ColorA color( pixelColor.x, pixelColor.y, pixelColor.z, pixelColor.w );

			m_image.setPixel( ci::Vec2i( x, y ), color );
		}
	}

	double endTime = ci::app::getElapsedSeconds();
	double totalTime = endTime - startTime;

	//ci::fs::path outputFile = p_scene.getOutputFile();
	//ci::writeImage( outputFile, m_image, ci::ImageTarget::Options().quality( 1.0f ) );
	
	m_isRendering	= false;

	//Logger::log() << "Render Complete!\n\tFile saved to: " << outputFile << "\n\tduration (seconds): " << totalTime << std::endl;
	Logger::log() << "Render Complete!\n\tduration (seconds): " << totalTime << std::endl;

	/*ci::fs::path saveFilePath = ci::app::getSaveFilePath( outputFile );

	if ( !saveFilePath.empty() )
	{
		ci::writeImage( saveFilePath, outputImage, ci::ImageTarget::Options().quality( 1.0f ) );
		Logger::log() << "Render Complete!\n\tFile saved to: " << saveFilePath << std::endl;
	}*/
}

void RayTracer::traceRay( const Ray& p_ray, const Scene& p_scene, ci::Vec4d& p_color, int p_depth )
{
	if ( p_depth > m_maxDepth )
	{
		return;
	}
	
	const double moveSlightly		= 0.00001;

	IntersectionData closestIntersection, testIntersection;
	closestIntersection.distance	= DBL_MAX;
	testIntersection.distance		= DBL_MAX;

	PrimitiveRef primitive			= nullptr;

	for ( decltype( p_scene.getNumPrimitives() ) i = 0; i < p_scene.getNumPrimitives(); ++i )
	{
		if ( p_scene.getPrimitive( i )->intersect( p_ray, testIntersection ) && testIntersection.distance < closestIntersection.distance )
		{
			primitive				= p_scene.getPrimitive( i );
			closestIntersection		= testIntersection;
		}
	}

	if ( primitive != nullptr )
	{
		ci::Vec3d	P	= closestIntersection.point;	// world space intersection point
		ci::Vec3d	N	= closestIntersection.normal;	// world space normal at intersection point

		for ( decltype( p_scene.getNumLights() ) i = 0; i < p_scene.getNumLights(); ++i )
		{
			const LightRef& light	= p_scene.getLight( i );
			double visibility		= 1.0;
			double lightDistance	= DBL_MAX;
			double attenuation		= 1.0;
			ci::Vec3d L				= light->getPosition();		// light direction for directional lights, light position for point lights

			if ( light->getType() == Light::Type::Point )
			{
				L					= L - P; // for point lights, the light direction is relative to the intersection point
				lightDistance		= ( P - light->getPosition() ).length();
				attenuation			= light->getAttenuation( lightDistance );
			}

			L.normalize();

			// shoot a shadow ray from intersection point to light
			// to test if point is in shadow
			ci::Vec3d shadowRayOrigin	= P + L * moveSlightly; // move the ray towards the light slightly to account for any numerical errors that might cause inadvertent self shadowing
			Ray shadowRay				= Ray( shadowRayOrigin, L );

			for ( decltype( p_scene.getNumPrimitives() ) j = 0; j < p_scene.getNumPrimitives(); ++j )
			{
				PrimitiveRef shadowPrimitive	= p_scene.getPrimitive( j );
				//testIntersection.distance		= DBL_MAX;

				if ( shadowPrimitive->intersect( shadowRay, testIntersection ) && testIntersection.distance > 0.0 && testIntersection.distance < lightDistance )
				{
					visibility	= 0.0;
					break;
				}
			}

			if ( visibility > 0.0 )
			{
				// do some lighting calculations
				ci::Vec3d E		= ( p_ray.getOrigin() - P ).normalized();
				ci::Vec3d H		= ( L + E ).normalized();
				double s		= primitive->getMaterial().getShininess();

				ci::Vec4d diffuse	= primitive->getMaterial().getDiffuse();
				diffuse				*= ( ci::math< double >::max( N.dot( L ), 0.0 ) );

				ci::Vec4d specular	= primitive->getMaterial().getSpecular();
				specular			*= ( ci::math< double >::pow( ci::math< double >::max( N.dot( H ), 0.0 ), s ) );

				p_color += ( ( light->getColor() * attenuation ) * ( diffuse + specular ) );
			}
		}

		if ( primitive->getMaterial().getRefract() )
		{
			const double NdotRd			= N.dot( p_ray.getDirection() );
			const bool isEntering		= NdotRd < 0.0;
			const ci::Vec3d normal		= ( isEntering ) ? N : -N;
			const double cosI			= -normal.dot( p_ray.getDirection() );
			
			const double n1			= ( isEntering ) ? 1.0 : 1.5;
			const double n2			= ( isEntering ) ? 1.5 : 1.0;
			const double n			= n1 / n2;
			const double sinT2		= n * n * ( 1.0 - cosI * cosI );

			double kr = 0.0;
			double kt = 1.0;

			// Schlick's approximation of the Fresnel Equation
			double cosX		= cosI;
			double r0		= ( n1 - n2 ) / ( n1 + n2 );
			
			r0 *= r0;

			if ( n1 > n2 )
			{
				const double n = n1 / n2;
				const double sinT2 = n * n * ( 1.0 - cosI * cosI );

				if ( sinT2 > 1.0 )
				{
					kr = 1.0;
					kt = 0.0;
				}
				else
				{
					cosX = sqrt( 1.0 - sinT2 );
				}
			}

			if ( kr == 0.0 )
			{
				const double x = 1.0 - cosX;
				kr = r0 + ( 1.0 - r0 ) * x * x * x * x * x;
				kt = 1.0 - kr;
			}

			if ( sinT2 > 1.0 )
			{
				// no refraction, total internal reflection
			}
			else
			{
				const double cosT		= ci::math< double >::sqrt( 1.0 - sinT2 );
				ci::Vec4d refrColor		= ci::Vec4d( 0.0, 0.0, 0.0, 1.0 );
				ci::Vec3d refrRayDirn	= ( n * p_ray.getDirection() + ( isEntering ? 1 : -1 ) * ( n * cosI - cosT ) * N ).normalized();
				ci::Vec3d refrRayOrig	= P + refrRayDirn * moveSlightly;
				Ray refrRay				= Ray( refrRayOrig, refrRayDirn );

				traceRay( refrRay, p_scene, refrColor, p_depth + 1 );

				p_color += ( kt * refrColor );
			}

			ci::Vec4d reflColor		= ci::Vec4d( 0.0, 0.0, 0.0, 1.0 );
			ci::Vec3d reflRayDirn	= ( p_ray.getDirection() + 2.0 * cosI * N ).normalized();
			ci::Vec3d reflRayOrig	= P + reflRayDirn * moveSlightly;
			Ray reflRay				= Ray( reflRayOrig, reflRayDirn );

			traceRay( reflRay, p_scene, reflColor, p_depth + 1 );

			p_color += ( kr * reflColor );
			
		}
		else
		{
			// Reflectivity
			ci::Vec4d reflectionColor	= ci::Vec4d( 0.0, 0.0, 0.0, 1.0 );
			ci::Vec3d reflectedRayDirn	= ( p_ray.getDirection() - ( ( 2.0 * N.dot( p_ray.getDirection() ) ) * N ) ).normalized();
			ci::Vec3d reflectedRayOrig	= P + reflectedRayDirn * moveSlightly;
			Ray reflectedRay			= Ray( reflectedRayOrig, reflectedRayDirn );

			traceRay( reflectedRay, p_scene, reflectionColor, p_depth + 1 );

			p_color += ( primitive->getMaterial().getSpecular() * reflectionColor );
			//p_color += ( primitive->getMaterial().getAmbient() + primitive->getMaterial().getEmissive() );
		}

		p_color += ( primitive->getMaterial().getAmbient() + primitive->getMaterial().getEmissive() );
	}
}

void RayMap::addRay( const ci::Vec3d& p_originPoint, const ci::Vec3d& p_intersectionPoint )
{
	OriginPointIndex index = m_originPoints.size();

	// search to see if the origin point is already in the list
	for ( OriginPointIndex i = 0; i < m_originPoints.size(); ++i )
	{
		if ( m_originPoints[ i ] == p_originPoint )
		{
			index	= i;
			break;
		}
	}

	// the origin point is not already in the list, so add it
	if ( index == m_originPoints.size() )
	{
		m_originPoints.push_back( p_originPoint );
		// index should now point to the added origin point
		assert( index == ( m_originPoints.size() - 1 ) );
	}

	m_intersectionPoints.insert( std::pair< OriginPointIndex, ci::Vec3d >( index, p_intersectionPoint ) );
}

void RayMap::dump()
{
	std::stringstream ssRayMap;

	for ( OriginPointIndex i = 0; i < m_originPoints.size(); ++i )
	{
		const ci::Vec3d& originPoint	= m_originPoints[ i ];
		auto intersectionPoints			= m_intersectionPoints.equal_range( i );
		
		for ( auto iter = intersectionPoints.first; iter != intersectionPoints.second; ++iter )
		{
			ssRayMap << originPoint << " : " << iter->second << std::endl;
			Logger::log() << originPoint << " : " << iter->second << std::endl;
		}
	}

	ci::Clipboard::setString( ssRayMap.str() );
}

void RayTracer::renderDebug( const Scene& p_scene, double p_pixelX, double p_pixelY )
{
	int8_t depth		= 0;
	Ray primaryRay		= p_scene.getCamera().generateRay( p_pixelX, p_pixelY );
	
	RayMap rayMap;
	traceRayDebug( primaryRay, p_scene, rayMap, depth );

	rayMap.dump();
}

void RayTracer::traceRayDebug( const Ray& p_ray, const Scene& p_scene, RayMap& p_rayMap, int p_depth )
{
	if ( p_depth > m_maxDepth )
	{
		return;
	}
	
	const double moveSlightly		= 0.00001;

	IntersectionData closestIntersection, testIntersection;
	closestIntersection.distance	= DBL_MAX;
	testIntersection.distance		= DBL_MAX;

	PrimitiveRef primitive			= nullptr;

	for ( decltype( p_scene.getNumPrimitives() ) i = 0; i < p_scene.getNumPrimitives(); ++i )
	{
		if ( p_scene.getPrimitive( i )->intersect( p_ray, testIntersection ) && testIntersection.distance < closestIntersection.distance )
		{
			primitive				= p_scene.getPrimitive( i );
			closestIntersection		= testIntersection;
		}
	}

	if ( primitive != nullptr )
	{
		ci::Vec3d	P	= closestIntersection.point;	// world space intersection point
		ci::Vec3d	N	= closestIntersection.normal;	// world space normal at intersection point

		p_rayMap.addRay( p_ray.getOrigin(), P );

		for ( decltype( p_scene.getNumLights() ) i = 0; i < p_scene.getNumLights(); ++i )
		{
			const LightRef& light	= p_scene.getLight( i );
			double visibility		= 1.0;
			double lightDistance	= DBL_MAX;
			double attenuation		= 1.0;
			ci::Vec3d L				= light->getPosition();		// light direction for directional lights, light position for point lights

			if ( light->getType() == Light::Type::Point )
			{
				L					= L - P; // for point lights, the light direction is relative to the intersection point
				lightDistance		= ( P - light->getPosition() ).length();
				attenuation			= light->getAttenuation( lightDistance );
			}

			L.normalize();

			// shoot a shadow ray from intersection point to light
			// to test if point is in shadow
			ci::Vec3d shadowRayOrigin	= P + L * moveSlightly; // move the ray towards the light slightly to account for any numerical errors that might cause inadvertent self shadowing
			Ray shadowRay				= Ray( shadowRayOrigin, L );

			for ( decltype( p_scene.getNumPrimitives() ) j = 0; j < p_scene.getNumPrimitives(); ++j )
			{
				PrimitiveRef shadowPrimitive	= p_scene.getPrimitive( j );
				//testIntersection.distance		= DBL_MAX;

				if ( shadowPrimitive->intersect( shadowRay, testIntersection ) && testIntersection.distance > 0.0 && testIntersection.distance < lightDistance )
				{
					visibility	= 0.0;
					p_rayMap.addRay( shadowRayOrigin, testIntersection.point );
					break;
				}
			}

			if ( visibility > 0.0 )
			{
				// do some lighting calculations
				ci::Vec3d E		= ( p_ray.getOrigin() - P ).normalized();
				ci::Vec3d H		= ( L + E ).normalized();
				double s		= primitive->getMaterial().getShininess();

				ci::Vec4d diffuse	= primitive->getMaterial().getDiffuse();
				diffuse				*= ( ci::math< double >::max( N.dot( L ), 0.0 ) );

				ci::Vec4d specular	= primitive->getMaterial().getSpecular();
				specular			*= ( ci::math< double >::pow( ci::math< double >::max( N.dot( H ), 0.0 ), s ) );

				p_rayMap.addRay( P, light->getPosition() );
			}
		}

		if ( primitive->getMaterial().getRefract() )
		{
			const double cosI		= -N.dot( p_ray.getDirection() );
			const bool isEntering	= cosI > 0.0;
			const double n1			= ( isEntering ) ? 1.0 : 1.5;
			const double n2			= ( isEntering ) ? 1.5 : 1.0;
			const double n			= n1 / n2;
			const double sinT2		= n * n * ( 1.0 - cosI * cosI );

			double kr = 0.0;
			double kt = 1.0;

			// Schlick's approximation of the Fresnel Equation
			double cosX		= cosI;
			double r0		= ( n1 - n2 ) / ( n1 + n2 );
			
			r0 *= r0;

			if ( n1 > n2 )
			{
				const double n = n1 / n2;
				const double sinT2 = n * n * ( 1.0 - cosI * cosI );

				if ( sinT2 > 1.0 )
				{
					kr = 1.0;
					kt = 0.0;
				}
				else
				{
					cosX = sqrt( 1.0 - sinT2 );
				}
			}

			if ( kr == 0.0 )
			{
				const double x = 1.0 - cosX;
				kr = r0 + ( 1.0 - r0 ) * x * x * x * x * x;
				kt = 1.0 - kr;
			}

			if ( sinT2 > 1.0 )
			{
				// no refraction, total internal reflection
			}
			else
			{
				const double cosT		= ci::math< double >::sqrt( 1.0 - sinT2 );
				ci::Vec4d refrColor		= ci::Vec4d( 0.0, 0.0, 0.0, 1.0 );
				ci::Vec3d refrRayDirn	= ( n * p_ray.getDirection() + ( isEntering ? 1 : -1 ) * ( n * cosI - cosT ) * N ).normalized();
				ci::Vec3d refrRayOrig	= P + refrRayDirn * moveSlightly;
				Ray refrRay				= Ray( refrRayOrig, refrRayDirn );

				traceRayDebug( refrRay, p_scene, p_rayMap, p_depth + 1 );
			}

			ci::Vec4d reflColor		= ci::Vec4d( 0.0, 0.0, 0.0, 1.0 );
			ci::Vec3d reflRayDirn	= ( p_ray.getDirection() + 2.0 * cosI * N ).normalized();
			ci::Vec3d reflRayOrig	= P + reflRayDirn * moveSlightly;
			Ray reflRay				= Ray( reflRayOrig, reflRayDirn );

			traceRayDebug( reflRay, p_scene, p_rayMap, p_depth + 1 );
		}
		else
		{
			// Reflectivity
			ci::Vec4d reflectionColor	= ci::Vec4d( 0.0, 0.0, 0.0, 1.0 );
			ci::Vec3d reflectedRayDirn	= ( p_ray.getDirection() - ( ( 2.0 * N.dot( p_ray.getDirection() ) ) * N ) ).normalized();
			ci::Vec3d reflectedRayOrig	= P + reflectedRayDirn * moveSlightly;
			Ray reflectedRay			= Ray( reflectedRayOrig, reflectedRayDirn );

			traceRayDebug( reflectedRay, p_scene, p_rayMap, p_depth + 1 );
		}
	}
}

} // namespace raytracer