#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/params/Params.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class SimpleVisualApp : public App {
  public:
    static void prepare(Settings *settings);
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    
    params::InterfaceGlRef          mParams;
};

void SimpleVisualApp::prepare(Settings *settings)
{
    settings->setTitle("Thesis Animation");
    settings->setWindowSize(800, 800);
    settings->setHighDensityDisplayEnabled();
    settings->disableFrameRate();
}

void SimpleVisualApp::setup()
{
    mParams = params::InterfaceGl::create( getWindow(), "App parameters", toPixels( ivec2( 200, 400 ) ) );
}

void SimpleVisualApp::mouseDown( MouseEvent event )
{
}

void SimpleVisualApp::update()
{
}

void SimpleVisualApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    
    mParams->draw();
}

CINDER_APP( SimpleVisualApp, RendererGl(RendererGl::Options().msaa(16)), &SimpleVisualApp::prepare )
