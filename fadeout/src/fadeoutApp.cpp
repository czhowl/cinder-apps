#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CameraUi.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class fadeoutApp : public App {
public:
    void draw() override;
    void resize() override;
    
//    void mouseMove( MouseEvent event ) override;
    
    void render();
    
private:
    gl::FboRef mFbo;
    ci::ivec2  mMousePos;
    
    CameraPersp        mCamera;
    CameraUi           mCamUi;
};

void fadeoutApp::draw()
{
    // Render scene to Fbo. This allows us to keep the contents of the previous
    // frame.
    render();
    // Now render the scene to the main buffer.
    gl::draw( mFbo->getColorTexture() );
}

//void fadeoutApp::mouseMove( MouseEvent event )
//{
//    // Keep track of the mouse position in window coordinates.
//    mMousePos = event.getPos();
//}

void fadeoutApp::resize()
{
    // Create an Fbo the size of the window.
    mCamUi = CameraUi( &mCamera, getWindow() );
//    gl::setMatrices( mCamera );
    
    gl::Fbo::Format fboFormat;
    fboFormat.setColorTextureFormat( gl::Texture2d::Format().internalFormat( GL_RGBA32F ) );
    mFbo = gl::Fbo::create( getWindowWidth(), getWindowHeight(), fboFormat );
    
    // Clear once at setup.
    gl::ScopedFramebuffer scpFbo( mFbo );
    gl::ScopedViewport    scpViewport( mFbo->getSize() );
    gl::clear();
    
    
    
    
}

void fadeoutApp::render()
{
    // Bind the Fbo and update the viewport.
    gl::ScopedFramebuffer scpFbo( mFbo );
//    gl::ScopedViewport    scpViewport( mFbo->getSize() );
//    gl::ScopedProjectionMatrix scpMatrices;

//    gl::setMatrices( mCamera );
    gl::pushMatrices();
    gl::setMatricesWindow( mFbo->getSize() );
    
    // Enable pre-multiplied alpha blending.
    gl::ScopedBlendPremult scpBlend;
//    gl::setMatricesWindow( mFbo.getSize() );
    // Draw transparent black rectangle.
    gl::ScopedColor scpColor( 0, 0, 0, 4.0f / 255.0f );
    gl::drawSolidRect( { 0, 0, (float)getWindowWidth(), (float)getWindowHeight() } );
    gl::popMatrices();
    gl::pushMatrices();
//    gl::setMatricesWindowPersp( mFbo->getSize() );
    gl::setMatrices( mCamera );
    // Draw a white circle.
    gl::color( 1, 1, 1 );
    gl::drawSphere(vec3(3,3,3), 3);
    gl::popMatrices();
}

CINDER_APP( fadeoutApp, RendererGl )
