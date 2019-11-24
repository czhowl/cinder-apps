#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>
using namespace ci;
using namespace ci::app;
using namespace std;

class realsenseApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;

	// Declare depth colorizer for pretty visualization of depth data
	rs2::colorizer color_map;
	// Declare rates printer for showing streaming rates of the enabled streams.
	rs2::rates_printer printer;

	// Declare RealSense pipeline, encapsulating the actual device and sensors
	rs2::pipeline pipe;
	gl::Texture2dRef			mTex;
};

void realsenseApp::setup()
{
	rs2::config cfg;
	cfg.enable_stream(RS2_STREAM_INFRARED, 640, 480, RS2_FORMAT_Y8, 30);
	cfg.enable_stream(RS2_STREAM_DEPTH, 640, 480, RS2_FORMAT_Z16, 30);
	pipe.start(cfg);
	rs2::frameset frames;
	for (int i = 0; i < 30; i++)
	{
		//Wait for all configured streams to produce a frame
		frames = pipe.wait_for_frames();
	}
}

void realsenseApp::mouseDown( MouseEvent event )
{
}

void realsenseApp::update()
{
	rs2::frameset frames = pipe.wait_for_frames();
	//Get each frame
	rs2::frame ir_frame = frames.first(RS2_STREAM_INFRARED);
	rs2::frame depth_frame = frames.get_depth_frame();
	mTex = gl::Texture2d::create((void*)ir_frame.get_data(), GL_RED, 640, 480, gl::Texture::Format().loadTopDown());
}

void realsenseApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	if (mTex) gl::draw(mTex);
}

CINDER_APP( realsenseApp, RendererGl )
