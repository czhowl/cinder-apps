#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"

#include "Particle.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class ParticlesApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    void keyDown( KeyEvent event ) override;
    
    void render();
    
  private:
    Particle           p;
    CameraPersp        mCamera;
    CameraUi           mCamUi;
    gl::FboRef         mFbo;
};

void ParticlesApp::setup()
{
    gl::clear( ColorA( 0, 0, 0 ) );
    p.setup(ColorA(1.0f, 1.0f, 1.0f, 1.0f));
    
    const vec2 windowSize = toPixels( getWindowSize() );
//    mCamera = CameraPersp( windowSize.x, windowSize.y, 45.0f, 0.01f, 100.0f );
//    mCamera.lookAt( vec3( -100.0f, 100.0f, 0.0f ), vec3( 0.0f, 2.0f, 0.0f ) );
    mCamUi = CameraUi( &mCamera, getWindow() );
    
    gl::Fbo::Format fboFormat;
    fboFormat.setColorTextureFormat( gl::Texture2d::Format().internalFormat( GL_RGBA32F ) );
    mFbo = gl::Fbo::create( getWindowWidth(), getWindowHeight(), fboFormat );
    
    // Clear once at setup.
    gl::ScopedFramebuffer scpFbo( mFbo );
    gl::ScopedViewport    scpViewport( mFbo->getSize() );
    gl::clear();
}

void ParticlesApp::mouseDown( MouseEvent event )
{
}

void ParticlesApp::keyDown( KeyEvent event )
{
    if( event.getChar() == 'c' ){
        gl::ScopedFramebuffer scpFbo( mFbo );
        gl::ScopedViewport    scpViewport( mFbo->getSize() );
        gl::clear();
    }
}

void ParticlesApp::update()
{
    p.update();
}

void ParticlesApp::draw()
{
    render();
    // Now render the scene to the main buffer.
    gl::draw( mFbo->getColorTexture() );
}

void ParticlesApp::render()
{
    // Bind the Fbo and update the viewport.
    gl::ScopedFramebuffer scpFbo( mFbo );
    
    gl::pushMatrices();
    gl::setMatricesWindow( mFbo->getSize() );
    // Enable pre-multiplied alpha blending.
    gl::ScopedBlendPremult scpBlend;
    // Draw transparent black rectangle.
//    gl::ScopedColor scpColor( 0, 0, 0, 0.1f / 255.0f );
    gl::ScopedColor scpColor( 0, 0, 0, 255.0f / 255.0f );
    gl::drawSolidRect( { 0, 0, (float)getWindowWidth(), (float)getWindowHeight() } );
    gl::popMatrices();
    
    gl::pushMatrices();
    gl::setMatrices( mCamera );
    // Draw a white circle.
    p.draw();
    gl::color(Color(1,1,1));
    
//    gl::drawSphere(vec3(0,0,0), 1);
//    for(int i = 0; i < 4; i++){
////        cinder::gl::color(color / i);
//        cinder::gl::drawSphere(p.poss[i], 1);
//        console() << p.poss[i] << endl;
//    }
//    console() << "====================" << endl;
    gl::popMatrices();
}

CINDER_APP( ParticlesApp, RendererGl )
