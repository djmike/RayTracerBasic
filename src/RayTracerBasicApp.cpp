#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/ip/Fill.h"
#include "cinder/params/Params.h"
#include "cinder/ImageIo.h"
#include "cinder/ObjLoader.h"
#include "cinder/System.h"
#include "cinder/Utilities.h"

#include "raytracer/Scene.h"
#include "raytracer/RayTracer.h"
#include "raytracer/TriangleMesh.h"

#include <thread>

using namespace ci;
using namespace ci::app;
using namespace ci::params;
using namespace std;

namespace rt = raytracer;

class RayTracerBasicApp : public AppNative
{
public:
	void prepareSettings( Settings* p_settings );
	void setup();
	void setupRenderImageView();
	void setupWindow();
	void setupGuiMain();
	void update();

	void loadSceneFile();
	void reloadSceneFile();
	void renderScene();
	void saveRenderedImage();

	void onWindowDraw();
	void onWindowResize();
	void onWindowKeyUp( KeyEvent& p_evt );
	void onWindowKeyDown( KeyEvent& p_evt );
	void onWindowMouseUp( MouseEvent& p_evt );
	void onWindowMouseDown( MouseEvent& p_evt );
	void onWindowMouseDrag( MouseEvent& p_evt );
	void onWindowMouseWheel( MouseEvent& p_evt );

	void onGuiLoadSceneFile();
	void onGuiReloadSceneFile();
	void onGuiRenderScene();
	void onGuiSaveRenderedImage();

private:
	fs::path			m_sceneFilePath;
	string				m_sceneFileName;
	InterfaceGlRef		m_guiMain;

	rt::Scene			m_scene;
	rt::RayTracer		m_rayTracer;
	Surface				m_renderedImage;
	gl::TextureRef		m_renderedImageTex;
};

void RayTracerBasicApp::prepareSettings( Settings* p_settings )
{
	Window::Format fmt;

	fmt.setSize( 1280, 720 );
	fmt.setTitle( "Ray Tracer Basic" );

	p_settings->prepareWindow( fmt );
	p_settings->setFrameRate( 60.0f );
	p_settings->enableConsoleWindow();
}

void RayTracerBasicApp::setup()
{
	setupWindow();
	setupGuiMain();
	setupRenderImageView();

	console() << "Num threads: " << thread::hardware_concurrency() << endl;
	console() << "Cinder System Num Cores/Threads: " << System::getNumCores() << ", Num CPUs: " << System::getNumCpus() << endl;
}

void RayTracerBasicApp::setupRenderImageView()
{
	m_renderedImage		= Surface( m_scene.getWidth(), m_scene.getHeight(), true, SurfaceChannelOrder::RGBA );
	
	ip::fill( &m_renderedImage, ColorA::black() );
	
	m_renderedImageTex	= gl::Texture::create( m_renderedImage );
}

void RayTracerBasicApp::setupGuiMain()
{
	m_guiMain	= InterfaceGl::create( getWindow()->getTitle(), Vec2i( 250, 200 ), ColorA::gray( 0.2f, 0.6f ) );

	m_guiMain->addParam( "Scene File", &m_sceneFileName, "group='Scene Management'", true );
	m_guiMain->addButton( "Load Scene File", std::bind( &RayTracerBasicApp::onGuiLoadSceneFile, this ), "group='Scene Management'" );
	m_guiMain->addButton( "Reload Scene File", std::bind( &RayTracerBasicApp::onGuiReloadSceneFile, this ), "group='Scene Management'" );
	m_guiMain->addButton( "Render", std::bind( &RayTracerBasicApp::onGuiRenderScene, this ), "group='Scene Management'" );
	m_guiMain->addButton( "Save Image", std::bind( &RayTracerBasicApp::onGuiSaveRenderedImage, this ), "group='Scene Management'" );

	// TODO: add elements for rendering progress
	// TODO: add elements for displaying pixel info when mouse is over renderer image pixels
}

void RayTracerBasicApp::loadSceneFile()
{
	m_sceneFilePath		= getOpenFilePath( getAssetPath( "" ) );

	if ( !m_sceneFilePath.empty() )
	{
		console() << "Scene File Path: " << m_sceneFilePath << endl;
		m_sceneFileName = m_sceneFilePath.filename().string();

		m_scene.clear();
		m_scene.parseSceneFile( m_sceneFilePath.string() );
	}
}

void RayTracerBasicApp::reloadSceneFile()
{
	if ( !m_sceneFilePath.empty() )
	{
		m_scene.clear();
		m_scene.parseSceneFile( m_sceneFilePath.string() );
	}
}

