#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class CurlNoiseApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void CurlNoiseApp::setup()
{
}

void CurlNoiseApp::mouseDown( MouseEvent event )
{
}

void CurlNoiseApp::update()
{
}

void CurlNoiseApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( CurlNoiseApp, RendererGl )
