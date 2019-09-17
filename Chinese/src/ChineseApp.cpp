#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class ChineseApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void ChineseApp::setup()
{
}

void ChineseApp::mouseDown( MouseEvent event )
{
}

void ChineseApp::update()
{
}

void ChineseApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( ChineseApp, RendererGl )
