#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Camera.h"
#include "cinder/CameraUi.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class chaosApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    
    void mouseDown( MouseEvent event ) override;
    void mouseDrag( MouseEvent event ) override;
    void keyDown( KeyEvent event ) override;
    
private:
    CameraPersp            mCamera;
    CameraUi            mCamUi;
    bool                mRecenterCamera;
    vec3                mCameraTarget, mCameraLerpTarget, mCameraViewDirection;
    double                mLastMouseDownTime;
};

void chaosApp::setup()
{
    gl::enableVerticalSync( true );
    gl::clear( Color( 0, 0, 0 ) );
}

void chaosApp::mouseDown( MouseEvent event )
{
}

void chaosApp::update()
{
}

void chaosApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( chaosApp, RendererGl )
