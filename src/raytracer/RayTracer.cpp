#include "cinder/Color.h"
#include "cinder/ImageIo.h"
#include "raytracer/RayTracer.h"
#include "Logger.h"

namespace raytracer {

bool RayTracer::tryGetImage( ci::Surface& p_outImage ) const
{
	bool isCloned = false;
	//std::unique_lock< std::mutex > lock( m_imageMutex );

	//if ( lock.try_lock() )
	{
		p_outImage	= m_image.clone();
		isCloned	= true;
	}

	return isCloned;
}

void RayTracer::renderThreaded( const Scene& p_scene )
{
	const uint16_t w	= p_scene.getWidth();
	const uint16_t h	= p_scene.getHeight();
	const Camera& cam	= p_scene.getCamera();

	Logger::log() << "RayTracer::render()\n\tw: " << w << "\n\th: " << h << std::endl;

	ci::Surface outputImage( w, h, true, ci::SurfaceChannelOrder::RGBA );

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

			outputImage.setPixel( ci::Vec2i( x, y ), color );
		}
	}

	double endTime = ci::app::getElapsedSeconds();
	double totalTime = endTime - startTime;

	ci::fs::path outputFile = p_scene.getOutputFile();
	ci::writeImage( outputFile, outputImage, ci::ImageTarget::Options().quality( 1.0f ) );
	Logger::log() << "Render Complete!\n\tFile saved to: " << outputFile << "\n\tduration (seconds): " << totalTime << std::endl;

	/*ci::fs::path saveFilePath = ci::app::getSaveFilePath( outputFile );

	if ( !saveFilePath.empty() )
	{
		ci::writeImage( saveFilePath, outputImage, ci::ImageTarget::Options().quality( 1.0f ) );
		Logger::log() << "Render Complete!\n\tFile saved to: " << saveFilePath << std::endl;
	}*/
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

	ci::fs::path outputFile = p_scene.getOutputFile();
	ci::writeImage( outputFile, m_image, ci::ImageTarget::Options().quality( 1.0f ) );
	
	m_isRendering	= false;

	Logger::log() << "Render Complete!\n\tFile saved to: " << outputFile << "\n\tduration (seconds): " << totalTime << std::endl;

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

		// Reflectivity
		ci::Vec4d reflectionColor	= ci::Vec4d( 0.0, 0.0, 0.0, 1.0 );
		ci::Vec3d reflectedRayDirn	= ( p_ray.getDirection() - ( ( 2.0 * N.dot( p_ray.getDirection() ) ) * N ) ).normalized();
		ci::Vec3d reflectedRayOrig	= P + reflectedRayDirn * moveSlightly;
		Ray reflectedRay			= Ray( reflectedRayOrig, reflectedRayDirn );

		traceRay( reflectedRay, p_scene, reflectionColor, p_depth + 1 );

		p_color += ( primitive->getMaterial().getSpecular() * reflectionColor );
		p_color += ( primitive->getMaterial().getAmbient() + primitive->getMaterial().getEmissive() );
	}
}

} // namespace raytracer