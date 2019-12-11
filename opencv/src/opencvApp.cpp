#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include <opencv2/core/core.hpp>
#include "CinderOpenCV.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace cv;

#define WIDTH 1920
#define HEIGHT 1080

class opencvApp : public App {
  public:
	static void prepare(Settings* settings);
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;

	VideoCapture		mCap;
	gl::TextureRef		mTexture;
};

void opencvApp::prepare(Settings* settings)
{
	auto displays = Display::getDisplays();

	settings->disableFrameRate();
	settings->setBorderless(true);
	//settings->setWindowPos(0, 0);
	settings->setWindowSize(WIDTH, HEIGHT);

#if !_DEBUG
	settings->setFullScreen(true);
#endif
}

void opencvApp::setup()
{
	if (!mCap.open(0, CAP_DSHOW)) {
		console() << "failed" << endl;
		throw "camera initialization failed!";
	}
	mCap.set(CAP_PROP_FRAME_WIDTH, WIDTH);
	mCap.set(CAP_PROP_FRAME_HEIGHT, HEIGHT);
    mCap.set(CAP_PROP_BUFFERSIZE, 1);
}

void opencvApp::mouseDown( MouseEvent event )
{
}

void opencvApp::update()
{
	Mat src;
	if (mCap.read(src)) {
		mTexture = gl::Texture::create(fromOcv(src), gl::Texture::Format().loadTopDown());;
	}
	else
		mTexture = NULL;
}

void opencvApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
	if (mTexture) {
		gl::draw(mTexture);
	}
}

CINDER_APP( opencvApp, RendererGl(RendererGl::Options().msaa(16)), &opencvApp::prepare)
