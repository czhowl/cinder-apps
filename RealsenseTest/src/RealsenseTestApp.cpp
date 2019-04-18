#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class RealsenseTestApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void RealsenseTestApp::setup()
{
}

void RealsenseTestApp::mouseDown( MouseEvent event )
{
}

void RealsenseTestApp::update()
{
}

void RealsenseTestApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( RealsenseTestApp, RendererGl )