void RayTracerBasicApp::renderScene()
{
	if ( !m_sceneFilePath.empty() )
	{
		//m_renderedImage			= Surface( m_scene.getWidth(), m_scene.getHeight(), true, SurfaceChannelOrder::RGBA );
		//m_renderedImageTex		= gl::Texture::create( m_renderedImage );

		m_scene.setOutputFilePath( getAssetPath( "" ) );
		m_rayTracer.render( m_scene );

		m_rayTracer.getImageCloneThreadSafe( m_renderedImage );
		m_renderedImageTex			= gl::Texture::create( m_renderedImage );

		/*rt::Scene scene;
		rt::RayTracer rayTracer;

		scene.setOutputFilePath( getAssetPath( "" ) );
		scene.parseSceneFile( m_sceneFilePath.string() );
		rayTracer.render( scene );*/
	}
	else
	{
		// TODO: display error message that a scene is not loaded
		// does Cinder have a MessageBox method?

		/*rt::Scene scene;
		rt::RayTracer rayTracer;
		rt::Camera renderCam( 128, 72, 60.0 );
		renderCam.lookAt( m_camera.getEyePoint(), m_cameraTarget );

		TriMesh plane;
		
		rt::Material sphereMat;
		sphereMat.setAmbient( 0.2, 0.2, 0.2 );
		sphereMat.setEmissive( 0.0, 0.0, 0.0 );
		sphereMat.setDiffuse( 1.0, 0.0, 0.0 );
		sphereMat.setSpecular( 1.0, 1.0, 1.0 );
		sphereMat.setShininess( 128.0 );
		
		rt::PrimitiveRef sphere = rt::TriangleMesh::create( m_objects[ 0 ] );
		sphere->setMaterial( sphereMat );

		scene.setWidth( 128 );
		scene.setHeight( 72 );
		scene.setCamera( renderCam );
		scene.addLight( rt::Light::create( m_camera.getEyePoint(), ci::Vec4d( 1.0, 1.0, 1.0, 1.0 ), rt::Light::Type::Point ) );
		scene.addPrimitive( sphere );
		scene.setOutputFileName( "scene-sphere-test.png" );
		scene.setOutputFilePath( getAssetPath( "" ) );
		rayTracer.render( scene );*/
	}
}

void RayTracerBasicApp::saveRenderedImage()
{
	fs::path saveFilePath	= getSaveFilePath( getAssetPath( "" ) );

	if ( !saveFilePath.empty() )
	{
		writeImage( saveFilePath, m_renderedImage, ImageTarget::Options().quality( 1.0f ) );
		cout << "Rendered Image Save to: " << saveFilePath << endl;
	}
}

void RayTracerBasicApp::update()
{
	
}

void RayTracerBasicApp::setupWindow()
{
	getWindow()->connectDraw( &RayTracerBasicApp::onWindowDraw, this );
	getWindow()->connectResize( &RayTracerBasicApp::onWindowResize, this );
	getWindow()->connectKeyUp( &RayTracerBasicApp::onWindowKeyUp, this );
	getWindow()->connectKeyDown( &RayTracerBasicApp::onWindowKeyDown, this );
	getWindow()->connectMouseUp( &RayTracerBasicApp::onWindowMouseUp, this );
	getWindow()->connectMouseDown( &RayTracerBasicApp::onWindowMouseDown, this );
	getWindow()->connectMouseDrag( &RayTracerBasicApp::onWindowMouseDrag, this );
	getWindow()->connectMouseWheel( &RayTracerBasicApp::onWindowMouseWheel, this );
}

void RayTracerBasicApp::onWindowDraw()
{
	gl::clear();
	gl::setMatricesWindow( toPixels( getWindowSize() ) );

	if ( m_renderedImageTex )
	{
		Vec2i windowSize	= toPixels( getWindowSize() );
		int xPos			= windowSize.x / 2 - m_renderedImageTex->getWidth() / 2;
		int yPos			= windowSize.y / 2 - m_renderedImageTex->getHeight() / 2;

		gl::color( ColorA::white() );
		gl::draw( m_renderedImageTex, Vec2f( xPos, yPos ) );

		gl::color( ColorA::white() );
		gl::drawStrokedRect( Rectf( xPos, yPos, xPos + m_renderedImageTex->getWidth(), yPos + m_renderedImageTex->getHeight() ) );
	}

	m_guiMain->draw();
}

void RayTracerBasicApp::onWindowResize()
{
}

void RayTracerBasicApp::onWindowKeyUp( KeyEvent& p_evt )
{
}

void RayTracerBasicApp::onWindowKeyDown( KeyEvent& p_evt )
{
}

void RayTracerBasicApp::onWindowMouseUp( MouseEvent& p_evt )
{
}

void RayTracerBasicApp::onWindowMouseDown( MouseEvent& p_evt )
{
}

void RayTracerBasicApp::onWindowMouseDrag( MouseEvent& p_evt )
{
}

void RayTracerBasicApp::onWindowMouseWheel( MouseEvent& p_evt )
{
}

void RayTracerBasicApp::onGuiLoadSceneFile()
{
	loadSceneFile();
}

void RayTracerBasicApp::onGuiReloadSceneFile()
{
	reloadSceneFile();
}

void RayTracerBasicApp::onGuiRenderScene()
{
	renderScene();
}

void RayTracerBasicApp::onGuiSaveRenderedImage()
{
	saveRenderedImage();
}

CINDER_APP_NATIVE( RayTracerBasicApp, RendererGl )
