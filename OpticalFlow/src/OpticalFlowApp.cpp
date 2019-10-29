#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>
#include "CinderOpenCV.h"

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace cv;

#define WIDTH 800
#define HEIGHT 480

class OpticalFlowApp : public App {
  public:
	static void prepare(Settings* settings);
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;

	VideoCapture		mCap;
	gl::TextureRef		mTexture;

	Mat frame2, next;
	Mat frame1, prvs;
};

void OpticalFlowApp::prepare(Settings* settings)
{
	auto displays = Display::getDisplays();

	settings->disableFrameRate();
	//settings->setBorderless(true);
	//settings->setWindowPos(0, 0);
	settings->setWindowSize(WIDTH, HEIGHT);

//#if !_DEBUG
//	settings->setFullScreen(true);
//#endif
}

void OpticalFlowApp::setup()
{
	if (!mCap.open(0, CAP_DSHOW)) {
		console() << "failed" << endl;
		throw "camera initialization failed!";
	}
	mCap.set(CAP_PROP_FRAME_WIDTH, WIDTH);
	mCap.set(CAP_PROP_FRAME_HEIGHT, HEIGHT);

	while (!mCap.read(frame1));
	cvtColor(frame1, prvs, COLOR_BGR2GRAY);

}

void OpticalFlowApp::mouseDown( MouseEvent event )
{
}

void OpticalFlowApp::update()
{
	if (mCap.read(frame2)) {
		if (frame2.empty())
			return;
		cvtColor(frame2, next, COLOR_BGR2GRAY);
		Mat flow(prvs.size(), CV_32FC2);
		calcOpticalFlowFarneback(prvs, next, flow, 0.5, 3, 15, 3, 5, 1.2, 0);
		// visualization
		Mat flow_parts[2];
		split(flow, flow_parts);
		Mat magnitude, angle, magn_norm;
		cartToPolar(flow_parts[0], flow_parts[1], magnitude, angle, true);
		normalize(magnitude, magn_norm, 0.0f, 1.0f, NORM_MINMAX);
		angle *= ((1.f / 360.f) * (180.f / 255.f));
		//build hsv image
		Mat _hsv[3], hsv, hsv8, bgr;
		_hsv[0] = angle;
		_hsv[1] = Mat::ones(angle.size(), CV_32F);
		_hsv[2] = magn_norm;
		merge(_hsv, 3, hsv);
		hsv.convertTo(hsv8, CV_8U, 255.0);
		cvtColor(hsv8, bgr, COLOR_HSV2BGR);
		//imshow("frame2", bgr);
		prvs = next;
		//mTexture = gl::Texture::create(fromOcv(bgr), gl::Texture::Format().loadTopDown());
		mTexture = gl::Texture::create((void *)bgr.data, GL_BGR, bgr.cols, bgr.rows, gl::Texture::Format().loadTopDown());
	}
}

void OpticalFlowApp::draw()
{
	gl::clear(Color(0, 0, 0));
	if (mTexture) {
		gl::draw(mTexture);
	}
}

CINDER_APP(OpticalFlowApp, RendererGl(RendererGl::Options().msaa(16)), &OpticalFlowApp::prepare)
