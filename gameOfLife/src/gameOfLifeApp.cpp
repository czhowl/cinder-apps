#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class gameOfLifeApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void gameOfLifeApp::setup()
{
}

void gameOfLifeApp::mouseDown( MouseEvent event )
{
}

void gameOfLifeApp::update()
{
}

void gameOfLifeApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( gameOfLifeApp, RendererGl )
